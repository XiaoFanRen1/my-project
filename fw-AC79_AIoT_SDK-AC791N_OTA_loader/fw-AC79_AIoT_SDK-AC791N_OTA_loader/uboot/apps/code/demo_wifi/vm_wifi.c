#include "system/includes.h"
#include "os/os_api.h"
#include "asm/hwi.h"
#include "wifi/wifi_connect.h"
#include "utils/syscfg_id.h"
#include "logic/boot.h"
#include "norflash.h"
#include "asm/crc16.h"

extern u32 get_flash_capability(void);
extern void doe(u16 k, void *pBuf, u32 lenIn, u32 addr);
extern s32 spi0flash_read(u8 *buf, u32 addr, u16 len);
extern s32 spi0flash_write(u8 *buf, u32 addr, u32 len);
extern void spi0flash_eraser(FLASH_ERASER eraser, u32 address);
extern u32 flash_get_vm_area_info(u32 *addr, u32 *len);
extern void spi0flash_any_write(u8 *buf, u32 addr, u32 len);

extern struct wifi_store_info wifi_vm_info;
int wifi_get_pa_trim_data(u8 *pa_data)
{
    return 0;
}
void wifi_get_xosc(u8 *xosc)
{
}
void wifi_get_mcs_dgain(u8 *mcs_dgain)
{
}
//===============================================从备份的VM区域读取备份VM的wifi数据================================================//
struct vm_find_index {
    u16 index;
    u8 *buf;
    u32 buf_size;
};
struct vm_info_type {
    u32 vm_addr;
    u32 vm_len;
    char *vm_data;
};
static struct vm_info_type vm_addr_info = {0};
static char http_url[2048] = {0};
static struct vm_find_index vm_find_index_table[] = {
    {WIFI_INFO_IDX, 			&wifi_vm_info, 	sizeof(struct wifi_store_info)},
    {VM_HTTP_SINGLE_UPDATE_URL,	http_url, 		sizeof(http_url)},
};

void vm_addr_info_save(void *boot_info)
{
    if (boot_info) {
        struct boot_info_t *boot = boot_info;
        struct vm_info_type *vm_info = boot->arch_info;
        put_buf((u8 *)vm_info, 8);
        vm_addr_info.vm_addr = vm_info->vm_addr;
        vm_addr_info.vm_len = vm_info->vm_len;
        put_buf(boot->arch_info, 32);
        printf("================vm_addr = 0x%x, len = %d ===============\n", vm_addr_info.vm_addr, vm_addr_info.vm_len);
    }
}
void boot_info_save(void *info)
{
    struct boot_info_t *boot = info;
    vm_addr_info_save(info);
}
static int vm_data_read(void)
{
    u8 buf[256];
    int ret;
    int size = vm_addr_info.vm_len;
    int remain = size;
    int read_size = 0;
    if (vm_addr_info.vm_addr && vm_addr_info.vm_len) {
        if (!vm_addr_info.vm_data) {
            vm_addr_info.vm_data = malloc(vm_addr_info.vm_len);
        }
        if (vm_addr_info.vm_data) {
            size = vm_addr_info.vm_len;
            while (remain) {
                size = MIN(remain, sizeof(buf));
                memset(buf, 0, sizeof(buf));
                ret = spi0flash_read((u8 *)buf, vm_addr_info.vm_addr + read_size, size);
                if (ret != size) {
                    printf("err in vm_data_read %d %d \n", ret, size);
                    if (vm_addr_info.vm_data) {
                        free(vm_addr_info.vm_data);
                        vm_addr_info.vm_data = NULL;
                        read_size = 0;
                    }
                    break;
                }
                memcpy(vm_addr_info.vm_data + read_size, buf, size);
                read_size += size;
                remain -= size;
            }
            if (read_size) {
                printf("vm_data_read ok, size = %d, vm_addr_info.vm_len = %d\n", read_size, vm_addr_info.vm_len);
            }
        }
    }
    return read_size;
}
int vm_data_recover(void)
{
#define FLASH_SECTOR_ERASER_SIZE 4096
    u8 buf[256];
    u32 vm_addr = 0;
    u32 vm_size = 0;
    int size = vm_addr_info.vm_len;
    u32 app_code_addr = JL_SFC->BASE_ADR;
    int remain = size;
    int read_size = 0;
    int ret = 0;
    int erase_addr;

    if (vm_addr_info.vm_data && vm_addr_info.vm_len) {
        flash_get_vm_area_info(&vm_addr, &vm_size);
        erase_addr = vm_addr;
        if (vm_addr > app_code_addr && vm_size >= vm_addr_info.vm_len) {
            if (erase_addr % FLASH_SECTOR_ERASER_SIZE == 0) {
                while (erase_addr < (vm_addr + vm_addr_info.vm_len)) {
                    spi0flash_eraser(FLASH_SECTOR_ERASER, erase_addr);
                    printf("vm_data_recover_eraser_addr = 0x%x\n", erase_addr);
                    erase_addr += FLASH_SECTOR_ERASER_SIZE;
                }
            }
            while (remain) {
                size = MIN(remain, sizeof(buf));
                memcpy(buf, vm_addr_info.vm_data + read_size, size);
                spi0flash_any_write((u8 *)buf, vm_addr + read_size, size);
                read_size += size;
                remain -= size;
            }
            ret = vm_addr_info.vm_len;
            printf("vm_data_recover ok , vm_addr = 0x%x , size = %d\n", vm_addr, ret);
            erase_addr = vm_addr_info.vm_addr;
            if (erase_addr % FLASH_SECTOR_ERASER_SIZE == 0) {
                while (erase_addr < (vm_addr_info.vm_addr + vm_addr_info.vm_len)) {
                    spi0flash_eraser(FLASH_SECTOR_ERASER, erase_addr);
                    printf("vm backup erase_sector_addr = 0x%x\n", erase_addr);
                    erase_addr += FLASH_SECTOR_ERASER_SIZE;
                }
            }
        }
    }
    return ret;
}

static int vm_area_info_index_find(int index, u32 addr, u8 vm_crc, int vm_size, u32 key)
{
    int size = vm_size;
    int find = 0;
    struct vm_find_index *vm_index;
    for (int i = 0; i < sizeof(vm_find_index_table) / sizeof(vm_find_index_table[0]); i++) {
        vm_index = &vm_find_index_table[i];
        if (vm_index->index == index && vm_index->buf) {
            size = MIN(vm_index->buf_size, size);
            spi0flash_read((u8 *)vm_index->buf, addr, size);
            doe(key, vm_index->buf, size, 0);
            if (vm_crc != (u8)CRC16(vm_index->buf, size)) {
                memset(vm_index->buf, 0, vm_index->buf_size);
                break;
            }
            printf("---> save index = %d , size = %d\n", index, vm_size);
            find = 1;
            break;
        }
    }
    return find;
}

int vm_area_info_find(void)
{
#define VM_MAGIC_USING          ((u32)0x55AAAA55)
#define LABEL_INDEX_LEN_CRC_SIZE 	4
#define FLASH_SECTOR_SIZE 			4096
    u32 vm_addr = 0, vm_size = 0;
    u32 app_code_addr = JL_SFC->BASE_ADR;
    u8 flag_buf[LABEL_INDEX_LEN_CRC_SIZE];
    u32 vm_len = 0;
    u32 vm_index = 0;
    u8 vm_crc, crc;
    u16 user_dec_key = 0xFFFF;//get_flash_dec_key(); //默认VM区域不加密，如果加密则这里的key需要使用芯片的KEY
    u32 last_secter_addr = get_flash_capability();
    int vm_used_size = LABEL_INDEX_LEN_CRC_SIZE;
    u8 find_vm = 0, power_off = 0;
    user_dec_key = user_dec_key == 0xFFFF ? 0 : user_dec_key;

    if (vm_addr_info.vm_addr <= app_code_addr) {
        vm_addr_info.vm_addr = last_secter_addr - FLASH_SECTOR_SIZE;
        vm_addr_info.vm_len = FLASH_SECTOR_SIZE;
        power_off = TRUE;
    }
    vm_addr = vm_addr_info.vm_addr;
    vm_size = vm_addr_info.vm_len;
    if (!vm_addr) {
        printf("err no vm_addr and vm_size\n");
        return 0;
    }
    printf("---> vm_addr = 0x%x , %d\n", vm_addr, vm_size);
    u32 magic = 0;
    u32 addr = vm_addr_info.vm_addr;
    u32 end_addr = (vm_addr_info.vm_addr + vm_size);
    u32 last_vm_addr = 0;
    int find = 0, redo_cnt = 0;
    int setp = vm_len + LABEL_INDEX_LEN_CRC_SIZE;
    for (; addr && addr < end_addr; addr += setp) {
        if (!find_vm) {
            spi0flash_read((u8 *)&magic, vm_addr_info.vm_addr, sizeof(magic));
            doe(user_dec_key, &magic, sizeof(magic), 0);
            addr += LABEL_INDEX_LEN_CRC_SIZE;
            if (magic != VM_MAGIC_USING) {
                goto next_sector;
            }
        }
        if (!last_vm_addr) {
            last_vm_addr = vm_addr_info.vm_addr;
        }
        find_vm = TRUE;
        spi0flash_read((u8 *)&flag_buf, addr, LABEL_INDEX_LEN_CRC_SIZE);
        doe(user_dec_key, flag_buf, LABEL_INDEX_LEN_CRC_SIZE, 0);
        vm_len = ((u8)flag_buf[3] << 4) | (((u8)flag_buf[2] & 0xF0) >> 4);
        vm_index = flag_buf[1] | (((u8)flag_buf[2] & 0xF) << 8);
        vm_crc = flag_buf[0];
        if (vm_index > 1024) {
next_sector:
            if (find_vm && !find && redo_cnt < 2) {
                find_vm = find = 0;
                redo_cnt++;
                last_secter_addr = get_flash_capability();
            }
            if (!find_vm) {
                last_secter_addr -= FLASH_SECTOR_SIZE;
                if (last_vm_addr && last_vm_addr == last_secter_addr) {
                    last_secter_addr -= FLASH_SECTOR_SIZE;
                }
                vm_addr = last_secter_addr;
                addr = last_secter_addr;
                end_addr = addr + FLASH_SECTOR_SIZE;
                vm_addr_info.vm_addr = addr;
                vm_addr_info.vm_len = vm_used_size;
                setp = 0;
                continue;
            }
            break;
        }
        printf("vm_index = %d\n", vm_index);
        find |= vm_area_info_index_find(vm_index, addr + LABEL_INDEX_LEN_CRC_SIZE, vm_crc, vm_len, user_dec_key);
        setp = vm_len + LABEL_INDEX_LEN_CRC_SIZE;
        vm_used_size += vm_len + LABEL_INDEX_LEN_CRC_SIZE;
        if (power_off) {
            vm_addr_info.vm_len = vm_used_size;
        }
    }
    return 0;
}
char *vm_http_get(void)
{
    if (http_url[0] != 0) {
        return http_url;
    }
    return NULL;
}
void vm_find(void)
{
    vm_area_info_find();
    vm_data_read();
}
//===============================================从备份的VM区域读取备份VM的wifi数据================================================//


