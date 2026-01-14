# PLNK(数字麦)功能配置和使用说明

> 本说明介绍了PLNK数字麦的配置和使用说明，通过使用demo_audio工程进行数字麦采样录音作为例子。

---

## 适用平台

> 本工程适用以下芯片类型：
>
> 1. AC79系列芯片：AC790N、AC791N

> 杰理芯片和评估板的更多信息可在以下地址获取：
> [链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)

## 工程配置说明

> 1.硬件连接：将数字麦引出的MIC_VDD(电源3.3V)、GND(地)、DAT(数据通道)和CLK(时钟线)进行连接。两个数字麦可共用DAT数据线和CLK时钟线
>
> ![schematic](PLNK(数字麦) readme.assets/schematic.PNG)
>
> 2..board.c：PLNK使用配置 ：
>
> ```
> //name 输入通道选择
> #define PLNK_MIC_MASK					(BIT(0) | BIT(1))
> #define PLNK_CH_MIC_L					BIT(0)	//两个MIC共用DAT0也使用该宏
> #define PLNK_CH_MIC_R					BIT(1)  //两个MIC共用DAT1也使用该宏
> #define PLNK_CH_MIC_DOUBLE				(BIT(0) | BIT(1))
> 
> plnk0_port_remap_cb()//重映射函数
> plnk0_port_unremap_cb()//解除重映射
> //重映射PDM DAT-PA2   PDM CLK-PA1    重映射的DAT脚和CLK时钟脚可选择
> 
> JL_IOMAP->CON3 |= BIT(18);//PLNK0 SCLK0 输出使能        若使用PLNK1数字麦1,JL_IOMAP->CON3 |= BIT(19) 
> JL_IOMAP->CON3 &= ~BIT(18);//PLNK0 SCLK0 输出失能       若使用PLNK1数字麦1，JL_IOMAP->CON3 &= ~BIT(19);
> 
> .hw_channel = PLNK_CH_MIC_L, //使用DAT0输入             若使用DAT1，.hw_channel = PLNK_CH_MIC_R,
> .sample_edge = 0,   //在CLK的下降沿采样左MIC      .sample_edge = 1, //在CLK的上升沿采样左MIC
> .share_data_io = 1, //两个数字MIC共用一个DAT脚  1为共用一个DAT脚，0为不共用    
> 
> gpio_plnk_rx_input(IO_PORTA_02, 0, 0);//plnk数字DAT0输入
> gpio_plnk_rx_input(IO_PORTA_02, 0, 1);//plnk数字DAT1输入
> ```
>
> ![borad_config_remap](PLNK(数字麦) readme.assets/borad_config_remap.PNG)
>
> ![board_plnk_audio](PLNK(数字麦) readme.assets/board_plnk_audio.PNG)
>
> 3.app_config：配置PLNK0作为录音的输入源
>
> ```
> #define CONFIG_AUDIO_ENC_SAMPLE_SOURCE      AUDIO_ENC_SAMPLE_SOURCE_PLNK0 //录音输入源选择
> ```
>
> ![app_config_audio](PLNK(数字麦) readme.assets/app_config_audio.PNG)
---

## 操作说明：

>1.编译工程，烧录镜像，复位启动
>
>2.调到录音模式，硬件连接好数字麦,即可听到由数字麦采样的录音通过dac输出，也可以录到SD卡上保存

> JIELI SDK的编译、烧写等操作方式的说明可在以下文档获取：
> [文档](/doc/stuff/usb%20updater.pdf)

---

## 常见问题

> * 1.连接数字麦无录音dac播出？
>
> * 答：(1)查看设置映射的DAT脚和CLK时钟脚是否和硬件连接相同
>
>   ​        (2)映射的DAT和CLK的IO芯片是否引出
>
>   ​		(3)如有特殊硬件需求情况下，share_data_io = 0,不共用DAT脚时，注意使用到的通道.hw_channel 配置，可为使用DAT0和DAT1两个通道，.hw_channel = PLNK_CH_MIC_L|PLNK_CH_MIC_R 

## 参考文档

> * N/A