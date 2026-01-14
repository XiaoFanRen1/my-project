#ifndef _HWI_H_
#define _HWI_H_

#include "csfr.h"

struct irq_info {
    unsigned char index;
    unsigned char prio;
    unsigned char cpu_id;
};

#define IRQ_EMUEXCPT_IDX   0		//0
#define IRQ_EXCEPTION_IDX  1		//0
#define IRQ_SYSCALL_IDX    2		//0
#define IRQ_TICK_TMR_IDX   3		//0
#define IRQ_TIMER0_IDX     4
#define IRQ_TIMER1_IDX     5
#define IRQ_TIMER2_IDX     6
#define IRQ_TIMER3_IDX     7

#define IRQ_TIME0_IDX      4   	//0
#define IRQ_TIME1_IDX      5   	//0
#define IRQ_TIME2_IDX      6   	//0
#define IRQ_TIME3_IDX      7   	//0
#define IRQ_USB_SOF_IDX    8   	//1
#define IRQ_USB_CTRL_IDX   9
#define IRQ_RTC_WDT_IDX    10
#define IRQ_ALNK0_IDX       11
#define IRQ_ALNK1_IDX      12
#define IRQ_AUDIO_IDX      13
#define IRQ_PORT_IDX       14
#define IRQ_SPI0_IDX       15

#define IRQ_SPI1_IDX       16
#define IRQ_SD0_IDX        17
#define IRQ_SD1_IDX        18
#define IRQ_UART0_IDX      19
#define IRQ_UART1_IDX      20
#define IRQ_UART2_IDX      21
#define IRQ_PAP_IDX        22
#define IRQ_IIC_IDX        23

#define IRQ_SARADC_IDX     24
#define IRQ_SPI3_IDX       25
#define IRQ_GPC_IDX        26
#define IRQ_EMI_IDX        27
#define IRQ_OSA_IDX        28
#define IRQ_BLE_RX_IDX     29
#define IRQ_NFC_IDX        30
#define IRQ_AES_IDX        31
#define IRQ_RDEC_IDX       32

#define IRQ_MCTMRX_IDX     33
#define IRQ_CHX_PWM_IDX    34
#define IRQ_SPDETX         35
#define IRQ_CTM_IDX        36
#define IRQ_SPI2_IDX       37
#define IRQ_PLNK_IDX       38
#define IRQ_PLNK1_IDX      39
#define IRQ_BREDR_IDX      40
#define IRQ_BT_CLKN_IDX    41

#define IRQ_BT_DBG_IDX     42
#define IRQ_WL_LOFC_IDX    43
#define IRQ_LRCT_IDX       44
#define IRQ_BLE_EVENT_IDX  45
#define IRQ_SBC_IDX        46
#define IRQ_SS_IDX         47
#define IRQ_DMA_COPY_IDX   48
#define IRQ_PMU_PND0_IDX   49

#define IRQ_PMU_PND1_IDX   50
#define IRQ_LDMA_IDX       51
#define IRQ_WF_IDX         52
#define IRQ_USB1_SOF_IDX   53
#define IRQ_USB1_CTRL_IDX  54
#define IRQ_JPG_IDX        55

#define IRQ_SHA_IDX        56
#define IRQ_FFT_IDX        57
#define IRQ_SRC_IDX        58
#define IRQ_ISC_IDX        59
#define IRQ_EQ_IDX         60

#define IRQ_WF_RSVD_IDX    61
#define IRQ_TIMER4_IDX     62   	//0
#define IRQ_TIMER5_IDX     63   	//0
#define IRQ_SOFT0_IDX      120
#define IRQ_SOFT1_IDX      121
#define IRQ_SOFT2_IDX      122
#define IRQ_SOFT3_IDX      123
#define IRQ_SOFT4_IDX      124
#define IRQ_SOFT5_IDX      125
#define IRQ_SOFT6_IDX      126
#define IRQ_SOFT7_IDX      127

#define IRQ_MEM_ADDR    (0x1c7fe00)

#define MAX_IRQ_ENTRY_NUM   128

void interrupt_init();
static inline int core_num(void)
{
//  unsigned int num;
//  asm volatile ("%0 = cnum" : "=r"(num) :);
//  return num;
    return 0;
}
void request_irq(unsigned char index, unsigned char priority, void (*handler)(void), unsigned char cpu_id);

void hwi_init(void);
void unrequest_irq(unsigned char index);

void bit_clr_ie(unsigned char index);
void bit_set_ie(unsigned char index);
unsigned char irq_read(unsigned int index);
void irq_common_handler(unsigned int irq_idx);
void irq_set_pending(unsigned int irq_idx);
void irq_handler_register(unsigned int irq_idx, void *hdl, unsigned int irq_prio);
//---------------------------------------------//
// low power waiting
//---------------------------------------------//
__attribute__((always_inline))
static void lp_waiting(int *ptr, int pnd, int cpd, char inum)
{
}
void HWI_Install(unsigned int index, unsigned int isr, unsigned int Priority);
#define INTALL_HWI(a,b,c) HWI_Install(a, (unsigned int)b, c)
//---------------------------------------------//
// interrupt cli/sti
//---------------------------------------------//

static inline int int_cli(void)
{
    int msg;
    asm volatile("cli %0" : "=r"(msg) :);
    return msg;
}

static inline void int_sti(int msg)
{
    asm volatile("sti %0" :: "r"(msg));
}

#ifdef IRQ_TIME_COUNT_EN
void irq_handler_enter(int irq);

void irq_handler_exit(int irq);

void irq_handler_times_dump();
#else

#define irq_handler_enter(irq)      do { }while(0)
#define irq_handler_exit(irq)       do { }while(0)
#define irq_handler_times_dump()    do { }while(0)

#endif


#endif


