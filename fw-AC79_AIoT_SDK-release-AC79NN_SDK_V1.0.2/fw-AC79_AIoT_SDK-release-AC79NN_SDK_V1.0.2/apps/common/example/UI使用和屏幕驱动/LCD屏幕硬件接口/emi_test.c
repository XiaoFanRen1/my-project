#include "system/includes.h"
#include "typedef.h"
#include "asm/pap.h"
#include "asm/emi.h"
#include "os/os_api.h"
#include "device/device.h"
#include "app_config.h"


static int emi_test_task(void)
{
    void *emi_hdl = NULL;
    int len;
    u8 buf[32];

    emi_hdl = dev_open("emi", NULL);//打开设备
    if (!emi_hdl) {
        puts("open emi err \n");
        return 0;
    }
    dev_ioctl(emi_hdl, EMI_USE_SEND_SEM, 0);//使用信号等待发送完成
    u8 cnt = 0;
    os_time_dly(300);
    printf("---> write emi data ...\n");
    os_time_dly(200);
    while (1) {
        cnt++;
        memset(buf, 0XA5, sizeof(buf));
        dev_write(emi_hdl, buf, sizeof(buf));//发送数据
        os_time_dly(500);
		printf("emi test ...\n");
    }
    return 0;
}
int emi_test(void)
{
    return thread_fork("emi_test_task", 18, 512, 0, NULL, emi_test_task, NULL);
}
