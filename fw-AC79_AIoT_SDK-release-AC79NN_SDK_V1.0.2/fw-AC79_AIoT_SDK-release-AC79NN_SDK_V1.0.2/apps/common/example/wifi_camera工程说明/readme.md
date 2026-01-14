# wifi_camera 工程说明

## wifi_ camera工程说明
>wifi_camera实现了wifi和摄像头结合的应用案例，例如当前火热的wifi航拍摄像头，IPC摄像头，儿童相机，USB摄像头（内窥镜），行车记录仪等,均都成熟案例。该工程提供了完善的视频，音频，显示，ui，wifi等接口，提供多样化的demo工程加快用户的熟悉和开发速度。该IC以低成本，设计简单，高性能，占据了市场的份额。并且提供强大的售后团队为您解决开发过程中的疑难杂症。
>wifi传输距离在空旷无干扰环境能轻松透传在100米。(640*480，15fps)
>ipc网络透传在网络良好的情况下能流程运行20帧640*480图像。
>儿童相机支持双图层显示，视频播放，音频播放，图像合成320*240能流畅运行20帧图像。

## wifi图传操作说明
>1.在公版SDK目录下找到sdk_tools/Android_DVRunning2_V2.8.0-20201113.apk  安装到手机
>设备下载好公版SDK中 wifi_camera只需要确摄像头连接是否对应默认板级配置即可。
>2.设备通电，打开DVRunning2，第一次app全部权限开启 ，点击连接设备找到名字为wifi_camera，并且信号为满格的wifi进行连接。默认wifi密码12345678进行连接。连接成功后返回APP即可出现图层图像。

## wifi图传代码流程重点讲解
>1.开机时候设备进行wifi初始化时会进行网络部分的初始化（net_app_init();）比便于后面更手机wifi通信使用。
>2.连接上设备后，当打开app手机就会和设备创建CTP连接 ,控制命名WIFI_CTP_CMD<<CTP_CLI_CONNECTED>>（ctp_cmd.c）
>设备连接上后会向手机进行核对信息例如手机类型，安卓和苹果有小区别。控制命令WIFI_CTP_CMD<<APP_ACCESS>>（ctp_cmd.c）
>获取手机的时间。控制命令 WIFI_CTP_CMD<<DATE_TIME>>（ctp_cmd.c）
>打开视频实时流WIFI_CTP_CMD<<OPEN_RT_STREAM>> （cmd_put_open_rt_stream（））该函数调用net_video_rec（）完成摄像头的初始化（net_video_rec0_start()）（ __this_net->net_video_rec = server_open("video_server", "video0.1");）以及图传数据通道的建立（stream_protocol_task_create）（net_video_rec.c）
3.app出图后所有的控制命令都会流入net_video_rec_control();


## 参考

> * N/A

