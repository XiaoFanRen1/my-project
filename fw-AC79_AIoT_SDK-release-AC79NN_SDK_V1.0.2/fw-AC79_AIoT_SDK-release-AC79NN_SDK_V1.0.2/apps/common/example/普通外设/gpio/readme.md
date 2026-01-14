# GPIO示例工程说明

> 本工程展示了GPIO的使用方法。
>
> 演示GPIO输入,上下拉配置；GPIO输出,强驱配置；GPIO中断模式使用；特殊引脚如USB/配置为GPIO使用方法；双重绑定IO配置方法；无互斥高速度要求操作GPIO方法

---

## 适用平台

> 本工程适用以下芯片类型：
> 1. AC79系列芯片：AC790N、AC791N

> 杰理芯片和评估板的更多信息可在以下地址获取：[链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)

## 工程配置说明

> 在SDK选择其中一个*.cbp主工程文件或者主工程Makefile, 然后添加本事例工程代码
>
> app_config.h：
>
> * 选择对应硬件开发环境的板级文件

## 模块依赖

> * cpu.a 包含GPIO底层硬件收发库

---



### 操作说明：

> 1. 编译工程，烧录镜像，复位启动

> JIELI SDK的编译、烧写等操作方式的说明可在以下文档获取：[文档](../../../../doc/stuff/usb updater.pdf)

### 代码流程

> 1. main()入口：
>     A）对普通IO进行无互斥高速度配置,同一组引脚之间没有互斥, 针对用户有高速度要求, 并且不需要考虑同一组引脚之间操作互斥才允许使用
>     
>     B）对普通IO进行输出，强驱配置
>     
>     C）GPIO中断模式使用,上升沿高电平触发,串口打印“hello,jl”
>     
>     D）特殊引脚如USB/配置为GPIO使用方法
>     
>     E）双重绑定IO配置方法，将其中一个设为高阻态
>     
>     F）GPIO输入,上下拉配置
---

## 常见问题

> * 特殊引脚如USB/配置为GPIO时注意是否开启以下的宏
>
>   #define IO_PORT_USB_DPA              (IO_PR_MAX + 0)
>   #define IO_PORT_USB_DMA              (IO_PR_MAX + 1)
>   #define IO_PORT_USB_DPB              (IO_PR_MAX + 2)
>   #define IO_PORT_USB_DMB              (IO_PR_MAX + 3)

## 参考文档

> * N/A

