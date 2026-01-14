

#include "system/includes.h"
#include "typedef.h"
#include "asm/pap.h"
#include "lcd_drive.h"
#include "lcd_te_driver.h"
#include "lcd_config.h"
#include "gpio.h"

/*  ili9488驱动说明 该驱动测试时使用的 wl80 79系列
 *  由于该IC推屏能力不够强 推屏的帧数较低 大概在25帧左右
 *  在推屏过程中需要使用TE屏幕帧中断 不然会有条纹
 *  由于ili9488横屏配置无法调出没有条纹的配置
 *  所有只能使用竖屏加RGB旋转来实现UI横屏显示
 */

/* //pap的这个三个配置如下 在板级文件中进行修改
    .timing_setup 	= 0,					//具体看pap.h
    .timing_hold  	= 0,					//具体看pap.h
    .timing_width 	= 1,					//具体看pap.h
*/

#if TCFG_LCD_ILI9488_ENABLE

#define ROTATE_DEGREE_0  	0
#define ROTATE_DEGREE_180       1

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

#define READ_ID 	0x04
#define REGFLAG_DELAY 0x45

#define DMA_DSPLY	1 //DMA operation

static u8 ui_data_ready = 0;

/*调用lcd_TE_driver完成数据发送*/
extern void send_date_ready();
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

static u32 Read_ID(u8 index)
{
    u32 id = 0;
    WriteCOM(index);
    /* id = ReadDAT_16() & 0xff00; */
    id = ReadDAT_16();
    id |= ReadDAT_16() << 16;
    return id;
}

void ili9488_SetRange(u16 xs, u16 xe, u16 ys, u16 ye)
{
    /******UI每次发送数据都会调用开窗告诉屏幕要刷新那个区域***********/
    get_lcd_ui_x_y(xs, xe, ys, ye);
}
void ili9488_SetRange_1(u16 xs, u16 xe, u16 ys, u16 ye)
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

void ili9488_clear_screen(u32 color)
{
    lcd_interface_non_block_wait();
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

void ili9488_Fill(u8 *img, u16 len)
{
    lcd_interface_non_block_wait();
    WriteCOM(0x2c);
    WriteDAT_DMA(img, len);
}

void ili9488_SleepInMode(void)
{
    WriteCOM(0x10); //Sleep in
    lcd_delay(120); //Delay 120ms
}

void ili9488_SleepOutMode(void)
{
    WriteCOM(0x11); //Sleep out
    lcd_delay(120);  //Delay 120ms
}

void st7789_shown_image(u8 *buff, u16 x_addr, u16 y_addr, u16 width, u16 height)
{
    lcd_interface_non_block_wait();
    ili9488_SetRange(x_addr, y_addr, width, height);
    WriteDAT_DMA(buff, width * height * 2);
}

static void ili9488_set_direction(u8 dir)
{
    WriteCOM(0x36);    //扫描方向控制

    if (dir == ROTATE_DEGREE_0) { //正向
#if HORIZONTAL_SCREEN
        WriteDAT_8(0x40);
#else
        WriteDAT_8(0x00);
#endif
        ili9488_SetRange(0, LCD_W - 1, 0, LCD_H - 1);
    } else if (dir == ROTATE_DEGREE_180) { //翻转180
#if HORIZONTAL_SCREEN
        WriteDAT_8(0xc0);
#else
        WriteDAT_8(0x80);
#endif
    }
}

static void ili9488_draw(u8 *map, u32 size)//获取Ui发送出来的数据
{
    ui_send_data_ready(map, size);
}

static void ili9488_draw_1(u8 *map, u32 size)//获取camera发送出来的数据 //数据帧数以camera为基准
{
    camera_send_data_ready(map, size);
}

static void ili9488_reset(void)
{
    printf("reset \n");
    lcd_rst_pinstate(1);
    lcd_rs_pinstate(1);
    lcd_cs_pinstate(1);

    lcd_rst_pinstate(1);
    lcd_delay(60);
    lcd_rst_pinstate(0);
    lcd_delay(10);
    lcd_rst_pinstate(1);
    lcd_delay(100);
}

typedef struct {
    u8 cmd;
    u8 cnt;
    u8 dat[128];
} InitCode;

static const InitCode code1[] = {
    {0XF7, 4, {0xA9, 0x51, 0x2C, 0x82}},
    {0xC0, 2, {0x11, 0x09}},
    {0xC1, 1, {0x41}},
    {0XC5, 3, {0x00, 0x0A, 0x80}},
    {0xB1, 2, {0xB0, 0x11}},
    {0xB4, 1, {0x02}},
    {0xB6, 2, {0x02, 0x42}},
    {0xB7, 1, {0xc6}},
    {0xBE, 2, {0x00, 0x04}},
    {0xE9, 1, {0x00}},
    {0x36, 1, {0x48}},   //可以控制旋转/flip/rota
    {0x3a, 1, {0x55}},
    {0xE0, 15, {0x00, 0x07, 0x10, 0x09, 0x17, 0x0B, 0x41, 0x89, 0x4B, 0x0A, 0x0C, 0x0E, 0x18, 0x1B, 0x0F}},
    {0XE1, 15, {0x00, 0x17, 0x1A, 0x04, 0x0E, 0x06, 0x2F, 0x45, 0x43, 0x02, 0x0A, 0x09, 0x32, 0x36, 0x0F}},
    {0x11, 0},//Sleep out
    {REGFLAG_DELAY, 200},
    {0x29, 0},//开显示
};

static void ili9488_init_code(const InitCode *code, u8 cnt)
{
    for (u8 i = 0; i < cnt; i++) {
        if (code[i].cmd == REGFLAG_DELAY) {
            lcd_delay(code[i].cnt);
        } else {
            WriteCOM(code[i].cmd);
            for (u8 j = 0; j < code[i].cnt; j++) {
                WriteDAT_8(code[i].dat[j]);
            }
        }
    }
}

static void ili9488_led_ctrl(u8 status)
{
    //背光控制以及放在//lcd_te_driver.c 优化开机显示
    /*lcd_bl_pinstate(status);*/
}

void ili9488_test(void)
{
    lcd_bl_pinstate(BL_ON);
    while (1) {
        os_time_dly(100);
        ili9488_clear_screen(WHITE);
        printf("LCD_ili9488_TSET_WHITE\n");
        os_time_dly(100);
        ili9488_clear_screen(BLACK);
        printf("LCD_ili9488_TSET_BLACK\n");
        os_time_dly(100);
        ili9488_clear_screen(RED);
        printf("LCD_ili9488_TSET_RED\n");
        os_time_dly(100);
        ili9488_clear_screen(YELLOW);
        printf("LCD_ili9488_TSET_YELLOW\n");
    }
}

static int ili9488_init(void)
{
    printf("LCD_ili9488 init_start\n");
    ili9488_reset();
    ili9488_init_code(code1, sizeof(code1) / sizeof(code1[0]));
    /*ili9488_set_direction(ROTATE_DEGREE_0);*/
    init_TE(ili9488_Fill);
    /*ili9488_test();*/
    printf("LCD_ili9488 config succes\n");
    return 0;
}


REGISTER_LCD_DEV(LCD_ili9488) = {
    .name              = "ili9488",
    .lcd_width         = LCD_W,
    .lcd_height        = LCD_H,
    .color_format      = LCD_COLOR_RGB565,
    .column_addr_align = 1,
    .row_addr_align    = 1,
    .LCD_Init          = ili9488_init,
    .SetDrawArea       = ili9488_SetRange,
    .LCD_Draw          = ili9488_draw,
    .LCD_Draw_1        = ili9488_draw_1,
    .LCD_DrawToDev     = ili9488_Fill,//应用层直接到设备接口层，需要做好缓冲区共用互斥，慎用！
    .LCD_ClearScreen   = ili9488_clear_screen,
    .Reset             = ili9488_reset,
    .BackLightCtrl     = ili9488_led_ctrl,
};

#endif


