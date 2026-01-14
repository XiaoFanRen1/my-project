# TCP Client示例工程说明

> 本工程展示了TCP Client模块接口使用方法。
>
> 通过创建tcp socket连接到指定ip和端口号的服务端。当成功建立连接后，应用程序会向服务端发送ASCII数据，并等待服务端回复，当客户端成功接收到服务端的数据后，会向服务端发送接收数据成功的信息

---

## 适用平台

> 本工程适用以下芯片类型：
>
> 1. AC79系列芯片：AC790N、AC791N

> 杰理芯片和评估板的更多信息可在以下地址获取：
> [链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)

## TCP客户端创建步骤

> 1.创建socket
>
> ```c
> void *fd = NULL;
> fd = sock_reg(AF_INET, SOCK_STREAM, 0, NULL, NULL);
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
> if(sock_setsockopt(fd, IPPROTO_TCP, TCP_QUICKACK, &opt, sizeof(opt)) < 0)
> {
> 	printf("sock_setsockopt fail\n");
> }
> ```
>
> 3.绑定IP地址、端口号等信息（可选）
>
> ```c
> struct sockaddr_in local_addr;
> 
> local_addr.sin_family = AF_INET;
> local_addr.sin_addr.s_addr = htonl(INADDR_ANY)
> local_addr.sin_port = htons(32768);
> 
> if (sock_bind(fd, (struct sockaddr *)&local_addr), sizeof(local_addr))
> {
> 	printf("sock_bind fail\n");
> }
> ```
>
> 4.设置连接方IP和端口号, 并连接。
>
> ```c
> struct sockaddr_in dest_addr;
> 
> dest_addr.sin_family = AF_INET;
> dest_addr.sin_addr.s_addr = inet_addr("192.168.1.1")
> dest_addr.sin_port = htons(32769);
> 
> if(sock_connect(fd, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr_in)))
> {
> 	printf("sock_connect fail\n");
> }
> ```
>
> 5.接收数据
>
> ```c
> char buf[1024] = {0};
> int recv_len = sock_recvfrom(fd, buf, sizeof(buf), 0, NULL, NULL);
> if(recv_len > 0)
> {
> 	printf("received %d Bytes, data : %s\n", recv_len, buf);
> }
> ```
>
> 6.发送数据
>
> ```c
> char *send_buf = "123456";
> int send_len = sock_send(fd, send_buf, strlen(send_buf), 0);
> ```
>
> 7.关闭socket
>
> ```c
> if(fd)
> {
> 	sock_unreg(fd);
> 	fd = NULL;
> }
> ```
>

---



## 工程配置说明

> 在SDK选择[PIC_BOOK_STORY](../../../../../../apps/wifi_story_machine/board)工程CBP文件或者工程Makefile, 然后添加本事例工程代码
>
> 工程CBP文件或者工程Makefile 定义 CONFIG_NET_ENABLE
>
> app_config.h：
>
> * 打开 #define CONFIG_WIFI_ENABLE

## 模块依赖

> * 网络连接需要依赖的库

---



### 操作说明：

> 1. 示例需要运行在STA模式
> 2. 借助tcq助手创建指定ip和端口号的服务端，并保证tcp助手和客户端处于同一网段下
> 3. 编译工程，烧录镜像，复位启动
> 4. 系统启动并成功联网后，tcp助手会接收到客户端的ASCII数据，同时可以通过tcp助手向客户端发送数据，数据发送成功后可以通过串口软件看到服务端发送回来的数据

> JIELI SDK的编译、烧写等操作方式的说明可在以下文档获取：
> [文档](/doc/stuff/usb%20updater.pdf)

### 控制命令

> * N/A

### 代码流程

> 1. c_main()入口：
>     A）创建任务tcp_client_start
>
> 2. tcp_client_start函数流程
>
>     A）通过wifi_get_sta_connect_state函数检测是否通过dhcp成功获取到ip
>
>     B)  成功通过dhcp获取到ip后，启动tcp_client_task任务，退出tcp_client_start
>
> 3. tcp_client_task函数流程
>
>     A)  通过tcp_client_init进行TCP Client初始化，包括：创建socket，连接到服务端
>
>     B)  通过tcp_send_data向服务端发送数据
>
>     C)  通过tcp_recv_data接收服务端数据
---

## 常见问题

> * N/A

## 参考文档

> * N/A