.section .sys.version, "ax"

.global lib_mqtt_version

lib_mqtt_version:

#ifdef CONFIG_RELEASE_ENABLE

#else

.asciz "MQTT-*modified*--@20220930-$eb81a39"
#endif
