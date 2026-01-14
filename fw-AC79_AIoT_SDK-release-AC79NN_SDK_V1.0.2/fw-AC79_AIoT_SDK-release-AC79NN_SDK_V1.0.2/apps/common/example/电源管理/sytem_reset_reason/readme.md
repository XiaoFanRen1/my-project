#   系统复位原因使用示例工程说明

> 本工程展示了系统复位使用方法。
>

---

## 适用平台

> 本工程适用以下芯片类型：
>
> 1. AC79系列芯片：AC790N、AC791N
>
> 杰理芯片和评估板的更多信息可在以下地址获取：[链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)
## 工程配置说明

在SDK选择[demo_hello](../../../../apps/demo_hello/board)主工程文件或者主工程Makefile, 编译直接运行，依赖文件为 [system_reset_reason.c](..\..\system\system_reset_reason.c) 和对应CPU的目录下 [system_reset_reason.h](..\..\..\..\include_lib\driver\cpu\wl80\asm\system_reset_reason.h) 。

> 1、使用说明
>
> 系统上电运行后自动调用system_reset_reason_get();来获取系统复位信息，同时也打印出系统复位原因，例如：
> ========= system reset reason: SOFT ========= 
>
> 2、系统复位原因
>
> system_reset_reason_get()函数返回值对应的位表示不同的复位原因，可获取复位原因后做其他的操作（比如IO唤醒则检测是不是长按还是短按，或者闹钟唤醒时更改闹钟时间等），对应CPU的头文件 [system_reset_reason.h](..\..\..\..\include_lib\driver\cpu\wl80\asm\system_reset_reason.h) 。
>
> enum sys_reset_type {
>     SYS_RST_NONE = 0,
>     SYS_RST_12V = BIT(1),//1.2V内核复位
>     SYS_RST_WDT = BIT(2),//看门狗复位
>     SYS_RST_VCM = BIT(3),//复位键复位
>     SYS_RST_SOFT = BIT(4),//软件复位
>     SYS_RST_ALM_WKUP = BIT(5),//闹钟唤醒复位
>     SYS_RST_PORT_WKUP = BIT(6),//IO唤醒复位
>     SYS_RST_LONG_PRESS = BIT(7),//长按复位
>     SYS_RST_VDDIO_PWR_ON = BIT(8),//上电复位
>     SYS_RST_VDDIO_LOW_PWR = BIT(9),//低电压复位
> };
>
> **复位原因解析：**
>
> 1.2V内核复位：一般不会出现。
>
> 看门狗复位：程序跑飞或者程序死循环导致不能清狗。
>
> 复位键复位：芯片的VCM复位引脚被置高电平（3.3V左右）。
>
> 软件复位：系统调用cpu_reset()或者system_reset()。
>
> 闹钟唤醒复位：使用RTC并且使能闹钟唤醒功能后软关机到达唤醒时间。
>
> IO唤醒复位：软关机后IO被触发唤醒。
>
> 上电复位：一般正常通电启动就是上电复位。
>
> 低电压复位：电压不足。
## 操作说明

> 1. 使用串口线连接打印
> 2. 编译工程，烧录镜像，复位启动
> 3. 系统启动后，查看系统复位原因。
>
> JIELI SDK的编译、烧写等操作方式的说明可在以下文档获取：[文档](../../../../doc/stuff/usb updater.pdf)
---

## 常见问题

> * N/A

## 参考文档

> * N/A