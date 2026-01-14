# OS API使用示例工程说明

> 本工程展示了OS API基本使用方法，包括任务创建（动态任务创建和静态任务创建）、信号量、互斥量的使用和消息队列的使用等，主要示例包括：
>
> （1）mutex_test()；						 互斥量使用示例
>
> （2）queue_test();  						  消息队列使用示例
>
> （3）sem_test();      						  信号量使用示例
>
> （4）static_task_test();   				  静态任务创建示例
>
> （5）thread_can_not_kill_test(); 	模拟线程无法杀死示例

---

## 适用平台

> 在SDK选择[demo_hello](../../../../apps/demo_hello/board)主工程文件或者主工程Makefile, 然后添加本事例工程代码

## 任务列表使用方法

> ​	   任务列表的作用可以对任务进行统一管理，方便对任务的优先级（优先级范围为0~31，数值越大优先级越高）、任务堆栈大小和消息队列大小进行调整，同时支持动态任务注册和静态任务注册,例如：
>
> ```
> #define STATIC_TASK_STK_SIZE 256 //任务堆栈大小
> #define STATIC_TASK_Q_SIZE 0     //消息队列大小
> 
> static u8 test_stk_q[sizeof(struct thread_parm) + STATIC_TASK_CREATE_STK_SIZE * 4 + \ 
> 					 (STATIC_TASK_CREATE_Q_SIZE ? (sizeof(struct task_queue) + APP_CORE_Q_SIZE) : 0 )] ALIGNE(4);
> 
> /*任务列表*/
> //{任务名, 任务优先级, 任务堆栈大小, 任务消息队列大小, 任务堆栈},
> const struct task_info task_info_table[] = {
> 	{"led_ui_server",       30,      256,   64    },      //动态任务注册
> 	{"task_create_static_test", 12, STATIC_TASK_CREATE_STK_SIZE, STATIC_TASK_CREATE_Q_SIZE, test_stk_q}, //静态任务注册
> 	{0, 0},
> };
> 
> ```
>
> 

---



### 操作说明：

> 1. 选择工程其中一种测试例进行测试。
> 2. 编译工程，烧录镜像，复位启动、
> 3. 系统启动后，可通过串口软件看到打印信息
>
> JIELI SDK的编译、烧写等操作方式的说明可在以下文档获取：[文档](../../../../doc/stuff/usb updater.pdf)



### 代码流程

> 1. c_main()入口：
>     A）根据定义运行对应的测试例。
>     
---

## 常见问题

> * 创建静态任务和创建动态任务有什么区别，该如何选择？
>
>   答：静态任务创建时采用的是静态内存，而动态任务创建时采用的动态内存。采用静态任务创建可以减小内存碎片；动态任务创建由于会频繁分配和释放内存，容易产生内存碎片。因此系统运行期间都必须存在的任务可采用静态任务创建，而运行周期较短的任务采用动态任务创建，这样一定程度上可以减小内存碎片和内存开销。
>   
> * 程序运行时出现thread can't kill! pid : 0x400ce5c,rets_addr = 0x20110e6，怎么解决？如何正确退出线程？
>
>   答：出现这个打印是因为对应的线程无法退出导致的，thread_kill会阻塞等待线程退出，通过pid可找到对应的线程，或通过rets_addr地址找到阻塞的位置。需要退出线程时可通过设置标志位的方法，可参考static_task_test.c例程，或通过消息队列给对应线程发送特定消息，线程收到该消息后进行退出操作。

## 参考文档

> * N/A