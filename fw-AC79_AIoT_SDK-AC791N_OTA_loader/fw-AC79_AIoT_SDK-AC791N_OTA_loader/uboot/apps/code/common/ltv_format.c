#include "lib_include.h"
#include "ltv_format.h"

#define LOG_TAG	"[LVT_FORMAT]"

//#define LOG_INFO_ENABLE
#include "debug.h"

static void *get_attr_ptr_by_type(u8 type, LTV_ATTR_GET *attr_ptr, u8 attr_item_max)
{
    u8 i;

    LTV_ATTR_GET *temp_attr_ptr;
    for (i = 0; i < attr_item_max; i++) {
        temp_attr_ptr = attr_ptr + i;
        if (temp_attr_ptr->type == type) {
            return temp_attr_ptr;
        }
    }

    return NULL;
}

u32 ltv_attr_total_len_get(u32 mask, LTV_ATTR_GET *attr_ptr, u8 attr_max)
{
    if (mask == 0) {
        return 0;
    }
    LTV_ATTR_GET *tmp_ptr;

    log_info("attr_max:%x\n", attr_max);
    u8 i;
    u32 attr_total_len = 0;
    for (i = 0; i < 32; i++) {
        if ((mask & BIT(i)) != 0) {
            tmp_ptr = get_attr_ptr_by_type(i, attr_ptr, attr_max);
            if (tmp_ptr && tmp_ptr->get_size) {
                attr_total_len += (tmp_ptr->get_size() + 2/*size + type*/);
            }
        }
    }

    return attr_total_len;
}


tbool ltv_attr_fill(u32 mask, LTV_ATTR_GET *attr_ptr, u8 attr_max, u8 *fill_buf)
{
    if (fill_buf == NULL) {
        return false;
    } else {
        u8 i;
        u8 *ptr = fill_buf;
        LTV_ATTR_GET *tmp_attr_ptr;
        for (i = 0; i < 32; i++) {
            if ((mask & BIT(i)) != 0) {
                tmp_attr_ptr = get_attr_ptr_by_type(i, attr_ptr, attr_max);
                if (tmp_attr_ptr && tmp_attr_ptr->get_size != NULL && tmp_attr_ptr->get_data != NULL) {
                    *ptr = tmp_attr_ptr->get_size() + 1/*type*/;
                    *(ptr + 1) = i;
                    memcpy(
                        ptr + 2,
                        tmp_attr_ptr->get_data(),
                        tmp_attr_ptr->get_size()
                    );
                    ptr += (*ptr) + 1/*size*/;
                }
            }
        }
    }

    return true;
}


