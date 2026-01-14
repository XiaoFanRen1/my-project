#ifndef _USB_SCSI_UPGRADE_H
#define _USB_SCSI_UPGRADE_H

#include "typedef.h"

#define LOG_D(tag,args,...)\
do{\
    printf("[%s] " args , tag,  ##__VA_ARGS__);\
}while(0)

#define update_printf(...)   LOG_D(LOG_TAG, __VA_ARGS__)


enum {
    UPGRADE_CODE_SUCCESS = 0,  //升级成功
    UPGRADE_LOADER_ERR,        //loader解析失败
    UPGRADE_UFW_ERR,           //ufw解析失败
    UPGRADE_FLASHBIN_ERR,      //flash bin解析失败
    UPGRADE_CFG_ERR,           //sys cfg解析失败
    UPGRADE_JUMP_ERR,          //跳转失败
    UPGRADE_READID_ERR,        //读flash id失败
    UPGRADE_CHIPID_ERR,        //读chip id失败
    UPGRADE_CHIPKEY_ERR,       //读chip key失败
    UPGRADE_KEY_ERR,           //key不匹配
    UPGRADE_FLASH_TYPE_ERR,    //flash类型错误
    UPGRADE_ERASE_FLASH_ERR,   //擦除flash失败
    UPGRADE_WRITE_FLASH_ERR,   //写flash失败
    UPGRADE_VERIFY_FLASH_ERR,  //校验flash失败
};

#endif
