# COAP示例说明

> 展示了coap client和coap server的用法

---

## COAP协议

> ![](C:\Users\ZPC20-011\Desktop\wl80sdk\trunk\AC79_SDK_REVIEW\apps\common\example\coap\coapFormat.png)
>
> ​														                     Message Format
>
> ```
> 1.Ver（版本编号）
> 	固定为0b01
> 2.T(报文类型)
> 	coap协议中定义了4中报文类型，分别为：
> 	CON(Confirmable，T=0b00):需要确认的报文类型。
> 	NON(Non-Confirmable, T=0b01)：不需要确认的报文类型。
> 	ACK(Acknowledgement， T=0b10):应答报文
> 	Reset（T=0b11）：复位报文
> 3.TKL（标签长度）
> 4.Code（格式：c（3bit class type）.dd(5bit detail code)）
> 	（1）表示请求时
> 	 	Code=0.01,表示GET方法
> 	 	Code=0.02，表示POST方法
> 	 	Code=0.03，表示PUT方法
> 	 	Code=0.04，表示DELETE方法
> 	（2）表示响应时
> 		Code=0.00，表示空报文
> 		Code=2.xx，表示为正确的响应
> 		Code=4.xx，表示为客户端的错误响应
> 		Code=5.xx，表示为服务器的错误响应
> 5.Message ID（报文序号）
> 	COAP请求和对应的COAP响应采用相同的Message ID
> 6.Token（标签）
> 7.Options（选项）
> 	选项包括Uri-Host、Uri-Path、Uri-Port、Content-Format、Accept等。
> 8.0xFF（分隔符）
> 9.payload（数据）
> ```
>

## 工程目录结构

```
.
├── coap_client               # coap客户端示例
├── coap_server               # coap服务器示例
├── readme.md                 # 工程说明文件
```

## 常见问题

> * N/A

## 参考文档

> * [libcoap官网](https://libcoap.net/)
> * [copper调试方法.docx](./copper调试方法.docx)