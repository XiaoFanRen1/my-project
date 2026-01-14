# SDRAM配置说明

## SDRAM使能以及SDRAM大小配置

>1、芯片封装**不带**sdram时，在对应CPU目录下的  [isd_config.ini](..\..\..\..\cpu\wl82\tools\isd_config.ini) 文件中"ENABLE_SDRAM=0;"禁掉sdram。
>
>2、芯片封装**带**有sdram时，在对应CPU目录下的  [isd_config.ini](..\..\..\..\cpu\wl82\tools\isd_config.ini) 文件中"ENABLE_SDRAM=1;"使能sdram，同时也在对应CPU目录下的  [isd_config.ini](..\..\..\..\cpu\wl82\tools\isd_config.ini) 文件中"SDRAM_SIZE=2M;"设置为对应的大小。
>
>**注意事项：**芯片不带sdram，但是在对应CPU目录下的  [isd_config.ini](..\..\..\..\cpu\wl82\tools\isd_config.ini) 文件中ENABLE_SDRAM=1打开sdram，则会跑不起；当实际封装的sdram大小为8M却配置SDRAM_SIZE=2M，则系统正常运行后，程序使用超过2M大小地址会出现异常死机等现象。

## SDRAM时钟

>sdram时钟在可以在对应CPU目录下的  [isd_config.ini](..\..\..\..\cpu\wl82\tools\isd_config.ini) 文件中"SDRAM_CL=3;"修改时钟配置
>
>sdram时钟应根据系统时钟来配置，SYS_CLK<=240M或=320M时:[0-120M,1-160M,2-192M,3-240M,4-246M,5-252M,6-258M,7-264M,8-270M,9-276M,10-282M,11-288M]，详见下图。
>
>![SDRAM时钟配置](SDRAM时钟配置.png)
>
>**注意事项：**
>
>sdram时钟尽量<=240M，大于240M以上时钟sdram稳定性性不高，特别是270M以上，温度漂移对sdram稳定性影响较大。
>
>一般情况下SDK默认240M，用户只能减小sdram时钟，不能随意增大，建议使用默认即可。
>
>当系统时钟SYS_CLK>320M并且 < 384M时:SDRAM时钟为SYS_CLK*1.5/2，SYS_CLK >= 400M时:SDRAM时钟为SYS_CLK/2。
>
>当系统时钟SYS_CLK>240M并且 < 320M时:SDRAM时钟为SYS_CLK*1.5/2.
>
>系统时钟配置详情 [readme.md](..\系统频率\readme.md) 。

## 参考文档

> * N/A

