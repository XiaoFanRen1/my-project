/*  ******************************************
 *
 *  |offset| hentry | priv | data|
 *  |              ret     |
 *  | 					   |
 *	|---------align--------|
 *
 *  *****************************************/

#include "lbuf.h"
#include "uart.h"
#include "os/os_cpu.h"

#define log_error(...)
#define log_info(...)
#define log_info_hexdump(...)

#define __get_entry(lbuf)    \
	(struct hentry *)((u8 *)lbuf - sizeof(struct hentry))


struct lbuff_head_btctrler {
    struct list_head head;
    struct list_head free;
    //u16 index;
};

struct hentry {
    struct list_head entry;
    struct lbuff_head_btctrler *head;
    u32 len;//修复长度超64K
    u8 state;
    char ref;
};


struct hfree {
    struct list_head entry;
    u32 len;//修复长度超64K
};



int lbuf_get_chunk_num_btctrler(u32 total_size, u32 chunk_size)
{
    return (total_size - sizeof(struct hfree)) / (chunk_size + sizeof(struct hentry) + 3);
}

struct lbuff_head_btctrler *lbuf_init_btctrler(void *buf, u32 buf_len)
{
    struct hfree *free;
    struct lbuff_head_btctrler *head;

    if ((int)buf & 0x03) {
        buf_len -= 4 - ((int)buf & 0x03);
        buf += 4 - ((int)buf & 0x03);
        /* ASSERT((((u32)buf) & 0x03) == 0, "%xrn", buf); */
        log_error("lbuf misalgin : 0x%x / 0x%x", buf, buf_len);
    }

    head = buf;
    ASSERT((((u32)buf) & 0x03) == 0, "%xrn", buf);

    free = (struct hfree *)(head + 1);
    free->len = buf_len - sizeof(*head) - sizeof(*free);

    INIT_LIST_HEAD(&head->head);
    INIT_LIST_HEAD(&head->free);

    list_add_tail(&free->entry, &head->free);

    return head;
}


void *lbuf_alloc_btctrler(struct lbuff_head_btctrler *head, u32 len)
{
    struct hentry *entry;
    struct hfree *p;
    struct hfree *new;
    void *ret = NULL;
    OS_SR_ALLOC();


    len += sizeof(*entry);
    if (len & 0x03) {
        len += 4 - (len & 0x03);
    }

    OS_ENTER_CRITICAL();

    list_for_each_entry(p, &head->free, entry) {
        if (p->len < len) {
            continue;
        }
        if (p->len > len + sizeof(struct hfree)) {
            new = (struct hfree *)((u8 *)p + len);
            new->len = p->len - len;
            __list_add(&new->entry, p->entry.prev, p->entry.next);
        } else {
            /* puts("***rn"); */
            len = p->len;
            __list_del_entry(&p->entry);
        }

        entry = (struct hentry *)p;
        entry->head = head;
        entry->state = 0;
        entry->len = len;
        entry->ref = 0;
        INIT_LIST_HEAD(&entry->entry);

        ret = entry + 1;
        /* log_info("rn>>>alloc %x / %d / %drn", entry, entry->len, len); */
        break;
    }
    if (ret == NULL) {
        /* puts("alloc-errrn"); */
        /* putchar('#'); */
    }

    OS_EXIT_CRITICAL();
    return ret;
}


void lbuf_free_btctrler(void *lbuf)
{
    struct hfree *p;
    struct hfree *new;
    struct hfree *prev = NULL;
    struct hentry *entry;
    struct lbuff_head_btctrler *head;
    OS_SR_ALLOC();

    if (lbuf == NULL) {
        log_error("****lbuf nothing to be free***");
        /* ASSERT(0, "Nothing to freern"); */
        return;
    }

    entry = __get_entry(lbuf);
    new = (struct hfree *)entry;
    head = entry->head;

    OS_ENTER_CRITICAL();

    __list_del_entry(&entry->entry);
    new->len = entry->len;
    /* log_info("rn<<free %x/%drn", new, new->len); */
    /* log_info("new free len %drn", new->len); */

    list_for_each_entry(p, &head->free, entry) {
        if ((p <= new) && ((u8 *)p + p->len > (u8 *)new)) {
            /* log_info("free-err: %xrn", lbuf); */
            goto _exit;
        }
        if (p > new) {
            __list_add(&new->entry, p->entry.prev, &p->entry);
            goto __free;
        }
    }
    list_add_tail(&new->entry, &head->free);

__free:
    list_for_each_entry(p, &head->free, entry) {
        if (prev == NULL) {
            prev = p;
            continue;
        }
        /* log_info("free p %xrn", p); */
        /* log_info("prev len %x/%drn", prev, prev->len); */

        if ((u32)prev + prev->len == (u32)p) {
            /* puts("okrn"); */
            prev->len += p->len;
            __list_del_entry(&p->entry);
        } else {
            prev = p;
        }
    }

_exit:
    OS_EXIT_CRITICAL();
}

void *lbuf_realloc_btctrler(void *lbuf, int size)
{
    struct hentry *new;
    struct hentry *entry = __get_entry(lbuf);

    if (size & 0x03) {
        size += 4 - (size & 0x03);
    }

    ASSERT(size < entry->len);

    if (size > (entry->len - (sizeof(*entry)))) {
        /* puts("realloc errorrn"); */
        /* log_info("realloc entry->len %x / %d / %drn", entry, entry->len, size + sizeof(*new) + sizeof(*entry)); */
        return NULL;
    }

    /* log_info("@realloc entry->len %x / %d / %drn", entry, entry->len, size); */
    if (size + sizeof(*new) + sizeof(*entry) < entry->len) {
        new = (struct hentry *)((u8 *)lbuf + size);
        new->head = entry->head;
        new->len = entry->len - sizeof(*entry) - size;
        new->ref = 0;
        INIT_LIST_HEAD(&new->entry);
        entry->len = sizeof(*entry) + size;
        /* log_info("@entry->len %x/%drn", entry, entry->len); */
        /* log_info("@new->len %x/%drn", new, new->len); */

        lbuf_free_btctrler(new + 1);
    }
    return lbuf;
}


int lbuf_empty_btctrler(struct lbuff_head_btctrler *head)
{
    if (list_empty(&head->head)) {
        return 1;
    }
    return 0;
}





void lbuf_push_btctrler(void *lbuf)
{
    struct hentry *p = __get_entry(lbuf);
    OS_SR_ALLOC();

    OS_ENTER_CRITICAL();

    p->state = 0;
    if (p->ref++ == 0) {
        list_add_tail(&p->entry, &p->head->head);
    }

    OS_EXIT_CRITICAL();
}


bool lbuf_more_data_btctrler(struct lbuff_head_btctrler *head)
{
    struct hentry *p;
    OS_SR_ALLOC();

    OS_ENTER_CRITICAL();

    list_for_each_entry(p, &head->head, entry) {
        if (p->state == 0) {        /*bug : p->ref*/
            OS_EXIT_CRITICAL();
            return TRUE;
        }
    }

    OS_EXIT_CRITICAL();
    return FALSE;
}


void *lbuf_pop_btctrler(struct lbuff_head_btctrler *head)
{
    struct hentry *p;
    OS_SR_ALLOC();

    OS_ENTER_CRITICAL();

    list_for_each_entry(p, &head->head, entry) {
        if (p->state == 0) {
            p->state = 1;
            OS_EXIT_CRITICAL();
            return p + 1;
        }
    }

    OS_EXIT_CRITICAL();
    return NULL;
}



void lbuf_clear_btctrler(struct lbuff_head_btctrler *head)
{
    struct hentry *p, *n;
    OS_SR_ALLOC();

    OS_ENTER_CRITICAL();

    list_for_each_entry_safe(p, n, &head->head, entry) {
        lbuf_free_btctrler(p + 1);
    }

    OS_EXIT_CRITICAL();
}

void lbuf_debug_btctrler(struct lbuff_head_btctrler *head, u8 flag)
{
    struct hentry *entry;
    struct hentry *q;
    struct hfree *p;
    struct hfree *new;
    void *ret = NULL;
    OS_SR_ALLOC();

    OS_ENTER_CRITICAL();
    if (flag) {
        /* puts("rn-----logrn"); */
    } else {
        /* puts("rn******logrn");*/
    }
    log_info("begin : %x", head);
    log_info_hexdump((u8 *)head, 16);

    list_for_each_entry(q, &head->head, entry) {
        log_info("alloc %x - %drn", q, q->len);
    }

    list_for_each_entry(p, &head->free, entry) {
        log_info("free %x - %drn", p, p->len);
        log_info("freenext %x - %xrn", p, p->entry.next);
    }
    if (flag) {
        /* puts("-----log endrn"); */
    } else {
        /* puts("*****log endrn"); */
    }
    OS_EXIT_CRITICAL();
}

u32 lbuf_remain_len_btctrler(struct lbuff_head_btctrler *head, u32 len)
{
    struct hentry *entry;
    struct hfree *p;
    u32 res = 0;

    OS_SR_ALLOC();

    len += sizeof(*entry);
    if (len & 0x3) {
        len += 4 - (len & 0x3);
    }

    OS_ENTER_CRITICAL();

    list_for_each_entry(p, &head->free, entry) {
        if (p->len < len) {
            continue;
        } else {
            res = 1;
            break;
        }
    }

    OS_EXIT_CRITICAL();

    return res;

}

void lbuf_info_btctrler(struct lbuff_head_btctrler *head)	/*获取lbuf的alloc使用情况*/
{
    struct hfree *p;

    log_info("lbuf_free_list_info");
    list_for_each_entry(p, &head->free, entry) {
        log_info("p = 0x%x,	len = 0x%x", p, p->len);
    }
}



void *lbuf_lite_init(void *buf, u32 len, int align, int priv_head_len)
{
    return lbuf_init_btctrler(buf, len);
}

void *lbuf_init(void *buf, u32 len, int align, int priv_head_len)
{
    return lbuf_init_btctrler(buf, len);
}

void lbuf_lite_free(void *head)
{
    return lbuf_free_btctrler(head);
}

void lbuf_free_check(void *lbuf, u32 rets)
{

}
int lbuf_real_size(void *lbuf)
{
    struct hentry *entry = __get_entry(lbuf);

    return  entry->len;
}

void lbuf_free(void *head)
{
    return lbuf_free_btctrler(head);
}

void *lbuf_lite_alloc(void *head, u32 len)
{
    return lbuf_alloc_btctrler(head, len);
}

void *lbuf_alloc(void *head, u32 len)
{
    return lbuf_alloc_btctrler(head, len);
}

void *lbuf_lite_realloc(void *lbuf, int size)
{
    return lbuf_realloc_btctrler(lbuf, size);
}

void *lbuf_realloc(void *lbuf, int size)
{
    return lbuf_realloc_btctrler(lbuf, size);
}

void lbuf_clear(struct lbuff_head *head)
{
    lbuf_clear_btctrler((struct lbuff_head_btctrler *)head);
}

void lbuf_inc_ref(void *lbuf)
{
    struct hentry *entry = __get_entry(lbuf);

    entry->ref++;
}

void lbuf_lite_dump(void *head)
{

}

int lbuf_remain_space_btctrler(struct lbuff_head_btctrler *head)
{
    int max_len = 0;
    struct hfree *p;
    struct hentry *entry;

    OS_ENTER_CRITICAL();

    list_for_each_entry(p, &head->free, entry) {
        max_len += p->len;
    }

    OS_EXIT_CRITICAL();

    return max_len;
}

int lbuf_remain_space(struct lbuff_head *head)
{
    return lbuf_remain_space_btctrler((struct lbuff_head_btctrler *)head);
}

void lbuf_dump(void *head)
{

}
/* int lbuf_avaliable(struct lbuff_head *head, int size)
{
} */
