# gsensor使用示例工程说明

> 本工程展示了gsensor使用示例:
>
> 1. gsensor相关配置方法
> 2. 挂载da380传感器撞击测试

---

## 适用平台

> 本工程适用以下芯片类型：
>
> 1. AC79系列芯片：AC790N、AC791N
>
> 杰理芯片和评估板的更多信息可在以下地址获取：[链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)

## 依赖文件

> 本工程依赖一下文件：
>
> 1. iic.c
> 2. software_iic.c 
> 3. gSensor_manage.c
> 4. da380.c

## 工程配置说明

> 1. 在SDK选择[demo_hello](../../../../apps/demo/demo_hello/board)或者其他demo的主工程文件或者主工程Makefile, 添加 iic.c 、 software_iic.c 文件或者直接包含iic目录下的Makefile：
>
>    ```c
>    include $(ROOT)/apps/common/iic/Makefile
>    ```
>
>    并在选择的主工程中添加gSensor_manage.c、da380.c文件或者直接包含gensor目录下的Makefile：
>
>    ```c
>    include $(ROOT)/apps/common/gsensor/Makefile
>    ```
>
> 2. board.c 添加iic和gsensor配置参数
>
>    ```c
>    //1.添加头文件
>    #include "gSensor_manage.h"
>    
>    //2.添加软件IIC配置信息
>    SW_IIC_PLATFORM_DATA_BEGIN(sw_iic0_data)
>    	.clk_pin = IO_PORTH_00,//clk
>    	.dat_pin = IO_PORTH_01,//sdat
>    	.sw_iic_delay = 50,//clk时钟周期（系统的nop时间个数）
>    SW_IIC_PLATFORM_DATA_END()
>        
>    //3.添加gensor配置信息 
>    const struct gsensor_platform_data gsensor_data = {
>    	    .iic = "iic0",
>    };
>    
>    //4.设备列表添加iic设备和gsensor设备
>    { "iic0",  &iic_dev_ops, (void *)&sw_iic0_data },
>    {"gsensor", &gsensor_dev_ops, (void *)&gsensor_data},
>    ```
>
> 3. 在app_main.c中的添加测试代码
>
>    ```c
>    //1.添加头文件
>    #include "gSensor_manage.h"
>    
>    //2.添加gsensor灵敏度设置函数   
>    static int gra_set_function(u32 parm)
>    {
>    //#ifdef CONFIG_GSENSOR_ENABLE
>        switch (parm) {
>    	case GRA_SEN_OFF:
>    		parm = G_SENSOR_CLOSE;
>    		break;
>    	case GRA_SEN_LO:
>    		parm = G_SENSOR_LOW;
>    		break;
>    	case GRA_SEN_MD:
>    		parm = G_SENSOR_MEDIUM;
>        	break;
>        case GRA_SEN_HI:
>    		parm = G_SENSOR_HIGH;																break;
>    	}
>        set_gse_sensity(parm);
>    //#endif
>    	return 0;
>    }
>    
>    //3.在app_main()中调用gsensor灵敏度设置函数
>    gra_set_function(GRA_SEN_MD);
>    ```
>

## 操作说明

> 1. 编译工程，烧录镜像，复位启动
>
> 2. 系统启动后，可以通过串口软件看到gsensor挂载da380的情况，返回id号13即为挂载成功
>
>    <img src=".\1.png" alt="image-20210819134036" style="zoom:60%;" >
>
> 3. 若da380挂载成功，撞击带有da380的开发板，会产生如下打印信息
>
>    <img src=".\2.png" alt="image-20210819134036" style="zoom:60%;" >
>
> JIELI SDK的编译、烧写等操作方式的说明可在以下文档获取：[文档](../../../../doc/stuff/usb updater.pdf)

## 常见问题

> * N/A

## 参考文档

> * N/A

