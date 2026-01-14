# wait_completion使用示例工程说明

> 本工程展示了wait_completion接口使用示例:
>
> 1. wait_completion接口使用;

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
>
> JIELI SDK的编译、烧写等操作方式的说明可在以下文档获取：[文档](../../../../doc/stuff/usb updater.pdf)

### 代码流程

> 1. c_main()入口：
>     A）创建wait_completion_test_task任务
>
> 2. wait_completion_test_task
>
>     A）把wait_completion_callback加到app_core去执行
>
>     B ) 等待wait_condition成立
>     
>     C ) app_core 执行 wait_completion_callback
>     
>     D ) 中途取消wait_completion
>
> 
---

## 常见问题

> * wait_completion一般什么情况下使用
>
>   答:一般用于异步事件非阻塞处理,需要等待条件成立再去运行某个函数,并且想这个函数指定在任务中如app_core运行,避免阻塞等死,也可以达到单线程去运行函数

## 参考文档

> * N/A