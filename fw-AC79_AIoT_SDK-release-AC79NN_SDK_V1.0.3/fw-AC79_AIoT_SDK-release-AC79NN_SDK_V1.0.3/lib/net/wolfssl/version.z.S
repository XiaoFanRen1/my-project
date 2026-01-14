.section .sys.version, "ax"

.global lib_wolfssl_version

lib_wolfssl_version:

#ifdef CONFIG_RELEASE_ENABLE

#else
.asciz __FILE__
.asciz "WOLFSSL-*modified*-@20220826-$5abe746"
#endif
