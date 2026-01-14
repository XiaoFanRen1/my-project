﻿## 			复位IO维持电平使用说明

>  **复位维持某IO电平使用说明：**
> 1、复位前设置IO状态
>
> 注意有些封装多IO内绑在一起，则需要设置另外IO为高阻模式，如PB8高阻：
> 	gpio_set_die(IO_PORTB_08, 0);
>     gpio_set_pull_up(IO_PORTB_08, 0);
>     gpio_set_pull_down(IO_PORTB_08, 0);
>     gpio_direction_input(IO_PORTB_08);
> 	
> 硬件没外置上拉电阻则使用内部上下拉电阻
> 硬件外部上拉拉，则需要关闭内部的上下拉
> 硬件没有上下拉，则使用内部上下拉：根据输出1开上拉关下拉，输出0关上拉开下拉
> 如：使用内部上拉电阻，输出1,
>     gpio_latch_en(IO_PORTC_00,0);//先解除IO
>     gpio_direction_output(IO_PORTC_00, 1);//设置输出
> 	gpio_set_pull_up(IO_PORTC_00, 1)//开上拉，0则关上拉
>     gpio_set_pull_down(IO_PORTC_00, 0);//关下拉，1则开上拉
>     gpio_latch_en(IO_PORTC_00,1);//锁住IO
> 	
> 2、使用system_reset()函数复位函数才能维持IO
>
> 3、uboot启动配置
> isd_tools_sdram.cfg配置uboot启动的IO状态
> 如port_output=PC00_0;//PC00输出0
> 如port_output=PC00_1;//PC00输出1
>
> 或者isd_config.ini文件配置uboot启动的IO状态
> PORT_OUTPUT=PC00_0;//PC00输出0
> PORT_OUTPUT=PC00_1;//PC00输出1

