#ifndef MBEDTLS_AES_ALT_H
#define MBEDTLS_AES_ALT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char uint8_t;

#ifdef MBEDTLS_AES_ALT
typedef struct {
    uint8_t key_len;
    uint8_t key[32];
} mbedtls_aes_context;
#endif

#ifdef __cplusplus
}
#endif

#endif/* aes_alt.h */
