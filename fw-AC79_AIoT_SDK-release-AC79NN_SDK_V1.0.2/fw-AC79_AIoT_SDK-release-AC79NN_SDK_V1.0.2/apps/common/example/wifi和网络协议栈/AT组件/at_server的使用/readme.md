# AT组件使用说明

> 本工程展示了AT组件使用：
>
> 1.电脑PC的串口助手作为AT客户端用于发送AT命令
>
> AC79芯片作为AT服务端响应AT命令，返回执行结果到串口助手
>
> 2.添加自定义AT命令

---

## 适用平台

> 本工程适用以下芯片类型：
> 1. AC79系列芯片：AC790N、AC791N
>
> 杰理芯片和评估板的更多信息可在以下地址获取：[链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)
>

## 工程配置说明

> 1.在SDK选择[demo_hello](../../../../../apps/demo/demo_hello/board)主工程文件或者主工程Makefile
>
> 2.在app_main.c中包含头文件 
>
> ​    #include "at.h"
>
> 3.在工程的board.c文件中配置相对应的串口并增加如下at_get_uart()函数返回，串口需要同时用做打印和通信功能，工程默认uart1为AT命令发送和接收数据串口，关于串口的具体配置可以参考 [readme.md](..\..\uart\readme.md) 。
>
> <img src=".\2.png" alt="image-20210819134036" style="zoom:60%;" >

## 模块依赖：

> - at.a 文件库
>

## 操作说明：

> 1.工程配置之后，启动AT Server 功能时，需要对它进行初始化，在主函数app_main()调用如下函数：
>
> ​	at_server_init()
>
> 2.烧录成功之后，在电脑PC串口发送如下默认支持的操作命令，输入操作为"命令+\r+回车键"，其中\r
>
> 为命令结束标志，所有命令必须以此标志结尾
>
> - AT：测试AT启动命令；
> - ATZ：设备恢复出厂设置；
> - AT+RST：设备重启；
> - AT&L：列出全部命令列表；
> - AT+UART：显示串口参数；
>
> 3.观察串口打印的响应结果， 例如：输入如下命令AT\r、AT+UART?\r，均会响应相关执行结果
>
> <img src=".\1.png" alt="image-20210819134036" style="zoom:70%;" >
>
> 
>
> 

---

## 常见问题

> * 如何自定义AT命令？
>
>   答：用户想使用更多功能需要针对不同 AT 设备完成自定义 AT Server 命令，AT 组件提供类似于 finsh/msh命令添加方式的 AT 命令添加方式，方便用户实现需要的命令。
>
>   AT 命令根据传入的参数格式不同可以实现不同的功能，对于每个 AT 命令最多包含四种功能，如下所述：
>
>   - 测试功能：`AT+<x>=?` 用于查询命令参数格式及取值范围；
>   - 查询功能：`AT+<x>?` 用于返回命令参数当前值；
>   - 设置功能：`AT+<x>=...` 用于用户自定义参数值；
>   - 执行功能：`AT+<x>` 用于执行相关操作。
>
>   每个命令的四种功能并不需要全部实现，用户自定义添加 AT Server 命令时，可根据自己需求实现一种或几种上述功能函数，未实现的功能可以使用 `NULL` 表示，再通过自定义命令添加函数添加到基础命令列表，添加方式类似于 finsh/msh命令添加方式，添加函数如下：
>
>   ```c
>   AT_CMD_EXPORT(_name_, _args_expr_, _test_, _query_, _setup_, _exec_);
>   ```
>
> ![image-20210819135709](.\3.png)
>
>   如下为 AT 命令注册"AT+TEST"命令示例:
>
> （1）在at_basd_cmd.c中添加如下"AT+TEST"命令的执行函数at_test_exec()、查询函数at_test_query()以及命令输出函数AT_CMD_EXPORT()
>
>   ```c
>   static at_result_t at_test_exec(void)
>   {
>       printf("AT test commands execute!");
> 
>       return AT_RESULT_OK;
>   }
>   static at_result_t at_test_query(void)
>   {
>       printf("AT test commands query!");
> 
>       return AT_RESULT_OK;
>   }
> 
>   AT_CMD_EXPORT("AT+TEST", "=<value1>", RT_NULL, at_test_query, RT_NULL, at_test_exec);
>   ```
>
> （2）添加后编译烧录程序，串口分别输入执行命令"AT+TEST\r"、查询命令"AT+TEST?\r"，执行结果如下
>
> <img src=".\4.png" alt="image-20210819134036" style="zoom:70%;" >

## 参考案例

> * N/A
>
>   