#include "system/includes.h"
#include "typedef.h"
#include "asm/WL82_imd.h"
#include "os/os_api.h"
#include "device/device.h"
#include "app_config.h"


static int imd_test_task(void)
{
    void *imd_hdl = NULL;
    int ret;
    u32 len = 0;
    u8 *buf = NULL;

    imd_hdl = dev_open("imd", NULL);
    if (!imd_hdl) {
        puts("open imd err \n");
        return 0;
    }
#if 1  //使用内部双缓冲区的一个缓冲区来更改数据
    while (1) {
		//1.获取缓冲区可用的数据地址
        dev_ioctl(imd_hdl, IOCTL_IMD_GET_FRAME_BUFF, (u32)&buf);
		//2.获取缓冲区数据地址长度
		dev_ioctl(imd_hdl, IOCTL_IMD_GET_FRAME_BUFF_SIZE, (u32)&len);
		if(len && buf) {
			memset(buf, 0x0, len);//全写0黑色RGB数据
			//3.更新数据到获取缓冲区，更新屏显
			dev_ioctl(imd_hdl, IOCTL_IMD_UPDATE_BUFF, (u32)buf);
			buf = NULL;
		}
		printf("imd test ...\n");
		os_time_dly(500);
	}
#else //使用外部malloc出来的buff，此用法会多占用一幅RGB图内存大小
	//1.获取缓冲区数据地址长度
	dev_ioctl(imd_hdl, IOCTL_IMD_GET_FRAME_BUFF_SIZE, (u32)&len);
	if(!len){
		puts("err in get imd len\n");
		goto exit;
	}
	buf = malloc(len);
	if (!buf) {
		puts("err no mem \n");
		goto exit;
	}
	while (1) {
		memset(buf, 0x0, len);//全写0黑色RGB数据
		//3.更新数据到获取缓冲区，更新屏显
		dev_write(imd_hdl, (void*)buf, len);
		printf("imd test ...\n");
		os_time_dly(500);
	}
#endif

exit:
    if(imd_hdl){
		dev_close(imd_hdl);
	}
	return 0;
}
int imd_test(void)
{
    return thread_fork("imd_test_task", 18, 512, 0, NULL, imd_test_task, NULL);
}
