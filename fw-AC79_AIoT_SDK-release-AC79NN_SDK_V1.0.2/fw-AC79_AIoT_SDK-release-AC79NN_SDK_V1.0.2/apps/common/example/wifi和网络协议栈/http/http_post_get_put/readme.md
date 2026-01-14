# Http  (Post, Get, Put)方法使用示例工程说明

> 本工程展示了http client基本使用方法：
>
> 1. HTTP POST方法的使用
> 2. HTTP GET方法的使用
> 3. HTTP PUT方法的使用

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

## 模块依赖

> * 网络连接需要依赖的库

---



### 操作说明：

> 1. 示例需要运行在STA模式
> 2. 选择工程其中一种请求方法进行测试。
> 3. 编译工程，烧录镜像，复位启动
> 4. 系统启动并成功联网后，应用程序会向服务端发送对应请求，可通过串口软件看到打印信息
>
> JIELI SDK的编译、烧写等操作方式的说明可在以下文档获取：[文档](../../../../doc/stuff/usb updater.pdf)



### 代码流程

> 1. c_main()入口：
>     A）创建任务http_test_start
>
> 2. http_test_start函数流程
>
>     A）通过wifi_get_sta_connect_state函数检测是否通过dhcp成功获取到ip
>
>     B)  成功通过dhcp获取到ip后，创建任务http_test_task，退出http_test_start
>
> 3. http_test_task任务流程
>
>     A) 根据定义的请求方法，初始化http相关参数
>
>     B)  向服务端发送对应请求
---

## 常见问题

> * WIFI库选择说明:
>
>   wl_wifi.a     			   包含AP和STA模式,占据大内存,性能强悍
>   wl_wifi_sfc.a     		包含AP和STA模式,占据内存小
>   wl_wifi_ap.a			  只包含AP模式,占据大内存,性能强悍
>   wl_wifi_ap_sfc.a        只包含AP模式,占据内存小
>   wl_wifi_sta.a			只包含STA模式,占据大内存,性能强悍
>   wl_wifi_sta_sfc.a     	只包含STA模式,占据内存小
>
>   lwip.a       占据大内存,性能强悍
>   lwip_sfc.a   占据内存小
>
>   如果使用sta模式,还需要包含wpasupplicant.a库

## 参考文档

> * N/A