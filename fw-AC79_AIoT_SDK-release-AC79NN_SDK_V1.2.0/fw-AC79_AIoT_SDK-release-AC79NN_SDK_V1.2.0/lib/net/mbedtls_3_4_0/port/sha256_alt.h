#ifndef MBEDTLS_SHA256_ALT_H
#define MBEDTLS_SHA256_ALT_H

typedef unsigned int uint32_t;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MBEDTLS_SHA256_ALT
typedef struct mbedtls_sha256_context {
    uint32_t total[2];          /*!< The number of Bytes processed.  */
    uint32_t state[8];          /*!< The intermediate digest state.  */
    unsigned char buffer[64];   /*!< The data block being processed. */
    int is224;                  /*!< Determines which function to use:
                                 0: Use SHA-256, or 1: Use SHA-224. */
}
mbedtls_sha256_context;
#else
#include "mbedtls/sha256.h"
#endif

#ifdef __cplusplus
}
#endif

#endif
