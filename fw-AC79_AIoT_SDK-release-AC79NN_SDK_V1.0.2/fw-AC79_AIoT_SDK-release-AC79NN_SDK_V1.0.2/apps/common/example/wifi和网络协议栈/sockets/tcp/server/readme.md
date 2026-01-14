

# TCP Server示例工程说明

> 本工程展示了TCP Server模块接口使用方法。
>
> 通过创建TCP socket并绑定指定端口后开启监听，等待客户端连接。当接收到客户端连接请求并成功建立连接后，应用程序会等待接收客户端的数据，当服务端成功接收到客户端的数据后，会向对应客户端发送接收数据成功的信息

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

## 模块依赖

> * 网络连接需要依赖的库

---

## TCP服务端创建步骤

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
> 4.开启监听
>
> ```c
> if (sock_listen(fd, 3))
> {
> 	printf("sock_listen fail\n");
> }
> ```
>
> 5.阻塞等待客户端连接
>
> ```c
> void *c_fd = NULL;
> int flag = 1;
> struct sockaddr_in c_addr;
> socklen_t len = sizeof(struct sockaddr_in); 
> 
> c_fd = sock_accept(fd, (struct sockaddr*)&c_addr, &len, NULL, NULL);
> flag = 0;
> if(c_fd == NULL)
> {
> 	printf("sock_accept fail\n");
> }
> ```
>
> 6.接收数据
>
> ```c
> char buf[1024] = {0};
> int recv_len = sock_recv(fd, buf, sizeof(buf), 0);
> if(recv_len > 0)
> {
> 	printf("received %d Bytes, data : %s\n", recv_len, buf);
> }
> ```
>
> 7.发送数据
>
> ```c
> char *send_buf = "123456";
> int send_len = sock_send(fd, send_buf, strlen(send_buf), 0);
> ```
>
> 8.关闭socket
>
> ```c
> if(fd)
> {
> 	flag = 1;
> 	sock_set_quit(fd);
> 	while(flag)   //等待sock_accept退出后再释放socket，防止释放socket后还在使用。
> 	{
> 		os_time_dly(20);
> 	}
> 	sock_unreg(fd);
> 	fd = NULL;
> }
> ```
>

---



### 操作说明：

> 1. 示例需要运行在STA模式
> 2. 编译工程，烧录镜像，复位启
> 3. 系统启动并成功联网后，借助tcq助手创建客户端，设置好服务端ip和端口号后发起连接，成功连接后可以通过串口软件看到提示信息，此时客户端可以向服务端发送一些数据，当服务端成功接收到数据后，会向对应客户端发送接收数据成功的信息

> JIELI SDK的编译、烧写等操作方式的说明可在以下文档获取：
> [文档](/doc/stuff/usb%20updater.pdf)

### 控制命令

> * N/A

### 代码流程

> 1. main()入口：
>
>     A）创建tcp_server_start任务
>
> 2. tcp_server_start函数流程
>
>     A）通过wifi_get_sta_connect_state函数检测是否通过dhcp成功获取到ip
>
>     B)  成功通过dhcp获取到ip后，调用tcp_server_init进行初始化，退出tcp_server_start
>
> 3. tcp_server_init函数流程
>
>     A）创建TCP socket。
>
>     B)   创建接收客户端连接请求任务tcp_sock_accpet
>
>     C)   创建接收客户端数据任务tcp_recv_handler
>
> 4. tcp_sock_accpet函数流程
>
>     A)  等待客户端连接，成功接收到客户端请求后，会把对应客户端的信息保存到链表中
>
> 5. tcp_recv_handler函数流程
>
>     A)  接收客户端数据，成功接收到数据后，向对应客户端发送接收数据成功的信息
>
> 6. 调用tcp_server_exit可关闭网路连接，需要使用者自行调用
---

## 常见问题

> * N/A

## 参考文档

> * N/A