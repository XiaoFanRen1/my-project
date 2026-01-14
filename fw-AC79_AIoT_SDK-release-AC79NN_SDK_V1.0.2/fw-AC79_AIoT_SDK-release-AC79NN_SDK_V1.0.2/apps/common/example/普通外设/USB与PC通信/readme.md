# USB与PC通信

> 本文列举了两种USB与PC端通信协议的使用方式，SCSI协议与CDC协议，可参考应用于PC端的上位机开发。

---

## 适用平台

> 本工程适用以下芯片类型：
>
> 1. AC79系列芯片：AC790N、AC791N

> 杰理芯片和评估板的更多信息可在以下地址获取：[链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)

## 工程配置说明

> 在SDK选择[demo_ble](../../../demo/demo_ble/board/)主工程文件或者主工程Makefile, 然后添加本事例工程代码

---



### 操作说明：

> 1.USB SCSI协议的使用
>
> ​	（1）USB配置
>
> ​		SCSI协议一般用于U盘、读卡器等大容量存储设备。在SDK中，将USB配置成从机模式，并配置设备类型为存储设备；在app_config.h文件中进行配置，如下：
>
> ```c
> //3.从机连接PC模式下：电脑盘符功能、UVC功能
> #define TCFG_PC_ENABLE		1    //打开usb从机功能(打开连接PC电脑模式)
> #if TCFG_PC_ENABLE
> ...
> #define USB_DEVICE_CLASS_CONFIG (MASSSTORAGE_CLASS) //从机电脑盘符功能选择
> #endif
> 
> ```
>
> ​	（2）SCSI协议数据处理
>
> ​		在SDK/include_lib/driver/device/usb/scsi.h文件中，添加自定义的SCSI协议的操作码，注意不能和现有的操作码重复，如下：
>
> ```c
> #define MY_TEST_OPCODE        0x7d
> ```
>
> ​		在SDK/apps/common/usb/device/msd.c文件中，添加SCSI协议数据的处理函数，如下:
>
> ```c
> enum {
>     TEST_CMD_0,
>     TEST_CMD_1,
> };
> void test_cmd_deal(const struct usb_device_t *usb_device)
> {
>     u8 len;
>     u8 buf[32]; 
>     
>     switch (msd_handle->cbw.lun) {
>         case TEST_CMD_0: 
>             printf("^^^TEST_CMD_0\n");
>             //返回数据至USB主机
>             memset(buf, 0xaa, sizeof(buf));
>             msd_mcu2usb(usb_device, buf, sizeof(buf));
>             break;
>         case TEST_CMD_1:
>             printf("^^^TEST_CMD_1\n");
>             //接收USB主机发送的数据
>             len = msd_handle->cbw.LengthL;
>             msd_usb2mcu(usb_device, buf, len);
>             put_buf(buf, sizeof(buf));
>             break;
>         default:
>             printf("^^^no support cmd\n");
>             break; 
>     }
> }
> 
> void USB_MassStorage(const struct usb_device_t *usb_device)
> {
> 	//......
>     switch (msd_handle->cbw.operationCode) {
>         //......
>         case MY_TEST_OPCODE:	//自定义的操作码处理
>             test_cmd_deal(usb_device);
>             break;
>         //......
>     }
> }
> ```
>
> ​		（3）数据收发测试
>
> ​			可使用USB调试助手，向USB从机发送SCSI命令进行测试，如下：
>
> | ![1](.\1.png) |
> | ------------- |
> | ![2](.\2.png) |
>
> 
>
> 2.USB CDC协议的使用
>
> ​	（1）USB配置
>
> ​		CDC协议将USB设备枚举为虚拟串口，PC端通过串口协议与USB从机进行通信。在SDK中，将USB配置成从机模式，并配置设备类型为CDC类设备；在app_config.h文件中进行配置，如下：
>
> ```c
> //3.从机连接PC模式下：电脑盘符功能、UVC功能
> #define TCFG_PC_ENABLE		1    //打开usb从机功能(打开连接PC电脑模式)
> #if TCFG_PC_ENABLE
> ...
> #define USB_DEVICE_CLASS_CONFIG (CDC_CLASS) //从机电脑盘符功能选择
> #endif
> 
> ```
>
> ​	（2）CDC协议数据处理
>
> ​	在apps/common/usb/device/cdc.c中，添加自定义的CDC协议数据处理函数，如下：
>
> ```c
> s32 usb_cdc_output_handler(void *priv, u8 *buf, u32 len)
> {
>     //将收到的数据打印
>     printf("len = %d, buf = %s\n", len, buf);
>     
>     //将收到的数据发送至终端
>     u32 ret = cdc_write_data(0, buf, len);
>     printf("tx_ret = %d\n", ret);
>     return 0;
> }
> 
> void cdc_register(const usb_dev usb_id)
> {
> 	//......
>     cdc_set_output_handle(NULL, usb_cdc_output_handler);//注册数据处理函数
>     return;
> __exit_err:
>     //......
> }
> ```
>
> ​		（3）数据收发测试
>
> ​			可通过串口调试助手进行收发测试，如下：
>
> | ![3](.\3.png) |
> | ------------------------------------------------------------ |

> JIELI SDK的编译、烧写等操作方式的说明可在以下文档获取：[文档](../../../../doc/stuff/usb updater.pdf)
>

## 常见问题

> * N/A

## 参考文档

> * N/A
