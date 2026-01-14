#ifndef _WIFI_CONNECT_H_
#define _WIFI_CONNECT_H_

#include "generic/typedef.h"

enum WIFI_MODE {
    STA_MODE = 1,//STA_MODE位置必须为第一个
    AP_MODE,
    SMP_CFG_MODE,
    MP_TEST_MODE,
    P2P_MODE,
    NONE_MODE,
};


enum P2P_ROLE {
    P2P_GC_MODE = 1,
    P2P_GO_MODE,
};

struct wifi_store_info {
    enum WIFI_MODE mode;
    u8 pwd[2][64];
    u8 ssid[2][33];
    enum P2P_ROLE p2p_role;
    u8 sta_cnt;
    u8 	connect_best_network;
} __attribute__((packed));

struct wifi_stored_sta_info {
    u8 pwd[64];
    u8 ssid[33];
} __attribute__((packed));


struct wifi_scan_ssid_info {
    char ssid[32];
    unsigned int ssid_len;
    unsigned char mac_addr[6];
    int rssi;
    unsigned int channel_number;
    unsigned char	SignalStrength;//(in percentage)
    unsigned char	SignalQuality;//(in percentage)
    unsigned char   SupportedRates[16];
};


enum wifi_sta_connect_state {
    WIFI_STA_DISCONNECT,
    WIFI_STA_CONNECT_SUCC,
    WIFI_STA_CONNECT_TIMEOUT_NOT_FOUND_SSID,
    WIFI_STA_CONNECT_TIMEOUT_ASSOCIAT_FAIL,
    WIFI_STA_NETWORK_STACK_DHCP_SUCC,
    WIFI_STA_NETWORK_STACK_DHCP_TIMEOUT,
};

enum SMP_CFG_TYPE {
    PRIV_SMP_CFG1 = 1,
    PRIV_SMP_CFG2,
    AIRKISS_SMP_CFG,
};
struct smp_cfg_result {
    enum SMP_CFG_TYPE type;
    unsigned int ssid_crc;
    unsigned int random_val;
    char ssid[32];
    char passphrase[64];
};

enum WIFI_EVENT {
    WIFI_EVENT_MODULE_INIT,
    WIFI_EVENT_MODULE_START,
    WIFI_EVENT_MODULE_STOP,
    WIFI_EVENT_MODULE_START_ERR,

    WIFI_EVENT_AP_START,
    WIFI_EVENT_AP_STOP,
    WIFI_EVENT_STA_START,
    WIFI_EVENT_STA_STOP,
    WIFI_EVENT_STA_SCAN_COMPLETED,

    WIFI_EVENT_STA_CONNECT_SUCC,
    WIFI_EVENT_STA_CONNECT_TIMEOUT_NOT_FOUND_SSID,
    WIFI_EVENT_STA_CONNECT_TIMEOUT_ASSOCIAT_FAIL,
    WIFI_EVENT_STA_DISCONNECT,

    WIFI_EVENT_SMP_CFG_START,
    WIFI_EVENT_SMP_CFG_STOP,
    WIFI_EVENT_SMP_CFG_TIMEOUT,
    WIFI_EVENT_SMP_CFG_COMPLETED,

    WIFI_EVENT_STA_NETWORK_STACK_DHCP_SUCC,
    WIFI_EVENT_STA_NETWORK_STACK_DHCP_TIMEOUT,

    WIFI_EVENT_AP_ON_DISCONNECTED,
    WIFI_EVENT_AP_ON_ASSOC,

    WIFI_EVENT_MP_TEST_START,
    WIFI_EVENT_MP_TEST_STOP,

    WIFI_EVENT_P2P_START,
    WIFI_EVENT_P2P_STOP,
    WIFI_EVENT_P2P_GC_DISCONNECTED,
    WIFI_EVENT_P2P_GC_NETWORK_STACK_DHCP_SUCC,
    WIFI_EVENT_P2P_GC_NETWORK_STACK_DHCP_TIMEOUT,
    WIFI_EVENT_PM_SUSPEND,
    WIFI_EVENT_PM_RESUME,
    WIFI_FORCE_MODE_TIMEOUT,
};

struct wifi_mode_info {
    enum WIFI_MODE mode;
    char *ssid;
    char *pwd;
};


// @brief 配置WIFI MP测试后校准后的 晶体频偏参数 ,PA参数 , 各个速率功率的数字增益参数
struct wifi_calibration_param {
    u8 xosc_l;
    u8 xosc_r;
    u8 pa_trim_data[6];
    u8 mcs_dgain[20];
};
extern const struct wifi_calibration_param wifi_calibration_param;


struct ieee80211_frame {
    u8		i_fc[2];
    u8		i_dur[2];
    u8		i_addr1[6];
    u8		i_addr2[6];
    u8		i_addr3[6];
    u8		i_seq[2];
    /* possibly followed by addr4[6]; */
} __packed;

/**
   * @brief IEEE 802.11 management frame subtype definition
    */
#define IEEE80211_FC_STYPE_SHIFT        4
#define IEEE80211_FC_STYPE_MASK         (0xf << IEEE80211_FC_STYPE_SHIFT)
#define IEEE80211_FC_STYPE_ASSOC_REQ    (0x0 << IEEE80211_FC_STYPE_SHIFT)
#define IEEE80211_FC_STYPE_ASSOC_RESP   (0x1 << IEEE80211_FC_STYPE_SHIFT)
#define IEEE80211_FC_STYPE_REASSOC_REQ  (0x2 << IEEE80211_FC_STYPE_SHIFT)
#define IEEE80211_FC_STYPE_REASSOC_RESP (0x3 << IEEE80211_FC_STYPE_SHIFT)
#define IEEE80211_FC_STYPE_PROBE_REQ    (0x4 << IEEE80211_FC_STYPE_SHIFT)
#define IEEE80211_FC_STYPE_PROBE_RESP   (0x5 << IEEE80211_FC_STYPE_SHIFT)
#define IEEE80211_FC_STYPE_BEACON       (0x8 << IEEE80211_FC_STYPE_SHIFT)
#define IEEE80211_FC_STYPE_ATIM         (0x9 << IEEE80211_FC_STYPE_SHIFT)
#define IEEE80211_FC_STYPE_DISASSOC     (0xa << IEEE80211_FC_STYPE_SHIFT)
#define IEEE80211_FC_STYPE_AUTH         (0xb << IEEE80211_FC_STYPE_SHIFT)
#define IEEE80211_FC_STYPE_DEAUTH       (0xc << IEEE80211_FC_STYPE_SHIFT)
#define IEEE80211_FC_STYPE_ACTION       (0xd << IEEE80211_FC_STYPE_SHIFT)
#define IEEE80211_FC0_TYPE_MASK         0x0c
#define IEEE80211_FC0_TYPE_MGT          0x00
#define IEEE80211_FC0_TYPE_CTL          0x04
#define IEEE80211_FC0_TYPE_DATA         0x08
#define IEEE80211_FC0_SUBTYPE_MASK      0xf0

// @brief 注册WIFI事件回调函数
extern void wifi_set_event_callback(int (*cb)(void *, enum WIFI_EVENT));

// @brief 计算airkiss接收包crc校验
extern u8 wifi_airkiss_calcrc_bytes(u8 *p, unsigned int num_of_bytes);

// @brief 设置WIFI底层接收到数据包回调函数
extern void wifi_set_frame_cb(void (*cb)(void *rxwi, struct ieee80211_frame *header, void *data, void *reserve));

// @brief 设置WIFI 模拟功率等级, 0-6, 默认6
extern void wifi_set_pwr(unsigned char pwr_sel);

// @brief 启动WIFI
extern int wifi_on(void);

// @brief 查询WIFI是否启动
extern int wifi_is_on(void);

// @brief 关闭WIFI
extern int wifi_off(void);

// @brief 获取WIFI MAC地址
extern int wifi_get_mac(u8 *mac);

// @brief 设置WIFI MAC地址
extern int wifi_set_mac(char *mac_addr);

// @brief 设置WIFI接收过滤
extern void wifi_rxfilter_cfg(char mode);

// @brief 设置WIFI TX速率
extern void wifi_set_tx_rate_control_tab(u32 tab);

// @brief 获取WIFI当前信道
extern u32 wifi_get_channel(void);

// @brief 获取WIFI上行速率
extern u32 wifi_get_upload_rate(void);

// @brief 获取WIFI下行速率
extern u32 wifi_get_download_rate(void);

// @brief 获取WIFI当前是什么模式,或者当前指定模式下的配置信息
extern void wifi_get_mode_cur_info(struct wifi_mode_info *info);

// @brief 获取WIFI最后记忆的是什么模式,或者最后记忆的指定模式下的配置信息
extern int wifi_get_mode_stored_info(struct wifi_mode_info *info);

// @brief 设置WIFI启动默认模式配置
extern int wifi_set_default_mode(struct wifi_store_info *parm, char force, char store);

// @brief 保存WIFI模式配置信息,覆盖默认模式
extern int wifi_store_mode_info(enum WIFI_MODE mode, char *ssid, char *pwd);

// @brief 设置目标接收地址,有利于提高抗干扰能力,专门针对目标地址的数据包进行接收AGC和频偏调整
extern void wifi_set_target_mac_for_agc_recv(char *mac);


/*-------------------------------------AP MODE--------------------------------------*/
// @brief 设置WIFI进入AP模式
extern int wifi_enter_ap_mode(char *ap_ssid, char *ap_pwd);

// @brief  AP模式下获取接入的每个STA信号质量和MAC地址
extern int wifi_get_sta_entry_rssi(char wcid, char **rssi, u8 **evm, u8 **mac);

// @brief  AP模式下断开指定MAC地址的STA,reason 通常填8:Deauthenticated because sending station is leaving  */
extern void wifi_disconnect_station(char *mac, u16 reason);



/*-------------------------------------STA MODE--------------------------------------*/
// @brief 设置WIFI最多保存多少个连接的SSID数目
extern void wifi_set_store_ssid_cnt(u32 cnt);

// @brief 设置WIFI进入STA模式的时候如果匹配到信号最好的网络就去连接
extern void wifi_set_sta_connect_best_ssid(u8 enable);

// @brief 设置WIFI 连接STA超时时间
extern void wifi_set_sta_connect_timeout(int sec);

// @brief 设置WIFI 连接STA是否阻塞, 默认非阻塞通过事件通知
extern void wifi_set_connect_sta_block(int block);

// @brief 设置WIFI进入STA模式
extern int wifi_enter_sta_mode(char *sta_ssid, char *sta_pwd);

// @brief WIFI STA模式或者AP模式下启动一次扫描空中SSID
extern int wifi_sta_scan_once(void);

// @brief 启动一次扫描空中SSID后,获取扫描结果
extern struct wifi_scan_ssid_info *wifi_get_scan_result(u32 *ssid_cnt);

// @brief 获取扫描结果后,清空上次扫描结果
extern void wifi_clear_scan_result(void);

// @brief 获取WIFI保存过的SSID
extern int wifi_get_stored_sta_info(struct wifi_stored_sta_info wifi_stored_sta_info[]);

// @brief 获取WIFI连接的STA RSSI
extern char wifi_get_rssi(void);

// @brief 获取WIFI是连接STA状态
enum wifi_sta_connect_state wifi_get_sta_connect_state(void);




/*-------------------------------------MONITOR MODE--------------------------------------*/
// @brief 设置WIFI进入配网模式/monitor模式
extern int wifi_enter_smp_cfg_mode(void);

// @brief 设置WIFI配网超时事件时间
extern void wifi_set_smp_cfg_timeout(int sec);

// @brief 设置WIFI配网模式/monitor模式下是否扫描全部信道,默认否
extern void wifi_set_smp_cfg_scan_all_channel(char onoff);

// @brief 设置WIFI airkiss配网模式下是否接收完整的SSID,默认否,有助于加快配网时效,有概率配网失败
extern void wifi_set_smp_cfg_airkiss_recv_ssid(char onoff);

// @brief 设置WIFI进入配网/monitor模式后,是否只保留monitor模式,默认否
extern void wifi_set_smp_cfg_just_monitor_mode(char onoff);

// @brief 在WIFI只保留monitor模式的情况下,设置扫描每个信道的时间间隔
extern void wifi_set_monitor_mode_scan_channel_time(int time_ms);

// @brief 设置WIFI配网模式下airkiss的key
extern void wifi_set_airkiss_key(u8 key[16]);

// @brief 配网成功后获取WIFI配网信息
extern int wifi_get_cfg_net_result(struct smp_cfg_result *smp_cfg);

// @brief 设置WIFI当前信道
extern void wifi_set_channel(u8 ch);

// @brief 使用WIFI底层接口直接发送数据帧
enum wifi_tx_rate {
    WIFI_TXRATE_1M = 0,
    WIFI_TXRATE_2M,
    WIFI_TXRATE_5M,
    WIFI_TXRATE_6M,
    WIFI_TXRATE_7M,
    WIFI_TXRATE_9M,
    WIFI_TXRATE_11M,
    WIFI_TXRATE_12M,
    WIFI_TXRATE_14M,
    WIFI_TXRATE_18M,
    WIFI_TXRATE_21M,
    WIFI_TXRATE_24M,
    WIFI_TXRATE_28M,
    WIFI_TXRATE_36M,
    WIFI_TXRATE_43M,
    WIFI_TXRATE_48M,
    WIFI_TXRATE_54M,
    WIFI_TXRATE_57M,
    WIFI_TXRATE_65M,
    WIFI_TXRATE_72M,
};
extern void wifi_send_data(char *pkg, int len, enum wifi_tx_rate rate);

// @brief 设置WIFI底层长帧重传次数
extern void wifi_set_long_retry(u8  retry);
// @brief 设置WIFI底层短帧重传次数
extern void wifi_set_short_retry(u8  retry);

/**
 * @brief wifi_get_remain_tx_queue，用于获取WIFI底层发送队列剩余个数
 *
 * @param ac_queue 为Qos优先级,用户无配置就是填0
 *
 * @return 剩余多少个包可填充,最大为 MAX_PACKETS_IN_QUEUE
 */
extern u32 wifi_get_remain_tx_queue(char ac_queue);


#endif  //_WIFI_CONNECT_H_

