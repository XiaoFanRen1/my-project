#   PWM_LED使用示例工程说明

> 本工程展示了PWM模块接口使用方法。

---

## 适用平台

> 本工程适用以下芯片类型：
>
> 1. AC79系列芯片：AC790N、AC791N
>
> 杰理芯片和评估板的更多信息可在以下地址获取：[链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)
## 工程配置说明

在SDK选择[AC790N_WIFI_CAMERA](../../../../apps/wifi_camera/board/wl80)主工程文件或者主工程Makefile, 然后添加本事例工程代码。

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
> 2、测试代码
>
> ```c
>#include "app_config.h"
> #include "system/includes.h"
>#include "asm/pwm.h"
> #include "device/device.h"
>
> /*********************************PWM_LED设备例子****************************************************
>  注意：初始化之后，只支持单通道选择
>   PWM通过dev_write写，返回值为成功0，失败非0；
>  PWM通过dev_read读，返回值为通道的占空比值(0-100%)；
>   PWM通过dev_ioctl控制，形参arg为PWM通道；
>************************************************************************************************/
> static void pwm_led_test(void)
>{
>     int ret;
>    u32 duty;
>     u32 channel;
>	void *pwm_dev_handl = NULL;
>     struct pwm_platform_data pwm = {0};
>
>     /*1.open ，open之后的PWM通道最多支持同一个通道的H和L设置，不能进行多通道的设置*/
>    pwm_dev_handl = dev_open("pwm1", &pwm);//打开PWM1设备，传参可以获取board.c配置的参数
>     if (!pwm_dev_handl) {
>        printf("open pwm err !!!\n\n");
>         return ;
>    }
>     printf("pwm : ch=0x%x,duty=%2f%%,pbit=%d,freq=%dHz\n", pwm.pwm_ch, pwm.duty, pwm.point_bit, pwm.freq);
>    os_time_dly(200);
> /*open PWM设备之后就会初始化PWM，PWM相关参数为board.c配置，在不需要更改参数时，只需要open就行，不需要进行以下操作*/
>    #if 0
>     /*write and read 配置占空比*/
>
>     pwm.pwm_ch = PWMCH0_H | PWMCH0_L;//配置通道
>    pwm.duty = 80;//配置占空比
>     dev_write(pwm_dev_handl, (void *)&pwm, 0);
>    ret = dev_read(pwm_dev_handl, (void *)&pwm, 0);//dev_read函数返回值为占空比（不带小数点）0-100
>     printf("pwm0 read duty : %d \n", ret);
>    os_time_dly(200);
> #endif
>
>      /*2.ioctl配置占空比*/
>    int i;
> 	while(1)
>	{
> 		//值范围：0~100  灯渐亮
>		for( i = 0; i<= 100 ; i++)
> 		{
>		    pwm.duty = i;
>             dev_ioctl(pwm_dev_handl, PWM_SET_DUTY, (u32)&pwm);//设置占空比
>            os_time_dly(10);
> 		}
>		//灯渐灭
> 		for( i = 100; i>= 0 ; i--)
>		{
> 			pwm.duty = i;
>             dev_ioctl(pwm_dev_handl, PWM_SET_DUTY, (u32)&pwm);//设置占空比
>             os_time_dly(10);
>		}
> 	}
>
> }
>static int c_main(void)
> {
>    os_task_create(pwm_led_test, NULL, 12, 1000, 0, "pwm_led_test");
>     return 0;
>}
> late_initcall(pwm_led_test);
>
> ```
>
> 
## 操作说明

> 1. 使用串口线连接打印
> 2. 编译工程，烧录镜像，复位启动
> 3. 系统启动后，接上led灯，观察现象
>
> JIELI SDK的编译、烧写等操作方式的说明可在以下文档获取：[文档](../../../../doc/stuff/usb updater.pdf)
---

## 常见问题

> * 建议选择pwm1，如果选择了pwm0，需添加通道，具体添加方法可以参考PWM例程
> * pwm频率不宜设置太低。当频率大于50Hz的时候，人眼就会产生视觉暂留效果，基本就看不到闪烁。

## 参考文档

> * N/A