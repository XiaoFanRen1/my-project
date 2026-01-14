#ifndef SYS_TIMER_H
#define SYS_TIMER_H

#include "typedef.h"
#include "common/list.h"

#if (defined(CONFIG_CPU_BR22) && (BT_UPDATA_MODULE_CONTROL == 1))      //BR22 btctler.a使用Sdk编译
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

void sys_timer_schedule(void);
void sys_timer_delay_schedule(void);
void sys_timer_set_user(struct sys_timer *timer, u32 user);
u32 sys_timer_get_user(struct sys_timer *timer);
void sys_timer_del_schedule(void);
void loop_timer_schedule(void);
void sys_timer_del(u16 t);
int sys_timer_modify(u16 id, u32 msec);
u16 sys_timeout_add(void *priv, void (*func)(void *priv), u32 msec);
void sys_timeout_del(u16 t);
u16 sys_timer_add(void *priv, void (*func)(void *priv), u32 msec);

#else

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
};

bool __timer_find(struct sys_timer *timer);
void sys_timer_var_init();
u16 sys_timer_add(void *priv, void (*func)(void *priv), u32 msec);
void sys_timer_schedule(void);
void sys_timer_delay_schedule(void);
void sys_timer_set_user(struct sys_timer *timer, u32 user);
u32 sys_timer_get_user(struct sys_timer *timer);
void sys_timer_del_schedule(void);
void loop_timer_schedule(void);
void sys_timer_del(u16 t);
int sys_timer_modify(u16 id, u32 msec);
u16 sys_timeout_add(void *priv, void (*func)(void *priv), u32 msec);
void sys_timeout_del(u16 t);
void sys_timer_re_run(u16 id);
void sys_timer_set_user_data(u16 id, void *priv);
void sys_timer_remove(sys_timer timer);
sys_timer sys_timer_register(u32 msec, void (*callback)(void *));
sys_timer sys_timer_register_periodic(u32 msec, void (*callback)(void *));
void sys_timer_set_context(sys_timer timer, void *context);
void sys_timer_reset(sys_timer timer);
void sys_timer_change_period(sys_timer timer, u32 msec);
#endif


#endif      //endif SYS_TIMER_H

