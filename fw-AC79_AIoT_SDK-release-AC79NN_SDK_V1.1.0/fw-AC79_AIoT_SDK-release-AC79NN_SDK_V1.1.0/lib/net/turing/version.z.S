.section .sys.version, "ax"

.global lib_turing_version

lib_turing_version:

#ifdef CONFIG_RELEASE_ENABLE

#else

.asciz "TURING-*modified*--@20220930-$eb81a39"
#endif
