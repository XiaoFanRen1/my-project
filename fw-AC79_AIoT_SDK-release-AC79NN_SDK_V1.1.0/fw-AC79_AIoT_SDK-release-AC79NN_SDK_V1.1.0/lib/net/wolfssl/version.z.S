.section .sys.version, "ax"

.global lib_wolfssl_version

lib_wolfssl_version:

#ifdef CONFIG_RELEASE_ENABLE

#else

.asciz "WOLFSSL-*modified*--@20220930-$eb81a39"
#endif
