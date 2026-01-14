#   长按复位使用示例工程说明

> 本工程展示了长按复位接口使用方法。
>
> 长按复位可4秒或8秒复位。

---

## 适用平台

> 本工程适用以下芯片类型：
>
> 1. AC79系列芯片：AC790N、AC791N
>
> 杰理芯片和评估板的更多信息可在以下地址获取：[链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)
## 工程配置说明

​				在SDK选择[demo_hello](../../../../apps/demo_hello/board)主工程文件或者主工程Makefile, 然后添加本事例工程代码。

> 1. board.c 添加长按IO配置和4秒/8秒配置:
>
>
> ```c
> 
> /************************** PWR wakeup config ****************************/
> //#define PORT_VCC33_CTRL_IO		IO_PORTA_03					//VCC33 DCDC控制引脚,该引脚控制DCDC器件输出的3.3V连接芯片HPVDD、VDDIO、VDD33
> #define PORT_WAKEUP_IO			IO_PORTB_01					//软关机和休眠唤醒引脚
> #define PORT_WAKEUP_NUM			(PORT_WAKEUP_IO/IO_GROUP_NUM)//默认:0-7:GPIOA-GPIOH, 可以指定0-7组
> 
> //软关机硬件唤醒IO相关配置
> static const struct port_wakeup port0 = {
>     .edge       = FALLING_EDGE,                            //唤醒方式选择,可选：上升沿\下降沿
>     .attribute  = BLUETOOTH_RESUME,                        //保留参数
>     .iomap      = PORT_WAKEUP_IO,                          //唤醒口选择
>     .low_power	= POWER_SLEEP_WAKEUP|POWER_OFF_WAKEUP,    //低功耗IO或睡眠唤醒,不需要写0
> };
> //正常工作长按复位IO配置
> static const struct long_press lpres_port = {
> 	.enable 	= TRUE,									//是否使用长按复位，TRUE为使能，FALSE则禁能
> 	.use_sec4 	= TRUE,										//enable = TRUE , use_sec4: TRUE --> 4 sec , FALSE --> 8 sec
> 	.edge		= FALLING_EDGE,								//长按方式,可选：FALLING_EDGE /  RISING_EDGE --> 低电平/高电平
> 	.iomap 		= PORT_WAKEUP_IO,							//长按复位IO和IO唤醒共用一个IO
> };
> //保留参数，无需更改
> static const struct sub_wakeup sub_wkup = {
>     .attribute  = BLUETOOTH_RESUME,
> };
> //保留参数，无需更改
> static const struct charge_wakeup charge_wkup = {
>     .attribute  = BLUETOOTH_RESUME,
> };
> //唤醒参数配置
> static const struct wakeup_param wk_param = {
>     .port[0] = NULL,//当需要IO唤醒，详情soft_power_off例子
>     .sub = &sub_wkup,
>     .charge = &charge_wkup,
> 	.lpres = &lpres_port,//当使用正常工作时可长按复位功能，详情longpress例子
> };
> 
> //电源初始化
> static void board_power_init(void)
> {
>     power_wakeup_init(&wk_param);//唤醒参数初始化，包括长按复位
> }
> 
> ```
> 
>
## 操作说明

> 1. 使用串口线连接打印
> 2. 编译工程，烧录镜像，复位启动
> 3. 系统启动后，长按PB1 IO测试复位功能。
>
> JIELI SDK的编译、烧写等操作方式的说明可在以下文档获取：[文档](../../../../doc/stuff/usb updater.pdf)
---

## 常见问题

> * 长按复位IO，默认根据配置边缘选择性的开启内部上下拉（下降沿则上拉，上升沿则下拉），因此长按复位硬件IO使用和唤醒IO不一样时，可以不用外接上下拉电阻；当长按复位硬件IO使用和唤醒IO为同一个IO时，长按功能不受影响，但IO需要外接上下拉来维持IO唤醒时的软关机后IO电平。
>
>   
>
> * 长按复位功能是board.c配置好对应程序后，系统上电运行就自动加载长按功能，因此系统运行后可以长按按键来完成复位功能。

## 参考文档

> * N/A