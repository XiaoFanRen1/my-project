.section .sys.version, "ax"

.global lib_tencent_version

lib_tencent_version:

#ifdef CONFIG_RELEASE_ENABLE

#else

.asciz "TENCENT-*modified*--@20220930-$eb81a39"
#endif
