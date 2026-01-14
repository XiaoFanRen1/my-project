.section .sys.version, "ax"

.global lib_echocloud_version

lib_echocloud_version:

#ifdef CONFIG_RELEASE_ENABLE

#else
.asciz __FILE__
.asciz "ECHOCLOUD-*modified*-@20220826-$5abe746"
#endif
