.section .sys.version, "ax"

.global lib_tencent_version

lib_tencent_version:

#ifdef CONFIG_RELEASE_ENABLE

#else
.asciz __FILE__
.asciz "TENCENT-*modified*-@20220826-$5abe746"
#endif
