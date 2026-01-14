#   USB示例工程说明

> 本工程展示了USB模块接口使用方法。
>
> * USB做masstorge从机方法
> * USB做UAC从机方法
> * USB做UVC从机方法
> * USB做HID从机方法
> * USB做masstorge主机方法

---

## 适用平台

> 本工程适用以下芯片类型：
>
> 1. AC79系列芯片：AC790N、AC791N

> 杰理芯片和评估板的更多信息可在以下地址获取：[链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)

## 工程配置说明

USB基本工程配置(usb例子.patch包含全部修改)

> 1.工程中增加common/usb/*.c 增加所有文件
>
> 2.增加USB相关头文件路径
>
> ..\\..\common\usb\include
>
> ..\\..\common\usb
>
> ..\\..\common
>
> ..\\..\common\usb\host
>
> ..\\..\common\usb\device
>
> ..\\..\common\usb\include
>
> ..\\..\common\usb\include\host
>
> ..\\..\\..\include_lib\driver\device\usb\host
>
> ..\\..\\..\include_lib\driver\device\usb\device
>
> ..\\..\\..\include_lib\driver\device\usb
>
> 
>
> 4.app_config.h  USB基本配置
>
> * ```c
>  #define TCFG_PC_ENABLE  1
>   #define USB_PC_NO_APP_MODE                  2
>   #define TCFG_USB_SLAVE_ENABLE    1
>   #define TCFG_UDISK_ENABLE                   0
>   #define USB_DEVICE_CLASS_CONFIG    (MASSSTORAGE_CLASS)
>   
>   
>   
>   #include "usb_std_class_def.h"
>   #include "usb_common_def.h"
>   
>   ```
>   
> * app_mian.c增加USB任务名字
>
>    ```c
>    
>    /*任务列表 */
>    const struct task_info task_info_table[] = {
>        {"app_core",            15,     2048,	  1024 },
>        {"sys_event",           29,     512,	   0 },
>        {"systimer",            14,     256, 	   0 },
>        {"sys_timer",            9,     512,	  128 },
>    #if CPU_CORE_NUM > 1
>        {"#C0usb_msd",           1,      512,   128   },
>    #else
>        {"usb_msd",              1,      512,   128   },
>    #endif
>    
>        {0, 0, 0, 0, 0},
>    };
>   ```
>
>    
>
> * board.c增加USB配置
>
>    ```c
>    
>    
>    #if TCFG_USB_SLAVE_ENABLE || TCFG_USB_HOST_ENABLE
>    #include "otg.h"
>    #include "usb_host.h"
>    #include "usb_common_def.h"
>    #include "usb_storage.h"
>    #include "asm/uvc_device.h"
>    #endif
>    
>    
>    
>    
>    #if TCFG_USB_SLAVE_ENABLE || TCFG_USB_HOST_ENABLE
>    static const struct otg_dev_data otg_data = {
>        .usb_dev_en = 0x01,
>    #if TCFG_USB_SLAVE_ENABLE
>        .slave_online_cnt = 10,
>        .slave_offline_cnt = 10,
>    #endif
>    #if TCFG_USB_HOST_ENABLE
>        .host_online_cnt = 10,
>    	.host_offline_cnt = 10,
>    #endif
>    	.detect_mode = OTG_HOST_MODE | OTG_SLAVE_MODE | OTG_CHARGE_MODE,
>        .detect_time_interval = 30,
>    };
>    #endif
>    
>    
>    
>    
>    REGISTER_DEVICES(device_table) = {
>    #if TCFG_USB_SLAVE_ENABLE || TCFG_USB_HOST_ENABLE
>    	 { "otg", &usb_dev_ops, (void *)&otg_data},
>    #endif
>    
>    };
>    
>    ```
>    
>    

## 模块依赖

> * cpu.a 包含USB底层硬件收发库
> * dev.a 包含设备读写接口库
>
>   

---



### 操作说明：

> 1. USB做masstorge从机方法 打开四个宏即可
>
>    1.1 配置SD卡相关参考[SD例子](../sd/readme.md)
>
>    1.2 app_config.h 修改配置
>
>    ```C
>    #define TCFG_SD0_ENABLE  1   //使能sd卡
>    #define TCFG_PC_ENABLE  1     //使用从机模式
>    #define USB_PC_NO_APP_MODE                  2 //不使用app_core
>    #define USB_DEVICE_CLASS_CONFIG    (MASSSTORAGE_CLASS)   //msd
>
>    ```
>
> 2. USB做UAC从机方法 
>
>    2.1 配置AUDIO工程相关参考demo_music工程[demo_music](../play_music/sd_music/readme.md)
>
>    2.2 app_config.h 修改宏
>
>    ```c
>    #define USB_DEVICE_CLASS_CONFIG    (AUDIO_CLASS)   //audio
>
>
>    #define CONFIG_AUDIO_ENABLE       //使能audio_server
>    #define CONFIG_PCM_ENC_ENABLE	   
>    #define CONFIG_PCM_DEC_ENABLE
>
>    #define CONFIG_AUDIO_ENC_SAMPLE_SOURCE      AUDIO_ENC_SAMPLE_SOURCE_MIC
>
>    ```
> 
>    2.3 增加uac相关库
> 
>    | 文件            | 用途                                  |
>    | --------------- | ------------------------------------- |
>    | audio_config.c  | 用到了相关的api函数(get_ps_cal_api)   |
>    | usb_audio_api.c | usb和audio对接api函数                 |
>    | audio_server.a  | 音频编解码库                          |
>   | lib_usb_syn.a   | 用于变采样 专用于usb 不能用于其他地方 |
> 
>     2.4 初始化dac相关硬件**(重要)**
> 
> ​```c
> void board_early_init()
> {
>     extern void dac_early_init(u8 trim_en, u8 pwr_sel, u32 dly_msecs);
>     dac_early_init(1, 1, 1000);
>     devices_init();
> }
> 
>    ```
>
> ​		2.5 板级增加相关代码
>
> ```c
> 
>     {"uac_sync",            20,      512,   0     },
>     {"uac_play",             7,      768,   0     },
>     {"uac_record",           7,      768,   32    },
> ```
>
> 3. USB做UVC从机方法 （支持DVP摄像头数据到电脑）
>
>    3.1 **配置VIDEO工程相关参考demo_video工程**
>
>    ```c
>    //*********************************************************************************//
>    //                                  USB配置                                        //             USB查看软件 请在sdk_tools文件夹中找到AMCapSetup.exe
>    //*********************************************************************************//
>    //1.主机UVC模式
>    /*#define CONFIG_UVC_VIDEO2_ENABLE*/
>    #ifdef CONFIG_UVC_VIDEO2_ENABLE /*打开主机模式*/
>    #define TCFG_HOST_UVC_ENABLE                  0   //打开USB主机UVC
>    #define TCFG_LCD_USB_SHOW_COLLEAGUE           0   //UVC显示屏 和 wifi出图同时显示
>    #else /*打开从机模式*/
>    #define TCFG_USB_SLAVE_ENABLE                 1   //打开usb从机功能
>    #endif
>    
>    //2.主机读取U盘功能
>    #define TCFG_UDISK_ENABLE     				 0	//打开读写U盘功能
>    //3.从机连接PC模式下：电脑盘符功能、UVC功能
>    #define TCFG_PC_ENABLE						1    //打开usb从机功能(打开连接PC电脑模式)
>    #if TCFG_PC_ENABLE
>    #define USB_PC_NO_APP_MODE					2   //不用APP状态机接收消息
>    #define USB_MALLOC_ENABLE                     1   //buff采用申请方式
>    #define USB_DEVICE_CLASS_CONFIG (UVC_CLASS)     //从机UVC电脑查看功能选择
>    //`#define USB_DEVICE_CLASS_CONFIG (MASSSTORAGE_CLASS) //从机电脑盘符功能选择
>    #if ((USB_DEVICE_CLASS_CONFIG & UVC_CLASS) == UVC_CLASS)
>    #define CONFIG_USR_VIDEO_ENABLE
>    #endif
>    #endif
>    


>
>   	 3.2 增加文件
>
> | 文件              | 用途                               |
>| ----------------- | ---------------------------------- |
> | stream_protocol.c | 实时流中间层                       |
>| user_video_rec.c  | 打开实时流api                      |
> | video_rt_usr.c    | 封装给stream_protocol              |
>| gc0308.c          | 摄像头驱动，根据实际摄像头选择驱动 |
> 
>
> 
>   3.4 修改app_config.h
> 
>   ```c
>    
>   #define USB_DEVICE_CLASS_CONFIG    (UVC_CLASS)
>    
>   ```
> 
> 4. USB做HID从机方法   
>    
>    4.1 增加hid_keyboard.c
>    
>    4.2 修改宏#define USB_DEVICE_CLASS_CONFIG    (HID_CLASS)
>    
> 5. USB做主机U盘
>
>    5.1 在板级增加配置
>       { "udisk", &mass_storage_ops, NULL },
> 
>    5.2 打开宏
> 
> 
>   ```c
>     #define TCFG_UDISK_ENABLE                   1
>   ```



---

## 常见问题

> * N/A

## 参考文档

> * N/A