# 图传、录卡或者UVC显示的摄像头-高帧率说明

摄像头高帧率：20帧以上，则需要修改工程配置才能达到高帧率传输或显示，以下修改为：**wifi_camera**工程，其他工程大同小异。

**（1）修改app_config.h的VIDEO_REC buffer模式和输出帧率**

> 注释乒乓buff模式，恢复使用lbuf（lbuf详情 [readme.md](..\lbuf\readme.md) ），如下：
>
> ```c
> //（1）buffer模式：注释CONFIG_VIDEO_REC_PPBUF_MODE。
> //#define CONFIG_VIDEO_REC_PPBUF_MODE
> 
> //（2）图传或UVC显示的输出帧率：如设置25。
> #define  NET_VIDEO_REC_FPS0   			25  //不使用0作为默认值，写具体数值
> 
> //（3）音频数据：有则采样率为8000，没有则为0，如下为0。
> #define VIDEO_REC_AUDIO_SAMPLE_RATE     0
> 
> //（4）SD卡录像的帧率：没有SD卡则0或不修改，如25帧。
> #define VIDEO_REC_FPS                   25
> ```
>
> 

**（2）确定工程没有开CONFIG_NO_SDRAM_ENABLE宏，工程使用sdram**

> windows环境，coldblocks：打开cbp工程，右键工程，build options--->compiler--->#define选项，查看是否含有：CONFIG_NO_SDRAM_ENABLE，有则说明不需要sdam，没有则需要sdram。在高帧率情况下，都是需要sdram，内存才足够，否则是无法满足高帧率。因此需要去掉：CONFIG_NO_SDRAM_ENABLE。
>
> linux环境，makefile：关闭CONFIG_NO_SDRAM_ENABLE宏。

**（3）修改video_buf_config.h的应用层视频缓冲区大小**

> video_buf_config.h对应内存配置说明如下，一般修改图传和UVC显示：NET_VREC0_FBUF_SIZE为300K即可。
>
> ```c
> #define VIDEO0_REC_FORMAT   VIDEO_FMT_AVI
> #define VIDEO1_REC_FORMAT   VIDEO_FMT_AVI
> #define VIDEO2_REC_FORMAT   VIDEO_FMT_AVI
> #ifdef CONFIG_PSRAM_ENABLE
> #define VREC0_FBUF_SIZE     (512*1024)
> #define VREC1_FBUF_SIZE     (256*1024)
> #define VREC2_FBUF_SIZE     (256 * 1024)
> #define AUDIO_BUF_SIZE      (256*1024)
> #else
> #if (!defined CONFIG_NO_SDRAM_ENABLE && __SDRAM_SIZE__ >= (2 * 1024 * 1024))
> #define VREC0_FBUF_SIZE     (500*1024)//开sdram，SD卡录像视频缓冲区
> #define VREC1_FBUF_SIZE     (0)
> #define VREC2_FBUF_SIZE     (0)
> #define AUDIO_BUF_SIZE      (64*1024)//开sdram，SD卡录像音频缓冲区，没有音频写0
> #else
> #define VREC0_FBUF_SIZE     (150*1024)//不开sdram，SD卡录像修改这里
> #define VREC1_FBUF_SIZE     (0)
> #define VREC2_FBUF_SIZE     (0)
> #define AUDIO_BUF_SIZE      (16*1024)//不开sdram，SD卡录像音频缓冲区，没有音频写0
> #endif
> #endif
> 
> #define USB_CAMERA_BUF_SIZE (1 * 1024 * 1024) // + 512 * 1024)
> #define CAMERA_CAP_BUF_SIZE (1 * 1024 * 1024)
> 
> 
> #if (!defined CONFIG_NO_SDRAM_ENABLE && __SDRAM_SIZE__ >= (2 * 1024 * 1024))
> #define NET_VREC0_FBUF_SIZE     (200*1024)//开sdram，图传或UVC显示视频缓冲区
> #define NET_VREC1_FBUF_SIZE     (0)
> #define NET_AUDIO_BUF_SIZE      (64*1024)//开sdram，图传或UVC显示音频缓冲区，没有音频写0
> #else
> #define NET_VREC0_FBUF_SIZE     (100*1024)//不开sdram，图传或UVC显示视频缓冲区
> #define NET_VREC1_FBUF_SIZE     (0)
> #define NET_AUDIO_BUF_SIZE      (16*1024)//开sdram，图传或UVC显示音频缓冲区，没有音频写0
> #endif
> ```
>
> ①当使用SD卡录像则可以修改：VREC0_FBUF_SIZE 和 AUDIO_BUF_SIZE大小；当需要提高视频帧率或者视频卡顿现象，改大：VREC0_FBUF_SIZE（一般VGA 500K， 720P 800K）；当需要提高音频卡顿现象，改大：AUDIO_BUF_SIZE（一般128K）。
>
> ②当使用网络图传或UVC显示则可以修改：NET_VREC0_FBUF_SIZE 和 NET_AUDIO_BUF_SIZE大小；当需要提高视频帧率或者视频卡顿现象，改大：NET_VREC0_FBUF_SIZE （**一般VGA 300K， 720P 500K**）；当需要提高音频卡顿现象，改大：NET_AUDIO_BUF_SIZE（一般128K）。
>
> ③**确定user_video_rec.c有没有添加到工程，wifi_camera工程默认不加**
>
> 当user_video_rec.c添加到工程编译，则在文件中修改：USER_VREC0_FBUF_SIZE 大小（一般为300K，使用音频需改善音频卡顿则修改：USER_AUDIO_BUF_SIZE（一般128K），没有音频则USER_AUDIO_BUF_SIZE为0），其他demo则USER_VREC0_FBUF_SIZE在对应的app_config.h。

**（4）修改提高摄像头源视频输出帧率，确定摄像头源输出高帧率**

> ①例如DVP-GC0308摄像头驱动设置25帧：
>
>  #define CONFIG_INPUT_FPS    25
>
> **注意：摄像头输出帧率需要确保是源头帧率，不能只修改CONFIG_INPUT_FPS的值，如原本驱动只能15帧，修改驱动软件的CONFIG_INPUT_FPS为25是不能提高摄像头帧率的，需要修改摄像头寄存器配置。**
>
> ②UVC摄像头则默认20-25帧，具体需要查看UVC摄像头数据手册如何更改帧率，一般不修改。

**（5）使用网络图传则更换lwip库和wifi库**

> 使用网络进行视频传输（图传功能）则：
> ①lwip_2_1_2_sfc.a换lwip_2_1_2.a；
>
> ②wl_wifi_ap_sfc.a换wl_wifi_ap.a；
>
> 或使用STA的wl_wifi_sta_sfc.a换wl_wifi_sta.a；
>
> 或者wl_wifi_sfc.a换wl_wifi.a。

**（6）在图传中，需要提高图片质量**

> 在 [net_video_rec.c](..\..\..\wifi_camera\wifi\net_video_rec.c) 修改码率，适当提高码率（1000-6000），如下。
>
> ```c
> /*码率控制，根据具体分辨率设置*/
> static int net_video_rec_get_abr(u32 width)
> {
>  /*视频码率kbps使用说明:
>   码率：一帧图片以K字节为单位大小 * 帧率 * 8，比如：一帧图片为30KB，帧率为20帧，则码率为30*20*8=4800
>   VGA图片大小说明：低等质量(小于20K)，中等质量(20K-40K)，高质量(大于40K，极限70K)
>   720P图片大小说明：低等质量(小于50K)，中等质量(50k-100K)，高质量(大于100K，极限150K)
>  */
> 
>  if (width <= 640) {
>      return 1800;
>  } else {
>      return 1000;
>  }
> }
> ```
>
> 如果是SD卡录像，则修改码率在 [video_rec.c](..\..\..\wifi_camera\video_rec.c) 的video_rec_get_abr()函数。
>



