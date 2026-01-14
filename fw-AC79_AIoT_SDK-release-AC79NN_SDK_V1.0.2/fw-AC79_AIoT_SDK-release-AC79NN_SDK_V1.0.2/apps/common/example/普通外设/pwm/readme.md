#   PWM使用示例工程说明

> 本工程展示了PWM模块接口使用方法。

---

## 适用平台

> 本工程适用以下芯片类型：
>
> 1. AC79系列芯片：AC790N、AC791N
>
> 杰理芯片和评估板的更多信息可在以下地址获取：[链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)
## 工程配置说明

在SDK选择[demo_hello](../../../../apps/demo_hello/board)主工程文件或者主工程Makefile, 然后添加本事例工程代码。

> 1. board.c 添加PWM参数和PWM设备（同时最多可以定义3个PWM设备，设备列表的设备名称可以自定义）
>
> ```c
> //----------------------------------定义PWM设备的参数------------------------------------------//
> //例如：PWM0
> PWM_PLATFORM_DATA_BEGIN(pwm_data0)
> 	.port	= 0,
>     .pwm_ch = PWMCH0_H | PWMCH0_L,//初始化可选多通道,如:PWMCH0_H | PWMCH0_L | PWMCH1_H ... | PWMCH7_H | PWMCH7_L | PWM_TIMER2_OPCH2 | PWM_TIMER3_OPCH3 ,
>     .freq   = 1000,//频率
>     .duty   = 50,//占空比
>     .point_bit = 0,//根据point_bit值调节占空比小数点精度位: 0<freq<=4K,point_bit=2;4K<freq<=40K,point_bit=1; freq>40K,point_bit=0;
> PWM_PLATFORM_DATA_END()
> 
> //例如：PWM1
> PWM_PLATFORM_DATA_BEGIN(pwm_data1)
> 	.port	= IO_PORTA_03,//选择定时器的TIMER PWM任意IO，pwm_ch加上PWM_TIMER3_OPCH3或PWM_TIMER2_OPCH2有效,只支持2个PWM,占用output_channel2/3，其他外设使用output_channel需留意
>     .pwm_ch = PWM_TIMER2_OPCH2,//初始化可选多通道,如:PWMCH0_H | PWMCH0_L | PWMCH1_H ... | PWMCH7_H | PWMCH7_L | PWM_TIMER2_OPCH2 | PWM_TIMER3_OPCH3 ,
>     .freq   = 1000,//频率
>     .duty   = 50,//占空比
>     .point_bit = 0,//根据point_bit值调节占空比小数点精度位: 0<freq<=4K,point_bit=2;4K<freq<=40K,point_bit=1; freq>40K,point_bit=0;
> PWM_PLATFORM_DATA_END()
> 
> //----------------------------------设备列表添加PWM设备------------------------------------------//    
> REGISTER_DEVICES(device_table) = {
> 	{"uart2", &uart_dev_ops, (void *)&uart2_data },
> 	{"rtc", &rtc_dev_ops, NULL},
> 	{ "pwm0",   &pwm_dev_ops,  (void *)&pwm_data0},//添加pwm0子设备,"pwm0"名称可自定义
> 	{ "pwm1",   &pwm_dev_ops,  (void *)&pwm_data1},//添加pwm1子设备,"pwm1"名称可自定义
> };
> 
> ```
>
> **2、PWM硬件IO说名**
>
> 1）MCPWM
>
> MCPWM是有多通道的PWM硬件模块，总共8组PWM通道，每组PWM包含2通道（即H和L通道），MCPWM包含死区时间设置、反向电平设置等，专用与驱动电机。
>
> MCPWM为：
>
> PWMCH0_H、PWMCH0_L：硬件IO为PA3、PA4
>
> PWMCH1_H、PWMCH1_L：硬件IO为PA7、PA8
>
> PWMCH2_H、PWMCH2_L：硬件IO为PC7、PC8
>
> PWMCH3_H、PWMCH3_L：硬件IO为PH0、PH1
>
> PWMCH4_H、PWMCH4_L：硬件IO为PC0、PC2
>
> PWMCH5_H、PWMCH5_L：硬件IO为PH3、PH7
>
> PWMCH6_H、PWMCH6_L：硬件IO为PB2、PB3
>
> PWMCH7_H、PWMCH7_L：硬件IO为PB6、PB7
>
> **2）定时器2/3的PWM**
>
> 定时器的PWM可以映射任意IO。
>
> **使用说明**：
>
> A）例如上述程序代码中的.pwm_ch参数，当使用对应的MCPWM IO时配置：
>
> .port = 0,//这个参数为定时器PWM，不是MCPWM，使用到定时器PWM映射任意IO才指定为IO
>
>  .pwm_ch= PWMCH0_H | PWMCH0_L,//例如使用PA3和PA4，
>
> .pwm_ch为需要打开的PWM通道，可以直接用或打开多通道，例如PWMCH1_H | PWMCH2_H | PWMCH3_H | PWMCH3_L
>
> B）.duty和.point_bit 参数
>
> 当.point_bit = 0，则表明.duty的占空比参数是整数，即占空比为0%-100%，最高频率25MHz左右（最高位lsb_clk频率的一半）。
>
> 当.point_bit = 1，则表明.duty的占空比参数可以包含1位小数点，例如.duty=20.5，即占空比为0.0%-100.0%，最高频率2.4MHz左右
>
> 当.point_bit = 2，则表明.duty的占空比参数可以包含2位小数点，例如.duty=20.55，即占空比为0.00%-100.00%，最高频率200KHz左右
>
> C）当使用的PWM硬件IO不在MCPWM的所有通道时，可以使用定时器PWM映射到任意IO，例如上述程序的硬件IO为 IO_PORTA_03
>
> .port	= IO_PORTA_03,//选择定时器的TIMER PWM任意IO，对使用MCPWM无效
>
> .pwm_ch = PWM_TIMER2_OPCH2,//pwm_ch加上PWM_TIMER3_OPCH3或PWM_TIMER2_OPCH2有效,只支持2个PWM,
>
> **说明：**当使用任意IO映射PWM时，可以用定时器2或3，即.pwm_ch = PWM_TIMER2_OPCH2,或.pwm_ch = PWM_TIMER3_OPCH3，定时器只需要用1个映射即可，当使用PWM_TIMER2_OPCH2则在其他程序不能使用定时器2，同理定时器3在使用PWM时其他程序也不能使用。
>
> 另外：.pwm_ch参数可以或上PWMCHx_H/L初始化时同时打开PMPWM的指定通道和定时器PWM。
>
> D）.deathtime为死区设置时间（系统时钟的 deathtime+1倍），当使用PWMCHx_H/L才有效，并且死区时间在PWM初始化是没有进行设置，用户需要则在dev_open打开PWM之后，再进行设置，例如程序：
> 	pwm.deathtime = 6;//最大值31，死区时间为系统时钟的(deathtime+1)倍，使用PWMCHx_H/L有效。
>     dev_ioctl(pwm_dev_handl, PWM_SET_DEATH_TIME, (u32)&pwm);//PWM死区时间设置
>
> **注意事项**：
>
> A）当使用MCPWM硬件时（PWMCHx_H 或 PWMCHx_L）， 每组的H和L 2个同时使用则：频率一定相同，占空比可不同。
>
> 例如：同通道的PWMCH0_H和PWMCH0_L频率一定相同，但是不同通道的PWMCH0_H和PWMCH1_H频率可不同，占空比都可以任意设置0.00%-100.00%
>
> 2）定时器PWM
>
> .port参数是选择定时器的TIMER PWM任意IO，对使用MCPWM时该参数无效。
>
> 当定时器2（PWM_TIMER2_OPCH2）的PWM指定IO为PH6、当定时器3（PWM_TIMER3_OPCH3）的PWM指定IO为PC10时，不会占用ouputchannel（ouputchannel为一个可以映射任何功能到某个任意IO的硬件模块），其他IO时定时器2占用ouputchannel_2，定时器3占用ouputchannel_3。
>
> 3）.pwm_ch参数不能同时使用2个定时器映射任意IO，当需要两个PWM任意IO，在设备列表多加一个PWM子设备即可。
>
> 4）point_bit 占空比小数点参数会影响PWM的频率，频率过高，占空比小数点比较多时，可能会导致频率不对或者占空比无效。
>
> 5）在打开PWM设备之后更改通道的频率只能选择一组通道（H和L为一组通道），比如只能选择.pwm_ch = PWMCH0_H | PWMCH0_L，或者.pwm_ch = PWM_TIMER2_OPCH2，PWM通道最多支持同一个通道的H和L设置，不能进行多通道的设置。
>
> 2、测试代码
>
> ```c
> #include "app_config.h"
> #include "system/includes.h"
> #include "asm/pwm.h"
> #include "device/device.h"
> 
> /*********************************PWM设备例子****************************************************
>   注意：初始化之后，只支持单通道选择
>   PWM通过dev_write写，返回值为成功0，失败非0；
>   PWM通过dev_read读，返回值为通道的占空比值(0-100%)；
>   PWM通过dev_ioctl控制，形参arg为PWM通道；
> ************************************************************************************************/
> static void pwm_test(void)
> {
>     int ret;
>     u32 duty;
>     u32 channel;
> 	void *pwm_dev_handl = NULL;
>     struct pwm_platform_data pwm = {0};
> 
>     /*1.open ，open之后的PWM通道最多支持同一个通道的H和L设置，不能进行多通道的设置*/
>     pwm_dev_handl = dev_open("pwm0", &pwm);//打开PWM0设备，传参可以获取board.c配置的参数
>     if (!pwm_dev_handl) {
>         printf("open pwm err !!!\n\n");
>         return ;
>     }
>     printf("pwm : ch=0x%x,duty=%2f%%,pbit=%d,freq=%dHz\n", pwm.pwm_ch, pwm.duty, pwm.point_bit, pwm.freq);
>     os_time_dly(200);
> 
>     /*open PWM设备之后就会初始化PWM，PWM相关参数为board.c配置，在不需要更改参数时，只需要open就行，不需要进行以下操作*/
> 
> #if 1
>     /*2.write and read 配置占空比*/
>     pwm.pwm_ch = PWMCH0_H | PWMCH0_L;
>     pwm.duty = 80;
>     dev_write(pwm_dev_handl, (void *)&pwm, 0);
>     ret = dev_read(pwm_dev_handl, (void *)&pwm, 0);//dev_read函数返回值为占空比（不带小数点）0-100
>     printf("pwm0 read duty : %d \n", ret);
>     os_time_dly(200);
> 
>     /*3.ioctl控制PWM暂停、运行、正反向,调用1次ioctl只支持1组通道PWMCH_H/L控制*/
>     printf("----pwm0 ioctl-------\n\n");
>     dev_ioctl(pwm_dev_handl, PWM_STOP, (u32)&pwm);//PWM停止
>     os_time_dly(200);
>     dev_ioctl(pwm_dev_handl, PWM_RUN, (u32)&pwm);//PWM运行
>     os_time_dly(200);
>     dev_ioctl(pwm_dev_handl, PWM_REVDIRC, (u32)&pwm);//PWM正向 PWMCHx_H/L才能支持
>     os_time_dly(200);
>     dev_ioctl(pwm_dev_handl, PWM_FORDIRC, (u32)&pwm);//PWM反向 PWMCHx_H/L才能支持
> 
> 	pwm.deathtime = 6;//最大值31 死区时间为系统时钟的(deathtime+1)倍,使用PWMCHx_H/L有效
>     dev_ioctl(pwm_dev_handl, PWM_SET_DEATH_TIME, (u32)&pwm);//PWM死区时间设置
>  
>     os_time_dly(200);
>     printf("----pwm0 set freq-------\n\n");
>     /*4.ioctl配置频率和占空比*/
>     pwm.pwm_ch = PWMCH0_H | PWMCH0_L;
>     pwm.freq = 2000;
>     pwm.duty = 20;
>     dev_ioctl(pwm_dev_handl, PWM_SET_FREQ, (u32)&pwm);//设置频率
>     os_time_dly(200);
> 
>     pwm.duty = 50;
>     dev_ioctl(pwm_dev_handl, PWM_SET_DUTY, (u32)&pwm);//设置占空比
>     os_time_dly(200);
> 
>     pwm.duty = 80;
>     dev_ioctl(pwm_dev_handl, PWM_SET_DUTY, (u32)&pwm);//设置占空比
>     os_time_dly(200);
> 
>     /*5.中途可以添加TIMER2 PWM 任意IO,添加通道后关闭前必须删除 */
>     printf("----timer add channel-------\n\n");
>     pwm.pwm_ch = PWM_TIMER2_OPCH2;
>     pwm.port = IO_PORTA_07;
>     pwm.duty = 10;
>     pwm.freq = 1500;
>     dev_ioctl(pwm_dev_handl, PWM_ADD_CHANNEL, (u32)&pwm);//中途添加通道，可以是PWMCHx_H/L和PWM_TIMER2_OPCH2或PWM_TIMER3_OPCH3
> 
>     pwm.duty = 80;
>     dev_write(pwm_dev_handl, (void *)&pwm, 0);//dev_write也可以设置占空比
>     ret = dev_read(pwm_dev_handl, (void *)&pwm, 0);//读取占空比
>     printf("pwm0 read duty : %d \n", ret);
> 
>     os_time_dly(200);
> 
> 
>     printf("----timer ioctl pwm.pwm_ch = 0x%x-------\n\n", pwm.pwm_ch);
>     /*6.ioctl控制PWM暂停、运行、正反向,调用1次ioctl只支持1组通道PWMCH_H/L控制*/
>     os_time_dly(300);
>     dev_ioctl(pwm_dev_handl, PWM_STOP, (u32)&pwm);//PWM停止
> 
>     os_time_dly(200);
>     dev_ioctl(pwm_dev_handl, PWM_RUN, (u32)&pwm);//PWM运行
>     os_time_dly(200);
> 
>     pwm.freq = 2000;
>     pwm.duty = 20;
>     printf("----timer set freq-------\n\n");
>     dev_ioctl(pwm_dev_handl, PWM_SET_FREQ, (u32)&pwm);//设置频率
>     os_time_dly(200);
>     pwm.duty = 50;
>     dev_ioctl(pwm_dev_handl, PWM_SET_DUTY, (u32)&pwm);//设置占空比
>     os_time_dly(200);
>     pwm.duty = 80;
>     dev_ioctl(pwm_dev_handl, PWM_SET_DUTY, (u32)&pwm);//设置占空比
>     os_time_dly(200);
> 
>     /*7.关闭前把添加通道删除,*/
>     dev_ioctl(pwm_dev_handl, PWM_REMOV_CHANNEL, (u32)&pwm);
> #endif
>     dev_close(pwm_dev_handl);
>     printf("pwm test end\n\n");
> 
>     while (1) {
>         os_time_dly(2);
>     }
> }
> static int c_main(void)
> {
>     os_task_create(pwm_test, NULL, 12, 1000, 0, "pwm_test");
>     return 0;
> }
> late_initcall(c_main);
> 
> ```
>
> 
## 操作说明

> 1. 使用串口线连接打印
> 2. 编译工程，烧录镜像，复位启动
> 3. 系统启动后，查看对应的PWM输出
>
> JIELI SDK的编译、烧写等操作方式的说明可在以下文档获取：[文档](../../../../doc/stuff/usb updater.pdf)
---

## 常见问题

> * N/A

## 参考文档

> * N/A