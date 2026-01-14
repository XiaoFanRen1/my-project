#include "system/includes.h"
#include "os/os_api.h"
#include "app_config.h"
#include "asm/isc.h"
#include "video/video_ioctl.h"
#include "device/device.h"
#include "video/video.h"

#ifdef CONFIG_VIDEO_ENABLE

#define YUV_TEST 	0 //可以打开测试YUV到SD卡
#define YUV_DATA_SOURCE "video0.1" //video0设置

static u8 yuv_buf[CONFIG_VIDEO_IMAGE_W * CONFIG_VIDEO_IMAGE_H * 3 / 2];

typedef struct {
    int pid;
    void (*cb)(u8 *data, u32 len, int width, int height) ;
    char init:1;
    char open:1;
} get_yuv_cfg;

static get_yuv_cfg  __info;

static void get_yuv_task(void *priv)
{
    int err;
    struct video_format f = {0};
    struct yuv_image yuv = {0};
    void *dev;
    void (*cb)(u8 * data, u32 len, int width, int height);

    /*
     *step 1
     */
    dev = dev_open(YUV_DATA_SOURCE, NULL);
    if (!dev) {
        printf("open video err !!!!\n\n");
        return;
    }

#if 1
    /********************YUV内部加外部双BUF,上层处理时间可长一些************************/
    /*
     *step 2
     */
    f.type = VIDEO_BUF_TYPE_VIDEO_OVERLAY;
    f.pixelformat = VIDEO_PIX_FMT_YUV420;
    err = dev_ioctl(dev, VIDIOC_SET_FMT, (u32)&f);
    if (err) {
        goto error;
    }

    /*
     *step 3
     */
    //BUFF配置1：使用外部BUFF，上层处理时间最大为摄像头的帧率时间 - 15ms，GC0308默认驱动处理最大时间35m
    yuv.addr = (u8 *)yuv_buf;
    yuv.size = sizeof(yuv_buf);
    err = dev_ioctl(dev, VIDIOC_SET_OVERLAY, (u32)&yuv);
    if (err) {
        goto error;
    }

#else
    /********************YUV支持外部静态BUF,上层处理时间较短************************/
    /*
     *step 2
     */
    f.type = VIDEO_BUF_TYPE_VIDEO_OVERLAY;
    f.pixelformat = VIDEO_PIX_FMT_YUV420;
    f.static_buf = (u8 *)yuv_buf;
    f.sbuf_size = sizeof(yuv_buf);

    err = dev_ioctl(dev, VIDIOC_SET_FMT, (u32)&f);
    if (err) {
        goto error;
    }

    //BUFF配置2：使用内部BUFF，内存节省450K(VGA),上层处理时间最大为摄像头的帧结束到帧起始时间，需要看摄像头帧同步信号波形测量，GC0308默认驱动处理最大时间16ms
    err = dev_ioctl(dev, VIDIOC_SET_OVERLAY, (u32)0);
    if (err) {
        goto error;
    }
#endif
    __info.open = 1;

#if YUV_TEST
    FILE *fd = fopen(CONFIG_ROOT_PATH"yuv_1.yuv", "wb+");
    int cnt = 0;
#endif

    while (1) {
        if (thread_kill_req()) {
            break;
        }

        cb = __info.cb;

        /*
         *step 4
         */
        err = dev_ioctl(dev, VIDIOC_GET_OVERLAY, (u32)&yuv);
        if (!err) {
            //do something in this step
#if YUV_TEST
            if (fd) {
                cnt++;
                fwrite(yuv.addr, yuv.size, 1, fd);
                if (cnt > 100) {
                    fclose(fd);
                    fd = NULL;
                }
                printf("write file ok : %d \n", cnt);
            }
#else
            if (cb) {
                if (f.static_buf) {//使用单帧静态缓冲区时，关闭接收YUV的ISC硬件防止数据覆盖
                    dev_ioctl(dev, VIDIOC_SET_HDWARE_STATE, 0);
                }

                cb(yuv.addr, yuv.size, CONFIG_VIDEO_IMAGE_W, CONFIG_VIDEO_IMAGE_H);

                if (f.static_buf) {//开启接收YUV的ISC硬件
                    dev_ioctl(dev, VIDIOC_SET_HDWARE_STATE, 1);
                }
            }
#endif
        }
#if YUV_TEST
        else if (fd) {
            fclose(fd);
            fd = NULL;
        }
#endif

        /*
         *step 5
         */
        dev_ioctl(dev, VIDIOC_CLEAR_OVERLAY, 0);
    }
error:
    /*
     *step 6
     */
    dev_close(dev);
    __info.open = 0;
    __info.init = 0;
}

void get_yuv_init(void (*cb)(u8 *data, u32 len, int width, int height))
{
    if (__info.init) {
        return;
    }
    __info.init = 1;
    __info.cb = cb;
    thread_fork("GET_YUV_TASK", 4, 1024, 0, &__info.pid, get_yuv_task, NULL);
}
void get_yuv_uninit(void)
{
    if (!__info.init) {
        return;
    }
    __info.cb = NULL;
    thread_kill(&__info.pid, KILL_WAIT);
    __info.init = 0;
}
void get_yuv_open_waite(void)
{
    if (__info.init) {
        while(!__info.open){
            os_time_dly(1);
        }
    }
}
void set_get_yuv_data_cb(void (*cb)(u8 *data, u32 len, int width, int height))
{
    __info.cb = cb;
}

void *get_static_yuv_buf(void)
{
    return yuv_buf;
}

#endif

