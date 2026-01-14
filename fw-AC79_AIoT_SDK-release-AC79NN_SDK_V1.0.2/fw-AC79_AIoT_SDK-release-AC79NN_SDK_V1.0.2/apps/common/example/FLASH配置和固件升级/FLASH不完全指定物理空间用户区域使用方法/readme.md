# FLASH不完全指定物理空间用户区域使用示例说明

> 本工程展示了FLASH不完全指定物理空间用户使用示例。
>
> **FLASH不完全指定物理空间用户区域是指**：用户需要预留出**flash的最后部分空间**为用户自己使用空间，该空间与系统、工具毫无相关，分配给系统的flash大
>
> 小不是flash的实际物理空间大小。
>
> **使用情况**：当用户需要一块和系统、工具等毫无相关的flash区域时，可以开启FLASH不完全指定物理空间用户区域使用方法。

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
> **注意事项：**本次示例的不完全指定物理空间用户区域使用方法 和 FLASH用户区域的（对应cpu的tools目录下）[isd_config](../../../../cpu/wl80/tools/isd_config.ini)文件中EXIF_ADR项有本质区别。
>
> * FLASH用户区域方法的EXIF_ADR是工具会根据当前代码资源等情况在FLASH靠后分配一块可用大小的空间，系统运行后会分配用户区域给用户使用。
>
> * FLASH不完全指定物理空用户区域使用方法是和工具和系统毫无关联，系统运行后不会操作该空间，同时当在[isd_config](../../../../cpu/wl80/tools/isd_config.ini)（对应CPU路径）文件中
>
>   EXIF_ADR配置使用，则用户同时可以使用EXIF_ADR区域。
>
> 比如：flash大小为16M，用户需要flash的最后4M地址不受系统相关限制且随意由用户自己掌控，则配置如下：
>
> 1、指定系统使用相关的物理空间，预留部分空间给用户，则在对应的CPU目录下的 [isd_config.ini](..\..\..\..\cpu\wl80\tools\isd_config.ini) 文件改"FLASH_SIZE=12M;"，即系统使用12M，剩下4M给用户使用。
>
> 2、flash使用API接口。
>
> ```c
> 
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
> 当通过norflash_write函数写，则norflash_origin_read读取与写的相同，需要加解密则应用层用户自行处理。FLASH不完全指定物理空间用户区域禁止使用。
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
> IOCTL_SET_WRITE_PROTECT，解除flash的写保护，arg=0（默认0）
> */
> int norflash_ioctl(struct device *device, u32 cmd, u32 arg);
> 
> 
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
> 3、main.c测试14M起始地址程序代码如下。
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
>         flash_write_buff(buff, 14*1024*1024, 4096);//14M地址起始测试，写
>         memset(buff, 0, 4096);
> 
>         //3.读数据
>         flash_read_buff(buff, 14*1024*1024, 4096);//14M地址起始测试，读
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
>
> **注意：**FLASH不完全指定物理空间用户区域**禁止使用** norflash_read 读函数。只能使用：norflash_origin_read。

---



## 操作说明

> 2. 编译工程，烧录镜像，复位启动
> 3. 系统启动后，可以通过串口软件看到示例的打印信息
>
> JIELI SDK的编译、烧写等操作方式的说明可在以下文档获取：[文档](../../../../doc/stuff/usb updater.pdf)

---

## 常见问题

> * FLASH不完全指定物理空间用户区域使用方法和FLASH用户区域的[isd_config](../../../../cpu/wl80/tools/isd_config.ini)（对应CPU路径）文件中EXIF_ADR项有什么区别？
>
>      ①FLASH用户区域方法[readme.md](..\FLASH用户区域使用方法\readme.md) 的EXIF_ADR是工具会根据当前代码资源等情况在FLASH靠后分配一块可用大小的空间，系统运行后会分配用户区域给
>
>      用户使用。 
>
>      ②FLASH不完全指定物理空间用户区域使用方法是和工具和系统毫无关联，系统运行后不会操作改空间，同时当在[isd_config](../../../../cpu/wl80/tools/isd_config.ini)（对应CPU路径）文件中
>
>      EXIF_ADR配置使用，则用户同时可以使用EXIF_ADR区域。

## 参考案例

> * N/A