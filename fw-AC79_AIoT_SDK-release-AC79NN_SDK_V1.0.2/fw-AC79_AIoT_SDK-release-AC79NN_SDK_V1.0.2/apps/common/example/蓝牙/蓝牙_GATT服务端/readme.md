# GATT服务端示例工程说明

> 本工程展示了一个低功耗蓝牙 (BLE) 通用属性 (GATT) 服务端的工作流程。该服务端围绕profile数据和一系列事件而设计，通过响应、处理这些事件完成一系列的配置步骤，如初始化广播参数、更新连接参数等；在与GATT客户端建立连接后，可以通过indicate和notify向GATT客户端发送数据，并响应GATT客户端发送的read、write事件，从而实现与GATT客户端的数据交互。

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

> 1. 替换le_trans_data.c和le_trans_data.h文件至工程中的同名文件，并在app_config.h中初始化以下宏：
>
>    ```c
>    #define TCFG_USER_BLE_ENABLE                      1
>    #define TRANS_DATA_EN                             1
>    ```
>
> 2. profile配置。本工程已经配置好的profile数据表，可直接使用，无需再配置。profile以数据表的方式在le_trans_data.h文件中定义。通过工具[make_gatt_services](../../../../sdk_tools/make_gatt_services)生成自定义的profile数据表，将新的profile_data数据表和对应的handle编号存放在le_trans_data.h。详细操作请参考[make_gatt_services工具说明](../../../../sdk_tools/make_gatt_services/make_gatt_services工具说明.pdf)。
>
> 3. 准备两块开发板，在开发板A中烧录[GATT客户端固件](../蓝牙_GATT客户端)，在开发板B中烧录GATT服务端固件。
>
> 4. 上电后，GATT服务端进入广播阶段，等待GATT客户端发起连接请求。具体测试过程如下：
>
>    （1）广播阶段：GATT服务端向周围广播自己的设备信息，等待被连接；
>
>    （2）连接阶段：GATT服务端收到连接请求，进入连接；
>
>    （3）收发测试：GATT服务端将收到的数据通过notify命令发送出去，从而完成一次测试。
>
>    | <img src=".\广播阶段.jpg" alt="广播阶段" style="zoom:150%;" /> | <img src=".\连接阶段.jpg" alt="连接阶段" style="zoom:150%;" /> | <img src=".\测试阶段.jpg" alt="测试阶段" style="zoom:150%;" /> |
>    | :----------------------------------------------------------: | :----------------------------------------------------------: | :----------------------------------------------------------: |
>    |                        （1）广播阶段                         |                        （2）连接阶段                         |                        （3）收发测试                         |

### 代码流程

> 1. 蓝牙模块初始化
>
>    - 蓝牙模块初始化的目的是对蓝牙进行自定义参数的配置，并初始化蓝牙协议栈。
>
>
>    ```c
>    void bt_ble_module_init(void)
>    {
>         lmp_set_sniff_disable();
>    	 //......
>         bt_max_pwr_set(3, 2, 2, 2);	//0-5
>         //......
>         lib_make_ble_address(tmp_ble_addr, (u8 *)bt_get_mac_addr());
>         le_controller_set_mac((void *)tmp_ble_addr);
>         //......
>         btstack_init();
>    }
>    ```
>
>    - 首先失能sniff模式；通过bt_max_pwr_set设置最大功率限定值，普通工作模式下最大功率等级范围为[0, 5]。
>
>    ```c
>    /*
>     *u8 pwr     ：EDR的最大功率
>     *u8 pg_pwr  ：page过程中的最大功率
>     *u8 iq_pwr  ：inquiry过程中的最大功率
>     *u8 ble_pwr ：BLE的最大功率
>     */
>    void bt_max_pwr_set(u8 pwr, u8 pg_pwr, u8 iq_pwr, u8 ble_pwr)
>    ```
>
>    - 通过lib_make_ble_address生成BLE地址，再调用le_controller_set_mac将其设置为当前BLE设备的地址。
>    - 最后调用btstack_init初始化蓝牙协议栈。att_server_init配置profile数据表，并注册write和read事件处理回调。hci_event_callback_set和le_l2cap_register_packet_handler分别注册HCI和L2CAP的回调函数，从而可以在应用层对BLE协议栈事件进行处理。ble_vendor_set_default_att_mtu设置默认的MTU，建立连接后可更改MTU。
>
>    ```c
>    void ble_profile_init(void)
>    {
>        //......
>        att_server_init(profile_data, att_read_callback, att_write_callback);
>        att_server_register_packet_handler(cbk_packet_handler);
>        hci_event_callback_set(&cbk_packet_handler);
>        le_l2cap_register_packet_handler(&cbk_packet_handler);
>        ble_vendor_set_default_att_mtu(ATT_LOCAL_MTU_SIZE);
>    }
>    ```
>
> 3. GATT服务端初始化
>
>    - 蓝牙协议栈初始化完成后，应用层的蓝牙状态回调函数中将收到BT_STATUS_INIT_OK事件，在此事件下调用bt_ble_init函数初始化GATT服务端。
>
>    ```c
>    int bt_connction_status_event_handler(struct bt_event *bt)
>    {
>    	//...... 
>        case BT_STATUS_INIT_OK:
>           bt_ble_init();
>    	//......    
>    }
>    ```
>
>    - ble_stack_gatt_role设置GATT为服务端角色。bt_ble_adv_enable对广播参数进行配置。
>
>    ```c
>    void bt_ble_init(void)
>    {
>    	//......
>        ble_module_enable(1);
>        //......
>    }
>    void ble_module_enable(u8 en)
>    {
>        //......
>        ble_stack_gatt_role(0);
>        //......
>        bt_ble_adv_enable(1);
>        //......
>    }
>    ```
>
> 3. 进入广播态
>
>    - advertisements_setup_init对广播参数进行配置，包括最小广播间隔、广播类型、广播信道，可根据需要修改配置值。完成上述初始化配置之后，调用ble_op_adv_enable，GATT服务端将进入广播状态，对周围的GATT客户端可发现，等待GATT客户端发起连接请求。
>
>
>    ```c
>    void bt_ble_adv_enable(u8 enable)
>    {
>        set_adv_enable(0, enable);
>    }
>    static int set_adv_enable(void *priv, u32 en)
>    {
>    	//......
>        advertisements_setup_init();
>        //......
>        ble_op_adv_enable(en);
>        //......
>    }
>    ```
>
> 4. 响应连接请求
>
>    - 协议栈接收到连接请求完成连接操作后，将会向应用层推送HCI_SUBEVENT_LE_CONNECTION_COMPLETE事件，在协议栈事件回调函数cbk_packet_handler中处理该事件。
>
>
>    ```c
>    //......
>    case HCI_SUBEVENT_LE_CONNECTION_COMPLETE:
>         con_handle = hci_subevent_le_connection_complete_get_connection_handle(packet);
>         server_profile_start(con_handle);
>    //......
>    ```
>
>    - ble_op_att_send_init配置ATT发送模块RAM大小，可根据需求更改配置。
>
>
>    ```c
>    void server_profile_start(u16 con_handle)
>    {
>        //......
>        ble_op_att_send_init(con_handle, att_ram_buffer, ATT_RAM_BUFSIZE, ATT_LOCAL_MTU_SIZE);
>        //......
>    }
>    ```
>
> 5. read、write事件管理
>
>    - 在与GATT客户端建立连接后，GATT客户端会获取GATT服务端的profile中描述的service和characteristic，从而GATT客户端可以通过相应的handle向GATT服务端发送read、write事件来进行数据交互。
>
>    - GATT服务端将收到的read事件集中到att_read_callback函数中进行处理。此处，收到read事件之后，将返回当前的设备名称。
>
>      ```c
>      
>      //......
>      case ATT_CHARACTERISTIC_2A00_01_VALUE_HANDLE:
>      //......
>      if (buffer) {
>          memcpy(buffer, &gap_device_name[offset], buffer_size);
>          att_value_len = buffer_size;
>          log_info("\n------read gap_name: %s \n", gap_device_name);
>      }
>      break;
>      //......
>      ```
>
>    - GATT服务端将收到的write事件集中到att_write_callback函数中进行处理。此处，ATT_CHARACTERISTIC_FF03_01_CLIENT_CONFIGURATION_HANDLE事件下，使能CCCD；ATT_CHARACTERISTIC_FF02_01_VALUE_HANDLE事件下，将收到的数据通过notify回传给GATT客户端。
>
>      ```c
>      //......
>      case ATT_CHARACTERISTIC_FF02_01_VALUE_HANDLE:
>      log_info("%s : ATT_CHARACTERISTIC_FF02_01_VALUE_HANDLE", __FUNCTION__);
>      //收发测试，自动发送收到的数据;for test
>      if (app_send_user_data_check(buffer_size)) {
>          app_send_user_data(ATT_CHARACTERISTIC_FF03_01_VALUE_HANDLE, buffer, buffer_size, ATT_OP_AUTO_READ_CCC);
>      }
>      break;
>                
>      case ATT_CHARACTERISTIC_FF03_01_CLIENT_CONFIGURATION_HANDLE:
>      set_ble_work_state(BLE_ST_NOTIFY_IDICATE);
>      check_connetion_updata_deal();
>      log_info("\n------write ccc:%04x,%02x\n", handle, buffer[0]);
>      att_set_ccc_config(handle, buffer[0]);
>      break;
>      //......
>      ```
>
> 6. 发送notify和indicate
>
> - GATT服务端可以通过notify和indicate主动向GATT客户端传递数据，但是GATT客户端需要使能CCCD才能接收notify和indicate命令。GATT服务端将带有NOTIFY或者INDICATION属性的 handle 通过传参到发送数据的函数中即可。
>
>
> ```c
> static int app_send_user_data(u16 handle, const u8 *data, u16 len, u8 handle_type)
> {
>     //......
>     if (!att_get_ccc_config(handle + 1)) {
>         log_info("fail,no write ccc!!!,%04x\n", handle + 1);
>         return APP_BLE_NO_WRITE_CCC;
>     }
> 	//......
>     ret = ble_op_att_send_data(handle, data, len, handle_type);
>     if (ret == BLE_BUFFER_FULL) {
>         ret = APP_BLE_BUFF_FULL;
>     }
>     //......
> }
> ```
---

## 常见问题

> * N/A

## 参考文档

> * N/A