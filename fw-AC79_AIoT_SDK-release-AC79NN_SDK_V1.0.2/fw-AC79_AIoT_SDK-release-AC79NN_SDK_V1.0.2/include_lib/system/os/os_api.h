#ifndef   OS_API_H
#define   OS_API_H


#ifdef __cplusplus
extern "C"
{
#endif




#include "generic/typedef.h"
//#include "generic/list.h"
#include "os/os_cpu.h"
#include "os/os_error.h"
#include "os/os_type.h"



#define Q_MSG           0x100000
#define Q_EVENT         0x200000
#define Q_CALLBACK      0x300000
#define Q_USER          0x400000

#define  OS_DEL_NO_PEND               0u
#define  OS_DEL_ALWAYS                1u

#define  OS_TASK_DEL_REQ           0x01u
#define  OS_TASK_DEL_RES           0x02u
#define  OS_TASK_DEL_OK            0x03u


#define  OS_TASK_SELF        (char *)0x1
#define  OS_TASK_FATHER      (char *)0x2


#define OS_MSG_KEY          1
#define OS_MSG_TOUCH        2
#define OS_MSG_EVENT        3
#define OS_MSG_CALLBACK     4
#define OS_MSG_DEL_REQ      5
#define OS_MSG_MSG          6


/*struct os_msg {
    int type;
    int msg;
    int err;
    u8 deleted;
    char ref;
    OS_SEM   sem;
    OS_MUTEX mutex;
    struct list_head entry;
    u32 data[0];
};*/

/*struct os_msg_callback {
    struct os_msg msg;
    u8 argc;
    u8 sync;
    u8 arg_exp;
    int argv[8];
    void *function;
};*/

#ifdef CONFIG_CPU_BR25
void *os_init();
#else
void os_init();
#endif
void os_start(void);
void os_init_tick(int);

int os_task_create(void (*task)(void *p_arg),
                   void *p_arg,
                   u8 prio,
                   u32 stksize,
                   int qsize,
                   const char *name);

int os_task_create_static(void (*task_func)(void *p_arg),
                          void *p_arg,
                          u8 prio,
                          u32 stksize,
                          int qsize,
                          const char *name,
                          u8 *tcb_stk_q);
const char *os_current_task(void);
unsigned int os_current_task_prio(void);
unsigned int os_current_task_stask_size(void);
unsigned int os_time_get(void);

void os_task_exit();

int os_task_del_req(const char *name);

int os_task_del_res(const char *name);

int os_task_del(const char *name);


void os_time_dly(int time_tick);


int __os_taskq_post(const char *name, int type, int argc, int *argv);

int __os_taskq_pend(int *argv, int argc, int tick);

int os_taskq_accept(int, int *);

int os_taskq_pend(const char *fmt, int *argv, int argc);

int os_task_pend(const char *fmt, int *argv, int argc);

int os_taskq_post(const char *name, int argc, ...);

int os_taskq_del(const char *name, int type);

int __os_taskq_del(const char *name, int type, int argc, int *msg);

int os_taskq_post_type(const char *name, int type, int argc, int *argv);

// int task_queue_post_msg(const char *name, void *data, int len);
int task_queue_post_event(const char *name, void *data, int len);

int os_taskq_post_msg(const char *name, int argc, ...);

int os_taskq_post_event(const char *name, int argc, ...);

int os_taskq_del_type(const char *name, int type);

int os_taskq_waiting(const char *name);

int os_taskq_flush();

int os_sem_create(OS_SEM *, int);

int os_sem_accept(OS_SEM *);

int os_sem_pend(OS_SEM *, int timeout);

int os_sem_post(OS_SEM *);

int os_sem_del(OS_SEM *, int block);

int os_sem_set(OS_SEM *, u16 cnt);

int os_sem_valid(OS_SEM *);

int os_sem_query(OS_SEM *);

int os_mutex_create(OS_MUTEX *);

/* ----------------------------------------------------------------------------*/
/**
 * @brief 等待互斥量
 * @param[in]  mutex: 互斥量
 * @param[in]  timeout: 0:阻塞等待（注意:不能传入portMAX_DELAY）; -1:该参数已经弃用，非阻塞请使用os_mutex_accept
 * @return 0: 成功
 * @return OS_TIMEOUT: 等待超时
 * @note	不可在中断函数或者临界区调用
 */
/* ----------------------------------------------------------------------------*/
int os_mutex_pend(OS_MUTEX *, int timeout);

/* ----------------------------------------------------------------------------*/
/**
 * @brief 获取互斥量（非堵塞）
 * @param[in]  mutex: 互斥量
 * @return 0: 成功
 * @return OS_TIMEOUT: 获取失败
 * @note	不可在中断函数或者临界区调用
 */
/* ----------------------------------------------------------------------------*/
int os_mutex_accept(OS_MUTEX *p_mutex);

int os_mutex_post(OS_MUTEX *);

int os_mutex_del(OS_MUTEX *, int block);

int os_mutex_valid(OS_MUTEX *);

/*struct os_msg *os_message_create(int size);

int os_message_receive(struct os_msg **msg, int block_time);

int os_message_send(const char *task_name, struct os_msg *msg, int msgflg);

int os_message_delete(struct os_msg *msg);*/



int os_q_create(OS_QUEUE *pevent, /*void **start, */QS size);

int os_q_create_static(OS_QUEUE *pevent, void *storage, QS size);

int os_q_del_static(OS_QUEUE *pevent, u8 opt);

int os_q_del(OS_QUEUE *pevent, u8 opt);

int os_q_flush(OS_QUEUE *pevent);

int os_q_pend(OS_QUEUE *pevent, int timeout, void *msg); //timeout: 0是一直等待，-1不等待

int os_q_post(OS_QUEUE *pevent, void *msg);

int os_q_peek(OS_QUEUE *pevent, void *msg, int timeout_ms);

int os_q_recv(OS_QUEUE *pevent, void *msg, int timeout_ms);

int os_q_post_to_back(OS_QUEUE *pevent, void *msg, int timeout_ms);

int os_q_post_to_front(OS_QUEUE *pevent, void *msg, int timeout_ms);

int os_q_query(OS_QUEUE *pevent);

int os_q_valid(OS_QUEUE *pevent);

int os_q_is_full(OS_QUEUE *pevent);

int os_eventgroup_create(OS_EVENT_GRP **event, u32 flags);

int os_eventgroup_delete(OS_EVENT_GRP *event, u8 opt);

int os_eventgroup_set_bits(OS_EVENT_GRP *event, int bits);

int os_eventgroup_clear_bits(OS_EVENT_GRP *event, int bits);

int os_eventgroup_wait_bits(OS_EVENT_GRP *event, int bits, int isClearOnExit, int isWaitForAllBits, int waitms);

int os_tasks_num_query(void);


void os_task_suspend_all(const char (*Otherwise)[][configMAX_TASK_NAME_LEN], int array_num);

void os_task_resume_all(void);

void os_task_resume(const char *name);

void os_task_suspend(const char *name);





/*! \addtogroup OS_THREAD
 *  @ingroup OS
 *  @brief	 OS thread api
 *  @{
 */


/**
 *  @brief enum kill_mode
 *  used by thread_kill
 */
enum kill_mode {
    KILL_WAIT,		/*!<WAIT */
    KILL_REQ,		/*!<REQUEST	*/
    KILL_FORCE,		/*!<FORCE ON */
};



/*----------------------------------------------------------------------------*/
/**@brief  创建thread_fork_kill线程
   @return 0: 成功
   @return other: enum OS_ERROR_CODES
   @note   该线程用来Kill掉其他线程，其任务优先级最高
*/
/*----------------------------------------------------------------------------*/
int thread_fork_init(void);

/*----------------------------------------------------------------------------*/
/**@brief  创建一个线程
   @param[in] thread_name: 标识此线程的字符串
   @param[in] prio: 优先级
   @param[in] stk_size: 堆栈空间的大小（以四个字节为一个单位）
   @param[in] q_size: 消息队列空间的大小（以四个字节为一个单位）
   @param[out] pid: 保存新线程的pid
   @param[in] func: 入口功能函数
   @param[in] parm: 入口函数的传入参数
   @return 0: 成功
   @return -1: 参数错误
   @return other: enum OS_ERROR_CODES
   @note   由于系统靠thread_name区分不同任务，所以请保证thread_name的唯一
   @note   replace pthread_create function
*/
/*----------------------------------------------------------------------------*/
int thread_rpc(int (*condition)(void), const char *thread_name, int prio, int stk_size, u32 q_size, int *pid, void (*func)(void *), void *parm, void (*thread_complete_cb)(void *), void *thread_complete_cb_parm);
int thread_fork(const char *thread_name, int prio, int stk_size, u32 q_size, int *pid, void (*func)(void *), void *parm);

/*----------------------------------------------------------------------------*/
/**@brief  杀死一个线程
   @param[in]  pid: 指向需要杀死的pid号的指针
   @param[in]  mode: enum kill_mode
   @return 0: 成功
   @return -1: 参数错误
   @return other: enum OS_ERROR_CODES
   @note   推荐使用KILL_WAIT
   @note   replace pthread_kill function
*/
/*----------------------------------------------------------------------------*/
void thread_kill(volatile int *pid, enum kill_mode mode);

/*----------------------------------------------------------------------------*/
/**@brief  获取正在运行的线程是否有其他线程请求自己杀死
   @return 0: 没有杀死请求
   @return 1: 有杀死请求
   @note
*/
/*----------------------------------------------------------------------------*/
int thread_kill_req(void);

/*----------------------------------------------------------------------------*/
/**@brief  获取指向当前线程pid的指针
   @return 指向当前线程pid的指针
   @note
*/
/*----------------------------------------------------------------------------*/
int *get_cur_thread_pid(void);

/*----------------------------------------------------------------------------*/
/**@brief  微秒级延时函数
   @param[in] t: us
   @note   TODO
*/
/*----------------------------------------------------------------------------*/
void udelay(unsigned int t);

/*----------------------------------------------------------------------------*/
/**@brief  毫秒级延时函数
   @param[in] ms: ms
   @note
*/
/*----------------------------------------------------------------------------*/
void mdelay(unsigned int ms);

/*----------------------------------------------------------------------------*/
/**@brief  毫秒级延时函数
   @param[in] ms: ms
   @note
*/
/*----------------------------------------------------------------------------*/
void msleep(unsigned int ms);

/*----------------------------------------------------------------------------*/
/**@brief  初始化信号量
   @param[out]  sem: 保存新建信号量的信息
   @param[in]  cnt: 计数器的初始值
   @return enum OS_ERROR_CODES
   @note
*/
/*----------------------------------------------------------------------------*/
int sema_init(int *sem, int cnt);

/*----------------------------------------------------------------------------*/
/**@brief  等待信号量
   @param[in]  sem: 指向需要等待的信号量的指针
   @param[in]  timeout: 等待超时（时钟节拍），取0时无限等待
   @return enum OS_ERROR_CODES
   @note
*/
/*----------------------------------------------------------------------------*/
int sem_pend(int *sem, unsigned int timeout);

/*----------------------------------------------------------------------------*/
/**@brief  发送信号量
   @param[in]  sem: 指向需要发送的信号量的指针
   @return enum OS_ERROR_CODES
   @note
*/
/*----------------------------------------------------------------------------*/
int sema_post(int *sem);

/*----------------------------------------------------------------------------*/
/**@brief  设置信号量的计数器的值
   @param[in]  sem: 指向需要设置的信号量的指针
   @param[in]  init_val: 计数器的设置值
   @return enum OS_ERROR_CODES
   @note
*/
/*----------------------------------------------------------------------------*/
int sem_set(int *sem, int init_val);

/*----------------------------------------------------------------------------*/
/**@brief  销毁信号量
   @param[in]  sem: 指向需要销毁的信号量的指针
   @return enum OS_ERROR_CODES
   @note
*/
/*----------------------------------------------------------------------------*/
int sem_del(int *sem);

/*----------------------------------------------------------------------------*/
/**@brief  查询信号量计数值
   @param[in]  sem: 指向需要查询的信号量的指针
   @return enum OS_ERROR_CODES
   @note
*/
/*----------------------------------------------------------------------------*/
int sem_query(int *sem);

/*----------------------------------------------------------------------------*/
/**@brief  检测信号量是否有效
   @param[out]  __msg: 保存获取到的消息
   @return 1: 成功
   @return 0: 失败
   @note   无限等待
*/
int sem_valid(int *sem);


#ifdef __cplusplus
}
#endif
#endif
