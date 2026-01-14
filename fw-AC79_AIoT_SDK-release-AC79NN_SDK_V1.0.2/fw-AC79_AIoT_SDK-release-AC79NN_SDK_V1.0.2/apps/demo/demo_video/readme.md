﻿﻿﻿﻿# VIDEO使用示例工程说明

> 本工程展示了video的jpeg图片使用示例:

---

## 适用平台

> 本工程适用以下芯片类型：
> 1. AC79系列芯片：AC790N、AC791N
>
> 杰理芯片和评估板的更多信息可在以下地址获取：[链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)

## 工程配置说明

> 说明：使用demo_video前最好先看摄像头移植，确保摄像头正常工作 [readme.md](..\..\common\example\dvp_摄像头说明\camera_sensor\readme.md) 
>
> 1、user_video_rec.c为用户打开、关闭、音视频数据回调的VIDEO接口。
>
> 2、video_rt_usr.c为协议层转发数据到应用层的视频输出接口。
>
> 3、stream_protocol.c为用户协议转发层，默认用户方式为转发到video_rt_usr.c。
>
> user_video_rec.c、video_rt_usr.c、stream_protocol.c一起配合使用。
>
> 
>
> 用户使用VIDEO配置和注意地方：
>
> 1、app_config.h打开用户VIDEO宏：#define CONFIG_USR_VIDEO_ENABLE		//用户VIDEO使能
>
> 当有咪头需要音频数据时，打开#define VIDEO_REC_AUDIO_SAMPLE_RATE		8000//音频采样率,注意：硬件没MIC则为0
>
> 注意board.c的音频mic硬件信息配置需要符合实际硬件电路，否则无法出音频数据，例如：
>
> /*
> *MIC的ADC配置
> */
> static const struct adc_platform_data adc_data = {
>  .mic_channel = LADC_CH_MIC1_P_N,// | LADC_CH_MIC3_P_N //硬件mic的接口
>  .mic_ch_num = 1,//硬件总共MIC个数,默认1个
>  .isel = 2,//默认保留参数
> };
>
> 2、配置video的大小，配置在video_buf_config.h头文件找到USER_VREC0_FBUF_SIZE和USER_AUDIO_BUF_SIZE宏，即应用层分配给地方音视频输出的缓冲区。
>
> USER_VREC0_FBUF_SIZE大小至少为两帧JPEG图片字节大小以上，一般VGA分辨率为80K-100K，720P分辨率为100K-120K；
>
> 音频固定采样率为8000，一包数据大小为8192（500ms出一帧音频），音频BUFF大小最小为16K。
>
> 3、确保摄像头驱动有加上，确保board.c的CONFIG_VIDEO_ENABLE宏控制的摄像头IO组选择正确。
>
> 4、app_main.c的hello_video_task函数则为使用例子。
>
> 
>
> **使用流程：**
> 1、调用user_video_rec0_open,打开摄像头成功后，会回调到video_rt_usr_init。
>
> 2、音视频数据回调：user_video_frame_callback函数，注意：该函数不能进行延时太长操作，否则丢帧，数据回调用在发送数据或者拷贝数据。
>
> 3、调用user_video_rec0_close关闭摄像头。

### （1）视频请求和关闭

user_video_rec.c

---

> 视频请求打开函数：user_video_rec0_open()，步骤：
>
> - 请求video0或1的子设备的服务，在video设备里支持子父设备，比如可以打开多个video0子设备，传参"video0.0"、"video0.1"、"video0.2"等。代码块如下。
>
> ```c
>     if (!__this_user->user_video_rec) {
>         __this_user->user_video_rec = server_open("video_server", "video0.1");//打开video0子设备1的服务
>         if (!__this_user->user_video_rec) {
>             goto __exit;
>         }
>     }
> ```
>
> - 在联合体配置视频参数，包括：buffer模式、双路模式、分辨率、帧率、是否需要音频、应用层jpeg的缓冲区等，如下代码块。
>
> ```c
>     req.rec.bfmode = VIDEO_PPBUF_MODE;// 0默认使用lbuffer模式；VIDEO_PPBUF_MODE使用乒乓buffer模式
>     req.rec.wl80_spec_mode = VIDEO_WL80_SPEC_DOUBLE_REC_MODE;//0默认单路模式；VIDEO_WL80_SPEC_DOUBLE_REC_MODE双路模式
>     req.rec.picture_mode = 0;//0非绘本模式（ISC硬件缓冲区使用32行YUV内存），1绘本模式（ISC硬件使用一帧YUV内存，和获取YUV数据一起使用则用到）
>     req.rec.isc_sbuf = __this_user->user_isc_buf;//可配置的ISC硬件缓冲区（默认32行YUV内存）、可指定到内部ram，与picture_mode配合使用。
>     req.rec.sbuf_size = USER_ISC_SBUF_SIZE;//可配置的ISC硬件缓冲区大小
>     req.rec.camera_type = VIDEO_CAMERA_NORMAL;//摄像头类型，VIDEO_CAMERA_NORMAL为正常默认
>     req.rec.channel = 1;//视频输出的通道，默认1，该值在请求关闭是的通道要一致，否则无法关闭。
>     req.rec.width 	= CONFIG_VIDEO_IMAGE_W;//视频分辨率：单路则和摄像头分辨率一致；双路则任意分辨率（不是摄像头原始分辨率会占用CPU进行缩放）
>     req.rec.height 	= CONFIG_VIDEO_IMAGE_H;//视频分辨率：单路则和摄像头分辨率一致；双路则任意分辨率（不是摄像头原始分辨率会占用CPU进行缩放）
>     req.rec.state 	= VIDEO_STATE_START;//启动video状态命令
>     req.rec.format 	= USER_VIDEO_FMT_AVI;//用户自定义AVI格式
>     req.rec.quality = VIDEO_LOW_Q;//VIDEO_MID_Q;//起始的请求视频质量（低等质量、中等质量、高等质量）
>     req.rec.fps = 0;//摄像头原始帧率，0则跟随摄像头
>     req.rec.real_fps = 15;//实际输出到应用层的帧率
> 
>     //需要音频：请写audio.sample_rate和audio.buf、audio.buf_len
>     req.rec.audio.sample_rate = VIDEO_REC_AUDIO_SAMPLE_RATE;//8000 音频采样率
>     req.rec.audio.channel = 1;//选择硬件1个MIC数
>     req.rec.audio.channel_bit_map = 0;//选择第几个MIC，0则是版籍配置文件默认,//LADC_CH_MIC1_P_N
>     req.rec.audio.volume = 100;//音频增益0-100
>     req.rec.audio.buf = __this_user->user_audio_buf;//音频BUFF
>     req.rec.audio.buf_len = USER_AUDIO_BUF_SIZE;//音频BUFF长度
> 
>     req.rec.abr_kbps = user_video_rec_get_abr(req.rec.width);//JPEG图片码率
>     req.rec.buf = __this_user->user_video_buf;//视频缓冲区
>     req.rec.buf_len = USER_VIDEO_SBUF_SIZE;//视频缓冲区长度
>     req.rec.block_done_cb = 0;//保留参数（ISC硬件接收块数据回调）
> 
>     //时间OSD配置
> #ifdef CONFIG_OSD_ENABLE
>     struct video_text_osd text_osd = {0};//时间OSD配置
>     text_osd.font_w = OSD_DEFAULT_WIDTH;//必须16对齐
>     text_osd.font_h = OSD_DEFAULT_HEIGHT;//必须16对齐
>     text_osd.text_format = user_osd_format_buf;//时间格式
>     text_osd.x = (req.rec.width - text_osd.font_w * strlen(text_osd.text_format) + 15) / 16 * 16;//起始纵轴像素点位置
>     text_osd.y = (req.rec.height - text_osd.font_h + 15) / 16 * 16;//起始横轴像素点位置
>     text_osd.osd_yuv = 0xe20095;//OSD颜色
>     req.rec.text_osd = &text_osd;
> #endif
> #ifdef CONFIG_USR_VIDEO_ENABLE
>     sprintf(path, "usr://%s", CONFIG_USR_PATH);//用户自定义协议路径
> #else
>     sprintf(path, "usr://%s", "192.168.1.1");//默认用户自定义协议路径
> #endif
> ```
>
> - 服务请求视频打开、注册数据包回调函数、协议转发任务创建、注册数据回调处理函数，如下代码块。
>
> ```c
> 	err = server_request(__this_user->user_video_rec, VIDEO_REQ_REC, &req);//传参，请求服务
>     if (err != 0) {
>         puts("user start rec err\n");
>         goto __exit;
>     }
>     req.rec.packet_cb = stream_packet_cb;//注册数据包回调函数进行协议转发
>     err = server_request(__this_user->user_video_rec, VIDEO_REQ_SET_PACKET_CALLBACK, &req);
>     if (err != 0) {
>         puts("stream_packet_cb set err\n");
>         goto __exit;
>     }
> #if 1 //需要加上video_rt_usr.c文件则可以创建协议转发任务
>     err = stream_protocol_task_create(path, NULL);//协议转发任务创建
>     if (err != 0) {
>         puts("stream_packet_cb set err\n");
>         goto __exit;
>     }
>     set_video_rt_cb(user_video_frame_callback);//注册用户自定义协议的数据回调处理函数
> #endif
>     __this_user->state = true;
>     printf("user video rec open ok\n");
> ```
>
> 
>
> 视频请求关闭函数：user_video_rec0_close()，步骤：
>
> - 设置关闭的通道和状态、请求服务、关闭服务、删除协议转发任务，如下代码块。
>
> ```c
> 	union video_req req = {0};
>     if (__this_user->user_video_rec) {
>         req.rec.channel = 1;//请求关闭的通道
>         req.rec.state = VIDEO_STATE_STOP;//video关闭的状态
>         err = server_request(__this_user->user_video_rec, VIDEO_REQ_REC, &req);//传参，请求服务
>         if (err != 0) {
>             printf("\nstop rec err 0x%x\n", err);
>             return -1;
>         }
>         server_close(__this_user->user_video_rec);//关闭服务
> #if 1//需要加上video_rt_usr.c文件则可删除协议转发任务
>         stream_protocol_task_kill();//删除协议转发任务
> #endif
>         __this_user->user_video_rec = NULL;
>         __this_user->state = false;
>         printf("user video rec close ok\n");
>     }
> ```

---

### （2）视频数据回调

stream_protocol.c 的stream_packet_cb函数

---

> - 当需要使用协议转发处理则（user_video_rec0_open打开协议转发任务创建并注册用户自定义数据回调），则stream_packet_cb函数如下代码块。
>
> ```c
> int stream_packet_cb(u8 type, u8 *buf, u32 size)
> {
>     u32 *head = (u32 *)buf;
>     if (!strm_ptl->init || strm_ptl->err || strm_ptl->kill) {
>         return 0;
>     } else if ((size <= 8) && (*head == JL_ENDF)) {
>         return size;
>     }
>     if (type == VIDEO_REC_JPEG_TYPE_VIDEO) {//视频
>         if (size > VIDEO_PKBUFF_SIZE) {
>             printf("VDPKBUFF_SIZE no enough !!!\n");
>             return 0;
>         }
>         if ((*head == JL_000DC && *(head + 2) == JPEG_HEAD) || *(head + 2) == JPEG_HEAD) {//头部判断或偏移
>             buf += 8;
>             size -= 8;
>         }
>         if (!strm_ptl->vd_use && !strm_ptl->kill) {
>             strm_ptl->copy = true;
>             strm_ptl->vd_use = true;
>             strm_ptl->vd_len = size;
>             memcpy(strm_ptl->video_pkbuff, buf, size);//拷贝数据
>             strm_ptl->copy = false;
>             os_sem_post(&strm_ptl->sem);
>         }
>     } else if (type == VIDEO_REC_PCM_TYPE_AUDIO) {//音频
> #ifdef AUDIO_PKBUFF_SIZE
>         if (size > AUDIO_PKBUFF_SIZE) {
>             printf("ADPKBUFF_SIZE no enough !!!\n");
>             return 0;
>         }
>         if (!strm_ptl->ad_use && !strm_ptl->kill) {
>             strm_ptl->copy = true;
>             strm_ptl->ad_use = true;
>             strm_ptl->ad_len = size;
>             memcpy(strm_ptl->audio_pkbuff, buf, size);//拷贝数据
>             strm_ptl->copy = false;
>             os_sem_post(&strm_ptl->sem);
>         }
> #endif
>     }
>     return size;
> }
> ```
>
> stream_protocol_task任务负责打开用户自定义协议，并把stream_packet_cb函数回调里复制的数据进行转发到用户自定义协议。
>
> ```c
> 	ret = stream_protocol_open(strm_ptl->spath.path, strm_ptl->spath.mode);//用户自定义协议打开（在.net_video_stream段里查找）
>     if (ret) {
>         printf("stream_protocol_open err, path : %s \n", strm_ptl->spath.path);
>         strm_ptl->err = true;
>         goto exit;
>     }
> ```
>
> ```c
> ......
> 				ret = stream_protocol_write(JPEG_TYPE_VIDEO, strm_ptl->video_pkbuff, strm_ptl->vd_len);//转发视频
>                 strm_ptl->vd_use = false;
>                 if (!ret) {
>                     strm_ptl->err = true;
>                     printf("err : stream_protocol_write, kill task !!!\n");
>                     break;
>                 }
> #ifdef AUDIO_PKBUFF_SIZE
>             if (strm_ptl->ad_use) {
>                 ret = stream_protocol_write(PCM_TYPE_AUDIO, strm_ptl->audio_pkbuff, strm_ptl->ad_len);//转发音频
>                 strm_ptl->ad_use = false;
>                 if (!ret) {
>                     strm_ptl->err = true;
>                     printf("err : stream_protocol_write, kill task !!!\n");
>                     break;
>                 }
>             }
> #endif
> ```
>
> - 当不使用协议转发处理（不用创建协议转发任务），只获取回调数据的回调函数如下代码块，则修改如下代码块。
>
> ```c
> int stream_packet_cb(u8 type, u8 *buf, u32 size)
> {
>     if (type == VIDEO_REC_JPEG_TYPE_VIDEO) {//视频
>         if ((*head == JL_000DC && *(head + 2) == JPEG_HEAD) || *(head + 2) == JPEG_HEAD) {//头部判断或偏移
>             buf += 8;
>             size -= 8;
>         }
>         //使用：buf和size
> 
>     } else if (type == VIDEO_REC_PCM_TYPE_AUDIO) {//音频
> 	   //使用：buf和size
> 
>     }
>     return size;
> }
> ```

---

### （3）用户自定义协议数据回调

video_rt_usr.c，包括初始化、数据回调、关闭，代码块如下。

---

> ```c
> static u32(*video_rt_cb)(void *buf, u32 len, u8 type);
> void set_video_rt_cb(u32(*cb)(void *buf, u32 len, u8 type))//可设置的数据回调处理
> {
>  video_rt_cb = cb;
> }
> static void *video_rt_usr_init(const char *path, const char *mode)//用户自定义协议的打开
> {
>  //如：连接服务器等操作
>  puts("video_rt_usr_init\n");
> 
>  return calloc(1, sizeof(struct rt_stream_info));//需要申请返回的自定义参数
> }
> static int video_rt_usr_send(void *hdr, void *data, u32 len, u8 type)//音视频数据回调处理
> {
>  struct rt_stream_info *r_info = (struct rt_stream_info *)hdr;//为video_rt_usr_init的返回的指针参数
> 
>  if (video_rt_cb) {
>      video_rt_cb(data, len, type);
>  }
> 
>  return len;
> }
> static int video_rt_usr_uninit(void *hdr)//关闭自定义协议
> {
>  //如：关闭接服务器等操作
>  puts("video_rt_usr_uninit\n");
>  struct rt_stream_info *r_info = (struct rt_stream_info *)hdr;
>  free(r_info);
>  return 0;
> }
> REGISTER_NET_VIDEO_STREAM_SUDDEV(usr_video_stream_sub) = {//注册在.net_video_stream段里，stream_protocol_open时遍历
>  .name = "usr",
>  .open =  video_rt_usr_init,
>  .write = video_rt_usr_send,
>  .close = video_rt_usr_uninit,
> };
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