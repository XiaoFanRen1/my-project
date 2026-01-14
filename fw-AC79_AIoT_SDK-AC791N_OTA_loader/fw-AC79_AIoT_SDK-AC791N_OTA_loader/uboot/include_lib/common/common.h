#ifndef _COMMON_
#define _COMMON_

#define my_memzero(addr,len)    memset((addr),0x00,(len))
extern volatile u32 delay_cnt;

// extern func
extern void CLR_WDT(void);
extern void delay(volatile u32 t);
extern void ENABLE_INT(void);
extern void DISABLE_INT(void);

extern void delay_2ms(int cnt);

#endif
