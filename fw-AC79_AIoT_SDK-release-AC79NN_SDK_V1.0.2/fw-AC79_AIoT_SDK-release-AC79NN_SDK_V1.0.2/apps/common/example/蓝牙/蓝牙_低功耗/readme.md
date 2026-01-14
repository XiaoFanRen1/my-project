# 蓝牙低功耗说明

> 本文档说明如何配置蓝牙低功耗。

---

## 适用平台

> 本工程适用以下芯片类型：
> 1. AC79系列芯片：AC790N、AC791N
>
> 杰理芯片和评估板的更多信息可在以下地址获取：
> [链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)

## 工程配置说明

> 在SDK选择[demo_edr](../../../demo/demo_edr/board/)主工程文件或者主工程Makefile

---



### 操作说明：

> 1. 在app_config.h文件中修改以下宏：
>
>    ```c
>     #define TCFG_LOWPOWER_LOWPOWER_SEL			RF_SLEEP_EN
>    ```
>    
> 2. 打开该宏
> 
>   ```c
>    #define CONFIG_LOW_POWER_ENABLE              //低功耗开关
>   ```
>   
> 3. JIELI SDK的编译、烧写等操作方式的说明可在以下文档获取：[文档](../../../../doc/stuff/usb updater.pdf)
> 
> 

---

## 常见问题

> * N/A

## 参考文档

> * N/A