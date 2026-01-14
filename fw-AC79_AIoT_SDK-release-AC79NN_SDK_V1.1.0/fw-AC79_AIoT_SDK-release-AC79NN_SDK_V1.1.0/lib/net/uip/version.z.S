.section .sys.version, "ax"

.global lib_uip_version

lib_uip_version:

#ifdef CONFIG_RELEASE_ENABLE

#else

.asciz "UIP-*modified*--@20220930-$eb81a39"
#endif
