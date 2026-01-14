# ai_sdk使用示例工程说明

> 本工程展示了ai_sdk接口使用示例:
>
> 1. 如何创建使用ai_sdk;
> 2. ai_sdk事件event上报, 响应流程;

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
		>     A）创建ai_sdk_example_task任务
		>
		> 2. ai_sdk_example_task
		>
		>     A）打开服务
		>
		>     B ) 注册服务事件通知回调
		>
		>     C）AI_SDK连接请求，查询是否存在AI_SDK，有则调用对应的connect函数
		>
		>     D）创建上报与下发消息线程
		>
		>     E）关闭服务,释放资源
		>     
		> 3. ai_test_server_event_handler ： 下发消息处理函数
		>
		> 4. ai_sdk_upload_task： 测试上报消息函数
		> 	
		> 5. ai_sdk_down_task： 测试下发消息函数



> * ai_test_server.c
>   > 1. SERVER_REGISTER：创建ai_test_server
>   >
>   > 2. ai_test_server_open：查询AI_SDK是否存在,是则存储到结构体中,并且返回结构体
>   >
>   > 3. ai_test_server_init
>   >
>   >    A）初始化变量
>   >
>   > 4. ai_test_server_close：在ai_test_server服务关闭时调用，AI_SDK的disconnect函数并且初始化变量
>   >
>   > 6. ai_server_request: 分发处理各类型的消息
>   >
>   > 7. ai_test_req_connect: 处理ai_server连接请求
>   >
>   > 8. ai_test_req_disconnect:处理ai_server断开连接请求
>   >
>   > 9. ai_test_req_listen: 处理ai_server监听请求
>   >
>   > 10. ai_test_req_event:处理ai_server事件请求
>   >
>   > 11. ai_test_server_event_notify: 用于下发notify事件给AI_SDK
>   >
>   > 12. ai_test_server_event_url: 用于下发url时间给AI_SDK
>   >
>   > 13. ai_test_check_connect_state: 用于下发连接或断开事件给AI_SDK



> * test_ai.c
>   > 1. REGISTER_AI_SDK :创建AI_SDK
>   > 2. ai1_sdk_open : 对应的ai_sdk的connect函数
>   > 3. ai1_sdk_check: 对应的ai_sdk的state_check函数
>   > 4. ai1_sdk_do_event  :  对应的ai_sdk的do_event函数
>   > 5. ai1_sdk_disconnect: 对应的ai_sdk的disconnect函数



## 常见问题

> * 为什么要使用ai_sdk
>
>   答:
>   
>   ​	1.AI_SDK常用于接入第三方服务器播歌,读绘本等抽象出来统一网络云平台服务功能的接口;
>   
>   ​	2.外部直接调用功能接口有可能存在互斥的问题,引入AI_SDK是为了解决这个问题,应用程序无需关心使用互斥问题, ai_server任务统一了事件的操作,用户操作对应AI_SDK消息时候只需要将对应的AI_SDK名字传入即可;
>
> 

## 参考文档

> * N/A
