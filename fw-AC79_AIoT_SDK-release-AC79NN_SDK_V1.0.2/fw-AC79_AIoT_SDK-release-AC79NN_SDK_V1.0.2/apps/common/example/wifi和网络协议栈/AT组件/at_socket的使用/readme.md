# at_socket使用示例

> 本工程展示了at_socket在AC79上的使用：
>
> 1.创建at_socket自定义AT命令
>
> "AT+TS" : 创建TCP SERVER模块接口命令
>
> "AT+TC" : 创建TCP CLIENT模块接口命令
>
> "AT+US" : 创建UDP SERVER模块接口命令
>
> 2.电脑PC的串口助手发送上述相关at_socket命令，AC79芯片会响应上述命令进而创建对应的TCP/UDP Socket模块接口，利用Socket网络调试助手配置对应的协议类型、IP地址、端口号即可进行Socket通信，在网络调试助手进行Socket收发数据时，串口助手也会收到提示信息

---

## 适用平台

> 本工程适用以下芯片类型：
> 1. AC79系列芯片：AC790N、AC791N
>
> 杰理芯片和评估板的更多信息可在以下地址获取：[链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)
>

## 工程配置说明

> 1.在SDK选择[demo_wifi](../../../../../apps/demo/demo_wifi/board)主工程文件或者主工程Makefile
>
> 2.工程CBP文件或者工程Makefile 中定义 CONFIG_NET_ENABLE
>
> app_config.h：
>
> * 打开 #define CONFIG_WIFI_ENABLE
>
> wifi_demo_task.c：
>
> - 包含头文件 #include "at.h"
> - 添加 late_initcall(at_server_init);
>
> 3.在工程的board.c文件中配置相对应的串口，串口需要同时用做打印和通信功能，工程默认uart1为AT命令发送和接收数据串口，关于串口的具体配置可以参考 [readme.md](..\..\uart\readme.md) 。

## 模块依赖：

> - at.a 文件库
>

## 操作说明：

> 1.工程配置之后，示例需要运行在STA模式，并保证Socket网络调试助手和AC79处于同一网段下
>
> 2.烧录成功之后，在电脑PC串口端发送如下支持的at_socket命令，输入操作为"命令+\r+回车键"，其中\r
>
> 为命令结束标志，所有命令必须以此标志结尾
>
> - "AT+TS" : 创建TCP SERVER模块接口命令
> - "AT+TC" : 创建TCP CLIENT模块接口命令
> - "AT+US" : 创建UDP SERVER模块接口命令
> - 关于TCP/UDP Socket模块接口的具体配置说明文档可以参考 [sockets](..\..\sockets) 。
>
> 3.下面以at_socket命令"AT+TS"为例创建TCP SERVER模块接口， 详细说明at_socket的使用
>
> （1）在demo_wifi工程的STA模式下，在电脑串口端输入命令AT+TS\r，得到如下响应，表示tcp server创建成功
>
> <img src=".\1.png" alt="image-20210819134036" style="zoom:60%;" >
>
> （2）在电脑PC端的Socket网络调试助手配置协议类型、IP地址、端口号如下
>
> <img src=".\4.png" alt="image-20210819134036" style="zoom:50%;" >
>
> ​          连接成功后串口助手会提示一下信息
>
> <img src=".\3.png" alt="image-20210819134036" style="zoom:60%;" >
>
> ​          发送ascii数据 JL_TECH_2021，网络调试助手得到如下响应
>
> <img src=".\5.png" alt="image-20210819134036" style="zoom:50%;" >
>
> （3）同时串口助手端也会收到如下提示信息
>
> <img src=".\6.png" alt="image-20210819134036" style="zoom:60%;" >

---

## 常见问题

> * at_socket预先配置各端口号是多少？怎么查看当前设备的IP地址？
>
>   答：at_socket所有预设的端口号都是60000，当前设备的IP地址通过串口的以下信息得到
>
>   <img src=".\7.png" alt="image-20210819134036" style="zoom:60%;" >
>
> * 串口配置默认配置为uart1，若要换为其他串口收发数据，在board.c中需要添加如下at_get_uart()函数
>
>   <img src=".\2.png" alt="image-20210819134036" style="zoom:60%;" >
>

## 参考案例

> * N/A
>
>   