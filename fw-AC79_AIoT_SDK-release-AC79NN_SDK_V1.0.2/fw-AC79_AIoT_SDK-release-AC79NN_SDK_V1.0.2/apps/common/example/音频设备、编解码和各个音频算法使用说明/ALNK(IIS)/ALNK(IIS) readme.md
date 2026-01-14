# ALNK(IIS)功能配置和使用说明

> 本说明介绍了分别使用两块开发板进行主从机的IIS输入/输出。
>
> 测试用例中一块开发板用demo_edr工程进行蓝牙播歌IIS输出，另一块开发板用demo_audio工程经过IIS输入后输出到DAC播放。

---

## 适用平台

> 本工程适用以下芯片类型：
>
> 1. AC79系列芯片：AC790N、AC791N

> 杰理芯片和评估板的更多信息可在以下地址获取：
> [链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)

## 工程配置说明

> 1.使用前需查看原理图使用到ALNK(IIS)信号传输使用到的IO是否引出来选择IO组。(如AC7911B的PC3未引出) 
>
> | IIS_IO    | MCLK | SCLK | LRCK | CH0  | CH1  | CH2  | CH3  |
>| --------- | ---- | ---- | ---- | ---- | ---- | ---- | ---- |
> | IIS_PORTA | PA4  | PA5  | PA6  | PA7  | PA8  | PA9  | PA10 |
> | IIS_PORTC | PC0  | PC1  | PC2  | PC3  | PC4  | PC5  | PC6  |
> | IIS_PORTG | PG8  | PG9  | PG10 | PG11 | PG12 | PG13 | PG14 |
> 
> MCLK为音频接口主时钟。SCLK，叫做串行时钟，也叫位时钟（BCLK），对应数字音频的每一位数据。SCLK的频率=2×采样频率×64或者32。LRCK，帧时钟，也叫作WS，负责左右声道切换。如为高电平表示右声道声音数据，为低电平表示传输左声道声音数据。其频率等于采样频率。
>
> CH0~CH3为四个独立通道，都可设为输入或输出，但同一模块的同一通道使用时不能同时设置为输出和输入。四条通道共用MCLK/LRCK/SCLK信号。
>
> 2.测试用例配置：
> 
>（1）开发板烧录demo_edr工程，测试使用的蓝牙播歌音源输出配置为iis0：
> 
>- 在board.c添加：设置通道BIT(0)为输出
> 
>![edr_board_config](ALNK(IIS) readme.assets/edr_board_config.png)
> 
>- 在bt_decode.c修改earphone_a2dp_audio_codec_open()函数中dec解码输出方式，将蓝牙播歌输出到“iis0”。
> 
>![bt_decode_dec](ALNK(IIS) readme.assets/bt_decode_dec.png)
> 
>（2）开发板烧录demo_audio工程，测试IIS输入后输出到DAC播放：
> 
>- board.c:设置对于通道BIT(0)为输入
> 
>![audio_board_config](ALNK(IIS) readme.assets/audio_board_config.png)
> 
>- app_config.h:设置录音采样源为iis0。设置录音采样率为44100(蓝牙播歌输出采样率)，保持输入输出采样率一致。录音通道数改为2（iis为双声道）。
> 
>![audio_app_config](ALNK(IIS) readme.assets/audio_app_config.png)
> 
>3.主从机模式
> 
>SCLK和LRCK由主机提供给从机。MCLK由主机提供给从机，当从机有晶振时可不需由主机提供MCLK。
> 
>​                                 

---

## 操作说明：

>1.进行IIS_IO硬件连接
>
>2.编译工程，烧录镜像，复位启动
>
>3.系统启动后，先将一块开发板使用蓝牙进行播歌，然后将另一块开发板调到录音模式，即可听到播歌

> JIELI SDK的编译、烧写等操作方式的说明可在以下文档获取：
> [文档](/doc/stuff/usb%20updater.pdf)

---

## 常见问题

> * 1.怎么查看是否正常输出iis信号？
> * 答：可通过逻辑分析仪查看io口信号是否输出正常
> * 2.IIS输入信号不正常？
> * 答：查看原理图使用到的IO是否引出可使用，查看两个board.c和demo_audio的app_config是否正确配置，尤其是录音通道数和录音采样率的设置是否正确。

## 参考文档

> * N/A