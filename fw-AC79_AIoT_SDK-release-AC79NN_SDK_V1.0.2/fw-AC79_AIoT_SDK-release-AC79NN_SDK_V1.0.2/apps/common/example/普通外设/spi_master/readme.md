# SPI主机使用示例工程说明

> 本工程展示了spi主机接口使用示例:
>
> 1. 设置spi主机参数。
> 2. 主spi设备收发数据。

---

## 适用平台

> 本工程适用以下芯片类型：
> 1. AC79系列芯片：AC790N、AC791N
>
> 杰理芯片和评估板的更多信息可在以下地址获取：[链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)

## 工程配置说明

> 在SDK选择[demo_hello](../../../../apps/demo_hello/board)或者其他demo的主工程文件或者主工程Makefile, 然后添加本事例工程代码
>
> 1. board.c 添加SPI配置参数
>
> ```c
> //1 添加SPI硬件参数，标准spi协议mosi miso clk 
> SPI1_PLATFORM_DATA_BEGIN(spi1_data)//当使用SPI2则：SPI2_PLATFORM_DATA_BEGIN(spi2_data)
> 	.clk    = 24000000, //主机spi时钟
> 	.mode   = SPI_STD_MODE,//spi数据线传输:mosi miso clk 
> 	.port   = 'B',//SPI接口
> 	.attr	= SPI_SCLK_H_UPL_SMPH,//参数：默认为主机，CLK高 更新数据高。
> SPI1_PLATFORM_DATA_END()//当使用SPI2则：SPI2_PLATFORM_DATA_END()
> 
> //2 设备列表添加SPI设备
> {"spi1", &spi_dev_ops, (void *)&spi1_data },
> ```
>
> **说明**：一般用户可用SPI1和SPI2，SPI0为系统使用的flash。
>
> A）参数.clk：spi时钟，最高为lsb_clk时钟。
>
> B）参数.port：SPI硬件接口，**注意数据手册的SPI的spi_do   spi_di   spi_clk即为标准的MOSI  MISO  CLK**。
>
> /-----------------------SPI1 ---------------/
>
> //port   spi_do   spi_di   spi_clk
>
> ​    A       PB7       PB5       PB6
>
> ​    B       PC10     PC8       PC9
>
> /-----------------------SPI2 ---------------/
>
> //port   spi_do   spi_di   spi_clk
>
> ​    A       PH1       PH2       PH0
>
> ​    B       USBDMA   PG15   USBDPA
>
> C）参数.mode：SPI_STD_MODE：默认的标准spi模式：mosi miso clk 。
>
> D）参数.attr：
>
> 参数1主从模式：从机模式SPI_MODE_SLAVE（默认0则主机模式）
>
> 参数2采样模式：SPI_SCLK_H_UPL_SMPH 为CLK空闲H高电平，上升沿更新数据，下降沿采样数据；其他采样如下，详情对应CPU下的 [spi.h](..\..\..\..\include_lib\driver\cpu\wl80\asm\spi.h) 
>
> SPI_SCLK_H_UPH_SMPL	 	CLK空闲H,上升沿更新数据，下降沿采样数据
> SPI_SCLK_H_UPL_SMPH	 	CLK空闲H,下降沿更新数据，上升沿采样数据
> SPI_SCLK_H_UPH_SMPH	 	CLK空闲H,上升沿更新数据，上升沿采样数据
> SPI_SCLK_H_UPL_SMPL	 	CLK空闲H,下降沿更新数据，下降沿采样数据
> SPI_SCLK_L_UPH_SMPL	 	CLK空闲L,上升沿更新数据，下降沿采样数据
> SPI_SCLK_L_UPL_SMPH	 	CLK空闲L,下降沿更新数据，上升沿采样数据
> SPI_SCLK_L_UPH_SMPH	 	CLK空闲L,上升沿更新数据，上升沿采样数据
> SPI_SCLK_L_UPL_SMPL	 	CLK空闲L,下降沿更新数据，下降沿采样数据
>
>  其他参数可参考从机模式[readme.md](..\spi_slave\readme.md) . 
>
> 2. 测试代码
>
> ```c
> #include "app_config.h"
> #include "system/includes.h"
> #include "device/device.h"
> #include "asm/spi.h"
> #include "device/gpio.h"
> 
> 
> #define SPI_CS  IO_PORTA_00
> 
> 
> //定义中断回调函数,发送完成和接收完成会回调该函数
> void spi_irq_cb(void *priv, u8 **data, u32 *len)
> {
>     putchar('@');
> }
> 
> void spi_cs(char val)
> {
>     gpio_direction_output(SPI_CS, val);
> }
> 
> static void spi_test_task(void *p)
> {
>     char *buf = NULL;
> 	void *spi_hdl = NULL;
> 	int ret;
> 
>     //1.打开spi设备
>     spi_hdl = dev_open("spi1", NULL);
>     if (!spi_hdl) {
>         printf("spi open err \n");
>         goto exit;
>     }
>     //2.注册回调函数
>     struct spi_cb cb;
>     cb.cb_func = spi_irq_cb;//回调函数
>     cb.cb_priv = NULL;//回调函数的第一个私有参数
>     dev_ioctl(spi_hdl, IOCTL_SPI_SET_CB, (u32)&cb);
>     dev_ioctl(spi_hdl, IOCTL_SPI_SET_USE_SEM, 0);//等待数据时，用信号量等待，不用则为抢占查询硬件中断标记，建议应信号量等待
> 
>     //3.发送一个字节，有发送字节的board.c需配置为双向莫模式（使用DO和DI数据线）
>     u8 data = 0x12;
>     spi_cs(0);
>     dev_ioctl(spi_hdl, IOCTL_SPI_SEND_BYTE, data);//发送一个字节
>     spi_cs(1);
>     os_time_dly(100);
> 
>     //4.读取一个字节
>     spi_cs(0);
>     data = dev_ioctl(spi_hdl, IOCTL_SPI_READ_BYTE, 0);//读取一个字节
>     spi_cs(1);
>     os_time_dly(100);
> 
>     buf = malloc(1024);
>     if(!buf){
>         printf("err spi no buf \n");
>         goto exit;
>     }
> 
>     //4.发送1024字节
>     spi_cs(0);
>     ret = dev_write(spi_hdl, buf, 1024);
>     spi_cs(1);
>     os_time_dly(100);
> 
>     while (1) {
>         spi_cs(0);
>         ret = dev_read(spi_hdl, buf, 1024);
>         spi_cs(1);
>         if (ret == 0) {//返回值=0则已经读取到数据
>             //6. 数据存在buf，长度为1024
>             //todo
>             put_buf(buf, 1024);//打印数据
>         }
>         os_time_dly(100);
>     }
> 
> exit:
>     if(buf){//释放内存
>         free(buf);
>     }
>     if(spi_hdl){//关闭spi设备
>         dev_close(spi_hdl);
>     }
>     while(1){//等待其他任务杀此任务
>         os_time_dly(100);
>     }
> }
> 
> static int c_main(void)
> {
>     os_task_create(spi_test_task, NULL, 22, 1000, 0, "spi_test_task");
>     return 0;
> }
> late_initcall(c_main);
> 
> ```
>
> **2、注意事项**
>
> A）WL80(790x系列) DMA最大字节数的SPI_MAX_SIZE，当芯片为AC790N则SPI_MAX_SIZE = 65535，当芯片为AC791N则SPI_MAX_SIZE = 2^31，因此在DMA接收和发送是需注意DMA数据量不能超过最大值。 WL82(7910) 系列DMA支持超64k读写数据 
>
> B）主机模式硬件也不支持CS拉，CS需要用户自行加代码，参考测试程序代码。
>
> C）使用时需要注意调用SPI底层驱动互斥问题。

## 操作说明

> 1. 编译工程，烧录镜像，复位启动
>
> 2. 系统启动后，可以通过串口软件看到示例的打印信息
>
> JIELI SDK的编译、烧写等操作方式的说明可在以下文档获取：[文档](../../../../doc/stuff/usb updater.pdf)

## 常见问题

> * N/A

## 参考文档

> * N/A