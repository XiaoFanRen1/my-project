#   VDDIO_SYSVDD_DCDC14系统电压配置说明

> 本工程展示了VDDIO、SYSVDD、DCDC14系统电压配置。

---

## 适用平台

> 本工程适用以下芯片类型：
>
> 1. AC79系列芯片：AC790N、AC791N
>
> 杰理芯片和评估板的更多信息可在以下地址获取：[链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)
## 工程配置说明

在board_xxx.c选择电源参数power_param配置选择，具体在工程添加步骤可见 [soft_power_off](..\soft_power_off) 的电源配置参数，电压设置在app_config.h的电源低功耗配置宏定义。

> 1. board.c 的电源配置参数
>
> ```c
> //----------------------------------电源配置参数------------------------------------------//
> 
> /************************** LOW POWER config ****************************/
> static const struct low_power_param power_param = {
>     .config         = TCFG_LOWPOWER_LOWPOWER_SEL,
>     .btosc_disable  = TCFG_LOWPOWER_BTOSC_DISABLE,         //进入低功耗时BTOSC是否保持(不需要需修改)
>     .vddiom_lev     = TCFG_LOWPOWER_VDDIOM_LEVEL,          //强VDDIO等级,可选：2.2V  2.4V  2.6V  2.8V  3.0V  3.2V  3.4V  3.6V
>     .vddiow_lev     = TCFG_LOWPOWER_VDDIOW_LEVEL,          //弱VDDIO等级,可选：2.1V  2.4V  2.8V  3.2V
> 	.vdc14_dcdc 	= TRUE,	   							   //打开内部1.4VDCDC，关闭则用外部
>     .vdc14_lev		= VDC14_VOL_SEL_LEVEL, 				   //VDD1.4V配置
> 	.sysvdd_lev		= SYSVDD_VOL_SEL_LEVEL,				   //系统内核、sdram电压配置
>     .vlvd_value     = VLVD_SEL_25V,                        //低电压复位电压值
> };
> ```
>
> 2. app_config.h 的低功耗配置 参数
>
> ```c
> //*********************************************************************************//
> //                                  电源低功耗配置                                    //
> //*********************************************************************************//
> #define TCFG_LOWPOWER_BTOSC_DISABLE			0
> #define TCFG_LOWPOWER_LOWPOWER_SEL			0              //该宏在睡眠低功耗才用到，此处设置为0
> #define TCFG_LOWPOWER_VDDIOM_LEVEL			VDDIOM_VOL_32V //正常工作的内部vddio电压值，一般使用外部3.3V，内部设置需比外部3.3V小
> #define TCFG_LOWPOWER_VDDIOW_LEVEL			VDDIOW_VOL_21V //软关机或睡眠的内部vddio最低电压值
> #define VDC14_VOL_SEL_LEVEL					VDC14_VOL_SEL_140V //内部的1.4V默认1.4V
> #define SYSVDD_VOL_SEL_LEVEL				SYSVDD_VOL_SEL_126V //系统内核电压，默认1.26V
> 
> ```
>
> 3. struct low_power_param power_param结构体参数使用说明
>
> （1）.config：低功耗休眠配置项（默认0），包含：SYS_SLEEP_EN（系统休眠使能）、SYS_SLEEP_BY_IDLE（系统空闲休眠使能）、RF_SLEEP_EN（）、RF_FORCE_SYS_SLEEP_EN。
>
> （2）.btosc_disable：低功耗休眠保留参数（一般不用修改）。
>
> （3）vddiom_lev：设置芯片正常工作的IO口的电压（即VDDIO电压），一般实际PCB电路会把VDDIO接到外部的3.3V，这种情况时：当VDDIO设置电压档位小于3.3V则芯片使用外部的3.3V，当VDDIO设置电压档位大于3.3V时，芯片使用内部的VDDIO电压；VDDIO没有外接3.3V则使用内部的电压。
> 具体档位在对应的cpu路径下的power_interface.h文件，例如AC790:VDDIOM_VOL_22V - VDDIOM_VOL_36V； AC791:VDDIOM_VOL_28V - VDDIOM_VOL_35V。
>
> （4）vddiow_lev：设置芯片低功耗情况下工作的IO口的电压（即VDDIO电压），一般实际PCB电路会把VDDIO接到外部的3.3V，这种情况时：低功耗情况下会切断外部3.3连接VDDIO电路；VDDIO没有外接3.3V则使用内部的电压。
>
> 具体档位在对应的cpu路径下的power_interface.h文件，例如AC790:VDDIOM_VOL_22V - VDDIOM_VOL_36V； AC791:VDDIOM_VOL_28V - VDDIOM_VOL_35V。
>
> （5）vdc14_dcdc：使能内部DCDC-1.4V，芯片内部1.4V为wifi和蓝牙硬件的提供工作电压，分为：LDO-1.4V和DCDC-1.4V，当vdc14_dcdc 	= TRUE则开启内部DCDC-1.4V,一般需要打开（否则wifi初始化可能会瞬间消耗大电流导致死机发生）；当vdc14_dcdc 	= FALSE，则为关闭内部的DCDC1.4V，使用内部LDO-1.4V。
>
> （6）vdc14_lev：（.vdc14_dcdc = TRUE有效）DCDC-1.4V电压档位，具体档位在对应的cpu路径下的p33.h文件，例如AC790:VDC14_VOL_SEL_120V - VDC14_VOL_SEL_155V； AC791:VDC14_VOL_SEL_125V - VDC14_VOL_SEL_160V。一般默认使用1.4V以上。
>
> （7）sysvdd_lev：系统内核电压配置（sdram的电压使用改电压），具体档位在对应的cpu路径下的p33.h文件，例如AC790:SYSVDD_VOL_SEL_087V - SYSVDD_VOL_SEL_132V； AC791:SYSVDD_VOL_SEL_093V - SYSVDD_VOL_SEL_138V。一般默认使用1.2V以上。
>
> （8）vlvd_value：VBAT的低电压复位电压值（VBAT电压某时刻掉低于vlvd_value值则系统复位），VLVD_SEL_19V - VLVD_SEL_26V，具体详情p33.h含有VLVD_SEL_19V的联合体。
>
> 
>
> 注意事项：
>
> sysvdd_lev系统内核电压关系到sdram稳定性，在sdram速度120M以上，则系统内核电压最好需配置1.26V以上。
>
> 使用内部LDO-1.4V则wifi可能会在fi初始化可能会瞬间消耗大电流导致异常发生（wifi工作在AP模式时候会容易发生），因此强烈建议使用内部的DCDC-1.4V。
## 操作说明

> 1. 使用串口线连接打印
> 2. 编译工程，烧录镜像，复位启动
> 3. 系统启动后，查看系统打印。
>
> JIELI SDK的编译、烧写等操作方式的说明可在以下文档获取：[文档](../../../../doc/stuff/usb updater.pdf)
---

## 常见问题

> * N/A

## 参考文档

> * N/A