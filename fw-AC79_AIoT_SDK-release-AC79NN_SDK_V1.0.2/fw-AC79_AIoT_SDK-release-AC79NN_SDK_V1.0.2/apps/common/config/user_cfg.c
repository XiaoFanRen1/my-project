#include "app_config.h"
#include "fs/fs.h"
#include "system/includes.h"
#include "syscfg/syscfg_id.h"
#include "asm/crc16.h"
#include "asm/sfc_norflash_api.h"
#include "server/audio_dev.h"
#if defined CONFIG_BT_ENABLE || defined CONFIG_WIFI_ENABLE
#include "wifi/wifi_connect.h"
#endif

#define LOG_TAG_CONST       USER_CFG
#define LOG_TAG             "[USER_CFG]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_DUMP_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

//======================================================================================//
//                                        BTIF配置项表                                  //
//    参数1: 配置项名字                                                                 //
//    参数2: 配置项需要多少个byte存储                                                   //
//    说明: 配置项ID注册到该表后该配置项将读写于BTIF区域, 其它没有注册到该表            //
//          的配置项则默认读写于VM区域.                                                 //
//======================================================================================//

const struct btif_item btif_table[] = {
//   item_id                len      //
    {CFG_BT_MAC_ADDR,           6 },
    {CFG_BT_FRE_OFFSET,         6 },   //测试盒矫正频偏值
    {CFG_WIFI_FRE_OFFSET,       6 },   //测试盒矫正频偏值
    //{CFG_DAC_DTB,             2 },
    //{CFG_MC_BIAS,             1 },
    {0,                         0 },   //reserved cfg
};

//============================= VM 区域空间最大值 ======================================//
const int vm_max_size_config = VM_MAX_SIZE_CONFIG; //该宏在app_cfg中配置
const int VM_DEFRAG_SIZE = 70; //VM 碎片整理。百分比
////======================================================================================//

extern int bytecmp(unsigned char *p, unsigned char ch, unsigned int num);

#ifdef CONFIG_BT_ENABLE

#define LOCAL_NAME_LEN	32	/*BD_NAME_LEN_MAX*/

static char edr_name[LOCAL_NAME_LEN];

const u8 *bt_get_mac_addr()
{
    static u8 mac_addr[6];

#ifdef CONFIG_WIFI_ENABLE
    wifi_get_mac(mac_addr);
    if (bytecmp(mac_addr, 0, 6)) {
        return mac_addr;
    } else
#endif
    {
        if (syscfg_read(CFG_BT_MAC_ADDR, mac_addr, 6) == 6) {
            return mac_addr;
        }

        u8 flash_uid[16];
        memcpy(flash_uid, get_norflash_uuid(), 16);
        do {
            u32 crc32 = rand32()^CRC32(flash_uid, sizeof(flash_uid));
            u16 crc16 = rand32()^CRC16(flash_uid, sizeof(flash_uid));
            memcpy(mac_addr, &crc32, sizeof(crc32));
            memcpy(&mac_addr[4], &crc16, sizeof(crc16));
        } while (!bytecmp(mac_addr, 0, 6));
        //此处用户可自行修改为本地生成mac地址的算法
        mac_addr[0] &= ~((1 << 0) | (1 << 1));
        syscfg_write(CFG_BT_MAC_ADDR, mac_addr, 6);
        return mac_addr;
    }
}

const char *bt_get_local_name()
{
    const u8 *mac_addr;
    mac_addr = bt_get_mac_addr();
    sprintf(edr_name, "JL-AC79XX-%02X%02X", mac_addr[4], mac_addr[5]);
    return edr_name;
}

const char *bt_get_pin_code()
{
    return "0000";
}

#if TCFG_BT_SNIFF_ENABLE

typedef struct __LRC_CONFIG {
    u16 lrc_ws_inc;
    u16 lrc_ws_init;
    u16 btosc_ws_inc;
    u16 btosc_ws_init;
    u8 lrc_change_mode;
} _GNU_PACKED_ LRC_CONFIG;

struct lp_ws_t {
    u16 lrc_ws_inc;
    u16 lrc_ws_init;
    u16 bt_osc_ws_inc;
    u16 bt_osc_ws_init;
    u8  osc_change_mode;
};

static struct lp_ws_t default_lp_winsize = {
    .lrc_ws_inc = 480,
    .lrc_ws_init = 400,
    .bt_osc_ws_inc = 480,
    .bt_osc_ws_init = 140,
    .osc_change_mode = 2,
};
#endif

#ifdef CONFIG_AEC_ENC_ENABLE
typedef struct __AEC_CONFIG {
    u8 mic_again;           //DAC增益,default:3(0~14)
    u8 dac_again;           //MIC增益,default:22(0~31)
    u8 aec_mode;            //AEC模式,default:advance(diable(0), reduce(1), advance(2))
    u8 ul_eq_en;            //上行EQ使能,default:enable(disable(0), enable(1))
    /*AGC*/
    float ndt_fade_in;      //单端讲话淡入步进default: 1.3f(0.1 ~ 5 dB)
    float ndt_fade_out;     //单端讲话淡出步进default: 0.7f(0.1 ~ 5 dB)
    float dt_fade_in;       //双端讲话淡入步进default: 1.3f(0.1 ~ 5 dB)
    float dt_fade_out;      //双端讲话淡出步进default: 0.7f(0.1 ~ 5 dB)
    float ndt_max_gain;     //单端讲话放大上限,default: 12.f(0 ~ 24 dB)
    float ndt_min_gain;     //单端讲话放大下限,default: 0.f(-20 ~ 24 dB)
    float ndt_speech_thr;   //单端讲话放大阈值,default: -50.f(-70 ~ -40 dB)
    float dt_max_gain;      //双端讲话放大上限,default: 12.f(0 ~ 24 dB)
    float dt_min_gain;      //双端讲话放大下限,default: 0.f(-20 ~ 24 dB)
    float dt_speech_thr;    //双端讲话放大阈值,default: -40.f(-70 ~ -40 dB)
    float echo_present_thr; //单端双端讲话阈值,default:-70.f(-70 ~ -40 dB)
    /*AEC*/
    float aec_dt_aggress;   //原音回音追踪等级, default: 1.0f(1 ~ 5)
    float aec_refengthr;    //进入回音消除参考值, default: -70.0f(-90 ~ -60 dB)
    /*ES*/
    float es_aggress_factor;//回音前级动态压制,越小越强,default: -3.0f(-1 ~ -5)
    float es_min_suppress;  //回音后级静态压制,越大越强,default: 4.f(0 ~ 10)
    /*ANS*/
    float ans_aggress;      //噪声前级动态压制,越大越强default: 1.25f(1 ~ 2)
    float ans_suppress;     //噪声后级静态压制,越小越强default: 0.04f(0 ~ 1)
} _GNU_PACKED_ AEC_CONFIG;

static AEC_CONFIG aec_config = {
    .mic_again           =        3,
    .dac_again           =       22,
    .aec_mode            =        2,
    .ul_eq_en            =        0,
    /*AGC*/
    .ndt_fade_in         =     1.3f,
    .ndt_fade_out        =     0.9f,
    .dt_fade_in          =     1.3f,
    .dt_fade_out         =     0.9f,
    .ndt_max_gain        =     12.f,
    .ndt_min_gain        =      0.f,
    .ndt_speech_thr      =    -50.f,
    .dt_max_gain         =     12.f,
    .dt_min_gain         =      0.f,
    .dt_speech_thr       =    -40.f,
    .echo_present_thr    =    -70.f,
    /*AEC*/
    .aec_dt_aggress      =      1.f,
    .aec_refengthr       =    -70.f,
    /*ES*/
    .es_aggress_factor   =     -3.f,
    .es_min_suppress     =      4.f,
    /*ANS*/
    .ans_aggress         =     1.25f,
    .ans_suppress        =     0.04f,
};

void get_cfg_file_aec_config(struct aec_s_attr *aec_param)
{
    if (aec_config.aec_mode == 0) {
        aec_param->EnableBit = 0;
    } else if (aec_config.aec_mode == 1) {
        aec_param->EnableBit &= ~BIT(0);
    }

    aec_param->AGC_NDT_fade_in_step  = aec_config.ndt_fade_in;
    aec_param->AGC_NDT_fade_out_step = aec_config.ndt_fade_out;
    aec_param->AGC_DT_fade_in_step   = aec_config.dt_fade_in;
    aec_param->AGC_DT_fade_out_step  = aec_config.dt_fade_out;
    aec_param->AGC_NDT_max_gain      = aec_config.ndt_max_gain;
    aec_param->AGC_NDT_min_gain      = aec_config.ndt_min_gain;
    aec_param->AGC_NDT_speech_thr    = aec_config.ndt_speech_thr;
    aec_param->AGC_DT_max_gain       = aec_config.dt_max_gain;
    aec_param->AGC_DT_min_gain       = aec_config.dt_min_gain;
    aec_param->AGC_DT_speech_thr     = aec_config.dt_speech_thr;
    aec_param->AGC_echo_present_thr  = aec_config.echo_present_thr;
    aec_param->AEC_DT_AggressiveFactor = aec_config.aec_dt_aggress;
    aec_param->AEC_RefEngThr         = aec_config.aec_refengthr;
    aec_param->ES_AggressFactor      = aec_config.es_aggress_factor;
    aec_param->ES_MinSuppress        = aec_config.es_min_suppress;
#ifdef CONFIG_DNS_ENC_ENABLE
    aec_param->DNS_gain_floor        = aec_config.ans_aggress;
    aec_param->DNS_over_drive        = aec_config.ans_suppress;
    aec_param->DNS_loudness          = 1.0f;
    aec_param->ANS_AggressFactor     = 1.25f;
    aec_param->ANS_MinSuppress       = 0.09f;
#else
    aec_param->ANS_AggressFactor     = aec_config.ans_aggress;
    aec_param->ANS_MinSuppress       = aec_config.ans_suppress;
#endif
}

#endif

#if TCFG_USER_BLE_ENABLE
//BLE_CFG, ID: 608, CFG_BLE_ID, 0x, len = 0x0
typedef struct __BLE_CONFIG {
    u8 ble_cfg_en;      //ble配置有效标志
    u8 ble_name[32];    //ble蓝牙名
    u8 ble_rf_power;    //ble发射功率
    u8 ble_addr_en;     //ble 地址有效标志
    u8 ble_addr[6];     //ble地址, 若ble_addr_sw = 1; 则有效
} _GNU_PACKED_ BLE_CONFIG;
#endif

//自动关机时间配置
typedef struct __AUTO_OFF_TIME_CONFIG {
    u8 auto_off_time;
} _GNU_PACKED_ AUTO_OFF_TIME_CONFIG;

void cfg_file_parse(void)
{
    u8 tmp[64] = {0};
    u8 buf[32] = {0};
    int ret = 0;

    /*************************************************************************/
    /*                      CFG READ IN cfg_tools.bin                        */
    /*************************************************************************/

#ifdef CONFIG_BT_ENABLE
    //-----------------------------CFG_BT_NAME--------------------------------------//
    ret = syscfg_read(CFG_BT_NAME, tmp, LOCAL_NAME_LEN);
    if (ret < 0) {
        log_info("read bt name err\n");
    } else if (ret >= LOCAL_NAME_LEN) {
        memset(edr_name, 0x00, LOCAL_NAME_LEN);
        memcpy(edr_name, tmp, LOCAL_NAME_LEN);
        edr_name[LOCAL_NAME_LEN - 1] = 0;
    } else {
        memset(edr_name, 0x00, LOCAL_NAME_LEN);
        memcpy(edr_name, tmp, ret);
    }
    log_info("bt name config:%s\n", edr_name);

    //-----------------------------CFG_BT_RF_POWER_ID----------------------------//
    u8 bt_power = 8;
    u8 ble_power = 8;
    ret = syscfg_read(CFG_BT_RF_POWER_ID, &bt_power, 1);
    if (ret < 0) {
        log_debug("read rf power err\n");
        bt_power = 8;
    }

    ble_power = bt_power;

#if TCFG_USER_BLE_ENABLE
    BLE_CONFIG *ble_config = (BLE_CONFIG *)tmp;
    ret = syscfg_read(CFG_BLE_ID, ble_config, sizeof(BLE_CONFIG));
    if (ret > 0 && ble_config->ble_cfg_en) {
        if (syscfg_read(CFG_BLE_NAME, buf, sizeof(ble_config->ble_name)) != sizeof(ble_config->ble_name) || memcmp(buf, ble_config->ble_name, sizeof(ble_config->ble_name))) {
            syscfg_write(CFG_BLE_NAME, ble_config->ble_name, sizeof(ble_config->ble_name));
        }
        log_info("ble name config:%s\n", ble_config->ble_name);
        if (ble_config->ble_addr_en) {
            if (syscfg_read(CFG_BLE_MAC_ADDR, buf, sizeof(ble_config->ble_addr)) != sizeof(ble_config->ble_addr) || memcmp(buf, ble_config->ble_addr, sizeof(ble_config->ble_addr))) {
                syscfg_write(CFG_BLE_MAC_ADDR, ble_config->ble_addr, sizeof(ble_config->ble_addr));
            }
        }
        ble_power = ble_config->ble_rf_power;
    }
#endif

    extern void bt_max_pwr_set(u8 pwr, u8 pg_pwr, u8 iq_pwr, u8 ble_pwr);
#ifdef CONFIG_BT_TEST_ENABLE
    bt_max_pwr_set(11, 6, 6, 4);
#else
    bt_max_pwr_set(bt_power, 6, 6, ble_power);	//0-11 设置蓝牙发射功率
#endif
    log_info("rf bt_power:%d, ble_power:%d\n", bt_power, ble_power);

#if TCFG_BT_SNIFF_ENABLE
    extern void lp_winsize_init(struct lp_ws_t *lp);

    LRC_CONFIG lrc_cfg = {0};
    ret = syscfg_read(CFG_LRC_ID, &lrc_cfg, sizeof(LRC_CONFIG));
    if (ret > 0) {
        log_info("lrc cfg\n");
        /* log_info_hexdump(&lrc_cfg, sizeof(LRC_CONFIG)); */
        default_lp_winsize.lrc_ws_inc      = lrc_cfg.lrc_ws_inc;
        default_lp_winsize.lrc_ws_init     = lrc_cfg.lrc_ws_init;
        default_lp_winsize.bt_osc_ws_inc   = lrc_cfg.btosc_ws_inc;
        default_lp_winsize.bt_osc_ws_init  = lrc_cfg.btosc_ws_init;
        default_lp_winsize.osc_change_mode = lrc_cfg.lrc_change_mode;
    }
    lp_winsize_init(&default_lp_winsize);
#endif

#endif

#ifdef CONFIG_AEC_ENC_ENABLE
    //-----------------------------CFG_AEC_ID------------------------------------//
#ifdef CONFIG_DNS_ENC_ENABLE
    ret = syscfg_read(CFG_SMS_DNS_ID, &aec_config, sizeof(AEC_CONFIG));
#else
    ret = syscfg_read(CFG_AEC_ID, &aec_config, sizeof(AEC_CONFIG));
#endif
    if (ret > 0) {
        /* log_info_hexdump(&aec_config, sizeof(AEC_CONFIG)); */
    }
    log_info("aec config mode:%d, ans_aggress:%f, ans_suppress:%f\n", aec_config.aec_mode, aec_config.ans_aggress, aec_config.ans_suppress);
#endif

#if 0
    /* g_printf("auto low power config:\n"); */
    log_info("auto low power config:\n");
    AUTO_LOWPOWER_V_CONFIG auto_lowpower;
    ret = syscfg_read(CFG_LOWPOWER_V_ID, &auto_lowpower, sizeof(AUTO_LOWPOWER_V_CONFIG));
    if (ret > 0) {
        app_var.warning_tone_v = auto_lowpower.warning_tone_v;
        app_var.poweroff_tone_v = auto_lowpower.poweroff_tone_v;
    }
    log_info("warning_tone_v:%d poweroff_tone_v:%d\n", app_var.warning_tone_v, app_var.poweroff_tone_v);
#endif

    AUTO_OFF_TIME_CONFIG auto_off_time;
    ret = syscfg_read(CFG_AUTO_OFF_TIME_ID, &auto_off_time, sizeof(AUTO_OFF_TIME_CONFIG));
    if (ret > 0) {
        log_info("auto off time config:\n");
        log_info("auto_off_time:%d\n", auto_off_time.auto_off_time);
    }
}

#endif


#if defined CONFIG_BT_ENABLE || defined CONFIG_WIFI_ENABLE
__attribute__((weak)) void wifi_get_xosc(u8 *xosc)
{
    xosc[0] = wifi_calibration_param.xosc_l;
    xosc[1] = wifi_calibration_param.xosc_r;
    if (syscfg_read(VM_XOSC_INDEX, xosc, 2) != 2) {
        syscfg_write(VM_XOSC_INDEX, xosc, 2);
    }
}

__attribute__((weak)) void wifi_get_mcs_dgain(u8 *mcs_dgain)
{
    memcpy(mcs_dgain, wifi_calibration_param.mcs_dgain, sizeof(wifi_calibration_param.mcs_dgain));
    if (syscfg_read(VM_WIFI_PA_MCS_DGAIN, mcs_dgain, sizeof(wifi_calibration_param.mcs_dgain)) != sizeof(wifi_calibration_param.mcs_dgain)) {
        syscfg_write(VM_WIFI_PA_MCS_DGAIN, mcs_dgain, sizeof(wifi_calibration_param.mcs_dgain));
    }
}

__attribute__((weak)) int wifi_get_pa_trim_data(u8 *pa_data)
{
    memcpy(pa_data, wifi_calibration_param.pa_trim_data, sizeof(wifi_calibration_param.pa_trim_data));
    if (syscfg_read(VM_WIFI_PA_DATA, pa_data, sizeof(wifi_calibration_param.pa_trim_data)) != sizeof(wifi_calibration_param.pa_trim_data)) {
        syscfg_write(VM_WIFI_PA_DATA, pa_data, sizeof(wifi_calibration_param.pa_trim_data));
    }
    return 1;//不使用自动tune
}
#endif
