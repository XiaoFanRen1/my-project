#include "lib_include.h"
#include "rand64.h"
#include "thread.h"
#include "common.h"
#include "btctrler/hci_transport.h"
#include "dev_update/fs_v2_update.h"
#include "dev_update/ex_api_code.h"
#include "update_main.h"
#include "asm/wdt.h"
#include "custom_cfg.h"
#include "sys_timer.h"

enum {
    SEEK_SET = 0x01,
    SEEK_CUR = 0x02
};
/*---------------------extern api start --------------------*/
extern void lmp_hci_write_local_name(const char *name);
extern int link_bulk_init();
extern void btctler_nv_memory_apply(void);

extern struct common_rf_param rf;
extern void bt_max_pwr_set(u8 pwr, u8 pg_pwr, u8 iq_pwr, u8 ble_pwr);
extern int lmp_hci_write_scan_enable(u8 scan_enable);
extern void stack_run_loop_resume();
extern void sys_timer_var_init();
extern void lmp_hci_write_local_address(const u8 *addr);
extern void lmp_hci_write_super_timeout(u16 timeout);
extern void bt_set_osc_cap(u8 sel_l, u8 sel_r);
extern void bt_rf_init(void);
extern void bredr_bd_init(void);
extern void bredr_link_init(void);
extern void lmp_init();
extern void reg_schedule_hdl(void *hdl);
extern void task_message_init(void);
extern void sys_timer_schedule();

extern void btstack_hci_init();
extern void btstack_mem_init(void);
extern void btstack_bredr_le_init();

extern void init_soft_interrupt();
extern void ble_profile_init(void);
extern void hci_interface_setup(void);
extern void bt_ble_init(void);
extern void ll_reset(void);
extern void ll_hci_init(void);
extern int le_controller_set_mac(void *addr);
extern void udelay(u32 nus);

extern const hci_transport_t *h4_transport;
extern const hci_transport_t *hci_transport_h4_controller_instance(void);

extern volatile u32 bt_time_timeout;
#define UPDATE_CMD_WAIT_TIMEOUT	(2000UL) //unit:2ms

extern void CLR_WDT(void);
static int bt_task_create(void (*fun)(u8), u8 priority)
{
    /*printf("prio: %d, fun=%x\n", priority, fun);*/
    return 0;
}

static int bt_task_delete(u8 priority)
{
    return 0;
}

static int bt_task_suspend(u8 priority, u8 timeout)
{
    //u32 ulNotificationValue;
    /* low_power_unlock(); */
    /* rtos_sem_pend(&__this->sem, timeout); */
    /* log_info(">"); */

    //ulNotificationValue = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    /* log_info("0-ulNotificationValue 0x%x", ulNotificationValue ); */

    return 0;
}

static int bt_task_resume(void)
{
    /* low_power_lock(); */
    /* rtos_sem_post(&__this->sem); */
    /* log_info("<"); */

    /*xTaskNotifyGive(__this->xTaskToNotify);*/
    //os_taskq_post_type(THIS_TASK_NAME, 0xff, 0, NULL);
    stack_run_loop_resume();
    return 0;
}

/*---------------------extern api end	--------------------*/
struct thread_owner bt_task_thread = {
    .create         = (int (*)(void (*fun)(struct thread_owner *), u8))bt_task_create,
    .delete         = bt_task_delete,
    .resume         = bt_task_resume,
    .suspend        = bt_task_suspend,
};

static u8 ble_mac_addr[6] = {0x11, 0x22, 0x33, 0x33, 0x22, 0x11};
u8 pin_code[4] = {0};
void JL_rcsp_set_no_auth(void);
void JL_rcsp_set_pvid_flag(u8 flag);
static void btctler_init(void)
{
    btctler_nv_memory_apply();
    //link_bulk_init();

    //Create thread run loop for controller layers
    thread_owner_init(&bt_task_thread);
#if 0
    //设置本地蓝牙名称
    lmp_hci_write_local_name(BT_LOCAL_NAME);
    printf("bt_name:%s\n", BT_LOCAL_NAME);

    //生成随机地址
    u8 bt_addr[6];
    pseudo_random_generate(bt_addr, sizeof(bt_addr));
    puts("bt_addr:\n");
    printf_buf(bt_addr, sizeof(bt_addr));
#endif
    //rf_bredr_max_pwr_set(10);

    h4_transport = hci_transport_h4_controller_instance();
    bt_set_osc_cap(0xa, 0xa);
    bt_rf_init();
    bt_max_pwr_set(10, 5, 8, 9);

    //bredr_bd_init();
    //bredr_link_init();
    //lmp_init();
    //ll_reset();
    ll_hci_init();

    //Hci setup
    hci_interface_setup();

#if BLE_GATT_UPDATA_MODULE_CONTROL
    ex_cfg_get_content_api(CFG_ITEM_BLE_ADDR, ble_mac_addr, sizeof(ble_mac_addr));
    ex_cfg_get_content_api(CFG_ITEM_PIN_CODE, pin_code, sizeof(pin_code));
    printf("pin code\n");
    put_buf(pin_code, 4);
    if (pin_code[0] != 0) {
        //不进行校验
        JL_rcsp_set_no_auth();
    }

    if (pin_code[1] != 0) {
        JL_rcsp_set_pvid_flag(0);
    }
    puts("ble_mac_addr:\n");
    put_buf(ble_mac_addr, sizeof(ble_mac_addr));
    le_controller_set_mac(ble_mac_addr);//BLE广播地址

#endif

#if 0
    //配置蓝牙地址
    lmp_hci_write_local_address(bt_addr);

    //设置超时时间
    lmp_hci_write_super_timeout(4000);

    lmp_hci_write_scan_enable(0x3);
#endif
}

static void btstack_init(void)
{
    btstack_hci_init();
    btstack_mem_init();
    btstack_bredr_le_init();
}

/* extern u16 get_le_pool_len(void);

static u8 app_le_pool[1820] sec(.btstack_pool)  ALIGNED(4);

u8 *get_le_pool_addr(void)
{
    u16 len = 0;

    len = get_le_pool_len();
    printf("le pool len %d\n", len);
    if (len > sizeof(app_le_pool)) {
        ASSERT(0, "le_pool is small\n");
    }

    return &app_le_pool;
} */

#define _ETM_CON                q32DSP(0)->ETM_CON
UPDATA_RESULT ble_gatt_update_loop(void)
{
#if BLE_GATT_UPDATA_MODULE_CONTROL
    //init message queue
    task_message_init();

    //init sys timer
    sys_timer_var_init();
    /* sys_timer_delay_handler_register(stack_run_loop_resume); */
    /* reg_schedule_hdl(sys_timer_schedule); */

    //init btstack run isr
    init_soft_interrupt();

    //bt ctrler init
    btctler_init();

    //ble_profile_init();
    btstack_init();

    bt_ble_init();

    sys_timer_add(NULL, stack_run_loop_resume, 5);

    return ble_updata_run();

#else
    return DEV_UPDATA_SUCC;
#endif
}



UPDATA_RESULT spp_update_loop(void)
{
#if SPP_UPDATA_MODULE_CONTROL
    return DEV_UPDATA_SUCC;
#else
    return DEV_UPDATA_SUCC;
#endif
}

#include "lib_include.h"
//#include "ble_updata.h"
//#include "dev_fs_connect.h"
//#include "standard_hid.h"
#include "boot.h"
//#include "hid_server.h"
#include "hw_cpu.h"
#include "msg.h"
#include "rcsp_control.h"
#include "custom_cfg.h"
#define LOG_TAG     "[ble_updata]"

#define LOG_ERROR_ENABLE
#define LOG_INFO_ENABLE
#define LOG_DUMP_ENABLE
#include "debug.h"

#if USB_HID_MODULE_CONTROL
#define UPDATE_PRE_READ_EN	0
#else
#define UPDATE_PRE_READ_EN	1
#endif

typedef enum __UPDATA_LED_STATUS {
    UPDATA_STA_LED_CLOSE = 0,
    UPDATA_STA_LED_OPEN,
    UPDATA_STA_LED_SIGN,
    UPDATA_STA_LED_FILCKER,
} _UPDATA_LED_STATUS;
static _UPDATA_LED_STATUS u_led_status = UPDATA_STA_LED_FILCKER;

static void set_ble_work_state(u8 state);

typedef enum __UPDATA_STATUS {
    UPDATA_STA_DEV_SLEEP = 0,
    UPDATA_STA_DEV_DISCONNECTION,
    UPDATA_STA_DISCONNECTION_COMPLETED,
    UPDATA_STA_DEV_CONNECTION,
    UPDATA_STA_READY_UPDATE,
    UPDATA_STA_BFU_UPDATE_LOOP,
    UPDATA_STA_WAIT_FOR_REBOOT,
} _UPDATA_STATUS;

static volatile _UPDATA_STATUS u_ble_status = UPDATA_STA_DEV_SLEEP;
static volatile ble_control_info_t ble_control_info;
static updata_write_flash_flag = 0;

static void (*ble_update_msg_deal_handle)(void *hdl, int *msg) = NULL;

void ble_update_msg_deal_handle_register(void (*hdl)(void *priv, int *msg))
{
    ble_update_msg_deal_handle = hdl;
}

#define FLASH_WRITE_ONCE_LEN		512
static u8 write_enable_flag = 0;
typedef struct _update_pre_read_ctl {
    u8 pre_read_cnt;
    u8 pre_read_flag;
    volatile u8 buf_busy_flag;
    u32 curr_file_offset;
    u32 last_addr;
    u8 *buf0;
    u32 buf0_addr;
    u8 *buf1;
    u32 buf1_addr;
} update_pre_read_ctl_t;

#define ONCE_UPDATE_READ_LEN	512
#define CONTINUE_READ_TIMES	    5

static u8 update_temp_buf[ONCE_UPDATE_READ_LEN * 2] __attribute__((aligned(0x4)));
static volatile update_pre_read_ctl_t update_pre_read;

static update_op_api_t *p_update_op = NULL;
#define __this	(p_update_op)

//用read_buf的后面512K作为预读buff,当出现开始预读标志的时候,先预读多一包,之前判断地址是否符合,若符合直接拷贝并预读下一包;
//1.判断开始预读条件;连续5次读512byte的时候,开始自动发送预读包;
//2.
//extern void XM_recieve_parse_resume(void);
enum {
    PRE_READ_FLAG_DISABLE = 0,
    PRE_READ_FLAG_TOGGLE_TO_DIS,
    PRE_READ_FLAG_TOGGLE_TO_EN,
    PRE_READ_FLAG_ENABLE,
};

static u32 dev_read_update_block_handle(u8 *buffer, u16 buffer_size)
{
    printf("-rx data:%x\n", buffer_size);
    //putchar('R');
    if (ble_control_info.btupdata_sw == UPDATA_READ) {
        //log_info("-up read\n");
        //防止传入buffer小仍被越界写;
        if (ble_control_info.need_rx_len !=  buffer_size) {
            log_info("\n\n\nLEN ERR!!!:%x\n\n\n", ble_control_info.need_rx_len);
            putchar('L');
            //ble_control_info.date_len = 0;
            return 0;
            //ble_control_info.btupdata_sw = UPDATA_ACCEPT_OK;
            //ble_control_info.ble_update_state |= LMP_STA_UPDATA_ACCEPT;
        }

#if ONE_TIME_RECEIVE_ALL_DATA
        if (ble_control_info.read_buf) {
            if (ble_control_info.once_write_len >= ble_control_info.once_rx_len + buffer_size) {
                memcpy(ble_control_info.read_buf + ble_control_info.once_rx_len, buffer, buffer_size);
            } else {
                memcpy(ble_control_info.read_buf + ble_control_info.once_rx_len, buffer, buffer_size);
                //deg_printf("w_len_ov:%x\n",ble_control_info.once_rx_len+buffer_size-ble_control_info.once_write_len);

            }
            /* deg_printf("buffer_size=0x%x\n", buffer_size); */
            /* deg_printf_buf(ble_control_info.read_buf, buffer_size); */
        }
        ble_control_info.once_rx_len += buffer_size;
#endif
        ble_control_info.date_len = buffer_size;
        /* deg_printf("a:%x n:%x\n",ble_control_info.date_len,ble_control_info.need_rx_len); */
        //要写flash必须读长度大于512，否则不能写flash
#if ONE_TIME_RECEIVE_ALL_DATA
        if (updata_write_flash_flag && (ble_control_info.need_rx_len == ble_control_info.date_len)) {
            //&& (ble_control_info.date_len%FLASH_WRITE_ONCE_LEN == 0)
            // ble_control_info.once_rx_len = 0;
            //log_info("f_addr:%x\n", ble_control_info.flash_write_addr);
            spi0flash_any_write(buffer, ble_control_info.flash_write_addr, FLASH_WRITE_ONCE_LEN);
            ble_control_info.flash_write_addr += FLASH_WRITE_ONCE_LEN;
            ble_control_info.once_rx_len -= ble_control_info.once_write_len;

            if (ble_control_info.once_rx_len) {
                //deg_printf("rx_len:%x\n",ble_control_info.once_rx_len);
                memcpy(ble_control_info.read_buf, ble_control_info.read_buf + ble_control_info.once_write_len, ble_control_info.once_rx_len);
            }
        }
#endif
        if ((ble_control_info.need_rx_len == ble_control_info.date_len)) {
            //log_info("rx len ok\n");
#if UPDATE_PRE_READ_EN
            if (update_pre_read.pre_read_flag > PRE_READ_FLAG_DISABLE) {
                if (!(update_pre_read.buf_busy_flag & BIT(0))) {
                    memcpy(update_pre_read.buf0, buffer, ble_control_info.date_len);
                    update_pre_read.buf_busy_flag |= BIT(0);
                    update_pre_read.buf0_addr = update_pre_read.curr_file_offset;
                } else if (!(update_pre_read.buf_busy_flag & BIT(1))) {
                    memcpy(update_pre_read.buf1, buffer, ble_control_info.date_len);
                    update_pre_read.buf_busy_flag |= BIT(1);
                    update_pre_read.buf1_addr = update_pre_read.curr_file_offset;
                }

                putchar('0' + update_pre_read.buf_busy_flag);
                log_info("offset:%x busy:%x\n", update_pre_read.curr_file_offset, update_pre_read.buf_busy_flag);
                update_pre_read.curr_file_offset = update_pre_read.curr_file_offset - ONCE_UPDATE_READ_LEN;
                //ble_control_info.btupdata_sw = UPDATA_ACCEPT_OK;
                //ble_control_info.ble_update_state |= LMP_STA_UPDATA_ACCEPT;
                return 0;
            }
#endif
            memcpy(ble_control_info.read_buf, buffer, ble_control_info.date_len);
            ble_control_info.btupdata_sw = UPDATA_ACCEPT_OK;
            ble_control_info.ble_update_state |= LMP_STA_UPDATA_ACCEPT;
        } else {
            putchar('N');
        }
    } else {
        putchar('K');
        //XM_recieve_parse_resume();
    }

    return 0;
}

static u8 jl_device_update_status = DEVICE_UPDATE_STA_FAIL;

static u8 dev_update_status_req_handle(void)
{
    log_info("-TRIGGER RETURN\n");
    if (ble_control_info.btupdata_sw == UPDATA_STOP) {
        ble_control_info.btupdata_sw = UPDATA_ACCEPT_OK;
        ble_control_info.ble_update_state |= LMP_STA_UPDATA_ACCEPT;
    } else if (ble_control_info.btupdata_sw == UPDATA_STOP_KEYERR) {
        ble_control_info.btupdata_sw = UPDATA_ACCEPT_OK;
        ble_control_info.ble_update_state |= LMP_STA_UPDATA_ACCEPT;
    }

    return jl_device_update_status;
}

static void ble_set_jl_update_status(u8 status)
{
    jl_device_update_status = status;
}

u8 *get_update_pre_read_buf(u32 addr, u8 *buf,  u16 len);
u32 ble_updata_handler(u8 btupdata_sw, u8 *rec, u32 len)
{
    u32 t;

    u8 *pre_read_buf;
    bt_time_timeout = 0;
    //printf("sw:%x\n", btupdata_sw);
    while (1) {
        wdt_clear();
        if (bt_time_timeout > UPDATE_CMD_WAIT_TIMEOUT) {            //APP直接切到后台，响应不了命令，在这里添加2S超时
            putchar('O');
            return 0;
        }
        switch (btupdata_sw) {
        case UPDATA_WAIT_REBOOT:
            log_info("tmr:%d wait_timeout_instant:%d\n", tmr_2ms_cnt_get(), ble_control_info.wait_timeout_instant);
            if (tmr_2ms_cnt_get() >= ble_control_info.wait_timeout_instant) {           //已经到达超时的时刻，直接返回
                log_info("already timeout.\n");
                return 1;
            }
        case UPDATA_OPEN:
        case UPDATA_SEEK:
        case UPDATA_STOP:
        case UPDATA_STOP_KEYERR:
            if ((ble_control_info.ble_update_state & LMP_STA_UPDATA_ACCEPT) && (ble_control_info.btupdata_sw == UPDATA_ACCEPT_OK)) {
                log_info("return form handler\n");
                return 1;
            }
            break;

        case UPDATA_READ:
#if UPDATE_PRE_READ_EN
            //log_info("*");
            pre_read_buf = get_update_pre_read_buf((u32)rec, ble_control_info.read_buf, len);
            if (NULL != pre_read_buf) {
                return len;
                //ble_control_info.btupdata_sw = UPDATA_ACCEPT_OK;
                //ble_control_info.ble_update_state |= LMP_STA_UPDATA_ACCEPT;
            }
#endif
            if ((ble_control_info.ble_update_state & LMP_STA_UPDATA_ACCEPT) &&
                (ble_control_info.btupdata_sw == UPDATA_ACCEPT_OK)) {
                //(ble_control_info.date_len == len)) {
                ble_control_info.read_buf = NULL;
                if (0 == ble_control_info.date_len) {
                    log_info("-UPDATE READ ERR!\n");
                }
                //log_info("read ret\n");
                return ble_control_info.date_len;
            }
            break;

        default:
            break;
            //return 0;

        }

        if (UPDATA_STA_DISCONNECTION_COMPLETED == u_ble_status) {
            log_info("RETURN DUE TO DISCON\n");
            return 0;
        } else if (UPDATA_STA_READY_UPDATE == u_ble_status) {
            log_info("RETURN DUE TO ATT_READY\n");
            return 0;
        }

        if (t++ == 100000) {
            putchar('.');
            t = 0;
        }

    }

    log_info("EXCEPTION RETURN\n");
    return 0;
}



static int ble_updata_control(void *conn, u8 btupdata_sw, u32 len, u8 seek_type, u32 seek_offsize)
{
    void *ret;
    //printf("---addr:%x len:%x\n", seek_offsize, len);
    if (p_update_op && p_update_op->data_send) {
        ret = p_update_op->data_send(0, btupdata_sw, seek_type, seek_offsize, len);
    } else {
        log_error("p_update_op->data_send null\n");
        ret = NULL;
    }

    return (int)ret;
}

static int ble_notify_update_content_size(void *priv, u32 size)
{
    void *ret;
    if (p_update_op && p_update_op->notify_update_content_size) {
        ret = p_update_op->notify_update_content_size(priv, size);
    } else {
        log_error("notify_update_content_size handle is null\n");
        ret = NULL;
    }

    return (int)ret;
}

void update_pre_read_ctl_init(void);
static u32 ble_file_offset = 0;
u16 ble_updata_bfu_open(void *conn, u8 type, void *buf, u16 btr)
{
    ble_file_offset = 0;
#if UPDATE_PRE_READ_EN
    update_pre_read_ctl_init();
#endif
    return 1;
}


void ble_updata_rsp_complete(void)
{
    if (UPDATA_STA_BFU_UPDATE_LOOP != u_ble_status) {
        log_info("ble sta err2\n");
        return;
    }
    ble_control_info.btupdata_sw = UPDATA_STOP;
    ble_control_info.ble_update_state &= ~LMP_STA_UPDATA_ACCEPT;
    ble_control_info.date_len = 0;
    ble_control_info.need_rx_len = 0;

    log_info("send zero addr\n");
    ble_updata_control(NULL, UPDATA_STOP, 0, 0, 0);
}

u16 ble_updata_stop(u8 err)
{
    //return 1;
    u16 stop_state;
    if (UPDATA_STA_BFU_UPDATE_LOOP != u_ble_status) {
        log_info("ble sta err0\n");
        return 0;
    }
    if ((err >= DEVICE_UPDATE_STA_SUCCESS) && (err < DEVICE_UPDATE_STA_MAX_ERR)) {
        //ble_updata_control(NULL, UPDATA_STOP, 0, 0, 0);
        //ble_set_jl_update_status(DEVICE_UPDATE_STA_SUCCESS);
        stop_state = ble_updata_handler(UPDATA_STOP, NULL, 0);
        JL_controller_resp_udpate_status_request(err);
        return stop_state;

    } else {
        log_info("-UPDATE STOP CODE ERR!\n");
        return 0;
    }
}


void update_pre_read_ctl_init(void)
{
    update_pre_read.pre_read_cnt = 0;
    update_pre_read.pre_read_flag = PRE_READ_FLAG_DISABLE;
    update_pre_read.last_addr = 0;
    update_pre_read.buf_busy_flag = 0;
    update_pre_read.buf0 = update_temp_buf;
    update_pre_read.buf1 = update_temp_buf + ONCE_UPDATE_READ_LEN;
}

u8 *get_update_pre_read_buf(u32 addr, u8 *buf,  u16 len)
{
    u8 *buf_ptr = NULL;

    if (update_pre_read.buf_busy_flag) {
        if ((update_pre_read.buf0_addr == addr) && (update_pre_read.buf_busy_flag & BIT(0))) {
            memcpy(buf, update_pre_read.buf0, len);
            update_pre_read.buf_busy_flag &= ~BIT(0);
            buf_ptr = update_pre_read.buf0;
        } else if ((update_pre_read.buf1_addr == addr) && (update_pre_read.buf_busy_flag & BIT(1))) {
            memcpy(buf, update_pre_read.buf1, len);
            update_pre_read.buf_busy_flag &= ~BIT(1);
            buf_ptr = update_pre_read.buf1;
        }
    }


    if (buf_ptr != NULL) {
        log_info("BUF_PTR:%x\n", buf_ptr);
    }
    return buf_ptr;
}

static u8 get_update_pre_read_flag(u32 addr, u16 len)
{
    //是否满足addr和len同时符合规律
    if ((addr == (update_pre_read.last_addr - ONCE_UPDATE_READ_LEN)) \
        && (len == ONCE_UPDATE_READ_LEN)) {
        if (PRE_READ_FLAG_DISABLE == update_pre_read.pre_read_flag) {
            update_pre_read.pre_read_cnt++;
            if (update_pre_read.pre_read_cnt >= CONTINUE_READ_TIMES) {
                update_pre_read.pre_read_flag = PRE_READ_FLAG_TOGGLE_TO_EN;
            }
        } else if (PRE_READ_FLAG_TOGGLE_TO_EN == update_pre_read.pre_read_flag) {
            update_pre_read.pre_read_flag = PRE_READ_FLAG_ENABLE;
        }

    } else {
        if (PRE_READ_FLAG_ENABLE == update_pre_read.pre_read_flag) {
            update_pre_read.pre_read_flag = PRE_READ_FLAG_TOGGLE_TO_DIS;
        } else if (PRE_READ_FLAG_TOGGLE_TO_DIS == update_pre_read.pre_read_flag || PRE_READ_FLAG_TOGGLE_TO_EN == update_pre_read.pre_read_flag) {
            update_pre_read.pre_read_flag = PRE_READ_FLAG_DISABLE;
        }
        //update_pre_read.pre_read_flag = 0;
        update_pre_read.pre_read_cnt = 0;
    }

    printf("pre_read_flag:%x last:%x\n", update_pre_read.pre_read_flag, update_pre_read.last_addr);
    update_pre_read.last_addr = addr;

    /* if (update_pre_read.pre_read_flag && (len != ONCE_UPDATE_READ_LEN)) {
        update_pre_read.pre_read_flag = 0;
        update_pre_read.pre_read_cnt = 0;
    } */
    return update_pre_read.pre_read_flag;
}

extern u32 g_bfu_burn_start_addr;  //bfu文件烧写bin起始地址
u32 ble_updata_read(void *handle, void *buf, u32 btr, u8 read_type, u8 seek_type, u32 seek_offsize)
{
    u32 read_len;
    if (UPDATA_STA_BFU_UPDATE_LOOP != u_ble_status) {
        log_info("ble sta err3\n");
        return 0;
    }
    u32 file_offset;

#if ONE_TIME_RECEIVE_ALL_DATA
    ble_control_info.flash_write_addr = seek_offsize - g_bfu_burn_start_addr;
    ble_control_info.once_write_len = FLASH_WRITE_ONCE_LEN;
    ble_control_info.once_rx_len = 0;
#endif


#if UPDATE_PRE_READ_EN
    u32 last_addr = update_pre_read.last_addr;
    u8 pre_read_flag = get_update_pre_read_flag(seek_offsize, btr);
    if (pre_read_flag == PRE_READ_FLAG_TOGGLE_TO_DIS) {
        printf("wait last buff\n");
        read_len = ble_updata_handler(UPDATA_READ, (u8 *)(last_addr - ONCE_UPDATE_READ_LEN), btr);
        printf("wait last buff11\n");
        update_pre_read.pre_read_flag = PRE_READ_FLAG_DISABLE;
    }
    file_offset = (PRE_READ_FLAG_ENABLE == pre_read_flag) ? seek_offsize - ONCE_UPDATE_READ_LEN : seek_offsize;
    ble_control_info.read_buf = buf;
    ble_control_info.btupdata_sw = UPDATA_READ;
    ble_control_info.ble_update_state &= ~LMP_STA_UPDATA_ACCEPT;
    ble_control_info.date_len = 0;
    ble_control_info.need_rx_len = btr;
    ble_updata_control(NULL, UPDATA_READ_OFFSIZE, btr, seek_type, file_offset);
#else
    ble_control_info.read_buf = buf;
    ble_control_info.btupdata_sw = UPDATA_READ;
    ble_control_info.ble_update_state &= ~LMP_STA_UPDATA_ACCEPT;
    ble_control_info.date_len = 0;
    ble_control_info.need_rx_len = btr;
    ble_updata_control(NULL, UPDATA_READ_OFFSIZE, btr, seek_type, seek_offsize);
#endif

#if UPDATE_PRE_READ_EN
    if (PRE_READ_FLAG_TOGGLE_TO_EN == pre_read_flag) {
        log_info("only send one time\n");
        update_pre_read.curr_file_offset = seek_offsize;
        ble_updata_control(NULL, UPDATA_READ_OFFSIZE, btr, seek_type, seek_offsize - ONCE_UPDATE_READ_LEN);

    }
#endif
    log_info("addr=0x%x,0x%x\n", seek_offsize, ble_control_info.need_rx_len);
    read_len = ble_updata_handler(UPDATA_READ, (u8 *)seek_offsize, btr);
    return read_len;
//return 0;
}

u32 ble_read_bt_updata(u8 _xdata *buff, u32 btr, u8 read_type, u8 type, u32 offsize)
{
    updata_write_flash_flag = 1;
    return ble_updata_read(NULL, buff, btr, read_type, type, offsize);
}

u32 ble_only_read_bt_updata(u8 _xdata *buff, u32 btr, u8 read_type, u8 type, u32 offsize)
{
    updata_write_flash_flag = 0;
    return ble_updata_read(NULL, buff, btr, read_type, type, offsize);
}
int ble_gatt_f_seek(void *fp, u8 type, u32 offsize)
{
    if (type == SEEK_SET) {
        ble_file_offset = offsize;
    } else if (type == SEEK_CUR) {
        ble_file_offset += offsize;
    }

    return 0;//FR_OK;
}

u16 ble_gatt_f_read(void *fp, u8 *buff, u16 len)
{
    return ble_updata_read(NULL, buff, len, 0, 0, ble_file_offset);
}

void update_result_handle(u8 err)
{
    u8 res =  DEVICE_UPDATE_STA_SUCCESS;
    switch (err) {
    case UPDATE_ERR_NONE:
        res = DEVICE_UPDATE_STA_SUCCESS;
        break;

    case UPDATE_ERR_KEY_ERR:
        res = DEVICE_UPDATE_STA_KEY_ERR;
        break;

    //升级文件头校验失败
    case UPDATE_ERR_UFW_HEAD_CRC_ERR:
    case UPDATE_ERR_LOADER_HEAD_CRC_ERR:
    case UPDATE_ERR_FLASH_HEAD_CRC_ERR:
    case UPDATE_ERR_REMOTE_RES_FILE_CRC_ERR:
        res = DEVICE_UPDATE_STA_FILE_ERR;
        break;

    //flash头文件校验失败
    case UPDATE_ERR_LOCAL_FILE_HEAD_CRC_ERR:
        res = DEVICE_UPDATE_STA_FILE_ERR;
        break;

    //找不到目标文件
    case UPDATE_ERR_NOT_FIND_LOADER_FILE:
    case UPDATE_ERR_NOT_FIND_TARGET_LOADER:
    case UPDATE_ERR_NOT_FIND_FLASH_BIN:
    case UPDATE_ERR_NOT_FIND_RESERVE_DIR_FILE:
    case UPDATE_ERR_NOT_FIND_TARGET_FILE:
        res = DEVICE_UPDATE_STA_FILE_ERR;
        break;

    //文件操作错误
    case UPDATE_ERR_MALLOC_ERR:
    case UPDATE_ERR_LOADER_WRITE_ERR:
    case UPDATE_ERR_FILE_HANDLE_ERR:
    case UPDATE_ERR_READ_REMOTE_FILE_ERR:
        res = DEVICE_UPDATE_STA_FILE_ERR;
        break;

    //文件内容校验失败
    case UPDATE_ERR_LOADER_VERIFY_ERR:
        res = DEVICE_UPDATE_STA_VERIFY_ERR;
        break;

    //flash数据校验失败
    case UPDATE_ERR_LOCAL_FILE_DATA_CRC_ERR:
    //efuse数据校验失败
    case UPDATE_ERR_LOCAL_EFUSE_CRC_ERR:
        res = DEVICE_UPDATE_STA_FILE_ERR;
        break;

    //uboot不匹配
    case UPDATE_ERR_UBOOT_NOT_MATCH:
        res = DEVICE_UPDATE_STA_TYPE_ERR;
        break;

    case UPDATE_ERR_PRODUCT_ID_NOT_MATCH:
        res = DEVICE_UPDATE_STA_FILE_ERR;
        break;
    default:
        res = DEVICE_UPDATE_STA_FAIL;
        break;
    }

    ble_updata_stop(res);
}

extern u8 ufw_head_check(u32 *update_part_flag);
extern u8 flash_bin_head_check(void);
extern u8 update_type_check(u32 *update_len);
extern u8 flash_update_process(void);
extern u8 flash_file_data_verify(void);
extern u8 flash_update_reserve_area(void);
extern u8 flash_all_data_verify(void);
extern u8 flash_parts_update_check(u32 update_part_flag);
extern void clr_update_loader_record(void);
extern bool check_flash_is_ze(void);
extern u8 g_read_buf[];
UPDATA_RESULT ble_updata_loop(u8 updata_type)
{
    void *conn;
    u32 res;
    UPDATA_RESULT ret = DEV_UPDATA_SUCC;
    u32 update_part_flag;

    log_info("ble_updata_loop 1\n");

    ble_updata_bfu_open(conn, updata_type, g_read_buf, 512);

    log_info("bt_updata_bfu_open success\n");
    ble_control_info.ble_update_state |= LMP_STA_UPDATA_ACTIVE_SCUSS;

    deg_puts("ufw_check\n");
    res = ufw_head_check(&update_part_flag);
    if (UPDATE_ERR_NONE != res) {
        deg_puts("---ufw_head_check err\n");
        goto _LOOP_RET;
    } else {
        deg_puts("---ufw_head_check ok\n");
        //deg_printf("FS_COMMON_OK updata_type = 0x%x\n", updata_type_res);
    }
    deg_printf("update_part_flag:%x\n", update_part_flag);

    if (UPDATE_PART_ALL_FLASH == update_part_flag) {
        res = flash_bin_head_check();
        if (UPDATE_ERR_NONE != res) {
            deg_puts("---flash_bin_head_chek err\n");
            goto _LOOP_RET;
        } else {
            deg_puts("---flash_bin_head_chek ok\n");
        }
    } else {
        res = flash_parts_update_check(update_part_flag);
        if (UPDATE_ERR_NONE != res) {
            deg_puts("---flash_part update err\n");
        } else {
            deg_puts("---flash_part update succ\n");
            //return DEV_UPDATA_SUCC;
        }
        goto _LOOP_RET;
    }

    u32 update_len;
    //检查ufw文件的各个文件,将要升级的区域记录在链表中;
    res = update_type_check(&update_len);

#if ALL_FLASH_INCLUDE_UBOOT_UPDATE_EN
    if ((UPDATE_ERR_NONE != res) && (UPDATE_ERR_UBOOT_NOT_MATCH != res)) {
#else
    if (UPDATE_ERR_NONE != res) {
#endif
        deg_printf("--update_type_check_err %x\n", res);
        //update_result_handle(res);
        goto _LOOP_RET;
    } else {
        deg_puts("---update_type_check ok\n");
    }

    //获取是否整个flash升级标志
    u8 all_flash_update_flag = (update_len) ? 1 : 0;
    if (check_flash_is_ze()) {
        all_flash_update_flag = 1;
    }

    //通知测试盒升级内容总长度
    deg_printf("\n\n---update_len:%x\n\n", update_len);
    if (ble_notify_update_content_size(NULL, update_len)) {
        res = UPDATE_ERR_CONN_ERR;
        goto _LOOP_RET;
    }

    //开始擦除响应区域并升级代码(todo:代码校验)
    res = flash_update_process();
    if (UPDATE_ERR_NONE == res) {
        puts("---flash update succ\n");
    } else {
        printf("---flash update fail:%x\n", res);
        goto _LOOP_RET;
    }

    //ble_updata_rsp_complete();
    if (all_flash_update_flag) {
        //flash全校验
        res = flash_all_data_verify();
        if (UPDATE_ERR_NONE == res) {
            puts("----flash all data verify succ\n");
        } else {
            puts("----flash all data verify fail\n");
            goto _LOOP_RET1;
        }
    } else {
        //升级区域校验
        res = flash_all_data_verify();
        //res = flash_file_data_verify();
        if (UPDATE_ERR_NONE == res) {
            puts("----flash verify succ\n");
        } else {
            printf("---flash verify fail:%x\n", res);
            goto _LOOP_RET1;
        }
    }

    clr_update_loader_record();
    res = flash_update_reserve_area();
    if (UPDATE_ERR_NONE == res) {
        puts("---update_reserve_area ok\n");
        //bt_f_stop(BT_UPDATE_OVER);
        //return DEV_UPDATA_SUCC;
    } else {
        printf("---update_reserve_area fail:%x\n", res);
    }

    goto _LOOP_RET1;
_LOOP_RET:
    puts("UPDATA_LOOP_RET\n");
_LOOP_RET1:
    //bt_f_stop(res | UPDATE_RESULT_FLAG_BITMAP);
    if (UPDATE_ERR_NONE == res) {
        ret = DEV_UPDATA_SUCC;
    } else if ((UPDATE_ERR_KEY_ERR == res) || \
               (UPDATE_ERR_UBOOT_NOT_MATCH == res) || \
               (UPDATE_ERR_PRODUCT_ID_NOT_MATCH == res)) {
        clr_update_loader_record();
        ret = DEV_UPDATA_KEY_ERR;
    } else {
        ret = DEV_UPDATA_FAIL;
    }
    updata_status_to_user(UPDATE_STOP, res);
    ble_updata_rsp_complete();
    update_result_handle(res);
    return ret;
}

static void att_send_schedule(void)
{
#if 0
    static volatile u8 sending = 0;
    if (sending) {
        //防重入
        return;
    }

    sending = 1;
}

schedule_end:
sending = 0;
#endif
}



extern void update_wait_connect_timeout_stop(void);
extern void update_wait_connect_timeout_start(void);
extern void update_wait_no_update_timeout_start(void);

static u8 reboot_status = REBOOT_STA_NOT_ALLOW;
u8 jl_update_get_reboot_sta(void)
{
    printf("get reboot sta\n");
    if (ble_control_info.btupdata_sw == UPDATA_WAIT_REBOOT) {
        ble_control_info.btupdata_sw = UPDATA_ACCEPT_OK;
        ble_control_info.ble_update_state |= LMP_STA_UPDATA_ACCEPT;
    } else {
        log_info("#BLE STATUS ERR\n");
    }
    //task_post_msg(NULL, 1, MSG_JL_UPDATE_REVICE_REBOOT);
    return reboot_status;
}

void jl_update_set_reboot_status(u8 status)
{
    reboot_status = status;
}

static void ble_update_wait_reboot_cmd(void)
{
    log_info("-wait reboot cmd\n");
    ble_control_info.btupdata_sw = UPDATA_WAIT_REBOOT;
    ble_control_info.ble_update_state &= ~LMP_STA_UPDATA_ACCEPT;
    ble_control_info.wait_timeout_instant = tmr_2ms_cnt_get() + 2000 / 2;        //等待2S超时

    ble_updata_handler(UPDATA_WAIT_REBOOT, NULL, 0);
}

static void ble_update_wait_disconnect(void)
{
    if (__this && __this->get_number_of_pkt_sending) {
        while (__this->get_number_of_pkt_sending(NULL)) {
            log_info("w");
            wdt_clear();
            if (__this->task_run) {
                __this->task_run();
            }
        }
    }

    if (__this->dev_disconnect) {
        __this->dev_disconnect(NULL);
        //ble_updata_handler(0xff, NULL, 0);
    } else {
        ASSERT(__this->dev_disconnect);
    }
}

u8 ble_updata_process(void)
{
    u8 result;

    if (UPDATA_STA_BFU_UPDATE_LOOP == u_ble_status) {
        //update_wait_connect_timeout_stop();
        u_led_status = UPDATA_STA_LED_SIGN;
        jl_update_set_reboot_status(REBOOT_STA_NOT_ALLOW);
        result = ble_updata_loop(0);
        if (result == DEV_UPDATA_SUCC) {
            //log_info("reboot_ready\n");
            jl_update_set_reboot_status(REBOOT_STA_READY);
            //return result;
        } else {
            printf("set u_ble_status SLEEP\n");
            u_led_status = UPDATA_STA_LED_FILCKER;
            u_ble_status = UPDATA_STA_DEV_SLEEP;
        }

        switch (u_ble_status) {
        case UPDATA_STA_BFU_UPDATE_LOOP:
        case UPDATA_STA_DISCONNECTION_COMPLETED:
            if (result == DEV_UPDATA_SUCC) {
                u_ble_status = UPDATA_STA_WAIT_FOR_REBOOT;
                ble_update_wait_reboot_cmd();
            }

            ble_update_wait_disconnect();
            udelay(200 * 1000);     //200ms
            u_ble_status = UPDATA_STA_DISCONNECTION_COMPLETED;
            if ((result == DEV_UPDATA_KEY_ERR) || (result == DEV_UPDATA_SUCC)) {
                return result;
            }
            putchar('f');
            u_led_status = UPDATA_STA_LED_FILCKER;
            break;
        }
        //u_led_status = UPDATA_STA_LED_OPEN;
        //log_info("wait reboot\n");
    }

    return 0;
}

#if 0
#define UBOOT_SW_TIMEOUT_CNT    (120L)
static u8 judge_uboot_sw_timeout(void)
{
    u32 *uboot_sw_timeout = UBOOT_SW_TIMEOUT_ADDR;
    (*uboot_sw_timeout)++;
    printf("sw_cnt:%d\n", (*uboot_sw_timeout));
    if (*uboot_sw_timeout >= UBOOT_SW_TIMEOUT_CNT) {
        task_post_msg(NULL, 1, MSG_UBOOT_SOFT_POWEROFF);
        return 1;
    }
    return 0;
}

static void set_next_time_other_uboot_start_flag(void)
{
    u8 *p = (u8 *)UBOOT_SEL_FLAG;
#if 1
    *p = UBOOT_FOR_SPP_UPDATE_FLAG;
#else
    *p = UBOOT_FOR_BLE_UPDATE_FLAG;
#endif
}

static void uboot_running_timeout_handle(struct sys_timer *tmr)
{
    log_info("RUN TIME OUT\n");
    set_updata_result(UPDATA_NON);
    set_next_time_other_uboot_start_flag();
    //PORTA_OUT &= ~BIT(10);
    int tmp = 0;

    if (judge_uboot_sw_timeout()) {
        return;
    }

    //todo
    if (__this->dev_close) {
        __this->dev_close();
    }

    /* __asm__ volatile("mov ie0,%0" :: "r"(tmp));
    __asm__ volatile("mov ie1,%0" :: "r"(tmp)); */

#if 0
    __asm__ volatile("mov %0,sp" : "=r"(tmp));
    printf("SP = %08x \r\n", tmp);
    __asm__ volatile("mov %0,usp" : "=r"(tmp));
    printf("USP = %08x \r\n", tmp);
    __asm__ volatile("mov %0,ssp" : "=r"(tmp));
    printf("SSP = %08x \r\n", tmp);
#endif
    //__asm__ volatile("mov sp,0x400");
    //__asm__ volatile("mov usp,0x800");
    u32 mini_uboot_run_addr = 0x2000;


#if 0
    __asm__ volatile("movh reti,0");
    __asm__ volatile("movl reti,0x2008");
    __asm__ volatile("rti");
    //((void (*)(void))mini_uboot_run_addr)();
#else
    //PWR_CON |= BIT(4);
    chip_reset();
#endif
}

extern u8 get_bt_updatae_power_on_flag();
void uboot_running_timeout_start(void)
{
    if (get_bt_updatae_power_on_flag()) {
        log_info("UBOOT_RUN_TIMEOUT_START\n");
        sys_timer_register(&timeout_for_uboot_run, UBOOT_RUNNING_TIMEOUT_MS, uboot_running_timeout_handle);
        //PORTA_DIR &= ~BIT(10);
        //PORTA_OUT |= BIT(10);
    }
}

void uboot_running_timeout_finish(void)
{
    if (get_bt_updatae_power_on_flag()) {
        log_info("UBOOT_RUN_TIMEOUT_FINISH\n");
        sys_timer_remove(&timeout_for_uboot_run);
    }
}

#endif
typedef u32(*bt_read_handle_t)(void *buff, u32 btr, u8 read_type, u8 type, u32 offsize);
typedef u32(*bt_only_read_handle_t)(void *buff, u32 btr, u8 read_type, u8 type, u32 offsize);

extern void bt_update_read_handle_register(bt_read_handle_t read_hdl, bt_only_read_handle_t only_read_hdl);
extern void adc_init(void);

static void dev_conn_state_change_handle(void *priv, u8 state)
{
    log_info("dev_conn_state:%x\n", state);

    switch (state) {
    case DEV_CONN_STATE_READY_UPDATE:
        u_ble_status = UPDATA_STA_READY_UPDATE;
        //update_wait_no_update_timeout_start();
        break;
    case DEV_CONN_STATE_DISCONNECTED:
        u_ble_status = UPDATA_STA_DISCONNECTION_COMPLETED;
        u_led_status = UPDATA_STA_LED_FILCKER;
        //update_wait_connect_timeout_start();
        break;
    case DEV_CONN_STATE_CONNECTED:
        u_ble_status = UPDATA_STA_DEV_CONNECTION;
        //update_wait_no_update_timeout_start();
        //update_wait_connect_timeout_stop();
        break;
    case DEV_CONN_STATE_START_UPDATE:
        log_info("--UPDATE_START\n");
        u_ble_status = UPDATA_STA_BFU_UPDATE_LOOP;
        break;
    default:
        break;
    }
}

extern void dev_get_update_opearation_table(update_op_api_t **interface_ptr);
UPDATA_RESULT ble_updata_run(void)
{
    UPDATA_RESULT res;

#if LOW_POWER_DET_EN
    adc_init();
#endif

#ifndef GET_GPIO_INFO_FLAG
    //strcpy(io_str, "pb03");
#else
    /* char power_io[7] = {0};
    if (EX_CFG_ERR_NONE != ex_cfg_get_content_api(CFG_ITEM_POWER_IO_OFF, power_io, 7)) {
        memset(power_io, 0, 7);
    } else {
        gpio_init(power_io);
    }

    if (EX_CFG_ERR_NONE != ex_cfg_get_content_api(CFG_ITEM_PILOT_LAMP_IO_INFO, io_str, 5)) {
        memset(io_str, 0, 5);
    } else {
        gpio_init(io_str);
    } */
#endif

    //sys_timer_register(&timeout_led_scan, LED_SCAN_FREQ, app_led_scan);
    dev_get_update_opearation_table(&__this);

    ASSERT(__this);

    //__this->btstack_init();
    __this->register_update_data_handle(dev_read_update_block_handle);
    __this->register_update_status_req_handle(dev_update_status_req_handle);
    __this->register_conn_state_change_handle(dev_conn_state_change_handle);

    //bt_update_read_handle_register(ble_read_bt_updata, ble_only_read_bt_updata);
#if (BLE_GATT_UPDATA_MODULE_CONTROL || USB_HID_MODULE_CONTROL)
    rcsp_init(NULL, NULL);
#endif
    //update_wait_connect_timeout_start();

    while (1) {
        wdt_clear();
        res = ble_updata_process();
        if (res) {
            //log_info("ll shutdown\n");
            /* if (__this->dev_close) {
                __this->dev_close();
            } */
            //log_info("ll shutdown exit\n");
            break;
        }
#if USB_HID_MODULE_CONTROL
        // hid发空包
        extern void usb_delay(void);
        usb_delay();
#endif
    }

    puts("ret update succ\n");
    return res;
}


