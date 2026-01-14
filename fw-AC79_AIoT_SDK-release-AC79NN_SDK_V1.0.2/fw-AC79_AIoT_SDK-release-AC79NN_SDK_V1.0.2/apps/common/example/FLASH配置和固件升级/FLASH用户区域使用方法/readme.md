﻿# FLASH用户区域使用示例工程说明

> 本工程展示了FLASH用户区域使用示例

---

## 适用平台

> 本工程适用以下芯片类型：
> 1. AC79系列芯片：AC790N、AC791N
>
> 杰理芯片和评估板的更多信息可在以下地址获取：
> [链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)

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
>     A）获取用户可用空间flash地址和大小
>     
>     B ) 擦除读写flash地址空间测试
---

## 常见问题

> * 如何修改用户flash可用空间大小?
>
>      答:找到对应cpu的tools目录下的[isd_config](../../../../cpu/wl82/tools/isd_config.ini)文件,然后修改如下配置的EXIF_LEN
>
>      ​	#保留客户使用
>      ​	EXIF_ADR=AUTO;
>      ​	EXIF_LEN=0x1000;
>      ​	EXIF_OPT=1;
>
> * 用户flash可用空间地址如何指定?
>
>      答:不能够强制指定,工具会根据当前代码资源等情况在FLASH靠后分配一块可用大小的空间, 如果空间不够编译的时候会报错
>
> * 用户如何对用户区域进行配置？
>
>      答：请参考[资源配置例子](../资源配置例子)

## 参考案例

> * [FlashDB 使用例程](../../../../lib/Third_party_Library/FlashDB)
> * [Littlefs 使用例程](../../../../lib/Third_party_Library/littlefs)
> * [资源配置例子](../资源配置例子)

