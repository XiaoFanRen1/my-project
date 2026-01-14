#ifndef _CONFIG_
#define _CONFIG_

//#include <string.h>
#include "hw_cpu.h"
#include "typedef.h"
#include "common.h"
#include "clock.h"
#include "lib_include.h"

/*最小化UBOOT*/
#define UBOOT_SIMPLE_MODE   1

/*设备升级完成提醒*/
//#define UPDATA_LED_REMIND    //升级完成LED提醒
//#define UPDATA_VOICE_REMIND  //升级完成“滴滴声”提醒
#define BT_MODULE_CLASSIC                   BIT(0)
#define BT_MODULE_LE                        BIT(1)

#if EDR_UPDATA_SUPPORT_CONNECT
#define BT_MODULES_SUPPORT                  (BT_MODULE_CLASSIC)
#else
#define BT_MODULES_SUPPORT                  (BT_MODULE_LE)
#endif

#define BT_NORMAL       BIT(0)
#define BT_BQB          BIT(1)
#define BT_FCC          BIT(2)
#define BT_FRE          BIT(3)

/*
 *  *-------------------
 *   * 蓝牙基带运行的模式
 *    */
#define BT_MODE                             (BT_NORMAL)

/*-----------------------------------------------------------*/

/**
 *  * @brief Bluetooth LE setting
 *   */
//int btctler_le_features = LE_FEATURES;
//int btctler_le_roles    = LE_ROLES;
/*-------------------LE FEATURE SUPPORT *  brief : 运行时优化（LTO）下，代码空间优化；
 *  *  *
 *   *   *   */
#define LE_ENCRYPTION                       BIT(0)
#define CONNECTION_PARAMETER_REQUEST        BIT(1)
#define EXTENDED_REJECT_INDICATION          BIT(2)
#define LE_SLAVE_INIT_FEATURES_EXCHANGE     BIT(3)
#define LE_PING                             BIT(4)
#define LE_DATA_PACKET_LENGTH_EXTENSION     BIT(5)
#define LL_PRIVACY                          BIT(6)
#define EXTENDED_SCANNER_FILTER_POLICIES    BIT(7)
#define LE_2M_PHY                           BIT(8)
#define LE_CODED_PHY                        BIT(11)
#define LE_EXTENDED_ADVERTISING             BIT(12)
#define LE_PERIODIC_ADVERTISING             BIT(13)
#define CHANNEL_SELECTION_ALGORITHM_2       BIT(14)

#define LE_MASTER                           BIT(0)
#define LE_SLAVE                            BIT(1)
#define LE_ADV                              BIT(2)
#define LE_SCAN                             BIT(3)
#define LE_INIT                             BIT(4)

#define BT_BTSTACK_CLASSIC                   BIT(0)
#define BT_BTSTACK_LE_ADV                    BIT(1)
#define BT_BTSTACK_LE                        BIT(2)

extern const u8 *uart_update_default_io;

#endif
