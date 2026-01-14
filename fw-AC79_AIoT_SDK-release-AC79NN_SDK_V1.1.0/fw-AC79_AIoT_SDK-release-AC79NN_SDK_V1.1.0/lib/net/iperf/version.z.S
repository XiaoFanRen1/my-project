.section .sys.version, "ax"

.global lib_iperf_version

lib_iperf_version:

#ifdef CONFIG_RELEASE_ENABLE

#else

.asciz "IPERF-*modified*--@20220930-$eb81a39"
#endif
