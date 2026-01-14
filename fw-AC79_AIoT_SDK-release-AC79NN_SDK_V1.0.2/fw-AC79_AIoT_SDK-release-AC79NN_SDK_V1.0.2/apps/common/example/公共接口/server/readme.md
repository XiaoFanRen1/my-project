# server使用示例工程说明

> 本工程展示了server接口使用示例:
>
> 1. 如何创建使用server;
> 2. server事件event响应流程;

---

## 适用平台

> 本工程适用以下芯片类型：
> 1. AC79系列芯片：AC790N、AC791N

> 杰理芯片和评估板的更多信息可在以下地址获取：[链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)

## 工程配置说明

> 在SDK选择[demo_hello](../../../../apps/demo_hello/board)主工程文件或者主工程Makefile, 然后添加本事例工程代码

---



### 操作说明：

> 2. 编译工程，烧录镜像，复位启动
> 3. 系统启动后，可以通过串口软件看到示例的打印信息

> JIELI SDK的编译、烧写等操作方式的说明可在以下文档获取：[文档](../../../../doc/stuff/usb updater.pdf)

### 代码流程

> * main.c
		> 1. c_main()入口：
		>     A）创建server_example_task任务
		>
		>     2. server_example_task
		>
		>     A）打开服务
		>
		>     B ) 注册服务事件通知回调
		>     
		>     C）同步和异步请求服务做事情
		>     
		>     D）清除服务队列还没执行的 server_event_handler 事件通知
		>     
		>     E）关闭服务,释放资源

> * test_server.c
>   > 1. SERVER_REGISTER：创建server
>   >
>   > 2. test_server_open：创建一个与 SERVER_REGISTER name 同名的任务
>   >
>   > 3. test_server_task
>   >
>   >    A）响应层应用请求事件
>   >
>   >    B ) 事件通知上层应用
>   >
>   > 4. test_server_close
>   >
>   >    A）删除test_server任务
---

## 常见问题

> * 为什么要使用server
>
>   答:用户新增一个功能库, 外部直接调用功能接口有可能存在互斥的问题,引入server是为了解决这个问题,应用程序无需关心使用互斥问题,server任务统一了事件的操作,并且提供一种框架给用户使用.用户也可以通过熟悉此例子来了解其他server库的使用
>
>   

## 参考文档

> * N/A