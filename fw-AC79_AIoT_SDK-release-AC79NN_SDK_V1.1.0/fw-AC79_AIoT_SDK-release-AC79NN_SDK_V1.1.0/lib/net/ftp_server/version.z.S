.section .sys.version, "ax"

.global lib_ftpserver_version

lib_ftpserver_version:

#ifdef CONFIG_RELEASE_ENABLE

#else

.asciz "FTPSERVER-*modified*--@20220930-$eb81a39"
#endif
