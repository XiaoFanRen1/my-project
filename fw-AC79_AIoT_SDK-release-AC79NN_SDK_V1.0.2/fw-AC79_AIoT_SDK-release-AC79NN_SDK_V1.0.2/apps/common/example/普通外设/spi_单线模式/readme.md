# SPI单线模式使用示例工程说明

> 本工程展示了spi用作单线模式：
>
> 1. 设置spi单线模式参数。
> 2. 从spi设备发送数据。
> 3. 测试请参考st7789v驱动代码，SPI单线主要用于推屏

---

## 适用平台

> 本工程适用以下芯片类型：
> 1. AC79系列芯片：AC790N、AC791N
>
> 杰理芯片和评估板的更多信息可在以下地址获取：[链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)

## 工程配置说明

在SDK选择[demo_hello](../../../../apps/demo_hello/board)或者其他demo的主工程文件或者主工程Makefile, 然后添加本事例工程代码

1. board.c 添加SPI配置参数

```c
//1 添加SPI硬件参数，标准spi协议mosi miso clk 
SPI1_PLATFORM_DATA_BEGIN(spi1_data)//当使用SPI2则：SPI2_PLATFORM_DATA_BEGIN(spi2_data)
	.clk    = 80000000, //时钟，可设置，无影响，时钟从机模式跟随主机模式
	.mode   = SPI_1WIRE_MODE,//spi数据线传输:mosi miso clk 
	.port   = 'B',//SPI接口
	.attr	= SPI_SCLK_H_UPL_SMPH | SPI_UNIDIR_MODE,//参数：从机，CLK高 更新数据高，单向模式
SPI1_PLATFORM_DATA_END()

SPI1_PLATFORM_DATA_BEGIN(spi1_data)//当使用SPI2则：SPI2_PLATFORM_DATA_BEGIN(spi2_data)
	.clk    = 80000000, //时钟，可设置，无影响，时钟从机模式跟随主机模式
	.mode   = SPI_1WIRE_MODE,//spi数据线传输:mosi miso clk 
	.port   = 'B',//SPI接口
	.attr	= SPI_SCLK_H_UPL_SMPH | SPI_UNIDIR_MODE,//参数：从机，CLK高 更新数据高，单向模式
SPI1_PLATFORM_DATA_END()
   
//2 设备列表添加SPI设备
{"spi1", &spi_dev_ops, (void *)&spi1_data },
```

说明：

A）参数.clk：spi时钟，从机模式则跟随主机。

B）参数.port：SPI硬件接口，**注意数据手册的SPI的spi_do   spi_di   spi_clk即为标准的MOSI  MISO  CLK**。

/-----------------------SPI1 ---------------/

SPI1   A组  	PB5(DI)     PB7(DO)  

​		   B组      PC9(CLK)    PC10(DO)

/-----------------------SPI2 ---------------./

SPI2   A组  	PH0(CLK)     PH1(DO)  

​		   B组      USBDPA(CLK)    USBDPB(DO)

C）参数.mode：SPI_1WIRE_MODE  一线模式同一时刻只能为 一个模式 采用dev_read 或者 dev_wirte 读写

D）参数.attr：

参数1主从模式：从机模式SPI_MODE_SLAVE（默认0则主机模式）

参数2采样模式：SPI_SCLK_H_UPL_SMPH 为CLK空闲H高电平，上升沿更新数据，下降沿采样数据；其他采样如下，详情对应CPU下的 [spi.h](..\..\..\..\include_lib\driver\cpu\wl80\asm\spi.h) 

SPI_SCLK_H_UPH_SMPL	 	CLK空闲H,上升沿更新数据，下降沿采样数据
SPI_SCLK_H_UuPL_SMPH	 	CLK空闲H,下降沿更新数据，上升沿采样数据
SPI_SCLK_H_UPH_SMPH	 	CLK空闲H,上升沿更新数据，上升沿采样数据
SPI_SCLK_H_UPL_SMPL	 	CLK空闲H,下降沿更新数据，下降沿采样数据
SPI_SCLK_L_UPH_SMPL	 	CLK空闲L,上升沿更新数据，下降沿采样数据
SPI_SCLK_L_UPL_SMPH	 	CLK空闲L,下降沿更新数据，上升沿采样数据
SPI_SCLK_L_UPH_SMPH	 	CLK空闲L,上升沿更新数据，上升沿采样数据
SPI_SCLK_L_UPL_SMPL	 	CLK空闲L,下降沿更新数据，下降沿采样数据

参数3单双向模式：SPI_UNIDIR_MODE为单向：只用SPI的DO一根线数据传输（一般只用于作为从机接收数据，不能发送数据），SPI_BIDIR_MODE为双向：用SPI的DO/DI两根线数据传输，从机单双向都可用（具体看实际应用）。

**2、注意事项**

*）WL80(790x系列) DMA最大字节数的SPI_MAX_SIZE，当芯片为AC790N则SPI_MAX_SIZE = 65535，当芯片为AC791N则SPI_MAX_SIZE = 2^31，因此在DMA接收和发送是需注意DMA数据量不能超过最大值。 WL82(7910) 系列DMA支持超64k

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