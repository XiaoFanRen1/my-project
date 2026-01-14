AC79 系列通用WiFi&蓝牙 SDK 固件程序

本仓库包含SDK release 版本代码，线下线上支持同步发布.

本工程提供的[例子](./apps/common/example/)，需要结合对应[开源板Demo](./apps/demo)进行编译.

快速开始
------------

欢迎使用杰理开源项目，在开始进入项目之前，请详细阅读SDK相关文档，
从而获得对杰理系列芯片和SDK 的大概认识，并且可以通过[SDK快速上手指南](./doc/AC79_SDK_QuickStartGuide.pdf)来进行开发.

工具链
------------

关于如何获取`杰理工具链` 和 如何进行环境搭建，请阅读以下内容：

* 编译工具 ：请安装杰理编译工具来搭建起编译环境, [下载链接](https://pan.baidu.com/s/1f5pK7ZaBNnvbflD-7R22zA) 提取码: `ukgx`

* USB 升级工具 : 在开发完成后，需要使用杰理烧写工具将对应的`hex`文件烧录到目标板，进行开发调试, 关于如何获取工具请进入申请 [链接](https://item.taobao.com/item.htm?spm=a1z10.1-c-s.w4004-22883854875.5.504d246bXKwyeH&id=620295020803) 并详细阅读对应的[文档](doc/stuff/usb%20updater.pdf)，以及相关下载脚本[配置](doc/stuff/ISD_CONFIG.INI配置文件说明.pdf)

介绍文档
------------

* 芯片简介 : [SoC 数据手册扼要](./doc/datasheet/AC791N规格书)

* SDK 版本信息 : [SDK 历史版本](doc/AC79NN_SDK_发布版本信息.pdf)

* SDK快速上手指南 : [SDK_QuickStartGuide](./doc/AC79_SDK_QuickStartGuide.pdf)

* SDK 结构文档 : [SDK 模块结构](./doc/architure)

编译工程
-------------
请选择以下一个工程进行编译，下列目录包含了便于开发的工程文件：

* WIFI应用 : [WIFI_CAMERA](./apps/wifi_camera)，适用领域：航拍,使用WIFI AP模式的产品
* WIFI应用 : [PIC_BOOK_STORY](./apps/wifi_story_machine)，适用领域：绘本故事机,WIFI/蓝牙音箱,点读笔/扫描笔,使用WIFI STA模式的产品
* 蓝牙应用 : [SCAN_BOX](./apps/scan_box) , 适用领域：蓝牙&USB扫码枪
* 各类功能Demo : [开源板Demo](./apps/demo)，适用领域：便于客户测试具体功能和重新开发自定功能需求的方案



SDK 同时支持Codeblock 和 Make 编译环境，请确保编译前已经搭建好编译环境

* Codeblock 编译 : 进入[apps](./apps)对应的工程目录,例如[wifi_camera](./apps/wifi_camera/board)并找到后缀为 `.cbp` 的文件, 双击打开便可进行编译.

* Makefile 编译 :  首先进入[tools目录](./tools)下, 双击 [make_prompt](tools/) 并输入 `make ac791n_wifi_camera`

  `在编译下载代码前，请确保USB 升级工具正确连接并且进入编程模式`
  
* OTA : 适用领域：代码资源单备份，代码资源双备份, 代码双备份资源部分固定部分可更新

蓝牙官方认证
-------------

经典蓝牙LMP / 低功耗蓝牙Link Layer 层和Host 协议栈均支持蓝牙5.0 和 5.1版本实现

* Core v5.0 [QDID 134104](https://launchstudio.bluetooth.com/ListingDetails/88799)

* Core v5.1 [QDID 136145](https://launchstudio.bluetooth.com/ListingDetails/91371)

WIFI官方认证
-------------
已完成Wi-Fi联盟互操作性测试,包含WPA/WPA2-Personal安全认证,B/G/N连通性测试

* Wi-Fi CERTIFIED™ Interoperability Certificate [Certification ID: WFA100441](https://www.wi-fi.org/content/search-page?keys=WFA100441)


硬件环境
-------------

* 开发评估板 ：开发板申请入口[链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)

* 生产烧写工具 : 为量产和裸片烧写而设计, 申请入口 [连接](https://item.taobao.com/item.htm?spm=a1z10.1-c-s.w4004-22883854875.8.504d246bXKwyeH&id=620941819219) 并仔细阅读相关 [文档](./doc/stuff/烧写器使用说明文档.pdf)

* 无线测试盒 : 为空中升级/射频标定/快速产品测试而设计, 申请入口 [链接](https://item.taobao.com/item.htm?spm=a1z10.1-c-s.w4004-22883854875.10.504d246bXKwyeH&id=620942507511), 阅读[文档](./doc/stuff/AC690x_1T2测试盒使用说明.pdf) 获取更多详细信息.


社区
--------------

* 技术交流[钉钉群](./doc/stuff/dingtalk.jpg)群号:31275808


免责声明
------------

AC79 SDK 支持AC79 系列芯片开发.
AC79 系列芯片支持了通用蓝牙/WIFI/音频/视频/UI显示常见应用，可以作为开发，评估，样品，甚至量产使用
