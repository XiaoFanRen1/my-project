#ifndef LBUF_H
#define LBUF_H

#include "typedef.h"
#include "generic/list.h"
#include "spinlock.h"
#if 0
struct lbuff_head {
    struct list_head head;
    struct list_head free;
    int magic_a;
    spinlock_t lock;
    u8 align;
    u16 priv_len;
    u32 total_size;
    u32 last_addr;
    void *priv;
    int magic_b;
    //u16 index;
};

struct lbuff_state {
    u32 avaliable;
    u32 fragment;
    u32 max_continue_len;
    int num;
};

int lbuf_get_chunk_num(u32 total_size, u32 chunk_size);
void lbuf_memory_init(void *adr, u32 len);
//struct lbuff_head *lbuf_init(void *buf, u32 buf_len);

struct lbuff_head *lbuf_init(void *buf, u32 len, int align, int priv_head_len);
void *lbuf_alloc(struct lbuff_head *head, u32 len);
//int lbuf_state(struct lbuff_head *head);
void lbuf_state(struct lbuff_head *head, struct lbuff_state *state);
//void *lbuf_realloc(void *lbuf, int size);

void *lbuf_realloc(struct lbuff_head *head, void *lbuf, int size);
//void lbuf_push(void *lbuf);

void lbuf_push(struct lbuff_head *head, void *lbuf, u8 channel_map);
//void *lbuf_pop(struct lbuff_head *head);

void *lbuf_pop(struct lbuff_head *head, u8 channel);
//void lbuf_free(void *lbuf);

void lbuf_free(struct lbuff_head *head, void *lbuf);
int lbuf_empty(struct lbuff_head *head);

void lbuf_clear(struct lbuff_head *head);

u8 lbuf_have_next(struct lbuff_head *head);

int get_lbuf_len(void *lbuf);

void lbuf_info(struct lbuff_head *head);
int lbuf_get_chunk_num(u32 total_size, u32 chunk_size);

void lbuf_lite_state(struct lbuff_head *head, struct lbuff_state *state);
u32 lbuf_lite_free_space(struct lbuff_head *head);
void *lbuf_lite_alloc(struct lbuff_head *head, u32 len);
void *lbuf_lite_realloc(void *lbuf, int size);
struct lbuff_head *lbuf_lite_init(void *buf, u32 len, int align, int priv_head_len);
void lbuf_lite_free(struct lbuff_head *head, void *lbuf);
#else
struct lbuff_state {
    u32 avaliable;
    u32 fragment;
    u32 max_continue_len;
    int num;
};

#if 0
#define LBUF_ADAPTER_DECLARE(suffix)      \
struct lbuff_head##suffix {                                                 \
    struct list_head head;                                          \
    struct list_head free;                                          \
    /*u16 index;*/                                                  \
};                                                                  \
                                                                    \
int lbuf_get_chunk_num##suffix(u32 total_size, u32 chunk_size);             \
                                                                    \
struct lbuff_head##suffix *lbuf_init##suffix(void *buf, u32 buf_len);               \
                                                                    \
void *lbuf_alloc##suffix(struct lbuff_head##suffix *head, u32 len);                 \
                                                                    \
void *lbuf_realloc##suffix(void *lbuf, int size);                           \
                                                                    \
void lbuf_push##suffix(void *lbuf);                                         \
                                                                    \
void *lbuf_pop##suffix(struct lbuff_head##suffix *head);                            \
                                                                    \
void *lbuf_query_used##suffix(struct lbuff_head##suffix *head);                     \
                                                                    \
void lbuf_free##suffix(void *lbuf);                                         \
                                                                    \
int lbuf_empty##suffix(struct lbuff_head##suffix *head);                            \
                                                                    \
void lbuf_clear##suffix(struct lbuff_head##suffix *head);                           \
                                                                    \
bool lbuf_more_data##suffix(struct lbuff_head##suffix *head);                       \
                                                                    \
u32 lbuf_remain_len##suffix(struct lbuff_head##suffix *head, u32 len);              \
                                                                    \
/* void lbuf_debug##suffix(struct lbuff_head *head);*/                        \
void lbuf_debug##suffix(struct lbuff_head##suffix *head, u8 flag);                  \
                                                                        \
void lbuf_info##suffix(struct lbuff_head##suffix *head);	//获取lbuf的alloc使用情况

LBUF_ADAPTER_DECLARE(_btctrler)
#endif
#endif

struct lbuff_head {
    int magic_a;
    struct list_head head;
    struct list_head free;
};

void lbuf_state(struct lbuff_head *head, struct lbuff_state *state);
void lbuf_dump(void *head);
int lbuf_avaliable(struct lbuff_head *head, int size);
void *lbuf_alloc(void *head, u32 len);
void lbuf_free_check(void *lbuf, u32 rets);
void lbuf_free(void *head);
int lbuf_real_size(void *lbuf);
void *lbuf_realloc(void *lbuf, int size);
void *lbuf_init(void *buf, u32 len, int align, int priv_head_len);
void lbuf_inc_ref(void *lbuf);
int lbuf_remain_space(struct lbuff_head *head);

extern struct lbuff_head_btctrler;

int lbuf_get_chunk_num_btctrler(u32 total_size, u32 chunk_size);

struct lbuff_head_btctrler *lbuf_init_btctrler(void *buf, u32 buf_len);

void *lbuf_alloc_btctrler(struct lbuff_head_btctrler *head, u32 len);

void *lbuf_realloc_btctrler(void *lbuf, int size);

void lbuf_push_btctrler(void *lbuf);

void *lbuf_pop_btctrler(struct lbuff_head_btctrler *head);

void *lbuf_query_used_btctrler(struct lbuff_head_btctrler *head);

void lbuf_free_btctrler(void *lbuf);

int lbuf_empty_btctrler(struct lbuff_head_btctrler *head);

void lbuf_clear_btctrler(struct lbuff_head_btctrler *head);

bool lbuf_more_data_btctrler(struct lbuff_head_btctrler *head);

u32 lbuf_remain_len_btctrler(struct lbuff_head_btctrler *head, u32 len);

/* void lbuf_debug_btctrler(struct lbuff_head *head);*/
void lbuf_debug_btctrler(struct lbuff_head_btctrler *head, u8 flag);

void lbuf_info_btctrler(struct lbuff_head_btctrler *head);	//获取lbuf的alloc使用情况

void lbuf_clear(struct lbuff_head *head);
#endif


