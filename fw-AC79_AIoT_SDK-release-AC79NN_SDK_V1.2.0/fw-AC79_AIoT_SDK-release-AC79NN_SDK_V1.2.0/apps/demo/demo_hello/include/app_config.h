#ifndef APP_CONFIG_H
#define APP_CONFIG_H



#define __FLASH_SIZE__    (4 * 1024 * 1024)
#define __SDRAM_SIZE__    (2 * 1024 * 1024)


//*********************************************************************************//
//                                  电源配置                                     //
//*********************************************************************************//
#define TCFG_LOWPOWER_BTOSC_DISABLE			0
#define TCFG_LOWPOWER_LOWPOWER_SEL			0//(RF_SLEEP_EN|RF_FORCE_SYS_SLEEP_EN|SYS_SLEEP_EN) //该宏在睡眠低功耗才用到，此处设置为0
#define TCFG_LOWPOWER_VDDIOM_LEVEL			VDDIOM_VOL_32V //正常工作的内部vddio电压值，一般使用外部3.3V，内部设置需比外部3.3V小

#ifdef CONFIG_RTC_ENABLE
#define TCFG_LOWPOWER_VDDIOW_LEVEL			VDDIOW_VOL_32V       //弱VDDIO电压档位。RTCVDD电压低于3.2V可能不走时，因此RTCVDD由IOVDD供电时，VDDIOW应设置为VDDIOW_VOL_32V档
#else
#define TCFG_LOWPOWER_VDDIOW_LEVEL			VDDIOW_VOL_21V       //弱VDDIO电压档位
#endif

#define VDC14_VOL_SEL_LEVEL					VDC14_VOL_SEL_140V //内部的1.4V默认1.4V
#define SYSVDD_VOL_SEL_LEVEL				SYSVDD_VOL_SEL_126V //系统内核电压，默认1.26V


#define CONFIG_CXX_SUPPORT //使能C++支持

#define CONFIG_DEBUG_ENABLE                     /* 打印开关 */

//#define SDTAP_DEBUG

#if !defined CONFIG_DEBUG_ENABLE || defined CONFIG_LIB_DEBUG_DISABLE
#define LIB_DEBUG    0
#else
#define LIB_DEBUG    1
#endif
#define CONFIG_DEBUG_LIB(x)         (x & LIB_DEBUG)

#endif

