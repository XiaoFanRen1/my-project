#include "system/includes.h"
#include "os/os_api.h"
#include "asm/hwi.h"
#include "wifi/wifi_connect.h"
#include "utils/syscfg_id.h"
#include "logic/boot.h"
#include "FreeRTOS/FreeRTOSConfig.h"
#include "sys/time.h"

/*中断列表 */
const struct irq_info irq_info_table[] = {
    //中断号   //优先级0-7   //注册的cpu(0或1)
#if CPU_CORE_NUM == 1
    { IRQ_SOFT5_IDX,      7,   0    }, //此中断强制注册到cpu0
    { IRQ_SOFT4_IDX,      7,   1    }, //此中断强制注册到cpu1
    { -2,     			-2,   -2   },//如果加入了该行, 那么只有该行之前的中断注册到对应核, 其他所有中断强制注册到CPU0
#endif
    { -1,     -1,   -1    },
};
#define NET_OTA_STK_SIZE 1024
#define NET_OTA_Q_SIZE 0
static u8 net_ota_tcb_stk_q[sizeof(StaticTask_t) + NET_OTA_STK_SIZE * 4 + sizeof(struct task_queue) + NET_OTA_Q_SIZE] ALIGNED(4);

/*任务列表 */
const struct task_info task_info_table[] = {
    {"app_core",            5,     1024,    64},
    {"sys_event",           3,     512,	     0},
    {"systimer",            6,     256,	     0},
    {"sys_timer",           4,     512,	   128},
    {"net_ota",            	5,     NET_OTA_STK_SIZE, 0, net_ota_tcb_stk_q},

#ifdef CONFIG_WIFI_ENABLE
    {"tasklet",             5,     1600,     0},//通过调节任务优先级平衡WIFI收发占据总CPU的比重
    {"RTMLTask",        	6,     800,  	 0},//任务名称长度<12
    {"RTCQTask",        	6,     400,   	 0},//任务名称长度<12
    {"WLRQTask",    		6,     356,  	 0},//任务名称长度<12
    {"WLQUTask",  		  	6,     356,  	 0},//任务名称长度<12
    {"tcpiptask",        	6,     800,	     0},//任务名称长度<12
#endif
    {0, 0, 0, 0, 0},
};

//===============================================系统补充函数===============================================//
int syscfg_read(u16 item_id, void *buf, u16 len)
{
    return -1;
}
int syscfg_write(u16 item_id, void *buf, u16 len)
{
    return 0;
}
int syscfg_dma_write(u16 item_id, void *buf, u16 len)
{
    return 0;
}
void *get_save_frame_ptr()
{
    return NULL;
}
u32 OSGetTime()
{
    return jiffies_to_msecs(jiffies);
}
unsigned int random32(int type)
{
    return JL_RAND->R64L;
}
unsigned int time_lapse(unsigned int *handle, unsigned int time_out)//2^32/10    00/60/60/24 后超时
{
    unsigned int t, t2;
    extern unsigned int timer_get_ms();

    if (*handle == 0) {
        *handle = (unsigned int)timer_get_ms();
        return 0;
    }

    t = (unsigned int)timer_get_ms();
    t2 = t - *handle;

    if (t2 > time_out) {
        *handle = t;
        return t2;
    }

    return 0;
}
void delay_us(unsigned int us)
{
    int clk_get(const char *name);

    if (JL_SFC->CON & BIT(0)) {
        us *= clk_get("sys") / 1000000 / 8;
    } else {
        us *= clk_get("sys") / 1000000 / 4;
    }

    while (us--) {
        asm volatile("nop");
    }
}

unsigned int timer_get_ms()
{
    return jiffies_to_msecs(jiffies);
}
unsigned int timer_get_sec(void)
{
    return jiffies_to_msecs(jiffies) / 1000;
}
int gettimeofday(struct timeval *tv, void *tz)
{
    if (tv) {
        time_t t = (time_t)timer_get_ms();

        tv->tv_sec = t / 1000;
        tv->tv_usec = t % 1000 * 1000;
    }
    if (tz) {
        ((struct timezone *)tz)->tz_minuteswest = 0;
        ((struct timezone *)tz)->tz_dsttime = 0;
    }

    return 0;
}
int bytecmp(unsigned char *p, unsigned char ch, unsigned int num)
{
    while (num-- > 0) {
        if (*p++ != ch) {
            return -1;
        }
    }
    return 0;
}
//=====================================cach ram================================================//
static void *pcach_mem = NULL;
static u32 cach_mem_size = 0;
static u32 cach_mem_addr = 0;
SEC_USED(.volatile_ram_code) void cache_ram_config(void)
{
#if SDRAM_SIZE /*使用sdram为内存*/
#define __SDRAM_ADDR    ((u8*)0x4000000)
#define SDRAM_ADDR(addr)    ((void*)(__SDRAM_ADDR+(addr)))
    cach_mem_size = SDRAM_SIZE;
    cach_mem_addr = SDRAM_ADDR(0);
#else
#ifndef CONFIG_WL_AP_ENABLE
    u8 FREE_DACHE_WAY = 7;
    u8 FREE_IACHE_WAY = 7;
    int i;
    volatile int *ptr;
    u8 free_dcache_way = FREE_DACHE_WAY, free_icache_way = FREE_IACHE_WAY;

    if (free_dcache_way == 0 && free_icache_way == 0) {
        return;
    }
    cach_mem_size = free_dcache_way * 4096 + free_icache_way * 4096 - 512;
    cach_mem_addr = 0x1f20000 + ((8 - free_icache_way) * 4096);
    while (!(corex2(0)->CACHE_CON & BIT(14)));
    corex2(0)->CACHE_CON &= ~(3 << 8); //disable ddr sfc cache

    corex2(0)->DCACHE_WAY &= ~0XFFFFFF;
    for (i = 0; i < 8 - free_dcache_way; i++) {
        corex2(0)->DCACHE_WAY |= (1 << (7 - i)) | (1 << (15 - i)) | (1 << (23 - i));
    }

    corex2(0)->ICACHE_WAY &= ~0XFFFF;
    for (i = 0; i < 8 - free_icache_way; i++) {
        corex2(0)->ICACHE_WAY |= (1 << (i)) | (1 << (8 + i));
    }
#if 1
    for (ptr = (int *)0x1f00000; (int)ptr < 0x1f04000; ptr++) {
        *ptr = 0;    // DTAG0 DTAG1 clr
    }

    for (ptr = (int *)0x1f08000; (int)ptr < 0x1f0a000; ptr++) {
        *ptr = 0;    // ITAG clr
    }

    for (ptr = (int *)0x1f0a000; (int)ptr < 0x1f0a200; ptr++) {
        *ptr = 0;    // DRPTAG clr
    }

    for (ptr = (int *)0x1f0b000; (int)ptr < 0x1f0b200; ptr++) {
        *ptr = 0;    // IRPTAG clr
    }
    corex2(0)->CACHE_CON |= BIT(8); //RO CACHE EN
#endif
#endif
#endif
}
int lbuf_remain_space(struct lbuff_head *head);
static void cache_ram_init(void)
{
    cache_ram_config();
    if (cach_mem_addr) {
        pcach_mem = lbuf_init((void *)cach_mem_addr, cach_mem_size, 4, 0);
    }
}
static void *cach_mem_malloc(unsigned int size)
{
    if (pcach_mem && cach_mem_addr) {
        return lbuf_alloc(pcach_mem, size);;
    }
    return NULL;
}
static unsigned char cach_mem_free(void *ptr)
{
    if (pcach_mem && cach_mem_addr) {
        if ((unsigned int)ptr >= cach_mem_addr && (unsigned int)ptr < (cach_mem_addr + cach_mem_size)) {
            lbuf_free(ptr);
            return 1;
        }
    }
    return 0;
}
static unsigned int cach_mem_free_space(void)
{
    if (pcach_mem) {
        return lbuf_remain_space(pcach_mem);
    }
    return 0;
}

//=====================================app ram, uboot所占据的区域================================================//
static void *pmem_ptr = 0;
#define APP_MEM_SIZE		(42 * 1024)
#define APP_MEM_ADDR_START	(0x1C00000)
#define APP_MEM_ADDR_END	(0x1C00000 + APP_MEM_SIZE)
void app_mem_init(void)
{
#ifndef CONFIG_WL_AP_ENABLE
    memset((void *)APP_MEM_ADDR_START, 0, APP_MEM_SIZE);
    pmem_ptr = lbuf_init((void *)APP_MEM_ADDR_START, APP_MEM_SIZE, 4, 0);
    cache_ram_init();
#endif
}
void *app_mem_malloc(unsigned int size)
{
    void *p = cach_mem_malloc(size);
    if (p) {
        return p;
    }
    if (pmem_ptr) {
        return lbuf_alloc(pmem_ptr, size);;
    }
    return NULL;
}
unsigned char app_mem_free(void *ptr)
{
    if (cach_mem_free(ptr)) {
        return 1;
    }
    if (pmem_ptr) {
        if ((unsigned int)ptr >= APP_MEM_ADDR_START && (unsigned int)ptr < APP_MEM_ADDR_END) {
            lbuf_free(ptr);
            return 1;
        }
    }
    return 0;
}
unsigned int app_mem_free_space(void)
{
    if (pmem_ptr) {
        return lbuf_remain_space(pmem_ptr);
    }
    return 0;
}
//===============================================系统补充函数===============================================//
void *pvPortMalloc(unsigned long  xWantedSize)
{
    return malloc(xWantedSize);
}
void vPortFree(void *pv)
{
    free(pv);
}

void *kmalloc(unsigned long size, int flags)
{
    return malloc(size);
}
void *realloc(void *pv, unsigned long size)
{
    if (pv) {
        free(pv);
    }
    return malloc(size);
}
void *vmalloc(unsigned long size)
{
    return malloc(size);
}
void vfree(void *addr)
{
    free(addr);
}
void *kzalloc(unsigned int len, int a)
{
    return zalloc(len * a);
}
void *calloc(unsigned long len, unsigned long a)
{
    return kzalloc(len,  a);
}
void *_calloc_r(unsigned long len, unsigned long a)
{
    return calloc(len, a);
}
void *_malloc_r(unsigned int sz)
{
    return malloc(sz);
}
void kfree(void *p)
{
    free(p);
}
unsigned int if_nametoindex(const char *ifname)
{
    return 1;
}
char *if_indextoname(unsigned ifindex, char *ifname)
{
    return ifname;
}
//===============================================系统补充函数===============================================//
void vm_find(void);
void net_ota_update(void);
int demo_wifi(void);
int thread_fork_init(void);
int mem_free_space(void);
void get_task_state(void *parm);
int net_ota_update_read_size_check(void);

static int app_core(void)
{
    int cnt = 0;
    int check, check_err_cnt = 0;
    int check_last = 0;
    thread_fork_init();
    demo_wifi();
    os_time_dly(500);
#if (configGENERATE_RUN_TIME_STATS==1)
    get_task_state(NULL); //1分钟以内调用一次才准确
#endif
    while (1) {
        os_time_dly(100);
        if (cnt++ % 5 == 0) {
            printf("free_space = %dK \n\n", (mem_free_space() + cach_mem_free_space() + app_mem_free_space()) / 1024);
        }
        check = net_ota_update_read_size_check();
        if (check) {
            if (!check_last) {
                check_last = check;
                check_err_cnt = 0;
            } else if (check_last != check) {
                check_last = check;
                check_err_cnt = 0;
            } else {
                check_err_cnt++;
            }
        }
        if (check_err_cnt && check_err_cnt % (60 * 1) == 0) {//启动升级后，1分钟没有读取文件重启系统
            puts("net_ota no read file err\n");
            os_time_dly(100);
            cpu_reset();
        }
    }
}

/*
 * 应用程序主函数
 */
void app_main(void)
{
    app_mem_init();
    vm_find();
    task_create(app_core, NULL, "app_core");
    task_create(net_ota_update, NULL, "net_ota");
}

