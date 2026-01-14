#include "app_config.h"
// *INDENT-OFF*
#ifdef __SHELL__


// 1. 定义工具链的基础路径
JIELI_TOOLCHAIN_BASE="/opt/jieli/pi32v2"
JIELI_TOOLCHAIN_BIN="${JIELI_TOOLCHAIN_BASE}/bin"

// 1.1 修复赋值空格问题，获取当前工作目录 (CWD)
TOOL="$(pwd)"
/* cd ${TOOL}/../../../../ */
[[ "$TOOL" == *"demo"* ]] && cd "${TOOL}/../../../../../" || cd "${TOOL}/../../../../"
// 1.2 计算 SDK 根目录 (上溯三级)
// 使用 cd 和 pwd 确保获得绝对路径
SDK_ROOT=$(pwd)
export SDK_ROOT

cd cpu/wl82/tools

DOWNLOAD_DIR=$(pwd) 
LZ4_PACKET="${DOWNLOAD_DIR}/lz4_packet"
PACKRES="${DOWNLOAD_DIR}/pack_res"
FAT_COMM="${DOWNLOAD_DIR}/fat_comm"
DOWNLOAD_TOOL="${DOWNLOAD_DIR}/isd_download"
FAT_IMAGE_TOOL="${DOWNLOAD_DIR}/fat_image_tool"
ISD_DOWNLOAD="${DOWNLOAD_DIR}/isd_download"
UBOOT_LZ4="${DOWNLOAD_DIR}/uboot_lz4" # 假设该工具名为 uboot_lz4

echo "======================"
echo "TOOL: $TOOL"
echo "SDK_ROOT: $SDK_ROOT"
echo "DOWNLOAD_DIR: $DOWNLOAD_DIR"
echo "======================"



// 2. 检查基础路径是否存在且为目录
if [ ! -d "$JIELI_TOOLCHAIN_BIN" ]; then
    echo "错误：Jieli 工具链目录不存在或路径错误：" >&2
    echo "期望路径: $JIELI_TOOLCHAIN_BIN" >&2
    exit 1  # 返回非零状态码表示失败
fi


// 3. 检查关键工具是否存在（可选，但推荐）
OBJCOPY_TOOL="${JIELI_TOOLCHAIN_BIN}/objcopy"
OBJDUMP_TOOL="${JIELI_TOOLCHAIN_BIN}/objdump"
OBJSIZEDUMP_TOOL="${JIELI_TOOLCHAIN_BIN}/objsizedump" //导出新变量

if [ ! -f "$OBJCOPY_TOOL" ] || [ ! -f "$OBJDUMP_TOOL" ]; then
    echo "警告：Jieli 工具链目录存在，但关键工具 (objcopy/objdump) 文件缺失。" >&2
    echo "检查文件: $OBJCOPY_TOOL 和 $OBJDUMP_TOOL" >&2
    // 即使文件缺失，我们仍可以设置变量，但建议还是退出
    exit 2 
fi

// 4. 设置并导出环境变量
// 使用 export 确保这些变量可以在脚本启动的子进程或后续命令中使用
export OBJCOPY="$OBJCOPY_TOOL"
export OBJDUMP="$OBJDUMP_TOOL"
export OBJSIZEDUMP="$OBJSIZEDUMP_TOOL"

echo "Jieli 工具链路径已验证并设置成功。"
echo "OBJCOPY 环境变量已设置为: $OBJCOPY"
echo "OBJDUMP 环境变量已设置为: $OBJDUMP"


ELF_NAME="$1"
ELF_FILE="${DOWNLOAD_DIR}/${ELF_NAME}.elf"
LIST_FILE="${DOWNLOAD_DIR}/${ELF_NAME}.lst"
SYMBOL_TABLE_FILE="${DOWNLOAD_DIR}/symbol_tbl.txt"
SEGMENT_FILE="${DOWNLOAD_DIR}/segment_list.txt" 

//核心反汇编命令 (使用双引号包裹变量以防止路径中包含空格或其他特殊字符)
//"${OBJDUMP}" -D -print-imm-hex -print-dbg -mcpu=r3 "$ELF_FILE" > "$LIST_FILE"
echo "正在生成段头信息文件 ($SEGMENT_FILE)..."
"${OBJDUMP}" -section-headers "$ELF_FILE" > "$SEGMENT_FILE"
echo "正在使用 $OBJSIZEDUMP 生成符号表 ($SYMBOL_TABLE_FILE)..."
"${OBJSIZEDUMP}" -lite -skip-zero -enable-dbg-info "$ELF_FILE" | sort -k 1 > "$SYMBOL_TABLE_FILE"
echo "--- ELF 文件段头信息 ---"
"${OBJDUMP}" -section-headers "$ELF_FILE" 
echo "--------------------------"
//${OBJDUMP} -section-headers ${ELF_FILE} > segment_list.txt
//${OBJSIZEDUMP} -lite -skip-zero -enable-dbg-info $1.elf | sort -k 1 > symbol_tbl.txt
//${OBJDUMP} -section-headers $1.elf
echo "正在提取 ELF 段并生成二进制文件..."

// 80: 提取 .text 段
"${OBJCOPY}" -O binary -j .text "$ELF_FILE" "${DOWNLOAD_DIR}/text.bin"

// 81: 提取 .data 段
"${OBJCOPY}" -O binary -j .data "$ELF_FILE" "${DOWNLOAD_DIR}/data.bin"

// 82: 提取 .ram0_data 段
"${OBJCOPY}" -O binary -j .ram0_data "$ELF_FILE" "${DOWNLOAD_DIR}/ram0_data.bin"

// 83: 提取 .dcache_ram_data 段
"${OBJCOPY}" -O binary -j .dcache_ram_data "$ELF_FILE" "${DOWNLOAD_DIR}/dcache_ram_data.bin"

// 84: 提取 .dynamic_data 段
"${OBJCOPY}" -O binary -j .dynamic_data "$ELF_FILE" "${DOWNLOAD_DIR}/dynamic_data.bin"


echo "二进制文件提取完成。"



    cat "${DOWNLOAD_DIR}/text.bin" \
        "${DOWNLOAD_DIR}/data.bin" \
        "${DOWNLOAD_DIR}/dcache_ram_data.bin" \
        "${DOWNLOAD_DIR}/dynamic_data.bin" \
        "${DOWNLOAD_DIR}/ram0_data.bin" \
        > "${DOWNLOAD_DIR}/app.bin"

if [ $? -eq 0 ]; then
    echo "app.bin 文件生成成功：${PROJ_BUILD}/app.bin"
else
    echo "app.bin 文件生成失败。" >&2
    exit 11
fi


#if defined CONFIG_UI_ENABLE
#if defined CONFIG_UI_FILE_SAVE_IN_RESERVED_EXPAND_ZONE
"$PACKRES" -n ui -o packres/UIPACKRES ui_res
#else
UI_RES="ui_res"
#endif
#endif

#if defined CONFIG_AUDIO_ENABLE && defined CONFIG_VOICE_PROMPT_FILE_PATH
#if defined CONFIG_VOICE_PROMPT_FILE_SAVE_IN_RESERVED_EXPAND_ZONE
"$PACKRES" -n tone -o packres/AUPACKRES audlogo
#else
AUDIO_RES="audlogo"
#endif
#endif


#if defined CONFIG_DOUBLE_BANK_ENABLE
UPDATE_FILES="-update_files normal"
export UPDATE_FILES
#endif



#if defined CONFIG_SDFILE_EXT_ENABLE
    "${FAT_IMAGE_TOOL}" CONFIG_SDNAND_HFAT_LEN_TEXT \
                        --sectors-per-cluster \
                        CONFIG_SDNAND_HFAT_CLUSTER_SIZE \
                        --lfn true --n-root 512 \
                        --volume-name HFAT_IMG \
                        --output jl_hfat.bin \
                        --fat-dir hfat_dir

    "${FAT_IMAGE_TOOL}"  CONFIG_SDNAND_FAT1_LEN_TEXT \
                        --sectors-per-cluster \
                        CONFIG_SDNAND_FAT1_CLUSTER_SIZE \
                        --lfn true --n-root 512 \
                        --volume-name FAT1_IMG \
                        --output jl_fat1.bin \
                        --fat-dir fat1_dir


    sudo "${ISD_DOWNLOAD}" isd_config.ini \
                      -gen2 -to-sdcard -dev wl82 -boot 0x1c02000 \
                      -div1 -app app.bin cfg_tool.bin \
                      -res "${AUDIO_RES}" "${UI_RES}" cfg "${MEDIA_RES}" \
                      -extend-bin -output-bin jl_hfs.bin "${UPDATE_FILES}" \
                      -no-app-bin-enc

#elif defined CONFIG_SFC_ENABLE

    sudo "${ISD_DOWNLOAD}" isd_config.ini \
                      -gen2 -tonorflash -dev wl82 -boot 0x1c02000 \
                      -div1 -wait 300 -uboot uboot.boot -app app.bin cfg_tool.bin \
                      -res "${AUDIO_RES}" "${UI_RES}" cfg \
                      -reboot 500 "${UPDATE_FILES}" \
                      -extend-bin 
#else


run_addr="0x103000"
load_addr="0x10B000"
mask_addr="0xf0000"
rm -f rom.image 
    "${UBOOT_LZ4}" app.bin app.lz4 \
                   "${run_addr}" "${load_addr}" rom.image "${mask_addr}"
    sudo "${ISD_DOWNLOAD}" isd_config.ini \
                      -gen2 -tonorflash -dev wl82 -boot 0x1c02000 \
                      -div1 -wait 300 -uboot uboot.boot -app app.lz4 cfg_tool.bin \
                      -res "${AUDIO_RES}" "${UI_RES}" cfg \
                      -reboot 50 "${UPDATE_FILES}" \
                      -extend-bin "${EXFLASH_FILES}" \
                      -output-ufw jl_isd.ufw
#endif

#if defined(CONFIG_DOUBLE_BANK_ENABLE) && defined(CONFIG_UPDATE_COMPRESS)
sudo "${ISD_DOWNLOAD}" -make-upgrade-bin -ufw jl_isd.ufw -output db_update.bin
#endif

//echo "isd_download.exe isd_config.ini -gen2 -tonorflash -dev wl82 -boot 0x1c02000 -div1 -wait 300 -uboot uboot.boot -app app.bin cfg_tool.bin -res %AUDIO_RES% %UI_RES% cfg -reboot 500 %UPDATE_FILES% -extend-bin" >> ${PROJ_BUILD}



exit 0

#if 0
echo "if exist *.mp3 del *.mp3" >> ${PROJ_BUILD}
echo "if exist *.PIX del *.PIX" >> ${PROJ_BUILD}
echo "if exist *.TAB del *.TAB" >> ${PROJ_BUILD}
echo "if exist *.res del *.res" >> ${PROJ_BUILD}
echo "if exist *.sty del *.sty" >> ${PROJ_BUILD}
#endif

#if defined CONFIG_SDFILE_EXT_ENABLE
echo "copy isd_config_loader.ini loader_tools\isd_config.ini" >> ${PROJ_BUILD}
echo "copy jl_hfs.bin loader_tools\jl_hfs.bin" >> ${PROJ_BUILD}
echo "copy jl_hfat.bin loader_tools\jl_hfat.bin" >> ${PROJ_BUILD}
echo "copy jl_fat1.bin loader_tools\jl_fat1.bin" >> ${PROJ_BUILD}
#else
#if !(CONFIG_DOUBLE_BANK_ENABLE)
echo "fw_add.exe -noenc -fw jl_isd.fw -add ota.bin -type 100 -out jl_isd.fw" >> ${PROJ_BUILD}
#endif
echo "fw_add.exe -noenc -fw jl_isd.fw -add script.ver -out jl_isd.fw" >> ${PROJ_BUILD}

echo "ufw_maker.exe -fw_to_ufw jl_isd.fw" >> ${PROJ_BUILD}
#endif
echo "ping /n 2 127.1>null" >> ${PROJ_BUILD}
//echo "IF EXIST null del null" >> ${PROJ_BUILD}
echo "del dynamic_data.bin" >> ${PROJ_BUILD}
echo "del cache_ram_data.bin" >> ${PROJ_BUILD}
echo "del data.bin" >> ${PROJ_BUILD}
echo "del ram0_data.bin" >> ${PROJ_BUILD}
echo "del text.bin" >> ${PROJ_BUILD}
#if defined CONFIG_SDFILE_EXT_ENABLE
echo "del jl_hfs.bin" >> ${PROJ_BUILD}
echo "del jl_hfat.bin" >> ${PROJ_BUILD}
echo "del jl_fat1.bin" >> ${PROJ_BUILD}
echo "cd loader_tools" >> ${PROJ_BUILD}
echo "call download.bat" >> ${PROJ_BUILD}
echo "cd ..\\" >> ${PROJ_BUILD}
#endif
echo "TIMEOUT /T 3" >> ${PROJ_BUILD}
echo "exit /b 0" >> ${PROJ_BUILD}

#if defined CONFIG_SDFILE_EXT_ENABLE
files="isd_config.ini isd_config_loader.ini download.bat"
#else
files="isd_config.ini download.bat"
#endif

host-client -project ${NICKNAME}$2 -f ${files} $1.elf

rm download.bat
rm isd_config.ini

#else


@echo off

@echo *********************************************************************
@echo                           AC791N SDK
@echo *********************************************************************
@echo %date%

cd /d %~dp0

echo %*

#if defined CONFIG_UI_ENABLE
#if defined UI_USE_WIFI_CAMERA_PROJECT
cd ..\..\..\ui_project\3.ui_demo_wifi_camera\ui_320x480_test\project
start auto_ui_res.bat
cd ..\..\..\..\cpu\wl82\tools
TIMEOUT /T 8
#endif
#endif

set OBJDUMP=C:\JL\pi32\bin\llvm-objdump.exe
set OBJCOPY=C:\JL\pi32\bin\llvm-objcopy.exe
set ELFFILE=sdk.elf

REM %OBJDUMP% -D -address-mask=0x1ffffff -print-dbg %ELFFILE% > sdk.lst
%OBJCOPY% -O binary -j .text %ELFFILE% text.bin
%OBJCOPY% -O binary -j .data %ELFFILE% data.bin
%OBJCOPY% -O binary -j .ram0_data  %ELFFILE% ram0_data.bin
%OBJCOPY% -O binary -j .cache_ram_data  %ELFFILE% cache_ram_data.bin
%OBJCOPY% -O binary -j .dynamic_data  %ELFFILE% dynamic_data.bin

%OBJDUMP% -section-headers -address-mask=0x1ffffff %ELFFILE%
%OBJDUMP% -t %ELFFILE% > symbol_tbl.txt

copy /b text.bin+data.bin+dynamic_data.bin+ram0_data.bin+cache_ram_data.bin app.bin

#if defined CONFIG_UI_ENABLE
#if defined CONFIG_UI_FILE_SAVE_IN_RESERVED_EXPAND_ZONE
packres\packres.exe -n ui -o packres/UIPACKRES ui_res
#else
set UI_RES=ui_res
#endif
#endif

#if defined CONFIG_AUDIO_ENABLE
#if defined CONFIG_VOICE_PROMPT_FILE_PATH && \
defined CONFIG_VOICE_PROMPT_FILE_SAVE_IN_RESERVED_EXPAND_ZONE
packres\packres.exe -n tone -o packres/AUPACKRES audlogo
#else
set AUDIO_RES=audlogo
#endif
#endif

REM set KEY_FILE=-key JL_791N-XXXX.key
REM set KEY_FILE=-key1 JL_791N-XXXX.key1 -mkey JL_791N-XXXX.mkey

set CFG_FILE=cfg

REM  添加新参数-update_files说明
REM  1/normal就是普通的文件目录结构
REM  2/embedded就是每个文件数据前都会有一个头

REM  只生成code + res升级文件
REM  -update_files normal
REM  生成的文件名字为：db_update_files_data.bin

REM  只生成预留区资源升级文件
REM  -update_files embedded_only $(files) ,其中$(files)为需要添加的资源文件
REM  生成的文件名字为：db_update_files_data.bin

REM  生成code + res + 预留区资源升级文件
REM  -update_files embedded $(files) ,其中$(files)为需要添加的资源文件
REM  生成的文件名字为：db_update_files_data.bin

#if CONFIG_DOUBLE_BANK_ENABLE
set UPDATE_FILES=-update_files normal
#endif

#if defined CONFIG_SDFILE_EXT_ENABLE
fat_image_tool.exe --size CONFIG_SDNAND_HFAT_LEN_TEXT --sectors-per-cluster CONFIG_SDNAND_HFAT_CLUSTER_SIZE --lfn true --n-root 512 --volume-name HFAT_IMG --output jl_hfat.bin --fat-dir hfat_dir
fat_image_tool.exe --size CONFIG_SDNAND_FAT1_LEN_TEXT --sectors-per-cluster CONFIG_SDNAND_FAT1_CLUSTER_SIZE --lfn true --n-root 512 --volume-name FAT1_IMG --output jl_fat1.bin --fat-dir fat1_dir
isd_download.exe isd_config.ini -gen2 -to-sdcard -dev wl82 -boot 0x1c02000 -div1 -app app.bin cfg_tool.bin -res %AUDIO_RES% %UI_RES% cfg -extend-bin -output-bin jl_hfs.bin %UPDATE_FILES% -no-app-bin-enc
#elif defined CONFIG_SFC_ENABLE
isd_download.exe isd_config.ini -gen2 -tonorflash -dev wl82 -boot 0x1c02000 -div1 -wait 300 -uboot uboot.boot -app app.bin cfg_tool.bin -res %AUDIO_RES% %UI_RES% %CFG_FILE% -reboot 500 %KEY_FILE% %UPDATE_FILES% -extend-bin
#else
REM @@@@@@@无效参数
set run_addr=0x2000
set load_addr=0x4000
set mask_addr=0x100000
uboot_lz4.exe app.bin app.lz4 %run_addr% %load_addr% rom.image %mask_addr%
REM @@@@@@@@@@@@@@@

isd_download.exe isd_config.ini -gen2 -tonorflash -dev wl82 -boot 0x1c02000 -div1 -wait 300 -uboot uboot.boot -app app.lz4 cfg_tool.bin -res %AUDIO_RES% %UI_RES% %CFG_FILE% -reboot 500 %KEY_FILE% %UPDATE_FILES% -extend-bin
#endif


@REM 常用命令说明
@rem -format vm         // 擦除VM 区域
@rem -format all        // 擦除所有
@rem -reboot 500        // reset chip, valid in JTAG debug

echo %errorlevel%

@REM 删除临时文件
if exist *.mp3 del *.mp3
if exist *.PIX del *.PIX
if exist *.TAB del *.TAB
if exist *.res del *.res
if exist *.sty del *.sty

#if defined CONFIG_SDFILE_EXT_ENABLE
copy isd_config_loader.ini loader_tools\isd_config.ini
copy jl_hfs.bin loader_tools\jl_hfs.bin
copy jl_hfat.bin loader_tools\jl_hfat.bin
copy jl_fat1.bin loader_tools\jl_fat1.bin
#else
#if !(CONFIG_DOUBLE_BANK_ENABLE)
@REM 生成固件升级文件
fw_add.exe -noenc -fw jl_isd.fw -add ota.bin -type 100 -out jl_isd.fw
#endif

@REM 添加配置脚本的版本信息到 FW 文件中
fw_add.exe -noenc -fw jl_isd.fw -add script.ver -out jl_isd.fw

ufw_maker.exe -fw_to_ufw jl_isd.fw
#endif

@REM 生成配置文件升级文件
rem ufw_maker.exe -chip AC630N %ADD_KEY% -output config.ufw -res bt_cfg.cfg

ping /n 2 127.1>null
IF EXIST null del null

::del app.bin
del dynamic_data.bin
del cache_ram_data.bin
del data.bin
del ram0_data.bin
del text.bin

#if defined CONFIG_SDFILE_EXT_ENABLE
del jl_hfs.bin
del jl_hfat.bin
del jl_fat1.bin
cd loader_tools
call download.bat
cd ..\

#endif

::退出当前批处理加返回值(0),才能多个批处理嵌套调用批处理
exit /b 0

#endif
// *INDENT-ON*

