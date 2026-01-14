#include "config.h"
#include "boot.h"
#include "includes.h"

const u8 *uart_update_default_io = "PP00";

const u8 FLASH_HEAD_FS_VERSION = WL82_FS_VERSION_VAL;

#ifdef CONFIG_WL_AP_ENABLE
#define MALLOC_POOL_SIZE	(90 * 1024)
#else
/*#error "err no malloc_pool"*/
#define MALLOC_POOL_SIZE	(2 * 1024)
#endif
u32 malloc_pool[MALLOC_POOL_SIZE / 4];
u32 malloc_pool_size = MALLOC_POOL_SIZE;

void update_enter_jump_maskrom(void)
{
    //TODO
}
