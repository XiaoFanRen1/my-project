#include "app_config.h"
#include "system/includes.h"
#include "os/os_api.h"
#include "app_config.h"
#include "fs/fs.h"
#include "asm/jpeg_codec.h"

extern void get_yuv_init(void (*cb)(u8 *data, u32 len, int width, int height));
extern void my_lcd_test(void);
extern void lcd_clear_screen(u16 color);
extern void lcd_show_frame(u8 *buf, u32 len);
extern void yuv420p_quto_rgb565(unsigned char *yuvBuffer_in, unsigned char *rgbBuffer_out, int width, int height);
extern void yuv420p_quto_rgb24(unsigned char *yuvBuffer_in, unsigned char *rgbBuffer_out, int width, int height);
extern int YUV420p_Soft_Scaling(unsigned char *src, unsigned char *out, int src_w, int src_h, int out_w, int out_h);
extern void lcd_get_width_height(int *width, int *height);
extern void lcd_clear_screen_color(void);
extern int lcd_get_color_format_rgb24(void);
extern void lcd_test(void);
extern int storage_device_ready(void);

//写数据到SD卡
static void save_jpg_ontime(u8 *buf, u32 len)
{
    if (storage_device_ready()) {
        char file_name[64];//定义路径存储
        snprintf(file_name, 64, CONFIG_ROOT_PATH"JPG/JPG***.JPG");
        FILE *fd = fopen(file_name, "wb+");
        if (fd) {
            fwrite(buf, 1, len, fd);
            fclose(fd);
            printf("JPG save ok name=JPG\r\n");
        }
    }
}
//写Y数据到SD卡
static void save_YUV_Ydate_ontime(u8 *buf, u32 len)
{
    if (storage_device_ready()) {
        char file_name[64];//定义路径存储
        snprintf(file_name, 64, CONFIG_ROOT_PATH"YUV_Y/YUV_Y***.y");
        FILE *fd = fopen(file_name, "wb+");
        if (fd) {
            fwrite(buf, 1, len, fd);
            fclose(fd);
            printf("YUV_Y save ok name=JPG\r\n");
        }
    }
}
static void get_RGB_save_to_SD(char *RGB, u32 len)
{
    if (storage_device_ready()) {
        char file_name[64];//定义路径存储
        snprintf(file_name, 64, CONFIG_ROOT_PATH"rgb/RGB***.rgb");
        FILE *fd = fopen(file_name, "wb+");
        if (fd) {
            fwrite(RGB, 1, len, RGB);
            fclose(fd);
            printf("RGB save ok name=JPG\r\n");
        }
    }
}
void get_yuv_y_save_to_SD(char *yuv_buf, int src_w, int src_h, int yuv_y_out_w, int yuv_y_out_h)
{
    u32 buf_len =  yuv_y_out_w * yuv_y_out_h * 3 / 2;
    u8 *yuv_y_out_buf =  malloc(buf_len);
    if (!yuv_y_out_buf) {
        printf("yuv buf not enough err!!!\n");
        return;
    }
    YUV420p_Soft_Scaling(yuv_buf, yuv_y_out_buf, src_w, src_h, yuv_y_out_w, yuv_y_out_h);
    save_YUV_Ydate_ontime(yuv_y_out_buf, (yuv_y_out_w * yuv_y_out_h));
    free(yuv_y_out_buf);
}
void get_JPG_save_to_SD(char *yuv_buf, int src_w, int src_h, int yuv_out_w, int yuv_out_h)
{
    struct jpeg_encode_req req = {0};
    u32 buf_len =  yuv_out_w * yuv_out_h * 3 / 2 ;
    u8 *yuv_out_buf =  malloc(buf_len);
    u8 err = 0;

    if (!yuv_out_buf) {
        printf("jpeg buf not enough err!!!\n");
        return;
    }
    YUV420p_Soft_Scaling(yuv_buf, yuv_out_buf, src_w, src_h, yuv_out_w, yuv_out_h);

    //q >=2 , size  = w*h*q*0.32
    req.q = 10;	//jepg编码质量(范围0-13),0最小 质量越好,下面的编码buf需要适当增大
    req.format = JPG_SAMP_FMT_YUV420;
    req.data.buf = yuv_out_buf;
    req.data.len = buf_len;
    req.width = yuv_out_w ;
    req.height =  yuv_out_h ;
    req.y = yuv_out_buf;
    req.u = req.y + (yuv_out_w * yuv_out_h);
    req.v = req.u + (yuv_out_w * yuv_out_h) / 4;

    err = jpeg_encode_one_image(&req);//获取一张JPG图片
    if (!err) {
        save_jpg_ontime(req.data.buf, buf_len); //保存图片到SD卡
    } else {
        printf("get_jpg err!!!\n");
    }
    free(yuv_out_buf);
}
void camera_show_lcd(u8 *buf, u32 size, int width, int height)
{
    int src_w = width;
    int src_h = height;
    int out_w;
    int out_h;
    int out_size;
    char rgb24 = 0;
    static u8 *out = NULL;
    static u8 *scaling_yuv = NULL;

    rgb24 = lcd_get_color_format_rgb24();
    lcd_get_width_height(&out_w, &out_h);
    if (out_w <= 0 || out_h <= 0) {
        printf("lcd_get_width_height err \n");
        return ;
    }
    out_size = out_w * out_h * (rgb24 ? 3 : 2);
    if (!out) {
        out = malloc(out_size);
        if (!out) {
            printf("no mem in camera_show_lcd \n");
            return ;
        }
    }
    if (out_w > src_w || out_h > src_h) {//有需要放大的申请YUV
        if (!scaling_yuv) {
            scaling_yuv = malloc(out_w * out_h * 3 / 2);
            if (!scaling_yuv) {
                printf("no mem in camera_show_lcd \n");
                return ;
            }
        }
    }
    /***********保存数据到SD卡例子:*************/
    //get_JPG_save_to_SD(buf, src_w, src_h, out_w, out_h);//保存原始摄像头YUV成JPEG图片到SD卡
    //get_yuv_y_save_to_SD(buf, src_w, src_h, out_w, out_h);//缩放YUV保存指定分辨率Y到SD卡
    /*******************************************/

    YUV420p_Soft_Scaling(buf, scaling_yuv, src_w, src_h, out_w, out_h);//缩放到LCD指定的宽高
    if (rgb24) {
        yuv420p_quto_rgb24(scaling_yuv ? scaling_yuv : buf, out, out_w, out_h);//YUV转RGB
    } else {
        yuv420p_quto_rgb565(scaling_yuv ? scaling_yuv : buf, out, out_w, out_h);//YUV转RGB
    }
    lcd_show_frame(out, out_size);//显示一帧摄像头数据
}
void camera_to_lcd_test(void)
{
    my_lcd_test();
    get_yuv_init(camera_show_lcd);
}
