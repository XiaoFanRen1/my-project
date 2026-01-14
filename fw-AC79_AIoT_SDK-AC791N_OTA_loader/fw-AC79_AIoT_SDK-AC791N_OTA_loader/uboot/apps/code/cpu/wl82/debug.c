#include "system/includes.h"
#include "asm/debug.h"

static u8 debug_index = 0;

extern void p33_system_reset(void);

void exception_analyze(int *sp)
{
    unsigned int reti = sp[16];
    unsigned int rete = sp[17];
    unsigned int retx = sp[18];
    unsigned int rets = sp[19];
    unsigned int psr  = sp[20];
    unsigned int icfg = sp[21];
    unsigned int usp  = sp[22];
    unsigned int ssp  = sp[23];
    unsigned int _sp  = sp[24];

    puts("\n\n===========exception===========\r\n");
    for (int r = 0; r < 16; r++) {
        printf("R%d: %x\n", r, sp[r]);
    }

    printf("icfg: %x \n", icfg);
    printf("psr:  %x \n", psr);
    printf("rets: 0x%x \n", rets);
    printf("reti: 0x%x \n", reti);
    printf("usp : %x, ssp : %x sp: %x\r\n\r\n", usp, ssp, _sp);
}


static void debug_enter_critical(void)
{
    if (corex2(0)->DBG_WR_EN & BIT(0)) {
        return;
    }
    corex2(0)->DBG_WR_EN = 0xe7;//set debug reg write enable
}

static void debug_exit_critical(void)
{
    if (corex2(0)->DBG_WR_EN & BIT(0)) {
        corex2(0)->DBG_WR_EN = 0xe7;//set debug reg write disable
    }
}

static inline int get_debug_index(void)
{
    for (u8 i = 0; i < 8 ; i++) {
        if (!(debug_index & BIT(i))) {
            return i;
        }
    }

    return -1;
}

void pc0_rang_limit(void *low_addr, void *high_addr)
{
    corex2(0)->DBG_MSG_CLR = 0xffffffff;
    corex2(0)->PC_LIMIT0_H = (u32)high_addr;
    corex2(0)->PC_LIMIT0_L = (u32)low_addr;
}
void pc_rang_limit0(void *low_addr, void *high_addr)
{
    pc0_rang_limit(low_addr, high_addr);
}
void pc1_rang_limit(void *low_addr, void *high_addr)
{
    corex2(0)->DBG_MSG_CLR = 0xffffffff;
    corex2(0)->PC_LIMIT1_H = (u32)high_addr;
    corex2(0)->PC_LIMIT1_L = (u32)low_addr;
}

void pc_rang_limit(void *low_addr0, void *high_addr0, void *low_addr1, void *high_addr1)
{
    debug_enter_critical();
    if (low_addr0 && high_addr0) {
        pc0_rang_limit(low_addr0, high_addr0);
    }
    if (low_addr1 && high_addr1) {
        pc1_rang_limit(low_addr1, high_addr1);
    }
    debug_exit_critical();
}

#define wr_limit_h    ((volatile u32 *)&corex2(0)->WR_LIMIT0_H)
#define wr_limit_l    ((volatile u32 *)&corex2(0)->WR_LIMIT0_L)
#define wr_allow_num  ((volatile u32 *)&corex2(0)->WR_ALLOW_ID0)
void all_dev_range_limit(void *addr, int size)//所有外设写进异常(地址:sram+经过cach的sdram)
{
    int limit_index = 0;
    debug_enter_critical();
    corex2(0)->PRP_WR_LIMIT_EN &= ~BIT(limit_index);
    corex2(0)->DBG_MSG_CLR = 0xffffffff;
    debug_index |= BIT(limit_index);
    corex2(0)->WR_LIMIT0_L = (u32)((u32)addr);
    corex2(0)->WR_LIMIT0_H = (u32)((u32)addr + size);
    corex2(0)->WR_ALLOW_ID0 = 0;
    corex2(0)->PRP_WR_LIMIT_EN = BIT(limit_index);
    debug_exit_critical();
}
void all_dev_range_unlimit(void *addr)
{
    debug_enter_critical();
    corex2(0)->DBG_MSG_CLR = 0xffffffff;
    corex2(0)->WR_LIMIT0_L = 0;
    corex2(0)->WR_LIMIT0_H = 0;
    corex2(0)->WR_ALLOW_ID0 = 0;
    corex2(0)->PRP_WR_LIMIT_EN = 0;
    debug_exit_critical();
}
int cpu_write_range_limit(void *low_addr, u32 win_size)
{
    void *high_addr = low_addr + win_size - 1;
    int i = get_debug_index();
    if (i == -1) {
        return -1;
    }
    debug_enter_critical();
    debug_index |= BIT(i);
    corex2(0)->DBG_MSG_CLR = 0xffffffff;
    wr_limit_h[i] = (u32)high_addr;
    wr_limit_l[i] = (u32)low_addr;
    corex2(0)->C0_WR_LIMIT_EN |= BIT(i);
    corex2(0)->C1_WR_LIMIT_EN |= BIT(i);
    debug_exit_critical();
    return 0;
}

void cpu_write_range_unlimit(void *low_addr)
{
    int i;
    debug_enter_critical();
    corex2(0)->DBG_MSG_CLR = 0xffffffff;
    for (i = 0; i < 8; i++) {
        if (wr_limit_l[i] == (u32)low_addr) {
            wr_limit_h[i] = 0;
            wr_limit_l[i] = 0;
            debug_index &= ~BIT(i);
            corex2(0)->C0_WR_LIMIT_EN &= ~BIT(i);
            corex2(0)->C1_WR_LIMIT_EN &= ~BIT(i);
        }
    }
    debug_exit_critical();
}

void debug_clear(void)
{
    debug_enter_critical();

    JL_SDR->DBG_INFO = 0;
    JL_DBG->CON = 0;

    corex2(0)->DBG_CON = 0;
    corex2(0)->DBG_EN = 0;

    corex2(0)->C0_WR_LIMIT_EN = 0;
    corex2(0)->C1_WR_LIMIT_EN = 0;

    corex2(0)->PRP_WR_LIMIT_EN = 0;
    corex2(0)->PRP_WR_ALLOW_EN = 0;
    corex2(0)->PRP_OUTLIM_ERR_EN = 0;

    corex2(0)->DBG_MSG_CLR = 0xffffffff;

    q32DSP(0)->EMU_MSG = -1;
    q32DSP(1)->EMU_MSG = -1;

    debug_index = 0;

    debug_exit_critical();
}

void debug_msg_clear(void)
{
    debug_enter_critical();

    JL_DBG->CON |= BIT(6);
    JL_SDR->DBG_INFO |= BIT(11);
    corex2(0)->DBG_CON |= (BIT(17) | BIT(18));
    corex2(0)->DBG_MSG_CLR = 0xffffffff;

    debug_exit_critical();
}
SET_INTERRUPT
void exception_isr_handler(void)//u32 *sp,u32 *usp,u32 *ssp)
{
    u32 tmp1, tmp2;
    u32 rets, reti, sp, usp, ssp;

    __asm__ volatile("%0 = rets":"=r"(rets));
    __asm__ volatile("%0 = reti":"=r"(reti));
    __asm__ volatile("%0 = sp" : "=r"(sp));
    __asm__ volatile("%0 = usp" : "=r"(usp));
    __asm__ volatile("%0 = ssp" : "=r"(ssp));

    q32DSP(0)->CMD_PAUSE = -1;
    corex2(0)->C0_CON &= ~BIT(3) ;
    corex2(0)->C0_CON |= BIT(1) ;
    printf("\nCPU0 run addr = 0x%x \n", q32DSP(0)->PCRS);
    printf("DEBUG_MSG = 0x%x, EMU_MSG = 0x%x C1_CON=%x\n",
           corex2(0)->DBG_MSG, q32DSP(0)->EMU_MSG, corex2(0)->C1_CON);

    printf("\nRETS=0x%x\n", rets);
    printf("\nRETI=0x%x\n", reti);

    printf("CPU:0x%x -> 0x%x -> 0x%x -> 0x%x\n", q32DSP(0)->ETM_PC3, q32DSP(0)->ETM_PC2, q32DSP(0)->ETM_PC1, q32DSP(0)->ETM_PC0);
    p33_system_reset();
}
void ram_protect_close(void)
{
    u8 i = 0;
    debug_enter_critical();
    pc_rang_limit0((void *)0, (void *)0x1ffffff);
    debug_exit_critical();
}
void debug_init(void)
{
    debug_clear();
    debug_enter_critical();
    corex2(0)->DBG_EN |= (0x3f << 16);
    corex2(0)->DBG_EN |= (0x3  << 4);
    corex2(0)->DBG_CON = 0x7;
    q32DSP(0)->EMU_CON = 0;
    q32DSP(1)->EMU_CON = 0;
    debug_exit_critical();
}
