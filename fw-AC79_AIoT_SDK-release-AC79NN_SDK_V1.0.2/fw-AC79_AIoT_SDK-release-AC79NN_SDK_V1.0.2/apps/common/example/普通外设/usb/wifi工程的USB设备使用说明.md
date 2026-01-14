# USB设备使用说明

**所有的USB设备功能， 需要在board.c打开对应的USB配置，根据需要的USB口和功能和检测快慢进行具体参数配置，配置和说明如下：**

```c

#if TCFG_USB_SLAVE_ENABLE || TCFG_USB_HOST_ENABLE
static const struct otg_dev_data otg_data = {
    .usb_dev_en = 0x01, //USB口使能：0x1-->USB0；0x2-->USB1；0x3--->USB0和USB1
#if TCFG_USB_SLAVE_ENABLE
    .slave_online_cnt = 10,//USB从机的上线检测次数，单位为detect_time_interval值
    .slave_offline_cnt = 10,//USB从机的下线检测次数，单位为detect_time_interval值
#endif
#if TCFG_USB_HOST_ENABLE
    .host_online_cnt = 10,//US主机的上线检测次数，单位为detect_time_interval值
	.host_offline_cnt = 10,//US主机的下线检测次数，单位为detect_time_interval值
#endif
	.detect_mode = OTG_HOST_MODE | OTG_SLAVE_MODE | OTG_CHARGE_MODE,//检测模式：主机检测、从机检测、拔插充电检测。
    .detect_time_interval = 50,//检测上下线的时间间隔，单位ms，默认50ms（不建议修改变小，否则USB事件可能会丢，最低为10ms）。
};
#endif
```

USB的拔插事件通知在 [device_mount.c](..\..\system\device_mount.c) 的device_event_handler函数，当注册app状态机，则状态机事件处理也会有事件通知（详情 [readme.md](..\app_state_machine\readme.md) ）。

### 1、USB从机UVC功能（电脑端查看设备的摄像头数据）

**（1）wifi_camera工程**

> 配置文件： [app_config.h](..\..\..\wifi_camera\include\app_config.h) ，**注意：先要移植摄像头并调试可以出图，电脑端才能图像**。
>
> ①打开（打开UVC从机连接PC功能和使用USB使用UVC设备功能）：
>
> #define TCFG_PC_ENABLE                      1
>
> #define USB_DEVICE_CLASS_CONFIG (UVC_CLASS) 
>
> ②屏蔽（关闭USB读卡器）：
>
> //#define USB_DEVICE_CLASS_CONFIG (MASSSTORAGE_CLASS)
>
> ③电脑端使用AMCap软件或者其他可以看USB摄像头的软件查看

**（2）wifi_story_machine工程**

> 配置文件： [app_config.h](..\..\..\wifi_story_machine\include\app_config.h) ，注意：摄像头线要移植并调试可以出图，电脑端才能图像。
>
> ①打开（打开UVC从机连接PC功能和使用USB使用UVC设备功能）：
>
> #define TCFG_PC_ENABLE                      1
>
> ②把#define USB_DEVICE_CLASS_CONFIG     改成UVC_CLASS，如下：
>
> #define USB_DEVICE_CLASS_CONFIG (UVC_CLASS) 
>
> ③电脑端使用AMCap软件或者其他可以看USB摄像头的软件查看



### 2、USB从机读卡器功能（设备连接电脑作为读卡器功能）

**（1）wifi_camera工程**

> 配置文件： [app_config.h](..\..\..\wifi_camera\include\app_config.h) 
>
> ①打开（打开UVC从机连接PC功能和使用USB读卡器设备功能）：
>
> #define TCFG_PC_ENABLE                      1
>
> #define USB_DEVICE_CLASS_CONFIG (MASSSTORAGE_CLASS)
>
> ②屏蔽（关闭UVC功能）：
>
> //#define USB_DEVICE_CLASS_CONFIG  (UVC_CLASS) 
>
> **注意：读卡器功能，同时需要打开SD卡功能并插SD卡，电脑端才能显示读卡器的U盘盘符。**

**（2）wifi_story_machine工程**

> 配置文件： [app_config.h](..\..\..\wifi_story_machine\include\app_config.h) 
>
> ①打开（打开UVC从机连接PC功能和使用USB读卡器设备功能）：
>
> #define TCFG_PC_ENABLE                      1
>
> ②把#define USB_DEVICE_CLASS_CONFIG     改成MASSSTORAGE_CLASS，如下：
>
> #define USB_DEVICE_CLASS_CONFIG (MASSSTORAGE_CLASS) 
>
> **注意：读卡器功能，同时需要打开SD卡功能并插SD卡，电脑端才能显示读卡器的U盘盘符。**



### 3、USB主机读U盘功能（插U盘到设备USB口）

**（1）wifi_camera或wifi_story_machine工程**

> wifi_camera配置文件： [app_config.h](..\..\..\wifi_camera\include\app_config.h) ，wifi_story_machine配置文件： [app_config.h](..\..\..\wifi_story_machine\include\app_config.h) 
>
> ①打开（读卡器U盘功能设备功能）：
>
> #define TCFG_UDISK_ENABLE                   1



### 4、USB主机UVC功能（设备USB口接上UVC摄像头）

**（1）wifi_camera工程**

> 配置文件： [app_config.h](..\..\..\wifi_camera\include\app_config.h) 
>
> ①打开宏：#define CONFIG_UVC_VIDEO2_ENABLE：
>
> ②查看USB摄像头方法：
>
> A：使用Dvruning2的APP可以直接连接wifi热点出图。
>
> B：使用wifi_camera下demo_ui，则开启UI，打开UI宏：codeblock的工程编译选项(build options)的#define选项加上CONFIG_UI_ENABLE，可以显示到屏幕（或者linux环境下的makefile加上UI宏CONFIG_UI_ENABLE）。
>
> C： 在[app_main.c](..\..\..\wifi_camera\app_main.c) 开启插卡录像功能，如下代码块。插USB UVC摄像头后，再插SD卡就可以开启录像功能，在录像一个文件结束（默认3分钟）就可以拔出SD卡来插卡查看UVC录像视频。
>
> ```c
> void app_main()
> {
>     struct intent it;
> 
>     puts("------------- wifi_camera app main-------------\n");
> 
>     init_intent(&it);
>     //it.name	= "net_video_rec";//APP状态机在：net_video_rec.c，APP出图功能
>     it.name	= "video_rec";//APP状态机在：video_rec.c，插卡上电录像，可APP出图功能
>     it.action = ACTION_VIDEO_REC_MAIN;
>     start_app(&it);
> }
> ```
>
> 

