ui_server_test.c测试文件说明

> demo_ui工程已经将ui_server_test.c加入 只需要确认 CONFIG_UI_ENABLE宏，UI_COMPLETE_DEMO宏有没开启即可 
>
> ui_server_test.c实现了杰理ui常用ui控件的使用方法，列如：文字显示，数字显示，图片显示，时间显示，电池电量显示，开机动画显示等。
>
> 

---

## 适用平台

> 本工程适用以下芯片类型：
> 1. AC79系列芯片：AC790N、AC791N

> 杰理芯片和评估板的更多信息可在以下地址获取：
> [链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)

## 硬件说明

> * 本测试例子使用驱动为st7789s的显示屏带屏幕帧中断控制线。
> * 需要使用到的屏幕IO（D0-D7, CS , RS, WR, REST, TE）（RD脚外部上拉3.3v）
> * 硬件接线D0-D7依次对应H0-H7.（PAP推屏接口） 其中硬件固定D0-D7，WR 无法更改接线。具体接线请产考demo_ui工程中board.c 文件

## 模块依赖

> * ui.a  ui核心库
> * res.a 资源加载库
> * font.a 字库
> * ui_draw.a 绘图库

---



### 操作说明：

> 1. 修改download.bat加入下载文件夹ui_res 
> 2. 将ui工程文件夹中ui_server_test的工程内找到res文件夹将文件全部拷贝到[ui_res](../cpu/wl80/tools/ui_res)
> 3. 编译工程，烧录镜像，复位启动。
> 4. 开发过程中常用短接flash 1,2脚 上电后 释放1,2脚短接  进下载模式  也可以用专用的烧录工具进下载模式
>     JIELI SDK的编译、烧写等操作方式的说明可在以下文档获取：
>     [文档](/doc/stuff/usb%20updater.pdf)

### 显示失败说明：

> * 检查硬件接线
>
> * 检查app_config中有无开启测试工程宏 UI_COMPLETE_DEMO
>
> * 检查RD线有无上拉 
>
> * 检查ename.h文件是否更新(ename文件在UI工程内查找)
>
> * 检查供电是否正常
>
> * 检查修改脚本文件[download.bat](../cpu/wl80/tools)，在“-res” 选项增加资源目录，如：添加资源目录ui_res	
>
>   ```
>isd_download.exe isd_config.ini -tonorflash -dev wl80 -boot 0x1c02000 -div0 -wait 300 -uboot uboot.boot -app app.bin cfg_tool.bin -res cfg audlogo ui_res source -reboot 500 %KEY_FILE%

>   ​       注意有无ui_res，这个需要自己添加

### 代码流程

> * 显示开机界面
> * 文本综合测试 
> * ui综合应用界面测试
> * 刷图显示即动画显示

## 参考文档

> * 《LCD屏驱使用指南.pdf》
> * 《从0带你使用杰理ui.pdf》

