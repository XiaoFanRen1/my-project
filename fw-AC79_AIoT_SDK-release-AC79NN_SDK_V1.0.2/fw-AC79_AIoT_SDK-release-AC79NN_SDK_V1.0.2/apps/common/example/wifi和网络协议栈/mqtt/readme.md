# MQTT使用示例工程说明

> 本工程展示了通过MQTT 协议连接阿里云服务器，并向阿里云订阅主题和发布消息
>

---

## 适用平台

> 本工程适用以下芯片类型：
>
> 1. AC79系列芯片：AC790N、AC791N

> 杰理芯片和评估板的更多信息可在以下地址获取：[链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)

## 工程配置说明

> 在SDK选择[PIC_BOOK_STORY](../../../../../apps/wifi_story_machine/board)工程CBP文件或者工程Makefile, 然后添加本事例工程代码
>
> 工程CBP文件或者工程Makefile 定义 CONFIG_NET_ENABLE
>
> app_config.h：
>
> * 打开 #define CONFIG_WIFI_ENABLE

## 模块依赖

> * 网络连接需要依赖的库
> * MQTT依赖库：lib_mqtt.a

---

### 编程流程

> 1.mqtt参数初始化：
>
> ```c
> unsigned char sendbuf[80], readbuf[80];
> MQTTClient client;
> Network network;
> MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;
> 
> NetworkInit(&network);
> MQTTClientInit(&client, &network, 30000, sendbuf, sizeof(sendbuf), readbuf, sizeof(readbuf));
> ```
>
> 2.mqtt网络层连接
>
> ```c
> char* address = "iot.eclipse.org";
> if ((rc = NetworkConnect(&network, address, 1883)) != 0)
> 	printf("Return code from network connect is %d\n", rc);
> ```
>
> 3.mqtt协议层连接
>
> ```c
> connectData.MQTTVersion = 3;
> connectData.clientID.cstring = "FreeRTOS_sample";
> 
> if ((rc = MQTTConnect(&client, &connectData)) != 0)
> 	printf("Return code from MQTT connect is %d\n", rc);
> else
> 	printf("MQTT Connected\n");
> ```
>
> 4.主题订阅（可选）
>
> ```c
> void messageArrived(MessageData* data)
> {
> 	printf("Message arrived on topic %.*s: %.*s\n", data->topicName->lenstring.len, data->topicName->lenstring.data,
> 		data->message->payloadlen, data->message->payload);
> }
> 
> if ((rc = MQTTSubscribe(&client, "FreeRTOS/sample/#", 2, messageArrived)) != 0)
> 	printf("Return code from MQTT subscribe is %d\n", rc);
> ```
>
> 5.消息发布（可选）
>
> ```c
> MQTTMessage message;
> char payload[30] = "mqtt test!";
> 
> message.qos = 1;
> message.retained = 0;
> message.payload = payload;
> message.payloadlen = strlen(payload);
> 
> if ((rc = MQTTPublish(&client, "FreeRTOS/sample/a", &message)) != 0)
> 	printf("Return code from MQTT publish is %d\n", rc);
> ```
>
> 6.MQTTYield调用
>
> ```c
> while (1)
> {
> 	if ((rc = MQTTYield(&client, 1000)) != 0)
> 		printf("Return code from yield is %d\n", rc);
> }
> ```
>
> 注意：上述只为MQTT客户端连接服务器并进行主题订阅和消息发布的一般流程，编程时根据实际情况进行调整修改

### 操作说明：

> 1. 示例需要运行在STA模式
> 2. 根据在阿里云上创建的设备，设置好address、username、password、subscribeTopic、publishTopic和clientID等信息，配置格式请参考参考文档。
> 3. 编译工程，烧录镜像，复位启动
> 4. 系统启动并成功联网后，可通过串口软件看到打印信息
>
> JIELI SDK的编译、烧写等操作方式的说明可在以下文档获取：[文档](../../../../doc/stuff/usb updater.pdf)



### 代码流程

> 1. c_main()入口：
>     A）创建任务mqtt_test
>
> 2. mqtt_test函数流程
>
>     A）通过wifi_get_sta_connect_state函数检测是否通过dhcp成功获取到ip
>
>     B)  成功通过dhcp获取到ip后，创建任务mqtt_start任务
>
> 3. mqtt_start任务流程
>
>     A) 连接阿里云服务器
>
>     B)  连接成功后，进行主题订阅和消息发布
---

## 常见问题

> * WIFI库如何选择？
>
>   wl_wifi.a     			   包含AP和STA模式,占据大内存,性能强悍
>   wl_wifi_sfc.a     		包含AP和STA模式,占据内存小
>   wl_wifi_ap.a			  只包含AP模式,占据大内存,性能强悍
>   wl_wifi_ap_sfc.a        只包含AP模式,占据内存小
>   wl_wifi_sta.a			只包含STA模式,占据大内存,性能强悍
>   wl_wifi_sta_sfc.a     	只包含STA模式,占据内存小
>
>   lwip.a       占据大内存,性能强悍
>   lwip_sfc.a   占据内存小
>
>   如果使用sta模式,还需要包含wpasupplicant.a库
>   
> * 如何进行mqtt调试？
>
>     答：mqtt无法订阅或发布消息，一般跟连接配置参数设置和格式等有关。可以借助其他MQTT客户端软件如：MQTT.fx等，先成功连接mqtt服务器并可以进行订阅和发布后，再仿照其参数进行mqtt程序配置。
>   

## 参考文档

> * [用MQTT.fx连接阿里云_流浪猫的博客-CSDN博客](https://blog.csdn.net/qq_45097019/article/details/107418853)
> * [MQTT开源代码](https://www.eclipse.org/paho/index.php?page=clients/c/embedded/index.php)
> * [MQTT官网](https://mqtt.org/)
> * [MQTT常用接口.docx](./MQTT常用接口.docx)