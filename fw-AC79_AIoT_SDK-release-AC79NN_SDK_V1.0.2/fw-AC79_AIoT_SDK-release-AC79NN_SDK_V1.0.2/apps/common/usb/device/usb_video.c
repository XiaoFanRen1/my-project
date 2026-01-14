/**
 *
 * @file usb_video.c
 * @brief usb host video driver for jpeg stream
 * @author chenrixin@zh-jieli.com
 * @version 1.0
 * @date 2016-11-25
 */
#include <string.h>
#include "usb_ctrl_transfer.h"
#include "usb_host.h"
#include "usb_ctrl_transfer.h"
#include "host_uvc.h"
#include "usb_stack.h"
#include "asm/device_drive.h"
#include "app_config.h"
#include "usb/host/usb_host.h"

#if TCFG_HOST_UVC_ENABLE

/*static	USB_DEV uvc_ep3;*/
/*static	USB_DEV uvc_ep4;*/
#define     USB_DT_INTERFACE_HEADER_DESCRIPTOR_SIZE	0x0d
#define     STOP_TRANSFER   1

#define drv_info printf
#define drv_dbg printf
#define drv_err printf
#define drv_printf_buf //put_buf

#define time_get(t,ms)	t = jiffies + msecs_to_jiffies(ms);
#define time_out(a,b)  (((long)(a) - (long)(b)) > 0)

#define  check_ret(ret)          if((int)(ret) < DEV_ERR_NONE)\
    {\
        drv_err("err ret = %d\n",ret);\
        return ret;\
    }

struct usb_video_manager {
    u8 host_epout;
    u8 target_epout;
    u16 txmaxp;
    u8 host_epin;
    u8 target_epin;
    u16 rxmaxp;
    u8 online;
    u8 open;
    u8 interface_init;
    u8 in_irq;
    u8 req_close;
    u8 *name;
    u8 *buffer;
    u32 frame_cnt;
    struct usb_uvc *uvc;
    OS_SEM sem;
    struct device dev;
};
static struct usb_video_manager uvc_manager[2];
#define __this(id) (&uvc_manager[id])

void usb_uvc_release(u8 usb_id);
void *usb_alloc_ep_dmabuffer(const usb_dev usb_id, u32 ep, u32 dma_size);
int uvc_host_online(void)
{
    int id = 0;
    for (id = 0; id < 2; id++) {
        if (__this(id)->online) {
            return id;
        }
    }
    return -EINVAL;
}
u32 uvc_host_get_frame(u8 id)
{
    return __this(id)->frame_cnt;
}
static int uvc_set_power(struct usb_host_device *host_dev, u32 value)
{
    u8 id = usbpriv_to_usbid(usbdev_to_usbpriv(host_dev));
    if (__this(id)->online && !__this(id)->open && ((value == -1) || !value)) {
        usb_uvc_release(id);
        __this(id)->online = 0;
    } else {
        __this(id)->online = value == 1 ? 1 : 0;
    }
    return 0;
}
static int uvc_get_power(struct usb_host_device *host_dev, u32 value)
{
    u8 id = usbpriv_to_usbid(usbdev_to_usbpriv(host_dev));
    return __this(id)->online;
}
static int uvc_ioctl(struct usb_host_device *host_dev, u32 cmd, u32 arg)
{
    return 0;
}
static const struct interface_ctrl uvc_host_ops = {
    .interface_class = USB_CLASS_VIDEO,
    .set_power = uvc_set_power,
    .get_power = uvc_get_power,
    .ioctl = uvc_ioctl,
};
static const struct usb_interface_info uvc_host_inf = {
    .ctrl = (struct interface_ctrl *) &uvc_host_ops,
    .dev.uvc = NULL,
};
void usb_host_video_init(void)
{
    __this(0)->host_epin = UVC_STREAM_EP_IN;
    __this(0)->rxmaxp = UVC_HOST_FIFO_RXMAXP;
    __this(0)->target_epout = 0;
    __this(1)->host_epin = UVC_STREAM_EP_IN;
    __this(1)->rxmaxp = UVC_HOST_FIFO_RXMAXP;
    __this(1)->target_epout = 0;
}
static struct usb_uvc *device_to_uvc(struct device *d)
{
    struct usb_host_device *host_dev = device_to_usbdev(d);
    if (!host_dev) {
        return NULL;
    }
    struct usb_private_data *private = usbdev_to_usbpriv(host_dev);
    /*u8 id = usbpriv_to_usbid(usbdev_to_usbpriv(device_to_usbdev(d)));*/
    u8 id = usbpriv_to_usbid(private);
    ASSERT(id < 2, "err in usbpriv_to_usbid ");
    return __this(id)->uvc;
}
void device_to_uvc_clear(struct device *d)
{
    struct usb_host_device *host_dev = device_to_usbdev(d);
    u8 id = usbpriv_to_usbid(usbdev_to_usbpriv(device_to_usbdev(d)));
    __this(id)->uvc = NULL;;
}

static struct device *uvc_host_device_find(char *name)
{
    int id = name[3] - '0';
    if (id >= 0 && id <= 1 && name && __this(id)->name) {
        if (!strcmp((const char *)__this(id)->name, name)) {
            return &__this(id)->dev;
        }
    }
    return NULL;
}
static int uvc_dummy_stream_out(void *fd, int cnt, void *stream_list, int eof)
{
    return 0;
}
int usb_uvc_parser(struct usb_host_device *host_dev, u8 *pBuf, u8 num_ep)
{
    int len = 0;
    int cur_len;
    int cur_type;
    int cur_sub_type;
    int loop;
    int bControlSize ;
    int formart_num ;
    u32 frame_num;
    u16 wTotalLength;
    u32 wMaxPacketSize;
    u32 AlternateSetting;
    struct usb_endpoint_descriptor end_desc;
    struct usb_uvc *real_uvc = NULL;
    struct usb_private_data *private_data = &host_dev->private_data;
    u8 usb_id = private_data->usb_id;
    u8 int_ep = 0;

    pBuf += sizeof(struct usb_interface_descriptor);
    struct usb_uvc *uvc = (struct usb_uvc *)zalloc(sizeof(struct usb_uvc));
    if (!uvc) {
        drv_err("err no mem in usb_uvc_parser\n\n");
        goto __err;
    }
    len += USB_DT_INTERFACE_HEADER_DESCRIPTOR_SIZE;
    __this(usb_id)->in_irq = true;
    uvc->bfh = 0;
    uvc->cur_frame = 1;
    uvc->stream_out = uvc_dummy_stream_out;
    loop = 1;
    wMaxPacketSize = 0;
    AlternateSetting = 0;
    u8 *pformat1 = NULL;

    while (loop) {
        cur_len = *(pBuf + len);
        cur_type = *(pBuf + len + 1);
        cur_sub_type = *(pBuf + len + 2);

        if (!cur_len) {
            break;
        }

        if (cur_type == USB_DT_CS_INTERFACE) {
            switch (cur_sub_type) {
            case UVC_VC_INPUT_TERMINAL:
                bControlSize = *(pBuf + len + 14);

                drv_dbg("VC_INPUT_TERMINAL:%d %d %d", len, *(pBuf + len), bControlSize);
                /*drv_printf_buf(pBuf + len, cur_len);*/
                break;

            case UVC_VC_PROCESSING_UNIT:
                bControlSize = *(pBuf + len + 7);
                drv_dbg("VC_PROCESSING_UNIT:%d %d %d", len, *(pBuf + len), bControlSize);
                /*drv_printf_buf(pBuf + len, cur_len);*/
                break;

            case UVC_VC_OUTPUT_TERMINAL:
                bControlSize = *(pBuf + len + 8);
                drv_dbg("VC_OUTPUT_TERMINAL:%d %d %d", len, *(pBuf + len), bControlSize);
                /*drv_printf_buf(pBuf + len, cur_len);*/
                break;

            case UVC_VC_SELECTOR_UNIT:
                break;
            case UVC_VC_EXTENSION_UNIT:
                bControlSize = *(pBuf + len + 20);
                drv_dbg("VC_EXTENSION_UNIT:%d %d %d %d",
                        len,
                        *(pBuf + len),
                        bControlSize,
                        *(pBuf + len + 22 + bControlSize));
                /*drv_printf_buf(pBuf + len, cur_len);*/
                break;

            case UVC_VS_INPUT_HEADER:
                formart_num = *(pBuf + len + 3);
                bControlSize = *(pBuf + len + 12);
                memcpy(&wTotalLength, pBuf + len + 4, 2);

                cur_len =  wTotalLength;
                //format descriptor
                u8 *pformat = pBuf + len + *(pBuf + len);
                pformat1 = pformat;
                drv_dbg("VS_INPUT_HEADER:%d %d %d %d %d %d",
                        wTotalLength,
                        formart_num,
                        bControlSize,
                        pformat[0],
                        pformat[1],
                        pformat[2]);
                /*drv_printf_buf(pformat, cur_len);*/
                if (wTotalLength == 0x5b) {
                    printf("%s@%d()\n", __func__, __LINE__);
                    cur_len = 0x55;
                }
                wTotalLength -= *(pBuf + len);
                while (wTotalLength) {
                    if (pformat[0] == 0) {
                        break;
                    }

                    if (pformat[1] == USB_DT_CS_INTERFACE &&
                        pformat[2] == 0x06) {  //VS_FORMAT_MJPEG
                        frame_num = pformat[4];
                        uvc->mjpeg_format_index = pformat[3];
                        wTotalLength -= pformat[0];
                        drv_dbg("find mjpeg frame descriptor %d %d %d",
                                frame_num,
                                pformat[3],
                                wTotalLength);
                        uvc->cur_frame = pformat[6];
                        drv_dbg("default frame id %d", uvc->cur_frame);
                        pformat += pformat[0];
                        uvc->reso_num = frame_num;
                        real_uvc = (struct usb_uvc *)zalloc(sizeof(struct usb_uvc) + sizeof(struct uvc_frame_info) * (1 + frame_num));
                        if (!real_uvc) {
                            drv_err("err no mem in usb_uvc_parser\n\n");
                            goto __err;
                        }
                        for (int i = 0; i < frame_num; i++) {
                            memcpy(&real_uvc->pframe[i].width, &pformat[5], 2);
                            memcpy(&real_uvc->pframe[i].height, &pformat[7], 2);
                            drv_dbg("frame info %d %d*%d", 1 + i, real_uvc->pframe[i].width,
                                    real_uvc->pframe[i].height);
                            u32 f_interval = 0;
                            u32 f_interval_step = 0;
                            memcpy(&f_interval, pformat + 21, 4);
                            drv_dbg("default frame interval %d, fps %d", f_interval, 10000000 / f_interval);
                            if (pformat[25] == 0) {
                                memcpy(&f_interval, pformat + 26, 4);
                                drv_dbg("min frame interval %d, fps %d", f_interval, 10000000 / f_interval);
                                memcpy(&f_interval, pformat + 30, 4);
                                drv_dbg("max frame interval %d, fps %d", f_interval, 10000000 / f_interval);
                                memcpy(&f_interval_step, pformat + 34, 4);
                                drv_dbg("frame interval step %d, fps step %d", f_interval_step,
                                        10000000 / (f_interval_step ? f_interval_step : f_interval));
                            } else {
                                for (int icnt = 0; icnt < pformat[25]; icnt++) {
                                    memcpy(&f_interval, pformat + 26 + 4 * icnt, 4);
                                    drv_dbg("frame interval %d %d, fps %d", icnt, f_interval, 10000000 / f_interval);
                                }
                            }
                            uvc->fps = 10000000 / (f_interval_step ? f_interval_step : f_interval);
                            wTotalLength -= pformat[0];
                            pformat += pformat[0];
                        }
                        /* drv_printf_buf(pformat, 50); */
                    } else if (pformat[1] == USB_DT_CS_INTERFACE &&
                               pformat[2] == 0x04) { //vs_format_uncompressed
                        frame_num = pformat[4];
                        wTotalLength -= pformat[0];
                        drv_dbg("find yuy2 frame descriptor %d %d %d",
                                frame_num,
                                pformat[3],
                                wTotalLength);
                        pformat += pformat[0];
                        for (int i = 0; i < frame_num; i++) {
                            u16 width, heigth;
                            memcpy(&width, &pformat[5], 2);
                            memcpy(&heigth, &pformat[7], 2);
                            drv_dbg("frame info %d %d*%d", 1 + i, width,
                                    heigth);
                            wTotalLength -= pformat[0];
                            pformat += pformat[0];
                        }
                    } else if (pformat[1] == USB_DT_CS_INTERFACE &&
                               pformat[2] == 0x03) { //VS_STILL_IMAGE_FRAME
                        wTotalLength -= pformat[0];
                        drv_dbg("find still image frame descriptor %d %d", frame_num, wTotalLength);
                        pformat += pformat[0];
                    } else if (pformat[1] == USB_DT_CS_INTERFACE &&
                               pformat[2] == 0x0d) { //VS_COLORFORMAT
                        wTotalLength -= pformat[0];
                        drv_dbg("find colorformat descriptor %d %d", frame_num, wTotalLength);
                        pformat += pformat[0];
                    } else if (pformat[1] == USB_DT_INTERFACE &&
                               pformat[0] == USB_DT_INTERFACE_SIZE) {
                        cur_len = (u32)pformat - (u32)pformat1 + 0x0e;
                        printf("cur_len = %d\n", cur_len);
                        break;
                    } else {
                        break;
                    }
                    /*drv_printf_buf(pformat, 16);*/
                }
                break;

            default :
                loop = 0;
                break;
            }

        } else if (cur_type == USB_DT_CS_ENDPOINT) {
            drv_dbg("CS_ENDPOINT");
            /*drv_printf_buf(pBuf + len, cur_len);*/

            if (cur_len != 5) {
                loop = 0;
            }
        } else if (cur_type == USB_DT_INTERFACE) {
            drv_dbg("USB_DT_INTERFACE");
            /*drv_printf_buf(pBuf + len, cur_len);*/
            AlternateSetting = *(pBuf + len + 3);
            drv_dbg("AlternateSetting:%d", AlternateSetting);

            if (cur_len != USB_DT_INTERFACE_SIZE) {
                loop = 0;
            }
        } else if (cur_type == USB_DT_ENDPOINT) {
            drv_dbg("USB_DT_ENDPOINT_SIZE");
            /*drv_printf_buf(pBuf + len, cur_len);*/
            memcpy(&end_desc, pBuf + len, USB_DT_ENDPOINT_SIZE);

            if (end_desc.bEndpointAddress & USB_DIR_IN) {
                end_desc.bEndpointAddress &= ~USB_DIR_IN;
                drv_dbg("Endpoint IN %d", end_desc.bEndpointAddress);
            } else {
                drv_dbg("Endpoint OUT %d", end_desc.bEndpointAddress);
            }

            drv_dbg("wMaxPacketSize  = %d, %d additional transaction", end_desc.wMaxPacketSize & 0x7ff, end_desc.wMaxPacketSize >> 11);


            if ((end_desc.bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_BULK) {
                drv_dbg(">>>>>XFER_BULK\n");
            } else if ((end_desc.bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_INT) {
                drv_dbg(">>>>>XFER_INT = %d\n", end_desc.bInterval);
                int_ep = 1;
            } else if ((end_desc.bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_ISOC) {
                drv_dbg(">>>>>XFER_ISOC = %d\n", end_desc.bInterval);
            } else if ((end_desc.bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_CONTROL) {
                drv_dbg(">>>>>XFER_CONTROL\n");
            }

            if ((end_desc.bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_INT) {
                uvc->int_ep.in_ep = end_desc.bEndpointAddress & 0x0f;
                uvc->int_ep.interval = end_desc.bInterval;
                uvc->int_ep.wMaxPacketSize = end_desc.wMaxPacketSize;
                uvc->int_ep.enable = 1;
            } else if ((end_desc.bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_ISOC) {
                if (end_desc.wMaxPacketSize > wMaxPacketSize) {
                    if ((end_desc.wMaxPacketSize * ((end_desc.wMaxPacketSize >> 11) & 0xff + 1)) > __this(usb_id)->rxmaxp) {
                        drv_dbg("err no surport this usb dev, bInterval=%d, MaxPacketSize=%d, rxmaxp=%d \n\n",
                                end_desc.bInterval,
                                end_desc.wMaxPacketSize,
                                __this(usb_id)->rxmaxp);
                        goto __err;
                    }
                    wMaxPacketSize = end_desc.wMaxPacketSize;
                    uvc->ep = end_desc.bEndpointAddress & 0x0f;
                    uvc->wMaxPacketSize = end_desc.wMaxPacketSize;
                    uvc->interval = end_desc.bInterval;
                    uvc->if_alt_num = AlternateSetting;
                    uvc->xfer_type = USB_ENDPOINT_XFER_ISOC;
                }
            } else if ((end_desc.bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_BULK) {
                if (end_desc.wMaxPacketSize > wMaxPacketSize) {
                    wMaxPacketSize = end_desc.wMaxPacketSize;
                    uvc->ep = end_desc.bEndpointAddress & 0x0f;
                    uvc->wMaxPacketSize = end_desc.wMaxPacketSize;
                    uvc->interval = end_desc.bInterval;
                    uvc->if_alt_num = AlternateSetting;
                    uvc->xfer_type = USB_ENDPOINT_XFER_BULK;
                }
            }
            /*log_d("=======[%s - %d] >>>> \nif_alt_num : %d\ninterval : %d\nwMaxPacketSize : %d\n", __FUNCTION__,*/
            /*__LINE__, AlternateSetting, end_desc.bInterval, end_desc.wMaxPacketSize);*/
            if (cur_len != USB_DT_ENDPOINT_SIZE) {
                loop = 0;
            }
        } else if (cur_type == USB_DT_INTERFACE_ASSOCIATION) {
            //	composite device
            loop = 0;
            cur_len = 0;
        } else {
            drv_dbg("error %x", len);
            /*drv_printf_buf(pBuf + len, 16);*/
            loop = 0;
            cur_len = 0;
        }
        len += cur_len;
    }
    if (real_uvc) {
        memcpy(real_uvc, uvc, sizeof(*uvc));
    }
    free(uvc);
    uvc = NULL;
    if (!__this(usb_id)->buffer) {
        __this(usb_id)->buffer = malloc(__this(usb_id)->rxmaxp);
        if (!__this(usb_id)->buffer) {
            drv_dbg("err no mem in __this(usb_id)->buffer \n");
            goto __err;
        }
    }
    host_dev->interface_info[usb_id] = &uvc_host_inf;
    __this(usb_id)->uvc = real_uvc;
    __this(usb_id)->dev.private_data = host_dev;
    __this(usb_id)->name = (u8 *)(usb_id ? "uvc1" : "uvc0");
    __this(usb_id)->in_irq = 0;
    if (host_dev->interface_info[usb_id]->ctrl->set_power) {
        host_dev->interface_info[usb_id]->ctrl->set_power(host_dev, 1);
    }
    return len;

__err:
    printf("error in usb_uvc_parser\n");
    if (real_uvc) {
        free(real_uvc);
        real_uvc = NULL;
    }
    if (uvc) {
        free(uvc);
        uvc = NULL;
    }
    __this(usb_id)->in_irq = 0;
    return -EINVAL;
}
void usb_uvc_release(u8 id)
{
    drv_info("usb_uvc_release \n");
    __this(id)->interface_init = 0;
    u32 timeout;
    time_get(timeout, 1000);
    while (__this(id)->in_irq && !time_out(jiffies, timeout));
    if (__this(id)->buffer) {
        free(__this(id)->buffer);
        __this(id)->buffer = NULL;
    }
    if (__this(id)->uvc) {
        free(__this(id)->uvc);
        __this(id)->uvc = NULL;
    }
    __this(id)->dev.private_data = NULL;
    __this(id)->name = NULL;
    __this(id)->online = 0;
}
#define     USB_GET_REQ		(USB_DIR_IN|USB_TYPE_CLASS|USB_RECIP_INTERFACE)
#define     USB_SET_REQ		(USB_DIR_OUT|USB_TYPE_CLASS|USB_RECIP_INTERFACE)
int uvc_host_get_backlight_compensation(void *fd, u32 *iostatus)
{
    struct device *device = (struct device *)fd;
    s32 ret;
    u32 tmp_buf = 0;
    //PU_BACKLIGHT_COMPENSATION_CONTROL
    ret = usb_control_msg(device_to_usbdev(device), UVC_GET_CUR, USB_GET_REQ, 0x0100, 0x0200, (u8 *)&tmp_buf, 2);
    memcpy(iostatus, &tmp_buf, 4);
    /* drv_dbg("%x", *iostatus); */
    check_ret(ret);
    return DEV_ERR_NONE;
}

int uvc_host_req_processing_unit(void *fd, struct uvc_processing_unit *pu)
{
    struct device *device = (struct device *)fd;
    int err = 0;

    err = usb_control_msg(device_to_usbdev(device), pu->request, pu->type, pu->value, pu->index, (u8 *)pu->buf, pu->len);

    return err;
}

static u8 Video_Probe_Commit_Control[26] = {
    0x00, 0x00,                     //bmHint
    0x01,                           //FormatIndex
    0x01,                           //FrameIndex
    DW1BYTE(FRAME_FPS_0), DW2BYTE(FRAME_FPS_0), DW3BYTE(FRAME_FPS_0), DW4BYTE(FRAME_FPS_0), //dwFrameInterval Frame interval in 100 ns units.
    0x00, 0x00,                     //KeyFrameRate
    0x00, 0x00,                     //PFrameRate
    0x00, 0x27,                     //CompQuanlity
    0x00, 0x00,                     //CompWindowsSize
    0x00, 0x00, //0x32,0x00,                      //Delay
    DW1BYTE(300 * 1024), DW2BYTE(300 * 1024), DW3BYTE(300 * 1024), DW4BYTE(300 * 1024), //MaxVideoFrameSize
    LOBYTE(512), HIBYTE(512), 0x00, 0x00, //MaxPayLoadTransferSize

};

static u32 StreamInterfaceNum_get_cur(struct device *device, u16 id)
{
    u32 ret;
    id = cpu_to_be16(id);
    ret = usb_control_msg(device_to_usbdev(device),  UVC_GET_CUR, USB_GET_REQ, id, 0x01, Video_Probe_Commit_Control, sizeof(Video_Probe_Commit_Control));
    //drv_dbg("%s:%x\n",__FUNCTION__,id);
    check_ret(ret);
    return DEV_ERR_NONE;
}
static u32 StreamInterfaceNum_set_cur(struct device *device, u16 id)
{
    struct usb_uvc *uvc = (struct usb_uvc *)device_to_uvc(device);
    u32 ret;
    id = cpu_to_be16(id);
    Video_Probe_Commit_Control[2] = uvc->mjpeg_format_index;
    Video_Probe_Commit_Control[3] = uvc->cur_frame;
    drv_dbg("%s:%x\n", __func__, uvc->cur_frame);
    ret = usb_control_msg(device_to_usbdev(device),  UVC_SET_CUR, USB_SET_REQ, id, 0x01, Video_Probe_Commit_Control, sizeof(Video_Probe_Commit_Control));
    //drv_dbg("%s:%x\n",__FUNCTION__,id);
    check_ret(ret);
    return DEV_ERR_NONE;
}
static void Stream_head_analysis(u8 *rx_addr)
{
#if 0
    u32 frame_cnt = 0, scr = 0;
    u16 sof_count = 0;
    /// Stream Head
    rx_addr[0x00] = 0x0C;//Head Length
    rx_addr[0x01] = hdr->bfh_val; //BFH[0] [7:0]={EOH,ERR,STI,RES,SCR,PTS,EOF,FID}
    /* rx_addr[0x1] &= ~(BIT(2) | BIT(3) | BIT(5) | BIT(6)); */
    frame_cnt = hdr->frame_cnt;
    rx_addr[0x02] = frame_cnt ;//PTS[7:0]
    rx_addr[0x03] = (frame_cnt >> 8); //PTS[15:8]
    rx_addr[0x04] = (frame_cnt >> 16) ; //PTS[23:16]
    rx_addr[0x05] = (frame_cnt >> 24) ; //PTS[31:24]
    scr = hdr->scr;
    rx_addr[0x06] = DW1BYTE(scr);//SCR[7:0]
    rx_addr[0x07] = DW2BYTE(scr);//SCR[15:8]
    rx_addr[0x08] = DW3BYTE(scr);//SCR[23:16]
    rx_addr[0x09] = DW4BYTE(scr);//SCR[31:24]
    sof_count = hdr->sof_cnt;
    rx_addr[0x0A] = LOBYTE(sof_count); //SCR[42:32] 1KHz SOF token counter
    rx_addr[0x0B] = HIBYTE(sof_count); //SCR[47:42] res set to 0.
#endif
}
struct uvc_stream_list uvc_global_l[2][ISO_INTR_CNT];
static void uvc_g_iso_transfer(struct usb_host_device *host_dev, u32 ep)
{
    usb_dev usb_id = host_device2id(host_dev);
    if (__this(usb_id)->req_close) {
        os_sem_post(&__this(usb_id)->sem);
        return;
    }
    int rx_len;
    u8 *start_addr;
    u8 error_flag;
    u8 stream_state = 0;
    u32 err_cnt = 0;
    struct usb_uvc *uvc = (struct usb_uvc *)device_to_uvc(&__this(usb_id)->dev);
    __this(usb_id)->in_irq = true;


    for (int i = 0 ; i < ISO_INTR_CNT ; i++) {
        rx_len = usb_h_ep_read_async(usb_id, UVC_STREAM_EP_IN, uvc->ep, __this(usb_id)->buffer, __this(usb_id)->rxmaxp, USB_ENDPOINT_XFER_ISOC, 0);
        start_addr = __this(usb_id)->buffer;
        struct uvc_stream_list *uvc_list = uvc_global_l[usb_id];
        if (rx_len > 0 && rx_len > start_addr[0]) {
            rx_len -= start_addr[0];
            error_flag = start_addr[1];
            if (!uvc->error_frame) {
                uvc_list->addr = start_addr + start_addr[0];
                uvc_list->length = rx_len;
            }
            //BFH[0] [7:0]={EOH,ERR,STI,RES,SCR,PTS,EOF,FID}
            if ((error_flag & BIT(6)) && uvc->error_frame == 0) {
                drv_err("stream_err\n");
                uvc->error_frame = 1;
                //send_error_info
                if (uvc->stream_out) {
                    uvc->stream_out(uvc->priv, -1, NULL, STREAM_ERR);
                    stream_state = STREAM_ERR;
                }
            }
            if (error_flag & BIT(1)) { //endof frame
                if (!uvc->error_frame) {
                    if (uvc->stream_out) {
                        uvc->stream_out(uvc->priv, 1, uvc_list, STREAM_EOF);
                        stream_state = STREAM_EOF;
                        __this(usb_id)->frame_cnt++;
                    }
                } else {
                    err_cnt ++;
                }
                uvc->error_frame = 0;//clear error_frame pending
            } else {
                if (!uvc->error_frame) {
                    if (uvc->bfh != (error_flag & BIT(0))) { //new frame
                        uvc->bfh = error_flag & BIT(0);//clear error_frame pending
                        if (uvc->stream_out) {
                            uvc->stream_out(uvc->priv, 1, uvc_list, STREAM_SOF);
                            stream_state = STREAM_SOF;
                        }
                    } else {
                        if (uvc->stream_out) {
                            uvc->stream_out(uvc->priv, 1, uvc_list, STREAM_NO_ERR);
                            stream_state = STREAM_NO_ERR;
                        }
                    }
                }
            }
        }
        usb_h_ep_read_async(usb_id, UVC_STREAM_EP_IN, uvc->ep, 0, 0, USB_ENDPOINT_XFER_ISOC, 1);
    }
    __this(usb_id)->in_irq = 0;
}
static void uvc_g_bulk_transfer(usb_dev usb_id, u32 ep)
{
    if (__this(usb_id)->req_close) {
        os_sem_post(&__this(usb_id)->sem);
        return;
    }
    u8 _header = 0;
    int rx_len;
    u8 *start_addr;
    u8 error_flag;
    u8 stream_state = 0;
    u32 err_cnt = 0;
    static u32 trans_err_cnt = 0;
    static u8 *last_buf = NULL;
    static u32 last_len = 0;
    static u32 frame_len = 0;
    struct usb_uvc *uvc = (struct usb_uvc *)device_to_uvc(&__this(usb_id)->dev);
    __this(usb_id)->in_irq = true;
    rx_len = usb_h_ep_read_async(usb_id, UVC_STREAM_EP_IN, uvc->ep, __this(usb_id)->buffer, __this(usb_id)->rxmaxp, USB_ENDPOINT_XFER_ISOC, 0);
    start_addr = __this(usb_id)->buffer;
    struct uvc_stream_list *uvc_list = uvc_global_l[usb_id];
    _header = 0;
    last_buf = start_addr;
    //BFH[0] [7:0]={EOH,ERR,STI,RES,SCR,PTS,EOF,FID}
    if (start_addr[0] == 0x0c) {
        if ((start_addr[1] & 0x8c) == 0x8c) {
            if (start_addr[0x0c] == 0xff && start_addr[0x0d] == 0xd8) {
                _header = 1;
            }
        }
    } else if (start_addr[0] == 0x02) {
        if ((start_addr[1] & 0x8c) == 0x80) {
            if (start_addr[0x02] == 0xff && start_addr[0x03] == 0xd8) {
                _header = 1;
            }
        }
    }
    if (_header) {
        if (rx_len >= start_addr[0]) {
            rx_len -= start_addr[0];
        }
        /* drv_printf_buf(start_addr, rx_len + start_addr[0]); */
        uvc_list->addr = start_addr + start_addr[0];
        uvc_list->length = rx_len;
        error_flag = start_addr[1];
        if ((error_flag & BIT(6)) && (uvc->error_frame == 0)) {
            frame_len = 0;
            uvc->error_frame = 1;
            if (uvc->stream_out) {
                uvc->stream_out(uvc->priv, -1, NULL, STREAM_ERR);
                stream_state = STREAM_ERR;
            }
        }
        if (uvc->bfh != (error_flag & BIT(0))) { //new frame
            uvc->bfh = error_flag & BIT(0);//clear error_frame pending
            if (last_len >= 2 &&
                last_buf[last_len - 2] == 0xff &&
                last_buf[last_len - 1] == 0xd9) {
                if (!uvc->error_frame) {
                    if (uvc->stream_out) {
                        uvc->stream_out(uvc->priv, 0, uvc_list, STREAM_EOF);
                        stream_state = STREAM_EOF;
                    }
                    //drv_info("frame_len %u\n", frame_len);
                }

                /* puts(" new\n"); */
                if (uvc_list->length) {
                    if (uvc->stream_out) {
                        uvc->stream_out(uvc->priv, 1, uvc_list, STREAM_SOF);
                        stream_state = STREAM_SOF;
                    }
                    uvc->error_frame = 0;
                    frame_len = rx_len;
                }
            } else {
                uvc->error_frame = 1;
                if (uvc->stream_out) {
                    uvc->stream_out(uvc->priv, -1, NULL, STREAM_ERR);
                    stream_state = STREAM_ERR;
                }
                frame_len = 0;
            }
        }
        last_len = rx_len;
        memcpy(last_buf, start_addr + start_addr[0], rx_len);
    } else {
        uvc_list->addr = start_addr;
        uvc_list->length = rx_len;
        if (!uvc->error_frame) {
            /* drv_printf_buf(start_addr, rx_len); */
            if (uvc->stream_out) {
                uvc->stream_out(uvc->priv, 1, uvc_list, STREAM_NO_ERR);
                stream_state = STREAM_NO_ERR;
            }
            frame_len += rx_len;
        }
        last_len = rx_len;
        memcpy(last_buf, start_addr, rx_len);
    }
    usb_h_ep_read_async(usb_id, UVC_STREAM_EP_IN, uvc->ep, 0, 0, USB_ENDPOINT_XFER_ISOC, 1);
    __this(usb_id)->in_irq = 0;
}
static void usb_intr_config(struct device *device, u8 ep, u8 enable)
{
    u8 id = usbpriv_to_usbid(usbdev_to_usbpriv(device_to_usbdev(device)));
    if (enable) {
        usb_set_intr_rxe(id, ep);
    } else {
        usb_clr_intr_rxe(id, ep);
        usb_write_rxcsr(id, ep, RXCSRH_ClrDataTog | RXCSRH_FlushFIFO);
        usb_write_rxinterval(id, ep, 0);
    }
}

static void iso_ep_rx_init(struct device *device)
{
    u8 id = usbpriv_to_usbid(usbdev_to_usbpriv(device_to_usbdev(device)));
    struct usb_uvc *uvc = (struct usb_uvc *)device_to_uvc(device);
    u8 *ep_buffer = usb_alloc_ep_dmabuffer(id, UVC_STREAM_EP_IN | USB_DIR_IN, UVC_HOST_FIFO_RXMAXP);
    ASSERT(ep_buffer, "usb alloc ep buffer failed");
    struct usb_host_device *host_dev = host_id2device(id);
    u8 devnum = host_dev->private_data.devnum;

    usb_write_rxfuncaddr(id, UVC_STREAM_EP_IN, devnum);


    usb_h_set_ep_isr(host_dev, UVC_STREAM_EP_IN | USB_DIR_IN, uvc_g_iso_transfer, host_dev);

    usb_h_ep_config(id, UVC_STREAM_EP_IN | USB_DIR_IN, USB_ENDPOINT_XFER_ISOC, 1, uvc->interval, ep_buffer, UVC_HOST_FIFO_RXMAXP);

    usb_h_ep_read_async(id, UVC_STREAM_EP_IN, uvc->ep, 0, 0, USB_ENDPOINT_XFER_ISOC, 1);
}
static void bulk_ep_rx_init(struct device *device)
{
    u8 id = usbpriv_to_usbid(usbdev_to_usbpriv(device_to_usbdev(device)));
    struct usb_uvc *uvc = (struct usb_uvc *)device_to_uvc(device);
    u8 *ep_buffer = usb_alloc_ep_dmabuffer(id, UVC_STREAM_EP_IN | USB_DIR_IN, UVC_HOST_FIFO_RXMAXP);
    ASSERT(ep_buffer, "usb alloc ep buffer failed");
    struct usb_host_device *host_dev = host_id2device(id);
    usb_h_ep_config(id, UVC_STREAM_EP_IN | USB_DIR_IN, USB_ENDPOINT_XFER_BULK, 1, uvc->interval, ep_buffer, UVC_HOST_FIFO_RXMAXP);
    usb_h_set_ep_isr(host_dev, UVC_STREAM_EP_IN | USB_DIR_IN, uvc_g_bulk_transfer, host_dev);
    usb_h_ep_read_async(id, UVC_STREAM_EP_IN, uvc->ep, 0, 0, USB_ENDPOINT_XFER_BULK, 1);
}
int uvc_host_close_camera(void *fd)
{
    int ret;
    if (uvc_host_online() < 0) {
        printf("---> %s %d \n", __func__, __LINE__);
        return DEV_ERR_NONE;
    }
    struct device *device = (struct device *)fd;
    struct usb_uvc *uvc = (struct usb_uvc *)device_to_uvc(device);
    u8 status = usbpriv_to_usbonline(usbdev_to_usbpriv(device_to_usbdev(device)));
    u8 id = usbpriv_to_usbid(usbdev_to_usbpriv(device_to_usbdev(device)));
    u32 timeout;
    if (!uvc) {
        printf("---> %s %d \n", __func__, __LINE__);
        return DEV_ERR_NONE;
    }
    time_get(timeout, 1000);
    if (uvc->xfer_type == USB_ENDPOINT_XFER_ISOC) {

#if (STOP_TRANSFER == 0)
#if CAMERA_CLOSE_BY_IRQ
#endif
        if (uvc->camera_open) {
#if !CAMERA_CLOSE_BY_IRQ
            uvc->stream_out = uvc_dummy_stream_out;
#endif
            __this(id)->req_close = true;
            ret = os_sem_pend(&__this(id)->sem, 100);
            if (ret) {
                drv_err("err os_sem_pend err in %s\n", __func__);
            }
            while (__this(id)->in_irq && !time_out(jiffies, timeout));
            usb_intr_config(device, UVC_STREAM_EP_IN, 0);
            __this(id)->req_close = 0;
            os_sem_del(&__this(id)->sem, 0);
            uvc->camera_open = 0;
            if (status) {
                ret = usb_control_msg(device_to_usbdev(device), USB_REQ_SET_INTERFACE, 0x01, 0, 0x0001, NULL, 0);
                drv_dbg("USB_REQ_SET_INTERFACE %d to %d", 0x01, 0);
                if (ret) {
                    drv_err("USB_REQ_SET_INTERFACE err = %d \n", ret);
                    return 0;
                }
            }
        }
#else  /*STOP_TRANSFER*/
        if (uvc->camera_open) {
#if !CAMERA_CLOSE_BY_IRQ
            uvc->stream_out = uvc_dummy_stream_out;
#endif
#if ISO_INTR_MODE == 2
#elif ISO_INTR_MODE == 1
#else
#endif
#if CAMERA_CLOSE_BY_IRQ
#endif
            __this(id)->req_close = true;
            ret = os_sem_pend(&__this(id)->sem, 100);
            if (ret) {
                drv_err("err os_sem_pend err in %s\n", __func__);
            }
            while (__this(id)->in_irq && !time_out(jiffies, timeout));
            usb_intr_config(device, UVC_STREAM_EP_IN, 0);
            __this(id)->req_close = 0;
            os_sem_del(&__this(id)->sem, 0);
            uvc->camera_open = 0;
            if (status) {
                ret = usb_control_msg(device_to_usbdev(device), USB_REQ_SET_INTERFACE, 0x01, 0, 0x0001, NULL, 0);
                drv_dbg("USB_REQ_SET_INTERFACE %d to %d", 0x01, 0);
                if (ret) {
                    drv_err("USB_REQ_SET_INTERFACE err = %d \n", ret);
                    return 0;
                }
            }
        }
#endif

    } else if (uvc->xfer_type == USB_ENDPOINT_XFER_BULK) {
#if !CAMERA_CLOSE_BY_IRQ
        uvc->stream_out = uvc_dummy_stream_out;
#endif
        __this(id)->req_close = true;
        os_sem_pend(&__this(id)->sem, 100);
        while (__this(id)->in_irq && !time_out(jiffies, timeout));
        usb_intr_config(device, UVC_STREAM_EP_IN, 0);
        __this(id)->req_close = 0;
        os_sem_del(&__this(id)->sem, 0);
        uvc->camera_open = 0;
        if (status) {
            //clear endpoint feature, halt endpoint
            ret = usb_control_msg(device_to_usbdev(device), USB_REQ_CLEAR_FEATURE, 0x02, 0, uvc->ep | USB_DIR_IN, NULL, 0);
            drv_dbg("USB_REQ_CLEAR_FEATURE");
            check_ret(ret);

            Video_Probe_Commit_Control[0]  = 0;
            Video_Probe_Commit_Control[1]  = 0;
            Video_Probe_Commit_Control[2]  = uvc->mjpeg_format_index;
            Video_Probe_Commit_Control[3]  = uvc->cur_frame;
            ret = usb_control_msg(device_to_usbdev(device), UVC_SET_CUR, USB_SET_REQ, 0x0100, 0x0001, Video_Probe_Commit_Control, sizeof(Video_Probe_Commit_Control));
            check_ret(ret);
            drv_dbg("Video Probe Control SET_CUR");
            /*drv_printf_buf(Video_Probe_Commit_Control, sizeof(Video_Probe_Commit_Control));*/

            ret = usb_control_msg(device_to_usbdev(device), UVC_GET_CUR, USB_GET_REQ, 0x0100, 0x0001, Video_Probe_Commit_Control, sizeof(Video_Probe_Commit_Control));
            check_ret(ret);
            drv_dbg("Video Probe Control GET_CUR");
            /*drv_printf_buf(Video_Probe_Commit_Control, sizeof(Video_Probe_Commit_Control));*/

            ret = usb_control_msg(device_to_usbdev(device), UVC_SET_CUR, USB_SET_REQ, 0x0100, 0x0001, Video_Probe_Commit_Control, sizeof(Video_Probe_Commit_Control));
            check_ret(ret);
            drv_dbg("Video Probe Control SET_CUR");
            /*drv_printf_buf(Video_Probe_Commit_Control, sizeof(Video_Probe_Commit_Control));*/

            ret = usb_control_msg(device_to_usbdev(device), UVC_GET_CUR, USB_GET_REQ, 0x0100, 0x0001, Video_Probe_Commit_Control, sizeof(Video_Probe_Commit_Control));
            check_ret(ret);
            drv_dbg("Video Probe Control GET_CUR");
            /*drv_printf_buf(Video_Probe_Commit_Control, sizeof(Video_Probe_Commit_Control));*/

            u8 tmp_buf[10];
            ret = usb_control_msg(device_to_usbdev(device), UVC_GET_DEF, USB_GET_REQ, 0x1400, 0x0100, tmp_buf, 0x0a);
            check_ret(ret);
            drv_dbg("CameraTerminal GET_DEF");
            /*drv_printf_buf(tmp_buf, ret);*/

            memset(tmp_buf, 0, sizeof(tmp_buf));
            ret = usb_control_msg(device_to_usbdev(device), UVC_SET_CUR, USB_SET_REQ, 0x1400, 0x0100, tmp_buf, 0x0a);
            check_ret(ret);
            drv_dbg("CameraTerminal SET_CUR");
            /*drv_printf_buf(tmp_buf, ret);*/
        }
    }
    return DEV_ERR_NONE;
}


static int usb_uvc_info(struct device *device);
int uvc_host_open_camera(void *fd)
{
    u32 ret;
    struct device *device = (struct device *)fd;
    struct usb_uvc *uvc = (struct usb_uvc *)device_to_uvc(device);
    /*ret = uvc_host_close_camera(device);*/
    u8 id = usbpriv_to_usbid(usbdev_to_usbpriv(device_to_usbdev(device)));

    uvc->bfh = 0;
    uvc->error_frame = 1;
    if (uvc->xfer_type == USB_ENDPOINT_XFER_ISOC) {
        drv_dbg("USB_ENDPOINT_XFER_ISOC\n");
#if (STOP_TRANSFER == 0)
        if (uvc->camera_open) {
#if !CAMERA_CLOSE_BY_IRQ
            uvc->stream_out = uvc->bak_stream_out;
#endif
            drv_dbg("warning: UVC ALREADY OPEN\n\n\n");
            return 0;
        }
        ret = usb_uvc_info(device);
        if (ret) {
            goto __err;
        }
        ret = usb_control_msg(device_to_usbdev(device), USB_REQ_SET_INTERFACE, 0x01, uvc->if_alt_num, 0x0001, NULL, 0);
        if (ret) {
            goto __err;
        }
        drv_dbg("USB_REQ_SET_INTERFACE %d to %d", 0x01, uvc->if_alt_num);
        check_ret(ret);
        iso_ep_rx_init(device);
        uvc->camera_open = true;

#else  /*STOP_TRANSFER*/
        if (!uvc->camera_open) {
#if !CAMERA_CLOSE_BY_IRQ
            uvc->stream_out = uvc->bak_stream_out;
#endif
            ret = usb_uvc_info(device);
            if (ret) {
                goto __err;
            }
            ret = usb_control_msg(device_to_usbdev(device), USB_REQ_SET_INTERFACE, 0x01, uvc->if_alt_num, 0x0001, NULL, 0);
            drv_dbg("USB_REQ_SET_INTERFACE %d to %d", 0x01, uvc->if_alt_num);
            if (ret) {
                goto __err;
            }
            check_ret(ret);
            iso_ep_rx_init(device);
            uvc->camera_open = true;
        }
#endif

    } else if (uvc->xfer_type == USB_ENDPOINT_XFER_BULK) {
        drv_dbg("USB_ENDPOINT_XFER_BULK\n");
        ret = usb_uvc_info(device);
        if (ret) {
            goto __err;
        }
        bulk_ep_rx_init(device);
#if !CAMERA_CLOSE_BY_IRQ
        uvc->stream_out = uvc->bak_stream_out;
#endif
        uvc->camera_open = true;
    }
    os_sem_create(&__this(id)->sem, 0);
    drv_info("uvc usb open ok\n");;
    return DEV_ERR_NONE;
__err:
    return -EINVAL;
}

#define USE_HUSB    0
static int usb_uvc_info(struct device *device)
{
    struct uvc_parm p;
#if USE_HUSB
    u8 tmp_Video_Probe_Commit_Control[26] = {
        0x00, 0x00,
        0x02,//1 yuv 2 mjpg
        0x01,//6 1280*720 5 640*360 1 640*480
        0x15, 0x16, 0x05, 0x00,
        0x00, 0x00,
        0x00, 0x00,
        0x00, 0x00,
        0x00, 0x00,
        0x20, 0x00,
        0x00, 0x60, 0x09, 0x00,
        0x00, 0x0c, 0x00, 0x00
    };
    StreamInterfaceNum_get_cur(device, 1);
    memcpy(Video_Probe_Commit_Control, tmp_Video_Probe_Commit_Control, 26);
    StreamInterfaceNum_set_cur(device, 1);

    StreamInterfaceNum_get_cur(device, 1);
    /*drv_printf_buf(Video_Probe_Commit_Control, sizeof(Video_Probe_Commit_Control));*/

    memcpy(Video_Probe_Commit_Control, tmp_Video_Probe_Commit_Control, 26);
    StreamInterfaceNum_set_cur(device, 1);
    memcpy(Video_Probe_Commit_Control, tmp_Video_Probe_Commit_Control, 26);
    StreamInterfaceNum_set_cur(device, 2);


    //StreamInterfaceNum_get_cur(device,1);
    //drv_printf_buf(Video_Probe_Commit_Control,sizeof(Video_Probe_Commit_Control));
#else
    int ret  = 0;
    struct usb_uvc *uvc = (struct usb_uvc *)device_to_uvc(device);

    ret = usb_control_msg(device_to_usbdev(device), UVC_GET_CUR, USB_GET_REQ, 0x0100, 0x0001, Video_Probe_Commit_Control, sizeof(Video_Probe_Commit_Control));
    drv_dbg("Video Probe Control GET_CUR");
    /*drv_printf_buf(Video_Probe_Commit_Control, sizeof(Video_Probe_Commit_Control));*/
    check_ret(ret);

    ret = usb_control_msg(device_to_usbdev(device), UVC_SET_CUR, USB_SET_REQ, 0x0100, 0x0001, Video_Probe_Commit_Control, sizeof(Video_Probe_Commit_Control));
    drv_dbg("Video Probe Control SET_CUR");
    /*drv_printf_buf(Video_Probe_Commit_Control, sizeof(Video_Probe_Commit_Control));*/
    check_ret(ret);

    ret = usb_control_msg(device_to_usbdev(device), UVC_GET_MIN, USB_GET_REQ, 0x0100, 0x0001, Video_Probe_Commit_Control, sizeof(Video_Probe_Commit_Control));
    drv_dbg("Video Probe Control GET_MIN");
    /*drv_printf_buf(Video_Probe_Commit_Control, sizeof(Video_Probe_Commit_Control));*/
    check_ret(ret);

    ret = usb_control_msg(device_to_usbdev(device), UVC_GET_MAX, USB_GET_REQ, 0x0100, 0x0001, Video_Probe_Commit_Control, sizeof(Video_Probe_Commit_Control));
    drv_dbg("Video Probe Control GET_MAX");
    /*drv_printf_buf(Video_Probe_Commit_Control, sizeof(Video_Probe_Commit_Control));*/
    check_ret(ret);

    ret = usb_control_msg(device_to_usbdev(device), UVC_GET_CUR, USB_GET_REQ, 0x0100, 0x0001, Video_Probe_Commit_Control, sizeof(Video_Probe_Commit_Control));
    drv_dbg("Video Probe Control GET_CUR");
    /*drv_printf_buf(Video_Probe_Commit_Control, sizeof(Video_Probe_Commit_Control));*/
    check_ret(ret);

    Video_Probe_Commit_Control[2] = uvc->mjpeg_format_index;
    Video_Probe_Commit_Control[3] = uvc->cur_frame;
    ret = usb_control_msg(device_to_usbdev(device), UVC_SET_CUR, USB_SET_REQ, 0x0200, 0x0001, Video_Probe_Commit_Control, sizeof(Video_Probe_Commit_Control));
    drv_dbg("Video Commit Control SET_CUR");
    /*drv_printf_buf(Video_Probe_Commit_Control, sizeof(Video_Probe_Commit_Control));*/
    check_ret(ret);

#endif
    return DEV_ERR_NONE;
}
s32 usb_host_video_set_interface_init(struct usb_host_device *host_dev)
{
    int ret = 0;
    u8 id = usbpriv_to_usbid(usbdev_to_usbpriv(host_dev));
    if (!__this(id)->interface_init) {
        ret = usb_control_msg(host_dev, USB_REQ_SET_INTERFACE, UVC_SET_CUR, 0, 1,  NULL, 0);
        check_ret(ret);
        if (!ret) {
            __this(id)->interface_init = 1;
        }
    }
    return ret;
}

static void toogle_port()
{

}
struct status_packet_format {
    u8 bStatusType;//bit0-3 : 0=Reserved, 1=VideoControl interface, 2 = VideoStreaming interface
    u8 bOriginator;//ID of Teriminal, Unit or Interface
};

struct status_vc_packet {
    u8 bEvent;//0x0 : Control Change , 0x1 - 0xFF : Reserved
    u8 bSelector;//Report the Control Selector of th control that issued the interrupt
    u8 bAttribute;//0x0:Control value change, 0x1:Control info change, 0x2:Control failure change, 0x3-0xff : Reserved
    u8 bValue;//0x0:Equivalent to the result of a GET_CUR request, 0x1:GET_INFO request, 0x2:GET_CUR request on VC_REQUEST_ERROR_CODE_CONTROL
};

struct status_vs_packet {
    u8 bEvent;//0x0:Button Press, 0x1-0xff:Stream Error
    u8 bValue;//Button Press--0x0:button released, 0x1:button pressed
};
void *uvc_host_open(void *arg)
{
    s32 ret;
    struct uvc_host_param *info = (struct uvc_host_param *)arg;
    struct usb_uvc *uvc;
    struct device *device;
    u8 id;

    device = uvc_host_device_find(info->name);
    if (device) {
        atomic_inc(&(device->ref));
        id = usbpriv_to_usbid(usbdev_to_usbpriv(device_to_usbdev(device)));
        uvc = (struct usb_uvc *)device_to_uvc(device);
        if (__this(id)->open) {
            drv_err("err uvc host reopen !!!\n");
            return NULL;
        }
        if (uvc) {
            if (usb_host_video_set_interface_init(device_to_usbdev(device))) {
                drv_dbg("usb_host_video_set_interface_init err\n");
                return NULL;
            }
            uvc->priv = info->priv;
#if !CAMERA_CLOSE_BY_IRQ
            if (!uvc->camera_open) {
                uvc->stream_out = info->uvc_stream_out;
            }
            uvc->bak_stream_out = info->uvc_stream_out;
#else
            uvc->stream_out = info->uvc_stream_out;
#endif
            uvc->offline = info->uvc_out;
        }
        __this(id)->open = true;
        return device;
    } else {
        drv_dbg("err uvc_host_open device NULL\n");
        return NULL;
    }
}

int uvc_host_close(void *fd)
{
    struct device *device = (struct device *)fd;
    u8 id = usbpriv_to_usbid(usbdev_to_usbpriv(device_to_usbdev(device)));

    if ((device) && (device->private_data != NULL)) {
        if (atomic_dec_and_test(&device->ref)) {
            if (__this(id)->open && !__this(id)->online) {
                usb_uvc_release(id);
            }
            __this(id)->open = 0;
        }
    }
    return DEV_ERR_NONE;
}

int uvc_host_get_pix_table(void *fd, struct uvc_frame_info **pix_table)
{
    if (uvc_host_online() < 0) {
        return -EINVAL;
    }
    struct device *device = (struct device *)fd;
    struct usb_uvc *uvc = (struct usb_uvc *)device_to_uvc(device);

    if (uvc && uvc->reso_num) {
        *pix_table = uvc->pframe;
        return uvc->reso_num;
    }
    return 0;
}
int uvc_host_get_fps(void *fd)
{
    struct device *device = (struct device *)fd;
    struct usb_uvc *uvc = (struct usb_uvc *)device_to_uvc(device);
    if (uvc) {
        return uvc->fps;
    }
    return 25;
}

int uvc_host_set_pix(void *fd, int index)
{
    struct device *device = (struct device *)fd;
    struct usb_uvc *uvc = (struct usb_uvc *)device_to_uvc(device);

    if (uvc) {
        uvc->cur_frame = index;
    }
    return 0;
}

int uvc_force_reset(void *fd)
{
    struct device *device = (struct device *)fd;
    if (uvc_host_online() < 0) {
        return -EINVAL;
    }
    u8 id = usbpriv_to_usbid(usbdev_to_usbpriv(device_to_usbdev(device)));
    uvc_host_close_camera(fd);
    return usb_h_force_reset(id);
}

int uvc_host_camera_out(void *fd)
{
    if (uvc_host_online() < 0) {
        return -EINVAL;
    }
    struct device *device = (struct device *)fd;
    struct usb_uvc *uvc = (struct usb_uvc *)device_to_uvc(device);
    u8 id = usbpriv_to_usbid(usbdev_to_usbpriv(device_to_usbdev(device)));

    if (uvc->offline) {
        uvc->offline(uvc->priv);
    }
#if (STOP_TRANSFER == 0)
    uvc->camera_open = 0;
#endif
    uvc_host_close_camera(device);
    return 0;
}

int uvc2usb_ioctl(void *fd, u32 cmd, void *arg)
{
    struct device *device = (struct device *)fd;
    struct usb_uvc *uvc = (struct usb_uvc *)device_to_uvc(device);

    switch (cmd) {
    case UVCIOC_SET_EVENT_LISTENER: {
        struct uvc_event_listener *listener = (struct uvc_event_listener *)arg;
        uvc->event_priv = listener->priv;
        uvc->event_handler = listener->handler;
    }
    break;
    default:
        drv_err("err in uvc2usb_ioctl \n\n");
        return -EINVAL;
        /*return host_device_ioctl(device, cmd, arg);*/
    }
    return 0;
}

#endif // USB_HOST_UVC
