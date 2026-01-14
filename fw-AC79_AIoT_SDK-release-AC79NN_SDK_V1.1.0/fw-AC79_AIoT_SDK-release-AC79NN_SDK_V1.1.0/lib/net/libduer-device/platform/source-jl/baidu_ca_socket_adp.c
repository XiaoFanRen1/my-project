/**
 * Copyright (2017) Baidu Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/**
 * File: baidu_ca_socket_adp.c
 * Auth: Su Hao (suhao@baidu.com)
 * Desc: Adapt the socket function to esp32.
 */

#include "duerapp_config.h"

#include "baidu_ca_adapter_internal.h"

#include "os/os_api.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "lwip/err.h"
#include "lwip/dns.h"
#include "lwip/ip_addr.h"

#include "lightduer_connagent.h"
#ifdef DUER_HTTP_DNS_SUPPORT
#include "lightduer_http_dns_client_ops.h"
#endif // DUER_HTTP_DNS_SUPPORT
#include "lightduer_log.h"
#include "lightduer_lib.h"
#include "lightduer_memory.h"
#include "lightduer_events.h"
#include "lightduer_timers.h"
#include "lightduer_priority_conf.h"
#include "lightduer_timestamp.h"

#if defined(DUER_PLATFORM_A113L) || defined(DUER_PLATFORM_SV32WB0X)
#include <sys/errno.h>
#include "FreeRTOS.h"
#include "semphr.h"
#else
/* #include "freertos/FreeRTOS.h" */
/* #include "freertos/semphr.h" */
#include "os/os_api.h"
#endif

#ifdef DUER_PLATFORM_ESP8266
#include "adaptation.h"
#endif

#if defined(DUER_PLATFORM_ESP8266)
#define TASK_STACK_SIZE (1024 * 4)
#elif defined(DUER_PLATFORM_XR871)
#define TASK_STACK_SIZE (1024 * 4)
#elif defined(DUER_PLATFORM_SV32WB0X)
#define TASK_STACK_SIZE (1024 * 4)
#elif defined(DUER_PLATFORM_A113L)
#define TASK_STACK_SIZE (1024 * 4)
#else
#define TASK_STACK_SIZE (1024 * 4 / sizeof(portSTACK_TYPE))
#endif

#if defined(DUER_PLATFORM_MARVELL)
#define SemaphoreHandle_t   xSemaphoreHandle
#define TaskHandle_t        xTaskHandle
#endif

#ifndef NON_BLOCKING
#define NON_BLOCKING    (0)
#endif

extern duer_handler my_handler;

#if defined(NON_BLOCKING) && (NON_BLOCKING == 1)
#define MSG_FLAG        (MSG_DONTWAIT)
#else
#define MSG_FLAG        (0)
#endif

#ifndef DUER_SENDTIMEOUT
#define DUER_SENDTIMEOUT    8000 // 8s
#endif

typedef struct _bcasoc_s {
    volatile int fd;
    volatile int is_send_block;
    volatile int destroy;
    volatile int ref_count;
    duer_u32_t           send_data_block_begin_time; // unit is ms
    duer_timer_handler  _timer;
    duer_transevt_func  _callback;
} bcasoc_t;

static duer_events_handler  g_lightduer_events = NULL;
static SemaphoreHandle_t    g_mutex = NULL;
static duer_timer_handler   s_finalize_timer = NULL;

static volatile u8 socket_timer_release_flag = 1;
static volatile u8 socket_exit_flag = 0;



static void bcasoc_lock()
{
    if (g_mutex) {
        xSemaphoreTake(g_mutex, portMAX_DELAY);
    }
}

static void bcasoc_unlock()
{
    if (g_mutex) {
        xSemaphoreGive(g_mutex);
    }
}

static unsigned int bcasoc_parse_ip(const duer_addr_t *addr, duer_bool *use_http_dns)
{
    unsigned int rs = 0;

    if (addr && addr->host) {
        DUER_LOGD("bcasoc_parse_ip: ip = %s", addr->host);
        struct hostent *hp = gethostbyname(addr->host);
#ifdef DUER_HTTP_DNS_SUPPORT
        if (!hp) {
            DUER_LOGW("retry HTTP DNS, %s!", addr->host);
            duer_http_dns_task_init();
            const char *ip = duer_http_dns_query(addr->host, 2000); // timeout is 2s
            duer_http_dns_task_destroy();
            DUER_LOGD("http dns got ip:%s", ip);
            if (ip) {
                if (use_http_dns) {
                    *use_http_dns = DUER_TRUE;
                }
                hp = gethostbyname(ip);
            }
        }
#endif // DUER_HTTP_DNS_SUPPORT
        if (!hp) {
            DUER_LOGE("DNS failed!!!");
        } else {
            /* #if defined(DUER_PLATFORM_ESPRESSIF) || defined(DUER_PLATFORM_XR6166) \ */
            /* || defined(DUER_PLATFORM_A113L) || defined(DUER_PLATFORM_SV32WB0X) */
            struct ip4_addr *ip4_addr = (struct ip4_addr *)hp->h_addr;
            /* #else */
            /* struct ip_addr *ip4_addr = (struct ip_addr *)hp->h_addr; */
            /* #endif */
            if (ip4_addr) {
                DUER_LOGI("DNS lookup succeeded. IP=%s", inet_ntoa(*ip4_addr));
                // rs = htonl(ip4_addr->addr);
                rs = ip4_addr->addr;
            }
        }
    }

    DUER_LOGV("bcasoc_parse_ip: rs = 0x%08x, addr:%p", rs, addr);

    return rs;
}

static void bcasoc_check_data(int what, void *object)
{
    bcasoc_lock();
    bcasoc_t *soc = (bcasoc_t *)object;
    int rs = 0;
    int fd = -1;
    duer_transevt_func callback = NULL;

    if (!soc || soc->ref_count <= 0) {
        DUER_LOGE("soc:%p, ref_count:%d", soc, soc ? soc->ref_count : -99);
        goto out;
    } else {
        DUER_LOGD("soc:%p, destroy:%d, fd:%d, ref_count:%d, timer:%p",
                  soc, soc->destroy, soc->fd, soc->ref_count, soc->_timer);
        soc->ref_count--;
        if (soc->fd == -1) {
            DUER_LOGI("soc:%p, destroy:%d, fd:%d, ref_count:%d, timer:%p",
                      soc, soc->destroy, soc->fd, soc->ref_count, soc->_timer);
            if (soc->destroy == 1) {
                if (soc->_timer) {
                    duer_timer_release(soc->_timer);
                    soc->_timer = NULL;
                }
                if (!soc->ref_count) {
                    soc->ref_count = -1; // prevent reenter to free twice
                    DUER_FREE(soc);
                }
            }
            goto out;
        }
    }

    callback = soc->_callback;

    if (soc->is_send_block == 1) {
        duer_u32_t current_ts = duer_timestamp();
        if (current_ts - soc->send_data_block_begin_time > DUER_SENDTIMEOUT) {
            DUER_LOGD("send time out, current timestamp:%u, begin_time:%u",
                      current_ts, soc->send_data_block_begin_time);
            soc->send_data_block_begin_time = duer_timestamp();// reset the timestamp
            if (callback) {
                callback(DUER_TEVT_SEND_TIMEOUT);
            }
        }
    }

    fd = soc->fd;

    fd_set fdread, fdwrite, fdex;
    struct timeval time = {0, 0};

    FD_ZERO(&fdread);
    FD_ZERO(&fdwrite);
    FD_ZERO(&fdex);

    FD_SET(fd, &fdread);
    FD_SET(fd, &fdwrite);
    FD_SET(fd, &fdex);

    rs = select(fd + 1, &fdread, &fdwrite, &fdex, &time);
    DUER_LOGD("select rs:%d, block:%d", rs, soc->is_send_block);

    if (rs > 0) {
        if (FD_ISSET(fd, &fdread)) {
            if (callback) {
                callback(DUER_TEVT_RECV_RDY);
            }
        } else if (FD_ISSET(fd, &fdex)) {
            DUER_LOGE("exception occurs %d:%s", errno, strerror(errno));
            rs = -1;
        }

        if (FD_ISSET(fd, &fdwrite) && soc->is_send_block == 1) {
            if (callback) {
                callback(DUER_TEVT_SEND_RDY);
            }
            soc->is_send_block = 0;
            soc->send_data_block_begin_time = 0;
        }

        if (!FD_ISSET(fd, &fdread)) {
            if (soc->_timer) {
                duer_timer_start(soc->_timer, 50);
            }
        }
    } else if (rs == 0) {
        if (soc->_timer) {
            duer_timer_start(soc->_timer, 50);
        }
    }

out:
    bcasoc_unlock();
}

static int bcasoc_events_call(duer_events_func func, int what, void *object)
{
    if (g_lightduer_events) {
        return duer_events_call(g_lightduer_events, func, what, object);
    }
    return DUER_ERR_FAILED;
}

static void bcasoc_timer_expired(void *param)
{
    bcasoc_lock();
    bcasoc_t *soc = (bcasoc_t *)param;
    if (!soc->ref_count) {
        soc->ref_count++;
        bcasoc_events_call(bcasoc_check_data, 0, param);
    }
    bcasoc_unlock();
}

void bcasoc_initialize(void)
{
    if (g_lightduer_events == NULL) {
        g_lightduer_events = duer_events_create("lightduer_socket", TASK_STACK_SIZE, 10);
    }

    if (g_mutex == NULL) {
        g_mutex = xSemaphoreCreateMutex();
    }
}

void bcasoc_break_connect(void)
{
    socket_exit_flag = 1;
}

void bcasoc_uninitialize(void)
{
    if (g_lightduer_events) {
        while (!socket_timer_release_flag) {
            duer_sleep(50);
        }
        duer_events_destroy(g_lightduer_events);
        g_lightduer_events = NULL;
    }
    if (g_mutex) {
        /* pthread_mutex_destroy(&g_mutex); */
        os_mutex_del((OS_MUTEX *)(g_mutex), OS_DEL_ALWAYS);
        if (g_mutex) {
            free((void *)(g_mutex));
            g_mutex = 0;
        }
    }
    socket_exit_flag = 0;
}

duer_socket_t bcasoc_create(duer_transevt_func func)
{
    bcasoc_t *soc = DUER_MALLOC(sizeof(bcasoc_t));
    if (soc) {
        DUER_MEMSET(soc, 0, sizeof(bcasoc_t));
        soc->fd = -1;
        soc->is_send_block = 0;
        soc->destroy = 0;
        soc->ref_count = 0;
        soc->send_data_block_begin_time = 0;
        soc->_callback = func;
    }
    return soc;
}

static int set_sock_nonblock(int fd)
{
    long arg = 0;
    // Set non-blocking
    if ((arg = fcntl(fd, F_GETFL, 0)) < 0) {
        DUER_LOGE("Error fcntl(..., F_GETFL) (%s)\n", strerror(errno));
        return DUER_ERR_FAILED;
    }
    arg |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, arg) < 0) {
        DUER_LOGE("Error fcntl(..., F_SETFL) (%s)\n", strerror(errno));
        return DUER_ERR_FAILED;
    }
    return DUER_OK;
}

static int set_sock_block(int fd)
{
    long arg = 0;
    // Set non-blocking
    if ((arg = fcntl(fd, F_GETFL, 0)) < 0) {
        DUER_LOGE("Error fcntl(..., F_GETFL) (%s)\n", strerror(errno));
        return DUER_ERR_FAILED;
    }
    arg &= ~O_NONBLOCK;
    if (fcntl(fd, F_SETFL, arg) < 0) {
        DUER_LOGE("Error fcntl(..., F_SETFL) (%s)\n", strerror(errno));
        return DUER_ERR_FAILED;
    }
    return DUER_OK;
}

static void close_sock_fd(bcasoc_t *soc)
{
    bcasoc_lock();
    if (soc->fd != -1) {
        close(soc->fd);
        soc->fd = -1;
    }
    bcasoc_unlock();
}

static duer_status_t bcasoc_connect_ip_timeout(duer_socket_t ctx, const duer_addr_t *addr, duer_u32_t ip,
        duer_u32_t timeout_ms)
{
    int rs = DUER_ERR_FAILED;
    bcasoc_t *soc = (bcasoc_t *)ctx;
    fd_set myset;
    struct timeval tv;
    int opt_val = 0;
    socklen_t opt_len = sizeof(opt_val);
    if (!soc || !addr) {
        return DUER_ERR_FAILED;
    }

    soc->fd = socket(AF_INET, addr->type == DUER_PROTO_TCP ? SOCK_STREAM : SOCK_DGRAM,
                     addr->type == DUER_PROTO_TCP ? IPPROTO_TCP : IPPROTO_UDP);
    if (soc->fd < 0) {
        DUER_LOGE("Error socket (%s)\n", strerror(errno));
        return DUER_ERR_FAILED;
    }
    // Set to nonblock.
    rs = set_sock_nonblock(soc->fd);
    if (rs != DUER_OK) {
        close_sock_fd(soc);
        return rs;
    }
    // Trying to connect.
    struct sockaddr_in addr_in;
    addr_in.sin_family = AF_INET;
    addr_in.sin_port = htons(addr->port);
    DUER_LOGV("Result bcasoc_connect: addr = %u", ip);
    addr_in.sin_addr.s_addr = ip;

    rs = connect(soc->fd, (struct sockaddr *)&addr_in, sizeof(addr_in));
    if (rs < 0) {
        if (errno != 0 && errno != EINPROGRESS) {
            DUER_LOGE("Error connecting %d - %s", errno, strerror(errno));
            close_sock_fd(soc);
            return DUER_ERR_FAILED;
        }
        DUER_LOGI("EINPROGRESS in connect() - selecting");
        tv.tv_sec = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;
        FD_ZERO(&myset);
        FD_SET(soc->fd, &myset);
        rs = select(soc->fd + 1, NULL, &myset, NULL, &tv);
        if (rs < 0 && errno != EINTR) {
            DUER_LOGE("Error connecting %d - %s", errno, strerror(errno));
            close_sock_fd(soc);
            return DUER_ERR_FAILED;
        } else if (rs > 0) {
            // Socket selected for write.
            if (getsockopt(soc->fd, SOL_SOCKET, SO_ERROR, &opt_val, &opt_len) < 0) {
                DUER_LOGE("Error in getsockopt() %d - %s", errno, strerror(errno));
                close_sock_fd(soc);
                return DUER_ERR_FAILED;
            }
            // Check the value returned.
            if (opt_val) {
                DUER_LOGE("Error in delayed connection() %d - %s", opt_val, strerror(opt_val));
                close_sock_fd(soc);
                return DUER_ERR_FAILED;
            }
        } else {
            DUER_LOGE("Connect timeout, select rs: %d", rs);
            close_sock_fd(soc);
            return DUER_ERR_FAILED;
        }
    }
#if !defined(NON_BLOCKING) || (NON_BLOCKING != 1)
    // Set to blocking mode again.
    rs = set_sock_block(soc->fd);
    if (rs != DUER_OK) {
        close_sock_fd(soc);
        return DUER_ERR_FAILED;
    }
#endif  // NON_BLOCKING
    if (soc->_callback != NULL) {
        if (soc->_timer == NULL) {
            soc->_timer = duer_timer_acquire(bcasoc_timer_expired, soc, DUER_TIMER_ONCE);
        }
        bcasoc_timer_expired((void *)soc);
    }
    return DUER_OK;
}

static duer_status_t bcasoc_connect_ip(duer_socket_t ctx, const duer_addr_t *addr, duer_u32_t ip)
{
    int rs = DUER_ERR_FAILED;
    bcasoc_t *soc = (bcasoc_t *)ctx;

    DUER_LOGV("Entry bcasoc_connect ctx = %p", ctx);
    {
        char ip_str[INET_ADDRSTRLEN] = {0};
        if (inet_ntoa_r(ip, ip_str, INET_ADDRSTRLEN) != NULL) {
            DUER_LOGI("Try to connect: Host=%s, IP=%s", addr->host, ip_str);
        } else {
            DUER_LOGW("inet_ntoa_r failed: Host=%s, IP=%u", addr->host, ip);
        }
    }

    if (soc && addr) {
        soc->fd = socket(AF_INET, addr->type == DUER_PROTO_TCP ? SOCK_STREAM : SOCK_DGRAM,
                         addr->type == DUER_PROTO_TCP ? IPPROTO_TCP : IPPROTO_UDP);
        DUER_LOGV("Result bcasoc_connect fd = %d", soc->fd);
        if (soc->fd < 0) {
            DUER_LOGE("socket create failed: %d", soc->fd);
            return soc->fd;
        }

        struct sockaddr_in addr_in;
        addr_in.sin_family = AF_INET;
        addr_in.sin_port = htons(addr->port);
        DUER_LOGV("Result bcasoc_connect: addr = %u", ip);
        addr_in.sin_addr.s_addr = ip;

        rs = connect(soc->fd, (struct sockaddr *)&addr_in, sizeof(addr_in));

        if (rs >= 0) {

#if defined(NON_BLOCKING) && (NON_BLOCKING == 1)
            int flags = fcntl(soc->fd, F_GETFL, 0);
            fcntl(soc->fd, F_SETFL, flags | O_NONBLOCK);
#endif
            if (soc->_callback != NULL) {
                if (soc->_timer == NULL) {
                    soc->_timer = duer_timer_acquire(bcasoc_timer_expired, soc, DUER_TIMER_ONCE);
                }
                //DUER_LOGI("before call bcasoc_timer_expired, soc:%p, fd:%d", soc, soc->fd);
                bcasoc_timer_expired((void *)soc);
            }
        } else {
            bcasoc_lock();
            if (soc->fd != -1) {
                close(soc->fd);
                soc->fd = -1;
            }
            bcasoc_unlock();
        }
    }

    DUER_LOGV("Result bcasoc_connect rs = %d", rs);

    return rs;
}

duer_status_t bcasoc_connect(duer_socket_t ctx, const duer_addr_t *addr)
{
    return bcasoc_connect_timeout(ctx, addr, 0);
}

duer_status_t bcasoc_connect_timeout(duer_socket_t ctx, const duer_addr_t *addr, duer_u32_t timeout)
{
    duer_bool use_http_dns = DUER_FALSE;
    int rs = 0;

    if (addr == NULL) {
        return DUER_ERR_INVALID_PARAMETER;
    }

    duer_u32_t ip = addr->ip;
    if (ip == 0) {
        DUER_LOGV("Result bcasoc_connect: addr = %s", (char *)addr->host);
        ip = bcasoc_parse_ip(addr, &use_http_dns);
        if (ip == 0) {
            DUER_LOGI("got ip failed!");
            return DUER_ERR_TRANS_DNS_FAIL;
        }
    }
    if (timeout == 0) {
        rs = bcasoc_connect_ip(ctx, addr, ip);
    } else {
        rs = bcasoc_connect_ip_timeout(ctx, addr, ip, timeout);
    }

    if (rs >= 0 && use_http_dns) {
        rs = DUER_INF_TRANS_IP_BY_HTTP_DNS;
    }

    return rs;
}

duer_status_t bcasoc_send(duer_socket_t ctx, const void *data, duer_size_t size, const duer_addr_t *addr)
{
    bcasoc_t *soc = (bcasoc_t *)ctx;
    int rs = DUER_ERR_FAILED;
    fd_set fdw;
    struct timeval tv;

    if (!soc || soc->fd == -1) {
        return rs;
    }

    FD_ZERO(&fdw);
    FD_SET(soc->fd, &fdw);

    tv.tv_sec = 0;
    tv.tv_usec = 0;

    rs = select(soc->fd + 1, NULL, &fdw, NULL, &tv);
    if (FD_ISSET(soc->fd, &fdw)) {
        rs = send(soc->fd, data, size, MSG_FLAG);
    } else if (rs == 0) {
        rs = DUER_ERR_TRANS_WOULD_BLOCK;
        soc->is_send_block = 1;
        soc->send_data_block_begin_time = duer_timestamp();
    } else if (errno != EINTR) {
        rs = DUER_ERR_TRANS_INTERNAL_ERROR;
    }

    if (rs < 0 && rs != DUER_ERR_TRANS_WOULD_BLOCK) {
        DUER_LOGE("write socket error %d:%s", errno, strerror(errno));
    }

    return rs > 0 ? size : rs;
}

duer_status_t bcasoc_recv(duer_socket_t ctx, void *data, duer_size_t size, duer_addr_t *addr)
{
    bcasoc_t *soc = (bcasoc_t *)ctx;
    int rs = DUER_ERR_FAILED;
    fd_set fdr;
    struct timeval tv;

    if (!soc || soc->fd == -1) {
        return rs;
    }

    FD_ZERO(&fdr);
    FD_SET(soc->fd, &fdr);

    tv.tv_sec = 0;
    tv.tv_usec = 0;

    rs = select(soc->fd + 1, &fdr, NULL, NULL, &tv);
    if (FD_ISSET(soc->fd, &fdr)) {
        rs = recv(soc->fd, data, size, MSG_FLAG);
        if (rs <= 0) {
            DUER_LOGE("Can't reach remote host!!! rs = %d", rs);
            rs = DUER_ERR_TRANS_INTERNAL_ERROR;
        }
    } else if (rs == 0) {
        rs = DUER_ERR_TRANS_WOULD_BLOCK;
    } else if (errno != EINTR) {
        rs = DUER_ERR_TRANS_INTERNAL_ERROR;
    }

    if (rs > 0) {
        if (soc->fd != -1) {
            //DUER_LOGI("before call bcasoc_timer_expired, soc:%p, fd:%d", soc, soc->fd);
            bcasoc_timer_expired(soc);
        }
    }

    if (rs < 0 && rs != DUER_ERR_TRANS_WOULD_BLOCK) {
        DUER_LOGE("read socket error %d:%s", errno, strerror(errno));
    }

    return rs;
}

duer_status_t bcasoc_recv_timeout(duer_socket_t ctx, void *data, duer_size_t size, duer_u32_t timeout, duer_addr_t *addr)
{
    DUER_LOGV("bcasoc_recv_timeout: ctx = %p, data = %p, size = %d, timeout = %d, addr = %p", ctx, data, size, timeout,
              addr);
    int rs = DUER_ERR_TRANS_INTERNAL_ERROR;
    bcasoc_t *soc = (bcasoc_t *)ctx;
    duer_u32_t ts = 0;
    if (soc && soc->fd != -1) {
        fd_set fdr;
        struct timeval time = {timeout / 1000, (timeout % 1000) * 1000};

        FD_ZERO(&fdr);
        FD_SET(soc->fd, &fdr);

        ts = duer_timestamp();
        rs = select(soc->fd + 1, &fdr, NULL, NULL, &time);
        DUER_LOGV("bcasoc_recv_timeout select: rs = %d", rs);
        if (FD_ISSET(soc->fd, &fdr)) {
            rs = recv(soc->fd, data, size, 0);
            if (rs < 0) {
                DUER_LOGE("Can't reach remote host!!! rs = %d", rs);
                rs = DUER_ERR_TRANS_INTERNAL_ERROR;
            } else {
                duer_u32_t elapsed = duer_timestamp() - ts;
                if (timeout > 0 && elapsed > timeout) {
                    DUER_LOGE("elapsed %u ms more than timeout %u, rs %d", elapsed, timeout, rs);
                }
            }
        } else if (rs == 0) {
            if (timeout == 0) {
                rs = DUER_ERR_TRANS_WOULD_BLOCK;
            } else {
                rs = DUER_ERR_TRANS_TIMEOUT;
                DUER_LOGW("recv timeout!!!");
            }
        } else {
            rs = DUER_ERR_TRANS_INTERNAL_ERROR;
            DUER_LOGW("recv failed: rs = %d", rs);
        }
    }

    DUER_LOGV("bcasoc_recv_timeout: rs = %d, fd = %d", rs, soc ? soc->fd : -99);
    return rs;
}

duer_status_t bcasoc_close(duer_socket_t ctx)
{
    bcasoc_t *soc = (bcasoc_t *)ctx;
    if (soc) {
        if (soc->_timer) {
            duer_timer_stop(soc->_timer);
            DUER_LOGI("stop timer:%p", soc->_timer);
        }
        bcasoc_lock();
        if (soc->fd != -1) {
            close(soc->fd);
            soc->fd = -1;
        }

        bcasoc_unlock();
    }
    return DUER_OK;
}

duer_status_t bcasoc_destroy(duer_socket_t ctx)
{
    bcasoc_t *soc = (bcasoc_t *)ctx;
    bcasoc_lock();
    soc->destroy = 1;
    DUER_LOGI("start last timer:%p, soc:%p", soc->_timer, soc);
    if (soc->_timer) {
        duer_timer_start(soc->_timer, 200);
    } else {
        DUER_FREE(soc);
    }
    bcasoc_unlock();
    return DUER_OK;
}

static void bcasoc_destroy_events(int what, void *object)
{
    DUER_LOGI("bcasoc_destroy_events");
    duer_events_destroy(g_lightduer_events);
    g_lightduer_events = NULL;
    vSemaphoreDelete(g_mutex);
    g_mutex = NULL;
    duer_timer_release(s_finalize_timer);
    s_finalize_timer = NULL;
}

static void bcasoc_finalize_callback(void *param)
{
    bcasoc_events_call(bcasoc_destroy_events, 0, NULL);
}

void bcasoc_finalize(void)
{
    s_finalize_timer = duer_timer_acquire(bcasoc_finalize_callback, NULL, DUER_TIMER_ONCE);
    if (s_finalize_timer) {
        duer_timer_start(s_finalize_timer, 200);
    }
}
