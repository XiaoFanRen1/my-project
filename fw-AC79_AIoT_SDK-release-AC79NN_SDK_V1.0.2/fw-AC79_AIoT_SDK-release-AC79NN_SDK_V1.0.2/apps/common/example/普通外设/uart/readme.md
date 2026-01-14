﻿﻿﻿﻿﻿# UART示例工程说明

> 本工程展示了uart模块接口使用方法。
>
> 通过串口UART2收发数据演示uart模块DMA收发模式、接收非阻塞方法的使用

---

## 适用平台

> 本工程适用以下芯片类型：
> 1. AC79系列芯片：AC790N、AC791N

> 杰理芯片和评估板的更多信息可在以下地址获取：[链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)

## 工程配置说明

> 1.在SDK选择[demo_hello](../../../../apps/demo/demo_hello/board)或者其他demo的主工程文件或者主工程Makefile，测试例子在 [uart_test.c](uart_test.c) ，接收和发送进行两个板子对接测试例子在 
>
> [uart_tx_rx_test.c](uart_tx_rx_test.c) 。
>
> 2.在board.c：
>
> 配置好uart2_data的波特率,TX,RX出口等
>
> device_table 需要包含有 {"uart2", &uart_dev_ops, (void *)&uart2_data },
>
> 关于uart的具体配置可参考下面代码和后面的详细的UART配置说明
>
> * ```
>   UART1_PLATFORM_DATA_BEGIN(uart1_data)                       
>   	.baudrate = 460800,      			//波特率设置
>       .port = PORT_REMAP,      			//端口重映射
>       .tx_pin = IO_PORTC_00    			//发送引脚
>    	.rx_pin = IO_PORTC_01,  		    //接收引脚
>       .intput_channel = INTPUT_CHANNEL0,  //输入通道
>       .output_channel = OUTPUT_CHANNEL3,	//输出通道
>   	.max_continue_recv_cnt = 1024,      //连续接收最大字节
>   	.idle_sys_clk_cnt = 500000,     	//超时计数器，如果在指定的时间里没有收到任何数据，则产生超时中断
>       .clk_src = PLL_48M,          		//选择时钟源
>       .disable_tx_irq=1,     				//禁止发送中断开启
>       .disable_rx_irq=1, 					//禁止接收中断开启，一般情况不用开启，因为不清楚何时接收完成
>       .flags = UART_DEBUG,                //串口用作打印功能的标志位
>   UART1_PLATFORM_DATA_END(); 
>   ```
>
> 

## 模块依赖

> * cpu.a 包含串口底层硬件收发库

---



### 操作说明：

> 1. 使用串口线连接UART2接口
> 2. 编译工程，烧录镜像，复位启动
> 3. 系统启动后，可以通过串口软件给UART2发送数据，系统会将接受到的数据通过UART2发送回去

> JIELI SDK的编译、烧写等操作方式的说明可在以下文档获取：[文档](../../../../doc/stuff/usb updater.pdf)

### 代码流程

> 1. main()入口：
>     A）UART初始化
>     B）根据条件执行对应的uart收发模式
>     C）等待接受来自UART2的数据，收到数据后发送回去
---



# UART配置说明

## 两个串口使用情况配置：

### 串口2用做打印，串口1做通信只用RX引脚

- 串口2使用固定组引脚的配置方法，使用PORTC_9_10中的IO_PORTC_9作为打印, 

  串口1使用INPUT_CHANNEL3重映射引脚的配置方法，使用IO_PORTC_10作为通信RX引脚

```
UART1_PLATFORM_DATA_BEGIN(uart1_data)                       
	.baudrate = 460800,      
    .port = PORT_REMAP,  
    .tx_pin = -1,
 	.rx_pin = IO_PORTC_10,            
    .input_channel = INPUT_CHANNEL3,   
	.max_continue_recv_cnt = 1024, 
	.idle_sys_clk_cnt = 500000,     
    .clk_src = PLL_48M,          
    .disable_tx_irq=1,              
UART1_PLATFORM_DATA_END();  
```

```
UART2_PLATFORM_DATA_BEGIN(uart2_data)                       
	.baudrate = 1000000,                                   
    .port = PORTC_9_10,  
    .tx_pin = IO_PORTC_09,  
    .rx_pin = -1,
	.max_continue_recv_cnt = 1024, 
	.idle_sys_clk_cnt = 500000,    
    .clk_src = PLL_48M,                                     
    .flags = UART_DEBUG,
UART2_PLATFORM_DATA_END(); 
```



- 串口2使用OUTPUT_CHANNEL0重映射引脚的配置方法使用IO_PORTH_06作为打印, 
  串口1使用固定组引脚的配置方法，使用PORTH_6_7中的IO_PORTH_07作为通信RX引脚

```
UART1_PLATFORM_DATA_BEGIN(uart1_data)                       
	.baudrate = 460800,      
    .port = PORTH_6_7, 
    .tx_pin = -1,
 	.rx_pin = IO_PORTH_07,               
	.max_continue_recv_cnt = 1024, 
	.idle_sys_clk_cnt = 500000,     
    .clk_src = PLL_48M,          
    .disable_tx_irq=1,             
UART1_PLATFORM_DATA_END();  
```

```
UART2_PLATFORM_DATA_BEGIN(uart2_data)                       
	.baudrate = 1000000,                                   
    .port = PORT_REMAP,  
    .tx_pin = IO_PORTH_06,  
    .rx_pin = -1,
    .output_channel = OUTPUT_CHANNEL0,
	.max_continue_recv_cnt = 1024, 
	.idle_sys_clk_cnt = 500000,    
    .clk_src = PLL_48M,                                     
    .flags = UART_DEBUG,
UART2_PLATFORM_DATA_END(); 
```



- 串口2使用OUTPUT_CHANNEL0重映射引脚的配置方法，使用IO_PORTC_00作为打印, 
  串口1使用INPUT_CHANNEL3重映射引脚的配置方法，使用IO_PORTB_01作为通信RX引脚

```
UART1_PLATFORM_DATA_BEGIN(uart1_data)                       
	.baudrate = 460800,      
    .port = PORT_REMAP, 
    .tx_pin = -1,
 	.rx_pin = IO_PORTB_01,  
    .input_channel = INPUT_CHANNEL3,
	.max_continue_recv_cnt = 1024, 
	.idle_sys_clk_cnt = 500000,     
    .clk_src = PLL_48M,          
    .disable_tx_irq=1,             
UART1_PLATFORM_DATA_END();  
```

```
UART2_PLATFORM_DATA_BEGIN(uart2_data)                       
	.baudrate = 1000000,                                   
    .port = PORT_REMAP,  
    .tx_pin = IO_PORTC_00,  
    .rx_pin = -1,
    .output_channel = OUTPUT_CHANNEL0,
	.max_continue_recv_cnt = 1024, 
	.idle_sys_clk_cnt = 500000,    
    .clk_src = PLL_48M,                                     
    .flags = UART_DEBUG,
UART2_PLATFORM_DATA_END(); 
```



- 串口2使用固定组引脚的配置方法，使用PORTC_9_10中的IO_PORTC_9作为打印,  
  串口1使用固定组引脚的配置方法，使用PORTH_6_7中的IO_PORTH_07作为通信RX引脚

```
UART1_PLATFORM_DATA_BEGIN(uart1_data)                       
	.baudrate = 460800,      
    .port = PORTH_6_7, 
    .tx_pin = -1,
 	.rx_pin = IO_PORTH_07,               
	.max_continue_recv_cnt = 1024, 
	.idle_sys_clk_cnt = 500000,     
    .clk_src = PLL_48M,          
    .disable_tx_irq=1,             
UART1_PLATFORM_DATA_END();  
```

```
UART2_PLATFORM_DATA_BEGIN(uart2_data)                       
	.baudrate = 1000000,                                   
    .port = PORTC_9_10,  
    .tx_pin = IO_PORTC_09,  
    .rx_pin = -1,
	.max_continue_recv_cnt = 1024, 
	.idle_sys_clk_cnt = 500000,    
    .clk_src = PLL_48M,                                     
    .flags = UART_DEBUG,
UART2_PLATFORM_DATA_END(); 
```



### 串口2用做打印，串口1做通信只用TX引脚。

- 串口1使用重映射引脚的配置方法，将PC00重映射做tx引脚。使用了OUTPUT_CHANNEL3。

  串口2使用固定引脚的配置方法，使用了固定的一组引脚PC9,PC10。只用了PC09做tx脚。

```
UART1_PLATFORM_DATA_BEGIN(uart1_data)                       
	.baudrate = 460800,      
    .port = PORT_REMAP,  
    .tx_pin = IO_PORTC_00,
 	.rx_pin = -1,            
    .output_channel = OUTPUT_CHANNEL3,   
	.max_continue_recv_cnt = 1024, 
	.idle_sys_clk_cnt = 500000,     
    .clk_src = PLL_48M,          
    .disable_tx_irq=1,              
UART1_PLATFORM_DATA_END();  
```

```
UART2_PLATFORM_DATA_BEGIN(uart2_data)                       
	.baudrate = 1000000,                                   
    .port = PORTC_9_10,  
    .tx_pin = IO_PORTC_09,  
    .rx_pin = -1,
	.max_continue_recv_cnt = 1024, 
	.idle_sys_clk_cnt = 500000,    
    .clk_src = PLL_48M,                                     
    .flags = UART_DEBUG,
UART2_PLATFORM_DATA_END(); 
```



- 串口1使用重映射引脚的配置方法，将PC00重映射做tx引脚。使用了OUTPUT_CHANNEL3。

  串口2使用重映射引脚的配置方法，将PC01重映射做tx引脚。使用了OUTPUT_CHANNEL0。

```
UART1_PLATFORM_DATA_BEGIN(uart1_data)                       
	.baudrate = 460800,      
    .port = PORT_REMAP,  
    .tx_pin = IO_PORTC_00,
 	.rx_pin = -1,            
    .output_channel = OUTPUT_CHANNEL3,   
	.max_continue_recv_cnt = 1024, 
	.idle_sys_clk_cnt = 500000,     
    .clk_src = PLL_48M,          
    .disable_tx_irq=1,              
UART1_PLATFORM_DATA_END(); 
```

```
UART2_PLATFORM_DATA_BEGIN(uart2_data)                       
	.baudrate = 1000000,                                   
    .port = PORT_REMAP,  
    .tx_pin = IO_PORTC_01,  
    .rx_pin = -1,
    .output_channel = OUTPUT_CHANNEL0,
	.max_continue_recv_cnt = 1024, 
	.idle_sys_clk_cnt = 500000,    
    .clk_src = PLL_48M,                                     
    .flags = UART_DEBUG,
UART2_PLATFORM_DATA_END(); 
```



- 串口1使用固定引脚的配置方法，使用了固定的一组引脚PH06和PH07。只用了PH06做tx脚。

  串口2使用固定引脚的配置方法，使用了固定的一组引脚PC09和PC10。只用了PC09做tx脚。

```
UART1_PLATFORM_DATA_BEGIN(uart1_data)                       
	.baudrate = 460800,      
    .port = PORTH_6_7, 
    .tx_pin = IO_PORTH_06,
 	.rx_pin = -1,  
	.max_continue_recv_cnt = 1024, 
	.idle_sys_clk_cnt = 500000,     
    .clk_src = PLL_48M,          
    .disable_tx_irq=1,             
UART1_PLATFORM_DATA_END(); 
```

```
UART2_PLATFORM_DATA_BEGIN(uart2_data)                       
	.baudrate = 1000000,                                   
    .port = PORT_REMAP,  
    .tx_pin = IO_PORTC_01,  
    .rx_pin = -1,
    .output_channel = OUTPUT_CHANNEL0,
	.max_continue_recv_cnt = 1024, 
	.idle_sys_clk_cnt = 500000,    
    .clk_src = PLL_48M,                                     
    .flags = UART_DEBUG,
UART2_PLATFORM_DATA_END(); 
```



- 串口1使用固定引脚的配置方法，使用了固定的一组引脚PH06和PH07。只用了PH06做tx脚。

  串口2使用重映射引脚的配置方法，将PC01重映射做tx引脚。使用了OUTPUT_CHANNEL0。

```
UART1_PLATFORM_DATA_BEGIN(uart1_data)                       
	.baudrate = 460800,      
    .port = PORTH_6_7, 
    .tx_pin = IO_PORTH_06,
 	.rx_pin = -1,  
	.max_continue_recv_cnt = 1024, 
	.idle_sys_clk_cnt = 500000,     
    .clk_src = PLL_48M,          
    .disable_tx_irq=1,             
UART1_PLATFORM_DATA_END(); 
```

```
UART2_PLATFORM_DATA_BEGIN(uart2_data)                       
	.baudrate = 1000000,                                   
    .port = PORTC_9_10,  
    .tx_pin = IO_PORTC_09,  
    .rx_pin = -1,
	.max_continue_recv_cnt = 1024, 
	.idle_sys_clk_cnt = 500000,    
    .clk_src = PLL_48M,                                     
    .flags = UART_DEBUG,
UART2_PLATFORM_DATA_END(); 
```



### 串口2用做打印，串口1做通信既用TX引脚又用RX引脚。

- 串口1使用重映射引脚的配置方法，将PC00重映射做tx引脚。使用了OUTPUT_CHANNEL3。将PC01重映射做rx引脚。使用了INPUT_CHANNEL3。

  串口2使用固定引脚的配置方法，使用了固定的一组引脚PC9,PC10。只用了PC09做tx脚。

```
UART1_PLATFORM_DATA_BEGIN(uart1_data)                       
	.baudrate = 460800,      
    .port = PORT_REMAP,  
    .tx_pin = IO_PORTC_00,
 	.rx_pin = IO_PORTC_01,            
    .output_channel = OUTPUT_CHANNEL3,   
    .input_channel = INPUT_CHANNEL3,
	.max_continue_recv_cnt = 1024, 
	.idle_sys_clk_cnt = 500000,     
    .clk_src = PLL_48M,          
    .disable_tx_irq=1,              
UART1_PLATFORM_DATA_END(); 
```

```
UART2_PLATFORM_DATA_BEGIN(uart2_data)                       
	.baudrate = 1000000,                                   
    .port = PORTC_9_10,  
    .tx_pin = IO_PORTC_09,  
    .rx_pin = -1,
	.max_continue_recv_cnt = 1024, 
	.idle_sys_clk_cnt = 500000,    
    .clk_src = PLL_48M,                                     
    .flags = UART_DEBUG,
UART2_PLATFORM_DATA_END(); 
```



- 串口1使用固定引脚的配置方法，使用了固定的一组引脚PH06和PH07。PH06做tx脚，PH07做rx脚。

  串口2使用固定引脚的配置方法，使用了固定的一组引脚PC9,PC10。只用了PC09做tx脚。

```
UART1_PLATFORM_DATA_BEGIN(uart1_data)                       
	.baudrate = 460800,      
    .port = PORTH_6_7, 
    .tx_pin = IO_PORTH_06,
 	.rx_pin = IO_PORTH_07,  
	.max_continue_recv_cnt = 1024, 
	.idle_sys_clk_cnt = 500000,     
    .clk_src = PLL_48M,          
    .disable_tx_irq=1,             
UART1_PLATFORM_DATA_END(); 
```

```
UART2_PLATFORM_DATA_BEGIN(uart2_data)                       
	.baudrate = 1000000,                                   
    .port = PORTC_9_10,  
    .tx_pin = IO_PORTC_09,  
    .rx_pin = -1,
	.max_continue_recv_cnt = 1024, 
	.idle_sys_clk_cnt = 500000,    
    .clk_src = PLL_48M,                                     
    .flags = UART_DEBUG,
UART2_PLATFORM_DATA_END(); 
```



- 串口1使用固定引脚的配置方法，使用了固定的一组引脚PH06和PH07。PH06做tx脚，PH07做rx脚。

  串口2使用重映射引脚的配置方法，将PC01重映射做tx引脚。使用了OUTPUT_CHANNEL0。

```
UART1_PLATFORM_DATA_BEGIN(uart1_data)                       
	.baudrate = 460800,      
    .port = PORTH_6_7, 
    .tx_pin = IO_PORTH_06,
 	.rx_pin = IO_PORTH_07,  
	.max_continue_recv_cnt = 1024, 
	.idle_sys_clk_cnt = 500000,     
    .clk_src = PLL_48M,          
    .disable_tx_irq=1,             
UART1_PLATFORM_DATA_END(); 
```

```
UART2_PLATFORM_DATA_BEGIN(uart2_data)                       
	.baudrate = 1000000,                                   
    .port = PORT_REMAP,  
    .tx_pin = IO_PORTC_01,  
    .rx_pin = -1,
    .output_channel = OUTPUT_CHANNEL0,
	.max_continue_recv_cnt = 1024, 
	.idle_sys_clk_cnt = 500000,    
    .clk_src = PLL_48M,                                     
    .flags = UART_DEBUG,
UART2_PLATFORM_DATA_END(); 
```



- 串口1使用重映射引脚的配置方法，将PC06重映射做tx引脚。使用了OUTPUT_CHANNEL3。将PC07重映射做rx引脚。使用了INPUT_CHANNEL3。

  串口2使用重映射引脚的配置方法，将PC01重映射做tx引脚。使用了OUTPUT_CHANNEL0。

```
UART1_PLATFORM_DATA_BEGIN(uart1_data)                       
	.baudrate = 460800,      
    .port = PORT_REMAP,  
    .tx_pin = IO_PORTC_06,
 	.rx_pin = IO_PORTC_07,            
    .output_channel = OUTPUT_CHANNEL3,   
    .input_channel = INPUT_CHANNEL3,
	.max_continue_recv_cnt = 1024, 
	.idle_sys_clk_cnt = 500000,     
    .clk_src = PLL_48M,          
    .disable_tx_irq=1,              
UART1_PLATFORM_DATA_END();
```

```
UART2_PLATFORM_DATA_BEGIN(uart2_data)                       
	.baudrate = 1000000,                                   
    .port = PORT_REMAP,  
    .tx_pin = IO_PORTC_01,  
    .rx_pin = -1,
    .output_channel = OUTPUT_CHANNEL0,
	.max_continue_recv_cnt = 1024, 
	.idle_sys_clk_cnt = 500000,    
    .clk_src = PLL_48M,                                     
    .flags = UART_DEBUG,
UART2_PLATFORM_DATA_END(); 
```



## 一个串口使用情况配置：

### 串口1同时用做打印和通信RX,  打印用TX脚，通信用RX脚

- 串口1使用固定组引脚的配置方法，使用PORTH_6_7中的IO_PORTH_06作为打印, IO_PORTH_07作为通信RX引脚 

```
UART1_PLATFORM_DATA_BEGIN(uart1_data)                       
	.baudrate = 460800,      
    .port = PORTH_6_7, 
    .tx_pin = IO_PORTH_06
 	.rx_pin = IO_PORTH_07,               
	.max_continue_recv_cnt = 1024, 
	.idle_sys_clk_cnt = 500000,     
    .clk_src = PLL_48M,          
    .disable_tx_irq=1,     
    .flags = UART_DEBUG,
UART1_PLATFORM_DATA_END(); 
```



- 串口1使用INPUT_CHANNEL3和OUTPUT_CHANNEL3重映射引脚的配置方法，使用IO_PORTC_00作为打印, IO_PORTC_01作为通信RX引脚 

```
UART1_PLATFORM_DATA_BEGIN(uart1_data)                       
	.baudrate = 460800,      
    .port = PORT_REMAP, 
    .tx_pin = IO_PORTC_00
 	.rx_pin = IO_PORTC_01,   
    .input_channel = INPUT_CHANNEL3,
    .output_channel = OUTPUT_CHANNEL3,
	.max_continue_recv_cnt = 1024, 
	.idle_sys_clk_cnt = 500000,     
    .clk_src = PLL_48M,          
    .disable_tx_irq=1,    
    .flags = UART_DEBUG,
UART1_PLATFORM_DATA_END(); 
```



## 常见问题

> * 使用固定引脚的时候，需注意芯片封装是否有支持的引脚, 具体可以通过对应cpu的[uart.h](../../../../include_lib/driver/cpu/wl82/asm/uart.h)头文件
>
> 
> * 如果有不需要用到的tx_pin或者rx_pin, 需要将其赋值为-1
> 
>   
> 
> * 使用通信功能时,例如使用串口1作为通信串口，需要在使用的board文件加入以下代码注册串口, 只使用到打印功能时不需要注册
>           
>   ```
>   REGISTER_DEVICES(device_table) = {
>   	{"uart1", &uart_dev_ops, (void *)&uart1_data },
>   };
>   ```
> 
>             
> 
> * 使用打印功能时,例如使用串口2作为打印，需要在使用的board文件加入以下代码初始化串口
> 
>   ```
>   void debug_uart_init()
>   {
>      uart_init(&uart2_data);
>   }
>  ```
> 
>  
> 
>* 注意使用到的串口引脚在程序其他地方有没有同时占据使用
> 
>   
> 
> * 使用重映射方法时需要注意OUTPUT_CHANNEL和INPUT_CHANNEL在程序其他地方有没有同时占据使用
> 
>   
>
> * 串口TX或者RX能否配置任意引脚？
>
>   答：能,使用重映射方法配置OUTPUT_CHANNEL和INPUT_CHANNEL
> 
>   
> 
> * 优先推荐使用一个串口的配置方法, 优先推荐使用固定引脚的配置方法，可以节省OUTPUT_CHANNEL和INPUT_CHANNEL供其他功能使用

### 参考文档

> * N/A
