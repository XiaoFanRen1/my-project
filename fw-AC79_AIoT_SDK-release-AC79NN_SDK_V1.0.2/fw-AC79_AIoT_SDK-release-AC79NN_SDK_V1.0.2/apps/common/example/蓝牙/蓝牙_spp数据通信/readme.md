# 蓝牙SPP数据通信示例工程说明

> 本工程展示了蓝牙SPP数据通信接口使用示例。

---

## 适用平台

> 本工程适用以下芯片类型：
> 1. AC79系列芯片：AC790N、AC791N
>
> 杰理芯片和评估板的更多信息可在以下地址获取：
> [链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)

## 工程配置说明

> 在SDK选择[demo_edr](../../../demo/demo_edr/board/)主工程文件或者主工程Makefile, 然后添加本事例工程代码

---



### 操作说明：

> SPP收发测试：
>
> 1. 在app_config.h文件中使能以下宏：
>
>    ```c
>    #define CONFIG_POWER_ON_ENABLE_BT      1
>    #define TCFG_USER_BT_CLASSIC_ENABLE    1
>    #define SPP_TRANS_DATA_EN              1
>    ```
>
> 2. 在蓝牙初始化完成之后，初始化SPP模块；即在蓝牙的状态回调函数的BT_STATUS_INIT_OK事件下初始化SPP；
>
>    ```c
>    //所在文件bt_music.c
>    static int bt_connction_status_event_handler(struct bt_event *bt)
>    {
>        //......省略
>        switch (bt->event) {
>        case BT_STATUS_INIT_OK:
>            bt_connection_enable();
>            void transport_spp_init(void);
>            transport_spp_init();	//SPP模块初始化
>            break;
>     	//......省略
>     }
>    ```
>    
> 3. 编译工程，烧录镜像，复位启动；
>
> 4. 在手机或其他终端打开蓝牙SPP助手；连接成功后，在通过SPP助手进行收发测试，如图1所示；
>
>    | <img src=".\BlueSPP.jpg" alt="BlueSPP" style="zoom: 25%;" /> |
>    | :----------------------------------------------------------: |
>    |                             图1                              |
>
> JIELI SDK的编译、烧写等操作方式的说明可在以下文档获取：[文档](../../../../doc/stuff/usb updater.pdf)

### 接口说明

> 1. SPP代码文件：spp_trans_data.c
>
> 2. SPP接口说明：
>
>     ```c
>     - transport_spp_init            ：SPP模块初始化，一般在蓝牙初始化完成之后调用；
>     - transport_spp_state_cbk       ：SPP状态回调函数，处理SPP 连接和断开事件；
>     - transport_spp_send_data_check ：检查是否可以往协议栈发送数据，在SPP发送数据前调用；
>     - transport_spp_send_data       ：SPP数据发送接口，使用前先调用transport_spp_send_data_check接口；
>     - transport_spp_send_wakeup     ：SPP发送完成回调，表示可以继续往协议栈发数，用来触发继续发数；
>     - transport_spp_recieve_cbk     ：SPP接收数据接口，可在里面添加自定义数据处理操作；
>     ```

### 代码流程

> SPP收发测试流程：
>
> 1. app_main.c：在app_main函数中调用bt_ble_module_init函数初始化蓝牙模块；
> 2. bt_music.c：在蓝牙的状态回调函数bt_connction_status_event_handler的BT_STATUS_INIT_OK事件下调用transport_spp_init初始化SPP；
> 3. spp_trans_data.c：在SPP的数据接收回调transport_spp_recieve_cbk函数中，调用transport_spp_send_data函数将接收到的数据发送给与其连接的设备，从而完成一次回环测试。
---

## 常见问题

> * N/A

## 参考文档

> * N/A