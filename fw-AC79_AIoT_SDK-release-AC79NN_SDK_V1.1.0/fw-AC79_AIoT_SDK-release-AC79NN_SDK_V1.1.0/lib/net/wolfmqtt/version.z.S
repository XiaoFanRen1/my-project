.section .sys.version, "ax"

.global lib_wolfmqtt_version

lib_wolfmqtt_version:

#ifdef CONFIG_RELEASE_ENABLE

#else

.asciz "WOLFMQTT-*modified*--@20220930-$eb81a39"
#endif
