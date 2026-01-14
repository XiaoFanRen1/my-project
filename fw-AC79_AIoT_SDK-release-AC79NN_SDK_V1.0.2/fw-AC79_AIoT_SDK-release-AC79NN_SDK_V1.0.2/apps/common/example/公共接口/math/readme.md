4

> 本工程展示了FFT接口使用示例:
>
> 1. FFT和IFFT的使用
>

---

## 适用平台

> 本工程适用以下芯片类型：
> 1. AC79系列芯片：AC790N、AC791N
>
> 杰理芯片和评估板的更多信息可在以下地址获取：[链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)

## 工程配置说明

> 如果设备没有启动WIFI功能或者不在STA模式,在SDK选择[demo_hello](../../../../apps/demo_hello/board)主工程文件或者主工程Makefile, 然后添加本事例工程代码
>
> 1.需增加依赖文件
>
> | 文件                     | 用途                   |
> | ------------------------ | ---------------------- |
> | flfft_core_pi32v2.c      | 浮点fft和ifft运算      |
> | jl_fft.c                 | 硬件fft接口            |
> | jl_math.c                | 实现向量加减乘除       |
> | nn_function_vec.c        | 实现个位宽向量加减乘除 |
> | driver/cpu/math.c(cpu.a) | 三角函数相关           |
>
> 
>
> 2.其他数学函数参考对应cpu目录下的 [math.h](../../../../include_lib/driver/cpu/wl80/asm/math.h)

---



### 操作说明：

> 2. 编译工程，烧录镜像，复位启动
> 3. 系统启动后，可以通过串口软件看到示例的打印信息
>
> JIELI SDK的编译、烧写等操作方式的说明可在以下文档获取：[文档](../../../../doc/stuff/usb updater.pdf)

### 代码流程

> 1. c_main()入口：
>     A）FFT 和IFFT测试
>
> 
---

## 常见问题

> * N/A

## 参考文档

> * N/A