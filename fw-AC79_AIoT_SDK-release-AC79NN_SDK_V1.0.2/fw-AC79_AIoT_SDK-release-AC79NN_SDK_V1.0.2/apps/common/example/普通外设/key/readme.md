# KEY示例工程说明

> 本工程展示了iokey和adkey的使用方法。
>
> 1.演示读取按键方法。
>
> 2.AD值的硬件计算原理
>
> 3.用户使用不同按键数量说明

---

## 适用平台

> 本工程适用以下芯片类型：
> 1. AC79系列芯片：AC790N、AC791N

> 杰理芯片和评估板的更多信息可在以下地址获取：[链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)

## 工程代码配置说明

> 1.在SDK选择[demo_hello](../../../../apps/demo_hello/board)或者其他demo的主工程文件或者主工程Makefile
>
> 2.在demo_hello工程中添加master分支的[common/key](../../../../apps/common/key)中adkey.c、iokey.c和key_driver.c文件，同时在工程中添加[cpu/key](../../../../cpu/wl80/key)中adc_api.c文件
>
> 3.添加本示例main.c
>
> 4.使用adkey时需到app_config.h中#define  TCFG_ADKEY_ENABLE  1 
>
> 5.使用iokey时需到app_config.h中#define TCFG_IOKEY_ENABLE	1
>
> 6.在app_main.c文件中包含头文件#include "event/key_event.h"和#include "event/event.h"

---

### 板级配置：

> 在board.c中添加头文件
>
> ```
> #if (TCFG_IOKEY_ENABLE || TCFG_IRKEY_ENABLE || TCFG_RDEC_KEY_ENABLE || TCFG_ADKEY_ENABLE)
> #include "key/iokey.h"
> #include "key/adkey.h"
> #include "key/irkey.h"
> #include "key/rdec_key.h"
> #include "key/key_driver.h"
> #include "event/key_event.h"
> #endif
> ```
>
> adkey相关配置：
>
> ```
> #if TCFG_ADKEY_ENABLE
> 
> #define ADKEY_UPLOAD_R  22
> #define ADC_VDDIO (0x3FF)
> #define ADC_09   (0x3FF * 220 / (220 + ADKEY_UPLOAD_R))
> #define ADC_08   (0x3FF * 120 / (120 + ADKEY_UPLOAD_R))
> #define ADC_07   (0x3FF * 51  / (51  + ADKEY_UPLOAD_R))
> #define ADC_06   (0x3FF * 33  / (33  + ADKEY_UPLOAD_R))
> #define ADC_05   (0x3FF * 22  / (22  + ADKEY_UPLOAD_R))
> #define ADC_04   (0x3FF * 15  / (15  + ADKEY_UPLOAD_R))
> #define ADC_03   (0x3FF * 10  / (10  + ADKEY_UPLOAD_R))
> #define ADC_02   (0x3FF * 62  / (62  + ADKEY_UPLOAD_R * 10))
> #define ADC_01   (0x3FF *  3  / ( 3  + ADKEY_UPLOAD_R))
> #define ADC_00   (0)
> 
> #define ADKEY_V_9      	((ADC_09 + ADC_VDDIO)/2)
> #define ADKEY_V_8 		((ADC_08 + ADC_09)/2)
> #define ADKEY_V_7 		((ADC_07 + ADC_08)/2)
> #define ADKEY_V_6 		((ADC_06 + ADC_07)/2)
> #define ADKEY_V_5 		((ADC_05 + ADC_06)/2)
> #define ADKEY_V_4 		((ADC_04 + ADC_05)/2)
> #define ADKEY_V_3 		((ADC_03 + ADC_04)/2)
> #define ADKEY_V_2 		((ADC_02 + ADC_03)/2)
> #define ADKEY_V_1 		((ADC_01 + ADC_02)/2)
> #define ADKEY_V_0 		((ADC_00 + ADC_01)/2)
> 
> const struct adkey_platform_data adkey_data = {//按键不需要添加到设备列表
> .enable     = 1,								 //使能adkey
> .adkey_pin  = IO_PORTB_01,		//PB1引脚连接ADkey引脚
> .extern_up_en = 1,						 //允许外部上拉
> .ad_channel = 3,							 //通道3,通道3和PB1引脚是一对固定的关系。如需修改可参考常见问题
> .ad_value = {
>   ADKEY_V_0,
>   ADKEY_V_1,
>   ADKEY_V_2,
>   ADKEY_V_3,
>   ADKEY_V_4,
>   ADKEY_V_5,
>   ADKEY_V_6,
>   ADKEY_V_7,
>   ADKEY_V_8,
>   ADKEY_V_9,
> },
> .key_value = {
>   KEY_MENU, 			            /*0*/
>   KEY_PHOTO, 		       	 /*1*/
>   KEY_ENC,  				       /*2*/
>   KEY_F1,   				       /*3*/
>   KEY_MODE, 			    /*4*/
>   KEY_CANCLE,			/*5*/
>   KEY_OK,   			    /*6*/
>   KEY_DOWN,          /*7*/
>   KEY_UP,               /*8*/
> 		NO_KEY,
> },
> };
> 
> #endif
> ```
>
> iokey相关配置：
>
> ```
> #if TCFG_IOKEY_ENABLE
> 
> static const struct iokey_port iokey_list[] = {
> {
>   .connect_way = DOUBLE_PORT_TO_IO,         //IO按键的连接方式，可以单按键也可以双按键
>   //.key_type.one_io.port = IO_PORTB_01,   	 //IO按键对应的引脚
>   .key_type.two_io.in_port=IO_PORTA_01,		
>   .key_type.two_io.out_port=IO_PORTA_00,
>   .key_value = KEY_MENU,            						//按键值
> }
> };
> const struct iokey_platform_data iokey_data = {//按键不需要添加到设备列表
> .enable = 1,                             				 //是否使能IO按键
> .num = ARRAY_SIZE(iokey_list),            //IO按键的个数
> .port = iokey_list,                      			   //IO按键参数表
> };
> 
> #endif
> 
> void board_init()
> {
> #if TCFG_ADKEY_ENABLE
>  adc_init();
> #endif
> #if (TCFG_IOKEY_ENABLE || TCFG_IRKEY_ENABLE || TCFG_RDEC_KEY_ENABLE || TCFG_ADKEY_ENABLE)
> 	key_driver_init();
> #endif
> 
> }
> ```

### 操作说明：

> 编译工程，烧录镜像，复位启动

> JIELI SDK的编译、烧写等操作方式的说明可在以下文档获取：[文档](../../../../doc/stuff/usb updater.pdf)

### 代码流程

> 1. c_main()入口：(以下关于事件配置可以参考example中event的说明[readme.md](../../../../../apps/common/example/event/readme.md) )
>
>     ```
>     static int c_main(void)
>     {
>         os_task_create(app_event_test_task, NULL, 10, 1000, 128, "app_event_test_task");
>     	return 0;
>     }
>     late_initcall(c_main);
>     
>     ```
>     
>     ```
>     #include "app_config.h"
>     #include "system/includes.h"
>     #include "event/device_event.h"
>     #include "event/key_event.h"
>     #include "key/key_driver.h"
>     
>     /* 一个事件递交到当前APP处理,实际上最常用*/
>     static int app1_event_handler(struct application *app, struct sys_event *event)
>     {
>         if(event->type == SYS_KEY_EVENT)
>             {
>                 struct key_event *key_e = (struct key_event *)event->payload;
>                 printf("app1_event_handler[in %s task]  SYS_KEY_EVENT %d \r\n",os_current_task(),key_e->value);
>             }
>     
>         return true;    //如果返回true则不再调用 app_default_event_handler
>         //return false;
>     }
>     
>     
>     static void app_event_test_task(void*p)
>     {
>         struct intent it;
>         init_intent(&it);  //app_core_init初始化完成后才可以测试
>     
>         it.name = "APP1";
>         it.action = ACTION_DO_NOTHING; //事件发生后不做动作
>         start_app(&it); //启动app
>     
>         key_event_enable(); //针对按键事件,需要先使能开关
>     
>         while(1)
>         {
>             os_time_dly(200);
>             puts("\n\n\n\n");
>     
>             struct key_event key = {0};
>             key.action = KEY_EVENT_CLICK; //按键触发方式为单击按键
>            // key.value = NO_KEY;
>             key.type = KEY_EVENT_USER;    //按键类型来自用户
>             key_event_notify(KEY_EVENT_FROM_USER, &key);
>     
>             os_time_dly(200);
>             puts("\n\n\n\n");
>     
>         }
>     }
>     
>     static int app1_state_machine(struct application *app, enum app_state state,struct intent *it)
>     {
>          return 0;
>     }
>     static const struct application_operation app1_ops = {
>         .state_machine  = app1_state_machine,
>         .event_handler  = app1_event_handler,
>     };
>     REGISTER_APPLICATION(app1) = {
>         .name   = "APP1",
>         .ops    = &app1_ops,
>         .state  = APP_STA_DESTROY,
>     };
>     
>     
>     //注册一个静态事件句柄
>     SYS_EVENT_STATIC_HANDLER_REGISTER(static_event_handler) = {
>         .event_type     = SYS_ALL_EVENT,
>     };
>     
>     ```
>     
>     
>     
---

### AD值的硬件计算原理

> AD值的硬件计算原理（board.c）:
>
> ```
> /*-------------ADKEY GROUP 1----------------*/
> #define ADKEY_UPLOAD_R  22
> #define ADC_VDDIO (0x3FF)
> ```
>
> 说明：ADKEY_UPLOAD_R为22K上拉电阻，ADC_VDDIO为上拉时即电源电压3.3v所采集到的ADC的值，因为是10bitADC，所以是0x3ff(1023)
>
> ```
> #define ADC_09   (0x3FF * 220 / (220 + ADKEY_UPLOAD_R))
> #define ADC_08   (0x3FF * 120 / (120 + ADKEY_UPLOAD_R))
> #define ADC_07   (0x3FF * 51  / (51  + ADKEY_UPLOAD_R))
> #define ADC_06   (0x3FF * 33  / (33  + ADKEY_UPLOAD_R))
> #define ADC_05   (0x3FF * 22  / (22  + ADKEY_UPLOAD_R))
> #define ADC_04   (0x3FF * 15  / (15  + ADKEY_UPLOAD_R))
> #define ADC_03   (0x3FF * 10  / (10  + ADKEY_UPLOAD_R))
> #define ADC_02   (0x3FF * 62  / (62  + ADKEY_UPLOAD_R * 10))
> #define ADC_01   (0x3FF *  3  / ( 3  + ADKEY_UPLOAD_R))
> #define ADC_00   (0)
> ```
>
> 说明：上面的宏定义是AD按键不同电阻根据电阻分压原理计算出来的电压值转换后的ADC值
>
> ```
> #define ADKEY_V_9         ((ADC_09 + ADC_VDDIO)/2)
> #define ADKEY_V_8 		((ADC_08 + ADC_09)/2)
> #define ADKEY_V_7 		((ADC_07 + ADC_08)/2)
> #define ADKEY_V_6 		((ADC_06 + ADC_07)/2)
> #define ADKEY_V_5 		((ADC_05 + ADC_06)/2)
> #define ADKEY_V_4 		((ADC_04 + ADC_05)/2)
> #define ADKEY_V_3 		((ADC_03 + ADC_04)/2)
> #define ADKEY_V_2 		((ADC_02 + ADC_03)/2)
> #define ADKEY_V_1 		((ADC_01 + ADC_02)/2)
> #define ADKEY_V_0 		((ADC_00 + ADC_01)/2)
> ```
>
> 说明：上面为ADC取中间值的宏定义，这个也好理解，假如KEY0的键值ADC对应的是920，KEY1的是846，那么就取两个的中间值，当大于这个中间值我们就可以认为是KEY0，小于这个就认为是KEY1

---

### 用户使用不同按键数量说明

> ```
> #define ADC_09   (0x3FF)
> #define ADC_08   (0x3FF)
> #define ADC_07   (0x3FF)
> #define ADC_06   (0x3FF)
> #define ADC_05   (0x3FF)
> #define ADC_04   (0x3FF * 15  / (15  + ADKEY_UPLOAD_R))
> #define ADC_03   (0x3FF * 10  / (10  + ADKEY_UPLOAD_R))
> #define ADC_02   (0x3FF * 62  / (62  + ADKEY_UPLOAD_R * 10))
> #define ADC_01   (0x3FF *  3  / ( 3  + ADKEY_UPLOAD_R))
> #define ADC_00   (0)
> ```
>
> 说明：当用户不需要使用所有按键时，可以将不使用的按键ADC值均置为0x3FF，例如上述代码中的ADC05~ADC09



## 常见问题



> * （1）如何修改ad_key连接的引脚？
>
>   答：参考一下配置，ad_channel 和 adkey_pin  是一一对应的。PB01脚对应的是通道3。
>
>   ```
>   #define AD_CH_PA07    (0x0)
>   #define AD_CH_PA08    (0x1)
>   #define AD_CH_PA10    (0x2)
>   #define AD_CH_PB01    (0x3)
>   #define AD_CH_PB06    (0x4)
>   #define AD_CH_PB07    (0x5)
>   #define AD_CH_PC00    (0x6)
>   #define AD_CH_PC01    (0x7)
>   #define AD_CH_PC09    (0x8)
>   #define AD_CH_PC10    (0x9)
>   #define AD_CH_PH00    (0xa)
>   #define AD_CH_PH03    (0xb)
>   #define AD_CH_DM      (0xc)
>   #define AD_CH_DP      (0xd)
>   #define AD_CH_RTC     (0xe)
>   #define AD_CH_PMU     (0xf)
>   #define AD_CH_SYSPLL  (0xf)
>   #define AD_CH_AUDIO   (0xf)
>   #define AD_CH_WIFI    (0xf)
>   #define AD_CH_CTMU    (0xf)
>   ```
>
> - （2）io_key双按键使用说明
>
>   当我们配置的key_type.two_io.in_port的引脚接触到GND即触发，key_type.two_io.out_port配置的引脚输出低电平。
>
> - (3)io_key的key_value值与ad_key的值表是不一样的，io_key的key_value值可以查看一下定义
>
> - ```
>   #define     KEY_POWER           0
>   #define     KEY_PREV            1
>   #define     KEY_NEXT            2
>   #define     KEY_PLAY            3
>   #define     KEY_VOLUME_DEC      4
>   #define     KEY_VOLUME_INC      5
>   #define     KEY_MODE            6
>   #define     KEY_MENU            7
>   #define     KEY_ENC             8
>   #define     KEY_PHONE           9
>   #define     KEY_PHOTO           10
>     
>   #define 	KEY_F1 				11
>   #define     KEY_OK              12
>   #define     KEY_CANCLE          13
>   #define     KEY_LEFT            14
>   #define     KEY_UP              15
>   #define     KEY_RIGHT           16
>   #define     KEY_DOWN            17
>   #define     KEY_MUTE            18
>   #define     KEY_COLLECT         19
>   #define     KEY_LOCAL           20
>     
>   #define 	KEY_0 				30
>   #define 	KEY_1 				31
>   #define 	KEY_2 				32
>   #define 	KEY_3 				33
>   #define 	KEY_4 				34
>   #define 	KEY_5 				35
>   #define 	KEY_6 				36
>   #define 	KEY_7 				37
>   #define 	KEY_8 				38
>   #define 	KEY_9 				39
>     
>   #define     NO_KEY              255
>   ```

## 参考文档

> * N/A

