# FTP OTA示例工程说明

> 本工程展示了通过FTP协议进行网络升级的使用方法。
>
> 示例作为FTP服务器，FTP客户端连接到FTP服务器并上传升级文件进行升级

---

## 适用平台

> 本工程适用以下芯片类型：
>
> 1. AC79系列芯片：AC790N、AC791N

> 杰理芯片和评估板的更多信息可在以下地址获取：
> [链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)

## 工程配置说明

> 在SDK选择[PIC_BOOK_STORY](../../../../../apps/wifi_story_machine/board)工程CBP文件或者工程Makefile, 然后添加本事例工程代码
>
> 工程CBP文件或者工程Makefile 定义 CONFIG_NET_ENABLE
>
> app_config.h：
>
> * 打开 #define CONFIG_WIFI_ENABLE
> * 如果采用双备份升级，需要定义#define CONFIG_DOUBLE_BANK_ENABLE   1
> * 如果采用单备份升级，需要定义#define CONFIG_DOUBLE_BANK_ENABLE   0

## 模块依赖

> * 网络连接需要的库

---



### 操作说明：

> 1. 设置FTP服务器用户名和密码
> 3. 编译工程，烧录镜像，复位启动
> 4. 系统启动并成功联网后，通过FileZilla软件连接FTP服务器，连接成功后将升级文件上传到ftp服务器进行升级

> JIELI SDK的编译、烧写等操作方式的说明可在以下文档获取：
> [文档](/doc/stuff/usb%20updater.pdf)

### 控制命令

> * N/A

### 代码流程

> 1. c_main()入口：
>     A）创建任务ftp_upgrade_start
>
> 2. ftp_upgrade_start函数流程
>
>     A）注册读写操作集
>
>     B)  初始化FTP服务器用户名和密码，等待客户端连接
---

## 常见问题

> * N/A

## 参考文档

> * N/A