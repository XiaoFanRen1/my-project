#ifndef __LOGIC_INCLUDES_H__
#define __LOGIC_INCLUDES_H__

#include "lib_include.h"
#include "sdmmc_cpu_interface.h"

#define ONE_TIME_RECEIVE_ALL_DATA	1
#define UPDATA_ONCE_LEN_SUPPORT_OVER_4K	1

/*设备操作状态*/
enum {
    DEV_IDLE = 0,
    DEV_READING,
    DEV_WRITTING,
};

#endif/*__LOGIC_INCLUDES_H__*/

