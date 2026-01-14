# SPI单线摄像头说明

> SDK提供了两个C文件进行SPI的使用 spi_video.c DVP单线接口和 spi_recv.c spi接口请参考屏接口中SPI接口。
```c
 #define SPI_VIDEO_SPIDEV_NAME		"spi1"
 #define SPI_VIDEO_USE_SPI_RECV		0   //1使用SPI接收(需要软件分离YUV),0使用硬件接>收YUV
 #define SPI_VIDEO_YUV_USE_DMA   	1   //使用DMA拷贝
 #define SPI_VIDEO_REVERSAL   		0  //spi镜头翻转 0 90 180 270
 #define SPI_VIDEO_XCLK_12M_EN		0   //spi镜头的xclk时钟12M/24M, 1:12M, 0:24M
 #define SPI_VIDEO_ONLY_Y	0   //1:只有Y数据
 #define SPI_APP_USE_ORIGINL_SIZE	0   //1:手机APP使用摄像头尺寸不缩放

1、 SPI_VIDEO_USE_SPI_RECV该宏区分使用硬件DVP单线还是SPI中断进行收数据
    其中当配置好了 使用硬件BT565收数据时需要配置 硬件接收数据的IO 
    例如使用代码默认配置 
    #ifdef SPI_VIDEO_XCLK_12M_EN
    #ifdef CONFIG_CPU_WL80  //AC79N
    #define SPI_VIDEO_XCLK_PORT		IO_PORTC_00
    #else
    #define SPI_VIDEO_XCLK_PORT		IO_PORTC_00
    #endif
    #endif
    /////////////////BT656硬件接收/////////////////////////
    #define SPI_VIDEO_PCLK_PORT		IO_PORTC_08
    #define SPI_VIDEO_MISO_PORT		IO_PORTC_07

    DVP单线只能接收YUV422数据 使用硬件接收YUV422数据输出YUV420数据供应用层使用
    如果用户想之定义数据 可以使用 int yuv_frame_done_callback_set(void *func, void *priv);进行回调取数
    
    当用户配置成 SPI_VIDEO_USE_SPI_RECV		1
    使用SPI进行接收数据时 需要开不可屏蔽中断防止接收数据被打断 
    默认代码超15帧会纯在问题 丢数据导致摄像头复位 目前没有去解决这个问题 是接收协议导致的问题 
    后面因为这个问题才有了spi_recv.c专门用于接收SPI摄像头数据  还有一个就是代码里面的SPI摄像头数据格式为展讯通信协议 
    
    使用spi_video.c需要在app_config.h中配置 CONFIG_SPI_VIDEO_ENABLE  CONFIG_VIDEO1_ENABLE 
    使用wifi_camera工程可以进行SPI图层测试  通过手机查看spi出图图像
```



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