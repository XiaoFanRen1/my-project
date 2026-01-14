#include "system/includes.h"
#include "usb_scsi_upgrade.h"
#include "event/key_event.h"
#include "event/device_event.h"
#include "fs/fs.h"

#include "app_config.h"

#define UVC_JL_SLAVE_UPGRADE_EN   0   //升级杰理从机功能

#if UVC_JL_SLAVE_UPGRADE_EN

#define AC53X_BFU_FILE_PATH     CONFIG_ROOT_PATH"AC53x.bfu"
/* #define AC53X_BFU_FILE_PATH     CONFIG_ROOT_PATH"AC52x.bfu" */
/* #define AC53X_BFU_FILE_PATH     CONFIG_ROOT_PATH"AC57s.bfu" */

static volatile u8 enter_upgrade = 0;

int usb_burn_state = -1; /* 记录usb升级状态 */

/**
 * @brief uvc从机进入升级模式函数
 *
 * @param void
 *
 * @return 零表示失败
 *         非零表示成功,成功后将进入 uboot_dev_event_handler 的 DEVICE_EVENT_IN 分支，在里面读取bfu并升级
 */
int uvc_enter_upgrade_mode(void)
{
    usb_burn_state = -1;
    int usb_host_slave_enter_upgrade(u8 id);
    enter_upgrade = usb_host_slave_enter_upgrade(uvc_host_online());
    if (!enter_upgrade) {
        log_e("uvc_enter_upgrade faild!! %d\n", enter_upgrade);
    }

    return enter_upgrade;
}

/**
 * @brief uvc从机升级过程进度回调,里面获取升级进度百分比
 *
 * @param percent: 进度百分比
 *
 * @return
 */
void usb_upgrade_progress_percent(u32 percent)
{
    printf("upgrade >> %d\n", percent);
    return;
}

/**
 * @brief uvc从机升级过程中异常处理
 *        升级过程中中断，即变砖，从机大概率会进maskrom 的uvc模式
 *        在改函数重新发命令进入升级模式
 *
 * @param void
 *
 * @return
 */
int uvc_frame_not_find(void)
{
    printf("uvc frame not find\n");
    if (enter_upgrade) {
        uvc_enter_upgrade_mode();
    }
    return 1;
}

/**
 * @brief 设置 uvc从机是否成功进入升级模式
 * @param success:
 *        0: 未进入升级模式
 *        1: 进入了升级模式
 * @return
 */
void set_uvc_upgrade_state(u8 success)
{
    enter_upgrade = success;
}
/**
 * @brief 获取 uvc从机是否进入了升级模式
 * @param void
 * @return
 *        0: 未进入升级模式
 *        1: 进入了升级模式
 */
u8 get_uvc_upgrade_state(void)
{
    return enter_upgrade;
}

static void uboot_dev_event_handler(struct device_event *event)
{
    char *const udisk_list[] = {
        "udisk0",
        "udisk1",
    };
    int id = ((char *)event->value)[5] - '0';
    const char *dev  = udisk_list[id];

    FILE *fp = NULL;
    u8 *bfu_stream = NULL;
    int bfu_len = 0;
    u8 retry = 0;

    switch (event->event) {
    case DEVICE_EVENT_IN:
        fp = fopen(AC53X_BFU_FILE_PATH, "r");
        if (!fp) {
            printf("open bfu faild!!!\n");
            enter_upgrade = 0;
            break;
        }

        printf("\n %s %s: in\n", __func__, dev);
        bfu_len = flen(fp);
        printf("bfu_len=%d\n", bfu_len);
        bfu_stream = malloc(bfu_len);
        if (!bfu_stream) {
            fclose(fp);
            enter_upgrade = 0;
            break;
        }
        fread(bfu_stream, bfu_len, 1, fp);

        void *fd = dev_open(dev, NULL);

        //升级进程主函数
        u32 usb_burn_process(unsigned char *bfu_stream, int bfu_len);
        retry = 3; //重试升级流程
        while (retry--) {
            usb_burn_state = usb_burn_process(bfu_stream, bfu_len);
            if (usb_burn_state != UPGRADE_CODE_SUCCESS) {
                log_d("usb burn err retry = %d\n", retry);
                continue;
            } else {
                enter_upgrade = 0;
                break;
            }
        }
        if (usb_burn_state != UPGRADE_CODE_SUCCESS) {
            log_e(">>>> usb burn faild = %d!\n", usb_burn_state);
        }
        if (bfu_stream) {
            free(bfu_stream);
            bfu_stream = NULL;
        }
        fclose(fp);
        break;
    case DEVICE_EVENT_OUT:
        /* printf("\n %s %s: out\n", __func__, dev); */
        break;
    }
}

static int uvc_upgrade_event_handler(struct sys_event *event)
{
    FILE *fp = NULL;

    struct device_event *eve = NULL;
    struct key_event *key_eve = NULL;
    char *usb_msg;
    u8 usbid;

    if (event->type == SYS_KEY_EVENT) {
        key_eve = (struct key_event *)event->payload;
        printf("\n[ debug ]--func=%s line=%d key_eve action== %d\n", __func__, __LINE__, key_eve->action);
        if (key_eve->action == KEY_EVENT_CLICK) {
            printf("\n[ debug ]--func=%s line=%d key_eve value== %d\n", __func__, __LINE__, key_eve->value);
            if (key_eve->value == KEY_OK) {
                //for test
                fp = fopen(AC53X_BFU_FILE_PATH, "r");
                if (fp) {
                    fclose(fp);
                    //根目录下有AC53升级文件，按OK键进入升级模式
                    //用户也可设计其他方式触发进入升级
                    uvc_enter_upgrade_mode(); //测试进入升级模式
                    return -EINVAL;
                }
            }
        }
    }

    if (event->from == DEVICE_EVENT_FROM_USB_HOST) {
        eve = (struct device_event *)event->payload;
        printf("\n[ debug ]--func=%s line=%d %s\n", __func__, __LINE__, eve->value);
        if (!strncmp(eve->value, "udisk", 5)) {
            /* if (enter_upgrade) { */
            uboot_dev_event_handler(eve);
            return -EINVAL;
            /* } */
        }
    }

    return 0;
}
/*
 * 静态注册设备事件回调函数，优先级为0
 */
SYS_EVENT_STATIC_HANDLER_REGISTER(jl_uvc_upgrade_event) = {
    .event_type     = SYS_DEVICE_EVENT | SYS_KEY_EVENT,
    .prob_handler   = uvc_upgrade_event_handler,
    .post_handler   = NULL,
};


#else
u8 get_uvc_upgrade_state(void)
{
    return 0;
}

#endif

