#   LCD屏幕接口使用示例工程说明

> 本工程展示了屏幕驱动模块接口使用方法。
>

---

## 适用平台

> 本工程适用以下芯片类型：
>
> 1. AC79系列芯片：AC790N、AC791N
>
> 杰理芯片和评估板的更多信息可在以下地址获取：[链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)
>
> 
## 工程配置说明

在SDK选择[demo_hello](../../../../apps/demo_hello/board)主工程文件或者主工程Makefile, 然后添加本事例工程代码。

**EMI驱动的屏幕:**

协议类型：并行8/16/32bitDATA+WR，数据类型：由屏幕确定，硬件负责发数据，不分数据类型，数据量：应用层决定，不确定字节，数据由应用层发送。

**PAPI驱动的屏幕：**

协议类型：并行8/16bitDATA+WR+RD，数据类型：由屏幕确定，硬件负责发数据，不分数据类型，数据量：应用层决定，不确定字节，数据由应用层发送。

**IMD驱动的屏幕：**

协议类型：并行8/6bitDATA+HSYNC+VSYNC+DE，数据类型：RGB888/666，数据量：with x heigh x 3字节(RGB666则1字节剔除低2位发送高6位)，数据由硬件

自动发送，应用层只能修改缓冲区。



> **1、EMI接口**
>
> EMI接口是专为8bit、16bit、32bit的并行数据通信接口。EMI的结构体详情对应CPU的emi.h。
>
> （1）board.c添加emi设备应用层结构体、添加到设备列表、头文件#include "asm/emi.h"
>
> ```c
> /***********************下列为：AC790N（WL80）***********************/
> //emi设备应用层的硬件相关配置
> static const struct emi_platform_data emi_data = {
> 	.bits_mode	= EMI_8BITS_MODE,   //并行接口数据位，固定IO，详情查看硬件数据手册，EMI_8BITS_MODE、EMI_16BITS_MODE、EMI_32BITS_MODE
> 	.baudrate	= EMI_BAUD_DIV2,	//时钟 = LSB_CLK / (baudrate + 1) , LSB分频
> 	.colection	= EMI_FALLING_COLT,	//从设备边缘采集数据：EMI_FALLING_COLT / EMI_RISING_COLT : 下降沿 上升沿 采集数据
> };
> 
> /***********************下列为：AC791N（WL82）***********************/
> //emi设备应用层的硬件相关配置
> static const struct emi_platform_data emi_data = {
> 	.bits_mode		= EMI_8BITS_MODE, //并行接口数据位，固定IO，详情查看硬件数据手册，EMI_8BITS_MODE、EMI_16BITS_MODE、EMI_32BITS_MODE
> 	.baudrate		= EMI_BAUD_DIV2,		//时钟 = HSB_CLK / (baudrate + 1) , HSB分频
> 	.colection		= EMI_FALLING_COLT,		//从设备边缘采集数据：EMI_FALLING_COLT / EMI_RISING_COLT : 下降沿 上升沿 采集数据
> 	.time_out		= 1*1000,				//最大写超时时间ms
> 	.data_bit_en 	= 0,					//数据位使能输出位：0默认根据bits_mode数据位来配置
> 	.th				= EMI_TWIDTH_NO_HALF,	//具体看emi.h
> 	.ts				= 0,					//具体看emi.h
> 	.tw				= (EMI_BAUD_DIV > 1) ? EMI_BAUD_DIV / 2 : 1,//具体看emi.h
> };
> 
> //设备列表添加emi设备
> { "emi",   &emi_dev_ops, (void *)&emi_data},
> 
> 
> /***************************************时序图*************************************************/
> /*注意：
> AC790N（WL80）不能调节WR信号的脉宽，使用时需注意从设备是否能采集WR信号（脉宽信号为几个emi的时钟）。
> AC791N（WL82）可调节WR信号的脉宽。
> 
> timing_setup(tw) timing_half(th) timing_width(tw):数据传输配置
> 写时钟信号(极性):AC791N（WL82）才可以修改tw时间宽度
>                        __________               _________
> wr =0: ____________|          |_____________|         |__________
> wr =1: ____________            _____________           __________
>                       |__________|             |_________|
> 数据信号:
>                 ________________________                       ______
> data:  _____|                        |_____________________|
> 		           |          |
> ts,tw:      |      |          |      |
> 		    |<-ts->|<---tw--->|      |
> 		    |      |          |      |
> 			|                        |
> 			|<---hsb/(baudrate+1)--->|
> 			|                        |
> 			|                        |
> 			|                        |
> ts:0-255,ts个hsb时钟宽度
> tw:0-255,tw个hsb时钟宽度
> th:0-1,0-->tw：wr不减少半个hsb时钟周期，1-->tw：wr减少半个hsb时钟周期
> */
> ```
>
> （2）测试例子详见 [emi_test.c](emi_test.c)。 
>
> （3）**EMI硬件使用IO说明：**
> EMI只有固定接口，只有一个WR信号，主要分布在PC PG PH PE，详情数据手册。
>
> 
>
> **2、PAP接口**
>
> PAP口是专为8bit、16bit并行数据通信接口。PAP的结构体详情对应CPU的pap.h。
>
> （1）board.c添加pap设备应用层结构体、添加到设备列表、头文件#include "asm/pap.h"。
>
> ```c
> //pap设备应用层的硬件相关配置
> static const struct pap_info pap_data = {
> .datawidth 		= PAP_PORT_8BITS,       //并行接口数据位，固定IO，详情查看硬件数据手册
> .endian    		= PAP_LE,				//数据输出大小端
> .cycle     		= PAP_CYCLE_ONE,		//1/2字节发送次数
> .pre			= PAP_READ_LOW,			//读取rd有效电平
> .pwe			= PAP_WRITE_LOW,		//写wr有效电平
> .use_sem		= TRUE,					//使用信号等待
> .rd_en          = FALSE,                //不使用rd读信号
> .port_sel		= PAP_PORT_A,			//PAP_PORT_A PAP_PORT_B
> .timing_setup 	= 1,					//具体看pap.h
> .timing_hold  	= 1,					//具体看pap.h
> .timing_width 	= 2,					//具体看pap.h
> };
> 
> //设备列表添加pap设备
> { "pap",   &pap_dev_ops, (void *)&pap_data},
> 
> 
> /***************************************时序图*************************************************/
> /*注意：
> AC790N（WL80）:8bit模式使用硬件高位IO时，低8位不可用作其他功能（使用低8位IO则先关闭PAP IO占据。即PAP_PORT_DIS()）。
> AC791N（WL82）:8bit模式使用硬件高位IO时，低8位可以使用做其他功能。
> 
> timing_setup(tw) timing_half(th) timing_width(tw):数据传输配置
> 
> 写时钟信号（极性）:
>                        __________               _________
> pwe=H: ____________|          |_____________|         |__________
> pwe=L: ____________            _____________           __________
>                       |__________|             |_________|
> 数据信号:
>                 ________________________                       ______
> data:  _____|                        |_____________________|
> 		           |          |
> ts,tw,th:   |      |          |      |
> 		    |<-ts->|<---tw--->|<-th->|
> 		    |      |          |      |
> ts:0-3,ts个lsb系统时钟宽度
> tw:0-15,0-->16个lsb系统时钟宽度，1-15：tw个lsb系统时钟宽度
> th:0-3,th个lsb系统时钟宽度
> */
> ```
>
> （2）测试例子详见 [pap_test.c](pap_test.c)。 
>
> （3）**PAP硬件使用IO说明：**
> **A：当.datawidth = PAP_PORT_8BITS，则硬件可以选择数据IO：D0-D7或者D8-D15**
> **①.endian == PAP_LE，则使用PH0 - PH7，**
>
> **②.endian == PAP_BE，.port_sel=PAP_PORT_A，则使用PC1 - PC8**
>
> **②.endian == PAP_BE，.port_sel=PAP_PORT_B，则使用PE2 - PE9**
>
> 
>
> **B：当.datawidth = PAP_PORT_16BITS，endian无效，则硬件可以选择数据IO：D0-D15**
> **①.port_sel=PAP_PORT_A，则使用PH0 - PH7，PC1-PC8**
>
> **②.port_sel=PAP_PORT_B，则使用PPH0 - PH7，PE2-PE7**
>
> 
>
> **C：当..port_sel =PAP_PORT_A，则WR硬件使用PC9，RD使用PC10，当..port_sel =PAP_PORT_B，则WR硬件使用PE0，RD使用PC0**
>
> **注意：.rd_en  = FALSE,则可以不使用RD硬件IO。**
>
> 
>
> **3、IMD接口**
>
> IMD口是专为8bit-DVP并行数据通信接口。IMD的结构体详情对应CPU的WL82_imd.h。
>
> **注意：只有AC791N（WL82）芯片才具有IMD接口，硬件接口为固定的PA和PC和PH3，同时sync0、sync1、sync2都可选择为VSYN或HSYN或DE信号输入，详见芯片的数据手册**。
>
> （1）board.c添加imd设备应用层结构体、添加到设备列表、头文件#include "asm/WL82_imd.h"。应用层结构体配置详见 [lcd_480x272_8bits.c](..\..\ui\lcd_driver\lcd_480x272_8bits.c) 。
>
> ```c
> //IMD应用层结构体信息硬件相关配置
> IMD_RGB_PLATFORM_DATA_BEGIN(lcd_rgb_data)
> 	.test_mode		= TRUE,					//初始化颜色显示
> 	.double_buffer  = TRUE,					//底层使用双buffer切换
> 	.test_color		= 0xFFFF00,				//初始化颜色(r<<16|g<<8|b)
> 	.group_port 	= IMD_IO_GROUPA,		//IO口选择:IMD_IO_GROUPA/IMD_IO_GROUPC
> 	.irq_en			= TRUE,//FALSE,				//使能GB传输的帧中断,频率较高建议关闭:TRUE/FALSE
> 	.clk_edge		= IMD_CLK_UPDATE_H,		//更新数据边缘选择:IMD_CLK_UPDATE_H/IMD_CLK_UPDATE_L
> 	.sync0			= IMD_VSYN_EN | IMD_SYN_EDGE_H,		//行帧同步信号选择:
> 	.sync1			= IMD_DE_EN | IMD_SYN_EDGE_H,		//行帧步信号选择:
> 	.sync2			= IMD_HSYN_EN | IMD_SYN_EDGE_H,		//行帧同步信号选择:
> 	//.sync2			= IMD_SYNC_DIS,		//行帧同步信号选择:IMD_SYNC_DIS不使用该sync信号
> 	.data_out_mode 	= IMD_DATA_LSB,			//输出输出大小端:IMD_DATA_MSB/IMD_DATA_LSB
> 	.data_format	= IMD_RGB_888,			//RGB输出格式:IMD_RGB_888/IMD_RGB_565
> 	.data_shift_en	= FALSE,					//RGB666数据右移位2位(先右移在按照MSB/LSB输出)
> 	.pll_clk_div	= IMD_PLL_DIV14,//IMD_PLL_DIV40,  //clk时钟:IMD_PLL_DIV1 --> IMD_PLL_DIV56（480M的分频系数）
> 	.hsync_forward	= 10,					//行前沿宽度周期个数
> 	.hsync_behind	= 10,					//行后沿宽度周期个数
> 	.hsync_time		= 268,	                //行同步脉冲宽度周期个数
> 	.vsync_forward 	= 8,					//帧前沿宽度的行时间个数
> 	.vsync_behind	= 3,					//帧后沿宽度的行时间个数
> 	.vsync_time		= 5,					//帧同步脉冲宽的行时间个数
> 	.width			= LCD_DEV_WIDTH,					//RGB屏宽
> 	.height			= LCD_DEV_HIGHT,					//RGB屏高
> IMD_RGB_PLATFORM_DATA_END()
> 
> //设备列表添加emi设备
> { "imd",   &imd_dev_ops, NULL},
> 
> /***************************************时序图*************************************************/
> /*注意：只有AC791N（WL82）芯片才具有IMD接口
> 
> 1、vsync_forward(vf),vsync_behind(vb),vsync_time(vt)帧同步场时间配置(单位:行时间个数):
> 例如下面时序:vsync_forward = 2,vsync_behind = 3,vsync_time = 2;
> 帧VSYNC:
>          ________________________________________           ____________________________________
>    _____|                                        |_________|                                    |_____
> 
> 行HSYNC:
>      _   _   _   _   _   _          _   _   _   _   _   _   _   _   _   _          _   _   _   _   _
>    _| |_| |_| |_| |_| |_| |_......_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_......_| |_| |_| |_| |_| |_
> 
> 行有效DE:
>                      _   _          _   _                               _          _   _
>    _________________| |_| |_......_| |_| |_____________________________| |_......_| |_| |______________
> 
>                     |                    |       |         |           |
>                     |-->    height    <--|-> vf<-|-> vt  <-|->  vb   <-|
>                     |                    |       |         |           |
> 
> 
> 2、hsync_forward(hf),hsync_behind(hb),hsync_time(ht)行同步场时间配置(单位:时钟个数):
> 例如下面时序:hsync_forward = 2,hsync_behind = 3,hsync_time = 2;
> 像素点时钟PCLK:
>      _   _   _   _   _   _          _   _   _   _   _   _   _   _   _   _          _   _   _   _   _
>    _| |_| |_| |_| |_| |_| |_......_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_......_| |_| |_| |_| |_| |_
> 
> 行HSYNC:
>          ________________________________________           ____________________________________
>    _____|                                        |_________|                                    |_____
> 
> 行有效DE:
>                      ____________________                               _________________
>    _________________|                    |_____________________________|                 |____________
> 
>                     |                    |       |         |           |
>                     |--> width*3byte  <--|-> hf<-|-> ht  <-|->  hb   <-|
>                     |                    |       |         |           |
> 
> 
> */
> ```
>
> （2）测试例子详见 [imd_test.c](imd_test.c)。 
>
> （3）**IMD硬件使用IO说明：**
>
> 当：.syncx			= IMD_SYNC_DIS,则不占用IO，有些屏幕可以少使用1个IO，比如HSYNC信号等。
>
> 当：.data_format	= IMD_RGB_888,	数据接口占用全部IO（PA0-PA8或者PC0-PC8）。
>
> **当：.data_format	= IMD_RGB_666, .data_shift_en	= FALSE,则屏幕为RGB18，数据IO使用硬件高6位IO输出（PC2-PC7或者PA2-PA7）。**
>
> **当：.data_format	= IMD_RGB_666, .data_shift_en	= TRUE,则屏幕为RGB18，数据IO使用硬件低6位IO输出（PC0-PC5或者PA0-PA5）。**
## 操作说明

> 1. 使用串口线连接打印
> 2. 编译工程，烧录镜像，复位启动
> 3. 系统启动后，查看系统打印.
>
> JIELI SDK的编译、烧写等操作方式的说明可在以下文档获取：[文档](../../../../doc/stuff/usb updater.pdf)
---

## 常见问题

> * N/A

## 参考文档

> * N/A