# jpeg图片编解码使用示例工程说明

> 本工程展示了jpeg图片使用示例:
>
> 1. jpeg解码YUV;
> 2. YUV编码jpeg;

---

## 适用平台

> 本工程适用以下芯片类型：
> 1. AC79系列芯片：AC790N、AC791N
>
> 杰理芯片和评估板的更多信息可在以下地址获取：[链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)
>
> 注意：jpeg编解码，芯片务必带sdram 2M以上才能使用。

## 依赖性
>1、依赖vdieo相关库，同时依赖芯片需有sdram-2M以上，因此需要在[demo_video](../../../../apps/demo_video/board)工程基础上添加。
>
>2、同时本次例子解码jpeg图片数据源和YUV数据输出均保存在SD卡文件系统，因此需要在[demo_video](../../../../apps/demo_video/board)工程先移植SD卡，SD卡移植方法详情example\sd的 [readme.md](..\sd\readme.md) 。

## 工程配置说明

> 在SDK选择[demo_video](../../../../apps/demo_video/board)主工程文件或者主工程Makefile, 然后添加本事例工程代码
>
> 1、首先先在demo_video添加SD卡，SD卡移植方法详情example\sd的 [readme.md](..\sd\readme.md) 。
>
> 2、添加本例子的 [main.c](main.c) 和 [jpeg_test.c](jpeg_test.c) 到工程中。
>
> 3、注释app_main.c的video创建函数（本次不需要打开video，只是需要video相关库），如下。
>
> ```c
> /*
>  * 应用程序主函数
>  */
> void app_main()
> {
>     printf("\r\n\r\n\r\n\r\n\r\n -----------hello video demo run %s-------------\r\n\r\n\r\n\r\n\r\n", __TIME__);
> //    os_task_create(hello_video_task, NULL, 12, 1000, 0, "hello_video_task");
> }
> ```
>
> 4、jpg图片编解码
>
> A）解码：把SD卡插到电脑，复制本文件路径下的test.jpg到SD卡的根目录，**注意**：SD卡的文件系统需要是FAT32，插SD到设备，上电运行。
>
> ```c
> #include "app_config.h"
> #include "system/includes.h"
> #include "device/device.h"
> 
> static void jpeg_yuv_test(void)
> {
>     int jpeg_decoder_test(const char *path);
>     int jpeg_encode_test(const char *path, int width, int height);
> 	int storage_device_ready(void);
> 
>     while (!storage_device_ready()) {//等待sd文件系统挂载完成
>         os_time_dly(2);
>     }
>     jpeg_decoder_test(CONFIG_ROOT_PATH"test.jpg");//JPG解码成YUV，YUV再转RGB
>     //jpeg_encode_test(CONFIG_ROOT_PATH"test.yuv", 320, 240);//YUV编码成JPG
>     while (1) {
>         os_time_dly(10);
>     }
> }
> static int c_main(void)
> {
>     os_task_create(jpeg_yuv_test, NULL, 12, 1000, 0, "jpeg_yuv_test");
>     return 0;
> }
> late_initcall(c_main);
> 
> ```
>
> B）编码：使用上述的A步骤解码出来的YUV存在SD卡，插SD到设备，上电运行。
>
> ```c
> #include "app_config.h"
> #include "system/includes.h"
> #include "device/device.h"
> 
> static void jpeg_yuv_test(void)
> {
>     int jpeg_decoder_test(const char *path);
>     int jpeg_encode_test(const char *path, int width, int height);
> 	int storage_device_ready(void);
> 
>     while (!storage_device_ready()) {//等待sd文件系统挂载完成
>         os_time_dly(2);
>     }
>     //jpeg_decoder_test(CONFIG_ROOT_PATH"test.jpg");//JPG解码成YUV，YUV再转RGB
>     jpeg_encode_test(CONFIG_ROOT_PATH"test.yuv", 320, 240);//YUV编码成JPG
>     while (1) {
>         os_time_dly(10);
>     }
> }
> static int c_main(void)
> {
>     os_task_create(jpeg_yuv_test, NULL, 12, 1000, 0, "jpeg_yuv_test");
>     return 0;
> }
> late_initcall(c_main);
> 
> ```
>
> 
>
> **说明 [jpeg_test.c](jpeg_test.c) 部分函数：**
>
> int yuv_enc_image(char *yuvdata, int yuvdata_size, char *jpg_buf, int jpg_buf_size, int width, int height, int q_val);
>
> yuv_enc_image函数用户可以直接使用在实际应用中，函数功能为：YUV编码成JPG图片格式，还可根据JPEG_ENC_ONLY_Y宏指定编码灰度。

---



## 操作说明

> 1. 编译工程，烧录镜像，复位启动
>
> 2. 系统启动后，可以通过串口软件看到示例的打印信息
>
> JIELI SDK的编译、烧写等操作方式的说明可在以下文档获取：[文档](../../../../doc/stuff/usb updater.pdf)

### 代码流程

> *  [jpeg_test.c](jpeg_test.c) 编码和解码函数已有详细步骤解析。

---

## 常见问题

> * N/A
>
> 

## 参考文档

> * N/A