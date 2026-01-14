# YUV缩放使用示例工程说明

> 本工程展示了YUV缩放使用示例:

---

## 适用平台

> 本工程适用以下芯片类型：
> 1. AC79系列芯片：AC790N、AC791N
>
> 杰理芯片和评估板的更多信息可在以下地址获取：[链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)
>
> 注意：jpeg编解码，芯片务必带sdram 2M以上才能使用。

## 依赖性
>1、依赖库common_lib.a。
>
>2、依赖文件 [yuv_soft_scalling.h](..\..\..\..\include_lib\utils\yuv_soft_scalling.h) 。

## 工程配置说明

> 在SDK选择[demo_hello](../../../../apps/demo_hello/board)主工程文件或者主工程Makefile（其他工程添加需要加 [yuv_soft_scalling.h](..\..\..\..\include_lib\utils\yuv_soft_scalling.h) 和common_lib.a即可），然后添加本事例工程代码
>
> 1、首先先在demo_hello添加SD卡，SD卡移植方法详情example\sd的 [readme.md](..\sd\readme.md) （本次例子从SD卡读取YUV文件进行缩放测试，YUV不在SD卡的跳过该步骤）。
>
> 2、添加 [yuv_soft_scalling.h](..\..\..\..\include_lib\utils\yuv_soft_scalling.h) 到工程。
>
> 3、添加main.c测试例子，如下。
>
> ```c
> #include "app_config.h"
> #include "system/includes.h"
> #include "device/device.h"
> #include "yuv_soft_scalling.h"
> #include "fs/fs.h"
> 
> static void yuv_scalling_test(void)
> {
>     FILE *fd = NULL;
>     int len;
>     int w = 320;//YUV420数据分辨率宽
>     int h = 240;//YUV420数据分辨率高
>     int scal_w = 168;//480;//YUV420缩放后的宽
>     int scal_h = 128;//320;//YUV420缩放后的高
>     void *yuv = NULL;
>     void *yuv_scanl = NULL;
>     int ret;
> 
>     extern int storage_device_ready(void);
>     while (!storage_device_ready()) {//等待sd文件系统挂载完成
>         os_time_dly(2);
>     }
> 
>     //1.从SD卡打开yuv文件数据到内存中
>     fd = fopen(CONFIG_ROOT_PATH"test.yuv", "rb");
>     if (fd == NULL) {
>         printf("no file\n");
>         goto exit;
>     }
>     len = flen(fd);//读取YUV数据长度
>     yuv = malloc(len);
>     if (!yuv) {
>         printf("yuv buf malloc err \n");
>         goto exit;
>     }
>     if (fread(yuv, 1, len, fd) != len) {//读取YUV数据
>         printf("read file len err \n");
>         goto exit;
>     }
>     fclose(fd);
>     fd = NULL;
> 
>     //2.申请缩放的内存
>     yuv_scanl = malloc(scal_w * scal_h * 3 / 2);//YUV420大小：分辨率*1.5
>     if (!yuv_scanl) {
>         printf("yuv_scanl buf malloc err \n");
>         goto exit;
>     }
> 
>     //3.缩放YUV
>     ret = YUV420p_Soft_Scaling(yuv, yuv_scanl, w, h, scal_w, scal_h);//返回值为缩放后的数据量大小
>     if(ret){
>         fd = fopen(CONFIG_ROOT_PATH"test_scal.yuv", "w+");//创建文件
>         if (fd) {
>             fwrite(yuv_scanl, 1, ret, fd);
>             fclose(fd);
>             fd = NULL;
>             printf("YUV420p_Soft write file ok\n");
>         }
>     }
> 
> exit:
>     if (yuv_scanl) {
>         free(yuv_scanl);
>         yuv_scanl = NULL;
>     }
>     if (yuv) {
>         free(yuv);
>         yuv = NULL;
>     }
>     if (fd) {
>         fclose(fd);
>         fd = NULL;
>     }
>     while (1) {//等待被杀（注意：不能自杀）
>         os_time_dly(10);
>     }
> }
> static int c_main(void)
> {
>     os_task_create(yuv_scalling_test, NULL, 12, 1000, 0, "yuv_scalling_test");
>     return 0;
> }
> late_initcall(c_main);
> 
> ```
>
> 4、YUV缩放API详解，API头文件详见 [yuv_soft_scalling.h](..\..\..\..\include_lib\utils\yuv_soft_scalling.h) 
>
> ```c
> 1）YUYV422转YUV422P
> //yuyv:YUYV源, yuv422p:YUV422P输出, width/height:源宽/源高，返回值：转换后数据大小，0出错
> int YUYV422ToYUV422p(unsigned char *yuyv, unsigned char *yuv422p, int width, int height);
> 
> 2）YUV444P转YUV422P
> //yuv444p:YUV444P源, yuv422p:YUV422P输出, width/height:源宽/源高，返回值：转换后数据大小，0出错
> int YUV444pToYUV422p(unsigned char *yuv444p, unsigned char *yuv422p, int width, int height);
> 
> 3）YUV444P转YUV420P
> //yuv444p:YUV444P源, yuv420p:YUV420P输出, width/height:源宽/源高，返回值：转换后数据大小，0出错
> int YUV444pToYUV420p(unsigned char *yuv444p, unsigned char *yuv420p, int width, int height);
> 
> 4）YUV422P转YUV420P
> //yuv422p:YUV422P源, yuv420p:YUV420P输出, width/height:源宽/源高，返回值：转换后数据大小，0出错
> int YUV422pToYUV420p(unsigned char *yuv422p, unsigned char *yuv420p, int width, int height);
> 
> 5）YUV420P翻转
> //src:YUV源,out:YUV输出,src_w/src_h/out_w/out_h:源宽/源高/输出宽/输出高, angle:翻转角度(0/90/180/270)，返回值：转换后数据大小，0出错
> int YUV420p_REVERSAL(char *src, char *out, int src_w, int src_h, int *out_w, int *out_h, int angle);
> 
> 6）YUV420P尺寸对齐，只能小于源尺寸
> //YUV分辨率进行对齐，对齐只能比源尺寸要小(不会变大)，返回值：转换后数据大小，0出错
> //src:YUV源,src_w/src_h/out_w/out_h/width_align/height_align:源宽/源高/输出宽/输出高/宽对齐像素点数/高对齐像素点数
> int YUV420p_ALIGN(unsigned char *src, int src_w, int src_h, int *out_w, int *out_h, int width_align, int height_align);
> 
> 7）YUV420P缩小任意尺尺寸（2对齐）
> //YUV缩小任意尺寸
> //AC790N/AC791N(WL80/WL82) sys_clk=320M , 26ms:720P->VGA, 8ms:720P->320*240, 6ms:VGA->320*240;
> //缩小可以不使用out作为缓存，out写NULL即可，直接在src覆盖
> //src:YUV源,out:YUV输出,src_w/src_h/out_w/out_h:源宽/源高/输出宽/输出高
> //返回值：转换后数据大小，0出错
> int YUV420p_Zoom_Out(unsigned char *src, unsigned char *out, int src_w, int src_h, int out_w, int out_h);
> 
> 8）YUV420P放大任意尺尺寸（2对齐）
> //YUV放大任意尺寸
> //AC790N/AC791N(WL80/WL82) sys_clk=320M , 80ms:VGA->720P, 130ms:VGA->1080P, 260ms:720P->1080P;
> //src:YUV源,out:YUV输出,src_w/src_h/out_w/out_h:源宽/源高/输出宽/输出高
> //返回值：转换后数据大小，0出错
> int YUV420p_Zoom_In(unsigned char *src, unsigned char *out, int src_w, int src_h, int out_w, int out_h);
> 
> 9）YUV420P缩放意尺尺寸（2对齐）
> //YUV缩放
> //AC790N/AC791N(WL80/WL82) sys_clk=320M , 26ms:720P->VGA, 8ms:720P->320*240, 6ms:VGA->320*240, 80ms:VGA->720P, 130ms:VGA->1080P, 260ms:720P->1080P;
> //src:YUV源,out:YUV输出,src_w/src_h/out_w/out_h:源宽/源高/输出宽/输出高
> //返回值：转换后数据大小，0出错
> int YUV420p_ZOOM(unsigned char *src, unsigned char *out, int src_w, int src_h, int out_w, int out_h);
> 
> 10）YUV420P缩放意尺尺寸（2对齐）
> //YUV缩放,和YUV420p_ZOOM一样
> //AC790N/AC791N(WL80/WL82) sys_clk=320M , 26ms:720P->VGA, 8ms:720P->320*240, 6ms:VGA->320*240, 80ms:VGA->720P, 130ms:VGA->1080P, 260ms:720P->1080P;
> //src:YUV源,out:YUV输出,src_w/src_h/out_w/out_h:源宽/源高/输出宽/输出高
> //返回值：转换后数据大小，0出错
> int YUV420p_Soft_Scaling(unsigned char *src, unsigned char *out, int src_w, int src_h, int out_w, int out_h);
> 
> 11）YUV420P裁剪指定区域（2对齐）
> //YUV裁剪指定区域
> //yuv420p:YUV源,src_w/src_h/源宽/源高/
> //out:指定输出YUV缓存区，NULL则覆盖yuv420p，out_size指定输出缓冲区长度，out=NULL时out_size = 0即可
> //w_start:裁剪起始宽(2对齐)，w_size:裁剪长度(2对齐)
> //h_start:裁剪起始高(2对齐)，h_size:裁剪长度(2对齐)
> //宽高起始(0,0)点为图片的左上角
> int YUV420p_Cut(unsigned char *yuv420p, int src_w, int src_h, unsigned char *out, int out_size, int w_start, int w_size, int h_start, int h_size);
> 
> 
> 12）RGB旋转90度
> //in_buf:RGB源,out_buf:RGB输出
> void RGB_Soft_90(unsigned char mode, char *out_buf, char *in_buf, int width, int height);
> ```
>
> YUV转换例子想详见[jpeg_test.c](jpeg_test.c) 。
>
> **注意：**当在YUV缩小时的使用函数YUV420p_Zoom_Out、YUV420p_ZOOM、YUV420p_Soft_Scaling函数，第二个参数out可以为NULL；当out为NULL，则缩小的数据会覆盖src数据源，放大不支持out为NULL。
>
> 

---



## 操作说明

> 1. 编译工程，烧录镜像，复位启动
>
> 2. 系统启动后，可以通过串口软件看到示例的打印信息
>
> JIELI SDK的编译、烧写等操作方式的说明可在以下文档获取：[文档](../../../../doc/stuff/usb updater.pdf)

---

## 常见问题

> * N/A
>
> 

## 参考文档

> * N/A