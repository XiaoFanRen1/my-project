#include "system/includes.h"
#include "typedef.h"
#include "asm/pap.h"
#include "os/os_api.h"
#include "device/device.h"
#include "app_config.h"

static int pap_task_handler(void *priv)
{
    u8 data[512] = {0};
    u16 read_data = 0;

	void *pap_hdl = NULL;
    pap_hdl = dev_open("pap", NULL);//打开设备
    if (pap_hdl == NULL) {
        printf("-----PAP NG------\n");
        return -1;
    }

    for (int i = 0; i < ARRAY_SIZE(data); i++) {
        if (i % 2) {
            data[i] = 0xbb;
        } else {
            data[i] = 0x0000;
        }
    }

    while(1){
        printf("======================pap test=====================\n");
        dev_write(pap_hdl, data, 512);//写数据
        os_time_dly(100);
    }
    return 0;
}


int pap_test(void)
{
    return thread_fork("pap_task", 18, 1024, 0, NULL, pap_task_handler, NULL);
}
