#ifndef __SDMMC_CPU_INTERFACE_H__
#define __SDMMC_CPU_INTERFACE_H__

#include "lib_include.h"

/*SD控制器选择*/
typedef enum {
    SD_CONTROLLER_0 = 1, //sd0控制器
    SD_CONTROLLER_1,     //sd1控制器
} SD_CONTROLLER_TYPE;

/*SD控制器对应IO出口*/
typedef enum {
    SD0_IO_A = 1, //sd0控制器A出口
    SD0_IO_B,     //sd0控制器B出口

    SD1_IO_A,     //sd1控制器A出口
    SD1_IO_B,     //sd1控制器B出口
    SD0_IO_C,
    SD0_IO_D,
    SD0_IO_E,
    SD0_IO_F,

} SD_CONTROLLER_IO;


// extern func
extern void sd_port_io(u8 control_type, u8 control_io);
extern void SD_CMD_MODE(u8 controller_type, u8 mode);
extern u16 SD_CMD_CRC(u8 controller_type);
extern void SD_8CLK_EN(u8 controller_type, u8 enable);
extern u16 SD_CMD_TIMEOUT(u8 controller_type);
extern void CLR_SD_CMD_PENDING(u8 controller_type);
extern u16 SD_CMD_PENDING(u8 controller_type);
extern void SD_DAT_MODE(u8 controller_type, u8 mode);
extern u16 SD_DAT_CRC(u8 controller_type);
extern void SD_DAT_8CLK_EN(u8 controller_type, u8 enable);
extern void SD_SET_4WIRE_MODE(u8 controller_type, u8 enable);
extern void CLR_SD_DAT_PENDING(u8 controller_type);
extern u16 SD_DAT_PENDING(u8 controller_type);
extern void SD_CONTROLLER_RESET(u8 controller_type);
extern void SD_CONTROLLER_EN(u8 controller_type, u8 enable);
extern u16 SD_CRC_STATE(u8 controller_type);
extern void SD_DAT_EDGE(u8 controller_type, u8 edge);
extern void SD_IDLE_CLK_EN(u8 controller_type, u8 enable);
extern void SD_CMD_IE(u8 controller_type, u8 enable);
extern void SD_DAT_IE(u8 controller_type, u8 enable);
extern void SD_BAUD(u8 controller_type, u8 baudrate);
extern u8 SD_BLOCK_WRITE_ERR(u8 controller_type);
extern void SET_SD_CMD_PTR(u8 controller_type, void *pbuf);
extern void SET_SD_DAT_PTR(u8 controller_type, void *pbuf);
extern void SET_SD_CNT(u8 controller_type, u16 cnt);
extern u8 SD_CONTROLLER_READ(u8 controller_type);
extern void sdmmc0_isr_init(u32 isr_addr);
extern void sdmmc1_isr_init(u32 isr_addr);

extern u16 get_sd0_cmd_pending(void);
extern u16 get_sd1_cmd_pending(void);
extern u16 get_sd0_dat_pending(void);
extern u16 get_sd1_dat_pending(void);
extern void clr_sdmmc0_isr(void);
extern void clr_sdmmc1_isr(void);

#endif
