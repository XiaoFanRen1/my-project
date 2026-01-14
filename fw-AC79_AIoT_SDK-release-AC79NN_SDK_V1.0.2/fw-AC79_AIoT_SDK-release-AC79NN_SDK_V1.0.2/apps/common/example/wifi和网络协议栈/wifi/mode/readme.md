# wifi ap/sta/monitor三种模式示例工程说明

> 本工程展示了wifi模块ap/sta/monitor三种模式切换的使用示例:
>
> 1. wifi启动STA模式的使用
> 2. wifi启动AP模式的使用
> 3. wifi启动MONITOR模式的使用
> 4. STA,AP,MONITOR三种模式相互切换的使用

---

## 适用平台

> 本工程适用以下芯片类型：
>
> 1. AC79系列芯片：AC790N、AC791N
>
> 杰理芯片和评估板的更多信息可在以下地址获取：[链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)

## 工程配置说明

> 在SDK选择[demo_wifi](../../../../../apps/demo//demo_wifi/board)主工程文件或者主工程Makefile
>
> * [wifi_demo_task.c](../../../../../apps/demo/demo_wifi/wifi_demo_task.c) 
>
>   打开宏定义#define STA_MODE_TEST配置WIFI工作在STA模式,并且配置好STA_SSID和STA_PWD
>
>   或者打开宏定义#define AP_MODE_TEST配置WIFI工作在AP模式,并且配置好AP_SSID和AP_PWD
>
>   或者打开宏定义#define MONITOR_MODE_TEST配置WIFI工作在混杂/监听/配网模式
>
> * 若需要循环模式切换测试同时打开宏WIFI_MODE_CYCLE_TEST

---



### 操作说明：

> * 设备端操作:
>
>   1. 编译工程，烧录镜像，复位启动
>   2. 系统启动后，可以通过串口软件看到示例的打印信息, JIELI SDK的编译、烧写等操作方式的说明可在以下文档获取：[文档](../../../../doc/stuff/usb updater.pdf)

### 代码流程
> * STA_MODE_TEST配置WIFI工作在STA模式:
> 1. c_main()入口：
>    A）创建任务wifi_demo_task
>
> 2. wifi_demo_task
>
>    A）启动WIFI
>
>    B）打印WIFI信号质量信息和吞吐率信息
>    
> 3. wifi_event_callback
>    A）case WIFI_EVENT_MODULE_INIT: 配置WIFI启动为STA模式和连接路由器的SSID和密码
>    B）case WIFI_EVENT_STA_NETWORK_STACK_DHCP_SUCC:连接路由器成功,并且DHCP获取到IP地址

> * AP_MODE_TEST配置WIFI工作在AP模式:
> 1. c_main()入口：
>    A）创建任务wifi_demo_task
>
> 2. wifi_demo_task
>
>    A）启动WIFI
>
>    B）打印WIFI信号质量信息和吞吐率信息
>
> 3. wifi_event_callback
>    A）case WIFI_EVENT_MODULE_INIT: 调用wifi_set_lan_setting_info();配置AP模式的IP地址信息和DHCP池起始分配地址, 配置WIFI启动为AP模式和热点SSID和密码
>    B）case WIFI_EVENT_AP_START:AP模式启动成功,此时手机端可以搜索到对应SSID的热点
>    
>    C）case WIFI_EVENT_AP_ON_ASSOC:当有手机成功连接到WIFI,引发事件
>    
>    D）case WIFI_EVENT_AP_ON_DISCONNECTED:当有手机断开WIFI,引发事件

> * MONITOR_MODE_TEST配置WIFI工作在混杂/监听/配网模式:
> 1. c_main()入口：
>    A）创建任务wifi_demo_task
>
> 2. wifi_demo_task
>
>    A）启动WIFI
>
>    B）打印WIFI信号质量信息和吞吐率信息
>
> 3. wifi_event_callback
>    A）case WIFI_EVENT_MODULE_INIT: 配置WIFI启动为MONITOR模式
>    B）case WIFI_EVENT_SMP_CFG_START:注册接收802.11数据帧回调wifi_rx_cb
>    
>    C）case WIFI_EVENT_SMP_CFG_STOP:关闭接收802.11数据帧回调
>    
> 4. wifi_rx_cb
>
>    A）打印接收到的802.11数据帧