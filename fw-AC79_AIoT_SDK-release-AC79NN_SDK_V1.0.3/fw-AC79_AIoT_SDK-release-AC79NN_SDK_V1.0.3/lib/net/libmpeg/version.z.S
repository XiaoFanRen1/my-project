.section .sys.version, "ax"

.global lib_mpeg_version

lib_mpeg_version:

#ifdef CONFIG_RELEASE_ENABLE

#else
.asciz __FILE__
.asciz "MPEG-*modified*-@20220826-$5abe746"
#endif
