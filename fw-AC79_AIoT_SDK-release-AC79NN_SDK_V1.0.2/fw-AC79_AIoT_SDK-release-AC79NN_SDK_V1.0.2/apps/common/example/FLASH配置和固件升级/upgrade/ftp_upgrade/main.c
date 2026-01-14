#include "app_config.h"
#include "ftpserver/stupid-ftpd.h"
#include "net_update.h"
#include "wifi/wifi_connect.h"
#include "system/includes.h"

static void *stupid_vfs_open(char *path,  char *mode)
{
    printf("\n stupid_vfs_open : path is %s , mode is %s\n", path, mode);
    return net_fopen(path, mode);
}

static int stupid_vfs_write(void  *file, void  *buf, u32 len)
{
    return net_fwrite(file, buf, len, 0);
}

static int stupid_vfs_read(void  *file, void  *buf, u32 len)
{
    return net_fread(file, buf, len);
}

static int stupid_vfs_flen(void  *file)
{
    return net_flen(file);
}

static int stupid_vfs_close(void *file, char is_socket_err)
{
    return net_fclose(file, is_socket_err);
}

//注册一个ftpd的vfs接口
static void ftpd_vfs_interface_cfg(void)
{
    struct ftpd_vfs_cfg info;
    info.fopen = stupid_vfs_open;
    info.fwrite = stupid_vfs_write;
    info.fread = stupid_vfs_read;
    info.flen = stupid_vfs_flen;
    info.fclose = stupid_vfs_close;
	
	//注册接口到ftp中
    stupid_vfs_reg(&info);
}

static void ftp_upgrade_start(void *priv)
{
    enum wifi_sta_connect_state state;
	
    while(1)
    {
         printf("Connecting to the network...\n");
         state = wifi_get_sta_connect_state();
         if(WIFI_STA_NETWORK_STACK_DHCP_SUCC == state)
         {
            printf("Network connection is successful!\n");
            break;
         }
		 
		 os_time_dly(1000);
    }
	
	//注册读写操作集
	ftpd_vfs_interface_cfg();
	
	//这里设置用户名为：FTPX, 密码为：12345678
    stupid_ftpd_init("MAXUSERS=2\nUSER=FTPX 12345678     0:/      2   A\n", NULL);
}

void c_main(void *priv)
{
    if (thread_fork("ftp_upgrade_start", 10, 512, 0, NULL, ftp_upgrade_start, NULL) != OS_NO_ERR) {
        printf("thread fork fail\n");
    }
}

late_initcall(c_main);