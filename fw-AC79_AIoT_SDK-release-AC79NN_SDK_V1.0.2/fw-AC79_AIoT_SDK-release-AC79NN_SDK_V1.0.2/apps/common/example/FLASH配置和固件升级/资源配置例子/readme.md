﻿﻿# 资源配置说明

> 用于用户对资源文件存储进行配置。

---

## 资源区配置类型

> ​	   资源存储位置主要分为两个区域，res资源区和预留区。
>
> - res资源区：res资源区为app code中的一部分，在flash空间允许的范围内，res资源区大小没有限制。在双备份升级中，res资源区也会进行双备份处理，因此res资源在flash中会占用两倍的空间。
> - 预留区：预留区为独立于app code的一个区域，由用户自行配置，当有对应的配置时，会在flash中预留出指定大小的空间由用户自行使用。

---

## res区资源配置

> * 添加res区资源目录：
>
>   ​		修改脚本文件[download.bat](../cpu/wl80/tools)，在“-res” 选项增加资源目录，如：添加资源目录audlogo	
>
>   ```
> isd_download.exe isd_config.ini -tonorflash -dev wl80 -boot 0x1c02000 -div0 -wait 300 -uboot uboot.boot -app app.bin cfg_tool.bin -res cfg audlogo source -reboot 500 %KEY_FILE%
>   ```
>
> * 添加资源文件：
>
>   ​		只需要在已添加的资源目录中加入资源文件,如audlogo目录中增加提示音	
>
> * 资源文件读取：
>
>   ​		可以通过文件系统接口fread对res区资源进行读取，其路径格式为：mnt/sdfile/res/$(user_dir)/$(filename);其中user_dir为用户自定义资源目录，filename为用户添加的资源文件
>
>   ```
>    例子：
>    	char name[128];
>    	char read_buf[BUF_SIZE];
>   	FILE *f = fopen("mnt/sdfile/res/audlogo/test.mp3", "r");
>       if (!f) {
>           printf("fopen err!\n")
>       }else{
>           fget_name(f, name, sizeof(name));
>           printf("file_name: %s\n", name);
>           
>        	int len = fread(read_buf, 1, sizeof(read_buf), f);
>       	if(len > 0)
>        	{
>        		put_buf(read_buf, sizeof(read_buf));
>        	}
>   	}
>   ```
>
> 

---

## 预留区配置说明

> * 预留区配置：
>
>   ​		修改配置文件[isd_config.ini](../../../../cpu/wl80/tools/isd_config.ini)，[RESERVED_EXPAND_CONFIG]或[RESERVED_EXPAND_CONFIG]下添加配置项（注：具体配置说明请参考[ISD_CONFIG.INI配置文件说明.pdf](./../../../../doc/stuff/ISD_CONFIG.INI配置文件说明.pdf)），如：
>
>   ```
>   #预留区扩展
>   [RESERVED_EXPAND_CONFIG]
>   fixed.mp3_FILE=fixed_res/fixed.mp3;		#fixed_res目录下存放fixed.mp3文件
>   fixed.mp3_ADR=AUTO;						#由工具自动分配起始地址
>   fixed.mp3_LEN=0x4000;					#fixed.mp3文件的大小，需要4K对齐
>   fixed.mp3_OPT=1;						#0:下载代码时擦除指定区域；1:下载代码时不操作指定区域；2:下载代码时给指定区域加上保护；  
>   
>   或
>   #预留区扩展
>   [RESERVED_CONFIG]
>   fixed.mp3_FILE=fixed_res/fixed.mp3;		#fixed_res目录下存放fixed.mp3文件
>   fixed.mp3_ADR=AUTO;						#由工具自动分配起始地址
>   fixed.mp3_LEN=0x4000;					#fixed.mp3文件的大小，需要4K对齐
>   fixed.mp3_OPT=1;						#0:下载代码时擦除指定区域；1:下载代码时不操作指定区域；2:下载代码时给指定区域加上保护； 
>
> - 预留区数据读取：
>
>   ```
>   （1）[RESERVED_EXPAND_CONFIG]预留区使用方法可以通过文件系统接口fread对预留区数据进行读取，其路径格式为：mnt/sdfile/EXT_RESERVED/XXX（其中XXX为预留区文件名），如：
>      	char name[128];
>     	char read_buf[BUF_SIZE];
>     	FILE *f = fopen("mnt/sdfile/EXT_RESERVED/fixed.mp3", "r");
>     	if (!f) {
>         	 printf("fopen err!\n")
>     	}else{
>      		fget_name(f, name, sizeof(name));
>          	printf("file_name: %s\n", name);
>          	   	int len = fread(read_buf, 1, sizeof(read_buf), f);
>      		if(len > 0)
>      		{
>      			put_buf(read_buf, sizeof(read_buf));
>     	 	}
>   	}
>   
>   （2）[RESERVED_CONFIG]预留区使用方法可以通过文件系统接口fread对预留区数据进行读取，其路径格式为：mnt/sdfile/app/XXX（其中XXX为预留区文件名），如：
>      	char name[128];
>     	char read_buf[BUF_SIZE];
>     	FILE *f = fopen("mnt/sdfile/app/fixed.mp3", "r");
>     	if (!f) {
>         	 printf("fopen err!\n")
>     	}else{
>      		fget_name(f, name, sizeof(name));
>          	printf("file_name: %s\n", name);
>          	int len = fread(read_buf, 1, sizeof(read_buf), f);
>      		if(len > 0)
>      		{
>      			put_buf(read_buf, sizeof(read_buf));
>     	 	}
>   	}
>   ```
>   
>   
>   
> - 预留区使用注意事项
>
>   （1）[RESERVED_CONFIG]区域最多可以设置6个配置项，其中VM、BTIF和PRCT为系统默认使用，用户不能占用。用户可使用的只有3个配置项（如：配置文件中PROFILE、AISP和EXIF），可以根据实际需要进行使用和修改。当超出使用个数时，可以使用[RESERVED_EXPAND_CONFIG]区域。
>   
>   （2）当使用预留区作为资源升级区时，只能使用[RESERVED_CONFIG]；[RESERVED_EXPAND_CONFIG]区域暂时不支持作为资源升级区使用。
>   
>   （3）[RESERVED_EXPAND_CONFIG]区域在flash空间足够的情况下没有个数限制。

---

## 常见问题

> * 能否采用fwrite对不可变资源数据进行修改？
>
>   答：不能。
>

## 参考文档

> * 