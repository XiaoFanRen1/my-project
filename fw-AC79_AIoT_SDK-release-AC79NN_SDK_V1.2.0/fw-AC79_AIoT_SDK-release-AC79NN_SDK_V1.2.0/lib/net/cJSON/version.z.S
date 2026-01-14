.section .sys.version, "ax"

.global lib_cjson_version

lib_cjson_version:

#ifdef CONFIG_RELEASE_ENABLE

#else

.asciz "CJSON-*modified*--@20250403-$8b12749b"
#endif
