#include <string.h>
#include "generic/typedef.h"
#include "app_config.h"
#include "asm/crc16.h"
#include "database.h"
#include "syscfg/syscfg_id.h"
#include "lwip.h"


/*
// 设置WIFI DEBUG 信息输出等级
Debug information verbosity: lower values indicate higher urgency
0:RT_DEBUG_OFF
1:RT_DEBUG_ERROR
2:RT_DEBUG_WARN
3:RT_DEBUG_TRACE
4:RT_DEBUG_INFO
5:RT_DEBUG_LOUD
*/
const u8 RTDebugLevel = 2;

const char WL_TX_DEBUG = 0; //WIFI底层发送数据FIFO繁忙打印
const char WL_RX_DEBUG = 0; //WIFI底层接收FIFO塞满导致丢包打印

const char WL_TX_ERR_RATIO_DEBUG_SEC = 0; //统计每秒TX数据包成功/失败/重发和错误率的情况,配置每隔多少秒打印一次

const char WL_RX_ERR_RATIO_DEBUG_SEC = 0; //统计每秒RX数据包成功/失败/和错误率的情况,配置每隔多少秒打印一次
const u16 WL_RX_BACK_GROUND_ERR_CNT_PER_SECOND = 10; //硬件少了寄存器统计接收正确包,所以统计正确包和误报率不准,人工设定一个当前环境干扰每秒钟的本底错误包作为弥补,一开始需要先观察打印确认

const u8 WL_TX_PEND_DEBUG_SEC = 2; //WIFI底层FIFO塞满导致连续多少秒都发送不出数据时打印, 一般认为是干扰严重/wifi板TX性能差/CPU被挡等因素导致

const char WL_RX_PEND_DEBUG_SEC = 2; //统计WIFI底层连续多少秒都接收不到空中包时打印,一般认为是进了屏蔽房/加了MAC层过滤/板子硬件性能太差/CPU太繁忙来接收线程来不及取数因素导致
const char WL_RX_OVERFLOW_DEBUG = 0; //统计WIFI底层接收FIFO塞满导致丢包打印,一般认为对端发送太猛/空中干扰太强/CPU太繁忙来接收线程来不及取数因素导致, 使能后如果出现丢包打印每秒丢多少个数据包

const u8 RxReorderEnable = 0; //过滤wifi 重复数据帧，0为关闭，1为开启

const u16 MAX_CHANNEL_TIME_BSS_INFRA = 200;//扫描每个信道停留时间,单位ms,最小20ms, 200-400ms最佳

#if defined  CONFIG_NO_SDRAM_ENABLE
const u16 MAX_PACKETS_IN_QUEUE = 16; //配置WiFi驱动最大发送数据包队列
const u16 MAX_PACKETS_IN_MCAST_PS_QUEUE	= 4;	//配置WiFi驱动最大发送数据包队列 //8	16 modify by lyx 32
const u16 MAX_PACKETS_IN_PS_QUEUE	=	2;	//配置WiFi驱动最大发送数据包队列  //128	/*16 */
#else
const u16 MAX_PACKETS_IN_QUEUE = 64; //配置WiFi驱动最大发送数据包队列
const u16 MAX_PACKETS_IN_MCAST_PS_QUEUE = 8;  //配置WiFi驱动最大发送MCAST-power-save包队列 //modify by lyx 32
const u16 MAX_PACKETS_IN_PS_QUEUE	= 16; //配置WiFi驱动最大发送power-save队列	//128	/*16 */
#endif

const u8 ntp_get_time_init = 1;	//连上网后调用ntp向ntp_host列表所有服务器获取时间, 0为关闭, 1为开启

/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

static void print_debug_ipv4(u32 daddr, u32 saddr)
{
    put_buf(&daddr, 4);
    put_buf(&saddr, 4);
    printf("daddr : %s\n", inet_ntoa(daddr));
    printf("saddr : %s\n", inet_ntoa(saddr));
}

//用于根据LWIP接收队列溢出情况下快速丢包减轻CPU负担,预留空间接收重要数据包
int lwip_low_level_inputput_filter(u8 *pkg, u32 len)
{
    static const u8 bc_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
#define ipv4addr_ismulticast(addr) ((addr & PP_HTONL(0xf0000000UL)) == PP_HTONL(0xe0000000UL))

    //根据LWIP接收缓存情况快速丢包减轻CPU负担,并且防止丢失重要重要数据包
    struct iphdr_e *iph = (struct iphdr_e *)(pkg + 10);
    u16 protoType = ntohs(iph->h_proto);

    /*------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#define ICMP_ER   0    /* echo reply */
#define ICMP_ECHO 8    /* echo */
    if ((protoType != 0x0806) && iph->iphd.protocol == 1 && *((u8 *)iph + sizeof(struct iphdr_e)) != ICMP_ER && *((u8 *)iph + sizeof(struct iphdr_e)) != ICMP_ECHO) { //如果是ICMP并且不是echo和echo reply就丢弃, 因为是一些Time_Ex等无用包
        putbyte('I');
        return -1;
    }
    /*------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#define PBUF_RESERVED_FOR_ARP 1 //最起码预留一个 PBUF_POOL给ARP请求,否则不回复路由器导致断流
#define PBUF_RESERVED_FOR_TCP (PBUF_RESERVED_FOR_ARP+1)
    u32 remain_pbuf_pool = memp_get_pbuf_pool_free_cnt();
    if (remain_pbuf_pool <= PBUF_RESERVED_FOR_TCP) {
        printf(">>>>>>>>>>>%d\n", remain_pbuf_pool);
        if (remain_pbuf_pool == 0) { //PBUF_POOL 一个也没了,直接丢弃
            putbyte('X');
            return -1;
        } else if (remain_pbuf_pool <= PBUF_RESERVED_FOR_ARP && protoType != 0x0806) {//PBUF_POOL 在剩余小于 PBUF_RESERVED_FOR_ARP 的情况下, 丢弃非ARP包
            putbyte('Y');
            return -1;
        } else if (!(protoType == 0x0800 && iph->iphd.protocol == 6)) { //PBUF_POOL 在剩余小于 PBUF_RESERVED_FOR_TCP 的情况下, 丢弃非TCP包
            putbyte('D');
            return -1;
        }
    }
    /*------------------------------------------------------------------------------------------------------------------------------------------------------------*/
    return 0;
}

//在WIFI底层发送队列不足的情况, 预留空间给重要数据包发送
int lwip_low_level_output_filter(u8 *pkg, u32 len)
{
    struct iphdr_e *iph = (struct iphdr_e *)(pkg + 10);
    u16 protoType = ntohs(iph->h_proto);


    u32 remain_wifi_txq = wifi_get_remain_tx_queue(0);

#define WIFI_TXQ_RESERVED_FOR_ARP 1 //最起码预留一个WIFI_TXQ给ARP请求,否则不回复路由器导致断流
#define WIFI_TXQ_RESERVED_FOR_TCP (WIFI_TXQ_RESERVED_FOR_ARP+1)

    if (remain_wifi_txq <= WIFI_TXQ_RESERVED_FOR_TCP) {
        if (remain_wifi_txq == 0) { //WIFI_TXQ 一个也没了,直接丢弃
            putbyte('H');
            return -1;
        } else if (remain_wifi_txq <= WIFI_TXQ_RESERVED_FOR_ARP && protoType != 0x0806) {//WIFI_TXQ 在剩余小于 WIFI_TXQ_RESERVED_FOR_ARP 的情况下, 丢弃非ARP包
            putbyte('J');
            return -1;
        } else if (!(protoType == 0x0800 && iph->iphd.protocol == 6)) { //WIFI_TXQ 在剩余小于 WIFI_TXQ_RESERVED_FOR_TCP 的情况下, 丢弃非TCP包
            putbyte('K');
            return -1;
        }
    }
    return 0;
}

//空中干扰/网络拥塞严重的情况,WIFI底层发送较慢,MAX_PACKETS_IN_QUEUE 队列满导致丢弃上层数据包
void socket_send_but_netif_busy_hook(int s, char type_udp)
{
    if (type_udp) {
        putbyte('$');
        /* os_time_dly(30); //根据实际应用发送情况调节, 针对UDP多释放一下CPU, 一方面有利于系统其他线程顺畅运行, 另一方面防止猛发送导致网络拥塞加剧 */
    } else {
        putbyte('|');
        /* os_time_dly(2); */
    }
}

int wifi_recv_pkg_and_soft_filter(u8 *pkg, u32 len)  //通过软件过滤无用数据帧减轻cpu压力,pkg[20]就是对应抓包工具第一个字节的802.11 MAC Header 字段
{
#if 0
    static u32 thdll, count;
    int ret;
    ret = time_lapse(&thdll, 1000);
    if (ret) {
        printf("sdio_recv_cnt = %d,  %d \r\n", ret, count);
        count = 0;
    }
    ++count;
#endif

#if 0 //调试用

    static const u8 bc_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
#define ipv4addr_ismulticast(addr) ((addr & PP_HTONL(0xf0000000UL)) == PP_HTONL(0xe0000000UL))

    struct ieee80211_frame *wh = (struct ieee80211_frame *)&pkg[20];
    if ((wh->i_fc[0] & IEEE80211_FC0_TYPE_MASK) == IEEE80211_FC0_TYPE_DATA) {// 只丢弃数据帧
        if (pkg[54] == 0X88 && pkg[55] == 0x8E) {} else //如果是EAPOL不要丢弃
            if (len >= (52 + sizeof(struct iphdr_e))) {
                u16 protoType;
                struct iphdr_e *iph;
                if (0 == memcmp(bc_mac, wh->i_addr1, 6)) {
                    //广播
                    iph = (struct iphdr_e *)((u8 *)pkg + 48);
                } else {
                    iph = (struct iphdr_e *)((u8 *)pkg + 52);
                }
                protoType = ntohs(iph->h_proto);

                if (protoType == 0x0800) { //ipv4
                    if (iph->iphd.protocol == 17) { //udp
                        if (ipv4addr_ismulticast(iph->iphd.daddr)) { //组播
                            /*putbyte('M');*/
                        } else {
                            /*putbyte('U');*/
                        }

                        /* print_debug_ipv4(iph->iphd.daddr, iph->iphd.saddr); */

                    } else if (iph->iphd.protocol == 6) { //tcp
                        /*putbyte('T');*/
                        /* print_debug_ipv4(iph->iphd.daddr, iph->iphd.saddr); */
                    }
                } else if (protoType == 0x0806) { //arp
                    /*putbyte('R');*/
                }
            }
    }
#endif

    return 0;
}
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#if 0
WirelessMode:
PHY_11BG_MIXED = 0,
PHY_11B = 1,
PHY_11A = 2,,
PHY_11ABG_MIXED = 3,,
PHY_11G = 4,
PHY_11ABGN_MIXED,	/* both band   5 */
PHY_11N_2_4G,		/* 11n-only with 2.4G band      6 */
PHY_11GN_MIXED,		/* 2.4G band      7 */
PHY_11AN_MIXED,		/* 5G  band       8 */
PHY_11BGN_MIXED,	/* if check 802.11b.      9 */
PHY_11AGN_MIXED,	/* if check 802.11b.      10 */
PHY_11N_5G,		/* 11n-only with 5G band                11 */


#endif

/*
 MaxStaNum  最大连接数不能超过 MAX_LEN_OF_MAC_TABLE(5)
 */
static char WLAP_DAT[] = {
    "\
#The word of \"Default\" must not be removed\n\
Default\n\
MacAddress=00:00:00:00:00:00\n\
CountryRegion=1\n\
CountryRegionABand=0\n\
CountryCode=CN\n\
BssidNum=1\n\
MaxStaNum=2\n\
IdleTimeout=300\n\
SSID=####SSID_LENTH_MUST_LESS_THAN_32\n\
WirelessMode=9\n\
TxRate=0\n\
Channel=11#\n\
BasicRate=15\n\
BeaconPeriod=100\n\
DtimPeriod=1\n\
TxPower=100\n\
DisableOLBC=0\n\
BGProtection=0\n\
TxAntenna=\n\
RxAntenna=\n\
TxPreamble=1\n\
RTSThreshold=2347\n\
FragThreshold=2346\n\
TxBurst=0\n\
PktAggregate=0\n\
TurboRate=0\n\
WmmCapable=0\n\
APSDCapable=0\n\
DLSCapable=0\n\
APAifsn=3;7;1;1\n\
APCwmin=4;4;3;2\n\
APCwmax=6;10;4;3\n\
APTxop=0;0;94;47\n\
APACM=0;0;0;0\n\
BSSAifsn=3;7;2;2\n\
BSSCwmin=4;4;3;2\n\
BSSCwmax=10;10;4;3\n\
BSSTxop=0;0;94;47\n\
BSSACM=0;0;0;0\n\
AckPolicy=0;0;0;0\n\
NoForwarding=0\n\
NoForwardingBTNBSSID=0\n\
HideSSID=0\n\
StationKeepAlive=0\n\
ShortSlot=1\n\
AutoChannelSelect=0\n\
IEEE8021X=0\n\
IEEE80211H=0\n\
CSPeriod=10\n\
WirelessEvent=0\n\
IdsEnable=0\n\
AuthFloodThreshold=32\n\
AssocReqFloodThreshold=32\n\
ReassocReqFloodThreshold=32\n\
ProbeReqFloodThreshold=32\n\
DisassocFloodThreshold=32\n\
DeauthFloodThreshold=32\n\
EapReqFooldThreshold=32\n\
PreAuth=0\n\
AuthMode=################\n\
EncrypType=################\n\
RekeyInterval=0\n\
RekeyMethod=DISABLE\n\
PMKCachePeriod=10\n\
WPAPSK=#########wpa_passphrase_lenth_must_more_than_7_and_less_than_63\n\
DefaultKeyID=1\n\
Key1Type=0\n\
Key1Str=\n\
Key2Type=0\n\
Key2Str=\n\
Key3Type=0\n\
Key3Str=\n\
Key4Type=0\n\
Key4Str=\n\
HSCounter=0\n\
AccessPolicy0=0\n\
AccessControlList0=\n\
AccessPolicy1=0\n\
AccessControlList1=\n\
AccessPolicy2=0\n\
AccessControlList2=\n\
AccessPolicy3=0\n\
AccessControlList3=\n\
WdsEnable=0\n\
WdsEncrypType=NONE\n\
WdsList=\n\
WdsKey=\n\
RADIUS_Server=192.168.2.3\n\
RADIUS_Port=1812\n\
RADIUS_Key=ralink\n\
own_ip_addr=192.168.5.234\n\
EAPifname=br0\n\
PreAuthifname=br0\n\
HT_HTC=0\n\
HT_RDG=1\n\
HT_EXTCHA=0\n\
HT_LinkAdapt=0\n\
HT_OpMode=0\n\
HT_MpduDensity=4\n\
HT_BW=0\n\
HT_BADecline=0\n\
HT_AutoBA=1\n\
HT_AMSDU=0\n\
HT_BAWinSize=64\n\
HT_GI=0\n\
HT_MCS=33\n\
MeshId=MESH\n\
MeshAutoLink=1\n\
MeshAuthMode=OPEN\n\
MeshEncrypType=NONE\n\
MeshWPAKEY=\n\
MeshDefaultkey=1\n\
MeshWEPKEY=\n\
WscManufacturer=\n\
WscModelName=\n\
WscDeviceName=\n\
WscModelNumber=\n\
WscSerialNumber=\n\
RadioOn=1\n\
PMFMFPC=0\n\
PMFMFPR=0\n\
PMFSHA256=0"
};



/*PSMode=CAM\n\*/
/*PSMode=Legacy_PSP\n\//STA休眠*/
#ifdef CONFIG_LOW_POWER_ENABLE
#define WL_STA_SLEEP	1
#else
#define WL_STA_SLEEP	0
#endif

static const char WL_STA_DAT[] = {
    "\
#The word of \"Default\" must not be removed\n\
Default\n\
MacAddress=00:00:00:00:00:00\n\
CountryRegion=1\n\
CountryRegionABand=0\n\
CountryCode=CN\n\
ChannelGeography=1\n\
SSID=DEFAULT_CONNECT_SSID\n\
NetworkType=Infra\n\
WirelessMode=9\n\
Channel=1\n\
BeaconPeriod=100\n\
TxPower=100\n\
BGProtection=0\n\
TxPreamble=1\n\
RTSThreshold=2347\n\
FragThreshold=2346\n\
TxBurst=0\n\
PktAggregate=0\n\
WmmCapable=1\n\
AckPolicy=0;0;0;0\n\
AuthMode=OPEN\n\
EncrypType=NONE\n\
WPAPSK=\n\
DefaultKeyID=1\n\
Key1Type=0\n\
Key1Str=\n\
Key2Type=0\n\
Key2Str=\n\
Key3Type=0\n\
Key3Str=\n\
Key4Type=0\n\
Key4Str=\n"
#if WL_STA_SLEEP
    "PSMode=Legacy_PSP\n"
#else
    "PSMode=CAM\n"
#endif
    "AutoRoaming=0\n\
RoamThreshold=70\n\
APSDCapable=1\n\
APSDAC=0;0;0;0\n\
HT_RDG=1\n\
HT_EXTCHA=0\n\
HT_OpMode=0\n\
HT_MpduDensity=4\n\
HT_BW=0\n\
HT_BADecline=0\n\
HT_AutoBA=1\n\
HT_AMSDU=0\n\
HT_BAWinSize=64\n\
HT_GI=0\n\
HT_MCS=33\n\
HT_MIMOPSMode=3\n\
HT_DisallowTKIP=1\n\
HT_STBC=0\n\
EthConvertMode=\n\
EthCloneMac=\n\
IEEE80211H=0\n\
TGnWifiTest=0\n\
WirelessEvent=0\n\
MeshId=MESH\n\
MeshAutoLink=1\n\
MeshAuthMode=OPEN\n\
MeshEncrypType=NONE\n\
MeshWPAKEY=\n\
MeshDefaultkey=1\n\
MeshWEPKEY=\n\
CarrierDetect=0\n\
AntDiversity=0\n\
BeaconLostTime=16\n\
FtSupport=0\n\
Wapiifname=ra0\n\
WapiPsk=\n\
WapiPskType=\n\
WapiUserCertPath=\n\
WapiAsCertPath=\n\
PSP_XLINK_MODE=0\n\
WscManufacturer=\n\
WscModelName=\n\
WscDeviceName=\n\
WscModelNumber=\n\
WscSerialNumber=\n\
RadioOn=1\n\
WIDIEnable=0\n\
P2P_L2SD_SCAN_TOGGLE=8\n\
Wsc4digitPinCode=0\n\
P2P_WIDIEnable=0\n\
PMFMFPC=0\n\
PMFMFPR=0\n\
PMFSHA256=0"
};

const char *GET_WL_STA_DAT(void)
{
    return WL_STA_DAT;
}

int GET_WL_STA_DAT_LEN(void)
{
    return strlen(WL_STA_DAT);
}

const char *GET_WL_AP_DAT(void)
{
    return WLAP_DAT;
}

int GET_WL_AP_DAT_LEN(void)
{
    return strlen(WLAP_DAT);
}



int wl_set_wifi_channel(int channel)
{
    char channel_str[3] = {0};

    if (channel < 1 || channel > 14) {
        return -1;
    }

    sprintf(channel_str, "%d", channel);

    const char *find_channel = "Channel=";
    char *channel_position = strstr(GET_WL_AP_DAT(), find_channel) + strlen(find_channel);

    strcpy(channel_position, channel_str);
    channel_position[strlen(channel_str)] = '\n';
    memset(channel_position + strlen(channel_str) + 1, '#', 3 - strlen(channel_str) - 1);

    return 0;
}

static int wl_set_ssid(const char *ssid)
{
    if (ssid == 0) {
        return -1;
    }

    const char *find_ssid = "SSID=";
    char *ssid_position = strstr(GET_WL_AP_DAT(), find_ssid) + strlen(find_ssid);

    if (strlen(ssid) > 32) {
        printf("set_ssid_passphrase fail,ssid len (0x%lx) longer than 32!\n", strlen(ssid));
        return -1;
    }

    strcpy(ssid_position, ssid);
    ssid_position[strlen(ssid)] = '\n';
    memset(ssid_position + strlen(ssid) + 1, '#', 32 - strlen(ssid) - 1);

    return 0;
}

static int wl_set_passphrase(const char *passphrase)
{
    if (passphrase == 0) {
        return -1;
    }

    const char *AUTH_MODE, *ENCRYP_TYPE;
    const char *find_wpa_passphrase = "WPAPSK=";
    const char *find_AuthMode = "AuthMode=";
    const char *find_EncrypType = "EncrypType=";

    char *wpa_passphrase_position = strstr(GET_WL_AP_DAT(), find_wpa_passphrase) + strlen(find_wpa_passphrase);
    char *AuthMode_position = strstr(GET_WL_AP_DAT(), find_AuthMode) + strlen(find_AuthMode);
    char *EncrypType_position = strstr(GET_WL_AP_DAT(), find_EncrypType) + strlen(find_EncrypType);

    if (strcmp(passphrase, "")) {
        if ((strlen(passphrase) < 8) || (strlen(passphrase) > 63)) {
            printf("set_ssid_passphrase fail,passphrase len (0x%lx) must more than 7 and less than 63!\n", strlen(passphrase));
            return -1;
        }

        strcpy(wpa_passphrase_position, passphrase);
        wpa_passphrase_position[strlen(passphrase)] = '\n';
        memset(wpa_passphrase_position + strlen(passphrase) + 1, '#', 63 - strlen(passphrase) - 1);

        AUTH_MODE = "WPA2PSK";
        strcpy(AuthMode_position, AUTH_MODE);
        AuthMode_position[strlen(AUTH_MODE)] = '\n';
        memset(AuthMode_position + strlen(AUTH_MODE) + 1, '#', 16 - strlen(AUTH_MODE) - 1);

        ENCRYP_TYPE = "AES";
        strcpy(EncrypType_position, ENCRYP_TYPE);
        EncrypType_position[strlen(ENCRYP_TYPE)] = '\n';
        memset(EncrypType_position + strlen(ENCRYP_TYPE) + 1, '#', 16 - strlen(ENCRYP_TYPE) - 1);
    } else {
        AUTH_MODE = "OPEN";
        strcpy(AuthMode_position, AUTH_MODE);
        AuthMode_position[strlen(AUTH_MODE)] = '\n';
        memset(AuthMode_position + strlen(AUTH_MODE) + 1, '#', 16 - strlen(AUTH_MODE) - 1);

        ENCRYP_TYPE = "NONE";
        strcpy(EncrypType_position, ENCRYP_TYPE);
        EncrypType_position[strlen(ENCRYP_TYPE)] = '\n';
        memset(EncrypType_position + strlen(ENCRYP_TYPE) + 1, '#', 16 - strlen(ENCRYP_TYPE) - 1);
    }

    return 0;
}



int wl_ap_init(const char *ssid, const char *passphrase)
{
    int ret;
    wl_set_ssid(ssid);
    wl_set_passphrase(passphrase);

//    printf("WLAP_DAT = \r\n %s \r\n",GET_WL_AP_DAT());

    return 0;
}


#if 1
struct pmk_info {
    u32 crc;
    char pmk[32];
};
int get_stored_pmk(char *password, char *ssid, int ssidlength, char *output)
{
    int ret;
    u32 crc;
    struct pmk_info info;

    /* ret = db_select_buffer(WIFI_PMK_INFO, (char *)&info, sizeof(struct pmk_info)); */
    ret = syscfg_read(WIFI_PMK_INFO, (char *)&info, sizeof(struct pmk_info));
    if (ret < 0) {
        return 0;
    }

    crc = CRC16(password, strlen(password));
    crc += CRC16(ssid, ssidlength);

    if (crc != info.crc) {
        return 0;
    }

    memcpy(output, info.pmk, sizeof(info.pmk));
    printf("stored pmk_info match[%s] [%s],,,\n", ssid, password);
    return 1;
}
void set_stored_pmk(char *password, char *ssid, int ssidlength, char *output)
{
    printf("pmk_info not match, store... [%s] [%s],,,\n", ssid, password);

    struct pmk_info info;
    info.crc = CRC16(password, strlen(password));
    info.crc += CRC16(ssid, ssidlength);
    memcpy(info.pmk, output, sizeof(info.pmk));
    /* db_update_buffer(WIFI_PMK_INFO, (char *)&info, sizeof(struct pmk_info)); */
    syscfg_write(WIFI_PMK_INFO, (char *)&info, sizeof(struct pmk_info));
}

#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int __attribute__((weak)) get_foce_channel(void)
{
    return 0;
}

struct wifi_channel_info {
    u32 crc;
    char channel;
};
static struct wifi_channel_info s_wifi_channel_info;
static u8 sw_ch_cnt;
int get_stored_channel(char *ssid, int ssidlength, char *channel)
{
    if (get_foce_channel()) {
        *channel = get_foce_channel();
        return 1;
    }

#if NETWORK_SSID_INFO_CNT > 1 //如果需要扫描空中所有SSID才能够确定连接哪一个wifi就不使用
    return 0;
#endif

    int ret;
    u32 crc;
    struct wifi_channel_info info;

    if (!s_wifi_channel_info.channel && !s_wifi_channel_info.crc) {
        /* ret = db_select_buffer(WIFI_CHANNEL_INFO, (char *)&info, sizeof(struct wifi_channel_info)); */
        ret = syscfg_read(WIFI_CHANNEL_INFO, (char *)&info, sizeof(struct wifi_channel_info));
        if (ret < 0) {
            return 0;
        }
        crc = CRC16(ssid, ssidlength);
        if (crc != info.crc) {
            return 0;
        }
        s_wifi_channel_info.crc = crc;
        s_wifi_channel_info.channel = info.channel;
        *channel = s_wifi_channel_info.channel;
        printf("stored wifi_channel match[%s] [%d],,,\n", ssid, *channel);
        return 1;
    }

    if (s_wifi_channel_info.channel && s_wifi_channel_info.crc != CRC16(ssid, ssidlength)) {
        return 0;
    }

    if (sw_ch_cnt > 8) { // 如果多次扫描不到WIFI信号,恢复扫描所有信道
        return 0;
    }
    ++sw_ch_cnt;

    *channel = s_wifi_channel_info.channel;
    return 1;
}

#if NETWORK_SSID_INFO_CNT <= 1 //如果需要扫描空中所有SSID才能够确定连接哪一个wifi就不使用
void set_stored_wifi_channel(char *ssid, int ssidlength, char channel)
{
    if (s_wifi_channel_info.channel != channel) {
        printf("wifi_channel store..[%s][%d],,,\n", ssid, channel);
        struct wifi_channel_info info;
        info.crc = CRC16(ssid, ssidlength);
        info.channel = channel;
        /* db_update_buffer(WIFI_CHANNEL_INFO, (char *)&info, sizeof(struct wifi_channel_info)); */
        syscfg_write(WIFI_CHANNEL_INFO, (char *)&info, sizeof(struct wifi_channel_info));
        s_wifi_channel_info.channel = channel;
        s_wifi_channel_info.crc = info.crc;
    }
    sw_ch_cnt = 0;
}
#endif



