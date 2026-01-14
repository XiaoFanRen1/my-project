﻿# flash应用层用户使用API示例说明

> 本工程展示了flash应用层用户使用API接口示例。

---

## 适用平台

> 本工程适用以下芯片类型：
> 1. AC79系列芯片：AC790N、AC791N
>
> 杰理芯片和评估板的更多信息可在以下地址获取：
> [链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)

## 工程配置说明

> 在SDK选择[demo_hello](../../../../apps/demo_hello/board)主工程文件或者主工程Makefile, 然后添加本事例工程代码。
>
> 头文件：对应CPU目录下的 [sfc_norflash_api.h](..\..\..\..\include_lib\driver\cpu\wl80\asm\sfc_norflash_api.h) 。
>
> 1、flash应用层使用API接口。
>
> ```c
> //==========================================用户flash API函数说明=====================================================//
> /*norflash_open函数是初始化flash硬件，在用户使用flash的API接口首先需要打开才能使用flash API(多次打开没有影响)
> 参数：name、device、arg为NULL即可。
> */
> int norflash_open(const char *name, struct device **device, void *arg);
> 
> /*norflash_origin_read函数是直接SPI读取flash函数，用户应该使用的API接口。
> 当通过norflash_write函数写，则norflash_origin_read读取与写的相同，需要加解密则应用层用户自行处理。
> 参数：buf:数据地址，offsetf:flash地址，len:数据长度
> */
> int norflash_origin_read(u8 *buf, u32 offset, u32 len);
> 
> /*norflash_read函数是CPU读取flash，用户应使用该API接口时需要注意：读写地址小于VM区域时会做加解密处理。
> 参数：device为NULL，buf:数据地址，offsetf:flash地址，len:数据长度
> */
> int norflash_read(struct device *device, void *buf, u32 len, u32 offset);
> 
> /*norflash_write函数是直接SPI写flash函数，用户应该使用的API接口。
> 当通过norflash_write函数写，则norflash_origin_read读取与写的相同，需要加解密则应用层用户自行处理。
> 参数：device为NULL，buf:数据地址，offsetf:flash地址，len:数据长度
> */
> int norflash_write(struct device *device, void *buf, u32 len, u32 offset);
> 
> /*norflash_ioctl，flash操作命令接口
> 参数：device为NULL
> cmd：
> IOCTL_GET_CAPACITY，获取flash容量（flash实际物理容量大小），arg为容量缓冲区的地址
> IOCTL_ERASE_SECTOR，擦除扇区命令，arg为擦除扇区地址
> IOCTL_GET_ID，获取flash ID命令，arg为ID缓冲区的地址
> IOCTL_GET_UNIQUE_ID，获取flash的UUID，arg为UUID缓冲区的地址，地址可用大小为16字节
> IOCTL_SET_WRITE_PROTECT，flash的写保护参数，arg=0（默认0）则解除写保护
> IOCTL_GET_WRITE_PROTECT_VALUE，获取当前写保护寄存器1和2的值，返回值：寄存器2在8-15位，寄存器1在0-7位。
> */
> int norflash_ioctl(struct device *device, u32 cmd, u32 arg);
> 
> 
> //=======================================用户flash API读写函，用户可直接使用===================================================//
> //应用层用户始化flash硬件接口
> void flash_read_write_init(void);
> 
> //应用层用户读写API，使用该函数前，先调用norflash_open函数初始化flash硬件接口
> int flash_write_buff(u8 *buff, u32 addr, u32 len);
> 
> //应用层用户读写API，使用该函数前，先调用norflash_open函数初始化flash硬件接口
> int flash_read_buff(u8 *buff, u32 addr, u32 len);
> ```
>
> 
>
> 2、**注意事项：**应用层使用norflash_read函数读取flash时系统会进行解密，因此在读之前需要保证：写flash时候加密，否则读取数据会出错；当使用
>
> norflash_origin_read函数读取，无需加解密（如果需要加解密，此时加解密由用户自己完成）。**注意flash的写保护区域，详情 [readme.md](..\FLASH写保护参数使用说明\readme.md) 。**
>
> 写flash前对数据进行加密函数使用例子：
>
> > ```c
> > //==================flash写地址小于VM区，norflash_read函数读取的数据需要进行先加密写进=========================//
> > void doe(u16 k, void *pBuf, u32 lenIn, u32 addr);//系统的加密函数
> > u32 get_system_enc_key(void);//系统加密秘钥
> > u32 get_norflash_vm_addr(void);//获取VM地址，VM长度为isd_config.ini文件配置的VM_LEN
> > 
> > //写地址小于VM区，数据进行加密，norflash_read读取正确
> > //当写flash不加密时，读取只能用norflash_origin_read
> > void flash_data_doe(u8 *buf, int len)
> > {
> >     doe(get_system_enc_key(), buf, len, 0);
> > }
> > ```
>
> 3、测试程序代码如下。
>
> ```c
> #include "app_config.h"
> #include "system/includes.h"
> #include "fs/fs.h"
> #include "asm/sfc_norflash_api.h"
> 
> static void user_flash_test(void)
> {
>     void flash_test(void);
>     void flash_read_write_init(void);
>     int flash_write_buff(u8 *buff, u32 addr, u32 len);
>     int flash_read_buff(u8 *buff, u32 addr, u32 len);
> 
> #if 0
>     flash_test();//flash测试，flash_test函数可以测试起始地址到结束地址
> #else
>     //1.初始化flash硬件，并读取ID、容量等
>     flash_read_write_init();//初始化flash硬件，并读取ID、容量等
> 
>     unsigned char *buff = malloc(4096);
>     if(buff){//4096字节读写测试
>         for (int i = 0; i < 4096; i++) {
>             buff[i] = (unsigned char)i;
>         }
>         //2.写数据
>         flash_write_buff(buff, 2*1024*1024, 4096);//2M地址起始测试，写
>         memset(buff, 0, 4096);
> 
>         //3.读数据
>         flash_read_buff(buff, 2*1024*1024, 4096);//2M地址起始测试，读
>         for (int i = 0; i < 4096; i++) {
>             if (buff[i] != (unsigned char)i) {//校验
>                 printf("user_flash_test test err buff[%d] = %d \n",i,buff[i]);
>                 break;
>             }
>         }
>         free(buff);
>     }
>     printf("---> user_flash_test over \n");
> #endif
>     while (1) {
>         os_time_dly(10);
>     }
> }
> static int c_main(void)
> {
>     os_task_create(user_flash_test, NULL, 12, 1000, 0, "user_flash_test");
>     return 0;
> }
> late_initcall(c_main);
> 
> ```

---



## 操作说明

> 2. 编译工程，烧录镜像，复位启动
> 3. 系统启动后，可以通过串口软件看到示例的打印信息
>
> JIELI SDK的编译、烧写等操作方式的说明可在以下文档获取：[文档](../../../../doc/stuff/usb updater.pdf)

---

## 常见问题

> * 写入和读出数据不对？
>
>      答：写之前擦除对应扇区，确定写入地址和读取地址不在加密区域，对换读函数norflash_read、norflash_origin_read试试，
>
>      **注意flash的写保护区域，详情 [FLASH写保护参数使用说明](..\FLASH写保护参数使用说明\readme.md) 
>      
> * AC79 最大支持多大容量的FLASH?
>
>      答:最高支持128MB的flash,但是代码体积不能够超过16MB,并且如果使用大于16MB 容量的flash, 资源只能够配置单备份

## 参考案例

> * N/A