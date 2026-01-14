#ifndef __RAND64_H__
#define __RAND64_H__

#include "typedef.h"

// func declare
u8 get_rand64_l(void);
u8 get_rand64_h(void);
void get_random_number(u8 *ptr, u8 len);

void pseudo_random_generate(u8 *buf, u16 len);
#endif //#define __RAND64_H__
