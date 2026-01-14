#ifndef __ALL_INCLUDE_H__
#define __ALL_INCLUDE_H__

//#include <string.h>
#include "typedef.h"
#include "common.h"
#include "uart.h"
#include "printf.h"

/*FPGA调试*/
//#define FPGA_MODE_DEBUG

/*串口打印控制器IO口占据*/
#define UART_ENABLE
//br23 用dp,br30yong usbdp
#define UART_DEBUG_IO     "PA00"
#define UART_DEBUG_BAUD   1000000
/*打印调试信息*/
#ifdef UART_DEBUG
#if defined(CONFIG_CPU_BR22) || defined(CONFIG_CPU_BR26)
#define UART_API_USING_MASKROM_EN 	0
#else
#define UART_API_USING_MASKROM_EN 	0
#endif

#if defined(CONFIG_CPU_BR23)
#define BZ_FLASH_ENABLE             1
#endif

#define DEG_INFO
//#define MUST_INFO
//#define UART_BT_LIB_DEG
//#define SD_DEBUG

//#define  RCSP_DEBUG_PRINT_SPPDATA  //打开SPP底层发送接收数据打印
//#define  RCSP_DEBUG_PRINT_SENDTIME //打开SPP底层发送接口耗时打印
//#define  RCSP_DEBUG                //打开模块调试流程打印
//#define  EFUSE_DEBUG               //打开EFUSE升级打印
#endif/*UART_DEBUG*/

#ifdef  DEG_INFO
// #define deg_putchar     putchar
#define deg_putchar(...)
// #define deg_puts        puts
#define deg_puts(...)
#define deg_printf      printf
#define deg_printf_buf  printf_buf
// #define deg_put_u32hex  put_u32hex
#define deg_put_u32hex(...)
// #define deg_put_u8hex  	put_u8hex
#define deg_put_u8hex(...)
#else
#define deg_putchar(...)
#define deg_puts(...)
#define deg_printf(...)
#define deg_printf_buf(...)
#define deg_put_u32hex(...)
#define deg_put_u8hex(...)
#endif/*DEG_INFO*/

#ifdef  MUST_INFO
#define must_puts        puts
#else
#define must_puts(...)
#endif/*MUST_INFO*/

extern void uart_init_son(u32 baud);
// #ifdef UART_DEBUG
// #define	uart_init		uart_init_son
// #else
// #define	uart_init(...)
// #endif

/*蓝牙模块打印*/
#ifdef UART_BT_LIB_DEG
#define bt_puts                 puts
#define bt_put_u32hex           put_u32hex
#define bt_printf               printf
#define bt_put_u16hex           put_u16hex
#define bt_put_u8hex            put_u8hex
#define bt_printf_buf           printf_buf
#define bt_put_buf              printf_buf
#define bt_putchar              putchar


#else
#define bt_puts(...)
#define bt_put_u32hex(...)
#define bt_printf(...)

#define bt_put_u16hex(...)
#define bt_put_u8hex(...)
#define bt_printf_buf(...)
#define bt_put_buf(...)
#define bt_putchar(...)
#endif

/*SD模块打印*/
#ifdef SD_DEBUG
#define sd_puts             puts
#define sd_printf           printf
#define sd_putbuf           printf_buf
#else
#define sd_puts(...)
#define sd_printf(...)
#define sd_putbuf(...)
#endif


#ifdef  RCSP_DEBUG

#define rcsp_put_buf       put_buf
#define rcsp_printf_buf    printf_buf
#define rcsp_db_puts       puts
#define rcsp_db_printf     printf
#define rcsp_err_puts      puts
#define rcsp_err_printf    printf
#define rcsp_tag_puts          puts
#define rcsp_tag_printf        printf

#else

#define rcsp_put_buf(...)
#define rcsp_printf_buf(...)
#define rcsp_db_puts(...)
#define rcsp_db_printf(...)

#define rcsp_err_puts(...)
#define rcsp_err_printf(...)
#define rcsp_tag_puts(...)
#define rcsp_tag_printf(...)

#endif /* #ifdef RCSP_DEBUG */

#ifdef EFUSE_DEBUG  // EFUSE升级流程打印
#define efuse_putchar       putchar
#define efuse_put_buf       put_buf
#define efuse_puts          puts
#define efuse_printf        printf
#define efuse_printf_buf    printf_buf
#define efuse_put_u32hex    put_u32hex
#define efuse_put_u8hex  	put_u8hex
#else
#define efuse_putchar(...)
#define efuse_put_buf(...)
#define efuse_puts(...)
#define efuse_printf(...)
#define efuse_printf_buf(...)
#define efuse_put_u32hex(...)
#define efuse_put_u8hex(...)
#endif



/*模块源代码控制
#define SD_MODULE_CONTROL				0		//SD模块
#define USB_HOST_MODULE_CONTROL			0		//USB主机模块
#define USB_SLAVE_MODULE_CONTROL		0		//USB从机模块
#define BT_UPDATA_MODULE_CONTROL		2		//BT模块	0:off	1:class		2:ble
#define UART_UPDATA_MODULE_CONTROL		0		//UART模块
#define BLE_GATT_UPDATA_MODULE_CONTROL	1		//ble gatt模块
#define SPP_UPDATA_MODULE_CONTROL		0		//spp 模块
*/

#if (BT_UPDATA_MODULE_CONTROL == 1) ///edr和ble只能开一个
#define EDR_UPDATA_SUPPORT_CONNECT		1 //BT2.1 support 2.1 connect 模块
#define BLE_UPDATA_SUPPORT_CONNECT		0 //BLE 4.2模块
#elif (BT_UPDATA_MODULE_CONTROL == 2) ///edr和ble只能开一个
#define EDR_UPDATA_SUPPORT_CONNECT		0 //BT2.1 support 2.1 connect 模块
#define BLE_UPDATA_SUPPORT_CONNECT		1 //BLE 4.2模块
#endif

#if (EDR_UPDATA_SUPPORT_CONNECT == 1)
#define BLE_BREDR_MODE     BT_BREDR_EN
#undef BLE_UPDATA_SUPPORT_CONNECT
#define BLE_UPDATA_SUPPORT_CONNECT     0
#endif

#if (BLE_UPDATA_SUPPORT_CONNECT == 1)
#define BLE_BREDR_MODE      BT_BLE_EN
#undef  EDR_UPDATA_SUPPORT_CONNECT
#define EDR_UPDATA_SUPPORT_CONNECT    0
#endif

/*升级设备在线检测使能位*/
#define UPDATA_DEV_ONLINE_CHECK_EN		0
#define UPDATA_FLASH_CONTROL			1//升级使能

#define BT_BREDR_EN  BIT(0)
#define BT_BLE_EN    BIT(1)
#define BT_NOMALE_POWER_CFG				0    //模拟正常配置
#define BT_LOW_POWER_CFG				1    //模拟低功耗配置

#define BT_ANALOG_CFG  BT_NOMALE_POWER_CFG   //模拟配置

/*
#define    PWR_NO_CHANGE     0     //   0:  no change
#define    PWR_LDO33         1     //   1:  LDOIN 5v -> VDDIO 3.3v -> DVDD 1.2v
#define    PWR_LDO15         2     //   2:  LDOIN 5v -> LDO   1.5v -> DVDD 1.2v, support bluetooth
#define    PWR_DCDC15        3     //   3:  LDOIN 5v -> DCDC  1.5v -> DVDD 1.2v, support bluetooth

#define PWR_MODE_SELECT    PWR_LDO15
*/
enum {
    BT_UPDATA_POWER_ON,
    BT_UPDATA_JUMP_APP,

};
enum {
    REQUES_CHECK_DATA = 0,
    REQUES_READ_DATA = 1,
    REQUES_STOP_DATA = 2,
};
enum {
    BT_UPDATE_OVER = 0,
    BT_UPDATE_KEY_ERR,
    BT_UPDATE_CONNECT_ERR,

};
typedef enum _UPDATA_RESULT {
    DEV_UPDATA_SUCC = 1,
    DEV_UPDATA_FAIL,
    NO_UPDATA_DEV,
    DEV_POWER_BT_UPDATA,
    DEV_UPDATA_KEY_ERR,
} UPDATA_RESULT;
#if (BLE_UPDATA_SUPPORT_CONNECT)
// huayue add 2017/1/12
#define BT_POWER_MODE_ENABLE	    0
#define BLE_ENCRYPT_ENABLE		0
#define BLE_PRIVACY_ENABLE		0
#define BLE_MASTER_ENABLE		    0
#define THREAD_DEBUG_EN			1
#define BQB_RF_SET_DEBUG_EN		1
#define BQB_SYS_TIMER_DEBYG_EN	1
#define BLE_ATT_ENCRYPT_EN		0
#else
#define BT_POWER_MODE_ENABLE	    0
#define BLE_ENCRYPT_ENABLE		0
#define BLE_PRIVACY_ENABLE		0
#define BLE_MASTER_ENABLE		    0
#define THREAD_DEBUG_EN			0
#define BQB_RF_SET_DEBUG_EN		0
#define BQB_SYS_TIMER_DEBYG_EN	0
#define BLE_ATT_ENCRYPT_EN		0

#endif

//配置是否支持uboot升级
#if defined(CONFIG_CPU_BR22) || defined(CONFIG_CPU_BR21) || defined(CONFIG_CPU_BR18)
#define ALL_FLASH_INCLUDE_UBOOT_UPDATE_EN	0
#else //BR23/BR25/BR26/BR30/BD29
#define ALL_FLASH_INCLUDE_UBOOT_UPDATE_EN	1
#endif

//br18 & br22 origin version
#define FLASH_FRAMEWORK_VERSION_V1_EN		0
//br22 new flash framwork
#define FLASH_FRAMEWORK_VERSION_V2_EN		1


//配置单独配置升级采用什么格式
#define FORMAT_BR22_NEW		0
#define FORMAT_BR18_V2		1

#ifdef CONFIG_CPU_BR22
#define CFG_TOOL_BIN_FORMAT_SEL	FORMAT_BR22_NEW
#else 	//BR18/BR26/BR23/BD29/BR21/BR25/BR30
#define CFG_TOOL_BIN_FORMAT_SEL	FORMAT_BR18_V2
#endif

//配置是否支持外部用户接口
#if defined(CONFIG_CPU_BR23)
#define USER_API_CODE_SUPPORT_EN		1
#else
#define USER_API_CODE_SUPPORT_EN		0
#endif

//是否SPI0只采用TEAM A;
#define SPI0_PORT_TEAM_A_ONLY_EN		1

/*是否开启电量检测*/
#define LOW_POWER_DET_EN				0

//是否支持升级外部DSP
#if defined(CONFIG_CPU_BR22) && (EDR_UPDATA_SUPPORT_CONNECT)
#define EX_DSP_UPDATE_SUPPORT_EN		0
#else
#define EX_DSP_UPDATE_SUPPORT_EN		0
#endif

#if defined(CONFIG_CPU_BR23)
#define EX_FLASH_UPDATE_SUPPORT_EN      1
#else
#define EX_FLASH_UPDATE_SUPPORT_EN      0
#endif

#if 0//defined(CONFIG_CPU_BR23) && (EDR_UPDATA_SUPPORT_CONNECT)
//配置:经典蓝牙升级速度优化方案
//1.utilized the time waiting for rsp to program flash;
#define OTA_UPDATE_SPEED_OPTIMIZE_METHOD_1		1
//2.utilized the time waiting for programming to receive rsp data;
#define OTA_UPDATE_SPEED_OPTIMIZE_METHOD_2		1
#else
//配置:经典蓝牙升级速度优化方案
//1.utilized the time waiting for rsp to program flash;
#define OTA_UPDATE_SPEED_OPTIMIZE_METHOD_1		0
//2.utilized the time waiting for programming to receive rsp data;
#define OTA_UPDATE_SPEED_OPTIMIZE_METHOD_2		0

#endif

//支持ANC配置和系数升级使能
#if (defined(CONFIG_CPU_BR30) \
		|| defined(CONFIG_CPU_BR34) \
		|| defined(CONFIG_CPU_BR36) \
		|| defined(CONFIG_CPU_BR28) \
	 	|| defined(CONFIG_CPU_WL82)) \
		&& (EDR_UPDATA_SUPPORT_CONNECT || UART_UPDATA_MODULE_CONTROL)
#define ANC_CFG_AND_COEF_UPDATE_EN		1
#else
#define ANC_CFG_AND_COEF_UPDATE_EN		0
#endif

//配置:是否升级保留区域文件(anc param .ect)
#if (CONFIG_CPU_BR30) || (CONFIG_CPU_BR34) || (CONFIG_CPU_BR36) || (CONFIG_CPU_BR28)
#define RESERVED_FILE_UPDATE_EN			1
#else
#define RESERVED_FILE_UPDATE_EN			0
#endif

#if defined(CONFIG_CPU_BR40)
#define UPDATE_EFUSE                    1
#define EFUSE_CONFIG_SIZE               123 // br40 芯片中 efuse 的最大值
#else
#define UPDATE_EFUSE                    0
#define EFUSE_CONFIG_SIZE               123 // br40 芯片中 efuse 的最大值
#endif

//支持免晶振方案
#if (SD_MODULE_CONTROL || USB_HOST_MODULE_CONTROL || UART_UPDATA_USER_MODULE_CONTROL) && (defined(CONFIG_CPU_BR23) || defined(CONFIG_CPU_BR25))
#define CRYSTAL_FREE_EN					1
#else
#define CRYSTAL_FREE_EN					0
#endif

//仅支持串口测试模式
#define UART_UPDATE_ONLY_TEST_MODE		0

//蓝牙升级仅测试模式
#define BT_UPDATE_ONLY_TEST_MODE		0
//蓝牙测试模式是否不需要回连
#define BT_UPDATE_ONLY_TEST_WITHOUT_RECONNECT    0

#if FLASH_FRAMEWORK_VERSION_V1_EN && FLASH_FRAMEWORK_VERSION_V2_EN
#error "FLASH_FRAMEWORK_VERSION_1&2 NOT OPEN IN SAME TIME!!!\n"
#endif
#endif
