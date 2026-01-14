﻿﻿# 烧写文件到指定的flash地址说明

> 适用于只烧写文件到指定的flash地址，无需重新烧写整个固件

---

## 适用平台

> 本工程适用以下芯片类型：
> 1. AC79系列芯片：AC790N、AC791N
>
> 杰理芯片和评估板的更多信息可在以下地址获取：[链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)

---



### 操作说明：

> 1.修改[write_file_to_flash.bat](../../../../cpu/wl82/tools/write_file_to_flash.bat)，修改需要烧写的文件名称和flash地址。
>
> 2.进入烧写模式，显示盘符后，双击[write_file_to_flash.bat](../../../../cpu/wl82/tools/write_file_to_flash.bat)文件。

> JIELI SDK的编译、烧写等操作方式的说明可在以下文档获取：[文档](../../../../doc/stuff/usb updater.pdf)

---

## 常见问题

> * 如何获取需要烧写的预留区的flash地址？
>
>   答：参考[FLASH用户区域使用方法](../../../../apps/common/example/FLASH用户区域使用方法)
>

## 参考文档

> * N/A