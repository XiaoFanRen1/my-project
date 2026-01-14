# FLASH自定义读取寄存器数据说明

> 本工程展示了FLASH自定义读取寄存器数据使用示例

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

> 1. c_main()入口，参考本示例 [main.c](main.c) ：
>     A）应用层读取flash寄存器测试
>
> 2. API说明，：头文件#include "asm/sfc_norflash_api.h"
>
>     void norflash_enter_spi_code(void);//用户自定义操作flash的第一步骤
>
>     void norflash_exit_spi_code(void);//用户自定义操作flash的最后步骤
>
>     void norflash_spi_cs(char cs);//用户拉低或者拉高flash CS片选，0拉低，1拉高
>
>     void norflash_spi_write_byte(unsigned char data);//用户往flash写字节
>
>     u8 norflash_spi_read_byte(void);//用户从flash读字节
>
>     int norflash_wait_busy(void);//用户等flash忙结束，返回值0则不忙，返回非0则判忙等待超时（出现这种情况则说明flash出问题或者程序出问题了）
>
> 3. **注意事项，所有的调用上述函数应该统一放在一个函数里执行，且函数都要放在spi_code段，参考本示例 [main.c](main.c)** 
>
>     **在norflash_enter_spi_code和norflash_exit_spi_code框内：所有代码和数据和const数组和变量只能在sdram或者内部sram，数据和const数组和变量可以指定内部sram的SEC_USED(.sram)或者sdram的SEC_USED(.data)，如果系统的代码是跑flash（即打开了CONFIG_SFC_ENABLE宏），则不能加打印信息**。
---

## 常见问题

> * N/A

## 参考案例

> * N/A