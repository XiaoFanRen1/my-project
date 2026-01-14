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
 * File: lightduer_timers.c
 * Auth: Su Hao (suhao@baidu.com)
 * Desc: Provide the timer APIs.
 */

#include "typedef.h"
#include "net_timer.h"

#include "lightduer_timers.h"
#include "lightduer_adapter_config.h"
#include "lightduer_log.h"
#include "lightduer_memory.h"
#include "lightduer_connagent.h"

#if defined(DUER_PLATFORM_A113L) || defined(DUER_PLATFORM_SV32WB0X)
#include "FreeRTOS.h"
#include "timers.h"
#else
/* #include "freertos/FreeRTOS.h" */
#include "os/os_api.h"
#include "FreeRTOS/timers.h"
#endif

#ifdef DUER_PLATFORM_ESP8266
#include "adaptation.h"
#endif

#define DUER_TIMER_DELAY	(100)

#define assert ASSERT

typedef struct _duer_timers_s {
    int 	            _timer;
    int                 _type;
    duer_timer_callback _callback;
    void               *_param;
} duer_timers_t;

static void duer_timer_callback_wrapper(void *param)
{
    assert(param);

    // Which timer expired?
    duer_timers_t *timer = (duer_timers_t *)param;

    if (timer != NULL && timer->_callback != NULL) {
        timer->_callback(timer->_param);
    }
}

duer_timer_handler duer_timer_acquire(duer_timer_callback callback, void *param, int type)
{
    duer_timers_t *handle = NULL;
    int rs = DUER_ERR_FAILED;
    int status;
    DUER_LOGD("duer_timer_acquire, type:%d", type);
    do {
        handle = (duer_timers_t *)DUER_MALLOC(sizeof(duer_timers_t));
        if (handle == NULL) {
            DUER_LOGE("Memory Overflow!!!");
            break;
        }

        handle->_callback = callback;
        handle->_param = param;
        handle->_type = type;
        handle->_timer = -1;

        rs = DUER_OK;
    } while (0);

    if (rs < DUER_OK) {
        duer_timer_release(handle);
        handle = NULL;
    }

    return handle;
}

int duer_timer_start(duer_timer_handler handle, size_t delay)
{
    duer_timers_t *timer = (duer_timers_t *)handle;
    int rs = DUER_ERR_FAILED;

    do {
        if (timer == NULL) {
            rs = DUER_ERR_INVALID_PARAMETER;
            break;
        }

        if (timer->_type == DUER_TIMER_PERIODIC) {
            timer->_timer = net_timer_add(timer, duer_timer_callback_wrapper, delay, 1);
        } else if (timer->_type == DUER_TIMER_ONCE) {
            timer->_timer = net_timer_add(timer, duer_timer_callback_wrapper, delay, 0);
        }

        rs = DUER_OK;
    } while (0);

    return rs;
}

int duer_timer_is_valid(duer_timer_handler handle)
{
    duer_timers_t *timer = (duer_timers_t *)handle;
    return (timer != NULL);
}

int duer_timer_stop(duer_timer_handler handle)
{
    duer_timers_t *timer = (duer_timers_t *)handle;
    if (timer != NULL) {
        if (timer->_timer != -1) {
            net_timer_del(timer->_timer);
            timer->_timer = -1;
        }
    }
    return 0;
}

void duer_timer_release(duer_timer_handler handle)
{
    duer_timers_t *timer = (duer_timers_t *)handle;
    if (timer != NULL) {
        duer_timer_stop(timer);
        DUER_FREE(timer);
    }
}
