/*********************************************************************************************
    *   Filename        : MEMConfig.h

    *   Description     :

    *   Author          : Bingquan

    *   Email           : bingquan_cai@zh-jieli.com

    *   Last modifiled  : 2018-06-07 14:12

    *   Copyright:(c)JIELI  2011-2017  @ , All Rights Reserved.
*********************************************************************************************/
#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "typedef.h"

extern u32 HEAP_BEGIN   ;
extern u32 HEAP_END  ;

#define heap_begin  HEAP_BEGIN
#define heap_end    HEAP_END

extern u32 UPDATE_BEG;
#define UPDATE_FLAG_ADDR		((u32)(&UPDATE_BEG) + 0x8)//(0x18000UL - 0x78)
#define UART_UPDATE_FLAG_ADDR  	(u32)(&UPDATE_BEG)
// extern unsigned char heap_begin, heap_end;

#define configUSE_VIRTUAL_MEM   (1)
#define USE_VIRTUAL_MEM         (1)

//物理内存映射范围(并非实际内存范围)
#define PHYSIC_RAM_START        (0x00000)
#define MAX_PHYSIC_RAM_SIZE     (200*1024)
#define PHYSIC_RAM_END          (PHYSIC_RAM_START + MAX_PHYSIC_RAM_SIZE)

//物理内存粒度
#define PAGE_SHIFT              7
#define PAGE_SIZE               (1UL << PAGE_SHIFT)

//内存对齐
#define PAGE_MASK               (~(PAGE_SIZE-1))
#define PAGE_CUT                ((PAGE_SIZE-1))
#define PAGE_ALIGN(addr)        (((addr)+PAGE_SIZE-1)&PAGE_MASK)
#define PAGE_ALIGN_BACKWARD(addr)      (addr & PAGE_MASK)

//虚拟内存起始
#define V_MEM_START             (0x200000)


//任务堆栈内存起始
#define STACK_START             (V_MEM_START)
#define STACK_TASK_MAX_SIZE     (1024*4)
#define STACK_TASK_PAGE         (STACK_TASK_MAX_SIZE/PAGE_SIZE)

//任务堆栈大小
#define configMAX_STACK_NUMBER          (5 * 3)
#define configTOTAL_STACK_HEAP_SIZE     (configMAX_STACK_NUMBER * STACK_TASK_MAX_SIZE)

//根据实际物理内存配置虚拟内存
#define configMMU_TLB_SIZE      ((64+64+8)*1024/PAGE_SIZE*2)

struct mmu_tlb_tag {
    u16 phy_adr: 13;
    u16 page_en: 1;
    u16 rsv: 2;
} __attribute__((__packed__));

void vStackMemoryInit(void);

#endif


