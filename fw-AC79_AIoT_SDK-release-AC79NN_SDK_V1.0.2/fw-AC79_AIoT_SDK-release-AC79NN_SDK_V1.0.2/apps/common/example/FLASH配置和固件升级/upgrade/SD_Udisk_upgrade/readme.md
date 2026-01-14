﻿# SD卡/U盘升级示例工程说明

> 本工程展示了通过SD卡/U盘升级的使用方法。
>
> 通过检测SD卡/U盘中是否存在升级文件，如果存在便通过文件系统读取升级文件并写入升级区进行升级。

---

## 适用平台

> 本工程适用以下芯片类型：
>
> 1. AC79系列芯片：AC790N、AC791N

> 杰理芯片和评估板的更多信息可在以下地址获取：
> [链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)

## 工程配置说明

> 在SDK选择[PIC_BOOK_STORY](../../../../../apps/wifi_story_machine/board)工程CBP文件或者工程Makefile, 然后添加本事例工程代码
>
> app_config.h：
>
> * SD卡升级，需要定义#define TCFG_SD0_ENABLE   1
> * U盘升级，需要定义#define TCFG_UDISK_ENABLE  1
> * 如果采用双备份升级，需要定义#define CONFIG_DOUBLE_BANK_ENABLE   1
> * 如果采用单备份升级，需要定义#define CONFIG_DOUBLE_BANK_ENABLE   0

## 模块依赖

> * N/A

---



### 操作说明：

> 1. 需要相关硬件支持（USB接口和SD卡接口）
> 2. 将升级文件拷贝到SD卡或U盘中
> 3. 编译工程，烧录镜像，插上SD卡或U盘，复位启动
> 4. 系统启动后，会自动检测是否存在升级文件，存在则读取升级文件并写入升级区进行升级

> JIELI SDK的编译、烧写等操作方式的说明可在以下文档获取：
> [文档](/doc/stuff/usb%20updater.pdf)

### 控制命令

> * N/A

### 代码流程

> 1. c_main()入口：
>     A）创建任务fs_update_start
>
> 2. fs_update_start函数流程
>
>     A)通过storage_device_ready检测文件系统是否挂载完成。
>
>     B）成功挂载后，调用update_test
>
> 3. update_test任务流程
>
>     A)文件系统挂载成功后调用fs_update_check
>
> 4. fs_update_check函数流程
>
>     A）检测SD卡或U盘中是否存在升级文件，不存在则直接返回-1
>
>     B)  如果存在升级文件，则设置升级相关参数（读写操作集，升级过程状态回调函数等）
>
> 5. fs_update_state_cbk状态回调函数流程
>
>     A) 判断是否为UPDATE_CH_EXIT升级完成状态，是则复位启动
---

## 常见问题

> * 采用SD卡升级或U盘升级时打开升级文件失败？
>
>   答：确保SD卡或U盘已经挂载成功后才能调用文件系统接口。
>
> * 有些SD卡能挂载成功，有些却挂载不了？
>
>   答：不同类型的SD卡读写速率不一样，对于低速卡如果挂载不成功时，尝试调整对应的板级配置

## 参考文档

> * N/A