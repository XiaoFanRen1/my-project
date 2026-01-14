# 硬件定时器使用示例工程说明

> 本工程展示了系统时间接口使用示例:
>
> 1. 硬件定时器;

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

> 1. c_main()入口：
>     
>     A）timer_clk = clk_get("timer");//先获取定时器的时钟源
>     
>     B）request_irq(timer_irq, 3, timer_isr, 0);//注册中断函数定和中断优先级
>     
>     C ) timer_set(50 * 1000); //初始化50ms进一次中断
>     
>     D) os_time_dly(500);//延时5s后，timer4_stop();//定时器停止
>     
>     E)  os_time_dly(500);//延时5s后，timer4_run(2);//定时器开始
>     
>     F) os_time_dly(500);//延时5s后，test(); //输入捕获功能测试
>     
> 2. test()入口：
>
>     A）timer5_init(1);//定时器5初始化，1代表下降沿捕获
>
>     B）while（1）//进入while循环中
>
> 3. while(1):
>
>     A) timer5_start();//清空数据
>
>     B) while(!timer5_interrupt())
>
>     ​	{	os_time_dly(20);	}//检测到有下降沿则为true,采用轮询方式每200ms检测一次
>
>     C ) printf("-------%s-------%d get cnt = %d\r\n",__func__,__LINE__,timer5_get_cnt());//返回出来的    	  是单位是10us 的cnt  就是 cnt* 100 000就是秒
---

## 常见问题

> * 定时器频率比较高，中断响应慢，或者使得系统变慢。
>
>   答:中断函数和中断函数调用函数全部指定在内部sram，指定方法：在函数定义加上SEC_USED(.volatile_ram_code)。
>
>   建议中断时间<=1ms使用这种方法，以便提高整个系统效率。
>   
>   例如：static SEC_USED(.volatile_ram_code)  void timer_cfg(u32 freq, u32 us)

## 参考文档

> * N/A