
.section .sys.version, "ax"

.global lib_nopoll_version

lib_nopoll_version:

#ifdef CONFIG_RELEASE_ENABLE

#else
.asciz __FILE__
.asciz "LIBNOPOLL-*modified*-@20220826-$5abe746"
#endif
