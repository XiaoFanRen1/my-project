
#include "system/includes.h"
#include "typedef.h"
#include "asm/pap.h"
#include "app_config.h"
#include "lcd_drive.h"
#include "lcd_te_driver.h"
#include "lcd_config.h"

#if TCFG_LCD_GC9307_ENABLE

#define ROTATE_DEGREE_0  	0
#define ROTATE_DEGREE_90  	1
#define ROTATE_DEGREE_180  	2
#define ROTATE_DEGREE_270 	3

#define WHITE         	 0xFFFF
#define BLACK         	 0x0000
#define BLUE         	 0x001F
#define BRED             0XF81F
#define GRED 			 0XFFE0
#define GBLUE			 0X07FF
#define RED           	 0xF800
#define MAGENTA       	 0xF81F
#define GREEN         	 0x07E0
#define CYAN          	 0x7FFF
#define YELLOW        	 0xFFE0
#define BROWN 			 0XBC40 //棕色
#define BRRED 			 0XFC07 //棕红色
#define GRAY  			 0X8430 //灰色i

void msleep(unsigned int ms);

#define lcd_delay(x) msleep(x)

static void WriteCOM(u8 cmd)
{
    lcd_cs_pinstate(0);
    lcd_rs_pinstate(0);//cmd
    lcd_send_byte(cmd);
    lcd_cs_pinstate(1);
}

static void WriteDAT_8(u8 dat)
{
    lcd_cs_pinstate(0);
    lcd_rs_pinstate(1);//dat
    lcd_send_byte(dat);
    lcd_cs_pinstate(1);
}

static void WriteDAT_DMA(u8 *dat, u32 len)
{
    lcd_cs_pinstate(0);
    lcd_rs_pinstate(1);//dat
    lcd_send_map(dat, len);
    lcd_cs_pinstate(1);
}

static void ReadDAT_DMA(u8 *dat, u16 len)
{
    lcd_cs_pinstate(0);
    lcd_rs_pinstate(1);//dat
    lcd_read_data(dat, len);
    lcd_cs_pinstate(1);
}

static u8 ReadDAT_8(void)
{
    u8 dat = 0;
    lcd_cs_pinstate(0);
    lcd_rs_pinstate(1);//dat
    lcd_read_data(&dat, 1);
    lcd_cs_pinstate(1);
    return dat;
}

static u8 ReadDAT_16(void)
{
    u16 dat = 0;
    lcd_cs_pinstate(0);
    lcd_rs_pinstate(1);//dat
    lcd_read_data((u8 *)&dat, 2);
    lcd_cs_pinstate(1);
    return dat;
}

static void GC9307_SetRange(u16 xs, u16 xe, u16 ys, u16 ye)
{
    /******UI每次发送数据都会调用开窗告诉屏幕要刷新那个区域***********/
    get_lcd_ui_x_y(xs, xe, ys, ye);
}

static void GC9307_SetRange_1(u16 xs, u16 xe, u16 ys, u16 ye)
{
    WriteCOM(0x2A);
    WriteDAT_8(ys >> 8);
    WriteDAT_8(ys);
    WriteDAT_8(ye >> 8);
    WriteDAT_8(ye);
    WriteCOM(0x2B);
    WriteDAT_8(xs >> 8);
    WriteDAT_8(xs);
    WriteDAT_8(xe >> 8);
    WriteDAT_8(xe);
}

static void GC9307_io_init()
{
    lcd_rs_pinstate(0);
    lcd_cs_pinstate(1);
    lcd_rst_pinstate(1);
}


void GC9307_set_direction(u8 dir)
{
    WriteCOM(0x36);    //扫描方向控制

    if (dir == ROTATE_DEGREE_0) { //正向
#if HORIZONTAL_SCREEN
        WriteDAT_8(0x0c);
#else
        WriteDAT_8(0x0c);
#endif
    } else if (dir == ROTATE_DEGREE_180) { //翻转180
#if HORIZONTAL_SCREEN
        WriteDAT_8(0x08);
#else
        WriteDAT_8(0x88);
#endif
    }
    GC9307_SetRange(0, LCD_W - 1, 0, LCD_H - 1);
}

void GC9307_clear_screen(u16 color)
{
    WriteCOM(0x2c);
    u8 *buf = malloc(LCD_W * LCD_H * 2);
    if (!buf) {
        printf("no men in %s \n", __func__);
        return;
    }
    for (u32 i = 0; i < LCD_W * LCD_H; i++) {
        buf[2 * i] = (color >> 8) & 0xff;
        buf[2 * i + 1] = color & 0xff;
    }
    WriteDAT_DMA(buf, LCD_W * LCD_H * 2);
    free(buf);
}

void GC9307_Fill(u8 *img, u16 len)
{
    WriteCOM(0x2c);
    WriteDAT_DMA(img, len);
}

void GC9307HS177PanelSleepInMode(void)
{
    WriteCOM(0x10); //Sleep in
    lcd_delay(120); //Delay 120ms
}

void GC9307HS177PanelSleepOutMode(void)
{
    WriteCOM(0x11); //Sleep out
    lcd_delay(120);  //Delay 120ms
}

void st7735_shown_image(u8 *buff, u16 x_addr, u16 y_addr, u16 width, u16 height)
{
    u32 i = 0;
    GC9307_set_xy_addr(x_addr, y_addr, width, height);
    WriteDAT_DMA(buff, width * height * 2);
}

static void GC9307_draw(u8 *map, u32 size)//获取Ui发送出来的数据
{
    ui_send_data_ready(map, size);
}

static void GC9307_draw_1(u8 *map, u32 size)//获取camera发送出来的数据 //数据帧数以camera为基准
{
    camera_send_data_ready(map, size);
}

static void GC9307_reset(void)
{

}

static void GC9307_led_ctrl(u8 status)
{

}

void GC9307_clr(void)
{
    GC9307_clear_screen(0x00);
}

static void GC9307_reg_cfg(void)
{
    lcd_rst_pinstate(1);
    lcd_delay(50);
    lcd_rst_pinstate(0);
    lcd_delay(20);
    lcd_rst_pinstate(1);
    lcd_delay(120);


    WriteCOM(0xfe);
    WriteCOM(0xef);
    WriteCOM(0x36);
    WriteDAT_8(0x48);
    WriteCOM(0x3a);
    WriteDAT_8(0x05);
    //------------------9307

    //te
    WriteCOM(0x35);
    WriteDAT_8(0xc0);
    WriteCOM(0x44);
    WriteDAT_8(0x01);
    WriteDAT_8(0x20);

    //=============
    WriteCOM(0x20);

    WriteCOM(0x85);
    WriteDAT_8(0xc0);
    WriteCOM(0x86);
    WriteDAT_8(0x98);
    WriteCOM(0x88);
    WriteDAT_8(0x02);
    WriteCOM(0x89);
    WriteDAT_8(0x33);
    WriteCOM(0x8b);
    WriteDAT_8(0x80);
    WriteCOM(0x8d);
    WriteDAT_8(0x3b);
    WriteCOM(0x8e);
    WriteDAT_8(0x0f);



    WriteCOM(0xff);
    WriteDAT_8(0x62);

    WriteCOM(0x99);
    WriteDAT_8(0x3e);
    WriteCOM(0x9d);
    WriteDAT_8(0x4b);
    WriteCOM(0x98);
    WriteDAT_8(0x3e);
    WriteCOM(0x9c);
    WriteDAT_8(0x4b);


    WriteCOM(0xbe);
    WriteDAT_8(0x41);

    WriteCOM(0x9b);
    WriteDAT_8(0x47);
    WriteCOM(0xe1);
    WriteDAT_8(0x04);
    WriteDAT_8(0x05);

    WriteCOM(0xe8); //frame rate
    WriteDAT_8(0x11); //83fps
    WriteDAT_8(0x20);


    WriteCOM(0xec);
    WriteDAT_8(0x33);
    WriteDAT_8(0x02);
    WriteDAT_8(0xff);
    WriteCOM(0xed);
    WriteDAT_8(0x19);
    WriteDAT_8(0x09);

    WriteCOM(0xc9);
    WriteDAT_8(0x05);
    WriteCOM(0xc3);
    WriteDAT_8(0x14);
    WriteCOM(0xc4);
    WriteDAT_8(0x10);

    WriteCOM(0xF0);
    WriteDAT_8(0x87);
    WriteDAT_8(0x0a);
    WriteDAT_8(0x0a);
    WriteDAT_8(0x09);
    WriteDAT_8(0x05);
    WriteDAT_8(0x29);

    WriteCOM(0xF1);
    WriteDAT_8(0x41);
    WriteDAT_8(0x6d);
    WriteDAT_8(0x53);
    WriteDAT_8(0x2f);
    WriteDAT_8(0x31);
    WriteDAT_8(0x3a);

    WriteCOM(0xF2);
    WriteDAT_8(0x46);
    WriteDAT_8(0x05);
    WriteDAT_8(0x07);
    WriteDAT_8(0x08);
    WriteDAT_8(0x06);
    WriteDAT_8(0x2e);

    WriteCOM(0xF3);
    WriteDAT_8(0x47);
    WriteDAT_8(0xd1);
    WriteDAT_8(0x88);
    WriteDAT_8(0x30);
    WriteDAT_8(0x31);
    WriteDAT_8(0x6a);


    WriteCOM(0x11);
    lcd_delay(120);
    WriteCOM(0x29);
    WriteCOM(0x2c);

}

static void GC9307_test(void)
{
    lcd_bl_pinstate(BL_ON);
    while (1) {
        os_time_dly(100);
        GC9307_clear_screen(BLUE);
        printf("LCD_GC9307_TSET_BLUE\n");
        os_time_dly(100);
        GC9307_clear_screen(GRED);
        printf("LCD_GC9307_TSET_GRED\n");
        os_time_dly(100);
        GC9307_clear_screen(BRRED);
        printf("LCD_GC9307_TSET_BRRED\n");
        os_time_dly(100);
        GC9307_clear_screen(YELLOW);
        printf("LCD_GC9307_TSET_YELLOW\n");
    }
}

static int GC9307_init(void)
{
    lcd_bl_pinstate(BL_ON);
    GC9307_io_init();
    lcd_d("LCD_GC9307 config...\n");
    GC9307_reg_cfg();
    GC9307_set_direction(ROTATE_DEGREE_0);
    init_TE(GC9307_Fill);
    lcd_d("LCD_GC9307 config succes\n");
    return 0;
}

// *INDENT-OFF*
REGISTER_LCD_DEV(LCD_GC9307) = {
    .name              = "GC9307",
    .lcd_width         = LCD_W,
    .lcd_height        = LCD_H,
    .color_format      = LCD_COLOR_RGB565,
    .column_addr_align = 1,
    .row_addr_align    = 1,
    .LCD_Init          = GC9307_init,
    .SetDrawArea       = GC9307_SetRange,
    .LCD_Draw          = GC9307_draw,
    .LCD_Draw_1        = GC9307_draw_1,
    .LCD_DrawToDev     = GC9307_Fill,//应用层直接到设备接口层，需要做好缓冲区共用互斥，慎用！
    .LCD_ClearScreen   = GC9307_clear_screen,
    .Reset             = GC9307_reset,
    .BackLightCtrl     = GC9307_led_ctrl,
};
// *INDENT-ON*

#endif

