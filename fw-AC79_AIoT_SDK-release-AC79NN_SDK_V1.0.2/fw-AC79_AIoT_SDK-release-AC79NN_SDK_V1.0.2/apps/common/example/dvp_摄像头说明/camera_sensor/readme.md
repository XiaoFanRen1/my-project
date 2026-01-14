# 摄像头驱动添加和调试示例工程说明

> 本工程展示了摄像头驱动添加和调试示例:
>
> 1. 工程添加摄像头驱动;
> 2. 打开摄像头读取ID;

---

## 适用平台

> 本工程适用以下芯片类型：
> 1. AC79系列芯片：AC790N、AC791N
>2. AC790N支持DVP格式镜头，AC791N支持DVP格式和BT656-1/2/4/8/bit格式镜头
> 
>杰理芯片和评估板的更多信息可在以下地址获取：[链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)

## 依赖性
>1、获取摄像头YUV功能依赖video0和vdieo相关库，因此需要在demo_video工程基础上添加。
>
>2、AC790N需要DVP格式镜头，AC791N需要DVP和BT656-1/2/4/8/bit格式镜头。

## 工程配置说明

> 在SDK选择[demo_video](../../../../apps/demo_video/board)主工程就可以直接参考程序代码。
>
> 本例子说明如何使用gc0308摄像头驱动移植：
>
> 1、board.c版籍配置，摄像头硬件需要iic、power_down、xclk、hsync、vsync、pclk、data0-data7(或data2-data9)的8位数据IO。参考原理图 。 [AC7901BA TEST V1.0.pdf](..\..\..\..\doc\datasheet\AC790N规格书\schematic\AC7901BA TEST V1.0.pdf) 
>
> board.c代码说明，**注意：hsync、vsync、pclk、data0-data7(或data2-data9)这些IO是固定的，不能随意更改，接摄像头接口的只有PA0-PA10和PC0-PC10，具体需要看封装**。
>
> ```c
> //---------------------------摄像头相关硬件配置------------------------//
> /*
> *摄像头软件IIC
> */
> SW_IIC_PLATFORM_DATA_BEGIN(sw_iic0_data)
> 	.clk_pin = IO_PORTC_01,//摄像头的IIC CLK IO
> 	.dat_pin = IO_PORTC_02,//摄像头的IIC SDA IO
> 	.sw_iic_delay = 50,//软件iic延时nop
> SW_IIC_PLATFORM_DATA_END()
> 
> /*
> *摄像头IO-PORT
> */
> #ifdef CONFIG_VIDEO_ENABLE
> #define CAMERA_GROUP_PORT	ISC_GROUPA //摄像头数据接口接芯片的PA口
> /* #define CAMERA_GROUP_PORT	ISC_GROUPC //摄像头数据接口接芯片的PC口 */
> static const struct camera_platform_data camera0_data = {
>     .xclk_gpio      = IO_PORTC_00,//供时钟注意： 如果硬件xclk(mclk)接到芯片IO，24MHz会占用OUTPUT_CHANNEL1，12MHz会占用OUTPUT_CHANNEL0
>     .xclk_hz        = 0, // 0或24-->xclk out:24MHz, 12-->xclk out:12MHz ，默认24M即可
>     .reset_gpio     = IO_PORTC_03,//复位IO，一般该引脚可以直接接3.3V的高电平，不需要特定IO来控制
>     .online_detect  = NULL,//默认NULL即可
>     .pwdn_gpio      = -1,//pwdn引脚可以配合引脚写具体IO如IO_PORTC_04，一般摄像头pwdn引脚直接接地即可，具体要看摄像头数据手册
>     .power_value    = 0,//pwdn引脚使用IO控制时的正常工作pwdn引脚电平
>     .interface      = SEN_INTERFACE0,//SEN_INTERFACE_CSI2,//默认即可
>     .dvp={
> #if (CAMERA_GROUP_PORT == ISC_GROUPA)//不同数据接口配置的摄像头pclk和hsync和vsync
>     .pclk_gpio   = IO_PORTA_08,
>     .hsync_gpio  = IO_PORTA_09,
>     .vsync_gpio  = IO_PORTA_10,
> #else
>     .pclk_gpio   = IO_PORTC_08,
>     .hsync_gpio  = IO_PORTC_09,
>     .vsync_gpio  = IO_PORTC_10,
> #endif
>     .group_port  = CAMERA_GROUP_PORT,
>     .data_gpio={
> #if (CAMERA_GROUP_PORT == ISC_GROUPA)//不同数据接口配置的摄像头数据接口IO
>                IO_PORTA_07,//从7到0
>                IO_PORTA_06,
>                IO_PORTA_05,
>                IO_PORTA_04,
>                IO_PORTA_03,
>                IO_PORTA_02,
>                IO_PORTA_01,
>                IO_PORTA_00,
>                         -1,
>                         -1,
> #else
>                IO_PORTC_07,//从7到0
>                IO_PORTC_06,
>                IO_PORTC_05,
>                IO_PORTC_04,
>                IO_PORTC_03,
>                IO_PORTC_02,
>                IO_PORTC_01,
>                IO_PORTC_00,
>                         -1,
>                         -1,
> #endif
>         },
>     }
> };
> static const struct video_subdevice_data video0_subdev_data[] = {//镜头设备参数
> { 
>     VIDEO_TAG_CAMERA, (void *)&camera0_data },
> };
> static const struct video_platform_data video0_data = {//video参数,设备列表参数需要该参数
>     .data = video0_subdev_data,
>     .num = ARRAY_SIZE(video0_subdev_data),
> };
> #endif
> 
> 
> //---------------------------设备列表------------------------//
> REGISTER_DEVICES(device_table) = {
> 	{ "iic0",  &iic_dev_ops, (void *)&sw_iic0_data },
> 	{ "audio", &audio_dev_ops, (void *)&audio_data },
> #ifdef CONFIG_VIDEO_ENABLE
> 	{ "video0.*",  &video_dev_ops, (void *)&video0_data },//添加vdeo设备
> #endif
> 	{"uart2", &uart_dev_ops, (void *)&uart2_data },
> };
> ```
>
> 2、app_config.h的摄像头分辨率配置说明，相关CONFIG_VIDEO_IMAGE_W和CONFIG_VIDEO_IMAGE_H:
>
> ```c
> //*********************************************************************************//
> //                             编码图片分辨率                                      //
> //*********************************************************************************//
> //摄像头尺寸，此处需要和摄像头驱动可匹配，默认摄像头为640*480
> //#define CONFIG_VIDEO_720P
> #ifdef CONFIG_VIDEO_720P
> #define CONFIG_VIDEO_IMAGE_W    1280
> #define CONFIG_VIDEO_IMAGE_H    720
> #else
> #define CONFIG_VIDEO_IMAGE_W    640
> #define CONFIG_VIDEO_IMAGE_H    480
> #endif
> 
> ```
>
> 3、添加gc0308驱动到工程（摄像头驱动目录统一放在 [camera](..\..\camera) ）[gc0308.c](..\..\camera\gc0308_dvp\gc0308.c) 驱动说明，**摄像头驱动的寄存器需要原厂提供驱动寄存器源码**，否则很难对一款摄像头进行调试，得到原厂驱动后在结合摄像头数据手册查看和修改：hsync极性、vsync极性、plck极性、输出分辨率、输出帧率等。
>
> ```c
> //主要函数1，移植驱动关键的寄存器写函数
> static unsigned char wrGC0308Reg(unsigned char regID, unsigned char regDat)
> {
>  u8 ret = 1;
>  dev_ioctl(iic, IIC_IOCTL_START, 0);//iic起始信号
>  if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, GC0308_WRCMD)) {//发送gc0308 IIC的写地址命令
>      ret = 0;
>      printf("iic write err!!! line : %d \n", __LINE__);
>        goto exit;
>  }
>  delay(1000);
>  if (dev_ioctl(iic, IIC_IOCTL_TX, regID)) {//发送需要写的寄存器ID地址
>      ret = 0;
>      printf("iic write err!!! line : %d \n", __LINE__);
>        goto exit;
>  }
>  delay(1000);
>  if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_STOP_BIT, regDat)) {//发送需要写的寄存器的值
>      ret = 0;
>      printf("iic write err!!! line : %d \n", __LINE__);
>        goto exit;
>  }
>  delay(1000);
> 
>  exit:
>  dev_ioctl(iic, IIC_IOCTL_STOP, 0);//iic结束信号
>  return ret;
> }
> //主要函数2，移植驱动关键的寄存器读函数
> static unsigned char rdGC0308Reg(unsigned char regID, unsigned char *regDat)
> {
>  u8 ret = 1;
>  dev_ioctl(iic, IIC_IOCTL_START, 0);//iic起始信号
>  if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, GC0308_WRCMD)) {//发送gc0308 IIC的写地址命令
>      ret = 0;
>      printf("iic write err!!! line : %d \n", __LINE__);
>        goto exit;
>  }
>  delay(1000);
>  if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_STOP_BIT, regID)) {//发送需要读取的寄存器ID地址
>      ret = 0;
>      printf("iic write err!!! line : %d \n", __LINE__);
>        goto exit;
>  }
>  delay(1000);
>  if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, GC0308_RDCMD)) {//发送读取命令
>      ret = 0;
>      printf("iic write err!!! line : %d \n", __LINE__);
>        goto exit;
>  }
>  delay(1000);
>  dev_ioctl(iic, IIC_IOCTL_RX_WITH_STOP_BIT, (u32)regDat);//读取寄存器值
> 
>  exit:
>  dev_ioctl(iic, IIC_IOCTL_STOP, 0);//iic结束信号
>  return ret;
> }
> //主要函数3，检测ID函数
> static s32 GC0308_ID_check(void)
> {
>  u8 pid = 0x00;
>  rdGC0308Reg(0x00, &pid);//读取id，gc0308读取id的IIC寄存器地址是0x00
>  delay(40000);
>  rdGC0308Reg(0x00, &pid);//多读取id一次，gc0308读取id的IIC寄存器地址是0x00
>  delay(40000);
>  printf("GC0308 Sensor ID : 0x%x\n", pid);
>  if (pid != 0x9b) {//判断ID是否是gc0308的id，不是则返回-1,是则返回0
>  	return -1;
>  }
> 	return 0;
> }
> //主要函数4，系统首先回调该函数进行摄像头检测，当返回值0时代表打开成功代开对应摄像头。
> static s32 GC0308_check(u8 isp_dev, u32 _reset_gpio, u32 _power_gpio)
> {
>  if (!iic) {
>      if (isp_dev == ISP_DEV_0) {
>          iic = dev_open("iic0", 0);//默认打开IIC设备0
>        } else {
>            iic = dev_open("iic1", 0);//
>        }
>        gc0308_reset_io[isp_dev] = _reset_gpio;//存储复位和电源IO配置
>        gc0308_power_io[isp_dev] = _power_gpio;//存储复位和电源IO配置
>  }
>  if (iic == NULL) {
>      printf("GC0308 iic open err!!!\n\n");
>      return -1;
>  }
>  GC0308_reset(isp_dev);//复位摄像头
>  if (0 != GC0308_ID_check()) {//id检测，id不是gc0308则返回-1通知系统
>      printf("-------not GC0308------\n\n");
>      dev_close(iic);
>    	iic = NULL;
>    	return -1;
>  }
>  printf("-------hello GC0308------\n\n");
>  return 0;//id检测，id是gc0308则返回0通知系统
> }
> //主要函数5，镜头驱动寄存器配置函数，需要改寄存器在在改函数加上即可
> static void GC0308_config_SENSOR(u16 *width, u16 *height, u8 *format, u8 *frame_freq)
> {
>  wrGC0308Reg(0xfe, 0x80);
>  wrGC0308Reg(0xfe, 0x00);        // set page0
>  wrGC0308Reg(0xd2, 0x10);    // close AEC
>  wrGC0308Reg(0x22, 0x55);    // close AWB
>  wrGC0308Reg(0x5a, 0x56);
>  wrGC0308Reg(0x5b, 0x40);
>  wrGC0308Reg(0x5c, 0x4a);
>  wrGC0308Reg(0x22, 0x57);   // Open AWB
> ......
> ......
> }
> //主要函数6，镜头设置分辨率函数
> static s32 GC0308_set_output_size(u16 *width, u16 *height, u8 *freq)
> {
>  //注意有些摄像头的分辨率需要多加8或者16或者32
>  u16 liv_width = *width + 8;//*width为：GC0308_DEVP_INPUT_W
>  u16 liv_height = *height + 8;//*height：GC0308_DEVP_INPUT_H
> 
>  wrGC0308Reg(0x09, liv_height >> 8);//配置分辨率高 高位
>  wrGC0308Reg(0x0a, liv_height & 0xff);//配置分辨率高 低位
>  wrGC0308Reg(0x0b, liv_width >> 8);//配置分辨率宽 高位
>  wrGC0308Reg(0x0c, liv_width & 0xff);//配置分辨率宽 低位
> 
>  printf("GC0308 : %d , %d \n", *width, *height);
>  return 0;
> }
> //主要函数7，系统调用，来完成镜头检测和初始化镜头
> static s32 GC0308_init(u8 isp_dev, u16 *width, u16 *height, u8 *format, u8 *frame_freq)
> {
>  puts("\n\n GC0308_init \n\n");
> 
>  if (0 != GC0308_check(isp_dev, 0, 0)) {//检测ID
>  	return -1;
>  }
>  GC0308_config_SENSOR(width, height, format, frame_freq);//配置镜头驱动寄存器值
>  return 0;
> }
> //主要函数8，镜头翻转，应用层可以在系统初始化镜头之后直接调用进行镜头翻转
> void set_rev_sensor_GC0308(u16 rev_flag)
> {
>  if (!rev_flag) {
>  	wrGC0308Reg(0x14, 0x13);//翻转寄存器和寄存器值需要根据镜头数据手册说明
>  } else {
>    wrGC0308Reg(0x14, 0x10);//翻转寄存器和寄存器值需要根据镜头数据手册说明
>  }
> }
> 
> //注册驱动
> REGISTER_CAMERA(GC0308) = {
>  .logo 				= 	"GC0308",//驱动名称
>  .isp_dev 			= 	ISP_DEV_NONE,
>  .in_format 			= 	SEN_IN_FORMAT_UYVY,//镜头输出数据格式UYVY/YUYV等，出现颜色相反可以修改该项，配置需要和镜头驱动寄存器配置一样
>  .mbus_type          =   SEN_MBUS_PARALLEL,
> 
>  //------------mbus_config配置项说明-----------------------//
>  //SEN_MBUS_DATA_WIDTH_8B 数据接口D0-D7和芯片IO正接，反接则SEN_MBUS_DATA_WIDTH_8B_REVERSE
>  //SEN_MBUS_HSYNC_ACTIVE_HIGH:HSYNC信号是高有效，低有效则SEN_MBUS_HSYNC_ACTIVE_LOW，根据摄像头驱动配置hsync极性吻合
>  //SEN_MBUS_PCLK_SAMPLE_FALLING:时钟采样边缘，出现轮廓有噪点时试试改为SEN_MBUS_PCLK_SAMPLE_RISING，同理相反，需摄像头pclk极性吻合
>  //SEN_MBUS_VSYNC_ACTIVE_HIGH:VSYNC高有效，低有效则SEN_MBUS_VSYNC_ACTIVE_LOW，需摄像头驱动配置vsync极性吻合
>  .mbus_config        =   SEN_MBUS_DATA_WIDTH_8B  | SEN_MBUS_HSYNC_ACTIVE_HIGH | SEN_MBUS_PCLK_SAMPLE_FALLING | SEN_MBUS_VSYNC_ACTIVE_HIGH,//数据接口
>  
> //------------sync_config配置项WL82/AC791才能：HSYNC-VSYNC互换-----------------------//
> .sync_config		=   SEN_MBUS_SYNC0_HSYNC_SYNC1_VSYNC,//WL82/AC791才可以H-V SYNC互换，请留意
> .fps         		= 	CONFIG_INPUT_FPS,//输入帧率
>  .out_fps			=   CONFIG_INPUT_FPS,//输出帧率设置为输入帧率即可
>  .sen_size 			= 	{GC0308_DEVP_INPUT_W, GC0308_DEVP_INPUT_H},//分辨率
>  .cap_fps         	= 	CONFIG_INPUT_FPS,
>  .sen_cap_size 		= 	{GC0308_DEVP_INPUT_W, GC0308_DEVP_INPUT_H},//设置为分辨率即可
>  .ops                =   {
>  .avin_fps           =   NULL,
>  .avin_valid_signal  =   NULL,
>    .avin_mode_det      =   NULL,
>    .sensor_check 		= 	GC0308_check,//检测镜头函数
>    .init 		        = 	GC0308_init,//初始化镜头函数
>    .set_size_fps 		=	GC0308_set_output_size,//设置分辨率函数
>    .power_ctrl         =   GC0308_power_ctl,//电源控制引脚函数
>    .sleep 		        =	NULL,
>    .wakeup 		    =	NULL,
>    .write_reg 		    =	GC0308_dvp_wr_reg,//写寄存器函数
>    .read_reg 		    =	GC0308_dvp_rd_reg,//读寄存器函数
>    .set_sensor_reverse =   set_rev_sensor_GC0308,//翻转函数
>  }
> }
> ```
>
> 
>
> 4、使用demo_video的app_main的hello_video_task函数，通过用户video打开摄像头。
>
> **本次打开摄像头有是通过user_video_rec0_open()，另外一种方法参考YUV获取 [get_yuv_data_test.c](..\camera_yuv_data\get_yuv_data_test.c)** 

---



### 操作说明

> 1.编译工程，烧录镜像，复位启动
>
> 2.系统启动后，可以通过串口软件看到示例的打印信息
>
> 3.当打印有"-------hello GC0308------"则GC0308成功打开
>
> JIELI SDK的编译、烧写等操作方式的说明可在以下文档获取：[文档](../../../../doc/stuff/usb updater.pdf)

### 代码流程

> * app_main.c
>   > 1. hello_video_task()入口：
>   >     user_video_rec0_open()--->系统服务层--->系统设备层--->设备层驱动层--->摄像头驱动。

---

## 常见问题

> * 1、摄像头IIC检测出错，id无法检测。
>
>   A）排查board.c ：IIC、xclk、pwdn_gpio、power_value有没有配置正确，根据摄像头数据手册确定摄像头pwdn正常工作电平。
>
>   B）确定xclk输出到摄像头时钟正常（可以用示波器、逻辑分仪抓取），确定xclk IO是否被占用， **当供给摄像头的时钟xclk不正常时无法读取id！**
>
>   C）确定摄像头是否已经损坏，**当接通电源发热严重则可能电源不对或者电源接反！**
>
>  * 2、摄像头id读取正常，但出现“isc line in err ”或者“isc line out err”打印，说明摄像头行数据输出到cpu的行数出错。
>
>    A）确定hsync和vsync接线是否正确。
>
>    B）确定app_config.h配置的CONFIG_VIDEO_IMAGE_W和CONFIG_VIDEO_IMAGE_H是否在镜头的输出范围内。
>
>    **C）确定驱动的.mbus_config参数极性选择正确！！！！！**
>
>    **D）确定驱动.sync_config 配置HSYNC和VSYNC对应硬件IO-SYNC0/1正确（仅WL82/AC791支持hsync和vsync互换）！！！！！！！。**
>
>    **E）确定摄像头的data0-7和芯片data0-7没有接反，接反则.mbus_config参数或上SEN_MBUS_DATA_WIDTH_8B_REVERSE。**
>
>  * 3、出现“isc bwerr !!!”和“osd bandwith err”，说明cpu带宽不足。
>
>    A）确定摄像头输出帧率和分辨率是否过高导致出现带宽不足。
>
>    **B）确定摄像头的data0-7和芯片data0-7没有接反，接反则.mbus_config参数或上SEN_MBUS_DATA_WIDTH_8B_REVERSE。**
>    
>    C）把相应的video的buffer指定到内部sram，或者提高系统的频率（详见：[系统频率说明](..\系统频率\readme.md) ）。
>
> 

## 参考文档

> * N/A