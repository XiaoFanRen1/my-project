# Http Download示例工程说明

> 本工程展示了http download使用方法。
>
> 通过HTTP协议，下载URL指定的网络文件存储至指定路径，并具有断点重载的功能

---

## 适用平台

> 本工程适用以下芯片类型：
>
> 1. AC79系列芯片：AC790N、AC791N

> 杰理芯片和评估板的更多信息可在以下地址获取：[链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)

## 工程配置说明

> 在SDK选择[PIC_BOOK_STORY](../../../../../apps/wifi_story_machine/board)工程CBP文件或者工程Makefile, 然后添加本事例工程代码
>
> 工程CBP文件或者工程Makefile 定义 CONFIG_NET_ENABLE
>
> app_config.h：
>
> * 打开 #define CONFIG_WIFI_ENABLE
> * 定义#define TCFG_SD0_ENABLE   1

## 模块依赖

> * 网络连接需要依赖的库

---



### 操作说明：

> 1. 示例需要运行在STA模式
> 2. 设置好下载链接（注：修改HTTP_DOWNLOAD_URL定义），网络文件存储路径（注：修改HTTP_STORAGE_PATH定义）
> 3. 编译工程，烧录镜像，复位启动
> 4. 系统启动，成功联网并且存储设备就绪后，会对指定网络文件进行下载并保存在指定的存储路径下

> JIELI SDK的编译、烧写等操作方式的说明可在以下文档获取：[文档](../../../../../doc/stuff/usb updater.pdf)

### 代码流程

> 1. c_main()入口：
>     A）创建任务http_download_start
>
> 2. tcp_client_start函数流程
>
>     A）通过wifi_get_sta_connect_state函数检测是否通过dhcp成功获取到ip
>
>     B)  成功通过dhcp获取到ip后，调用net_file_download_test，退出http_download_start
>
> 3. net_file_download_test函数流程
>
>     A) 通过storage_device_ready对存储设备状态进行检测，如果存储设备未就绪，会直接退出应用程序
>
>     B)  调用create_download_task
>
> 4. create_download_task函数流程
>
>     A) 初始化http相关参数
>
>     B)  创建任务download_task
>
> 5. download_task任务流程
>
>     A) 初始化http相关参数
>
>     B)  发起连接请求，建立socket连接
>     
>     C)  建立连接成功后，读取网络文件数据，并将数据存储到指定路径下
---

## 常见问题

> * N/A

## 参考文档

> * N/A