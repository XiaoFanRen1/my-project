# app_state_machine 示例工程说明


> 本工程展示了app_state_machine接口使用示例:
>
> 1. app_state_machine的创建
> 2. app_state_machine的使用与切换
![](图片1.png) 

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
>
> JIELI SDK的编译、烧写等操作方式的说明可在以下文档获取：[文档](../../../../doc/stuff/usb updater.pdf)

### 代码流程

> 1. c_main()入口：
>     A）创建app_state_machine_test_task任务
>     
>     
>     
> 2. app_state_machine_test_task
>
>     A）创建并运行APP1的相关action
>
>     B ) 创建并运行APP2的相关action
>
>     C ) 卸载APP2状态机,回退到APP1状态机
>
>     D) 运行APP1的相关action
>
>     E ) 卸载APP1状态机,回退到APP2状态机
>
>     F ) 创建并运行APP2的相关action
>
>     G ) 停止APP1状态机
>
>     H ) 停止APP2状态机
---

## 常见问题

> * 为什么要使用app_state_machine?
>
>   答:
>
>   1.使用app_state_machine可以使得所有执行的action都在 "app_core"任务执行,所有动作和事件都是单线程运转,不需要考虑互斥问题;
>
>   2.当客户应用方案,存在多个模式,并且多个模式之间是互斥关系(非后台)的情况下, 建立多个app_state_machine切换可以使得系统资源合理利用和降低CPU的消耗;
>
>   3.系统所有事件产生都会发送到当前app_state_machine的event_handler;
>
> 

## 参考文档

> * N/A