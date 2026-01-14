#include "system/includes.h"
#include "asm/port_waked_up.h"
#include "typedef.h"
#include "os/os_api.h"
#include "asm/iic.h"
#include "device/iic.h"
#include "device/device.h"
#include "app_config.h"
#include "system/includes.h"
#include "gpio.h"
#include "lcd_drive.h"
#include "ui_api.h"

#if TCFG_TOUCH_GT911_ENABLE
#define TEST 1

//I2C读写命令
#define GT911_WRCMD 			0X28     	//写命令
#define GT911_RDCMD 			0X29		//读命令
/*#define GT911_WRCMD 			0Xbb     	//写命令*/
/*#define GT911_RDCMD 			0Xba		//读命令*/

//GT911 部分寄存器定义
#define GT_CTRL_REG 			0X8040   	//GT911控制寄存器
#define GT_CFGS_REG 			0X8047   	//GT911配置起始地址寄存器
#define GT_CHECK_REG		 	0X80FF   	//GT911校验和寄存器

#define GT_PID_REG0				0X8140   	//GT911产品ID寄存器
#define GT_PID_REG1				0X8141   	//GT911产品ID寄存器
#define GT_PID_REG2				0X8142   	//GT911产品ID寄存器

#define GT_GSTID_REG 	        0X814E   	//GT911当前检测到的触摸情况
#define GT_TP1_REG 		        0X8150  	//第一个触摸点数据地址 //每个点4四个地址控制GT_TP1_X_L_REG
#define GT_TP2_REG 				0X8158		//第二个触摸点数据地址
#define GT_TP3_REG 				0X8160		//第三个触摸点数据地址
#define GT_TP4_REG 				0X8168		//第四个触摸点数据地址
#define GT_TP5_REG 				0X8170		//第五个触摸点数据地址

static void *iic = NULL;

static unsigned char wrGT911Reg(unsigned int regID, unsigned char regDat)
{
    u8 ret = 1;

    dev_ioctl(iic, IIC_IOCTL_START, 0);
    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, GT911_WRCMD)) {
        ret = 0;
        printf("iic write err!!! line : %d \n", __LINE__);
        goto exit;
    }
    delay(1000);
    if (dev_ioctl(iic, IIC_IOCTL_TX, regID >> 8)) {
        ret = 0;
        printf("iic write err!!! line : %d \n", __LINE__);
        goto exit;
    }
    delay(1000);
    if (dev_ioctl(iic, IIC_IOCTL_TX, regID & 0xff)) {
        ret = 0;
        printf("iic write err!!! line : %d \n", __LINE__);
        goto exit;
    }
    delay(1000);
    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_STOP_BIT, regDat)) {
        ret = 0;
        printf("iic write err!!! line : %d \n", __LINE__);
        goto exit;
    }
    delay(1000);

exit:
    dev_ioctl(iic, IIC_IOCTL_STOP, 0);
    return ret;
}

static unsigned char rdGT911Reg(u16 regID, u8 *regDat)
{
    u8 ret = 1;
    dev_ioctl(iic, IIC_IOCTL_START, 0);
    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, GT911_WRCMD)) {
        ret = 0;
        printf("iic write err!!! line : %d \n", __LINE__);
        goto exit;
    }
    delay(100);
    if (dev_ioctl(iic, IIC_IOCTL_TX, regID >> 8)) {
        ret = 0;;
        printf("iic write err!!! line : %d \n", __LINE__);
        goto exit;
    }
    delay(100);
    if (dev_ioctl(iic, IIC_IOCTL_TX, regID & 0xff)) {
        ret = 0;;
        printf("iic write err!!! line : %d \n", __LINE__);
        goto exit;
    }
    delay(100);
    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, GT911_RDCMD)) {
        ret = 0;
        goto exit;
    }
    delay(100);
    dev_ioctl(iic, IIC_IOCTL_RX_WITH_STOP_BIT, (u32)regDat);
exit:
    dev_ioctl(iic, IIC_IOCTL_STOP, 0);
    return ret;
}

int get_GT911_pid(void)
{
    u8 temp = 0;
    rdGT911Reg(GT_PID_REG0, &temp);
    if (temp != 57) { // 9
        return 1;
    }
    rdGT911Reg(GT_PID_REG1, &temp);
    if (temp != 49) { // 1
        return 1;
    }
    rdGT911Reg(GT_PID_REG2, &temp);
    if (temp != 49) { // 1
        return 1;
    }
    printf(">>>>>>>>>>>hell touch GT911<<<<<<<<<<<");
    return 0;
}

static void get_GT911_xy(u16 addr, u16 *x, u16 *y)
{
    u8 buf[4];
    for (u8 i = 0; i < 4; i++) {
        rdGT911Reg((addr + i), &buf[i]);	//读取XY坐标值
    }
    *x = buf[0] + buf[1] * 255;
    *y = buf[2] + buf[3] * 255;
}

static void GT911_test(void)
{
    u8 status = 0;
    u16 x = 0;
    u16 y = 0;
    rdGT911Reg(GT_GSTID_REG, &status);	//读取触摸点的状态   // BIT7表示有数据 ,bit0-3 表示触摸点个数
    if (status & 0x80) { //有触摸值
        if (status > 127) { //有一个触摸点按下
            get_GT911_xy(GT_TP1_REG, &x, &y);
            printf(">>>>>>>>>>>[pint 1]x = %d, y = %d", x, y);
        }
        if (status > 128) { //有两个触摸点按下
            get_GT911_xy(GT_TP2_REG, &x, &y);
            printf(">>>>>>>>>>>[pint 2]x = %d, y = %d", x, y);
        }
        if (status > 129) { //有三个触摸点按下
            get_GT911_xy(GT_TP3_REG, &x, &y);
            printf(">>>>>>>>>>>[pint 3]x = %d, y = %d", x, y);
        }
        if (status > 130) { //有四个触摸点按下
            get_GT911_xy(GT_TP4_REG, &x, &y);
            printf(">>>>>>>>>>>[pint 4]x = %d, y = %d", x, y);
        }
        if (status > 131) { //有五个触摸点按下
            get_GT911_xy(GT_TP5_REG, &x, &y);
            printf(">>>>>>>>>>>[pint 5]x = %d, y = %d", x, y);
        }
        status = 0;
        wrGT911Reg(GT_GSTID_REG, status); //清标志
    }
}

static void GT911_interrupt(void)
{
#if TEST
    GT911_test();
#else
    u8 status = 0;
    u16 x = 0;
    u16 y = 0;
    rdGT911Reg(GT_GSTID_REG, &status);	//读取触摸点的状态   // BIT7表示有数据 ,bit0-3 表示触摸点个数
    if (status & 0x80) { //有触摸值
        if (status > 127) { //有一个触摸点按下
            get_GT911_xy(GT_TP1_REG, &x, &y);
            printf(">>>>>>>>>>>[pint 1]x = %d, y = %d", x, y);
        }
        if (status > 128) { //有两个触摸点按下
            get_GT911_xy(GT_TP2_REG, &x, &y);
            printf(">>>>>>>>>>>[pint 2]x = %d, y = %d", x, y);
        }
        if (status > 129) { //有三个触摸点按下
            get_GT911_xy(GT_TP3_REG, &x, &y);
            printf(">>>>>>>>>>>[pint 3]x = %d, y = %d", x, y);
        }
        if (status > 130) { //有四个触摸点按下
            get_GT911_xy(GT_TP4_REG, &x, &y);
            printf(">>>>>>>>>>>[pint 4]x = %d, y = %d", x, y);
        }
        if (status > 131) { //有五个触摸点按下
            get_GT911_xy(GT_TP5_REG, &x, &y);
            printf(">>>>>>>>>>>[pint 5]x = %d, y = %d", x, y);
        }

        struct ui_touch_event t;

        tp_last_staus = ELM_EVENT_TOUCH_UP;
        t.event = tp_last_staus;
        t.x = x;
        t.y = y;
        ui_touch_msg_post(&t);
        printf("event = %d.\n", tp_last_staus);
        status = 0;
        wrGT911Reg(GT_GSTID_REG, status); //清标志
    }
#endif
}

static int GT911_init(void)
{
    iic = dev_open("iic0", NULL);

    if (get_GT911_pid()) {
        printf("[err]>>>>>GT911 err!!!");
        return 1;
    }

    extern const struct ui_devices_cfg ui_cfg_data;
    static const struct ui_lcd_platform_data *pdata;
    pdata = (struct ui_lcd_platform_data *)ui_cfg_data.private_data;

    gpio_direction_output(pdata->touch_reset_pin, 0);
    os_time_dly(1);
    gpio_direction_output(pdata->touch_int_pin, 1);
    os_time_dly(1);
    gpio_direction_output(pdata->touch_reset_pin, 1);
    os_time_dly(1);
    gpio_direction_output(pdata->touch_int_pin, 0);
    os_time_dly(10);

    port_wakeup_reg(EVENT_IO_1, IO_PORTH_07, EDGE_NEGATIVE, GT911_interrupt);

    return 0;
}

static void my_touch_test_task(void *priv)
{
    /*=====TE中断配置=====*/
    GT911_init();
}

static int GT911_task_init(void)
{
    return thread_fork("my_touch_test_task", 10, 1024, 0, NULL, my_touch_test_task, NULL);
}
late_initcall(GT911_task_init);
#endif








