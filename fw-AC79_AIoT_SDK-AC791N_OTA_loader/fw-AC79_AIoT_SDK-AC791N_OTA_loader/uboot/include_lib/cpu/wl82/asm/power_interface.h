#ifndef POWER_INTERFACE_H
#define POWER_INTERFACE_H

// #include "asm/hwi.h"
//
#include "generic/typedef.h"
#define NEW_BASEBAND_COMPENSATION       0

#define AT_VOLATILE_RAM             AT(.volatile_ram)
#define AT_VOLATILE_RAM_CODE        AT(.volatile_ram_code)
#define AT_NON_VOLATILE_RAM         AT(.non_volatile_ram)
#define AT_NON_VOLATILE_RAM_CODE    AT(.non_volatile_ram_code)

#define SYS_SLEEP_EN                        BIT(1)
#define SYS_SLEEP_BY_IDLE                   BIT(2)
#define RF_SLEEP_EN                         BIT(3)
#define RF_FORCE_SYS_SLEEP_EN               BIT(4)
#define SLEEP_SAVE_TIME_US                  1L

enum {
    BT_OSC = 0,
    RTC_OSCL,
    LRC_32K,
    LRC_16M,
};


enum {
    OSC_TYPE_LRC = 0,
    OSC_TYPE_BT_OSC,
};

enum {
    PWR_NO_CHANGE = 0,
    PWR_LDO33,
    PWR_LDO15,
    PWR_DCDC15,
};

enum {
    LONG_4S_RESET = 0,
    LONG_8S_RESET,
};

//Macro for VDDIOM_VOL_SEL
enum {
    VDDIOM_VOL_28V = 0,
    VDDIOM_VOL_29V,
    VDDIOM_VOL_30V,
    VDDIOM_VOL_31V,
    VDDIOM_VOL_32V,
    VDDIOM_VOL_33V,
    VDDIOM_VOL_34V,
    VDDIOM_VOL_35V,
};

//Macro for VDDIOW_VOL_SEL
enum {
    VDDIOW_VOL_21V = 0,
    VDDIOW_VOL_24V,
    VDDIOW_VOL_28V,
    VDDIOW_VOL_32V,
};

struct low_power_param {
    u8 osc_type;
    u32 btosc_hz;
    u8  delay_us;
    u8  config;
    u8  btosc_disable;

    u8 vddiom_lev;
    u8 vddiow_lev;
    u8 pd_wdvdd_lev;
    u8 lpctmu_en;
    u8 vddio_keep;

    u32 osc_delay_us;
};

#define BLUETOOTH_RESUME    BIT(1)

#define RISING_EDGE         0
#define FALLING_EDGE        1

enum {
    PORT_FLT_NULL = 0,
    PORT_FLT_256us,
    PORT_FLT_512us,
    PORT_FLT_1ms,
    PORT_FLT_2ms,
    PORT_FLT_4ms,
    PORT_FLT_8ms,
    PORT_FLT_16ms,
};

struct port_wakeup {
    u8 pullup_down_enable;        //
    u8 edge;        //[0]: Rising / [1]: Falling
    u8 both_edge;
    u8 filter;
    u8 iomap;       //Port Group-Port Index
};


struct lvd_wakeup {
    u8 attribute;   //Relate operation bitmap OS_RESUME | BLUETOOTH_RESUME
};


//<Max hardware wakeup port
#define MAX_WAKEUP_PORT     8

struct wakeup_param {
    const struct port_wakeup *port[MAX_WAKEUP_PORT];
    const struct charge_wakeup *charge;
    const struct alarm_wakeup *alram;
    const struct lvd_wakeup *lvd;
    const struct sub_wakeup *sub;
};

struct reset_param {
    u8 en;
    u8 mode;
    u8 level;
    u8 iomap;   //Port Group, Port Index
};

struct low_power_operation {

    const char *name;

    u32(*get_timeout)(void *priv);

    void (*suspend_probe)(void *priv);

    void (*suspend_post)(void *priv, u32 usec);

    void (*resume)(void *priv, u32 usec);

    void (*resume_post)(void *priv, u32 usec);
};

u32 __tus_carry(u32 x);

u8 __power_is_poweroff(void);

void poweroff_recover(void);

void power_init(const struct low_power_param *param);

u8 power_is_low_power_probe(void);

u8 power_is_low_power_post(void);

void power_set_soft_poweroff();

void power_set_soft_poweroff_advance();

void power_set_mode(u8 mode);

void power_keep_dacvdd_en(u8 en);

struct soft_flag0_t {
    u8 wdt_dis: 1;
    u8 poweroff: 1;
    u8 is_port_b: 1;
    u8 res: 5;
};
struct soft_flag1_t {
    u8 usbdp: 2;
    u8 usbdm: 2;
    u8 uart_key_port: 1;
    u8 ldoin: 3;
};
struct soft_flag2_t {
    u8 pa7: 4;
    u8 pb4: 4;
};
struct soft_flag3_t {
    u8 pc3: 4;
    u8 pc5: 4;
};

struct boot_soft_flag_t {
    union {
        struct soft_flag0_t boot_ctrl;
        u8 value;
    } flag0;
    union {
        struct soft_flag1_t misc;
        u8 value;
    } flag1;
    union {
        struct soft_flag2_t pa7_pb4;
        u8 value;
    } flag2;
    union {
        struct soft_flag3_t pc3_pc5;
        u8 value;
    } flag3;
};
enum soft_flag_io_stage {
    SOFTFLAG_HIGH_RESISTANCE,
    SOFTFLAG_PU,
    SOFTFLAG_PD,

    SOFTFLAG_OUT0,
    SOFTFLAG_OUT0_HD0,
    SOFTFLAG_OUT0_HD,
    SOFTFLAG_OUT0_HD0_HD,

    SOFTFLAG_OUT1,
    SOFTFLAG_OUT1_HD0,
    SOFTFLAG_OUT1_HD,
    SOFTFLAG_OUT1_HD0_HD,
};
void mask_softflag_config(const struct boot_soft_flag_t *softflag);
void power_set_callback(u8 mode, void (*powerdown_enter)(u8 step), void (*powerdown_exit)(u32), void (*soft_poweroff_enter)(void));

// u8 power_is_poweroff_post(void);
#define  power_is_poweroff_post()   0

void power_set_proweroff(void);

u8 power_reset_source_dump(void);
/*-----------------------------------------------------------*/

void low_power_request(void);

void *low_power_get(void *priv, const struct low_power_operation *ops);

void low_power_put(void *priv);

void low_power_sys_request(void *priv);

void *low_power_sys_get(void *priv, const struct low_power_operation *ops);

void low_power_sys_put(void *priv);

u8 low_power_sys_is_idle(void);

s32 low_power_trace_drift(u32 usec);

void low_power_reset_osc_type(u8 type);

u8 low_power_get_default_osc_type(void);

u8 low_power_get_osc_type(void);
/*-----------------------------------------------------------*/

void power_wakeup_index_enable(u8 index);

void power_wakeup_index_disable(u8 index);

void power_wakeup_disable_with_port(u8 port);

void power_wakeup_enable_with_port(u8 port);

void power_wakeup_set_edge(u8 port_io, u8 edge);

void power_wakeup_init(const struct wakeup_param *param);

void power_wakeup_init_test();

u8 get_wakeup_source(void);

u8 is_ldo5v_wakeup(void);
// void power_wakeup_callback(JL_SignalEvent_t cb_event);

void p33_soft_reset(void);
/*-----------------------------------------------------------*/

void power_reset_close();

void lrc_debug(u8 a, u8 b);

void sdpg_config(int enable);

void p11_init(void);
/*-----------------------------------------------------------*/

typedef u8(*idle_handler_t)(void);

struct lp_target {
    char *name;
    idle_handler_t is_idle;
};

#define REGISTER_LP_TARGET(target) \
        const struct lp_target target sec(.lp_target)


extern const struct lp_target lp_target_begin[];
extern const struct lp_target lp_target_end[];

#define list_for_each_lp_target(p) \
    for (p = lp_target_begin; p < lp_target_end; p++)
/*-----------------------------------------------------------*/

struct deepsleep_target {
    char *name;
    u8(*enter)(void);
    u8(*exit)(void);
};

#define DEEPSLEEP_TARGET_REGISTER(target) \
        const struct deepsleep_target target sec(.deepsleep_target)


extern const struct deepsleep_target deepsleep_target_begin[];
extern const struct deepsleep_target deepsleep_target_end[];

#define list_for_each_deepsleep_target(p) \
    for (p = deepsleep_target_begin; p < deepsleep_target_end; p++)
/*-----------------------------------------------------------*/


#endif

