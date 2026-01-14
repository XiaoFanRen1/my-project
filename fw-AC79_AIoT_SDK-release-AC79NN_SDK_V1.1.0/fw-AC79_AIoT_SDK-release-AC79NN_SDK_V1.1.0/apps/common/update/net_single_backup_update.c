#include "system/includes.h"
#include "app_config.h"
#include "update/update.h"
#include "fs/fs.h"
#include "update/update_loader_download.h"
#include "generic/errno-base.h"
#include "lwip/ip4_addr.h"
#include "wifi/wifi_connect.h"
#include "lwip/tcp.h"
#include "lwip/sockets.h"
#include "lwip/inet.h"
#include "lwip/netdb.h"

//========================网络单备份系统升级例子================================
#if (CONFIG_DOUBLE_BANK_ENABLE == 0)
#if 0
enum {
    SEEK_SET = 0x01,
    SEEK_CUR = 0x02
};
#endif

//升级端口
#define NET_UPDATE_PORT     3335


//升级状态
#define NET_UPDATE_STATUS_DOING     (0 << 0)//设备端主动发送
#define NET_UPDATE_STATUS_OK        (1 << 0)//设备端主动发送
#define NET_UPDATE_STATUS_ERR       (1 << 1)//设备端主动发送
#define NET_UPDATE_STATUS_READ      (1 << 2)//设备端主动发送
#define NET_UPDATE_STATUS_STOP      (1 << 3)//APP主动的停止升级
#define NET_UPDATE_STATUS_SAME      (1 << 4)//升级的是同一个固件

#define NET_OTA_FILE_RESV       0 //文件夹预留读取：0文件往后读取， 1往前读取

static struct tcp_pcb *client_pcb = 0;
static char client_accept = 0;
static char net_ota_stop = 0;
static OS_SEM read_sem;

//协议保持和APP和ota_loard一致
//协议：头部20字节
struct net_ota {
    unsigned int sum_check;//设备端：头部16字节和校验，APP端：头部16字节+读取字节的和校验，（sum_check不加上和校验计算）
    unsigned int seek_addr;//文件偏移地址
    unsigned int read_size;//读取文件长度
    unsigned int all_size;//升级过程中需要的文件总字节数（用在进度条），整个升级过程中会有一次的all_size > 0, 其他次all_size = 0
    unsigned int status: 8; //升级转态，NET_UPDATE_STATUS_NONE正在升级，NET_UPDATE_STATUS_OK升级成功，NET_UPDATE_STATUS_ERR升级失败
    unsigned int download_percent: 8; //升级过程中设备的百分比
    unsigned int resv: 16; //预留，最后收到这4个字节代表成功和失败
    unsigned char buf[0];
};
struct net_local { //除了buf以外，其他成员保持和struct net_ota一致
    unsigned int sum_check;
    unsigned int seek_addr;
    unsigned int read_size;
    unsigned int all_size;
    unsigned int status: 8; //预留，最后发送这4个字节代表成功和失败
    unsigned int download_percent: 8; //升级过程中设备的百分比
    unsigned int resv: 16; //预留，最后发送这4个字节代表成功和失败
    unsigned char *buf;//存留升级fread的地址
};
static u8 *ota_ram_addr = NULL; //OTA缓存地址，长度42*1024,42K
static struct net_local *net_local_addr = NULL;
static u32 ota_loard_size = 0;
static u32 ota_read_size = 0;

#define NET_OTA_BUF_SIZE	(20*1024) //务必<=40K，和APP相关
#define RECV_OFFSET_SIZE 	(sizeof(struct net_local))
#define RECV_BUF_MAX_SIZE 	(NET_OTA_BUF_SIZE - sizeof(struct net_local) - sizeof(struct net_ota))
#define NET_MAX_SIZE 		(RECV_BUF_MAX_SIZE)
#define NET_TCP_OTA			(ota_ram_addr + sizeof(struct net_local))


static unsigned int sumget(void *buf, int len)
{
    unsigned char *pbuf = (unsigned char *)buf;
    unsigned int sum = 0;
    int i = 0;
    while (i < len) {
        sum += pbuf[i++];
    }
    return sum;
}
static int sumcheck(void *buf, int len, unsigned int sum)
{
    unsigned char *pbuf = (unsigned char *)buf;
    unsigned int sum_check = 0;
    int i = 0;
    while (i < len) {
        sum_check += pbuf[i++];
    }
    if (sum_check != sum) {
        printf("err sumcheck = 0x%x, 0x%x\n", sum_check, sum);
        return -1;
    }
    return 0;
}
static int send_msg(struct tcp_pcb *pcb, u8 *buf, u32 len)
{
    err_t err;
    int size = tcp_sndbuf(pcb);
    if (size < len) {
        printf("err tcp no buf to send , %d , %d\n", size, len);
        return 0;
    }
    size = MIN(size, len);
    err = tcp_write(pcb, buf, size, 1);
    if (err != ERR_OK) {
        puts("tcp_write err \n");
        return 0;
    }
    return size;
}
int msgsenddata(u8 *buf, u32 len)
{
    int err = 0;
    struct net_local *net_local = (struct net_local *)(buf);
    struct net_ota *net_ota = (struct net_ota *)(NET_TCP_OTA);
    if (len > RECV_OFFSET_SIZE) {
        return 0;
    }
    if (client_pcb) {
        int tcp_size = RECV_BUF_MAX_SIZE;
        u32 read_size = 0;
        if (net_local_addr->status == NET_UPDATE_STATUS_READ) {
            if (!net_local->all_size) {
                if (!net_ota->seek_addr && !net_ota->read_size) {
                    net_ota->seek_addr = net_local->seek_addr;
                }
                if (!net_ota->read_size || net_local->seek_addr < net_ota->seek_addr ||
                    ((net_local->seek_addr + net_local->read_size) > (net_ota->seek_addr + net_ota->read_size))) {
                    net_ota->all_size = 0;
                    net_ota->read_size = tcp_size;
                    net_ota->seek_addr = net_local->seek_addr;
                } else {
                    memcpy(net_local->buf, net_ota->buf + (net_local->seek_addr - net_ota->seek_addr), net_local->read_size);
                    os_sem_post(&read_sem);
                    return net_local->read_size;
                }
#if NET_OTA_FILE_RESV
                if (net_ota->seek_addr > (RECV_BUF_MAX_SIZE - net_local->read_size)) {
                    net_ota->seek_addr -= (RECV_BUF_MAX_SIZE - net_local->read_size);
                }
#endif
            } else {
                net_ota->all_size = net_local->all_size;
            }
        } else {
            read_size = net_ota->read_size;
            net_ota->read_size = 0;
            net_ota->all_size = net_local->all_size ? net_local->all_size : net_ota->all_size;
        }
        net_ota->download_percent = net_local_addr->download_percent;
        net_ota->status = net_local_addr->status;
        net_ota->sum_check = sumget((char *)net_ota + sizeof(net_ota->sum_check), sizeof(struct net_ota) - sizeof(net_ota->sum_check));
        err = send_msg(client_pcb, (u8 *)net_ota, len);
        if (read_size) {
            net_ota->read_size = read_size;
        }
    }
    return err;
}
static err_t msgsend_buf(void *arg, struct tcp_pcb *pcb)
{
    return ERR_OK;
}
static void msgerr(void *arg, err_t err)
{
    /*printf("---> %s err = %d \n",__FUNCTION__,err);*/
}
static err_t msgpoll(void *arg, struct tcp_pcb *pcb)
{
    return msgsend_buf(arg, pcb);
}
static err_t msgsent(void *arg, struct tcp_pcb *pcb, u16_t len)
{
    return msgsend_buf(arg, pcb);
}
static void msgclose(struct tcp_pcb *pcb)
{
    if (pcb) {
        if (client_accept > 0) {
            client_accept--;
        }
        if (client_accept == 0 || client_pcb == pcb) {
            client_pcb = NULL;
        }
        tcp_arg(pcb, NULL);
        tcp_sent(pcb, NULL);
        tcp_recv(pcb, NULL);
        tcp_err(pcb, NULL);
        tcp_close(pcb);
    }
}
static err_t msgrecvmemcpy(void *arg, int len)
{
    struct net_local *net_local = (struct net_local *)(ota_ram_addr);
    struct net_ota *net_ota = (struct net_ota *)arg;

    if (net_ota->status == NET_UPDATE_STATUS_STOP) {
        net_local->status = net_ota->status;
        net_ota_stop = 1;
    } else {
        memcpy(net_local->buf, net_ota->buf + (net_local->seek_addr - net_ota->seek_addr), net_local->read_size);
    }
    os_sem_post(&read_sem);
    return ERR_OK;
}
static err_t msgrecv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
    struct net_ota *net_ota = (struct net_ota *)arg;
    if (!net_ota || !ota_ram_addr) {
        return ERR_OK;
    }
    if (err == ERR_OK && p != NULL) {
        struct pbuf *q;
        char First = 0;
        int tot_len = 0;
        char *pt = (char *)net_ota + net_ota->all_size;
        for (q = p; q != NULL; q = q->next) {
            memcpy(pt, q->payload, q->len);
            pt += q->len;
            tot_len += q->len;
        }
        /*tcp_recved(pcb, p->tot_len);*/
        tcp_recved(pcb, tot_len);
        pbuf_free(p);
        net_ota->all_size += tot_len;
        if (net_ota->all_size >= (net_ota->read_size + sizeof(struct net_ota))) {
            net_ota->all_size = 0;
            if (sumcheck((char *)net_ota + sizeof(net_ota->sum_check), net_ota->read_size + sizeof(struct net_ota) - sizeof(net_ota->sum_check), net_ota->sum_check)) {
            } else { //和校验成功
                msgrecvmemcpy(net_ota, tot_len);
            }
        }
    } else if (err == ERR_OK && p == NULL) {
        puts("msgrecv close tcp\n");
        msgclose(pcb);
    }
    return ERR_OK;
}
static err_t msgaccept(void *arg, struct tcp_pcb *pcb, err_t err)
{
    client_accept++;
    if (client_accept > 1) {
        msgclose(pcb);
        printf("client_accept = %d \n", client_accept);
    } else {
        client_pcb = pcb;
        tcp_arg(pcb, NET_TCP_OTA);
        tcp_recv(pcb, msgrecv);
        tcp_sent(pcb, msgsent);
        tcp_err(pcb, msgerr);
        tcp_poll(pcb, msgpoll, 1);
        int net_ota_single_back_update(void);
        sys_timeout_add(NULL, net_ota_single_back_update, 100);
        puts("client accept ok\n");
    }
    return ERR_OK;
}
int net_ota_single_back_init(void)
{
    int err = 0;
    int cnt = 0;
    if (ota_ram_addr) {
        printf("net_ota_single_back_init already\n");
        return 0;
    }
    if (!ota_ram_addr) {
        ota_ram_addr = malloc(NET_OTA_BUF_SIZE);
        if (!ota_ram_addr) {
            printf("net_ota_single_back_init err , no mem\n");
            return -1;
        }
    }
    memset(ota_ram_addr, 0, NET_OTA_BUF_SIZE);
    struct tcp_pcb *pcb = tcp_new();
    if (!pcb) {
        printf("tcp_new err\n");
        return -1;
    }
    tcp_bind(pcb, IP_ADDR_ANY, NET_UPDATE_PORT);
    pcb = tcp_listen(pcb);
    if (!pcb) {
        tcp_close(pcb);
        printf("tcp_new err\n");
        return -1;
    }
    tcp_accept(pcb, msgaccept);
    os_sem_create(&read_sem, 0);
    puts("net_ota_open, wait APP accept .....\n");
    return 0;
}
static u16 net_ota_single_back_fs_open(void)
{
    int cnt = 500;//5秒
    if (!ota_ram_addr) {
        return false;
    }
    while (!client_pcb && --cnt) {
        os_time_dly(10);
    }
    if (!cnt) { //等待APP连接超时
        return false;
    }
    os_sem_set(&read_sem, 0);
    //清空数据
    net_local_addr = (struct net_local *)ota_ram_addr;
    struct net_ota *net_ota = (struct net_ota *)(NET_TCP_OTA);
    memset(net_ota, 0, sizeof(struct net_ota));
    memset(net_local_addr, 0, sizeof(struct net_local));
    net_local_addr->download_percent = 1;
    printf("net_ota open ok\n");
    return true;
}
static u16 net_ota_single_back_fs_read(void *fp, u8 *buff, u16 len)
{
    if (!net_local_addr || !ota_ram_addr) {
        printf("err ota read\n");
        return 0;
    }
    int err;
    if (len > NET_MAX_SIZE) {
        printf("err ota read size\n");
        return 0;
    }
    net_local_addr->read_size = len;
    net_local_addr->status = NET_UPDATE_STATUS_READ;
    net_local_addr->resv = 0;
    net_local_addr->buf = buff;
    net_local_addr->all_size = 0;
    if (ota_loard_size) {
        u8 percent = (unsigned long long)ota_read_size * 100 / ota_loard_size;
        net_local_addr->download_percent = percent > net_local_addr->download_percent ? percent : net_local_addr->download_percent;
        net_local_addr->download_percent = net_local_addr->download_percent < 100 ? net_local_addr->download_percent : 99;
    }
    if (net_local_addr->status == NET_UPDATE_STATUS_STOP || net_ota_stop) {
        printf("err ota read net_ota_stop\n");
        return 0;
    }
    msgsenddata(net_local_addr, sizeof(struct net_ota));
    err = os_sem_pend(&read_sem, 300);

    if (net_local_addr->status == NET_UPDATE_STATUS_STOP || net_ota_stop) {
        printf("err ota read net_ota_stop\n");
        return 0;
    }
    if (err) {
        puts("net_f_read err\n");
        return 0;
    }
    net_local_addr->seek_addr += net_local_addr->read_size;
    if (ota_loard_size) {
        ota_read_size += net_local_addr->read_size;
    }
    return net_local_addr->read_size;
}
static int net_ota_single_back_fs_seek(void *fp, u8 type, u32 offset)
{
    if (!net_local_addr || !ota_ram_addr) {
        return (int) - 1;
    }
    if (net_local_addr->status == NET_UPDATE_STATUS_STOP || net_ota_stop) {
        return 0;
    }
    if (type == SEEK_SET) {
        net_local_addr->seek_addr = offset;
    } else {
        net_local_addr->seek_addr += offset;
    }
    //printf("---> seek_addr = 0x%X \n",net_local_addr->seek_addr);
    return 0;
}
static u16 net_ota_single_back_fs_stop(u8 err)
{
    if (!net_local_addr || !ota_ram_addr) {
        return (int) - 1;
    }
    memset(net_local_addr, 0, sizeof(struct net_local));
    if (client_pcb) {
        net_local_addr->read_size = 0;
        if (!err) { //先发送100%
            net_local_addr->status = 0;//发送结果
            if (ota_loard_size) {
                net_local_addr->download_percent = 100;
            }
            msgsenddata(net_local_addr, sizeof(struct net_ota));
            os_time_dly(50);
        }
        net_local_addr->status = err ? NET_UPDATE_STATUS_ERR : NET_UPDATE_STATUS_OK;//发送结果
        msgsenddata(net_local_addr, sizeof(struct net_ota));
        net_local_addr->status = 0;
        net_local_addr->download_percent = 0;
    }
    os_time_dly(50);
    net_local_addr = NULL;
    if (client_pcb) {
        puts("client_pcb close\n");
        msgclose(client_pcb);
    }
    //清空数据
    struct net_ota *net_ota = (struct net_ota *)(NET_TCP_OTA);
    memset(net_ota, 0, sizeof(struct net_ota));
    net_ota_stop = 0;
    ota_loard_size = 0;
    ota_read_size = 0;
    puts("net_ota_fs_stop ok\n");
    return true;
}
static int net_ota_single_back_fs_need_size(void *p, u32 len)//第一次发送本次升级的总字节
{
    if (net_local_addr->status == NET_UPDATE_STATUS_STOP || net_ota_stop) {
        return -1;
    }
    net_local_addr->seek_addr = 0;
    net_local_addr->read_size = 0;
    net_local_addr->all_size = len;
    net_local_addr->status = len ? 0 : NET_UPDATE_STATUS_SAME;
    net_local_addr->resv = 0;
    ota_loard_size = len ? len : 0;
    ota_read_size = 0;
    msgsenddata(net_local_addr, sizeof(struct net_ota));
    return 0;
}

static const update_op_api_t net_ota_single_back_fs_update_opt = {
    .f_open = net_ota_single_back_fs_open,
    .f_read = net_ota_single_back_fs_read,
    .f_seek = net_ota_single_back_fs_seek,
    .f_stop = net_ota_single_back_fs_stop,
    .notify_update_content_size = net_ota_single_back_fs_need_size,
};

//单备份升级
static void net_ota_single_back_fs_update_param_private_handle(UPDATA_PARM *p)
{
}
static void net_ota_single_back_fs_update_state_cbk(int type, u32 state, void *priv)
{
    update_ret_code_t *ret_code = (update_ret_code_t *)priv;
    switch (state) {
    case UPDATE_CH_EXIT:
        if ((0 == ret_code->stu) && (0 == ret_code->err_code)) {
            UPDATA_PARM *update_ram = UPDATA_FLAG_ADDR;
            memset(update_ram, 0, 32);
            UPDATA_PARM *p = UPDATA_FLAG_ADDR + sizeof(UPDATA_PARM);
            if (support_dual_bank_update_en) {
                memset(update_ram, 0, sizeof(UPDATA_PARM));
                update_ram->magic = type;
                update_result_set(UPDATA_SUCC);
                printf(">>>>>>>>>>>>>>>>>>update ok , cpu reset ...\n");
            } else {
                //单备份
                update_mode_api_v2(type, net_ota_single_back_fs_update_param_private_handle, NULL);
            }
            void wifi_all_sta_discon(void);
            wifi_all_sta_discon();//断开所有连接STA
            os_time_dly(200);
            system_reset();
        } else {
            update_result_set(UPDATA_DEV_ERR);
            printf("\nupdate err !!! \n");
        }
        break;
    }
}

//单备份网络升级例子
int net_ota_single_back_update(void)
{
    printf("\n>>>>>>>>>>>>net_ota_single_back_update\n");
    update_mode_info_t info = {
        .type = NET_UFW_UPDATA,
        .state_cbk = net_ota_single_back_fs_update_state_cbk,
        .p_op_api = &net_ota_single_back_fs_update_opt,
        .task_en = 1,
    };
    app_active_update_task_init(&info);
    return 0;
}
#endif



