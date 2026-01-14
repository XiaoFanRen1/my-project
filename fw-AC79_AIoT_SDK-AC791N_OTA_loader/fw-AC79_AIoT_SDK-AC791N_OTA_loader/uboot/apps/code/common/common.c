#include "config.h"
#include "hw_cpu.h"
#include "csfr.h"

AT(.volatile_ram_code)
void delay(volatile u32 t)
{
    while (t--) {
        asm("nop");
    }
}





