#include "asm/cpu.h"
#include "asm/clock.h"
#include "generic/typedef.h"
#include "asm/power_interface.h"
#include "spinlock.h"

#define AT_RAM_CODE SEC_USED(.volatile_ram_code)
#define SFR_SET(sfr, start, len, dat) (sfr = sfr & ~((~(0xffffffff << len)) << start) | ((dat & (~(0xffffffff << len))) << start))

struct cpu_clock {
    u8 lsb_clk_div;
    u8 hsb_clk_div;
    u8 sfc_baud;
    u32 sys_clk;
    u32 uart_clk;
    u32 lsb_clk;
    u32 hsb_clk;
    u32 sys_div_con;
    u32 sdram_clk;
    u32 sys_clk_init;
    u32 sdram_clk_init;
};
static struct cpu_clock clock_info SEC_USED(.sram) = {0};
#define __this (&clock_info)

__attribute__((always_inline))
int clk_get(const char *name)
{
    if (!strcmp(name, "sys")) {
        return __this->sys_clk;
    }
    if (!strcmp(name, "sdram")) {
        return __this->sdram_clk;
    }

    if (!strcmp(name, "uart")) {
        /*return __this->lsb_clk;*/
        return __this->uart_clk;
    }

    if (!strcmp(name, "sd") || !strcmp(name, "timer")) {
        return __this->lsb_clk;
    }

    if (!strcmp(name, "spi")) {
        return __this->lsb_clk;
    }

    if (!strcmp(name, "hsb")) {
        return __this->hsb_clk;
    }

    if (!strcmp(name, "lsb")) {
        return __this->lsb_clk;
    }

    if (!strcmp(name, "osc")) {
#ifdef CONFIG_FPGA_ENABLE
        return 12000000;
#else
        if (((JL_ANA->PLL_CON0 >> 26) & 0x3) == 2) { //获取40M晶振，与uboot有关，后续其他晶振需要参考uboot修改
            return 40000000;
        }
        return 24000000;
#endif
    }

    return -EINVAL;
}

int clk_early_init()
{
    u8 clk_div1[] = {1, 3, 5, 7};
    u8 clk_div2[] = {1, 2, 4, 8};
    u8 div1 = clk_div1[(JL_CLOCK->CLK_CON2 >> 4) & 3];
    u8 div2 = clk_div2[(JL_CLOCK->CLK_CON2 >> 6) & 3];

    u8 pll_div = JL_CLOCK->CLK_CON2 & 0xF;
    u32 freq_m = ((((JL_ANA->PLL_CON0 >> 26) & 0x3) == 2) ? 40 : 24) * 1000000; //晶振
    u8 refds = ((JL_ANA->PLL2_CON0 >> 2) & 0x1F) + 2;
    u32 PLL2_IN_CLK = freq_m / refds;
    u16 DS0 = (JL_ANA->PLL2_CON1 & 0xFFF) + 2;
    u32 pll2_clk = PLL2_IN_CLK * DS0;
    u32 usb_pll3_clk = ((HUSB_COM_CON0 >> 18) & 0x3FF) + 2;

    //操作系统的定时器寄存器清0，防止打印出现除0
    q32DSP(0)->TTMR_CNT = 0;
    q32DSP(0)->TTMR_PRD = 0;

    if (pll_div <= 4) {
        switch (pll_div) {
        case 4:
            __this->sys_clk = 107000000;
            break;
        case 3:
            __this->sys_clk = 480000000;
            break;
        case 2:
            __this->sys_clk = 320000000;
            break;
        case 1:
            __this->sys_clk = 137000000;
            break;
        default:
            __this->sys_clk = 192000000;
            break;
        }
        __this->sys_clk = __this->sys_clk / div1 / div2;
    } else {
        u8 div = 0; //这个值需要配合UBOOT配置
        switch (pll_div) {
        case 5:
            div = 2 * 2;
            break;
        case 6:
            div = 3 /*1.5*2*/;
            break;
        case 7:
            div = 1 * 2;
            break;
        default:
            __this->sys_clk = 480000000;
            if (pll_div == 8 || pll_div == 9) {
                __this->sys_clk = (pll_div == 8 ? usb_pll3_clk : usb_pll3_clk / 2) * 1000000;
            }
            goto clk_set;
        }
        __this->sys_clk = div ? (2 * pll2_clk / div) : __this->sys_clk;//USB PLL未调试
    }
clk_set:
    ;
#ifdef CONFIG_FPGA_ENABLE
    __this->sys_clk = 48000000;
#endif
    u8 hsb_div = ((JL_CLOCK->SYS_DIV >> 16) & 0x3) + 1;
    __this->sys_div_con = JL_CLOCK->SYS_DIV;
    __this->sfc_baud = JL_SFC->BAUD;
    __this->hsb_clk_div = ((JL_CLOCK->SYS_DIV >> 16) & 0x3) + 1;
    __this->lsb_clk_div = ((JL_CLOCK->SYS_DIV >> 8) & 0x7) + 1;
    __this->hsb_clk = __this->sys_clk / __this->hsb_clk_div;
    __this->lsb_clk = __this->sys_clk / __this->hsb_clk_div / __this->lsb_clk_div;
    if (((HUSB_COM_CON2 >> 3) & 0x3) == 0x3) {//usb pll3 output
        __this->sdram_clk = (JL_SDR->CON0 & BIT(11)) ? (usb_pll3_clk / 2 * 1000000) : 0;//sdram 8phase固定使用PLL3的2分频
    } else {
        __this->sdram_clk = (JL_SDR->CON0 & BIT(11)) ? (pll2_clk / 2) : 0;//sdram 4phase固定使用PLL2的2分频
    }
    __this->sys_clk_init = __this->sys_clk;
    __this->sdram_clk_init = __this->sdram_clk;

    SFR(JL_CLOCK->CLK_CON1, 10, 2, 1);//uart 48M
    __this->uart_clk = 48000000;
    /*__this->uart_clk = __this->lsb_clk;*/

    JL_CLOCK->CLK_CON1 &= ~BIT(16);

    //关闭107M和137M时钟输出，用到时再打开
    JL_ANA->PLL_CON1 &= ~(BIT(27) | BIT(28));

    /*p33_spicon_clock_div(4);*/
    JL_P33->SPI_CON |= 0x3 << 2;

    q32DSP(0)->TTMR_PRD = __this->sys_clk / 100;
    printf("---> sys_clk = %dMHz, hsb_clk = %dMHz, lsb_clk = %dMHz\n",
           __this->sys_clk / 1000000, __this->hsb_clk / 100000, __this->lsb_clk / 100000);

    return 0;
}
