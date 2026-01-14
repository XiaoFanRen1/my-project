#ifndef _PRINTF_DEFINED_
#define _PRINTF_DEFINED_

#include <stdarg.h>
#include <typedef.h>

int printf(const char *format, ...);
int sprintf(char *out, const char *format, ...);
#define line_inf printf("%s %s %d \r\n" ,__FILE__, __func__ , __LINE__) ;

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


#endif
