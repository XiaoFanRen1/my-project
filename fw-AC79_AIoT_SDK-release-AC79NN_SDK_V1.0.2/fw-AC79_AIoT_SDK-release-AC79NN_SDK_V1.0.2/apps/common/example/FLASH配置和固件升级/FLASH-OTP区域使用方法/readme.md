﻿# FLASH-OTP区域使用示例工程说明

> 本工程展示了FLASH-OTP区域使用示例

---

## 适用平台

> 本工程适用以下芯片类型：
> 1. AC79系列芯片：AC790N、AC791N
>
> 杰理芯片和评估板的更多信息可在以下地址获取：
> [链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)

## 工程配置说明

> 在SDK选择[demo_hello](../../../../apps/demo_hello/board)主工程文件或者主工程Makefile, 然后添加本事例工程代码 [main.c](main.c) 。
>
> API说明：
>
> ```c
> int norflash_eraser_otp(void);//擦除
> int norflash_write_otp(u8 *buf, int len);//写，len<=768
> int norflash_read_otp(u8 *buf, int len);//读,len<=768
> ```
>
> **注意**：SDK的OTP读写最大字节768字节。

---



### 操作说明：

> 2. 编译工程，烧录镜像，复位启动
> 3. 系统启动后，可以通过串口软件看到示例的打印信息
>
> JIELI SDK的编译、烧写等操作方式的说明可在以下文档获取：[文档](../../../../doc/stuff/usb updater.pdf)

### 代码流程

> 1. c_main()入口：
>     A）对OPT区域进行擦除，然后进行写数据再读取返回打印对比
---

## 常见问题

> * N/A
>

## 参考案例

> * N/A

