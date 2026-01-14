.section .sys.version, "ax"

.global lib_mdns_version

lib_mdns_version:

#ifdef CONFIG_RELEASE_ENABLE

#else
.asciz __FILE__
.asciz "MDNS-*modified*-@20220826-$5abe746"
#endif
