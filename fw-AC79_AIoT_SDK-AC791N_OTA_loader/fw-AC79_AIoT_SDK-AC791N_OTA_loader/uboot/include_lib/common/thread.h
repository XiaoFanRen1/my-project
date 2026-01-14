#ifndef THREAD_H
#define THREAD_H

#include "typedef.h"
#include "common/list.h"

#if 0
#define PRIORITY_NUM   4


#if THREAD_DEBUG_EN
struct thread {
    struct list_head entry;
    void (*fun)(struct thread *);
    u8 priority;
    //resume
    u8 resume_cnt;
    //suspend
    u8 suspend_timeout;
};
#else
struct thread {
    struct list_head entry;
    void (*fun)(struct thread *);
    u8 priority;
    u8 run;
    u8 timeout;
};
#endif // #if THREAD_DEBUG_EN


struct thread_interface {
    int (*os_create)(void (*fun)(u8), u8);
    int (*os_delete)(u8);
    int (*os_suspend)(u8, u8);
    int (*os_resume)(u8);
};


void thread_init(const struct thread_interface *instance);


int thread_create(struct thread *th, void (*fun)(struct thread *), u8 priority);


void thread_suspend(struct thread *th, int timeout);


void thread_resume(struct thread *th);


int thread_delete(struct thread *th);

#else
#define PRIORITY_NUM   4

struct thread_owner {
    struct list_head head;
    int (*create)(void (*fun)(struct thread_owner *), u8);
    int (*delete)(u8);
    int (*suspend)(u8, u8);
    int (*resume)(void);
};


struct thread {
    struct list_head entry;
    struct thread_owner *owner;
    char *name;
    void (*fun)(struct thread *);
    u8 priority;
    //resume
    u8 resume_cnt;
    //suspend
    u8 suspend_timeout;
};

int thread_owner_init(struct thread_owner *owner);

int thread_create(struct thread *th, char *name, void (*fun)(struct thread *), struct thread_owner *owner);

void thread_suspend(struct thread *th, int timeout);

void thread_resume(struct thread *th);

int thread_delete(struct thread *th);

void thread_run(struct thread_owner *owner);


#endif

#endif

