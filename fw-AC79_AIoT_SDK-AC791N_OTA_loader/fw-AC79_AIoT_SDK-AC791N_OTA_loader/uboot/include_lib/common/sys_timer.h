#ifndef SYS_TIMER_H
#define SYS_TIMER_H


#include "typedef.h"
#include "common/list.h"

#if 1
//#if defined(CONFIG_CPU_BR18) || defined(CONFIG_CPU_BR26) || defined(CONFIG_CPU_BR23) || (defined(CONFIG_CPU_BR22) && (BT_UPDATA_MODULE_CONTROL!=1)) || defined(CONFIG_CPU_BD29) || defined(CONFIG_CPU_BR21) || defined(CONFIG_CPU_BR25) || defined(CONFIG_CPU_BR30)
typedef int sys_timer;


struct sys_timer {
    struct list_head entry;
    void (*func)(void *priv);
    void *priv;
    u32 jiffies;
    u32 msec: 24;
    u32 del: 1;
    u32 timeout: 1;
    u16 id;
    u8 used;
    // u32 user;
    // u32 delay_do;
    // u32 loop;
};

void sys_timer_var_init();
bool __timer_find(struct sys_timer *timer);
u16 sys_timer_add(void *priv, void (*func)(void *priv), u32 msec);
void sys_timer_schedule(void);
void sys_timer_delay_schedule(void);
void sys_timer_set_user(struct sys_timer *timer, u32 user);
u32 sys_timer_get_user(struct sys_timer *timer);
void sys_timer_del_schedule(void);
void loop_timer_schedule(void);
void sys_timer_re_run(u16 id);
#else
/* #endif

#ifdef CONFIG_CPU_BR22 */

typedef int sys_timer;

struct sys_timer {
struct list_head entry;
void (*fun)(struct sys_timer *);
u32 jiffies;
u32 user;
u32 delay_do;
u32 loop;
};



bool __timer_find(struct sys_timer *timer);
void sys_timer_register(struct sys_timer *timer, u32 msec,
void (*fun)(struct sys_timer *timer), u8 delay_do);
void sys_timer_remove(struct sys_timer *timer);
void sys_timer_reset(struct sys_timer *timer, u32 msec);

void sys_timer_init(struct sys_timer *timer);
void sys_timer_schedule(void);
void sys_timer_delay_schedule(void);
void sys_timer_set_user(struct sys_timer *timer, u32 user);
u32 sys_timer_get_user(struct sys_timer *timer);
void sys_timer_delay_handler_register(void(*handler)());
void sys_timer_del_schedule(void);
void loop_timer_schedule(void);
#endif
#endif

