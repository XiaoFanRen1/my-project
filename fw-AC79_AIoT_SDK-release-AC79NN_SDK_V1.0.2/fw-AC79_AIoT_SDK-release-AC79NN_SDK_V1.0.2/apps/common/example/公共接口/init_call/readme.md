# init_call使用示例工程说明

> 本工程展示了init_call接口使用示例以及各种init_call在系统内部调用顺序

---

## 适用平台

> 本工程适用以下芯片类型：
> 1. AC79系列芯片：AC790N、AC791N
>
> 杰理芯片和评估板的更多信息可在以下地址获取：[链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)

## 工程配置说明

> 在SDK选择[demo_hello](../../../../apps/demo_hello/board)主工程文件或者主工程Makefile, 然后添加本事例工程代码

---



### 操作说明：

> 2. 编译工程，烧录镜像，复位启动
> 3. 系统启动后，可以通过串口软件看到示例的打印信息

> JIELI SDK的编译、烧写等操作方式的说明可在以下文档获取：[文档](../../../../doc/stuff/usb updater.pdf)

### 代码流程

> 1. app_task_handler 入口：
>     A）do_early_initcall
>
>     B ) do_platform_initcall
>
>     C ) do_initcall
>
>     D ) do_module_initcall
>
>     E ) do_late_initcall
>
---

## 常见问题

> * init_call注册的函数什么时候运行,在哪里运行
>
>   答:在app_core任务 app_task_handler 入口顺序执行
>
>   * 什么情况下使用init_call
>
>   答:如果想隐式调用一个功能模块的初始化,可以使用init_call.类似于C++的构造函数
>
>
> * init_call 顺序问题
>
>   答:  因为同一类型init_call是无法区分顺序, 增加多种init_call是为了某些init_call需要依赖某些模块先初始化
>   
>   
>   
> * init_call仅仅为初始化，禁止在initcall函数程序代码中进行while死循环和长时间延时

## 参考文档

> * N/A