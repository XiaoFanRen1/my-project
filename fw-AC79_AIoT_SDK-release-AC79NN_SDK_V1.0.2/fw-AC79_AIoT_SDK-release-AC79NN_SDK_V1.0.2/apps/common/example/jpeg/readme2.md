#   获取JPG测试说明

> 本工程展示了如果获取摄像头YUV数据并将数据转为JPG并存储在SD卡中。

---

## 适用平台

> 本工程适用以下芯片类型：
>
> 1. AC79系列芯片：AC790N、AC791N
>
> 杰理芯片和评估板的更多信息可在以下地址获取：[链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)
>
> 
## 工程配置说明

在SDK选择[demo_ui](../../../../apps/demo_ui/board)主工程文件或者主工程Makefile, 然后添加本事例工程代码。

1、使能SD卡配置，在板级配置好摄像头IO,通过获取摄像头YUV进行缩放转JPG。

2、开启VIDEO宏配置 

### 1、获取JPG函数说明
>
>```c
>struct jpeg_encode_req req = {0};//JPG编码
>req.q = 13;	//jepg编码质量(范围0-13),0最小 质量越好,下面的编码buf需要适当增大
>req.format = JPG_SAMP_FMT_YUV420;//YUV编码格式
>req.data.buf = yuv_out_buf;//传入底层编码器的buf
>req.data.len = buf_len;//传入数据长度
>req.width = yuv_out_w ;//传入YUV数据宽度
>req.height =  yuv_out_h ;//传入YUV数据高度
>req.y = yuv_out_buf;//出入数据Y
>req.u = req.y + (yuv_out_w * yuv_out_h);//输出数据U
>req.v = req.u + (yuv_out_w * yuv_out_h) / 4;//输入数据V
>err = jpeg_encode_one_image(&req);//获取一张JPG图片
>
>```

### 2、JPG数据说明

> ```
> 通过修改宏yuv_out_w， yuv_out_h 就可以输出对应尺寸的JPG大小
> 一般情况下摄像头拿到YUV数据要经过 YUV缩放成理想发现在进行处理 在8M SDRAM 情况下支持最大400万像素JPG编码  8M SDRAM 情况下支持最大150万像素JPG编码  
>```
> 
>
> 
>
## 操作说明

> 1. 使用串口线连接打印
> 2. 使用demo_video工程加入测试文件，烧录镜像，复位启动
> 3. 系统启动后，查看系统打印.
>
> JIELI SDK的编译、烧写等操作方式的说明可在以下文档获取：[文档](../../../../doc/stuff/usb updater.pdf)
---

## 常见问题

> * N/A

## 参考文档

> * N/A