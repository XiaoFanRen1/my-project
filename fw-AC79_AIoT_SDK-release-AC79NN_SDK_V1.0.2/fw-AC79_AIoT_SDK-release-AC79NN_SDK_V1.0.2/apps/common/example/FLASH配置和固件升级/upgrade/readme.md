﻿﻿﻿﻿﻿﻿﻿﻿﻿﻿﻿﻿# 升级说明

> ​		杰理提供了多种固件升级的方法，包括：SD卡升级，U盘升级，HTTP OTA升级和FTP OTA升级等用户可以根据实际需求进行选择。

---

## 杰理升级模式介绍

> **1、术语**
>
> 本地升级：通过存储介质的方式获取本地升级文件，如：SD卡升级，U盘升级。
>
> 网络升级：通过网络协议远程获取升级文件，如：HTTP OTA升级，FTP OTA升级。
>
> res资源区：位于app code区域中，区域大小和地址随资源变化会自动调整。
>
> 预留区：位于reserved area区域，用户可以设置其大小、地址等属性，一旦生成后其大小和位置不能改变。
>
> 双备份升级：请参考双备份升级简介。
>
> 单备份升级：单备份升级简介。（注意：网络升级暂不支持单备份升级）
>
> 注意：app code区域和reserved area区域分布，请参考双备份升级简介中的双备份存储结构
>
> **2、各种升级方法支持的升级模式**
>
> SD卡升级：单备份升级、双备份升级、预留区资源+双备份升级。（其中预留区升级支持预留区资源备份升级和预留区资源强刷升级）
>
> U盘升级：单备份升级、双备份升级、预留区资源+双备份升级。（其中预留区升级支持预留区资源备份升级和预留区资源强刷升级）
>
> HTTP OTA升级：双备份升级、预留区资源+双备份升级、预留区资源升级。（其中预留区升级支持预留区资源备份升级和预留区资源强刷升级）
>
> FTP OTA升级：双备份升级、预留区资源+双备份升级、预留区资源升级。（其中预留区升级支持预留区资源备份升级和预留区资源强刷升级）
>
> **3、单备份升级简介**
>
> **（1）优缺点**
>
> * 优点
>
>   - 存储固件的flash容量仅需要一份固件的容量，节省flash空间
>   - 单备份主要升级过程在update loader中实现，与sdk流程无关，相对稳定，单一线程升级速度相对较快
> * 缺点
>   - update loader已编译二进制方式提供，客制化难度大，暂时无法支持二次开发
>   - 单备份升级出现意外中断时，无法正常开机
>   - 需在loader中等待重新升级成功方能开机使用，影响用户体验
>
> **（2）使用场景**
>
> - flash空间紧张方案
> - 成本敏感客户
>
> **4、双备份升级简介**
>
> **（1）优缺点**	
>
>   - 优点：
>
>     - 升级流程在app_code运行时执行，可以不影响正常功能使用，容易做到无感升级
>     - 升级意外中断仍有一份可运行代码，容灾体验好
>     - 无需update loader参与升级，客户可灵活在APP层自定义升级协议格式
>
>   - 缺点：
>
>     - 需要占用比固件大小接近多一倍flash空间
>     - 升级流程与app_code同步运行，升级可能受运行环境影响
>
>
> **（2）使用场合**	
>
> - 对升级体验有要求的用户
> - 方案的flash空间充裕
> - 需要客制化升级协议格式
>
> **（3）双备份存储结构**	
>
> ```
>  ------------------
>  | boot code      |
>  ------------------
>  | app code 0     |
>  ------------------
>  | app code 1     |
>  ------------------
>  | reserved area  |
>  ------------------
> ```
>
> ​			双备份存储结构有两个app code区域，当运行app code 0时，升级时将新固件写入app code 1区域，反之亦然升级时对app code N区域数据校验成功后，更新启动标志，复位后即可运行app code N代码，当升级中断或者新写入固件校验不成功，不会影响原来app code 的运行。
>
> ​			（1）双备份主动升级流程
>
> ![双备份主动升级流程](..\..\..\..\doc\stuff\dual_bank_active_update_flow_chart.png)
>
> ​			（2）双备份被动升级流程
>
> ![双备份被动升级流程](..\..\..\..\doc\stuff\\dual_bank_passive_update_flow_chart.png)
>
> **5、预留区资源升级**
>
> **（1）预留区备份升级**
>
> ​		 资源文件先升级到备份区，数据校验成功后，在将其搬运到预留区，在搬运过程中断电可以在备份区进行恢复。
>
> **（2）预留区强刷升级**
>
> ​		 直接升级到预留区，升级过程中断电，预留区资源数据会导致损坏。SD卡升级和U盘升级默认打开预留区强刷升级功能，即当备份区空间不够时，直接进行预留区强刷升级。HTTP OTA升级和FTP OTA升级可以通过配置是否打开预留区强刷升级功能。

---

## 单备份与双备份配置说明

> ​	**1、单备份配置**
>
>   - [download.bat](../../../../cpu/wl80/tools/download.bat)中打开如下配置
>
>     ```c
>     fw_add.exe -noenc -fw jl_isd.fw  -add ota.bin -type 100 -out jl_isd.fw
>     ```
>
>   - 在对应工程中的app_config.h里修改定义
>
>     ```c
>     #define  CONFIG_DOUBLE_BANK_ENABLE 0 //关闭双备份功能
>     ```
>
>  - [isd_config.ini](../../../../cpu/wl80/tools/isd_config.ini)需要修改如下配置
>
>    ```c
>    [EXTRA_CFG_PARAM]
>    #BR22_TWS_DB=YES;	  //dual bank flash framework enable（only valid for dual_bank）
>    #FLASH_SIZE=4M;		  //flash_size cfg (only valid for dual_bank)
>    #BR22_TWS_VERSION=0; //default fw version(only valid for dual_bank)
>
>    #DB_UPDATE_DATA=YES;  //generate db_update_data.bin(only valid for dual_bank)
>    NEW_FLASH_FS=YES;   //enable single bank flash framework (only valid for single_bank)
>    ```
>
> - 生成升级文件
>
>   - 采用sd卡/U盘升级：执行[升级文件.bat](../../../../cpu/wl80/tools/升级文件.bat)脚本文件生成升级文件update.ufw
>   - 采用网络升级：网络升级暂时不支持单备份升级
>
>
> ​	**2、双备份设置**
>
>  - [download.bat](../../../../cpu/wl80/tools/download.bat)中关闭如下配置
>
>    ```c
>     REM fw_add.exe -noenc -fw jl_isd.fw  -add ota.bin -type 100 -out jl_isd.fw
>    ```
>
>  - 在对应工程中的app_config.h里修改定义
>
>    ```c
>     #define  CONFIG_DOUBLE_BANK_ENABLE 1 //使能双备份功能
>    ```
>
>  - [isd_config.ini](../../../../cpu/wl80/tools/isd_config.ini)需要修改如下配置
>
>    ```c
>    [EXTRA_CFG_PARAM]
>    BR22_TWS_DB=YES;	  //dual bank flash framework enable（only valid for dual_bank）
>    FLASH_SIZE=4M;		  //flash_size cfg (only valid for dual_bank)
>    BR22_TWS_VERSION=0; //default fw version(only valid for dual_bank)
>
>    DB_UPDATE_DATA=YES;  //generate db_update_data.bin(only valid for dual_bank)
>     #NEW_FLASH_FS=YES;   //enable single bank flash framework (only valid for single_bank)
>    ```
>
> - 生成升级文件
>
>   - 采用网络升级：执行[升级文件-OTA.bat](../../../../cpu/wl80/tools/升级文件-OTA.bat)脚本文件生成升级文件update-ota.ufw
>   - 采用sd卡/U盘升级：执行[升级文件.bat](../../../../cpu/wl80/tools/升级文件-OTA.bat)脚本文件生成升级文件update.ufw
>
> ​	**3、注意事项**：
>
> ​			本地升级（SD卡升级、U盘升级）和网络升级（HTTP OTA升级、FTP OTA升级）采用的是不同的升级文件，不能相互混用。

## 预留区升级说明

> ​	**1、采用本地升级（SD卡升级、U盘升级）时预留区升级配置说明**		
>
> ```
> 步骤一：配置isd_config.ini文件，在[RESERVED_CONFIG]区域下设置可用于资源升级的预留区（如何配置预留区请查阅参考文档）
> [RESERVED_CONFIG]
> UPDATE_FILE=fixed_res/UPDATE; //其中UPDATE为升级文件
> UPDATE_ADR=AUTO;
> UPDATE_LEN=0x4000;
> UPDATE_OPT=1;
> 
> 步骤二：配置lib_update_config.c文件，使能预留区可升级功能。
> 设置const int support_reaserved_zone_file_dual_bank_update_en = 1;
> 设置const int support_passive_update_new_file_structure = 1;
> 
> 步骤三：编译工程，烧写程序，此时在flash空间中会升级一个名为UPDATE的预留区
> 
> 步骤四：当像升级UPDATE预留区时，如升级资源文件为UPDATE2，进行如下配置
> UPDATE_FILE=fixed_res/UPDATE2; //其中UPDATE为升级文件
> UPDATE_ADR=AUTO;
> UPDATE_LEN=0x4000;
> UPDATE_OPT=1;
> 
> 步骤五：执行”升级文件.bat“脚本就可以生成升级固件update.ufw
> 
> 注意：当下次升级不想对该预留区进行升级时，需要注释升级文件配置
> [RESERVED_CONFIG]
> #UPDATE_FILE=fixed_res/UPDATE; //其中UPDATE为升级文件
> UPDATE_ADR=AUTO;
> UPDATE_LEN=0x4000;
> UPDATE_OPT=1;
> ```
>
> **2、采用网络升级（HTTP OTA升级、FTP OTA升级）时预留区升级配置说明**
>
> ```
> 步骤一：配置download.bat文件，配置-update_files参数，如：
> 	isd_download.exe isd_config.ini -tonorflash -dev wl80 -boot 0x1c02000 -div0 -wait 300 -uboot uboot.boot -app app.bin cfg_tool.bin -res cfg audlogo -reboot 500 %KEY_FILE% -update_files normal
> 	其中，参数定义为：
> 	-update_files normal //生成只包含code升级文件
> 	-update_files embedded_only $(files) //生成只包含资源升级文件，其中$(files)为需要添加的资源文件
> 	-update_files embedded $(files) //生成包含code和资源的升级文件
> 
> 步骤二：配置isd_config.ini文件，在[RESERVED_CONFIG]区域下设置可用于资源升级的预留区
> UPDATE_ADR=AUTO;
> UPDATE_LEN=0x4000;
> UPDATE_OPT=1;
> 	
> 步骤三：配置lib_update_config.c文件，使能预留区可升级功能。
> 设置const int support_reaserved_zone_file_dual_bank_update_en = 1;
> 设置const int support_passive_update_new_file_structure = 1;
> 
> 步骤四：编译工程
> 
> 步骤五：执行“升级文件-OTA.bat”脚本升级对应的升级文件“update-ota.ufw”
> ```
>
> **3、预留区备份升级与预留区强刷升级配置说明**	
>
> ​		本地升级默认支持预留区强刷升级功能。对于网络升级，需要使能预留区强刷功能是，修改对应工程中的配置文件lib_update_config.c, 设置const int support_reserved_zone_forced_update = 1。
>
> **4、通过网络升级资源文件示例**	
>
> ```
> 示例说明：
> 	将test1.txt、test2.txt和test3.txt文件升级更新到UPDATE预留区，并读取test1.txt文件中的数据。
> 
> （1）利用文件cpu/$(cpu)/tools/packres目录下打包工具packres.exe，将这三个文件打包为UPDATE文件，如：
> 	packres.exe -n tone -o UPDATE test1.txt test2.txt test3.txt
> 	其中，参数定义为：
> 	-n，参数为需要把文件打包在那个文件目录下，目录名用于可以自定义。这里起名为tone目录名
> 	-o，输出的打包文件，名字用户可以自定义。注意：需要升级的打包文件名字需要和预留区名字保持一致。
> 
> （2）配置isd_config.ini文件，在[RESERVED_CONFIG]区域下设置可用于资源升级的预留区
> 	UPDATE_ADR=AUTO;
> 	UPDATE_LEN=0x4000;//这里设置预留区大小为16k，需要注意的是用户要根据可能需要存储的最大资源上限进行设定，设定后升级的资源大小必须小于该值。
> 	UPDATE_OPT=1;
> 	
> （3）设置好预留区后，需要烧写一次程序使得flash生成对应的预留区空间。
> 
> （4）配置download.bat文件，这里采用只升级资源，其配置为：
> 	isd_download.exe isd_config.ini -tonorflash -dev wl80 -boot 0x1c02000 -div0 -wait 300 -uboot uboot.boot -app app.bin cfg_tool.bin -res cfg audlogo -reboot 500 %KEY_FILE% -update_files embedded_only UPDATE
> 
> （5）编译工程
> 
> （6）执行“升级文件-OTA.bat”脚本，将生成的update-ota.ufw进行网络升级
> 
> （7）升级成功后，打开test1.txt文件读取其中的数据，示例代码如下：
> #include "system/includes.h"
> #include "app_config.h"
> #include "fs/fs.h"
> static void fs_test_task(void *priv)
> {
> 	int rlen = 0;
> 
> 	FILE *f = fopen("mnt/sdfile/app/update/tone/test1.txt", "r");
> 	if(!f)
> 	{
> 		printf("fopen err \n");	
> 		return;
> 	}
> 
> 	rlen = flen(f);
> 
> 	u8 *r_buf = (u8 *)malloc(rlen);
> 
> 	int len = fread(r_buf, 1, rlen, f);
> 	if (len != rlen) {
>      		fclose(f);
>     		 return;
> 	}
> 
> 	printf("fread data : %s\n", r_buf);
> 	fclose(f);
> 	free(r_buf);
> }
> 
> static void c_main_t(void *priv)
> {
>  	if (thread_fork("fs_test", 10, 512, 0, NULL, fs_test_task, NULL) != OS_NO_ERR) {
>     	 printf("thread fork fail\n");
>  	}
> }
> 
> late_initcall(c_main_t);
> ```
>
> 

---

## 常见问题

> * 预留区与res资源区有什么区别？
>
>   答：预留区位于reserved area区域，由用户自行配置，一旦生成其在flash中的位置和大小不能改变。res资源区位于app code区，在flash空间允许的范围内，res资源区大小没有限制。在双备份升级中，res资源区也会进行双备份处理，因此res资源在flash中会占用两倍的空间。而预留区作为资源区时，升级时资源文件大小不能超过对应预留区的空间大小。
>
> * 预留区备份升级与强刷升级有什么区别？
>
>   答：预留区备份升级时，借用app code备份区作为临时区（前提是备份区空间必须大于资源文件大小），先将资源写入该区，等数据校验成功后，再将资源从备份区覆盖到预留区，这样当意外断电时，也可以从备份区恢复到预留区。而强刷升级则是直接将资源覆盖到预留区，当意外断电时，原来的预留区资源会造成损坏。因此当使能强刷功能时，建议用户把重要的资源放在res资源区，如：一些开机时使用到的资源。
>
> * 资源升级预留区的大小如何选取？
>
>   答：预留区设置后，其空间大小不能改变，因此用户一开始就必须评估好以后可能存放最大资源上限，超出预留区空间的资源可以存放到res区域。
>
> * 在usb下载或OTA升级时需要擦除或保留对应reserved区域，该如何设置？
>
>   答：需要修改[isd_config.ini](../../../../cpu/wl82/tools/isd_config.ini),擦除对应区域时XX_OPT设置为0，不操作指定区域时XX_OPT设置为1。
>

## 参考文档

> * [资源配置例子](../资源配置例子/readme.md)