#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/mbedtls_config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#include "common.h"
#include "aes_alt.h"
#include "mbedtls/aes.h"
#include "mbedtls/error.h"

#if defined(MBEDTLS_AES_ENCRYPT_ALT)
int mbedtls_internal_aes_encrypt(mbedtls_aes_context *ctx,
                                 const unsigned char input[16],
                                 unsigned char output[16])
{
    jl_aes_encrypt_hw(input, output, ctx->aes_enc_key, ctx->nr == 14 ? 32 : 16);
    return 0;
}
#endif

#if defined(MBEDTLS_AES_DECRYPT_ALT)
int mbedtls_internal_aes_decrypt(mbedtls_aes_context *ctx,
                                 const unsigned char input[16],
                                 unsigned char output[16])
{
    jl_aes_decrypt_hw(input, output, ctx->aes_dec_key, ctx->nr == 14 ? 32 : 16);
    return 0;
}
#endif
