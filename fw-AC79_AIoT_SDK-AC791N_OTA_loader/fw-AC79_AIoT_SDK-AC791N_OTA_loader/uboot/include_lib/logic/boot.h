#ifndef __BOOT_H__
#define __BOOT_H__

#include "typedef.h"
#include "lib_include.h"

//updata_flag
#define UPDATA_FLAG_ADDR		((void *)(0x1C000-0x80))		/* (0x1C000-0x80)-0x1C000:		reserved_ram for updata */
#define UPDATE_PRIV_PARAM_LEN	32
#define UPDATA_MAGIC			(0x5A00)		//防止CRC == 0 的情况

typedef enum {
    UPDATA_NON = UPDATA_MAGIC,
    UPDATA_READY,
    UPDATA_SUCCESSFULLY,
    UPDATA_PARM_ERR,
    UPDATA_DEV_ERR,
    UPDATA_KEY_ERR,
} UPDATA_RESULT_TO_SDK;

typedef enum {
    USB_UPDATA = UPDATA_MAGIC,		//0x5A00
    SD0_UPDATA,						//0x5A01
    SD1_UPDATA,
    PC_UPDATA,
    UART_UPDATA,
    BT_UPDATA,
    BLE_APP_UPDATA,
    SPP_APP_UPDATA,
    DUAL_BANK_UPDATA,
    BLE_TEST_UPDATA,
    NORFLASH_UPDATA,
    // BLE_UPDATA,
    USER_NORFLASH_UFW_UPDATA,
    USER_LC_FLASH_UFW_UPDATA,
    USB_HID_UPDATA,
    DEV_NORFLASH_UFW_UPDATA,
    NET_UFW_UPDATA,

    NON_DEV_UPDATA = 0xFFFF,
} UPDATA_TYPE;

#if FLASH_FRAMEWORK_VERSION_V2_EN
#define UPDATE_PARAM_MAGIC		0x5441
typedef struct _UPDATA_PARM {
    u16 parm_crc;
    u16 parm_type;				//UPDATA_TYPE:sdk pass parm to uboot
    u16 parm_result;			//UPDATA_TYPE:uboot return result to sdk
    u16 magic;
    u8 	file_patch[32];
    u8 	parm_priv[32];
    u32 ota_addr;
    u16	ext_arg_len;
    u16 ext_arg_crc;
} UPDATA_PARM;

struct SYS_CFG {

    u32 sdtap: 8;

    u32 osc_type: 4;
    u32 osc_hc_en: 1;
    u32 osc_1pin_en: 1;
    u32 lsb_div: 4;
    u32 res: 4;

    u32 osc_freq;
    u32 sys_clk;

    char uttx[8];
    char utrx[8];
    u32 utbaud;

    char port_output[8];
    char port_input[8];

    char reset_pin[4];
};

struct boot_info_t {
    u32 entry_addr;
    u32 sfc_base_addr;

    struct SYS_CFG *sys_cfg;

    u16 chip_key;
    u16 res;//同步结构体和uboot一致
    u32 version;
    void *arch_info;
};
#elif FLASH_FRAMEWORK_VERSION_V1_EN
typedef struct _UPDATA_PARM {
    u16 parm_crc;
    u16 parm_type;				//UPDATA_TYPE:sdk pass parm to uboot
    u16 parm_result;			//UPDATA_TYPE:uboot return result to sdk
    u8 	file_patch[32];
    u8 	parm_priv[32];
} UPDATA_PARM;

struct SYS_CFG {
    //void *spi_data;

    u32 sdtap: 8;

    u32 osc_type: 4;
    u32 osc_hc_en: 1;
    u32 osc_1pin_en: 1;
    u32 lsb_div: 4;
    u32 res: 4;

    u32 osc_freq;
    u32 sys_clk;

    char uttx[8];
    char utrx[8];
    u32 utbaud;

    char port_output[8];
    char port_input[8];

    char reset_pin[4];
};

struct boot_info_t {
    u32 entry_addr;
    u32 sfc_base_addr;
    u32 sfc_encrypt_end_addr;
    u32 vm_end_addr;
    u32 flash_size;
    u32 size4burner;

    void *boot_device;
    char ota_loader_path[32];
    struct SYS_CFG *sys_cfg;

    u16 chip_key;
    u16 trim;
};
#endif


typedef enum _APP_RUN_RES {
    APP_RUN_SUCC = 0,
    APP_RUN_ERR0,
    APP_RUN_ERR1,
    APP_RUN_ERR2,
} APP_RUN_RES;

typedef enum _APP_DATA_CHECK_RES {
    APP_DATA_SUCC = 0,
    APP_DATA_ERR0,
    APP_DATA_ERR1,
    APP_DATA_ERR2,
    APP_DATA_KEY_ERR,
} APP_DATA_CHECK_RES;

typedef enum _DEV_STATUS {
    DEV_OFFLINE = 0,
    DEV_ONLINE,
} DEV_STATUS;


enum {
    BR18_FS_VERSION_VAL = 0,
    BR22_FS_VERSION_VAL,
    BR23_FS_VERSION_VAL,
    BR25_FS_VERSION_VAL,
    BR26_FS_VERSION_VAL,
    BR21_FS_VERSION_VAL,
    BR30_FS_VERSION_VAL,

    BD29_FS_VERSION_VAL,
    BR34_FS_VERSION_VAL,
    BD19_FS_VERSION_VAL,
    BR36_FS_VERSION_VAL,
    WL82_FS_VERSION_VAL,
};

extern u16 dev_num;

typedef struct _DEV_UPDATA_VARS {
    UPDATA_RESULT   updata_result;
    u16 dev_type;					//UPDATA_TYPE
} DEV_UPDATA_VARS;

void set_updata_type(u16 up_type);
void set_updata_result(u16 result);
void clr_updata_info(void);
bool updata_check_updata_result(void);


void sys_init(void *p_sdf_io);
APP_RUN_RES app_run(u32 addr);
void updata_mode(void *p_sdf_io, DEV_UPDATA_VARS *res, u32 addr);

u32 spi_reinit(void);
APP_DATA_CHECK_RES app_data_check(void);
#endif/*__BOOT_H__*/
