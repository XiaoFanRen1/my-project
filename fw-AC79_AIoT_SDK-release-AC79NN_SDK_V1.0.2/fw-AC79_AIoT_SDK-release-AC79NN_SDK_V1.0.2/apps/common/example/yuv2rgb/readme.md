# YUV和RGB转换用示例工程说明

> 本工程展示了YUV和RGB转换使用示例:

---

## 适用平台

> 本工程适用以下芯片类型：
> 1. AC79系列芯片：AC790N、AC791N
>
> 杰理芯片和评估板的更多信息可在以下地址获取：[链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)

## 依赖性
>1、依赖库common_lib.a。
>
>2、依赖文件 [yuv_to_rgb.h](..\..\..\..\include_lib\utils\yuv_to_rgb.h) 。

## 工程配置说明

> 在SDK选择[demo_hello](../../../../apps/demo_hello/board)主工程文件或者主工程Makefile（其他工程添加需要加 [yuv_to_rgb.h](..\..\..\..\include_lib\utils\yuv_to_rgb.h) 和common_lib.a即可），然后添加本事例工程代码
>
> 1、首先先在demo_hello添加SD卡，SD卡移植方法详情example\sd的 [readme.md](..\sd\readme.md) （本次例子从SD卡读取YUV文件进行转RGB测试，YUV不在SD卡的跳过该步骤）。
>
> 2、添加 [yuv_to_rgb.h](..\..\..\..\include_lib\utils\yuv_to_rgb.h) 到工程。
>
> 3、添加main.c测试例子，如下。
>
> ```c
> #include "app_config.h"
> #include "system/includes.h"
> #include "device/device.h"
> #include "yuv_to_rgb.h"
> #include "fs/fs.h"
> 
> static void yuv2rgb_test(void)
> {
>        FILE *fd = NULL;
>        int len;
>        int w = 320;//YUV420数据分辨率宽
>        int h = 240;//YUV420数据分辨率高
>        void *yuv = NULL;
>        void *rgb = NULL;
>        int ret;
>    
>        extern int storage_device_ready(void);
>        while (!storage_device_ready()) {//等待sd文件系统挂载完成
>            os_time_dly(2);
>        }
>    
>        //1.从SD卡打开yuv文件数据到内存中
>       fd = fopen(CONFIG_ROOT_PATH"test.yuv", "rb");
>        if (fd == NULL) {
>            printf("no file\n");
>            goto exit;
>        }
>        len = flen(fd);//读取YUV数据长度
>        yuv = malloc(len);
>        if (!yuv) {
>            printf("yuv buf malloc err \n");
>            goto exit;
>        }
>        if (fread(yuv, 1, len, fd) != len) {//读取YUV数据
>            printf("read file len err \n");
>            goto exit;
>        }
>        fclose(fd);
>        fd = NULL;
>    
>        //2.申请缩放的内存
>        rgb = malloc(w * h * 3);//RGB24数据量: 分辨率*3
>        if (!rgb) {
>            printf("rgb buf malloc err \n");
>            goto exit;
>        }
>    
>        //3.YUV转RGB24
>        yuv420p_quto_rgb24(yuv, rgb, w, h);
>        fd = fopen(CONFIG_ROOT_PATH"test_rgb24.rgb", "w+");//创建文件
>        if (fd) {
>            fwrite(rgb, 1, w * h * 3, fd);//rgb24 : w * h * 2
>            fclose(fd);
>            fd = NULL;
>            printf("YUV2RGB24 write file ok\n");
>        }
>    
>        //4.YUV转RGB565
>        yuv420p_quto_rgb565(yuv, rgb, w, h, 1);//转RGB565-BE大端模式
>        fd = fopen(CONFIG_ROOT_PATH"test_rgb565.rgb", "w+");//创建文件
>        if (fd) {
>         fwrite(rgb, 1, w * h * 2, fd);//rgb565 : w * h * 2
>            fclose(fd);
>            fd = NULL;
>            printf("YUV2RGB565 write file ok\n");
>        }
>    
>        //5.RGB565转YUV
>        fd = fopen(CONFIG_ROOT_PATH"test_rgb565.rgb", "rb");//读取文件
>        if (fd) {
>            fread(rgb, 1, w * h * 2, fd);//rgb565 : w * h * 2
>            fclose(fd);
>            fd = NULL;
>    
>            //RGB565转YUV
>            rgb565_to_yuv420p(rgb, yuv, w, h, 1);//RGB565-BE大端模式转YUV420P
>    
>         fd = fopen(CONFIG_ROOT_PATH"test_rgb2yuv.yuv", "w+");//创建文件
>         if (fd) {
>             fwrite(yuv, 1, w * h * 3 / 2, fd);//yuv420p : w * h * 3 / 2
>                fclose(fd);
>                fd = NULL;
>             printf("RGB2YUV write file ok\n");
>         }
>     }
> 
>exit:
>     if (rgb) {
>        free(rgb);
>         rgb = NULL;
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
>     os_task_create(yuv2rgb_test, NULL, 12, 1000, 0, "yuv2rgb_test");
>     return 0;
> }
> late_initcall(c_main);
> 
> ```
> 
> 4、YUV转RGB的API详解 [yuv_to_rgb.h](..\..\..\..\include_lib\utils\yuv_to_rgb.h) 。
> 
> ```c
> 1）YUV420P转RGB24
> //yuvBuffer_in:YUV输入源数据缓存区, rgbBuffer_out:rgb输出缓存区，width/height:分辨率宽高
> void yuv420p_quto_rgb24(unsigned char *yuvBuffer_in, unsigned char *rgbBuffer_out, int width, int height);
> 
> 2）YUV420P转RGB565，RGB565存在大小端问题，不同设备使用的RGB大小端不同，务必注意,
> //yuvBuffer_in:YUV输入源数据缓存区, rgbBuffer_out:rgb输出缓存区，width/height:分辨率宽高, be:大小端存储:1大端,0小端
> void yuv420p_quto_rgb565(unsigned char *yuvBuffer_in, unsigned char *rgbBuffer_out, int width, int height, char be);
> 
> 3）YUV422P转RGB565，RGB565存在大小端问题，不同设备使用的RGB大小端不同，务必注意
> //yuvBuffer_in:YUV输入源数据缓存区, rgbBuffer_out:rgb输出缓存区，width/height:分辨率宽高, be:大小端存储:1大端,0小端
> void yuv422p_quto_rgb565(unsigned char *yuvBuffer_in, unsigned char *rgbBuffer_out, int width, int height, char be);
> 
> 4）YUV444P转RGB565，RGB565存在大小端问题，不同设备使用的RGB大小端不同，务必注意
> //yuvBuffer_in:YUV输入源数据缓存区, rgbBuffer_out:rgb输出缓存区，width/height:分辨率宽高, be:大小端存储:1大端,0小端
> void yuv444p_quto_rgb565(unsigned char *yuvBuffer_in, unsigned char *rgbBuffer_out, int width, int height, char be);
> 
> 5）RGB565转YUV420P，RGB565存在大小端问题，不同设备使用的RGB大小端不同，char be为RGB输入源的大小端
> //rgb565:rgb565输入源数据缓存区, yuv420p:YUV数据输出缓存区, width/height:分辨率宽高, be:大小端存储:1大端,0小端
> int rgb565_to_yuv420p(unsigned char *rgb565, unsigned char *yuv420p, int width, int height, char be);
> 
> 6）RGB24转YUV420P
> //rgb24:rgb24输入源数据缓存区, yuv420p:YUV数据输出缓存区, width/height:分辨率宽高
> int rgb24_to_yuv420p(unsigned char *rgb24, unsigned char *yuv420p, int width, int height);
> ```

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