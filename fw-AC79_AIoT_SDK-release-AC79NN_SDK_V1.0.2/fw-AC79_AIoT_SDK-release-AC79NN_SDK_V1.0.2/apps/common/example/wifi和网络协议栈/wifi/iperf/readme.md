# IPERF示例工程说明

> 本工程展示了使用iperf测试wifi吞吐率的使用示例:
>
> 1. WIFI STA模式测试IPERF
> 2. WIFI AP模式测试IPERF
> 3. IPERF的上下行吞吐率测试以及IPERF工具的参数配置

---

## 适用平台

> 本工程适用以下芯片类型：
> 1. AC79系列芯片：AC790N、AC791N
>
> 杰理芯片和评估板的更多信息可在以下地址获取：[链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)

## 工程配置说明

> 在SDK选择[demo_wifi](../../../../../apps/demo//demo_wifi/board)主工程文件或者主工程Makefile, 然后添加本事例工程代码
>
> * [wifi_demo_task.c](../../../../../apps/demo/demo_wifi/wifi_demo_task.c)  打开宏定义AP_MODE_TEST或者STA_MODE_TEST配置WIFI工作模式
> * [wifi_demo_task.c](../../../../../apps/demo/demo_wifi/wifi_demo_task.c) wifi_on(); 之后调用 iperf_test();

### 操作说明：

> *	设备端操作:
> 	1. 编译工程，烧录镜像，复位启动
> 	2. 系统启动后，可以通过串口软件看到示例的打印信息
>
> 	JIELI SDK的编译、烧写等操作方式的说明可在以下文档获取：[文档](../../../../doc/stuff/usb updater.pdf)
> 	
> *	手机端操作:
>
>   1.安卓手机可安装[Magic iPerf.apk](../../../../../sdk_tools/IPERF3 测试工具), 苹果手机端在商店安装对应收费版本软件, PC端可安装[iperf-3.1.3-win64.exe](../../../../../sdk_tools/IPERF3 测试工具) 
>
>   2.参考文档[iperf3工具说明](../../../../../sdk_tools/IPERF3 测试工具/iperf3工具说明.pdf)进行测试

### 代码流程

> 1. c_main()入口：
>     A）创建WIFI启动任务wifi_demo_task
>     
>3. wifi_demo_task函数流程
>    A）注册WIFI事件回调函数
>   B）启动WIFI
> 
>    C）启动IPERF测试
---

## 常见问题

> * iperf测试吞吐率一般结果为多少?
>
>   答:AP模式下一般TX/RX速率有800KB/S-2MB/S,STA模式下一般有200KB-1MB/S
>
> 
>
> * 影响iperf测试吞吐率的因素有哪些?
>
>   答:
>
>   1.WIFI RF校准参数有没有进行仪器测试调整到SDK里面
>
>   2.板子天线是否有调试好
>
>   3.AP模式比STA模式吞吐率要高
>
>   4.空中信道是否有比较大的干扰
>
>   5.SDK运行过程中剩下的CPU使用率是否足够
>
>   6.测试手机,路由器,设备,距离是否足够小
>
>   7.使用占据资源多的高性能的WIFI库与LWIP库有利于提高吞吐率
>
>   8.程序运行在SDRAM模式比在SFC模式吞吐率要高

## 参考文档

> * [iperf3工具说明](../../../../../sdk_tools/IPERF3 测试工具/iperf3工具说明.pdf)