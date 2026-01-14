# 中断使用示例工程说明

> 本工程展示了系统中断使用示例:
>
> * 中断的头文件位于对应cpu目录下的[hwi.h](../../../../include_lib/driver/cpu/wl80/asm/hwi.h)
>
> * 通常安装中断使用request_irq,优先级为1-7, 越大优先级越高,cpu_id为安装到哪一个核去响应中断
>
> * SDK库内部已经安装好中断, 如果用户需要修改中断的优先级和安装核, 可以在app_main.c的irq_info_table强行修改对应的中断号,例如
>
>   ```
>   /*中断列表 */
>   const struct irq_info irq_info_table[] = {
>       { IRQ_TIMER4_IDX,      2,   1    }, //此中断强制配置为优先级为2,并且安装到cpu1
>   ```
>
> * 当用户想统一某些中断注册到核1,某些中断注册到核0,可以在app_main.c的irq_info_table配置,例如
>
>   ```
>   const struct irq_info irq_info_table[] = {
>       { IRQ_SOFT5_IDX,      7,   0    }, //此中断强制注册到cpu0
>       { IRQ_SOFT4_IDX,      7,   1    }, //此中断强制注册到cpu1
>       { -2,     			-2,   -2   },//如果加入了该行, 那么只有该行之前的中断注册到对应核, 其他所有中断强制注册到CPU0
>       
>   ```

---

## 适用平台

> 本工程适用以下芯片类型：
> 1. AC79系列芯片：AC790N、AC791N
>
> 杰理芯片和评估板的更多信息可在以下地址获取：[链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)

## 工程配置说明

> 在SDK选择[demo_hello](../../../../apps/demo_hello/board)或者其他demo的主工程文件或者主工程Makefile, 然后添加本事例工程代码

---



### 操作说明：

> 2. 编译工程，烧录镜像，复位启动
> 3. 系统启动后，可以通过串口软件看到示例的打印信息
>
> JIELI SDK的编译、烧写等操作方式的说明可在以下文档获取：[文档](../../../../doc/stuff/usb updater.pdf)

### 代码流程

> * N/A
---

## 常见问题

> * N/A

## 参考文档

> * N/A