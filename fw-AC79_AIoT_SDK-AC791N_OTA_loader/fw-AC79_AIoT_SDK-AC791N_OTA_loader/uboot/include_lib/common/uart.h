#ifndef _UART_
#define _UART_

#include "typedef.h"

/*强制声明*/
extern int   memcmp(const void *, const void *, long unsigned int);
extern void *memcpy(void *, const void *, long unsigned int);
extern void *memset(void *, int, long unsigned int);
extern int   strcmp(const char *, const char *);
extern int   printf(const char *format, ...);

// extern func
extern void putbyte(char a);
extern void putchar(char a);
extern void put_u4hex(u8 dat);
extern void put_u32hex(u32 dat);
extern void put_u16hex(u16 dat);
extern void put_u8hex(u8 dat);
extern void put_buf(u8 *buf, u32 len);
extern void printf_buf(u8 *buf, u32 len);
extern int  puts(const char *out);

void uart_init(const char *tx_pin, const char *rx_pin, u32 baud);

#endif

