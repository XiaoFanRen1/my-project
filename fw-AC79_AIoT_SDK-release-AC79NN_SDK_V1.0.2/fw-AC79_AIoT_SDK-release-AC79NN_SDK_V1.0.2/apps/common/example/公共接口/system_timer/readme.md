# 系统定时器使用示例工程说明

> 本工程展示了系统提供的sys_timer、sys_timeout、usr_timer、usr_timeout四种定时函数使用示例:
>
> 1. sys_timer定时扫描接口函数;
> 2. sys_timeout超时接口函数；
> 3. usr_timer定时扫描接口函数;
> 4. usr_timeout超时接口函数。

---

## 适用平台

> 本工程适用以下芯片类型：
> 1. AC79系列芯片：AC790N、AC791N
>
> 杰理芯片和评估板的更多信息可在以下地址获取：[链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)

## 工程配置说明

> 在SDK选择[demo_hello](../../../../apps/demo_hello/board)或者其他demo的主工程文件或者主工程Makefile, 然后添加本事例工程代码

---



### 操作说明：

> 2. 编译工程，烧录镜像，复位启动
> 3. 系统启动后，可以通过串口软件看到示例的打印信息
>
> JIELI SDK的编译、烧写等操作方式的说明可在以下文档获取：[文档](../../../../doc/stuff/usb updater.pdf)

### 代码流程

> 1. sys_timer_test()入口：
>
>     A）timer_id = sys_timer_add(NULL, callback_example, 5 * 1000);
>
>     //注册一个定时器timer，定时时间5秒，并返回id
>
>     B）#if 0   //条件0执行修改后的定时时间2秒，条件1继续执行定时时间5秒
>
>     ​      printf("timer_id = %d\r\n", timer_id);	//打印分配的timer id 
>
>     ​	  #else 
> ​      if(timer_id)
>     ​     {
>     ​        printf("timer_id = %d\r\n", timer_id);
>     ​        sys_timer_modify(timer_id, 2 * 1000);	//修改timer的定时时间为2秒
>     ​     }
>     
>     ​      #endif
>
>     C）每隔2秒的定时时间，则回调一次callback_example()
>
>     D）回调3次callback_example()后，执行sys_timer_del(timer_id);    
>
>     //删除timer，回调3次即删除timer
>
> 2. sys_timeout_test()入口：
>
>     A）timeout_id = sys_timeout_add(NULL, callback_example, 5 * 1000);    
>
>     //注册一个timeout，并返回id，定时时间到了自动删除timeout
>
>     B）#if 0   //条件0执行修改后的定时时间2秒;条件1删除定时时间为5秒的timeout
>           printf("timeout_id = %d\r\n", timeout_id);
>
>     ​	  sys_timeout_del(timeout_id);    //删除timeout，会自动删除，实际用不到该函数。
>
>     ​	  #else
>     ​      if(timeout_id)
>     ​     {
>     ​        printf("timeout_id = %d\r\n", timeout_id);
>     ​     //sys_timeout_modify(timeout_id, 2 * 1000);  //库里暂时没有该函数，仍执行5秒定时
>     ​     }
>     ​       #endif
>
>     C）一旦5秒的定时时间到，仅回调一次callback_example()，自动删除timeout
>
> 3. usr_timer_test()入口:
>
>     A) timer_id = usr_timer_add((void* )1, callback_example, 2*1000, 1);    
>
>     //注册一个高精度timer，传参为1，定时时间2秒，并返回id
>
>     B) #if 0   //条件0执行修改后的定时时间500毫秒，条件1执行定时时间2秒
>          printf("timer_id = %d\r\n", timer_id);
>          #else
>            if(timer_id)
>          {
>              printf("timer_id = %d\r\n", timer_id);
>              usr_timer_modify(timer_id, 500);  //修改timer的定时时间为500毫秒
>           }
>           #endif
>
>     C )  每隔500毫秒的定时时间，则回调一次callback_example(1)
>
>     D）回调3次callback_example(1)后，执行sys_timer_del(timer_id);    
>
>     //删除timer，回调3次即删除timer
>
> 4. usr_timeout_test()入口：
>
>     A）timeout_id = usr_timeout_add((void* )1, callback_example, 2*1000, 1);    
>
>     //注册一个高精度timeout，定时时间2秒，并返回id，只回调一次
>
>     B）#if 0   //条件0执行修改后的定时时间500毫秒;条件1删除定时时间为2秒的timeout
>           printf("timeout_id = %d\r\n", timeout_id);
>
>     ​      usr_timeout_del(timeout_id);    //删除timeout
>
>     ​	  #else
>     ​      if(timeout_id)
>     ​     {
>     ​        printf("timeout_id = %d\r\n", timeout_id);
>     ​        usr_timeout_modify(timeout_id, 500);  //修改timeout的定时时间为500毫秒
>     ​     }
>     ​       #endif
>
>     C）一旦500毫秒的定时时间到，仅回调一次callback_example()
---

## 常见问题

> * sys_timer/sys_timeout与usr_timer/usr_timeout分别适用于哪些应用？
>
>   答：sys_timer 和sys_timeout适用于在任务中执行，对时间精度要求不高的应用，usr_timer 和usr_timeout 适用于对时间精度要求高的应用。
>
> * sys_timer/usr_timer 与 sys_timerout/usr_timerout 接口的区别？
>
>   答：sys_timer/usr_timer 与 sys_timerout/usr_timerout 接口区别在于 timeout 接口的回调只会被调一次，也就是设定一个未来的时间， 时间到了响应之后便结束这个定时器的生命周期。
>
> * sys_timer 与usr_timer同步异步问题 
>
>   答：sys_timer 由 systimer 线程提供时基，属于同步接口，也就是说在哪个线程 add 的 sys_timer，定时时基到了 systimer 线程会发事件通知对应 的 add 线程响应（回调函数被执行）；usr_timer 属于异步接口， add 的时候注册的回调函数将在硬件定时器中时基到时候被调用。
>
> * 函数usr_timer_add(void *priv, void (*func)(void *priv), u32 msec, u8 priority)中参数 priority（优先级）的问腿
>
>   答：usr_timer 的参数 priority（优先级）为 1，系统无法进入低功耗，参数 priority（优先级）为 0，系统低功耗会忽略该节拍，节 拍不会丢失，但是周期会变。
>
> * sys_hi_timer/sys_s_hi_timer与usr_timer等同问题
>
>   答：sys_hi_timer 等同为 priority 为 1 的 usr_timer， 在 include_lib/system/timer.h 被宏定义 
>
>   ​        sys_s_hi_timer 等同为 priority 为 0 的 usr_timer，在 include_lib/system/timer.h 被宏定义

## 参考文档

> * [系统定时器接口设计说明](../../../../doc/architure/系统定时器接口设计说明文档.pdf)

