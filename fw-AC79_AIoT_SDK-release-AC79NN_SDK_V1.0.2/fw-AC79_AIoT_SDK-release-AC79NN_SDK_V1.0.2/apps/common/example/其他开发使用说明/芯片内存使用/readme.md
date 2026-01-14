﻿# 芯片内存使用使用示例工程说明

> 本工程展示了如何定义变量/函数到 flash/sdram/内部ram的方法

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

> * N/A
---

## 常见问题

> * 如何查看芯片内存使用情况?
>
>   答:系统开机以后会打印 芯片内存使用情况如下所示:
>
>   ```
>   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
>            setup_arch Jul 12 2021 10:07:49
>   sys_clk = 320000000,sdram_clk = 240000000,hsb_clk = 160000000,lsb_clk = 53333333, sfc_clk = 80000000
>   CODE+CONST SIZE = 1622496
>   SDRAM_SIZE = 2097152, DATA_SIZE = 45444,BSS_SIZE = 454944,REMAIN_SIZE = 1596764
>   RAM_SIZE = 523728,DATA_SIZE = 12624,BSS_SIZE = 499120,REMAIN_SIZE = 11984
>   CACHE_RAM_SIZE = 0,DATA_SIZE = 0, BSS_SIZE = 0,REMAIN_SIZE = 0
>   HEAP_SIZE = 1596704
>   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  
>   ```
>
>   CODE+CONST代表代码加const变量总大小
>
>   SDRAM_SIZE 代表SDRAM容量大小
>
>   RAM_SIZE 代表芯片内部RAM容量大小
>   
>   CACHE_RAM_SIZE 代表芯片内部一块小的,并且跟内部RAM不连续地址的额外可用大小
>   
>   DATA_SIZE 代表有初始化的变量大小
>   
>   BSS_SIZE 代表无初始化的变量大小
>   
>   REMAIN_SIZE 代表该区域剩余可用大小
>   
>   HEAP_SIZE 代表内存堆大小,通常是位于 SDRAM的 REMAIN_SIZE,如果没有SDRAM则位于内部RAM的REMAIN_SIZE
>
> 
>
> * 任务栈在什么内存上?
>
>   答:创建的任务如果是静态任务,栈位于全局变量所在内存; 如果是动态任务,栈位于堆内存
>
> 
>
> * 如何查看地址映射MAP文件
>
>   答:MAP文件位于对应cpu的tools目录下, 如 [sdk.map](../../../../cpu/wl80/tools/sdk.map)

## 参考文档

> * https://blog.csdn.net/bonson2004/article/details/83897458

