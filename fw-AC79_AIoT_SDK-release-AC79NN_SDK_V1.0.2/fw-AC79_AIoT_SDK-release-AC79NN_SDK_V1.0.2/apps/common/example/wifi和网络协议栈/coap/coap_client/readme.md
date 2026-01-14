# COAP Client示例工程说明

> 本工程展示了coap client的工程示例。
>

---

## 适用平台

> 本工程适用以下芯片类型：
>
> 1. AC79系列芯片：AC790N、AC791N

> 杰理芯片和评估板的更多信息可在以下地址获取：[链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)

## 工程配置说明

> 在SDK选择[PIC_BOOK_STORY](../../../../../apps/wifi_story_machine/board)工程CBP文件或者工程Makefile, 然后添加本事例工程代码
>
> 工程CBP文件或者工程Makefile 定义 CONFIG_NET_ENABLE，同时工程中添加libcoap头
>
> 文件搜索路径。
>
> app_config.h：
>
> * 打开 #define CONFIG_WIFI_ENABLE

## 模块依赖

> * 网络连接需要依赖的库
> * libcoap.a

---



### 操作说明：

> 1. 示例需要运行在STA模式
> 2. 编译工程，烧录镜像，复位启动
> 3. 系统启动并成功联网后，向指定coap服务器发送报文，可通过串口软件看到相关输出打印

> JIELI SDK的编译、烧写等操作方式的说明可在以下文档获取：[文档](../../../../../doc/stuff/usb updater.pdf)

### 代码流程

> 1. c_main()入口：
>     A）创建任务coap_client_start
>
> 2. coap_client_start函数流程
>
>     A）通过wifi_get_sta_connect_state函数检测是否通过dhcp成功获取到ip
>
>     B)  成功通过dhcp获取到ip后，调用coap_client_test
>
> 3. coap_client_test函数流程
>
>     A) 通过GET方法向coap服务器发送confirmabel报文
>
---

## 常见问题

> * N/A

## 参考文档

> * [libcoap官网](https://libcoap.net/)