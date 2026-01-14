.section .sys.version, "ax"

.global lib_wt_version

lib_wt_version:

#ifdef CONFIG_RELEASE_ENABLE

#else

.asciz "WT-*modified*--@20220930-$eb81a39"
#endif
