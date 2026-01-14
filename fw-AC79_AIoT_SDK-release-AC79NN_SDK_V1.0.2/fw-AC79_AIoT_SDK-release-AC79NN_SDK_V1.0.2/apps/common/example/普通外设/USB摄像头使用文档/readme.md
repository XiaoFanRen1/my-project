# USB摄像头使用说明(UVC)

> 0. 使用工程wifi_camera进行编译测试 。
> 1. 配置芯片使用SROM  即 去掉 CONFIG_NO_SDRAM_ENABLE宏  一般在全局宏配置。
> 2. 在app_config.h中将CONFIG_UVC_VIDEO2_ENABLE宏开启。
> 3. 在vvideo_buf_config.h中改大NET_VREC0_FBUF_SIZE宏大小。
> 4. 将CONFIG_VIDEO_REC_PPBUF_MODE宏进行屏蔽  屏蔽默认使用LBUF 即帧buf。
> 5. 目前占不支持USB2.0摄像头目前仅支持USB1.1摄像头。
> 6. 有关帧数使用规格640*480 25fps(USB摄像头) wifi图传20fps。 
> 7. 当想要使用UI显示UVC摄像头时候需要提前确认好屏幕驱动已经调好能正常显示，然后将CONFIG_UI_ENABLE宏开启 并且在app_config.h中将TCFG_DEMO_UI_RUN， NO_UI_LCD_TEST 均配置为 1。
> 8. 支持屏幕和UVC同时显示摄像头内容。

