#ifndef _DEBUG_H_
#define _DEBUG_H_

#include "typedef.h"

#define PRINTF(format, ...)         printf(format, ## __VA_ARGS__)

#ifdef LOG_INFO_ENABLE
#define log_info(format, ...)       PRINTF("[Info] :" LOG_TAG format "\r\n", ## __VA_ARGS__)
#else
#define log_info(...)
#endif

#ifdef LOG_DEBUG_ENABLE
#define log_debug(format, ...)       PRINTF("[Debug] :" LOG_TAG format "\r\n", ## __VA_ARGS__)
#define log_debug_hexdump(x, y)     printf_buf(x, y)
#else
#define log_debug(...)
#define log_debug_hexdump(x, y)
#endif

#ifdef LOG_ERROR_ENABLE
#define log_error(format, ...)      PRINTF("<Error> :" LOG_TAG format "\r\n", ## __VA_ARGS__)
#define log_error_hexdump(x, y)     printf_buf(x, y)
#else
#define log_error(...)
#define log_error_hexdump(...)
#endif

#ifdef LOG_DUMP_ENABLE
#define log_info_hexdump(x,y)       printf_buf(x,y)
#else
#define log_info_hexdump(...)
#endif

#ifdef LOG_DUMP_ENABLE
#define log_info_hexdump(x,y)       printf_buf(x,y)
#else
#define log_info_hexdump(...)
#endif



#endif
