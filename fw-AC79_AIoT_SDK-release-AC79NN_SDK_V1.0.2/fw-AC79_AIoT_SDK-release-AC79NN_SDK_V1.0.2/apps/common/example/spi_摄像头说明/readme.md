# SPI摄像头使用示例工程说明

> 本工程展示了SPI摄像头使用方式已经代码
>
> 1、SPI镜头板级配
```c
#ifdef CONFIG_VIDEO1_ENABLE
static const struct camera_platform_data camera1_data = {
	    .xclk_gpio      = -1,//IO_PORTC_08,//IO_PORTC_06,//注意： 如果硬件xclk接到芯片IO，则会占用OUTPUT_CHANNEL1
	    .reset_gpio     = -1,
	    .online_detect  = NULL,
	    .pwdn_gpio      = -1,
	    .power_value    = 0,
	    .interface      = -1,
	    .dvp={	
			.group_port  = -1,
			.pclk_gpio   = -1,
			.hsync_gpio  = -1,
			.vsync_gpio  = -1,
			.data_gpio={-1},
	    }
};
static const struct video_subdevice_data video1_subdev_data[] = {
	    { VIDEO_TAG_CAMERA, (void *)&camera1_data },
};
static const struct video_platform_data video1_data = {
	    .data = video1_subdev_data,
	    .num = ARRAY_SIZE(video1_subdev_data),
};
#endif

#ifdef CONFIG_VIDEO1_ENABLE
    { "video1.*",  &video_dev_ops, (void *)&video1_data },
#endif
```
> 2、使用SPI时需要开启CONFIG_VIDEO1_ENABLE宏定义、
>
> 3、board.c 添加SPI配置参数
>
> 4、#define CONFIG_VIDEO1_ENABLE
> 	#define CONFIG_SPI_VIDEO_ENABLE
> 	#define CONFIG_VIDEO_720P
>
> 需要包含这三个宏才能正常输出数据

​	



## 适用平台

> 本工程适用以下芯片类型：
> 1. AC79系列芯片：AC790N、AC791N
>
> 杰理芯片和评估板的更多信息可在以下地址获取：[链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)

## 代码说明

1. SPI已经完善好了接收的流程需要包含spi_recv.c。

```c
static void spi_recv_task(void *p)
{
    void *spi_hdl = NULL;
    int ret;
    int wlen, offset, online_data_size, sline_cnt;
    int *head, *end;
    char *read_buf;
    int now = 0, last = 0, last_fp = 0;
    int msg[4];
    u32 timer_get_ms(void);

    //1.打开spi设备
    spi_hdl = dev_open(JL_SPI_NAME, NULL); //这里会配置SPI的组号 列如"spi1"
    if (!spi_hdl) {
        printf("spi open err \n");
        goto exit;
    }
    //2.注册中断函数
    dev_ioctl(spi_hdl, IOCTL_SPI_SET_IRQ_FUNC, (u32)spi_irq_cb); //这里注册中断函数用于接收SPI数据 
    //790N系列spi接收不支持超64K 由于DMA限制  
    //791N系列修复了MDA的限制条件 
/////////////////////////////////////////////////////////////////////////////////////////////

    static int spi_camera_open(void)
{
    int err = 0;
    if (spi_head_info.state == SPI_VIDEO_OPEN || spi_head_info.state == SPI_VIDEO_OPENING) {
        puts("spi video is open \n");
        return 0;
    }
    spi_head_info.state = SPI_VIDEO_OPENING;
    /*------SPI摄像头的CLK，OUP_CH2 24M-----*/
    gpio_output_channle(SPI_XCLK_PORT, CH1_PLL_24M); //这里会配置摄像头 需要的时钟信号

    if (spi_head_info.state == SPI_VIDEO_RESEST) {
        printf("--->SPI_VIDEO_RESEST\n");
        spi_head_info.state = SPI_VIDEO_OPENING;
        if (spi_head_info.camera_hdl) {
            dev_ioctl(spi_head_info.camera_hdl, CAMERA_SET_SENSOR_RESET, 0);
            spi_head_info.state = SPI_VIDEO_OPEN;
        }
    } else {
        spi_head_info.camera_hdl = dev_open("video1.0", NULL);  //打开SPI摄像头的VIDEO功能 
        if (!spi_head_info.camera_hdl) {
            printf("camera open err\n");
            err = -EINVAL;
            goto error;
        }
    }
    spi_head_info.state = SPI_VIDEO_OPEN;
    spi_head_info.init = 1;
    printf("camera open ok\n");
    return 0;

error:
    spi_head_info.state = SPI_VIDEO_CLOSE;
    return err;
}

 ///////////////////////////////////////////////////
    
    
    
                    online_data_size = SPI_CAMERA_ONLINE_SIZE - SPI_LHEAD_SIZE - SPI_LEND_SIZE;
                while (wlen < SPI_CAMERA_ONFRAM_ONLY_Y_SIZE && read_buf < (read_buf + app_recv_len)) {
                    head = (int *)read_buf;
                    if (*head == SPI_LSTART) {
                        sline_cnt++;
                        memcpy((char *)app_recv_buf + wlen, (char *)((int)head + SPI_LHEAD_SIZE), online_data_size);
                        wlen += online_data_size;
                        read_buf += SPI_CAMERA_ONLINE_SIZE;
                        continue;
                    } else if (*head == SPI_FEND) {
                        break;
                    }
                    read_buf += SPI_LHEAD_SIZE;
                }
                if (sline_cnt == SPI_CAMERA_H) {
                    //void send_yuv_data(char *buf, u16 len);   在这个位置处理摄像头输出的YUV565数据
                    //send_yuv_data(app_recv_buf, wlen);     
                    //todo，数据处理
                }
                spi_head_info.err = 0;

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