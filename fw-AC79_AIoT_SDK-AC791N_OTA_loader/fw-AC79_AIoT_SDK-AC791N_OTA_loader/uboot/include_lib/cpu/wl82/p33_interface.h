#ifndef __PR_PORT_API_H__
#define __PR_PORT_API_H__
#include "typedef.h"


//PR PORT
#define PORTR0          	    0
#define PORTR1             		1
#define PORTR2              	2
#define PORTR3              	3
#define PORTR4              	4
#define PORTR5              	5

//WAKEUP SOURCE
#define WAKE_UP_PR0        		BIT(0)
#define WAKE_UP_PR1         	BIT(1)
#define WAKE_UP_PR2         	BIT(2)
#define WAKE_UP_PR3         	BIT(3)
#define WAKE_UP_PR4         	BIT(4)
#define WAKE_UP_PR5         	BIT(5)

#define EDGE_PR0            	BIT(0)
#define EDGE_PR1            	BIT(1)
#define EDGE_PR2            	BIT(2)
#define EDGE_PR3            	BIT(3)
#define EDGE_PR4            	BIT(4)
#define EDGE_PR5            	BIT(5)

#define PCNT_OVF_WKUP  		  	BIT(0)
#define EDGE_WKUP         		BIT(1)
#define ALMOUT_WKUP            	BIT(2)
#define LDO5v_WKUP        		BIT(3)
#define VDD50_LVD_WKUP    		BIT(4)
#define RTCVDD_LVD_WKUP   		BIT(5)
#define CHG_WKUP				BIT(6)
#define LRC_TRIM_WKUP			BIT(7)

#define PD_CON0_INIT                            \
        /*                       */ ( 0<<7 )  | \
        /*                       */ ( 0<<6 )  | \
        /* PD_RUN FLAG  1bit RO  */ ( 0<<5 )  | \
        /* WL SETL TMR  1bit RW  */ ( 0<<4 )  | \
        /* DIS EN       2bit RW  */ ( 2<<2 )  | \
        /* PD MD        1bit RW  */ ( 0<<1 )  /*0:Power down / 1:Power off*/     | \
        /* PD_EN        1bit RW  */ ( 1<<0 )

#define PD_CON0_POWEROFF                        \
        /*                       */ ( 0<<7 )  | \
        /*                       */ ( 0<<6 )  | \
        /* PD_RUN FLAG  1bit RO  */ ( 0<<5 )  | \
        /* WL SETL TMR  1bit RW  */ ( 0<<4 )  | \
        /* DIS EN       2bit RW  */ ( 2<<2 )  | \
        /* PD MD        1bit RW  */ ( 1<<1 )  /*0:Power down / 1:Power off*/     | \
        /* PD_EN        1bit RW  */ ( 1<<0 )

#define PD_CON1_INIT                            \
        /* POR FLAG     1bit RO  */ ( 0<<7 )  | \
        /* CLR POR PND  1bit RW  */ ( 1<<6 )  | \
        /* CK DIV       2bit RW  */ ( 0<<4 )  /* 0:div0 1:div4 2:div16 3:div6    4 */  | \
        /* PU SLOW      1bit RW  */ ( 0<<3 )  | \
        /* CK SEL       3bit RW  */ ( 4<<0 )  /* 0:RC 1:BT 2:RTCH 3:RTCL 4:LR    C */

#define PD_CON1_INIT_BT                            \
        /* POR FLAG     1bit RO  */ ( 0<<7 )  | \
        /* CLR POR PND  1bit RW  */ ( 1<<6 )  | \
        /* CK DIV       2bit RW  */ ( 3<<4 )  /* 0:div0 1:div4 2:div16 3:div6    4 */  | \
        /* PU SLOW      1bit RW  */ ( 0<<3 )  | \
        /* CK SEL       3bit RW  */ ( 1<<0 )  /* 0:RC 1:BT 2:RTCH 3:RTCL 4:LR    C */

#define PD_CON1_INIT_LRC_32K                         \
        /* POR FLAG     1bit RO  */ ( 0<<7 )  | \
        /* CLR POR PND  1bit RW  */ ( 1<<6 )  | \
        /* CK DIV       2bit RW  */ ( 0<<4 )  /* 0:div0 1:div4 2:div16 3:div6    4 */  | \
        /* PU SLOW      1bit RW  */ ( 0<<3 )  | \
        /* CK SEL       3bit RW  */ ( 4<<0 )  /* 0:RC 1:BT 2:RTCH 3:RTCL 4:LR    C */

#define pd_wldo12_auto_init                     \
        /* WLDO PRD     3bit RW  */ ( 7<<5 )  | \
        /*              1bit RW  */ ( 0<<4 )  | \
        /*              2bit RW  */ ( 0<<2 )  | \
        /*              2bit RW  */ ( 0<<0 )

#define pd_wldo06_auto_init                     \
        /* WLDO LVL LOW 3bit RW  */ ( 6<<5 )  | \
        /* WLDO EN      1bit RW  */ ( 1<<4 )  | \
        /* WLDO LEVEL   3bit RW  */ ( 6<<0 )
#define PD_CON2_INIT                            \
        /* SOFT USE     8bit RW  */ ( 0<<4 )  | \
        /* SPI BPORT    1bit RW  */ ( 0<<3 )  | \
        /* SPI APORT    1bit RW  */ ( 0<<2 )  | \
        /* BT_FAST      2bit RW  */ ( 3<<0 )  /* 0:NO CLK 1:BT 3ms 2:BT 1ms 3    :RTC  */
#define PD_CON1_INIT_RTCL                            \
        /* POR FLAG     1bit RO  */ ( 0<<7 )  | \
        /* CLR POR PND  1bit RW  */ ( 1<<6 )  | \
        /* CK DIV       2bit RW  */ ( 0<<4 )  /* 0:div0 1:div4 2:div16 3:div6    4 */  | \
        /* PU SLOW      1bit RW  */ ( 0<<3 )  | \
        /* CK SEL       3bit RW  */ ( 3<<0 )  /* 0:RC 1:BT 2:RTCH 3:RTCL 4:LR    C */


typedef enum {
    PORTR_CTL_DIR       = 0,
    PORTR_CTL_OUT,
    PORTR_CTL_HD,
    PORTR_CTL_PU,
    PORTR_CTL_PD,
    PORTR_CTL_DIE,
    PORTR_CTL_OE,
    PORTR_CTL_PWM_OE,

} PORTR_CTL_TYPE;

enum {
    LONG_4S_RESET = 0,
    LONG_8S_RESET,
};
enum {
    PCNT_RISING_DBGE = 0,
    PCNT_FALLING_DBGE = 1,
};

//PR PORT BACKUP
void P33_PORT_BACKUP(void);
void P33_PORT_RECOVER(void);
void P33_PORT_CLOSE(void);

void adc_mux_ch_set(u8 ch);

// WAKEUP
void soft_poweroff_wakeup_io(u8 wakeup_io, u8 wakeup_edge);
u8 get_wake_up_type(void);
u8 check_io_wakeup_pend();    //wakeup io
u8 wakeup_pending_deal(void);

//pcnt
void pcnt_init(u8 port, u8 edge);
void pcnt_kitstart(u8 cnt);
void pcnt_stop(void);
u8 get_pcnt_value();
void set_pcnt_value(u8 value);

//PR PORT OPERATIONS
void PORTR_DIR(u8 port, u8 val);
void PORTR_OUT(u8 port, u8 val);
void PORTR_HD(u8 port, u8 val);
void PORTR_PU(u8 port, u8 val);
void PORTR_PD(u8 port, u8 val);
void PORTR_DIE(u8 port, u8 val);
void PORTR_PWM_OE(u8 port, u8 val);
u8 PORTR_IN(u8 port);
void clr_pr_wkup_pending(void);

//P33 RESET SETTING mode 0:4S 1:8S
int p33_port_reset(u8 mode, u8 port, u8  enable, u8 edge);
int p33_port_reset_close(void);


//P33 LDO5V
int p33_ldo5v_detect(u8  enable, u8 edge);
int get_ldo5v_detect_flag(void);
void clr_ldo5v_pending(void);
int get_ldo5v_cmp_flag(void);
void set_ldo5v_level_wkup_en(u8 en);
void set_ldo5v_edge_wkup_en(u8 en);
int get_ldo5v_panding(void);

//P33_CHARGE
void CHARGE_FULL_V_SET(u8 val);
void CHARGE_FULL_mA_SET(u8 val);
void CHARGE_mA_SET(u8 val);
void CHARGE_CCVOL_V_SET(u8 val);
void CHARGE_EN(u8 en);
void CHGBG_EN(u8 en);
void CHARGE_LEVEL_DETECT_EN(u8 en);
void CHARGE_EDGE_DETECT_EN(u8 en);
void CHARGE_WKUP_SOURCE_SET(u8 val);
void CHARGE_WKUP_EDGE_SET(u8 val);
u8 CHARGE_WKUP_EDGE_GET(void);
void CHARGE_WKUP_EN(u8 en);
void CHARGE_WKUP_PND_CLR(void);
int CHARGE_FULL_FLAG_GET(void);
int CHARGE_CI_FLAG_GET(void);

u8 p33_get_power_flag();
void p33_clr_power_flag();
void clr_lrc_trim_pending(void);
void clr_pcnt_pending(void);

void VCM_DET_EN(u8 en);
void P33_PLVD_EN(u8 en);
void P33_VLVD_EN(u8 en);
void RESET_MASK_SW(u8 sw);
void WLDO12_DISABLE(void);
void CLR_PLVD_PND(void);
void CLR_LVD_PND(void);
void VDD13TO12_SYS_EN(u8 en);
void VDD13TO12_DSP_EN(u8 en);
void LDO13_EN(u8 en);
void DCDC13_EN(u8 en);
void MLDO33_EN(u8 en);
void PW_GATE_EN(u8 en);
void DSP_SHORT_EN(u8 en);
u8 GET_DSP_SHORT_EN(void);
void RC_250k_EN(u8 en);
void DCDC_COT(void);
void DCDC_PWM(void);
void WLDO12_EN(u8 en);
void WLDO06_EN(u8 en);
void WBTLDO_EN(u8 en);
void WVDDIO_EN(u8 en);
void SET_RTCVDD_LEV(u8 lev);//7(3.4v),6(3.2v),5(3.0v),4(2.8v),3(2.6v),2(2.4v),1(2.2v),0(2.0v)
u8 GET_RTCVDD_LEV(void);
void SET_VDDIO_LEV(u8 lev);//7(3.4v),6(3.2v),5(3.0v),4(2.8v),3(2.6v),2(2.4v),1(2.2v),0(2.0v)
u8 GET_VDDIO_LEV(void);
void D2SH_EN_SW(u8 en);
void LPM_RST_EN(u8 en);
void DSP_CONNECT(u8 en);
void DSP_CONNECT_KEEP(u8 keep);
void DET_PMU_OE(u8 en);
void ADC_CHANNEL_SEL(u8 ch);
void SET_VDC13_LEV(u8 lev);//0(1.098v)  1(1.148v) 2(1.199v) 3(1.249v) 4(1.299v) 5(1.35v) 6(1.4v) 7(1.5v)
u8 GET_VDC13_LEV(void);
void SET_VDD13TO12_SYS_LEV(u8 lev);//0(0.908v)  1(0.957v) 2(1.006v) 3(1.055v) 4(1.103v) 5(1.152v) 6(1.201v) 7(1.250v)
u8 GET_VDD13TO12_SYS_LEV(void);
void SET_VDD13TO12_DSP_LEV(u8 lev);//0(1.005v)  1(1.054v) 2(1.103v) 3(1.152v) 4(1.201v) 5(1.250v) 6(1.299v) 7(1.350v)
u8 GET_VDD13TO12_DSP_LEV(void);
void SET_VLVD_LEV(u8 lev);//0(1.9v)  1(2.0v) 2(2.1v) 3(2.2v) 4(2.3v) 5(2.4v) 6(2.5v) 7(2.6v)
void LVD_OE_EN(u8 en);
void LVD_DETECT_SOURCE(u8 source);
void LVD_DETECT_MD(u8 md);
u8 LVD_PND(void);
void SET_WVDD_LEV(u8 lev);
void LRC_CLK_EN(void);
u8 READ_PMU_RESET_SOURCE(void);

void p33_reset_source_check(void);
void adc_ldo_ch_sel(u32 ch);

#endif
