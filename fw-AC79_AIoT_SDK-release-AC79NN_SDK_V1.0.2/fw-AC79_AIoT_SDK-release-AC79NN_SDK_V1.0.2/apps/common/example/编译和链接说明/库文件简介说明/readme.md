# 库文件简介说明

> |                    库名称                    |                           功能说明                           |
> | :------------------------------------------: | :----------------------------------------------------------: |
> |                     fs.a                     |                           文件系统                           |
> |                   event.a                    |                         事件管理系统                         |
> |                   system.a                   |   操作系统、内存管理、日志管理、系统定时器等系统一系列接口   |
> |                  cfg_tool.a                  |                          VM管理系统                          |
> |                    cpu.a                     |                         底层设备驱动                         |
> |                   update.a                   |                           固件升级                           |
> |                 common_lib.a                 |                补充libc.a一些缺失的标准库函数                |
> |      ui.a   res.a   font.a   ui_draw.a       |                          UI界面功能                          |
> |                audio_server.a                |                    音频编解码的中间层服务                    |
> |                echo_server.a                 |                     混响功能的中间层服务                     |
> |                lib_mp3_dec.a                 |                        MP3格式解码库                         |
> |                lib_mp3_enc.a                 |                        MP3格式编码库                         |
> |                lib_m4a_dec.a                 |                      M4A和AAC格式解码库                      |
> |                lib_aac_enc.a                 |                        AAC格式编码库                         |
> |                lib_opus_dec.a                |                        OPUS格式解码库                        |
> |                lib_opus_enc.a                |                        OPUS格式编码库                        |
> |                  libspeex.a                  |                       SPX格式编解码库                        |
> |                lib_wma_dec.a                 |                        WMA格式解码库                         |
> |               lib_wma_dec_hw.a               |                 带硬件FFT优化的WMA格式解码库                 |
> |                lib_ape_dec.a                 |                        APE格式解码库                         |
> |                lib_flac_dec.a                |                        FLAC格式解码库                        |
> |                lib_wav_dec.a                 |                        WAV格式解码库                         |
> |                lib_dts_dec.a                 |                        DTS格式解码库                         |
> |                lib_amr_dec.a                 |                        AMR格式解码库                         |
> |                lib_amr_enc.a                 |                    8K采样率AMR格式编码库                     |
> |               lib_amrwb_enc.a                |                    16K采样率AMR格式编码库                    |
> |               lib_adpcm_enc.a                |                       ADPCM格式编码库                        |
> |                   libvad.a                   |                         语音活动检测                         |
> |              lib_rmono2stereo.a              |              单声道数据立体声化（混响功能需要）              |
> |               lib_reverb_cal.a               |                   混响处理（混响功能需要）                   |
> |              lib_pitchshifter.a              |              移频变声等魔音处理（混响功能需要）              |
> |                lib_howling.a                 |                   啸波抑制（混响功能需要）                   |
> |              lib_pitch_speed.a               |                       音频变速变调功能                       |
> |                 liblimiter.a                 |                          软件限幅器                          |
> |                   libaec.a                   |                           回声消除                           |
> |                lib_usb_syn.a                 |                     UAC时钟信号同步处理                      |
> |                lib_sbc_eng.a                 |                   SBC格式音频数据能量检测                    |
> |               lib_roobo_aec.a                |                      roobo打断唤醒算法                       |
> |               lib_roobo_fig.a                |                  roobo打断唤醒算法授权管理                   |
> |               lib_wanson_asr.a               |                       华镇打断唤醒算法                       |
> |          libauth.a   libauth_aisp.a          |                  思必驰打断唤醒算法授权管理                  |
> | aisIntlib_single_xiaoai.a   libaisp_single.a |           思必驰单麦打断唤醒算法（小爱同学唤醒词）           |
> |   aisIntlib_dual_xiaoai.a   libaisp_dual.a   |           思必驰双麦打断唤醒算法（小爱同学唤醒词）           |
> |              video_rec_server.a              |                   视频录像编码的中间层服务                   |
> |              video_dec_server.a              |                   视频播放解码的中间层服务                   |
> |                 net_server.a                 |                     视频网络实时流封装器                     |
> |            stream_media_server.a             |                      视频实时流RTSP回放                      |
> |                 ai_server.a                  |               第三方AI平台SDK的中间层管理接口                |
> |                  profile.a                   |                 第三方AI平台license授权管理                  |
> |                  btctrler.a                  |                    蓝牙协议栈（底层驱动）                    |
> |                  btstack.a                   |                          蓝牙协议栈                          |
> |                lib_sig_mesh.a                |                       ble mesh组网协议                       |
> |            crypto_toolbox_Osize.a            |                     蓝牙专用加解密工具箱                     |
> |                lib_ccm_aes.a                 |                     ble配对AES硬件加解密                     |
> |                wl_rf_common.a                |                       RF模拟初始化接口                       |
> |               cJSON.a   json.a               |           两套JSON格式构造和解析接口（第三方开源）           |
> |                 ftpclient.a                  |                        FTP协议客户端                         |
> |                 ftpserver.a                  |                        FTP协议服务端                         |
> |                  http_cli.a                  |                        HTTP协议客户端                        |
> |                http_server.a                 |                        HTTP协议服务端                        |
> |                  lib_mqtt.a                  |                        MQTT客户端协议                        |
> |                   iperf.a                    |             iperf网络带宽测速工具（第三方开源）              |
> |                  libmpeg.a                   |                  M3U8格式解析（第三方开源）                  |
> |                    kcp.a                     |                           KCP协议                            |
> |          libairplay.a    libmdns.a           |                         AIRPLAY协议                          |
> |                  libdlna.a                   |                   DLNA投屏协议（音箱功能）                   |
> |                  libcoap.a                   |                    COAP协议（第三方开源）                    |
> |                 websocket.a                  |                        WEBSOCKET协议                         |
> |                 libJlCloud.a                 |                   杰理平台（第三方AI SDK）                   |
> |               libecho_cloud.a                |                  回声云平台（第三方AI SDK）                  |
> |                   libdui.a                   |                  思必驰平台（第三方AI SDK）                  |
> |               libduer-device.a               |                   小度平台（第三方AI SDK）                   |
> |                   libdcl.a                   |                   义语平台（第三方AI SDK）                   |
> |                 libaliyun.a                  |                  阿里云平台（第三方AI SDK）                  |
> |                 libtelecom.a                 |                  电信云平台（第三方AI SDK）                  |
> |                 libtencent.a                 |                  腾讯云平台（第三方AI SDK）                  |
> |                 libturing.a                  |                   图灵平台（第三方AI SDK）                   |
> |             libvtbk.a   libwt.a              |               玩瞳绘本识别平台（第三方AI SDK）               |
> |       tmallgenie.a   tmallgenie_todo.a       |                 天猫精灵平台（第三方AI SDK）                 |
> |                  zliblite.a                  |                  zip压缩解压（第三方开源）                   |
> |                 pixel_flow.a                 |                         杰理飞控算法                         |
> |             page_turning_dect.a              |                       杰理翻页检测算法                       |
> |              network_download.a              |                       网络歌曲下载接口                       |
> |               lib_sound_wave.a               |                           声波识别                           |
> |                 libqrcode.a                  |                          二维码识别                          |
> |                  mongoose.a                  | 为客户端和服务端实现HTTP等网络协议的事件驱动的非阻塞API（第三方开源） |
> |                  mbedtls.a                   |               嵌入式SSL/TLS功能（第三方开源）                |
> |                    uip.a                     |               轻量级TCP/IP协议栈（第三方开源）               |
> |                  wolfmqtt.a                  |                   MQTT客户端（第三方开源）                   |
> |                  wolfssl.a                   |               嵌入式SSL/TLS功能（第三方开源）                |
> |                 lwip_2_1_2.a                 |                  TCP/IP协议栈（第三方开源）                  |
> |               lwip_2_1_2_sfc.a               |           TCP/IP协议栈（第三方开源）（内存优化版）           |
> |                    hsm.a                     |                WIFI中间层状态机和事件回调管理                |
> |               wpasupplicant.a                |            WEP,WPA/WPA2和WAPI无线协议和加密认证的            |
> |                  wl_wifi.a                   |                 WIFI协议栈（支持AP+STA模式)                  |
> |                wl_wifi_sfc.a                 |          WIFI协议栈（支持AP+STA模式)（内存优化版）           |
> |                 wl_wifi_ap.a                 |                   WIFI协议栈（支持AP模式)                    |
> |               wl_wifi_ap_sfc.a               |            WIFI协议栈（支持AP模式)（内存优化版）             |
> |                wl_wifi_sta.a                 |                   WIFI协议栈（支持STA模式)                   |
> |              wl_wifi_sta_sfc.a               |              WIFI协议栈（STA模式)（内存优化版）              |
>
> 