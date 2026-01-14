# SPI从机使用示例工程说明

> 本工程展示了spi从机接口使用示例:
>
> 1. 设置spi从机参数。
> 2. 从spi设备读取接收到的数据。

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
> 	.clk    = 80000000, //时钟，可设置，无影响，时钟从机模式跟随主机模式
> 	.mode   = SPI_STD_MODE,//spi数据线传输:mosi miso clk 
> 	.port   = 'B',//SPI接口
> 	.attr	= SPI_MODE_SLAVE | SPI_SCLK_H_UPL_SMPH | SPI_UNIDIR_MODE,//参数：从机，CLK高 更新数据高，单向模式
> SPI1_PLATFORM_DATA_END()//当使用SPI2则：SPI2_PLATFORM_DATA_END()
> 
> //2 设备列表添加SPI设备
> {"spi1", &spi_dev_ops, (void *)&spi1_data },
> ```
>
> 说明：
>
> A）参数.clk：spi时钟，从机模式则跟随主机。
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
> 参数3单双向模式：SPI_UNIDIR_MODE为单向：只用SPI的DO一根线数据传输（一般只用于作为从机只接收数据，或者只发送数据），SPI_BIDIR_MODE为双向：用SPI的DO/DI两根线数据传输，从机单双向都可用（具体看实际应用）。
>
> 2. 测试代码
>
> ```c
> #include "app_config.h"
> #include "system/includes.h"
> #include "device/device.h"
> #include "asm/spi.h"
> 
> //定义中断回调函数
> void spi_irq_cb(void *priv, u8 **data, u32 *len)
> {
>     putchar('@');
> }
> static void spi_test_task(void *p)
> {
>     char *buf = NULL;
>     struct spi_user su = {0};
> 	void *spi_hdl = NULL;
> 	int ret;
> 	int read_addr;
> 
> #if 1
>     /* 使用方法1：接收固定数据量，比如接收固定的SPI摄像头数据。
>     如：SPI摄像宽为128，高为224，仅传输Y灰度数据，一帧数据头为4字节，
>     一帧数据尾为4字节，摄像头进行一行一行数据往SPI发送数据，则当
>     一帧数据量小于65535时，SPI从机配置如下：
>     */
>     int w = 128;
>     int h = 224;
>     int fhead  = 4;
>     int fend = 4;
>     int line_size = w;//一行的数据（此处为Y，当数据为Y+U+V的YUV422时，line_size = w * 2）
>     int frame_size = line_size * h + fhead + fend;//一帧的数据（此处为Y，当数据为Y+U+V的YUV422时，frame_size = line_size * h * 2 + fhead + fend）
> 
>     //----------------用户指定接收块大小配置--------------------------//
>     /*1.当一帧数据量<=SPI_MAX_SIZE，则可以接收完整的一帧
>         当一帧数据量>SPI_MAX_SIZE，则先接收一部分，再偏移地址接收一部分（内部系统完成，应用层不用关注）
>         当芯片为AC790N则SPI_MAX_SIZE = 65535，当芯片为AC791N则SPI_MAX_SIZE = 2^31
>     */
>     //first_dma_size第一次接收数据量：一帧数据量<=SPI_MAX_SIZE，可设置第一次DMA数据量为一帧或者0，当一帧数据量>SPI_MAX_SIZE，第一次接收为：帧头 + 行的整数倍
>     su.first_dma_size 	= (frame_size > SPI_MAX_SIZE) ? (SPI_MAX_SIZE / line_size * line_size + fhead) : frame_size;
>     //dma最大个数为spi的最大dma字节
>     su.dma_max_cnt 		= SPI_MAX_SIZE;
>     //一个块的大小（分块在此处即为一帧大小）
>     su.block_size 		= frame_size;
>     //指定接收的缓冲区为一个块大小（此处为一帧）的2倍以上，此处为2倍
>     su.buf_size 		= frame_size * 2;
>     //申请缓冲区
>     su.buf = malloc(su.buf_size);
>     if(!su.buf){
>         printf("err spi no buf \n");
>         goto exit;
>     }
> 
>     //2.打开spi设备
>     spi_hdl = dev_open("spi1", NULL);
>     if (!spi_hdl) {
>         printf("spi open err \n");
>         goto exit;
>     }
> 
>     //3.设置：用户指定接收块大小配置信息，使用IOCTL_SPI_SET_USER_INFO时不能使用IOCTL_SPI_SET_CB注册回调函数
>     dev_ioctl(spi_hdl, IOCTL_SPI_SET_USER_INFO, (u32)&su);
>     dev_ioctl(spi_hdl, IOCTL_SPI_SET_IRQ_CPU_ID, (u32)1);//可以指定中断到核1执行
> 
>     while (1) {
>         //4.读取数据，注意：IOCTL_SPI_READ_DATA启用读取数据之前需保证此时对方SPI还没有发送数据，否则中途接收数据会出错
>         ret = dev_ioctl(spi_hdl, IOCTL_SPI_READ_DATA, (u32)&read_addr);
>         if (ret > 0) {//返回值大于0则已经读取到数据
>             //5. read_addr的值就是硬件存放数据在su.buf的可用地址，ret则为可用地址read_addr的数据量大小
>             //todo
>             put_buf((u8*)read_addr, ret);//打印数据
> 
> 
> 
> 
>             //6.用完释放数据
>             dev_ioctl(spi_hdl, IOCTL_SPI_FREE_DATA, 0);
>         }
>     }
> #else
> 
>     /* 使用方法1：单次指定接收固定数据量。
>     注意：此方法spi接收数据比较慢，因此对方主机的spi连续发送字节不能过快，否则丢数据（当数对方主机spi数据过快，请使用上述：用户指定接收块方法）
>     */
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
>     dev_ioctl(spi_hdl, IOCTL_SPI_SET_IRQ_CPU_ID, (u32)1);//可以指定中断到核1执行
>     dev_ioctl(spi_hdl, IOCTL_SPI_SET_USE_SEM, 0);//等待数据时，用信号量等待，不用则为抢占查询硬件中断标记，建议应信号量等待
> 
>     //3.发送一个字节，有发送字节的board.c需配置为使用DO和DI数据线
>     u8 data = 0x12;
>     dev_ioctl(spi_hdl, IOCTL_SPI_SEND_BYTE, data);//发送一个字节
> 
>     //4.读取一个字节
>     data = dev_ioctl(spi_hdl, IOCTL_SPI_READ_BYTE, 0);//读取一个字节
> 
>     buf = malloc(1024);
>     if(!buf){
>         printf("err spi no buf \n");
>         goto exit;
>     }
> 
>     while (1) {
>         //5.固定读取1024字节，注意当读取满1024字节时，需要任务调度此处才能返回去执行收数应用层操作，因此对方主机SPI发送完1024字节时，一般需要等待几个ms以上才能再次发固定数据
>         ret = dev_read(spi_hdl, buf, 1024);
>         if (ret == 0) {//返回值=0则已经读取到数据
>             //6. 数据存在buf，长度为1024
>             //todo
>             put_buf(buf, 1024);//打印数据
> 
>         }
>     }
> 
> #endif
> 
> exit:
>     if(su.buf){//释放内存
>         free(su.buf);
>     }
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
> B）当使用SPI作为从机接收不固定数据时，需要知道通信协议，详情参考程序 [spi_nomask_interrupt.c](spi_nomask_interrupt.c) 。
>
> C）使用不可屏蔽中断时，中断函数不能调用系统API（调试期间可加打印printf函数），调用函数需要用户自写，调用其他API也需要指定到指定在内部sram，即sec(.volatile_ram_code)指定函数，详情参考程序 [spi_nomask_interrupt.c](spi_nomask_interrupt.c) 。
>
> D）从机模式下不支持CS拉低或者拉高触发接收（主机下用户自行处理CS引脚时序，一般在读写前改变即可）

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