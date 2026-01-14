#include "app_config.h"
#include "system/includes.h"

//SEC_USED(.volatile_ram_code)
___interrupt static void timer_isr(void)
{
    if (JL_TIMER4->CON & BIT(15))
    {
        JL_TIMER4->CON |= BIT(14);
        printf("timer_isr run in cpu %d \r\n", current_cpu_id());
    }
}

static int c_main(void)
{
    JL_TIMER4->CON= 0x91;
    request_irq(IRQ_TIMER4_IDX, 3, timer_isr, 1);

    return 0;
}
late_initcall(c_main);
