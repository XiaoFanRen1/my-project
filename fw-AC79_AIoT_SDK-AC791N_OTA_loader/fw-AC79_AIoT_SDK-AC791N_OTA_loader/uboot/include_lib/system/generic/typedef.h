/*************************************************************
File: typedef.h
Author:Juntham
Discriptor:
    数据类型重定义
Version:
Date：
*************************************************************/
#ifndef _typedef_h_
#define _typedef_h_

#ifdef __GNUC__
#define SEC_USED(x)     __attribute__((section(#x),used))
#define sec(x)      __attribute__((section(#x)))
#define SEC(x) 			__attribute__((section(#x)))
#define sec_used(x) __attribute__((section(#x), used))
#define AT(x)       __attribute__((section(#x)))
#define SET(x)      __attribute__((x))
#define _GNU_PACKED_	__attribute__((packed))
#define _INLINE_	    __attribute__((always_inline))
#define _WEAK_	        __attribute__((weak))
#define ALIGNED(x)	    __attribute__((aligned(x)))

#define SET_INTERRUPT   __attribute__((interrupt("")))//SWITCH ,该uboot不用switch方式复用堆栈,因为sdk是用非switch方式定义中断;
#else
#define SEC_USED(x)
#define sec(x)
#define AT(x)
#define SET(x)
#define _GNU_PACKED_
#define _INLINE_
#define _WEAK_
#endif


typedef unsigned char	u8, uint8_t;
typedef char			s8;
typedef unsigned short	u16, uint16_t;
typedef signed short	s16;
typedef unsigned int	u32, tu8, tu16, tbool, tu32, uint32_t;
typedef signed int		s32;
typedef unsigned long long u64;

#include "asm/cpu.h"

//#define NULL    0
#define CPU_SR_ALLOC()

#define	LD_WORD(ptr)		(u16)(*(u16*)(u8*)(ptr))
#define	LD_DWORD(ptr)		(u32)(*(u32*)(u8*)(ptr))
#define	ST_WORD(ptr,val)	*(u16*)(u8*)(ptr)=(u16)(val)
#define	ST_DWORD(ptr,val)	*(u32*)(u8*)(ptr)=(u32)(val)


#define FALSE	0
#define TRUE    1

#define false	0
#define true    1

#ifndef NULL
#define NULL    0
#endif // NULL


#define _banked_func
#define _xdata
#define _root
#define _no_init
#define my_memset memset
#define my_memcpy memcpy
#define _static_  static



#define     BIT(n)	            (1 << (n))
#define     BitSET(REG,POS)     (REG |= (1 << POS))
#define     BitCLR(REG,POS)     (REG &= (~(1 << POS)))
#define     BitXOR(REG,POS)     (REG ^= (~(1 << POS)))
#define     BitCHK_1(REG,POS)   ((REG & (1 << POS)) == (1 << POS))
#define     BitCHK_0(REG,POS)   ((REG & (1 << POS)) == 0x00)
#define     testBit(REG,POS)    (REG & (1 << POS))

#define     clrBit(x,y)         x &= ~(1L << y)
#define     setBit(x,y)         x |= (1L << y)




#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#define readb(addr)   *((unsigned char*)(addr))
#define readw(addr)   *((unsigned short *)(addr))
#define readl(addr)   *((unsigned long*)(addr))

#define writeb(addr, val)  *((unsigned char*)(addr)) = (val)
#define writew(addr, val)  *((unsigned short *)(addr)) = (u16)(val)
#define writel(addr, val)  *((unsigned long*)(addr)) = (val)

#define READ_BT_8( buffer, pos) ( ((u8) buffer[pos]))
#define READ_BT_16( buffer, pos) ( ((u16) buffer[pos]) | (((u16)buffer[pos+1]) << 8))
#define READ_BT_24( buffer, pos) ( ((u32) buffer[pos]) | (((u32)buffer[pos+1]) << 8) | (((u32)buffer[pos+2]) << 16))
#define READ_BT_32( buffer, pos) ( ((u32) buffer[pos]) | (((u32)buffer[pos+1]) << 8) | (((u32)buffer[pos+2]) << 16) | (((u32) buffer[pos+3])) << 24)

#define READ_NET_16( buffer, pos) ( ((u16) buffer[pos+1]) | (((u16)buffer[pos  ]) << 8))
#define READ_NET_32( buffer, pos) ( ((u32) buffer[pos+3]) | (((u32)buffer[pos+2]) << 8) | (((u32)buffer[pos+1]) << 16) | (((u32) buffer[pos])) << 24)


#define ALIGN_4BYTE(size)   ((size+3)&0xfffffffc)

#define SIZEOF_ALIN(var, al)  (((((u32)var)+(al)-1)/(al))*(al))


#define CPU_BIG_EADIN

#if defined(CPU_BIG_EADIN)
#define __cpu_u16(lo, hi)  ((lo)|((hi)<<8))

#elif defined(CPU_LITTLE_EADIN)
#define __cpu_u16(lo, hi)  ((hi)|((lo)<<8))
#else
#error "undefine cpu eadin"
#endif

extern void cpu_reset(void);
/* bt_printf("%s %d", __FILE__, __LINE__); \
bt_printf("ASSERT-FAILD: "#a"\n"__VA_ARGS__); \ */
#define ARRAY_SIZE(array)  (sizeof(array)/sizeof(array[0]))
#if 1
#define ASSERT(a,...)   \
		do { \
			if(!(a)){ \
				u32 reti_addr,rets_addr; \
				__asm__ volatile("%0 = reti ;" : "=r"(reti_addr)); \
				__asm__ volatile("%0 = rets ;" : "=r"(rets_addr)); \
				printf("ASSERT:%s reti:%x rets:%x\n",__FUNCTION__,reti_addr,rets_addr); \
				cpu_reset(); \
			} \
		}while(0);
#else

extern void cpu_reset(void);
#define ASSERT(a,...)   \
		do { \
			if(!(a)){ \
				cpu_reset(); \
			}\
		}while(0)
#endif

#define SFR(sfr, start, len, dat) (sfr = (sfr & ~((~(0xffffffff << (len))) << (start))) | (((dat) & (~(0xffffffff << (len)))) << (start)))

extern void vPortEnterCritical(void);
extern void vPortExitCritical(void);

#define OS_SR_ALLOC()

#define CPU_INT_DIS()  vPortEnterCritical();
#define CPU_INT_EN()   vPortExitCritical();

#include "generic/errno_base.h"
#include "string.h"
#include "strings.h"
#include "system/malloc.h"

//extern void *memset(void *, int, long unsigned int);
//extern int   memcmp(const void *, const void *, long unsigned int);

void delay(unsigned int);

#endif
