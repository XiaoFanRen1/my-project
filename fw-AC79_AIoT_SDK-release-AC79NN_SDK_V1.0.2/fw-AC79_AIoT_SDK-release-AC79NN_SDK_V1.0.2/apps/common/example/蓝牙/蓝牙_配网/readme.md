# 蓝牙配网方法说明

> 本工程展示了通过蓝牙对WiFi进行网络配置的方法。

---

## 适用平台

> 本工程适用以下芯片类型：
>
> 1. AC79系列芯片：AC790N、AC791N

> 杰理芯片和评估板的更多信息可在以下地址获取：[链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)

## 工程配置说明

> 在SDK选择[demo_ble](../../../demo/demo_ble/board/)主工程文件或者主工程Makefile, 然后添加本事例工程代码

---



### 操作说明：

> 1.在app_config.h文件中，使能以下宏：
>
> ```c
> #define BT_NET_CFG_EN		1
> ```
>
> 2.在le_net_cfg.c文件中，可更改BLE设备名称：
>
> ```c
> static char gap_device_name[BT_NAME_LEN_MAX] = "wl80_ble_test";
> ```
>
> 3.打开手机的蓝牙开关，通过微信扫描二维码进入杰理小程序，二维码如图1所示；
>
> 4.进入小程序后，在界面中填写新的WiFi名称、WiFi密码。点击“搜索蓝牙设备”，等待蓝牙设备连接成功，如图2所示；
>
> 5.点击“蓝牙设备配网”，设备即可获取到WiFi名称、WiFi密码数据。

> JIELI SDK的编译、烧写等操作方式的说明可在以下文档获取：[文档](../../../../doc/stuff/usb updater.pdf)
>
> | ![1](.\1.png) | <img src=".\2.png" alt="2" style="zoom: 80%;" /> |
> | :-----------: | :----------------------------------------------: |
> |      图1      |                       图2                        |

### 代码流程

> （1）app_main.c：
>
> - bt_ble_module_init：初始化BLE，进入广播状态；
>
> （2）le_net_cfg.c：
>
> - att_write_callback：响应BLE设备的write事件；在ATT_CHARACTERISTIC_ae81_01_VALUE_HANDLE下接收小程序发送的WiFi名称、WiFi密码数据；
>
> - check_net_info_if_recv_complete：对WiFi名称、WiFi密码数据进行处理
>
>   P.S.：可通过ble_cfg_net_result_notify函数向小程序返回当前的配网状态。

## 常见问题

> * N/A

## 参考文档

> * N/A
