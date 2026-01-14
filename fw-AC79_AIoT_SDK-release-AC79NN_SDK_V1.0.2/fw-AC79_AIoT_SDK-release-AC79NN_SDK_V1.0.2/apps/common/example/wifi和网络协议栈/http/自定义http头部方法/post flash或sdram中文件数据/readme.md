# 通过自定义http头部post flash文件数据示例工程说明

> ​       出于一些开发需要，开发者需要自定义http头部信息，本工程展示如何通过自定义http头部post flash上的文件数据到http服务器。
>

---

## 适用平台

> 本工程适用以下芯片类型：
>
> 1. AC79系列芯片：AC790N、AC791N

> 功能链接

## 工程配置说明

> 在SDK选择[PIC_BOOK_STORY](../../../../../apps/wifi_story_machine/board)工程CBP文件或者工程Makefile, 然后添加本事例工程代码
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
> 2. 定义需要post到的HTTP_DEMO_URL；
> 3. 编译工程，烧录镜像，复位启动
> 4. 系统启动并成功连接网络后，可通过串口软件看到相关打印

> JIELI SDK的编译、烧写等操作方式的说明可在以下文档获取：[文档](../../../../../doc/stuff/usb updater.pdf)

### 代码流程

> 1. c_main()入口：
>     A）创建任务http_user_header_test
>
> 2. http_user_header_test函数流程
>
>     A）通过wifi_get_sta_connect_state函数检测是否通过dhcp成功获取到ip
>
>     B)  成功通过dhcp获取到ip后，创建任务http_test_task
>
> 3. http_test_task任务流程
>
>     A) 设置自定义http头部信息和http连接的相关参数 
>
>     B)  调用httpcli_post接口连接http服务器并post数据。
>
---

## 常见问题

> * 如何进行http通信调试？
>
>   答：由于自定义http头部需要开发者对http协议有一定程度的理解，很多时候无法进行http通信都是由于http通信协议格式错误，因此调试时需要先借助其他软件，如：postman等，将http通信调通，然后仿照其头部协议格式进行程序编写。

## 参考文档

> * N/A