# OTA示例工程说明

> 本工程展示了网络升级的使用方法。
>
> 通过HTTP协议，进行OTA升级

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

> 1. 示例需要运行在STA模式
> 2. 设置升级URL（注：例子测试的OTA_URL_ADDR为杰理服务器测试链接,客户可修改为自己服务器链接）
> 3. 编译工程，烧录镜像，复位启动
> 4. 系统启动并成功联网后，根据指定url下载升级文件并写入升级区

> JIELI SDK的编译、烧写等操作方式的说明可在以下文档获取：
> [文档](/doc/stuff/usb%20updater.pdf)

### 控制命令

> * N/A

### 代码流程

> 1. c_main()入口：
>     A）创建任务http_upgrade_start
>
> 2. http_upgrade_start函数流程
>
>     A）通过wifi_get_sta_connect_state函数检测是否通过dhcp成功获取到ip
>
>     B)  成功通过dhcp获取到ip后，调用http_create_download_task，退出http_upgrade_start
>
> 3. http_create_download_task函数流程
>
>     A) 初始化http相关参数
>
>     B)  创建任务download_task
>
> 4. download_task函数流程
>
>     A) 根据指定url发起连接请求，建立socket连接
>
>     B)  读取网络升级文件数据，通过net_fwrite接口写到升级区
>
>     C)  重启系统
---

## 常见问题

> * N/A

## 参考文档

> * N/A