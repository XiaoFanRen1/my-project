# BLE测速示例工程说明

> 本工程展示了BLE从机的上行速率测试方法。

---

## 适用平台

> 本工程适用以下芯片类型：
>
> 1. AC79系列芯片：AC790N、AC791N

> 杰理芯片和评估板的更多信息可在以下地址获取：[链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)

## 工程配置说明

> 在SDK选择[demo_ble](../../../demo/demo_ble/board/)主工程文件或者主工程Makefile, 然后添加本事例工程代码。

---

### 操作说明：

> 1. 在手机端安装BLE测速工具[BleSpeedTest.apk](../../../../sdk_tools)；
> 2. 在app_config.h文件中，使能TCFG_USER_BLE_ENABLE、TRANS_DATA_EN宏；
> 3. 在 le_trans_data.c 文件中，使能宏 TEST_SEND_DATA_RATE；
> 4. 编译工程，烧录镜像，复位启动；
> 5. 在手机端运行 BLE 测速工具 APP，在 Home 界面点击“开始扫描”， 在扫描列表中点击需要测速的 BLE 设备，进入 Dashboard 界面， MTU 值可更改，如图 1、2所示； 
> 6. 在 Dashboard 界面，点击第二项服务，并 CHANGE 其两项服务属性 值为“Enable”，随后勾选“计算速率”，开始测速，如图 3、4 所示。

> JIELI SDK的编译、烧写等操作方式的说明可在以下文档获取：[文档](../../../../doc/stuff/usb updater.pdf)
>
> | <img src=".\BleSpeedTest_1.jpg" alt="BleSpeedTest_1" style="zoom: 80%;" /> | <img src=".\BleSpeedTest_2.jpg" alt="BleSpeedTest_2" style="zoom: 80%;" /> |
> | :----------------------------------------------------------: | :----------------------------------------------------------: |
> |                             图1                              |                             图2                              |
> | <img src=".\BleSpeedTest_3.jpg" alt="BleSpeedTest_3" style="zoom:80%;" /> | <img src=".\BleSpeedTest_4.jpg" alt="BleSpeedTest_4" style="zoom:80%;" /> |
> |                             图3                              |                             图4                              |

## 常见问题

> * N/A

## 参考文档

> * N/A
