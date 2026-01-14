# 蓝牙发射器示例工程说明

> 本工程展示了蓝牙发射器接口使用示例。

---

## 适用平台

> 本工程适用以下芯片类型：
> 1. AC79系列芯片：AC790N、AC791N
>
> 杰理芯片和评估板的更多信息可在以下地址获取：
> [链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)

## 工程配置说明

> 在SDK选择打开[demo_edr](../../../demo/demo_edr/board/)主工程文件或者主工程Makefile

---



### 操作说明：

> SPP收发测试：
>
> 1. 在app_config.h文件中使能以下宏：
>
>    ```c
>    #define CONFIG_POWER_ON_ENABLE_BT      1
>    #define TCFG_USER_BT_CLASSIC_ENABLE    1
>    #define TCFG_USER_EMITTER_ENABLE       1
>    ```
>
> 2. 在bt_emitter.c文件中添加接收器的蓝牙设备名
> >    ```c
>   static const u8 bd_name_filt[][30] = {
>       "JL-AC79XX-AF0B",
>       "JL-AC790X-BD07",
>       "请在此处添加接收器设备名",
>   };
>   ```
>
> 3. 编译工程，烧录镜像，复位启动;
>
> 4. 在蓝牙初始化完成之后，长按KEY_MODE按键切换到蓝牙发射功能；
>
>    ```c
>    //所在文件bt_music.c
>    int bt_music_key_event_handler(struct key_event *key)
>    {
>        //......省略
>        case KEY_EVENT_LONG:
>        	//......省略
>            case KEY_MODE:
>                //发射器或者接收器切换
>                if (__this->enable) {
>                    if (__this->emitter) {
>                        bt_emitter_disable();
>                    } else {
>                        key_event_disable();
>                        bt_connection_disable();
>                        sys_timeout_add(NULL, bt_emitter_enable, 1000);
>                    }
>                }
>            	break;
>     	//......省略
>     }
>    ```
>
> 5. 在另一台设为接收器的开发板或带接收器功能的设备中，能看到发射器发送的数据，如MIC传输的声音数据；

>
> JIELI SDK的编译、烧写等操作方式的说明可在以下文档获取：[文档](../../../../doc/stuff/usb updater.pdf)

### 接口说明

> 1. emitter代码文件：bt_emitter.c
>
> 2. bt_emitter接口说明：
>
>     ```c
>     - emitter_rx_avctp_opid_deal    ：蓝牙发射接收到设备按键消息；
>     - emitter_rx_vol_change       	：蓝牙发射接收设备同步音量；
>     - emitter_media_source 			：蓝牙发射开启关闭通道；
>     ```

### 代码流程

> emitter测试流程：
>
> 1. app_main.c：在app_main函数中调用bt_ble_module_init函数初始化蓝牙模块；
> 2. bt_music.c：在蓝牙开启的状态长按KEY_MODE按键进行发射器跟接收器的切换,初始化蓝牙发射器；
> 3. bt_emitter.c：在开启发射器后,设备会先轮询保存在vm中的设备是否可连接,如果是则连接最近的一台接收器,否则从设备过滤清单中找出可连接的设备。
---

## 常见问题

> * N/A

## 参考文档

> * N/A