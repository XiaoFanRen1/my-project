#include "system/includes.h"
#include "lwip/sockets.h"
#include "lwip.h"
#include "lwip/ip4_addr.h"
#include "wifi/wifi_connect.h"
#include "lwip/tcp.h"
#include "lwip/sockets.h"
#include "lwip/inet.h"
#include "lwip/netdb.h"
#include "os/os_api.h"
#include "dev_update/fs_v2_update.h"
#include "lib_include.h"
#include "wifi/wifi_connect.h"
#include "stdlib.h"


#ifdef CONFIG_WL_AP_ENABLE
#define HTTP_UPDATE_ENABLE	0
#else
#define HTTP_UPDATE_ENABLE	1 //1，STA模式下http断点续传OTA升级
#endif

#if HTTP_UPDATE_ENABLE
#define HTTP_UPDATE_PORT	80			//http端口
#define HTTP_RECV_BUF		(4096*10)	//http升级缓存大小

/*#define HTTP_UPDATE_URL 	"http://test02.jieliapp.com/file/2022/update-ota.ufw"*/
#define HTTP_UPDATE_URL     "http://172.16.109.91:18888/update-ota.ufw"

#define HTTP_GET_HEAD		"GET %s HTTP/1.1\r\n"\
							"Host: %s\r\n"\
							"User-Agent: User\r\n"\
							"Accept: */*\r\n"\
							"Connection: Close\r\n"\
							"Range: bytes=%d-%d\r\n\r\n"
struct http_tcp {
    struct ip4_addr ip_addr;
    struct tcp_pcb *pcb;
    unsigned char conneted: 1;
    unsigned char lock: 7;
};
struct http_range_ota {
    struct http_tcp tcp;
    unsigned int file_size;//文件大小
    unsigned int content_length;
    unsigned int range_start;//断点续传起始
    unsigned int range_end;//断点续传结束
    unsigned int recv_offset;
    unsigned int read_offset;
    unsigned char *host;
    unsigned char *file;
    unsigned char *send_buf;
    unsigned int buf_size;
    unsigned char buf[0];
};
static struct http_range_ota *http_ota = NULL;
static char *http_url_path = NULL;
#endif


#define HTTP_DEV_DESC "HTTP/1.1 200 OK\r\nContent-Type:text\r\nContent-Length:454\r\nAccept-Ranges:bytes\r\nConnection:close\r\n\r\n{\"app_list\":{\"match_android_ver\":[\"1.0\",\"2.0\"],\"match_ios_ver\":[\"1.0\",\"2.0\"]},\"forward_support\":[\"0\"],\"behind_support\":[\"0\"],\"forward_record_support\":[\"0\"],\"behind_record_support\":[\"0\"],\"rtsp_forward_support\":[\"0\"],\"rtsp_behind_support\":[\"0\"],\"device_type\":\"1\",\"net_type\":\"1\",\"rts_type\":\"0\",\"product_type\":\"AC79x_wifi_camera\",\"support_projection\":\"0\",\"ota_type\":\"1\",\"ota_step\":\"1\",\"firmware_version\":\"1.0.1\",\"match_app_type\":\"DVRunning2\",\"uuid\":\"xxxxxx\"}"

#define CTP_PREFIX "CTP:"
#define CTP_PREFIX_LEN 4
#define CTP_TOPIC_LEN   2
#define CTP_TOPIC_CONTENT_LEN   4
#define CTP_KEEP_ALIVE_TOPIC        "CTP_KEEP_ALIVE"
#define CTP_KEEP_ALIVE_TOPIC_LEN    strlen(CTP_KEEP_ALIVE_TOPIC)
#define CTP_LOGIN_TOPIC             "CTP_LOGIN"
#define CTP_LOGIN_TOPIC_LEN         strlen(CTP_LOGIN_TOPIC)
#define CTP_APP_ACCESS				"APP_ACCESS"
#define CTP_APP_ACCESS_NOTIFY_START	"{\"op\":\"NOTIFY\",\"param\":{\"type\":\"0\",\"ver\":\"%d%s"
#define CTP_APP_ACCESS_NOTIFY_VER	20906
#define CTP_APP_ACCESS_NOTIFY_END	"\"}}"

#define CTP_CTRL_PORT       3333
#define NET_UPDATE_PORT		3335
#define HTTP_PORT			8080

#define NET_UPDATE_STATUS_NONE		(0 << 0)
#define NET_UPDATE_STATUS_OK		(1 << 0)
#define NET_UPDATE_STATUS_ERR		(1 << 1)
#define NET_UPDATE_STATUS_READ      (1 << 2)
#define NET_UPDATE_STATUS_STOP      (1 << 3)
#define NET_UPDATE_STATUS_SAME      (1 << 4)

#define NET_OTA_FILE_RESV		1 //文件夹预留读取：0文件往后读取， 1往前读取

enum {
    SEEK_SET = 0x01,
    SEEK_CUR = 0x02
};
static int msgclose(struct tcp_pcb *pcb);
static struct tcp_pcb *client_pcb = 0;
static char client_accept = 0;
static char http_accept = 0;
static char ctp_accept = 0;
static char net_ota_listen = 0;
static char net_ota_stop = 0;
static char net_recv_err = 0;
static char net_ota_lastpacket = 0;
static char net_ota_start = 0;
static unsigned int ota_all_size = 0;
static unsigned int ota_read_size = 0;

#define OS_SEM_TIME_MAX	500 //5s
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
#ifdef CONFIG_WL_AP_ENABLE
#define BUF_MAX_SIZE		(40*1024)
static u8 *ota_ram_addr 	= (u8 *)0x1C00000; //OTA缓存地址，长度42*1024,42K
#else
#define BUF_MAX_SIZE		(4*1024)
static u8 *ota_ram_addr 	= NULL;
#endif
static struct net_local *net_local_addr = NULL;

#define RECV_OFFSET_SIZE 	(sizeof(struct net_local))
#define RECV_BUF_MAX_SIZE 	(BUF_MAX_SIZE - sizeof(struct net_local) - sizeof(struct net_ota))//40K大小务必<42K
#define NET_MAX_SIZE 		(RECV_BUF_MAX_SIZE)
#define NET_TCP_OTA			(ota_ram_addr + sizeof(struct net_local))

extern char *vm_http_get(void);
extern enum wifi_sta_connect_state wifi_get_sta_connect_state(void);
extern u8 fs_update_result_transition(u8 err);
extern u8 ufw_head_check(u32 *update_part_flag);
extern u8 flash_bin_head_check(void);
extern u8 update_type_check(u32 *update_len);
extern u8 flash_update_process(void);
extern u8 flash_file_data_verify(void);
extern u8 flash_update_reserve_area(void);
extern u8 flash_all_data_verify(void);
extern u8 flash_parts_update_check(u32 update_part_flag);
extern void clr_update_loader_record(void);
extern u8 fs_update_judge_err_to_exit(u8 err);
extern int vm_data_recover(void);
extern unsigned int timer_get_ms();
int net_update_http_dev_desc(void);
int net_update_ctp_server(void);

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
int msgsenddata(u8 *buf, u32 len, u8 no_wait)//读取缓冲区的指定地址和长度
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
        os_sem_set(&read_sem, 0);
        net_ota->download_percent = net_local_addr->download_percent;
        net_ota->status = net_local_addr->status;//发送结果
        net_ota->sum_check = sumget((char *)net_ota + sizeof(net_ota->sum_check), sizeof(struct net_ota) - sizeof(net_ota->sum_check));
        err = send_msg(client_pcb, (u8 *)net_ota, len);
        if (read_size) {
            net_ota->read_size = read_size;
        }
        if (no_wait) {
            return err;
        }
        if (os_sem_pend(&read_sem, OS_SEM_TIME_MAX)) {
            return 0;
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
}
static err_t msgpoll(void *arg, struct tcp_pcb *pcb)
{
    return msgsend_buf(arg, pcb);
}
static err_t msgsent(void *arg, struct tcp_pcb *pcb, u16_t len)
{
    if (pcb->local_port == NET_UPDATE_PORT && net_ota_lastpacket) {
        net_ota_lastpacket = 0;
    }
    return msgsend_buf(arg, pcb);
}
static void msgerr_http(void *arg, err_t err)
{
    http_accept = http_accept > 0 ? --http_accept : http_accept;
    printf("HTTP err, http_accept = %d \n", http_accept);
}
static void msgerr_ctp(void *arg, err_t err)
{
    ctp_accept = ctp_accept > 0 ? --ctp_accept : ctp_accept;
    printf("CTP err, ctp_accept = %d \n", ctp_accept);
}
static void msgerr_ota(void *arg, err_t err)
{
    client_accept = client_accept > 0 ? --client_accept : client_accept;
    printf("OTA err, client_accept = %d \n", client_accept);
}
static int msgclose(struct tcp_pcb *pcb)
{
    if (pcb->local_port == NET_UPDATE_PORT) {
        client_accept = client_accept > 0 ? --client_accept : client_accept;
        if (client_accept == 0 || pcb == client_pcb) {
            client_pcb = NULL;
        }
    }
    if (pcb->local_port == CTP_CTRL_PORT) {
        ctp_accept = ctp_accept > 0 ? --ctp_accept : ctp_accept;
    }
#if HTTP_UPDATE_ENABLE
    http_ota->tcp.lock = 1;
#endif
    tcp_arg(pcb, NULL);
    tcp_sent(pcb, NULL);
    tcp_recv(pcb, NULL);
    tcp_err(pcb, NULL);
    tcp_abort(pcb);
    /*tcp_close(pcb);*/
#if HTTP_UPDATE_ENABLE
    if (http_ota && http_ota->tcp.pcb == pcb) {
        http_ota->tcp.pcb = NULL;
        http_ota->tcp.conneted = 0;
    }
    http_ota->tcp.lock = 0;
#endif
    return ERR_ABRT;
}
static err_t msgrecvmemcpy(void *arg, int len)
{
    struct net_local *net_local = (struct net_local *)(ota_ram_addr);
    struct net_ota *net_ota = (struct net_ota *)arg;

    if (net_ota->status == NET_UPDATE_STATUS_STOP) {
        net_local->status = net_ota->status;
        net_ota_stop = 1;
    } else if (net_local->read_size) {
        memcpy(net_local->buf, net_ota->buf + (net_local->seek_addr - net_ota->seek_addr), net_local->read_size);
    }
    os_sem_post(&read_sem);
    return ERR_OK;
}
static err_t msgrecv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
#if HTTP_UPDATE_ENABLE
    struct http_range_ota *httpota = (struct http_range_ota *)arg;
    if (!httpota) {
        return ERR_OK;
    }
    if (err == ERR_OK && p != NULL) {
        struct pbuf *q;
        char Find = 0;
        int tot_len = 0;
        char *pt = (char *)httpota->buf + httpota->recv_offset;
        int len = p->tot_len;
        if ((httpota->recv_offset + len) <= httpota->buf_size) {
            for (q = p; q != NULL; q = q->next) {
                memcpy(pt, q->payload, q->len);
                pt += q->len;
                tot_len += q->len;
            }
            tcp_recved(pcb, tot_len);
            /*printf("recv size : %d \n%s\n", tot_len, httpota->buf + httpota->recv_offset);*/
            if (!httpota->recv_offset) {
                char *p, *f, *c;
                p = (char *)httpota->buf + httpota->recv_offset;
                f = strstr(p, "HTTP/1.1 206");
                /*sdfile_save_test(p, tot_len, 0, 1);*/
                if (f) {
                    net_recv_err = 0;
                    c = strstr(f, "Content-Length:");
                    if (!c) {
                        c = strstr(f, "content-length:");
                    }
                    if (c) {
                        c += strlen("Content-Length:");
                        httpota->content_length = atoi(c);
                    }
                    c = strstr(f, "Content-Range:");
                    if (!c) {
                        c = strstr(f, "content-range:");
                    }
                    if (c) {
                        c += strlen("Content-Range:") + 1;
                        f = c;
                        while (c < (p + tot_len)) {
                            if (c[0] == '\r' && c[1] == '\n' && c[2] == '\r' && c[3] == '\n') {
                                c += 4;
                                Find = 1;
                                goto find;
                            }
                            if (c[0] == '\r' && c[1] == '\n') {
                                c += 2;
                                break;
                            }
                            if (c[0] == '/') {
                                httpota->file_size = atoi(++c);
                                /*printf("---> http ota file_size = %d \n", httpota->file_size);*/
                            }
                            c++;
                        }
                        f = strstr(f, "/");
                        if (f && f < c && !httpota->file_size) {
                            f += 1;
                            httpota->file_size = atoi(f);
                            /*printf("---> http ota file_size = %d \n",httpota->file_size);*/
                        }
                    } else {
                        c = f;
                    }
                    while (c < (p + tot_len)) {
                        if (c[0] == '\r' && c[1] == '\n' && c[2] == '\r' && c[3] == '\n') {
                            Find = 1;
                            c += 4;
                            break;
                        }
                        c++;
                    }
find:
                    if (Find) {
                        len = tot_len - (c - p);
                        memcpy(httpota->buf, c, len);
                        httpota->buf[len] = 0;
                        httpota->recv_offset += len;
                    } else {
                        printf("recv head data err\n");
                        goto err;
                    }
                } else {
err:
                    printf("recv err : %s \n", f);
                    net_recv_err++;
                    if (net_recv_err >= 3) {
                        net_recv_err = 0;
                        net_ota_stop = 1;
                        httpota->recv_offset = 0;
                    }
                    os_sem_post(&read_sem);
                }
            } else {
                httpota->recv_offset += tot_len;
            }
        } else {
            tcp_recved(pcb, p->tot_len);
        }
        pbuf_free(p);
        /*printf("recv size : %d, request : %d \n", httpota->recv_offset, httpota->range_end - httpota->range_start);*/
        if (httpota->recv_offset && (httpota->recv_offset >= (httpota->range_end - httpota->range_start) || (http_ota->recv_offset >= http_ota->file_size && http_ota->file_size))) {
            /*printf("---> recv size : %d, request : %d \n", httpota->recv_offset, httpota->range_end - httpota->range_start);*/

            if (net_local_addr->buf && net_local_addr->read_size) {
                memcpy(net_local_addr->buf, httpota->buf + (net_local_addr->seek_addr - httpota->range_start), net_local_addr->read_size);
            }
            os_sem_post(&read_sem);
        }
    }
#else
    struct net_ota *net_ota = (struct net_ota *)arg;
    if (!net_ota) {
        return ERR_OK;
    }
    if (err == ERR_OK && p != NULL) {
        struct pbuf *q;
        char *pt = (char *)net_ota;
        char First = 0;
        int tot_len = 0;
        if (p->tot_len > RECV_BUF_MAX_SIZE) {
            tcp_recved(pcb, p->tot_len);
            pbuf_free(p);
            return ERR_OK;
        }
        if (pcb->local_port == HTTP_PORT) {
            tcp_recved(pcb, p->tot_len);
            pbuf_free(p);
            send_msg(pcb, HTTP_DEV_DESC, strlen(HTTP_DEV_DESC));
            http_accept++;
            return ERR_OK;
        } else if (pcb->local_port == CTP_CTRL_PORT) {
            if (ctp_accept) {
                tcp_recved(pcb, p->tot_len);
                pbuf_free(p);
                return ERR_OK;
            }
            pt = (char *)net_ota->buf;
            char *ctp_msg = pt;
            for (q = p; q != NULL; q = q->next) {
                memcpy(pt, q->payload, q->len);
                pt += q->len;
                tot_len += q->len;
            }
            tcp_recved(pcb, tot_len);
            pbuf_free(p);
            ctp_msg[tot_len + 1] = 0;
            unsigned short topic_len = strlen(CTP_APP_ACCESS);
            unsigned int topic_ct_len = 0;
            unsigned int ctp_msg_len;
            unsigned int vers = 0;
            unsigned char *pver = strstr(ctp_msg, "ver\":\"");
            if (pver) {
                pver += strlen("ver\":\"");
                while (*pver >= '0' && *pver <= '9') {
                    vers = *pver - '0' + vers * 10;
                    pver++;
                }
                printf("APP ver = %d\n", vers);
            }
            memcpy(ctp_msg, CTP_PREFIX, CTP_PREFIX_LEN);
            memcpy(ctp_msg + CTP_PREFIX_LEN, &topic_len, CTP_TOPIC_LEN);
            memcpy(ctp_msg + CTP_PREFIX_LEN + CTP_TOPIC_LEN, CTP_APP_ACCESS, topic_len);
            memcpy(ctp_msg + CTP_PREFIX_LEN + CTP_TOPIC_LEN + topic_len, &topic_ct_len, CTP_TOPIC_CONTENT_LEN);
            unsigned char *content = ctp_msg + CTP_PREFIX_LEN + CTP_TOPIC_LEN + topic_len + CTP_TOPIC_CONTENT_LEN;
            int ret = sprintf(content, CTP_APP_ACCESS_NOTIFY_START, vers ? vers : CTP_APP_ACCESS_NOTIFY_VER, CTP_APP_ACCESS_NOTIFY_END);
            content[ret] = 0;
            topic_ct_len = strlen(content);
            memcpy(ctp_msg + CTP_PREFIX_LEN + CTP_TOPIC_LEN + topic_len, &topic_ct_len, CTP_TOPIC_CONTENT_LEN);
            ctp_accept++;
            send_msg(pcb, ctp_msg, CTP_PREFIX_LEN + CTP_TOPIC_LEN + topic_len + CTP_TOPIC_CONTENT_LEN + topic_ct_len);
            printf("ctp_accept notify = %s \n", content);
            return ERR_OK;
        } else {
            pt = (char *)net_ota + net_ota->all_size;
            for (q = p; q != NULL; q = q->next) {
                memcpy(pt, q->payload, q->len);
                pt += q->len;
                tot_len += q->len;
            }
            tcp_recved(pcb, tot_len);
            pbuf_free(p);
            net_ota->all_size += tot_len;
            if (net_ota->all_size >= (net_ota->read_size + sizeof(struct net_ota))) {
                net_ota->all_size = 0;
                if (sumcheck((char *)net_ota + sizeof(net_ota->sum_check), net_ota->read_size + sizeof(struct net_ota) - sizeof(net_ota->sum_check), net_ota->sum_check)) {
                    put_buf(net_ota, sizeof(struct net_ota));
                } else { //和校验成功
                    msgrecvmemcpy(net_ota, tot_len);
                }
            }
        }
    }
#endif
    else if (err == ERR_OK && p == NULL) {
        puts("msgrecv close tcp\n");
        return msgclose(pcb);
    }
    return ERR_OK;
}
static err_t msgaccept(void *arg, struct tcp_pcb *pcb, err_t err)
{
    if (pcb->local_port == NET_UPDATE_PORT) {
        client_accept++;
        if (client_accept > 1) {
            return msgclose(pcb);
        }
        client_pcb = pcb;
    }
    tcp_arg(pcb, NET_TCP_OTA);
    tcp_recv(pcb, msgrecv);
    tcp_sent(pcb, msgsent);
    tcp_err(pcb, pcb->local_port == NET_UPDATE_PORT ? msgerr_ota : (pcb->local_port == CTP_CTRL_PORT ? msgerr_ctp : msgerr_http));
    tcp_poll(pcb, msgpoll, 1);
    return ERR_OK;
}
#if HTTP_UPDATE_ENABLE
static err_t tcp_dataconnected(void *arg, struct tcp_pcb *pcb, err_t err)
{
    struct http_range_ota *httpota = (struct http_range_ota *)arg;
    tcp_recv(pcb, msgrecv);
    tcp_sent(pcb, msgsent);
    tcp_err(pcb, msgerr);
    httpota->tcp.conneted = 1;
    puts("connet server ok\n");
    return ERR_OK;
}
static int http_get(const char *url, char reset, int size)
{
    int port = 0;
    if (!url || !http_ota) {
        goto exit;
    }
    char *p = strstr(url, "http://");
    if (!p) {
        goto exit;
    }
    p += strlen("http://");
    char *e = strstr(p, ":");
    int len = 0;
    if (e) {
        len = e - p;
        if (len >= http_ota->buf_size) {
            goto exit;
        }
        e++;
        port = atoi(e);
        e = strstr(p, "/");
    } else {
        e = strstr(p, "/");
        if (e) {
            len = e - p;
            if (len >= http_ota->buf_size) {
                goto exit;
            }
        } else {
            http_ota->host = NULL;
            http_ota->file = NULL;
            http_ota->send_buf = NULL;
            http_ota->recv_offset = 0;
            http_ota->file_size = 0;
            goto exit;
        }
    }
    http_ota->host = http_ota->buf;
    memcpy(http_ota->host, p, len);
    http_ota->host[len] = 0;
    http_ota->send_buf = http_ota->host + len + 1;
    http_ota->file = NULL;
    http_ota->recv_offset = 0;
    if ((strlen(e) + len + 2) < http_ota->buf_size) {
        http_ota->file = http_ota->host + len + 1;
        len = strlen(e);
        memcpy(http_ota->file, e, len);
        http_ota->file[len] = 0;
        http_ota->send_buf = http_ota->file + len + 1;
    }
    if (!http_ota->tcp.ip_addr.addr || reset) {
        if (!atoi(http_ota->host)) {
            struct hostent *host_ent = gethostbyname(http_ota->host);
            if (!host_ent) {
                printf("  . ! lwip analayse ip err !!!\r\n");
                goto exit;
            }
            http_ota->tcp.ip_addr.addr = *((u32_t *)host_ent->h_addr);
        } else {
            http_ota->tcp.ip_addr.addr = inet_addr(http_ota->host);
        }
        printf("http update DNS remoute ip : %s  \n", inet_ntoa(http_ota->tcp.ip_addr.addr));
        http_ota->file_size = 0;
    }
    while (http_ota->tcp.lock) {
        os_time_dly(1);
    }
    http_ota->tcp.lock = 1;
    struct tcp_pcb *pcb = tcp_new();
    if (!pcb) {
        goto exit;
    }
    ip_addr_t ip;
    tcp_arg(pcb, http_ota);
    IP_SET_TYPE_VAL(ip, IPADDR_TYPE_V4);
    ip4_addr_copy(*ip_2_ip4(&ip), http_ota->tcp.ip_addr);
    http_ota->tcp.pcb = pcb;
    port = port ? port : HTTP_UPDATE_PORT;
    if (tcp_connect(pcb, &ip, port, tcp_dataconnected) != ERR_OK) {
        tcp_connect(pcb, &ip, 8080, tcp_dataconnected);
    }
    int cnt = 10 * 100;
    while (!http_ota->tcp.conneted && --cnt) {
        os_time_dly(10);
    }
    if (!cnt || net_ota_stop) {
        tcp_arg(pcb, NULL);
        tcp_sent(pcb, NULL);
        tcp_recv(pcb, NULL);
        tcp_err(pcb, NULL);
        tcp_close(pcb);
        http_ota->tcp.pcb = NULL;
        http_ota->tcp.ip_addr.addr = 0;
        goto exit;
    }
    size = size ? size : http_ota->buf_size;
    if (http_ota->file_size && (http_ota->range_start + http_ota->buf_size) > http_ota->file_size) {
        size = http_ota->file_size - http_ota->range_start;
    }
    http_ota->range_end = http_ota->range_start + size;
    if (http_ota->send_buf) {
        if (!http_ota->file) {
            http_ota->file = "/";
        }
        sprintf(http_ota->send_buf, HTTP_GET_HEAD, http_ota->file, http_ota->host, http_ota->range_start, http_ota->range_end - 1);
        /*printf("HTTP_GET_HEAD : \n%s\n", http_ota->send_buf);*/
        len = strlen(http_ota->send_buf);
        if (send_msg(pcb, http_ota->send_buf, len) != len) {
            goto exit;
        }
    }
    http_ota->tcp.lock = 0;
    return 0;

exit:
    http_ota->tcp.lock = 0;
    puts("http update err !!! \n");
    return -EINVAL;
}
static int http_read(void *priv, char reset)
{
    struct net_local *net_local = (struct net_local *)priv;
    if (!http_ota || !net_local) {
        return 0;
    }
    if (!http_ota->range_start && !http_ota->recv_offset) {
        http_ota->range_start = net_local->seek_addr;
    }
    if (!http_ota->recv_offset || net_local->seek_addr < http_ota->range_start ||
        ((net_local->seek_addr + net_local->read_size) > (http_ota->range_start + http_ota->recv_offset))) {
        http_ota->recv_offset = 0;
        http_ota->range_start = net_local->seek_addr;
    } else {
        memcpy(net_local->buf, http_ota->buf + (net_local->seek_addr - http_ota->range_start), net_local->read_size);
        return net_local->read_size;
    }
#if NET_OTA_FILE_RESV
    if (http_ota->range_start > (http_ota->buf_size - net_local->read_size)) {
        http_ota->range_start -= (http_ota->buf_size - net_local->read_size);
    }
#endif
    u8 cnt = 3;
redo:
    os_sem_set(&read_sem, 0);
    if (http_get(http_url_path, reset, 0)) {
        return 0;
    }
    if (os_sem_pend(&read_sem, OS_SEM_TIME_MAX)) {
        return 0;
    }
    if (net_recv_err && --cnt && !net_ota_stop) {
        goto redo;
    }
    return net_local->read_size ? net_local->read_size : http_ota->buf_size;
}
static int http_stop(int err)
{
    if (http_ota) {
        http_ota->tcp.lock = 1;
        if (http_ota->tcp.pcb) {
            tcp_arg(http_ota->tcp.pcb, NULL);
            tcp_sent(http_ota->tcp.pcb, NULL);
            tcp_recv(http_ota->tcp.pcb, NULL);
            tcp_err(http_ota->tcp.pcb, NULL);
            tcp_close(http_ota->tcp.pcb);
        }
        http_ota->tcp.lock = 0;
        http_ota->tcp.pcb = NULL;
        http_ota->tcp.ip_addr.addr = 0;
        http_ota->tcp.conneted = 0;
        memset(http_ota->buf, 0, http_ota->buf_size);
        memset(http_ota, 0, sizeof(struct http_range_ota));
    }
    return 0;
}
#endif
int net_f_open(int port)
{
#if HTTP_UPDATE_ENABLE
    int size = HTTP_RECV_BUF;
    if (!http_ota) {
        http_ota =  malloc(size);
    }
    if (http_ota) {
        memset(http_ota, 0, size);
        http_ota->buf_size = size - sizeof(struct http_range_ota);
        puts("http update init ok \n");
    } else {
        return false;
    }
    if (!os_sem_valid(&read_sem)) {
        os_sem_create(&read_sem, 0);
    }

    http_url_path = vm_http_get();
    http_url_path = http_url_path ? http_url_path : HTTP_UPDATE_URL;
    if (!net_local_addr) {
        net_local_addr = malloc(sizeof(struct net_local));
        if (!net_local_addr) {
            return false;
        }
    }
    memset(net_local_addr, 0, sizeof(struct net_local));
    net_local_addr->download_percent = 1;
    net_ota_stop = 0;
    if (!http_read((void *)net_local_addr, 1)) {
        puts("net_f_open err\n");
        return false;
    }
    puts("net_f_open\n");
#else
    int err = 0;
    int cnt = 5 * 10 * 60;//5分钟没有连接退出升级模式

    if (!ota_ram_addr) {
        ota_ram_addr = malloc(BUF_MAX_SIZE);
    }
    if (!ota_ram_addr) {
        return 0;
    }
    memset(ota_ram_addr, 0, BUF_MAX_SIZE);
    net_ota_stop = 0;

    if (net_ota_listen && port == NET_UPDATE_PORT) {
        puts("already listen\n");
        goto ota_wait;
    }
    struct tcp_pcb *pcb = tcp_new();
    if (!pcb) {
        return 0;
    }
    tcp_bind(pcb, IP_ADDR_ANY, port);
    pcb = tcp_listen(pcb);
    if (!pcb) {
        return 0;
    }
    tcp_accept(pcb, msgaccept);
    if (!os_sem_valid(&read_sem)) {
        os_sem_create(&read_sem, 0);
    }
    if (port == HTTP_PORT) {
        puts("HTTP server Listen\n");
    } else if (port == CTP_CTRL_PORT) {
        puts("CTP server Listen wait APP accept\n");
    } else if (port == NET_UPDATE_PORT) {
ota_wait:
        net_ota_listen = 1;
        puts("NET OTA wait accept\n");
        while (!client_accept && --cnt) {
            os_time_dly(10);
        }
        if (!http_accept) {
            puts("err device no get http dev_desc.txt\n");
            return 0;
        }
        if (!cnt) {
            puts("wait APP accept timeout !!!!\n");
            return 0;
        }
        net_local_addr = (struct net_local *)ota_ram_addr;
        net_local_addr->download_percent = 1;
        puts("APP accepted ok\n");
    }
#endif
    return 1;
}
int net_update_http_dev_desc(void)
{
    return net_f_open(HTTP_PORT);
}
int net_update_ctp_server(void)
{
    return net_f_open(CTP_CTRL_PORT);
}
int net_f_seek(void *p, u8 type, u32 offset)
{
#if HTTP_UPDATE_ENABLE
    if (!net_local_addr) {
        return -1;
    }
#else
    if (net_local_addr->status == NET_UPDATE_STATUS_STOP || net_ota_stop) {
        return -1;
    }
#endif
    if (type == SEEK_SET) {
        net_local_addr->seek_addr = offset;
    } else {
        net_local_addr->seek_addr += offset;
    }
    return 0;
}
int net_f_send_update_len(u32 len)//第一次发送本次升级的总字节
{
#if HTTP_UPDATE_ENABLE
    if (!net_local_addr || net_ota_stop || !len) {
        return -1;
    }
    net_local_addr->all_size = len;
    ota_all_size = len ? len : 0;
    ota_read_size = 0;
#else
    if (net_local_addr->status == NET_UPDATE_STATUS_STOP || net_ota_stop || !len) {
        return -1;
    }
    net_local_addr->seek_addr = 0;
    net_local_addr->read_size = 0;
    net_local_addr->all_size = len;
    net_local_addr->status = len ? 0 : NET_UPDATE_STATUS_SAME;
    net_local_addr->resv = 0;
    ota_all_size = len;
    ota_read_size = 0;
    msgsenddata(net_local_addr, sizeof(struct net_ota), 0);
#endif
    return 0;
}
int net_f_read(void *p, void *buf, u32 len)
{
    int err;
    putchar('@');
#if HTTP_UPDATE_ENABLE
    int cnt = 3;
    if (!net_local_addr || net_ota_stop) {
        printf("err ota read\n");
        return 0;
    }
    if (ota_all_size) {
        u8 percent = (unsigned long long)ota_read_size * 100 / ota_all_size;
        u8 last_percent = net_local_addr->download_percent;
        net_local_addr->download_percent = percent > net_local_addr->download_percent ? percent : net_local_addr->download_percent;
        net_local_addr->download_percent = net_local_addr->download_percent < 100 ? net_local_addr->download_percent : 99;
        if (net_local_addr->download_percent != last_percent) {
            printf("update >> %d%%\n", net_local_addr->download_percent);
        }
    }
    net_local_addr->read_size = len;
    net_local_addr->resv = 0;
    net_local_addr->buf = buf;
redo:
    cnt--;
    if (!http_read((void *)net_local_addr, 0)) {
        if (cnt) {
            goto redo;
        }
        puts("http_read err\n");
        return 0;
    }
#else
    if (len > NET_MAX_SIZE) {
        return -1;
    }
    net_local_addr->read_size = len;
    net_local_addr->status = NET_UPDATE_STATUS_READ;
    net_local_addr->resv = 0;
    net_local_addr->buf = buf;
    net_local_addr->all_size = 0;
    if (ota_all_size) {
        u8 percent = (unsigned long long)ota_read_size * 100 / ota_all_size;
        net_local_addr->download_percent = percent > net_local_addr->download_percent ? percent : net_local_addr->download_percent;
        net_local_addr->download_percent = net_local_addr->download_percent < 100 ? net_local_addr->download_percent : 99;
    }
    if (net_local_addr->status == NET_UPDATE_STATUS_STOP || net_ota_stop) {
        return -1;
    }
    if (!msgsenddata(net_local_addr, sizeof(struct net_ota), 0)) {
        puts("net_f_read err\n");
        return -1;
    }
    if (net_local_addr->status == NET_UPDATE_STATUS_STOP || net_ota_stop) {
        return -1;
    }
#endif
    if (ota_all_size) {
        ota_read_size += net_local_addr->read_size;
    }
    return net_local_addr->read_size;
}

int net_f_stop(void *p)
{
#if HTTP_UPDATE_ENABLE
    if (!net_local_addr) {
        return -1;
    }
    http_stop((int)p);
    memset(net_local_addr, 0, sizeof(struct net_local));
#else
    memset(net_local_addr, 0, sizeof(struct net_local));
    if (client_pcb) {
        net_local_addr->read_size = 0;
        if (!p) {
            net_local_addr->status = 0;//发送结果
            if (ota_all_size) {
                net_local_addr->download_percent = 100;
            }
            msgsenddata(net_local_addr, sizeof(struct net_ota), 0);
            os_time_dly(50);
        }
        net_ota_lastpacket = 1;
        net_local_addr->status = p ? NET_UPDATE_STATUS_ERR : NET_UPDATE_STATUS_OK;//发送结果
        msgsenddata(net_local_addr, sizeof(struct net_ota), 1);
        u32 cnt = 200;
        while (net_ota_lastpacket && --cnt) {
            os_time_dly(10);
        }
        if (!cnt) {
            puts("net_f_stop send packet timeout \n\n");
        }
        net_local_addr->status = 0;
        net_local_addr->download_percent = 0;
    }
    os_time_dly(50);
    if (os_sem_valid(&read_sem)) {
        os_sem_del(&read_sem, 0);
    }
    if (client_pcb) {
        msgclose(client_pcb);
    }
    struct net_ota *net_ota = (struct net_ota *)(NET_TCP_OTA);
    memset(net_ota, 0, sizeof(struct net_ota));
    net_local_addr = NULL;
#endif
    net_ota_stop = 0;
    ota_all_size = 0;
    ota_read_size = 0;
    return 0;
}
int net_ota_update_read_size_check(void)
{
    if (!net_ota_start) {
        return 0;
    }
    if (net_ota_stop) {
        return -1;
    }
    return ota_read_size ? ota_read_size : 1;
}
void net_update_result_handle(u8 err)
{
    u8 res;
    switch (err) {
    case UPDATE_ERR_NONE:
        break;
    case UPDATE_ERR_KEY_ERR:
        break;
    default:
        res = fs_update_result_transition(err);
        break;
    }
    net_f_stop((void *)res);
}


int net_update_loop(int port)//注意这个函数的任务调用，函数栈不能在icach或dcach区域，否则升级使用spi局部变量在cach区域死机
{
    u8 res = 0;
    u16 r_len;
    u32 update_part_flag;

    if (!net_f_open(port)) {
        res = -1;
        return res;
    }
    net_ota_start = 1;
    ///ufw文件头公共信息检阅
    puts("ufw_check\n");
    res = ufw_head_check(&update_part_flag);
    if (UPDATE_ERR_NONE != res) {
        puts("---ufw_head_check err\n");
        goto _LOOP_RET;
    } else {
        /*puts("---ufw_head_check ok\n");*/
    }
    printf("update_part_flag:%x\n", update_part_flag);

    if (UPDATE_PART_ALL_FLASH == update_part_flag) {
        res = flash_bin_head_check();
        if (UPDATE_ERR_NONE != res) {
            puts("---flash_bin_head_chek err\n");
            goto _LOOP_RET;
        } else {
            /*puts("---flash_bin_head_chek ok\n");*/
        }
    } else {
        res = flash_parts_update_check(update_part_flag);
        if (UPDATE_ERR_NONE != res) {
            puts("---flash_part update err\n");
        } else {
            puts("---flash_part update succ\n");
        }
        goto _LOOP_RET;
    }

    u32 update_len = 0;
    //检查ufw文件的各个文件,将要升级的区域记录在链表中;
    res = update_type_check(&update_len);
#if ALL_FLASH_INCLUDE_UBOOT_UPDATE_EN
    if ((UPDATE_ERR_NONE != res) && (UPDATE_ERR_UBOOT_NOT_MATCH != res)) {
#else
    if (UPDATE_ERR_NONE != res) {
#endif
        printf("--update_type_check_err\n", res);
        //net_update_result_handle(res);
        goto _LOOP_RET;
    } else {
        puts("---update_type_check ok\n");
    }

    //获取是否整个flash升级标志
    u8 all_flash_update_flag = (res == UPDATE_ERR_UBOOT_NOT_MATCH) ? 1 : 0;

    //通知测试盒升级内容总长度
    printf("\n\n---update_len:%x\n\n", update_len);
    net_f_send_update_len(update_len);
    if (!update_len) { //长度为0则是同一个固件
        res = UPDATE_RESULT_REMOTE_FILE_NOT_MATCH;
        printf("---flash update same all, no need to update\n");
        goto _LOOP_RET;
    }

    //开始擦除响应区域并升级代码(todo:代码校验)
    res = flash_update_process();
    if (UPDATE_ERR_NONE == res) {
        /*puts("---flash update succ\n");*/
    } else {
        printf("---flash update fail:%x\n", res);
        goto _LOOP_RET;
    }

    if (all_flash_update_flag) {
        //flash全校验
        res = flash_all_data_verify();
        if (UPDATE_ERR_NONE == res) {
            /*puts("----flash all data verify succ\n");*/
        } else {
            puts("----flash all data verify fail\n");
            goto _LOOP_RET;
        }
    } else {
        //升级区域校验
        //res = flash_file_data_verify();
        res = flash_all_data_verify();
        if (UPDATE_ERR_NONE == res) {
            /*puts("----flash verify succ\n");*/
        } else {
            printf("---flash verify fail:%x\n", res);
            goto _LOOP_RET;
        }
    }

    res = flash_update_reserve_area();
    if (UPDATE_ERR_NONE == res) {
        puts("---update_reserve_area ok\n");
    } else {
        printf("---update_reserve_area fail:%x\n", res);
    }
    vm_data_recover();//vm数据备份区恢复

_LOOP_RET:
    net_ota_start = 0;
    /*puts("UPDATA_LOOP_RET\n");*/
    net_update_result_handle(res);
    if (UPDATE_ERR_NONE == res) {
        puts("net_update succ ok\n");
        return 0;
    } else if (fs_update_judge_err_to_exit(res)) {
        clr_update_loader_record();
        return -1;
    } else {
        return -2;
    }
}
void net_ota_wifi_scan(int clear)
{
#if HTTP_UPDATE_ENABLE
    static u8 scan_cnt = 0;
    if (clear) {
        scan_cnt = 0;
    }
    int state = wifi_get_sta_connect_state();
    printf("state = %d ,scan_cnt = %d\n", state, scan_cnt);
    if (state != WIFI_STA_NETWORK_STACK_DHCP_SUCC) {
        scan_cnt++;
        if (scan_cnt > 20) {//连续扫描20次没有连接路由器，重启
            puts("wifi scan err , cpu_reset\n");
            cpu_reset();
        }
    }
#endif
}
void net_ota_update(void)
{
    int ret = 0;
    int port = NET_UPDATE_PORT;
    int cnt = 0;
#if HTTP_UPDATE_ENABLE
    do {
        cnt++;
        os_time_dly(10);
        int state = wifi_get_sta_connect_state();
        switch (state) {
        case WIFI_STA_NETWORK_STACK_DHCP_SUCC:
            ret = 1;
            cnt = 0;
            break;
        case WIFI_STA_CONNECT_TIMEOUT_NOT_FOUND_SSID:
            puts("NOT_FOUND_SSID\n");
        case WIFI_STA_CONNECT_ASSOCIAT_TIMEOUT:
            cnt = 0;
            break;
        default:
            break;
        }
    } while (cnt && cnt < 120 * 100);
    cnt = 3;
    if (ret) {
        printf("---> http url : %s \n", vm_http_get());
    }
    os_time_dly(100);//偶现的连上立马创建tcp无法连接服务器问题
#else
    os_time_dly(300);
    ret = net_update_ctp_server();
    ret |= net_update_http_dev_desc();
    cnt = 0xFFFFFFF;
#endif
    if (ret) {
        puts("=========net update doing ========\n");
        while (--cnt) {
            ret = net_update_loop(port);
            os_time_dly(50);
            if (!ret) {
                puts("=========net update ok ========\n");
                void wifi_all_sta_discon(void);
                wifi_all_sta_discon();//断开所有STA
                os_time_dly(100);
                cpu_reset();
            } else {
                puts("========= net update err =========\n");
            }
        }
    }
    puts("cpu_reset\n");
    os_time_dly(100);
    cpu_reset();
    while (1) {
        os_time_dly(100);
    }
}

