#ifndef __UPDATA_MODE_H__
#define __UPDATA_MODE_H__

#include  "logic_include.h"

#define MAX_DEVICE	4

u8 dev_type_2_index(u16 dev_type);

bool get_burn_area(void *buf);
bool write_begin_burn(u16 updata_type);
bool write_end_burn(u16 updata_type);
bool write_err_burn(u16 updata_type);

enum EXT_ARG_TYPE {
    EXT_LDO_TRIM_RES = 0,
    EXT_JUMP_FLAG,
    EXT_SHARE_OSC_EN,
    EXT_TYPE_MAX = 0xff,
};

#endif/*__UPDATA_MODE_H__*/
