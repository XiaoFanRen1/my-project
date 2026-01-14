# Websocket Client示例工程说明

> 本工程展示了Websocket Client接口使用方法。
>
> websocket示例工程展示了作为客户端发送数据到websocket服务器，并从服务器接收数据的示例

---

## 适用平台

> 本工程适用以下芯片类型：
>
> 1. AC79系列芯片：AC790N、AC791N

> 杰理芯片和评估板的更多信息可在以下地址获取：[链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)

## 工程配置说明

> 在SDK选择[PIC_BOOK_STORY](../../../../apps/wifi_story_machine/board)工程CBP文件或者工程Makefile, 然后添加本事例工程代码
>
> 工程CBP文件或者工程Makefile 定义 CONFIG_NET_ENABLE
>
> app_config.h：
>
> * 打开 #define CONFIG_WIFI_ENABLE

## 模块依赖

> * 网络连接需要依赖的库

---



### 操作说明：

> 1. 示例需要运行在STA模式
> 3. 编译工程，烧录镜像，复位启动
> 4. 系统启动并成功联网后，websocket客户端会循环发送数据到服务器，并接收websocket服务器的数据

> JIELI SDK的编译、烧写等操作方式的说明可在以下文档获取：[文档](../../../../doc/stuff/usb updater.pdf)

### 代码流程

> 1. c_main()入口：
>     A）创建任务http_websocket_start
>
> 2. http_websocket_start函数流程
>
>     A）通过wifi_get_sta_connect_state函数检测是否通过dhcp成功获取到ip
>
>     B)  成功通过dhcp获取到ip后，调用websocket_client_thread_create创建任务websockets_client_main_thread，退出http_websocket_start
>
> 3. websockets_client_main_thread任务流程
>
>     A)  初始化websocket相关参数和API（注：大多数接口都是内部已经实现好，用户只需要实现回调函数用于数据处理），连接websocket服务器
>
>     B)  连接成功后，创建websocket_client_heart任务和websocket_client_recv任务，接着循环向websocket服务器发送数据
>
> 4. websockets_callback为接收数据回调函数，用户可以进行数据处理
---

## 常见问题

> * N/A

## 参考文档

> * N/A