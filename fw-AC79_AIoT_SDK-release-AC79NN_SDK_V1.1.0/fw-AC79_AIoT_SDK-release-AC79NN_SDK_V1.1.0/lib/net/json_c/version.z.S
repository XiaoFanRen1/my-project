.section .sys.version, "ax"

.global lib_json_c_version

lib_json_c_version:

#ifdef CONFIG_RELEASE_ENABLE

#else

.asciz "JSON_C-*modified*--@20220930-$eb81a39"
#endif
