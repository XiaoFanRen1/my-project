#ifndef _NORFLASH_H
#define _NORFLASH_H

#include "asm/spi_hw.h"
#include "gpio.h"

/*************************************************/
/*
		COMMAND LIST - WinBond FLASH WX25X
*/
/***************************************************************/
#define WINBOND_WRITE_ENABLE		                        0x06
#define WINBOND_WRITE_DISABLE	                            0x04
#define WINBOND_HIGH_PERFORMANCE_MODE			            0xA3
#define WINBOND_RELEASE_POWER_DOWN_OR_HIGH_PERFORMANCE_MODE 0xAB
#define WINBOND_READ_SR1			                        0x05
#define WINBOND_READ_SR2			                        0x35
#define WINBOND_WRITE_SR			                        0x01
#define WINBOND_READ_DATA		                            0x03
#define WINBOND_FAST_READ			                        0x0B
#define WINBOND_FAST_READ_DUAL_OUTPUT	                    0x3B
#define WINBOND_FAST_READ_DUAL_IO                           0xBB
#define WINBOND_FAST_READ_QUAD_OUTPUT                       0x6B
#define WINBOND_FAST_READ_QUAD_IO                           0xEB
#define WINBOND_WORD_READ_QUAD_IO                           0xE7
#define WINBOND_OCTAL_WORD_READ_QUAD_IO                     0xE3
#define WINBOND_CONTINUOUS_READ_ENHANCE_MODE                0xa0
#define WINBOND_CONTINUOUS_READ_NORMAL_MODE                 0x00
#define WINBOND_PAGE_PROGRAM	                            0x02
#define WINBOND_PAGE_PROGRAM_QUAD	                        0x32
#define WINBOND_SECTOR_ERASE		                        0x20
#define WINBOND_BLOCK_ERASE		                            0xD8
#define WINBOND_CHIP_ERASE		                            0xC7
#define WINBOND_POWER_DOWN		                            0xB9
#define WINBOND_RELEASE			                            0xAB
#define WINBOND_MANUFACTURE		                            0x90
#define WINBOND_JEDEC_ID			                        0x9F
#define PAGE_ERASE_CMD										0x81
// #define WINBOND_WRITE_ENABLE		                        0x06
// #define WINBOND_READ_SR1			                        0x05
// #define WINBOND_READ_SR2			                        0x35
// #define WINBOND_WRITE_SR1			                        0x01
// #define WINBOND_WRITE_SR2			                        0x31
// #define WINBOND_READ_DATA		                            0x03
#define WINBOND_FAST_READ_DATA		                        0x0b
// #define WINBOND_FAST_READ_DUAL_OUTPUT                       0x3b
// #define WINBOND_PAGE_PROGRAM	                            0x02
#define WINBOND_PAGE_ERASE                                  0x81
// #define WINBOND_SECTOR_ERASE		                        0x20
// #define WINBOND_BLOCK_ERASE		                            0xD8
// #define WINBOND_CHIP_ERASE		                            0xC7
// #define WINBOND_JEDEC_ID                                    0x9F
//

typedef enum _FLASH_ERASER {
    FLASH_CHIP_ERASER,
    FLASH_BLOCK_ERASER,
    FLASH_SECTOR_ERASER,
    FLASH_PAGE_ERASER,
} FLASH_ERASER;

struct norflash_dev_platform_data {
    int spi_hw_num;         //只支持SPI1或SPI2
    u32 spi_cs_port;        //cs的引脚
    u32 spi_read_width;     //flash读数据的线宽
    u32 spi_power_pin;      //flash供电脚
    struct spi_platform_data *spi_pdata;
    u32 start_addr;         //分区起始地址
    u32 size;               //分区大小，若只有1个分区，则这个参数可以忽略
};

int _norflash_init(const char *name, struct norflash_dev_platform_data *pdata);
int _norflash_open(void);
int _norflash_write(u32 addr, void *buf, u32 len);
int _norflash_read(u32 addr, u8 *buf, u32 len);
void norflash_eraser_by_len(u32 erase_addr, u32 len);
int _norflash_close(void);
#endif


