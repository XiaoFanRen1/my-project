## 			代码 跑flash / 跑sdram 的配置方法

> * 如果芯片封装没有SDRAM, 代码只能够跑FLASH的情况:
>
>   1.在SDK主工程cbp文件或者主工程Makefile 加上宏定义 CONFIG_SFC_ENABLE 和CONFIG_NO_SDRAM_ENABLE
>
>   2.在对应cpu的[isd_config](../../../../cpu/wl82/tools/isd_config.ini) 配置 ENABLE_SDRAM=0;
>   
>   
>   
> * 如果芯片封装有SDRAM, 代码放置FLASH运行的情况:
>
>   1.在SDK主工程cbp文件或者主工程Makefile 加上宏定义 CONFIG_SFC_ENABLE 
>
>   2.在SDK主工程cbp文件或者主工程Makefile 去掉宏定义 CONFIG_NO_SDRAM_ENABLE
>
>   3.在对应cpu的[isd_config](../../../../cpu/wl82/tools/isd_config.ini) 配置 ENABLE_SDRAM=1;
>
>   
>
> * 如果芯片封装有SDRAM, 代码放置SDRAM运行的情况:
>
>   1.在SDK主工程cbp文件或者主工程Makefile 去掉宏定义 CONFIG_SFC_ENABLE 和 CONFIG_NO_SDRAM_ENABLE
>
>   3.在对应cpu的[isd_config](../../../../cpu/wl82/tools/isd_config.ini) 配置 ENABLE_SDRAM=1;
>
>   
>
> * 指定库代码放到指定内存运行的方法:
>
>   参考[库代码定位到指定段方法](../库代码定位到指定段方法/readme.md)
>
>   
>
> * 代码跑flash或者代码跑sdram的情况下, 指定函数放到内部ram运行的方法:参考[芯片内存使用](../芯片内存使用)
>
>   在函数定义前面加上 SEC_USED(.volatile_ram_code)
>
> * 芯片封装有SDRAM,代码跑flash下, 指定函数放到sdram运行的方法:
>
>   在函数定义前面加上 SEC_USED(.data)
