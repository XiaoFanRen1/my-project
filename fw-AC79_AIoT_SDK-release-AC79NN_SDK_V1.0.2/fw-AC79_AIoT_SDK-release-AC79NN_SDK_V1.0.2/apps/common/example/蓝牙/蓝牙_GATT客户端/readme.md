# GATT客户端示例工程说明

> 本工程展示了一个低功耗蓝牙 (BLE) 通用属性 (GATT) 客户端的工作流程。该客户端扫描附近的GATT服务端，与指定的设备建立连接，然后搜索可用特征并订阅已知特征，从而可以先GATT服务端发送数据，并接收它的通知或指示。

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

> 1. 将ble.c替换至工程目录中的同名文件，并使能app_config.h中以下宏；
>
> ```c
> #define TCFG_USER_BLE_ENABLE                      1
> #define BT_NET_CENTRAL_EN                         1
> ```
>
> 2. 在ble.c中，可修改在match_dev01指定需要连接设备的信息，和在search_uuid_table指定需要搜索的service和characteristic；
>
> 3. 准备两块开发板，在开发板A中下载GATT客户端固件，在开发板B中下载[GATT服务端固件](../蓝牙_GATT服务端)。
>
> 4. 上电后，GATT客户端将进入搜索阶段，等待其与GATT服务端建立连接。具体的测试过程如下：
>
>    （1）搜索阶段：烧录完成后，GATT客户端将发送搜索，当搜索到指定设备时，GATT客户端将发起连接请求。
>
>    （2）连接阶段：成功建立连接后，GATT客户端将获取指定的service和characteristic，对于带有indicate或notify属性的characteristic使能其CCD，从而可以接收到GATT服务端的indicate和notify数据；
>
>    （3）收发测试：获取完成后，GATT客户端将定时向GATT服务端write数据，GATT服务端接收到数据后通过notify将数据转发到GATT客户端，从而完成一次回环测试。
>
> | <img src=".\搜索阶段.jpg" alt="搜索阶段" style="zoom:80%;" /> | <img src=".\连接阶段.jpg" alt="连接阶段" style="zoom:50%;" /> |
> | :----------------------------------------------------------: | :----------------------------------------------------------: |
> |                        （1）搜索阶段                         |                        （2）连接阶段                         |

### 代码流程

> 1.蓝牙模块初始化
>
> - 蓝牙模块初始化的目的是对蓝牙进行自定义参数的配置，并初始化蓝牙协议栈。
>
>
> ```c
> void bt_ble_module_init(void)
> {
>   lmp_set_sniff_disable();
>   //......
>   bt_max_pwr_set(3, 2, 2, 2);	//0-5
>   //......
>   lib_make_ble_address(tmp_ble_addr, (u8 *)bt_get_mac_addr());
>   le_controller_set_mac((void *)tmp_ble_addr);
>   //......
>   ble_client_config_init();
>   btstack_init();
> }
> ```
>
> - 首先失能sniff模式；通过bt_max_pwr_set设置最大功率限定值，普通工作模式下最大功率等级范围为[0, 5]。
>
>
> ```c
> /*
>  *u8 pwr     ：EDR的最大功率
>  *u8 pg_pwr  ：page过程中的最大功率
>  *u8 iq_pwr  ：inquiry过程中的最大功率
>  *u8 ble_pwr ：BLE的最大功率
>  */
> void bt_max_pwr_set(u8 pwr, u8 pg_pwr, u8 iq_pwr, u8 ble_pwr)
> ```
>
> - 通过lib_make_ble_address生成BLE地址，再调用le_controller_set_mac将其设置为当前BLE设备的地址。
>
> - 在match_dev01指定需要连接设备的信息，在search_uuid_table指定需要搜索的service和characteristic。ble_report_data_deal函数中处理GATT服务端发送的indicate和notify数据；ble_report_data_deal函数中处理GATT客户端事件。配置完成后，通过ble_client_config_init初始化客户端配置。
>
> | GATT客户端事件                    | 说明                |
> | --------------------------------- | ------------------- |
> | CLI_EVENT_MATCH_DEV               | 搜索到匹配的设备    |
> | CLI_EVENT_CONNECTED               | 设备连接成功        |
> | CLI_EVENT_DISCONNECT              | 设备连接断开        |
> | CLI_EVENT_MATCH_UUID              | 搜索到匹配的UUID    |
> | CLI_EVENT_SEARCH_PROFILE_COMPLETE | 搜索profile服务结束 |
>
> ```c
> static const client_conn_cfg_t client_conn_config = {
>     .security_en = 0,
>     .match_dev_cfg[0] = &match_dev01,
>     .search_uuid_cnt = (sizeof(search_uuid_table) / sizeof(target_uuid_t)),
>     .search_uuid_table = search_uuid_table,
>     .report_data_callback = ble_report_data_deal,
>     .event_callback = client_event_callback,
> };
> 
> void ble_client_config_init(void)
> {
>     ble_client_api = ble_get_client_operation_table();
>     ble_client_api->init_config(0, (void *)&client_conn_config);
> }
> ```
>
> - 最后调用btstack_init初始化蓝牙协议栈。
>
> 2.GATT客户端初始化
>
> - 蓝牙协议栈初始化完成后，应用层的蓝牙状态回调函数中将收到BT_STATUS_INIT_OK事件，在此事件下调用bt_master_ble_init函数初始化GATT客户端。
>
> ```c
> int bt_connction_status_event_handler(struct bt_event *bt)
> {
>    //...... 
>     case BT_STATUS_INIT_OK:
>        bt_master_ble_init();
> 	//......    
> }
> ```
>
> - 在bt_master_ble_init中，gatt_client_init初始化GATT，注册GATT事件回调函数，从而可以在应用层对相关事件进行处理；conn_pair_vm_do函数用于初始化存放连接记录的VM区；device_bonding_init函数用于清空存储在VM中的历史连接记录。
>
> ```c
> void bt_master_ble_init(void)
> {
>     //......
>     gatt_client_init();
>     gatt_client_register_packet_handler(cbk_packet_handler);
>     //......
>     conn_pair_vm_do(&conn_pair_info, 0);
>     device_bonding_init();
>     ble_module_enable(1);
>     //......
> }
> 
> 
> ```
>
> - 在ble_module_enable函数中，ble_stack_gatt_role设置GATT为客户端模式，并通过hci_event_callback_set和le_l2cap_register_packet_handler分别注册HCI和L2CAP回调函数，从而可以在应用层上处理协议栈返回的数据。
>
> ```c
> void ble_module_enable(u8 en)
> {
>    //......
>     ble_stack_gatt_role(1);
>    hci_event_callback_set(&cbk_packet_handler);
>     le_l2cap_register_packet_handler(&cbk_packet_handler);
>     //......
> }
> ```
>
> 3.启动搜索
>
> - 完成以上初始化后，开始进入搜索阶段，发现周围可连接的BLE设备。首先，通过scanning_setup_init初始化搜索参数；搜索参数在le_net_central.c中的全局变量中进行配置，如下：
>
> | 搜索参数        | 说明                        |
> | :-------------- | --------------------------- |
> | SET_SCAN_TYPE   | 搜索类型                    |
> | SET_SCAN_TYPE   | 搜索周期大小(unit:0.625ms)  |
> | SET_SCAN_WINDOW | 搜索窗口大小 (unit:0.625ms) |
>
> - 调用ble_op_scan_enable2启动搜索。
>
> ```c
> int bt_ble_scan_enable(void *priv, u32 en)
> {
>    //......
>     scanning_setup_init();
>    //......
>     ble_op_scan_enable2(en, 0);
> 	//......
> }
> ```
>
> 4.获取搜索结果
>
> - 当GATT客户端发现周围的可连接设备时，会将其发现的设备信息推送到协议栈数据处理回调函数cbk_packet_handle，再通过client_report_adv_data处理搜索结果。
>
> ```c
> void cbk_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
> {
>    //......
>     case GAP_EVENT_ADVERTISING_REPORT:
>            client_report_adv_data((void *)&packet[2], packet[1]);
>             break;
>     //......
> }
> ```
>
> 5.连接GATT服务端
>
> - 在client_report_adv_data函数中，将搜索到的设备信息与前面配置中设定的设备信息进行比较；当发现指定的设备时，停止搜索，调用client_create_connect_api向GATT服务端发起连接。
>
> ```c
> static void client_report_adv_data(adv_report_t *report_pt, u16 len)
> {
>    bool find_remoter;
>     find_remoter = resolve_adv_report(report_pt->address, report_pt->length, report_pt->data, report_pt->rssi);
>    //......
>     if (find_remoter) {
>         //......
>         bt_ble_scan_enable(0, 0);
>         client_create_connect_api(report_pt->address, report_pt->address_type, 0);
>     }
> }
> ```
>
> - 连接参数在le_net_central.c中的全局变量中进行配置，如下：
>
> | 连接参数          | 说明                    |
> | ----------------- | ----------------------- |
> | SET_CONN_INTERVAL | 连接周期(unit:1.25ms)   |
> | SET_CONN_LATENCY  | (unit:conn_interval)    |
> | SET_CONN_TIMEOUT  | 连接超时时间(unit:10ms) |
>
> - 连接完成后，协议栈数据回调函数cbk_packet_handle将会收到连接结果。若连接失败，则设置当前状态为“未连接”；若连接成功，保存当前的连接句柄con_handle。如果该设备在断开连接后需要回连，则调用conn_pair_vm_do将该设备的信息写入VM区。
>
> ```c
> //......
> case HCI_SUBEVENT_LE_CONNECTION_COMPLETE:
>    if (packet[3]) {
>         set_ble_work_state(BLE_ST_DISCONN);
>        break;
>     }
>     con_handle = hci_subevent_le_connection_complete_get_connection_handle(packet);
>     //......
>     client_profile_start(con_handle);
> 	//......
>     conn_pair_vm_do(&conn_pair_info, 1);
> 	//......
>     break;
> //......
> ```
>
> 6.搜索服务
>
> - 成功建立连接后，调用client_profile_start搜索GATT服务端的服务。首先，调用ble_op_att_send_init初始化ATT_RAM和MTU，其大小可在le_net_central.c中进行配置，搜索服务的SEARCH_PROFILE_BUFSIZE也可根据需要配置。然后，client_search_profile_start开始搜索服务。
>
> ```c
> void client_profile_start(u16 con_handle)
> {
>    ble_op_att_send_init(con_handle, att_ram_buffer, ATT_RAM_BUFSIZE, ATT_LOCAL_MTU_SIZE);
>     set_ble_work_state(BLE_ST_CONNECT);
>    if (0 == client_config->security_en) {
>         client_search_profile_start();
>     }
> }
> ```
>
> 7.获取服务
>
> 开始搜索后，协议栈内部将调用user_client_report_search_result函数处理搜索结果。
>
> ```c
> void user_client_report_search_result(search_result_t *result_info)
> {
>    //......
>     check_target_uuid_match(result_info);
> }
> ```
>
> check_target_uuid_match函数将搜索结果与ble.c设定的search_uuid_table进行对比，当匹配到指定的service或characteristic后，将记录其对应的uuid和handle，从而可以获取或访问GATT服务端。
>
> ```c
> void check_target_uuid_match(search_result_t *result_info)
> {
>    //......
>     log_info("match one uuid\n");
> 	//......
>     opt_handle_t *opt_get = &opt_handle_table[opt_handle_used_cnt++];
>     opt_get->value_handle = result_info->characteristic.value_handle;
>     opt_get->search_uuid = t_uuid;
> 	//......
>     client_event_report(CLI_EVENT_MATCH_UUID, opt_get, sizeof(opt_handle_t));
> }
> ```
>
> 8.使能INDICATE和NOTIFY
>
> INDICATE和NOTIFY属性的特征值，会比读、写属性的特征值多一个CCC（Client Characteristic Configuration）。为了使GATT服务端调用indicate或notify函数时能正常发送数据，GATT客户端必须打开CCC。在完成搜索服务后，内部协议栈会通过do_operate_search_handle函数打开CCC，GATT客户端能成功接收到GATT服务端indicate和notify的数据。
>
> ```c
> void user_client_report_search_result(search_result_t *result_info)
> {
>    if (result_info == (void *) - 1) {
>         do_operate_search_handle();
>        client_event_report(CLI_EVENT_SEARCH_PROFILE_COMPLETE, 0, 0);
>         return;
>     }
>     //......
> }
> ```
>
> 协议栈将接收到的indicate和notify数据，传递到user_client_report_data_callback函数进行处理。
>
> ```c
> void user_client_report_data_callback(att_data_report_t *report_data)
> {
>    //......
>     const target_uuid_t *search_uuid = get_match_handle_target(report_data->value_handle);
>    if (client_config->report_data_callback) {
>         client_config->report_data_callback(report_data, search_uuid);
>         return;
>     }
>     switch (report_data->packet_type) {
>     case GATT_EVENT_NOTIFICATION://notify
>     case GATT_EVENT_INDICATION:  //indicate
>     case GATT_EVENT_CHARACTERISTIC_VALUE_QUERY_RESULT:     //read
>     case GATT_EVENT_LONG_CHARACTERISTIC_VALUE_QUERY_RESULT://read long
>         break;
>     default:
>         break;
>     }
> }
> ```
### 接口说明

> - le_net_central.c中封装了GATT_客户端的操作集，可通过ble_get_client_operation_table获取操作集句柄。
>
> ```C
> struct ble_client_operation_t *ble_get_client_operation_table(void)
> {
>     return &client_operation;
> }
> ```
>
> - 具体接口说明如下：
>
> | 接口                                 | 备注说明                                                     |
> | ------------------------------------ | ------------------------------------------------------------ |
> | bt_ble_init                          | 模块初始化                                                   |
> | bt_ble_exit                          | 模块初退出                                                   |
> | ble_module_enable                    | 模块开关使能                                                 |
> | client_disconnect                    | 断开连接                                                     |
> | cbk_packet_handler                   | 协议栈事件处理                                               |
> | cbk_sm_packet_handler                | 配对加密事件处理                                             |
> | scanning_setup_init                  | 扫描参数配置                                                 |
> | client_report_adv_data               | 扫描到的广播信息处理                                         |
> | bt_ble_scan_enable                   | 扫描开关                                                     |
> | client_create_connection             | 创建连接监听                                                 |
> | client_create_connection_cannel      | 取消连接监听                                                 |
> | client_search_profile_start          | 启动搜索从机的GATT服务                                       |
> | user_client_report_search_result     | 回调搜索GATT服务的结果                                       |
> | check_target_uuid_match              | 检查搜索到的GATT服务是否匹配目标                             |
> | do_operate_search_handle             | 搜索完成后，处理搜索到的目标handle。例如：对handle的读操作，对handle写使能通知功能等 |
> | user_client_report_data_callback     | 接收到从机的数据，例如notify和indicate数据                   |
> | app_send_user_data                   | 发送数据接口                                                 |
> | app_send_user_data_check             | 检查是否可以往协议栈发送数据                                 |
> | l2cap_connection_update_request_just | 是否接受从机连接参数的调整求，接受后协议栈会自动更新参数     |
> | client_send_conn_param_update        | 主动更新连接参数调整                                         |
> | connection_update_complete_success   | 连接参数调整成功                                             |
> | get_buffer_vaild_len                 | 获取发送buf可写如长度                                        |
> | can_send_now_wakeup                  | 协议栈发送完成回调，用来触发继续发数                         |
> | client_write_send                    | 向从机写数据，需要等待从机回复应答                           |
> | client_write_without_respond_send    | 向从机写数据，不需要从回复应答                               |
> | client_read_value_send               | 向从机发起读数据，只能读到<=MTU的长度数据                    |
> | client_read_long_value_send          | 向从机发起读数据，一次读完所有数据                           |
>
> 
---

## 常见问题

> * N/A

## 参考文档

> * N/A