# Http Server方法使用示例工程说明

> 本工程展示了http服务器的使用，通过手机连接http服务器获取服务器资源。注意：sdk工程中http server只提供了get请求，其他请求方式暂时不提供。
>

---

## 适用平台

> 本工程适用以下芯片类型：
>
> 1. AC79系列芯片：AC790N、AC791N

> 杰理芯片和评估板的更多信息可在以下地址获取：[链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)

## 工程配置说明

> 在SDK选择[wifi_camera](../../../../../apps/wifi_camera/board/wl80)工程CBP文件或者工程Makefile, 然后添加本事例工程代码
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

> 1. 示例需要运行在AP模式
> 2. 编译工程，烧录镜像，复位启动
> 3. 系统启动后，连接AP wifi，成功连接AP wifi后，在手机浏览器中输入网址192.168.1.1:9080/index.html，跳转到该网址后手机上就会获取到http服务器的网页页面数据。
> 
>JIELI SDK的编译、烧写等操作方式的说明可在以下文档获取：[文档](../../../../doc/stuff/usb updater.pdf)



### 代码流程

> 1. c_main()入口：
>     A）创建任务http_server_start
>
> 2. http_server_start函数流程
>
>     A）延时一段时间等待ap wifi稳定后，创建任务http_server_task
>
> 3. http_server_task函数流程
>
>     A）通过http_virfile_reg注册资源
>
>     B)  http_get_server_init初始化http服务器，并等待客户端连接。
>
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