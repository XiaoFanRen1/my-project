.section .sys.version, "ax"

.global lib_cjson_version

lib_cjson_version:

#ifdef CONFIG_RELEASE_ENABLE

#else

.asciz "CJSON-*modified*--@20220930-$eb81a39"
#endif
