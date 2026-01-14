# WIFI AIRKISS配网示例工程说明

> 本工程展示了AirKiss配网接口的使用方法:
>
> 1. AirKiss配网流程示例
> 2. 杰理配网APP配网流程示例

> 在使用AirKiss配网接口前，需要对AirKiss配网技术有一定的了解，详细可参考：<https://iot.weixin.qq.com/wiki/new/index.html?page=4-1-1> 
>
> AirKiss基本原理是，设置芯片在monitor模式下，通过抓取空中的包，利用数据帧的长度来承载有效信息 ，从而达到配网的目的。

---

## 适用平台

> 本工程适用以下芯片类型：
> 1. AC79系列芯片：AC790N、AC791N

> 杰理芯片和评估板的更多信息可在以下地址获取：[链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)

## 工程配置说明

> 在SDK选择[demo_wifi](../../../../../apps/demo//demo_wifi/board)主工程文件或者主工程Makefile
>
> [wifi_demo_task.c](../../../../../apps/demo/demo_wifi/wifi_demo_task.c) 打开宏定义#define MONITOR_MODE_TEST配置WIFI工作在配网模式

---



### 操作说明：

> 1. 编译工程，烧录镜像，复位启动后, 会自动进入配网模式
> 3. 安装并打开[AirKissDebugger.apk](../../../../../sdk_tools/airkiss.apk)软件,手机连接到某个用于AirKiss配网测试的wifi,在AirKissDebugger.apk软件中，填入要配网的wifi名、wifi密码及AES Key(默认不配置)，点击发送
> 3. 等待串口打印配网结果并且连接上对应的路由器

> JIELI SDK的编译、烧写等操作方式的说明可在以下文档获取：[文档](../../../../../doc/stuff/usb updater.pdf)



### 代码流程

> 1. c_main()入口：
>     A）创建WIFI启动任务wifi_demo_task
>     
>2. wifi_demo_task函数流程
>     A）打开wifi并且进入配网模式
>
> 3. wifi_event_callback
> 
>    A）case WIFI_EVENT_SMP_CFG_COMPLETED: 收到配网信息, 发送事件NET_SMP_CFG_COMPLETED到app_demo_event_handler,根据SSID完整性决定是否需要扫描空中SSID进行匹配
>     B）case WIFI_EVENT_STA_SCAN_COMPLETED:扫描空中SSID完成,调用airkiss_ssid_check检测是否扫描到匹配的SSID, 然后发送NET_EVENT_SMP_CFG_FINISH事件到app_demo_event_handler进行连接对应的路由器
>
>     C）case WIFI_EVENT_STA_NETWORK_STACK_DHCP_SUCC:连接路由器成功,发送事件NET_EVENT_CONNECTED到app_demo_event_handler,调用config_network_broadcast发送广播信号（告诉手机端AirKissDebugger.apk软件已经配网成功了） 
---

## 常见问题

> * > 问：airkiss连接不上或连接超时
>
>   答：
>
>   1.检查aes_key是否设置正确；
>
>   2.检查路由器是否是2.4G；
>
>   3.检查路由器是否设置为支持20M或20M/40M。
>
> 

## 参考文档

> * N/A