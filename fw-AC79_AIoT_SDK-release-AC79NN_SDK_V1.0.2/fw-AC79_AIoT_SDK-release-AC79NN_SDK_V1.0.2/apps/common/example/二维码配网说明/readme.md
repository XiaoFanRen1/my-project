﻿# 二维码配网说明

> 0. 打开微信搜索公众好杰理智能机器人点击右下角更多打开二维码配网。
> 1. 输入wifi名，以及密码点击显示二维码即可然后将二维码对准设备摄像头即可完成配网（需要进入设备配网模式）

## 代码说明
>app_config.h中需要开启CONFIG_QR_CODE_NET_CFG //二维码配网  该宏 以及需要包含二维码算法库libqrcode.a  
>
>```c
>在video_rt_usr.c中 该函数创建了摄像头的调用 以及二维码算法的运行 
>void config_network_get_yuv_init(void)
>{
>    get_yuv_init(get_one_frame); 
>}
>config_network_get_yuv_init();包含在config_network.c的config_network_start（）中 所以我们在自己定义二维码配网时候调用该函数即可 
>当配好了网后可调用 config_network_stop(); 就能停止二维码配网
>



