#include "printf.h"
#include "uart.h"
#include "clock.h"
#include "asm/gpio.h"
#include "lib_include.h"
#include "jiffies.h"

#ifdef UART_DEBUG

#define     DEBUG_UART  JL_UART0
static volatile unsigned int *omap_port ;

static const u32 gpio_regs[] = {
    (u32) JL_PORTA,
    (u32) JL_PORTB,
    (u32) JL_PORTC,
    (u32) JL_PORTD,
    (u32) JL_PORTE,
    (u32) JL_PORTF,
    (u32) JL_PORTG,
    (u32) JL_PORTH,
};

static volatile u8 uart_busy_flag = 0;
static u32 clk_get_uart(void)
{
    return 48000000;
}
#ifdef UART_PUT_TIME
extern volatile unsigned long jiffies;
static char time[28] = {0};
static u32 jiffies_base = 0;
void put_time(void)
{
    int msec = jiffies_to_msecs(jiffies) - jiffies_base;
    u32 hour, min, sec;

    if (!jiffies_base || jiffies_base > jiffies_to_msecs(jiffies)) {
        jiffies_base = jiffies_to_msecs(jiffies);
        return;
    }

    sec = msec / 1000;
    hour = sec / (60 * 60);
    min = (sec - hour * 60 * 60) / 60;
    sec = sec - hour * 60 * 60 - min * 60;
    msec = msec % 1000;

    if (hour >= 100) {
        sprintf(time, "[OTA %d:%02d:%02d.%03d]", hour, min, sec, msec);
    } else {
        sprintf(time, "[OTA %02d:%02d:%02d.%03d]", hour, min, sec, msec);
    }
    __asm_csync();
    char i = 0;
    while (time[i] != '\0') {
        while ((JL_UART0->CON0 & BIT(15)) == 0);
        JL_UART0->CON0 |= BIT(13);
        JL_UART0->BUF = time[i++];
        __asm__ volatile("csync");
    }
}
#endif
__attribute__((always_inline_when_const_args))
void putbyte(char a)
{
    if (!(JL_UART0->CON0 & BIT(0))) {
        return ;
    }
    if (a == '\r') {
        return ;
    }
#ifdef UART_PUT_TIME
    static char puttime = 0;
#endif
    if (a == '\n') {
        while ((JL_UART0->CON0 & BIT(15)) == 0);
        JL_UART0->CON0 |= BIT(13);
        JL_UART0->BUF = '\r';
        __asm__ volatile("csync");
        while ((JL_UART0->CON0 & BIT(15)) == 0);
        JL_UART0->CON0 |= BIT(13);
        JL_UART0->BUF = a;
        __asm__ volatile("csync");
#ifdef UART_PUT_TIME
        puttime = 0;
#endif
    } else {
#ifdef UART_PUT_TIME
        if (!puttime) {
            put_time();
        }
        puttime = a;
#endif
        while ((JL_UART0->CON0 & BIT(15)) == 0);    //TX IDLE

        JL_UART0->CON0 |= BIT(13);
        JL_UART0->BUF = a;
        __asm__ volatile("csync");
    }

}
static void uart_set_baud(u32 baud)
{
    JL_UART0->CON0 &= ~BIT(0);
    if ((clk_get_uart() / baud) % 4 < (clk_get_uart() / baud) % 3) {
        JL_UART0->CON0 &= ~BIT(4);
        JL_UART0->BAUD = (clk_get_uart() / baud) / 4 - 1;
    } else {
        JL_UART0->CON0 |= BIT(4);
        JL_UART0->BAUD = (clk_get_uart() / baud) / 3 - 1;
    }
    JL_UART0->CON0 |= BIT(0);
}

#endif

void uart_init(const char *tx_pin, const char *rx_pin, u32 baud)
{
    return ;//使用uboot打印，不初始化
#ifdef UART_DEBUG
    JL_CLOCK->CLK_CON1 &= ~(3 << 10);
    JL_CLOCK->CLK_CON1 |= (1 << 10);//pll48

    JL_IOMAP->CON3 &= ~BIT(3);
    JL_UART0->CON0 = BIT(0);

    if ((clk_get_uart() / baud) % 4 < (clk_get_uart() / baud) % 3) {
        JL_UART0->CON0 &= ~BIT(4);
        JL_UART0->BAUD = (clk_get_uart() / baud) / 4 - 1;
    } else {
        JL_UART0->CON0 |= BIT(4);
        JL_UART0->BAUD = (clk_get_uart() / baud) / 3 - 1;
    }




    struct gpio_reg *g;

    if (tx_pin[0] == 'P' && tx_pin[1] >= 'A' && tx_pin[1] <= 'H') {
        g = (struct gpio_reg *)gpio_regs[tx_pin[1] - 'A'];
        int mask = (tx_pin[2] - '0') * 10 + (tx_pin[3] - '0');
        if (mask < 16 && mask >= 0) {
            g->dir &= ~BIT(mask);
            g->pu |= BIT(mask);
            g->pd |= BIT(mask);
            g->die &= ~BIT(mask);
            JL_IOMAP->CON1 &= ~(0x0f << 8);//uart0
        }
    }
#endif
}

void uart_close_all(void)
{
    return ;//使用uboot打印，不关闭
    JL_UART0->CON0 = 0;
    JL_UART1->CON0 = 0;
    JL_UART2->CON0 = 0;
}
#if 0//def UART_DEBUG
__attribute__((always_inline_when_const_args))
void uart_close()
{
    if (DEBUG_UART->CON0 & BIT(0)) {
        DEBUG_UART->CON0 = 0;
        DEBUG_UART->CON1 = 0;
        *omap_port = 0;
    }
}
#endif






