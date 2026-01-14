# BLE透传示例工程说明

> 本工程展示了BLE透传的使用示例。

---

## 适用平台

> 本工程适用以下芯片类型：
> 1. AC79系列芯片：AC790N、AC791N
>
> 杰理芯片和评估板的更多信息可在以下地址获取：
> [链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)

## 工程配置说明

> 在SDK选择[demo_ble](../../../demo/demo_ble/board/)主工程文件或者主工程Makefile, 然后添加本事例工程代码

---



### 操作说明：

> BLE透传收发测试：
>
> 1. 在app_config.h文件中使能以下宏：
>
>    ```c
>    #define TCFG_USER_BLE_ENABLE		1
>    #define TRANS_DATA_EN                1
>    ```
>    
> 2. 配置GATT服务和属性：
>
>    - （1）修改profile_data数据表，通过工具[make_gatt_services](../../../../sdk_tools/make_gatt_services)进行自定义修改，将新的profile_data数据表和对应的handle编号存放在le_trans_data.h。详细操作请参考[make_gatt_services工具说明](../../../../sdk_tools/make_gatt_services/make_gatt_services工具说明.pdf)。
>
>    - （2）带有READ属性的handle放到 att_read_callback 函数中，例如 ：
>
>    ```c
>    static uint16_t att_read_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
>    {
>        //......省略
>        switch (handle) {
>        	//带有READ权限的handle
>            case ATT_CHARACTERISTIC_2a00_01_VALUE_HANDLE:
>                //以下为对应属性的操作处理
>                att_value_len = gap_device_name_len;
>                if ((offset >= att_value_len) || (offset + buffer_size) > att_value_len) {
>                    break;
>                }
>                if (buffer) {
>                    memcpy(buffer, &gap_device_name[offset], buffer_size);
>                    att_value_len = buffer_size;
>                    log_info("\n------read gap_name: %s \n", gap_device_name);
>                }
>                break;
>        //......省略
>        }
>    ```
>
>    - （3）将带有WRITE属性的handle放到 att_write_callback 函数中，例如：
>
>    ```c
>    static int att_write_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
>    {
>        //......省略
>        switch (handle) {
>             //带有WRITE权限的handle
>            case ATT_CHARACTERISTIC_ae03_01_VALUE_HANDLE:
>                    //以下为对应属性的操作处理
>                	printf("\n-ae03_rx(%d):", buffer_size);
>                    //收发测试，自动发送收到的数据;for test
>                    if (app_send_user_data_check(buffer_size)) {
>                        app_send_user_data(ATT_CHARACTERISTIC_ae05_01_VALUE_HANDLE, buffer, buffer_size, ATT_OP_AUTO_READ_CCC);
>                    }
>                    break;
>        //......省略
>     }
>    ```
>
>    - （4）将带有NOTIFY或者INDICATION属性的 handle 通过传参到发送数据的函数中；对于带有NOTIFY或者INDICATION属性的characteristic，主机端需要使能CCCD才能接收notify和indicate命令。
>
>    ```c
>    app_send_user_data(ATT_CHARACTERISTIC_ae05_01_VALUE_HANDLE, buffer, buffer_size, ATT_OP_AUTO_READ_CCC);
>    ```
>
> 3. 编译工程，烧录镜像，复位启动；
>
> 4. 在手机或其他终端打开BLE助手；连接成功后，在通过BLE助手进行收发测试：
>
>    - （1）蓝牙连接成功后，点击“蓝牙服务”查看当前蓝牙设备的service列表，如图1所示；
>
>    - （2）点击第二项service，点击UUID为ae03的characteristic对应的箭头，表示发送该characteristic对应的数据，如图2所示；
>
>    - （3）点击UUID为ae05的characteristic对应的按钮，表示使能该characteristic的CCD，即接收其indicate命令，如图2所示；
>
>    - （4）点击“实时日志”，在发送栏编辑数据后点击“发送”，通过日志可看到BLE助手发送数据成功，并且成功接收到从机发送的数据，如图3所示。
>
> JIELI SDK的编译、烧写等操作方式的说明可在以下文档获取：[文档](../../../../doc/stuff/usb updater.pdf)
>
> | <img src=".\BLE Utility_1.jpg" alt="BLE Utility_1" style="zoom:80%;" /> | <img src=".\BLE Utility_2.jpg" alt="BLE Utility_2" style="zoom:80%;" /> | <img src=".\BLE Utility_3.jpg" alt="BLE Utility_3" style="zoom:80%;" /> |
> | :----------------------------------------------------------: | :----------------------------------------------------------: | :----------------------------------------------------------: |
> |                             图1                              |                             图2                              |                             图3                              |

### 接口说明

> 1. BLE透传代码文件：le_trans_data.c
>
> 2. BLE透传接口说明：
>
>     ```c
>     - cbk_packet_handler       ：协议栈事件回调处理；
>     - att_read_callback        ：ATT读事件处理；
>     - att_write_callback       ：ATT写事件处理；
>     - app_send_user_data_check ：检查是否可以往协议栈发送数据；
>     - app_send_user_data       ：NOTIFY和INDICATE发送接口，发送前调接口app_send_user_data_check检查；
>     - can_send_now_wakeup      ：发送完成回调，表示可以继续往协议栈发数，用来触发继续发数
>     ```

### 代码流程

> SPP收发测试流程：
>
> 1. app_main.c：在app_main函数中调用bt_ble_module_init函数初始化蓝牙模块；
> 2. ble.c：在蓝牙的状态回调函数bt_connction_status_event_handler的BT_STATUS_INIT_OK事件下调用bt_ble_init对BLE进行配置；
> 3. spp_trans_data.c：在ATT写事件处理函数att_write_callback中，在ATT_CHARACTERISTIC_ae03_01_VALUE_HANDLE case下，调用app_send_user_data将接收到的数据通过ATT_CHARACTERISTIC_ae05_01_VALUE_HANDLE发送给与其连接的设备，从而完成一次回环测试。
---

## 常见问题

> * N/A

## 参考文档

> * N/A