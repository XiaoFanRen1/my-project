# 扫描笔图像拼接说明

本例子使用SPI摄像头BYD30A2输出only_Y(黑白)，图像大小176*128，帧率50帧。图像拼接算法库为img_stitch.a杰理自研拼接算法 。图像拼接帧率为66帧以上即满足一次图像数据来到下一张图像来的时间段完成一次拼接并输出数据。

## 依赖文件

>spi_recv.c用于开启收摄像头并接收数据以及转发数据，spi_data_deal.c处理摄像头数据并进行拼接算法。byd30a2.c摄像头驱动。app_music.c摄像头初始化以及按键处理（img_stitch.a算法库文件 ，fast_corner.h, img_stitch.h, img_transform.h, matrix_process.h算法库头文件4个）
 
## 拼接参数以及函数说明

初始化接口
img：第一帧图像
width，height：输入图像分辨率
output_img：输出图像
w_out，h_out：拼接后输出图像分辨率
outh_off：输出图像行移动的像素数
Max_corner_count：角点最大数量，通常取128或64
Fast_corner_mode：0和1两种模式，通常取1
Fast_th：角点阈值，通常取80左右
Match_th：匹配阈值，通常取2560或2560*4/3
offset_th：预匹配的阈值，通常取5
nms_windon_size:非极大值抑制的窗口，一般取7
Max_filter_th_x，Max_filter_th_y：最大值滤波的x,y方向的阈值，一般取5，3
Frame_stride：拼接时帧的步长，一定程度上减缓拼接后的缝隙
Adjustment_enable：处理放大问题的使能
max_off：行偏移的最大数，提高匹配的速度。
void img_stitch_reinit(uint8_t *img, int width, int height, uint8_t *output_img, int w_out, int h_out, int outh_off, int Max_corner_count, int Fast_corner_mode, uint32_t Fast_th, uint32_t Match_th, int Adjustment_enable);
重新初始化接口
void img_stitch_deinit();
结束时调用该接口，释放内存
int img_stitch_process(uint8_t *src_img, uint8_t *dst_img, uint8_t *output_img, uint8_t flag_force_stitch, uint8_t flag_force_exit);
src_img：第一帧图像
dst_img：第二帧图像
output_img：拼接后输出图像
flag_force_stitch：是否强制拼接（当拼接失败后，可以返回，也可以将该值设置为1，则会按照上一帧拼接的结果来拼接）
flag_force_exit：1代表强制推出。
返回值：返回1代表拼接成功，否则拼接失败。若为6，说明拼接后的输出图像达到了给定的最大buf，再继续拼接，会出现内存泄露。
int post_process(uint8_t *src, uint8_t *dst, int width, int height, int center_x, int center_y);
后处理接口:对图像进行旋转矫正
src:输入图像
dst：输出图像
width, height:输入和输出图像的分辨率
center_x,center_y：中心点（拼接图像的中心点，行需要加上outh_off）
返回值为1代表进行了矫正，为0代表没有进行矫正
enum img_stitch_status
{
	success = 1,
	corner_count_err,
	match_count_err,
	mapping_err,
	direction_err,
	out_w_err,
	force_exit_err,
};


## 操作说明
>确认工程中是否添加好对应依赖文件没有的话请自行进行添加。
>   #ifdef CONFIG_SCAN_PEN_ENABLE
	   #define CONFIG_VIDEO1_ENABLE
	   #define CONFIG_SPI_VIDEO_ENABLE
    #endif
注意CONFIG_SCAN_PEN_ENABLE定义及开启了CONFIG_VIDEO1_ENABLE， CONFIG_SPI_VIDEO_ENABLE
这个是摄像头使能宏
>测试代码在工程AC790N_picture_book_story，找到app_config.h。 使用名字CONFIG_BOARD_7901BA_TEST_STORY板级  打开CONFIG_SCAN_PEN_ENABLE宏需要自行确认硬件配置，不然测试无法正常进行。 将扫描笔头放置书本上，并触发目标按键按下，进行拼接代码。扫描笔抬起结束拼接并将结果保存SD卡中。

## 代码流程
> 在app_music.c中调度开始  具体请阅读spi_data_deal.c文件