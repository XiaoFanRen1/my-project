# RTC系统time时间使用示例工程说明

> 本工程展示了系统时间接口使用示例:
>
> 1. 设置和获取系统的RTC时间;
> 2. 获取系统NTP时间;
> 3. 获取系统运行时间;
> 4. 时间字符串格式化方法;
> 5. gettimeofday使用方法;
> 6. time_lapse使用方法;

---

## 适用平台

> 本工程适用以下芯片类型：
> 1. AC79系列芯片：AC790N、AC791N
>
> 杰理芯片和评估板的更多信息可在以下地址获取：[链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)

## 工程配置说明

> 如果不启用STA的WIFI功能，在SDK选择[demo_hello](../../../../apps/demo_hello/board)或者其他demo的主工程文件或者主工程Makefile, 然后添加本事例工程代码。
>
> 如果启动了STA模式的WIFI功能，需要获取网络时间，则需要把设备配置在STA模式,在SDK选择[demo_wifi](../../../../apps/demo_wifi/board)主工程文件或者主工程Makefile, 然后添加本事例工程代码。
>
> 1. board.c 添加board_init()进行RTC初始化和RTC设备
>
> ```c
> //设备列表 device_table 添加RTC设备，默认demo_hello和demo_wifi已经添加，例如demo_hello的board.c如下。
> REGISTER_DEVICES(device_table) = {
> 	{"uart2", &uart_dev_ops, (void *)&uart2_data },
> 	{"rtc", &rtc_dev_ops, NULL},//添加RTC设备
> };
> 
> //添加RTC初始化函数，系统上电会调用（有wifi功能的在board_init()函数检查有没有rtc_early_init()，没有就需要加上该函数），如demo_hello的board.c
> void board_init()
> {
>     //当有外部晶振传参为1，没有外部晶振传参为0
>     rtc_early_init(1);//传参为1则使用外部晶振RTC计时，为0则为：AC790N为内部软件定时器计时，AC791N为使用内部RC32K时钟(时间起始为SDK发布时间)，
> }
> 
> ```
>
> 2. 测试代码
>
> ```c
> #include "app_config.h"
> #include "system/includes.h"
> #include "device/device.h"
> #include <time.h>
> #include <sys/time.h>
> 
> #ifdef CONFIG_WIFI_ENABLE
> #include "wifi/wifi_connect.h"
> #endif
> 
> extern u32 timer_get_ms(void);
> extern u32 timer_get_sec(void);
> extern u32 time_lapse(u32 *handle, u32 time_out);//2^32/1000/60/60/24 后超时
> 
> //当应用层不定义该函数时，系统默认时间为SDK发布时间，当RTC设置时间小于SDK发布时间则设置无效
> void set_rtc_default_time(struct sys_time *t)
> {
>     t->year = 2021;
>     t->month = 6;
>     t->day = 1;
>     t->hour = 8;
>     t->min = 8;
>     t->sec = 8;
> }
> 
> static void time_print(void)//网络时间获取方法
> {
>     char time_str[64];
>     struct tm timeinfo;
>     time_t timestamp;
> 
>     timestamp = time(NULL) + 28800;
>     localtime_r(&timestamp, &timeinfo);
>     strftime(time_str, sizeof(time_str), "%Y-%m-%dT%H:%M:%S", &timeinfo);
>     printf("RTC TIME [%s]\n", time_str);
> 
>     printf("system boot run time [%d sec] [%d msec]\n", timer_get_sec(),timer_get_ms());
>     printf("jiffies run time [%d sec]\n", jiffies*10);
> 
>     struct timeval tv;
>     gettimeofday(&tv,NULL);
>     printf("gettimeofday [%d sec+%d usec]\n", tv.tv_sec, tv.tv_usec);
> 
>     static u32 time_lapse_hdl=0; //需要初始化为0,句柄在time_lapse使用过程中不可消亡
>     u32 to = time_lapse(&time_lapse_hdl,5000);  //指定时间后返回真,并且返回比上一次执行的时间经过了多少毫秒
>     if(to){
>         printf("time_lapse timeout %d msec\r\n",to);
>     }
> }
> void user_set_rtc_time(void)//用户设置RTC时间
> {
>     struct sys_time st;
>     void *fd = dev_open("rtc", NULL);
>     if (fd) {
>         st.year = 2020;
>         st.month = 2;
>         st.day = 2;
>         st.hour = 2;
>         st.min = 2;
>         st.sec = 2;
>         printf("user_set_rtc_time : %d-%d-%d,%d:%d:%d\n", st.year, st.month, st.day, st.hour, st.min, st.sec);
>         dev_ioctl(fd, IOCTL_SET_SYS_TIME, (u32)&st);
>         dev_close(fd);
>     }
> }
> void user_get_rtc_time(void)//用户获取RTC时间
> {
>     struct sys_time st;
>     void *fd = dev_open("rtc", NULL);
>     if (fd) {
>         dev_ioctl(fd, IOCTL_GET_SYS_TIME, (u32)&st);
>         dev_close(fd);
> 		printf("user_get_rtc_time : %d-%d-%d,%d:%d:%d\n", st.year, st.month, st.day, st.hour, st.min, st.sec);
>     }
> }
> 
> static void time_rtc_test_task(void *p)
> {
> 
> 	os_time_dly(100);
> 
> #ifdef CONFIG_WIFI_ENABLE //网络时间,当不需要网络时间则不需以下代码操作
>     while (wifi_get_sta_connect_state() != WIFI_STA_NETWORK_STACK_DHCP_SUCC) {
>         printf("Waitting STA Connected...\r\n");
>         //当网络连接成功前, 获取的是同步网络时间前的RTC时间
>         //当网络连接成功后并且连接远端NTP服务商成功, 执行time函数会获取网络时间同步本地RTC时间
>         time_print();
>         os_time_dly(100);
>     }
>     //联网成功后，系统自动把网络时间同步到系统RTC实际
>     while (1) {
>         time_print();  //当网络连接成功前, 获取的是同步网络时间前的RTC时间
>         os_time_dly(300);
>     }
> #else //本地RTC获取
>     user_get_rtc_time();
>     user_set_rtc_time();
>     while (1) {
>         os_time_dly(100);
>         user_get_rtc_time();
>     }
> #endif
> }
> 
> static int c_main(void)
> {
>     os_task_create(time_rtc_test_task, NULL, 10, 1000, 0, "time_rtc_test_task");
>     return 0;
> }
> late_initcall(c_main);
> 
> ```
>
> **2、注意事项**
>
> A）rtc_early_init()函数，当有外部晶振传参为1，没有外部晶振传参为0，传参为1则使用外部晶振RTC计时，为0则为：AC790N为内部软件定时器计时，AC791N为使用内部RC32K时钟(时间起始为SDK发布时间)。
>
> B）RTC内部系统有一个SDK发布时间作为默认时间：
> 当应用层重定义set_rtc_default_time原函数时（例如上述测试例子 的set_rtc_default_time函数定义），则设置默认时间为该函数设置的时间。
>
> 当没有定义set_rtc_default_time函数则是系统默认时间为SDK发布时间，RTC设置时间不能小于默认时间。
>
> C）外部晶振接芯片的PR0和PR1引脚，当在某些封装中PR口可能会和其他IO内绑在一块，此时在board_init()函数首先需要把其他IO关闭上下拉并设置输入模式，才能调用rtc_early_init初始化RTC。
>
> D）当芯片为AC790N，不外接晶振使用内部软件走时时，若需要在关机后重启后系统时间为上次关机时间则：不定义set_rtc_default_time函数，关机前设置一下系统时间即可。
## 操作说明

> 2. 编译工程，烧录镜像，复位启动
> 3. 系统启动后，可以通过串口软件看到示例的打印信息
>
> JIELI SDK的编译、烧写等操作方式的说明可在以下文档获取：[文档](../../../../doc/stuff/usb updater.pdf)

## 代码流程

> 1. c_main()入口：
>     A）创建time_test_task任务
>
> 2. time_test_task
>
>     A）如果WIFI在STA模式下,则等待连接上路由器后,调用time(NULL)函数获取网络NTP时间,否则就使用user_set_rtc_time函数配置时间
>
>     B ) 循环定时打印时间
## 常见问题

> * 外部晶振已经接了，但是RTC不走时.
>
>   答：
>
>   A）先查看外部晶振是否震荡起来，如果不震荡，确定晶振是否已坏，同时晶振引脚需要接电容到地。
>
>   B）检查芯片封装是否有内部IO和晶振的PR口内绑在一起，有则设置IO关闭上下拉并设置输入模式。

## 参考文档

> * N/A