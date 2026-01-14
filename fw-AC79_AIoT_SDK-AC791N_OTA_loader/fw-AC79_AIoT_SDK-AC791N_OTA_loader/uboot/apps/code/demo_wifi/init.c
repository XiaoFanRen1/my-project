#include "system/init.h"
#include "system/task.h"
#include "irq.h"
#include "jiffies.h"
#include "system/timer.h"

u32 get_irq_lock_cnt(void);
void local_irq_enable(void);
void local_irq_disable(void);
__attribute__((noinline))
void sp_ovf_unen(void)
{
    q32DSP(OS_CPU_ID)->EMU_CON = 0;
}
__attribute__((noinline))
void sp_ovf_en(void)
{
    /*q32DSP(OS_CPU_ID)->EMU_CON |= BIT(3);*/
}

__attribute__((always_inline))
void __local_irq_disable(void)
{
    local_irq_disable();
}

__attribute__((always_inline))
void __local_irq_enable(void)
{
    local_irq_enable();
}

__attribute__((always_inline))
int __local_irq_lock_cnt(void)
{
    return get_irq_lock_cnt();
}

__attribute__((always_inline))
void sys_local_irq_disable(void)
{
    local_irq_disable();
}

__attribute__((always_inline))
void sys_local_irq_enable(void)
{
    local_irq_enable();
}
enum pc0_clock_out_index {
    BT_OSC_CLK_OUT = 1,
    RTC_OSL_OUT,
    CTM_IN_OUT,
    LSB_CLK_OUT,
    HSB_CLK_OUT,
    SFC_CLK_OUT,
    HCO_CLK_OUT,
};

void clk_out_pc0(enum pc0_clock_out_index index)//
{
    JL_PORTC->DIR &= ~BIT(0);
    JL_PORTC->OUT &= ~BIT(0);
    JL_PORTC->HD |= BIT(0);
    SFR(JL_CLOCK->CLK_CON0, 10, 3, index);
}
void app_main(void);
int clk_early_init();
void hw_power_init(void);
void debug_init(void);
void wdt_init(u8 time);
void timer_init(void);
void exception_isr_handler(void);
void exception_init(u32 isr_addr);
int wifi_init(void)
{
    wdt_init(0xc);
    interrupt_init();
    exception_init((u32)exception_isr_handler); //初始化异常函数
    debug_init();
    clk_early_init();
    hw_power_init();
    /*clk_out_pc0(LSB_CLK_OUT);*/
    os_init();
    app_main();
    os_start();

    while (1) {
        os_time_dly(1000);
    }
    return 0;
}

