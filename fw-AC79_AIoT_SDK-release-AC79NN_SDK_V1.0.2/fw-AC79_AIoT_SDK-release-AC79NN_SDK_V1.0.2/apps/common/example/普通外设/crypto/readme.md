# 硬件加密使用示例工程说明

> 本工程展示了硬件加密接口使用示例:
>
> 1. AES128编解码使用
> 2. SHA1编码
> 3. SHA256编码


---

## 适用平台

> 本工程适用以下芯片类型：
> 1. AC79系列芯片：AC790N、AC791N
>
> 杰理芯片和评估板的更多信息可在以下地址获取：[链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)

## 工程配置说明

> 如果设备没有启动WIFI功能或者不在STA模式,在SDK选择[demo_hello](../../../../apps/demo_hello/board)主工程文件或者主工程Makefile, 然后添加本事例工程代码
>
> 如果设备启动了WIFI功能,需要把设备配置在STA模式,在SDK选择[demo_wifi](../../../../apps/demo_wifi/board)主工程文件或者主工程Makefile, 然后添加本事例工程代码

---



### 操作说明：

> 2. 编译工程，烧录镜像，复位启动
> 3. 系统启动后，可以通过串口软件看到示例的打印信息
>
> JIELI SDK的编译、烧写等操作方式的说明可在以下文档获取：[文档](../../../../doc/stuff/usb updater.pdf)

### 代码流程


> 1. c_main()入口：
>     A）AES128编解码使用
>	B）SHA1编码
>	C）SHA256编码


---

## 常见问题

> * AES能256bit吗
>
>   答:能，jl_aes_encrypt_hw 的key和key长度改成32
>
>   
>
> * 可以使用HMAC 或 MD5之类的吗？
> 
>   答:可以使用mbedtls，在工程中添加mbedtls.a即可

## 参考文档

> * N/A