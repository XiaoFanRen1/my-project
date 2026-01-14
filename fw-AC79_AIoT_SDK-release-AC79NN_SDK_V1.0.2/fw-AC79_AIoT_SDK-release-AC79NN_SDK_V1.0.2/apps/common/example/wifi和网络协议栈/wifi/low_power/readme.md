# STA节能模式PSMode示例工程说明

> 本工程展示了使用STA节能模式PSMode的使用示例:
>
> 1. STA节能模式PSMode使用的方法
> 2. 一段时间内程序限定WIFI不进入休眠的方法

---

## 适用平台

> 本工程适用以下芯片类型：
>
> 1. AC79系列芯片：AC790N、AC791N
>
> 杰理芯片和评估板的更多信息可在以下地址获取：[链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)

## 工程配置说明

> 在SDK选择[demo_wifi](../../../../../apps/demo//demo_wifi/board)主工程文件或者主工程Makefile, 然后添加本事例工程代码
>
> * [app_config.h](../../../../../apps/demo/demo_wifi/include/app_config.h) 打开 #define CONFIG_LOW_POWER_ENABLE              //WIFI节能模式开关
> * [app_config.h](../../../../../apps/demo/demo_wifi/include/app_config.h) #define TCFG_LOWPOWER_LOWPOWER_SEL			RF_SLEEP_EN //配置仅WIFI RF休眠
> * [wifi_demo_task.c](../../../../../apps/demo/demo_wifi/wifi_demo_task.c) 打开宏定义#define STA_MODE_TEST配置WIFI工作在STA模式,并且配置好STA_SSID和STA_PWD

---



### 操作说明：

> * 设备端操作:
>
>   1. 编译工程，烧录镜像，复位启动
>   2. 系统启动后，可以通过串口软件看到示例的打印信息, JIELI SDK的编译、烧写等操作方式的说明可在以下文档获取：[文档](../../../../doc/stuff/usb updater.pdf)
>
>      3.当从打印信息看到设备WIFI已经成功连接路由器以后,手机端或者电脑端可以在同一个局域网内进行PING设备或者iperf测试
>  
>      4.用户可以测试WIFI在STA模式下空闲状态/高吞吐率状态/休眠状态的电流
>   

### 代码流程

> 1. c_main()入口：
>    A）创建任务sta_low_power_test_task
>
> 2. sta_low_power_test_task
>
>    A）循环定时去允许和不允许WIFI 进行休眠

---

## 常见问题

> 
