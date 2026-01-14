#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/mbedtls_config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#include "common.h"
#include "sha256_alt.h"

#ifdef MBEDTLS_SHA256_PROCESS_ALT
int mbedtls_internal_sha256_process(mbedtls_sha256_context *ctx,
                                    const unsigned char data[64])
{
    int sha256_start = 1;
    jl_sha256_process(&sha256_start, ctx->state, data);
    return 0;
}
#endif
