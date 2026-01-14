#include "config.h"
#include "gpio.h"
#include "hw_cpu.h"
#include "asm/power/p33.h"
#include "reset.h"

/* #define LOG_INFO_ENABLE
#define LOG_ERROR_ENABLE
#define LOG_TAG		"[RESET]"
#include "debug.h" */
#define log_info 	deg_printf
#pragma code_seg(".reset_pin_code")
#pragma const_seg(".reset_pin_const")

void clr_long_press_reset_timer(void)
{
    u8 pinr_con = p33_rx_1byte(P3_PINR_CON);
    if (pinr_con & BIT(0)) {
        //先关后开才能使长按复位重新计时
        //putchar('c');
        p33_tx_1byte(P3_PINR_CON, pinr_con & (~BIT(0)));
        p33_tx_1byte(P3_PINR_CON, pinr_con);
    }
}

void power_reset_source_dump(void)
{
    u32 sfr;

    sfr = JL_SYSTEM->RST_SRC;
    log_info("--Reset Source : 0x%x\n", sfr);

    if (sfr & BIT(0)) {
        log_info("P11 RESET");
        u8 p11_reset_source_dump(void);
        p11_reset_source_dump();
    }
    if (sfr & BIT(1)) {
        log_info("SYS POR");
    }

    if (sfr & BIT(2)) {
        log_info("SYS DVDD");
    }

    if (sfr & BIT(5)) {
        log_info("SOFTR");
    }

    if (sfr & BIT(6)) {
        log_info("PLPM");
    }
}


