.section .sys.version, "ax"

.global lib_mdns_version

lib_mdns_version:

#ifdef CONFIG_RELEASE_ENABLE

#else

.asciz "MDNS-*modified*--@20220930-$eb81a39"
#endif
