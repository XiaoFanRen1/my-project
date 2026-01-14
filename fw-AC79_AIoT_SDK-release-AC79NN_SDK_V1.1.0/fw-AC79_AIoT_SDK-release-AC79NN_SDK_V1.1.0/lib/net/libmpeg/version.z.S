.section .sys.version, "ax"

.global lib_mpeg_version

lib_mpeg_version:

#ifdef CONFIG_RELEASE_ENABLE

#else

.asciz "MPEG-*modified*--@20220930-$eb81a39"
#endif
