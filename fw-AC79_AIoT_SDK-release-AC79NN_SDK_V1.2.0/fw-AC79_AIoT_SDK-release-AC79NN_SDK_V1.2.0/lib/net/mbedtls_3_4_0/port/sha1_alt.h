#ifndef MBEDTLS_SHA1_ALT_H
#define MBEDTLS_SHA1_ALT_H

typedef unsigned int uint32_t;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MBEDTLS_SHA1_ALT
typedef struct mbedtls_sha1_context {
    uint32_t total[2];          /*!< The number of Bytes processed.  */
    uint32_t state[5];          /*!< The intermediate digest state.  */
    unsigned char buffer[64];   /*!< The data block being processed. */
}
mbedtls_sha1_context;
#else
#include "mbedtls/sha1.h"
#endif

#ifdef __cplusplus
}
#endif

#endif
