﻿#   软关机的soft_power_off使用示例工程说明

> 本工程展示了软关机低功耗接口使用方法。

---

## 适用平台

> 本工程适用以下芯片类型：
>
> 1. AC79系列芯片：AC790N、AC791N
>
> 杰理芯片和评估板的更多信息可在以下地址获取：[链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)
## 工程配置说明

在SDK选择[demo_hello](../../../../apps/demo/demo_hello/board)主工程文件或者主工程Makefile, 然后添加本事例工程代码。

> **步骤：电源配置-->关机唤醒IO选择配置（边缘选择等）-->关机前的IO配置-->电源和唤醒初始化-->调用power_set_soft_poweroff关机。**
>
> 1. board.c 添加电源配置和IO唤醒配置参数，并添加到初始化。
>
> ```c
> //（1）----------------------------------电源配置参数------------------------------------------//
> static const struct low_power_param power_param = {
>     .config         = TCFG_LOWPOWER_LOWPOWER_SEL,
>     .btosc_disable  = TCFG_LOWPOWER_BTOSC_DISABLE,         //进入低功耗时BTOSC是否保持(不需要需修改)
>     .vddiom_lev     = TCFG_LOWPOWER_VDDIOM_LEVEL,          //强VDDIO等级,可选：2.2V  2.4V  2.6V  2.8V  3.0V  3.2V  3.4V  3.6V
>     .vddiow_lev     = TCFG_LOWPOWER_VDDIOW_LEVEL,          //弱VDDIO等级,可选：2.1V  2.4V  2.8V  3.2V
> 	.vdc14_dcdc 	= TRUE,	   							   //打开内部1.4VDCDC，关闭则用外部
>     .vdc14_lev		= VDC14_VOL_SEL_LEVEL, 				   //VDD1.4V配置
> 	.sysvdd_lev		= SYSVDD_VOL_SEL_LEVEL,				   //内核、sdram电压配置
>     .vlvd_enable    = TRUE,                                //TRUE电压复位使能
>     .vlvd_value     = VLVD_SEL_25V,                        //低电压复位电压值
> };
> 
> //（2）-------------------------------- 唤醒IO配置 -------------------------------//
> /*******************可以使用多个IO，但是一组IO只能使用一个IO口，如PA组只能使用PA0-PA15其中一个***************/
> 
> //#define PORT_VCC33_CTRL_IO		IO_PORTA_03					//VCC33 DCDC控制引脚,该引脚控制DCDC器件输出的3.3V连接芯片HPVDD、VDDIO、VDD33
> #define PORT_WAKEUP_IO0			IO_PORTB_01					//软关机和休眠的唤醒引脚0
> #define PORT_WAKEUP_IO1			IO_PORTA_01					//软关机和休眠的唤醒引脚1
> 
> //软关机硬件唤醒IO-0相关配置:PORT_WAKEUP_IO0
> static const struct port_wakeup port0 = {
>     .edge       = FALLING_EDGE,                            //唤醒方式选择,可选：上升沿\下降沿
>     .attribute  = 0x1,                        				//保留参数
>     .iomap      = PORT_WAKEUP_IO0,                          //唤醒IO口选择
>     .low_power	= POWER_SLEEP_WAKEUP|POWER_OFF_WAKEUP,    //低功耗IO或睡眠唤醒,不需要写0
> };
> //软关机硬件唤醒IO-1相关配置:PORT_WAKEUP_IO1
> static const struct port_wakeup port1 = {
>     .edge       = FALLING_EDGE,                            //唤醒方式选择,可选：上升沿\下降沿
>     .attribute  = 0x2,                       			 //保留参数
>     .iomap      = PORT_WAKEUP_IO1,                          //唤醒IO口选择
>     .low_power	= POWER_SLEEP_WAKEUP|POWER_OFF_WAKEUP,    //低功耗IO或睡眠唤醒,不需要写0
> };
> 
> //（3）-------------------------------- 保留项配置 -------------------------------//
> //无需更改
> static const struct sub_wakeup sub_wkup = {
>     .attribute  = BLUETOOTH_RESUME,
> };
> //无需更改
> static const struct charge_wakeup charge_wkup = {
>     .attribute  = BLUETOOTH_RESUME,
> };
> //唤醒参数配置
> static const struct wakeup_param wk_param = {
>     .port[0] = &port0,//配置IO组0
>     .port[1] = &port1,//配置IO组1
>     .sub = &sub_wkup,
>     .charge = &charge_wkup,
> 	.lpres = NULL,//当使用正常工作时可长按复位功能，详情longpress例子
> };
> 
> //（4）-------------------------------- 关机前IO配置 -------------------------------//
> /*进软关机之前默认将IO口都设置成高阻状态，需要保留原来状态的请修改该函数*/
> static void board_set_soft_poweroff(void)
> {
>     u32 IO_PORT;
>     JL_PORT_FLASH_TypeDef *gpio[] = {JL_PORTA, JL_PORTB, JL_PORTC, JL_PORTD, JL_PORTE, JL_PORTF, JL_PORTG, JL_PORTH};
> 
>     for (u8 p = 0; p < 8; ++p) {
>         //flash sdram PD PE PF PG口不能进行配置,由内部完成控制
>         if (gpio[p] == JL_PORTD || gpio[p] == JL_PORTE || gpio[p] == JL_PORTF || gpio[p] == JL_PORTG) {
>             continue;
>         }
>         for (u8 i = 0; i < 16; ++i) {
>             IO_PORT = IO_PORTA_00 + p * 16 + i;
>             gpio_set_pull_up(IO_PORT, 0);
>             gpio_set_pull_down(IO_PORT, 0);
>             gpio_set_direction(IO_PORT, 1);
>             gpio_set_die(IO_PORT, 0);
>             gpio_set_dieh(IO_PORT, 0);
>             gpio_set_hd(IO_PORT, 0);
>             gpio_set_hd1(IO_PORT, 0);
>             gpio_latch_en(IO_PORT, 1);
>         }
>     }
>     //尽量不能维持高电平（注意高电平功耗大），当软关机之后需要维持电平，则一并修改isd_config.ini文件的#PORT_INPUT和#PORT_OUTPUT配置才能在复位后也能维持所需电平
>     //DCDC电源类配置，软关机后IO只能低电平（高电平功耗大），因此需要电源高电平使能，低电平禁能，比如摄像头电源或LCD背光电源和芯片的HPVDD、VDDIO、VDD33等
> #ifdef PORT_VCC33_CTRL_IO
>     gpio_latch_en(PORT_VCC33_CTRL_IO, 0);//不锁存IO
>     gpio_direction_output(PORT_VCC33_CTRL_IO, 0);//先拉低
>     gpio_set_pull_up(PORT_VCC33_CTRL_IO, 0);//关上拉
>     gpio_set_pull_down(PORT_VCC33_CTRL_IO, 1);//开下拉
>     gpio_set_direction(PORT_VCC33_CTRL_IO, 1);//设置输入
>     gpio_set_die(PORT_VCC33_CTRL_IO, 0);//关数字
>     gpio_set_dieh(PORT_VCC33_CTRL_IO, 0);//关p33相关
>     gpio_latch_en(PORT_VCC33_CTRL_IO, 1);//锁存IO状态
> #endif
> }
> 
> //（5）-------------------------------- 电源和唤醒初始化 -------------------------------//
> //电源初始化
> static void board_power_init(void)
> {
>     //DCDC电源类配置，开启后IO使能为高电平打开电源，比如摄像头电源或LCD背光电源和芯片的HPVDD、VDDIO、VDD33等
> #ifdef PORT_VCC33_CTRL_IO
> 	gpio_direction_output(PORT_VCC33_CTRL_IO, 1);
>     gpio_set_pull_up(PORT_VCC33_CTRL_IO, 1);
> 	gpio_set_pull_down(PORT_VCC33_CTRL_IO,0);
> #endif
> 
>     power_init(&power_param);//系统相关电源初始化
>     power_set_callback(TCFG_LOWPOWER_LOWPOWER_SEL, NULL, NULL, board_set_soft_poweroff);//注册关机前回调函数
>     power_keep_state(POWER_KEEP_RESET);//0, POWER_KEEP_DACVDD | POWER_KEEP_RTC | POWER_KEEP_RESET ,//关机或睡眠后需要保持的功能
> 
> #ifdef CONFIG_OSC_RTC_ENABLE
>     power_keep_state(POWER_KEEP_RTC);//当有RTC时，关机或睡眠后RTC继续维持走时
> #endif
> 
>     power_wakeup_init(&wk_param);//唤醒参数初始化
> }
> void board_init()
> {
> #ifdef CONFIG_OSC_RTC_ENABLE
>     rtc_early_init(1);//使用外接RTC晶振打开CONFIG_OSC_RTC_ENABLE宏，则RTC时间为晶振计时计数
> #else
>     rtc_early_init(0);//不使用外接RTC晶振，则RTC时间为软件定时计数
> #endif
> 	board_power_init();
> }
> 
> ```
>
> 2. app_config.h 添加电源配置参数（步骤1的board.c使用）
>
> ```c
> //*********************************************************************************//
> //                                  低功耗配置                                     //
> //*********************************************************************************//
> #define TCFG_LOWPOWER_BTOSC_DISABLE			0
> #define TCFG_LOWPOWER_LOWPOWER_SEL			0//SLEEP_EN  //该宏在睡眠低功耗才用到，此处设置为0
> #define TCFG_LOWPOWER_VDDIOM_LEVEL			VDDIOM_VOL_32V //正常工作的内部vddio电压值，一般使用外部3.3V，内部设置需比外部3.3V小
> #define TCFG_LOWPOWER_VDDIOW_LEVEL			VDDIOW_VOL_21V //软关机或睡眠的内部vddio最低电压值
> #define VDC14_VOL_SEL_LEVEL					VDC14_VOL_SEL_140V //内部的1.4V默认1.4V
> #define SYSVDD_VOL_SEL_LEVEL				SYSVDD_VOL_SEL_126V //系统内核电压，默认1.26V
> 
> ```
>
> 3. 添加main.c测试软关机代码
>
> ```c
> #include "app_config.h"
> #include "system/includes.h"
> #include "asm/power_interface.h"
> #include "device/device.h"
> 
> static void poweroff_test(void)
> {
> 	void power_set_soft_poweroff(void);
> 	os_time_dly(200);
> 	printf("---> poweroff_test \n\n");
> 	power_set_soft_poweroff();
> }
> static int c_main(void)
> {
>     os_task_create(poweroff_test, NULL, 12, 1000, 0, "poweroff_test");
>     return 0;
> }
> late_initcall(c_main);
> 
> ```
>
> 2、使用说明
>
> 1）软关机使用函数为：power_set_soft_poweroff(); 函数在power_interface.h头文件中申明。
>
> 本次使用为IO_PORTB_01的PB1引脚，硬件外挂上拉电阻，选择下降沿触发唤醒。
>
> IO唤醒配置参考步骤1的board.c的struct port_wakeup port0相关配置，可选择边缘、IO口等。
## 操作说明

> 1. 使用串口线连接打印
> 2. 编译工程，烧录镜像，复位启动
> 3. 系统启动后，2秒钟后进软关机，查看对应打印，并测试PB1接GND可唤醒系统（PB1需要先加上拉电阻）。
>
> JIELI SDK的编译、烧写等操作方式的说明可在以下文档获取：[文档](../../../../doc/stuff/usb updater.pdf)
---

## 常见问题

> * 1）当需要按键触发关机时，在调用power_set_soft_poweroff()函数前最好先等待按键释放完全再调用关机函数。
> * 2）由于内部上下拉电阻精度不高（10K左右），因此IO唤醒的硬件IO需要外接上下拉电阻，具体看看需求接上拉还是下拉，系统进软关机后会把内部的上下拉都关闭，因此需要硬件上芯片外部上下拉电阻维持IO状态。

## 参考文档

> * N/A