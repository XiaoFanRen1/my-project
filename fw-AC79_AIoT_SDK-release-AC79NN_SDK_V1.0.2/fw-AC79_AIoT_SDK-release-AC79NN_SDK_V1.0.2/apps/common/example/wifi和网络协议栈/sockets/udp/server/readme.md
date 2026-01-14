# UDP Server示例工程说明

> 本工程展示了UDP Server模块接口使用方法:
>
> 通过创建的udp socket并绑定指定ip和端口号后，等待接收客户端数据，当成功接收到数据后，再重新把接收到的数据发送回客户端

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

## UDP服务端创建步骤

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
> unsigned int opt = 1;
> if(sock_setsockopt(fd, SOL_SOCKET, SO_REUSEADDR ,&opt, sizeof(opt)) < 0)
> {
> 	printf("sock_setsockopt fail\n");
> }
> ```
>
> 3.绑定IP地址、端口号等信息
>
> ```c
> struct sockaddr_in s_addr;
> 
> s_addr.sin_family = AF_INET;
> s_addr.sin_addr.s_addr = htonl(INADDR_ANY)
> s_addr.sin_port = htons(32768);
> 
> if (sock_bind(fd, (struct sockaddr *)&s_addr), sizeof(s_addr))
> {
> 	printf("sock_bind fail\n");
> }
> ```
>
> 4.接收数据
>
> ```c
> int recv_len;
> char recv_buf[1024];
> struct sockaddr_in c_addr = {0};
> socklen_t len = sizeof(c_addr);
> 
> for（;;）
> {
> 	recv_len = sock_recvfrom(fd, recv_buf, sizeof(recv_buf), 0, \
> 	  						(struct sockaddr *)&c_addr, &len);
> 	if(recv_len > 0)
> 	{
> 		printf("received %d Bytes, data : %s\n", recv_len, recv_buf);
> 	}
> }
> ```
>
> 5.发送数据
>
> ```c
> int send_len = sock_sendto(fd, send_buf, strlen(send_buf), 0, \
> 						   (struct sockaddr *)&c_addr, sizeof(struct sockaddr_in));
> ```
>
> 6.关闭socket
>
> ```c
> if(fd)
> {
> 	sock_unreg(fd);
> 	fd = NULL;
> }
> ```

---



## 模块依赖

> * 网络连接需要依赖的库

---



### 操作说明：

> 1. 示例需要运行在STA模式。
> 2. 编译工程，烧录镜像，复位启动。
> 3. 系统启动并成功联网后，借助tcp助手向指定ip和端口号的服务端发送数据，通过串口软件可以看到tcp助手发送过来的数据，同时tcp助手也会接收到客户端返回的同样的数据。

> JIELI SDK的编译、烧写等操作方式的说明可在以下文档获取：
> [文档](/doc/stuff/usb%20updater.pdf)

### 控制命令

> * N/A

### 代码流程

> 1. main()入口：
>
>    A）创建udp_server_start任务
>
> 2. tcp_server_start函数流程
>
>     A）通过wifi_get_sta_connect_state函数检测是否通过dhcp成功获取到ip
>
>     B)  成功通过dhcp获取到ip后，调用udp_server_init进行初始化，退出udp_server_start
>
> 3. udp_server_init函数流程
>
>     A）创建UDP socket，绑定ip和端口号
>
>     B)   创建接收客户端数据任务udp_recv_handler
>
> 4. udp_recv_handler函数流程
>
>     A)  等待接收客户端数据，成功接收到数据后再重新把同样的数据发回给客户端
>
> 5. 调用udp_server_exit可关闭连接
---

## 常见问题

> * N/A

## 参考文档

> * N/A