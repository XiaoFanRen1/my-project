.section .sys.version, "ax"

.global lib_zliblite_version

lib_zliblite_version:

#ifdef CONFIG_RELEASE_ENABLE

#else
.asciz __FILE__
.asciz "ZLIBLITE-*modified*-@20220826-$5abe746"
#endif
