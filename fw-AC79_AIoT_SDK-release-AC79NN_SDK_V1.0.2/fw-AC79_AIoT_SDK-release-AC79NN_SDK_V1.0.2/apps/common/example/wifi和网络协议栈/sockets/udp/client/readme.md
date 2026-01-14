# UDP Client示例工程说明

> 本工程展示了UDP Client模块接口使用方法:
>
> UDP通信无需建立连接关系，创建套接字后便可以向指定服务端发送数据，示例请参考UDP Server

---

## 适用平台

> 本工程适用以下芯片类型：
>
> 1. AC79系列芯片：AC790N、AC791N

> 杰理芯片和评估板的更多信息可在以下地址获取：
> [链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)

## 工程配置说明

> 在SDK选择[PIC_BOOK_STORY](../../../../../../apps/wifi_story_machine/board)工程CBP文件或者工程Makefile, 然后添加本事例工程代码
>
> 工程CBP文件或者工程Makefile 定义 CONFIG_NET_ENABLE
>
> app_config.h：
>
> * 打开 #define CONFIG_WIFI_ENABLE

## UDP客户端创建步骤

> 1.创建socket
>
> ```c
> void *fd = NULL;
> fd = sock_reg(AF_INET, SOCK_DGRAM, 0, NULL, NULL);
> if(NULL == fd)
> {
> 	printf("sock_reg fail\n");
> }
> ```
>
> 2.设置socket属性（可选）
>
> ```c
> 如：
> unsigned int millsec = 100;
> if(ock_setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (const void *)&millsec, \		                           sizeof(millsec)) < 0)
> {
> 	printf("sock_setsockopt fail\n");
> }
> ```
> 
>3.绑定IP地址、端口号等信息（可选）
> 
>```c
> struct sockaddr_in c_addr;
> 
> c_addr.sin_family = AF_INET;
> c_addr.sin_addr.s_addr = htonl(INADDR_ANY)
> c_addr.sin_port = htons(32768);
> 
> if (sock_bind(fd, (struct sockaddr *)&c_addr), sizeof(c_addr))
> {
> 	printf("sock_bind fail\n");
> }
> ```
> 
>4.接收数据
> 
>```c
> int recv_len;
> char recv_buf[1024];
> struct sockaddr_in s_addr = {0};
> socklen_t len = sizeof(s_addr);
> 
> for（;;）
> {
> 	recv_len = sock_recvfrom(fd, recv_buf, sizeof(recv_buf), 0, \
> 	  						(struct sockaddr *)&s_addr, &len);
> 	if(recv_len > 0)
> 	{
> 		printf("received %d Bytes, data : %s\n", recv_len, recv_buf);
> 	}
> }
> ```
> 
>5.发送数据
> 
>```c
> int send_len = sock_sendto(fd, send_buf, strlen(send_buf), 0, \
> 						   (struct sockaddr *)&s_addr, sizeof(struct sockaddr_in));
> ```
> 
>6.关闭socket
> 
>```c
> if(fd)
> {
> 	sock_unreg(fd);
> 	fd = NULL;
> }
> ```
> 

---



## 模块依赖

> * 网络连接需要依赖的库

---



### 操作说明：

> JIELI SDK的编译、烧写等操作方式的说明可在以下文档获取：
> [文档](/doc/stuff/usb%20updater.pdf)

### 控制命令

> * N/A

---

## 常见问题

> * N/A

## 参考文档

> * N/A