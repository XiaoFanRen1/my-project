.section .sys.version, "ax"

.global lib_aliyun_version

lib_aliyun_version:

#ifdef CONFIG_RELEASE_ENABLE

#else
.asciz __FILE__
.asciz "ALIYUN-*modified*-@20220826-$5abe746"
#endif
