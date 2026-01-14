#include "lib_include.h"

__attribute__((section(".version_tag1"), used))
#if (1 == BT_UPDATA_MODULE_CONTROL )
static const char version_type_tag[] = "edr_ota2";
#elif (2 == BT_UPDATA_MODULE_CONTROL)
static const char version_type_tag[] = "ble_ota";
#elif (1 == USB_HOST_MODULE_CONTROL)

// BR25 二级loader修改
#if defined(CONFIG_CPU_BR25)
static const char version_type_tag[] = "usb_update2/usb_sec_ota";
#else
static const char version_type_tag[] = "usb_update2";
#endif

#elif (1 == SD_MODULE_CONTROL)
// BR25 二级loader修改
#if defined(CONFIG_CPU_BR25)
static const char version_type_tag[] = "sd_update2/sd_sec_ota";
#else
static const char version_type_tag[] = "sd_update2";
#endif

#elif (1 == BLE_GATT_UPDATA_MODULE_CONTROL)
static const char version_type_tag[] = "ble_app_ota";
#elif (1 == SPP_UPDATA_MODULE_CONTROL)
static const char version_type_tag[] = "spp_app_ota";
#elif (1 == UART_UPDATA_MODULE_CONTROL)
static const char version_type_tag[] = "uart_update";
#elif (1 == UART_UPDATA_USER_MODULE_CONTROL)
static const char version_type_tag[] = "user_uart_update";
#elif (1 == USER_NORFLASH_UPDATA_MODULE_CONTROL)
static const char version_type_tag[] = "user_norflash_ufw_update";
#elif (1 == USER_LC_FLASH_UPDATA_MODULE_CONTROL)
static const char version_type_tag[] = "user_lc_flash_ufw_update";
#elif (1 == USB_HID_MODULE_CONTROL)
static const char version_type_tag[] = "usb_hid_ota";
#elif (1 == DEV_NORFLASH_UPDATA_MODULE_CONTROL)
static const char version_type_tag[] = "dev_nor_ota";
#elif (1 == NET_UPDATA_MODULE_CONTROL)
static const char version_type_tag[] = "net_ota";
#endif

__attribute__((section(".version_tag2"), used))
static const char version_date_tag[] = __DATE__;

__attribute__((section(".version_tag3"), used))
static const char version_time_tag[] = __TIME__;

