#!/bin/bash

${OBJCOPY} -O binary -j .text $1.exe $1_bank0.bin
${OBJCOPY} -O binary -j .ver_tag $1.exe $2_ver.bin
#${OBJCOPY} -O binary -j .flash_text $1.exe $1_flash.bin
${OBJDUMP} -disassemble -print-dbg  $1.exe > $1.lst

${OBJDUMP} -h $1.exe

objsizedump -lite -skip-zero -filename-only -enable-dbg-info $1.exe  > symbol.txt
#${BANKLINK} ${LOAD_ADDR} $1_bank0.bin ${PLATFORM}$1.bin

#cat ${PLATFORM}mini_$1.bin ${PLATFORM}$1.bin $1_flash.bin > $1.bin

#cp $1.bin ../../../../../sdk/tws/apps/download/ac694x/post_build/flash/
#cp $1_bank0.bin	$2.bin
#if [ $2 = edr_ota ]; then
#	host-client -project FILE -f $1_bank0.bin
#else

if [ $2 = uart_update ]; then
	../../../../uart_loader_enc $1_bank0.bin $2_${PLATFORM}.bin
else
	cp $1_bank0.bin	$2_${PLATFORM}.bin
fi

cat $2_${PLATFORM}.bin > $2.bin
#/opt/utils/ota_loader 0x1C22000 edr_ota2.bin compression ram 0x1C22000 uart_update.bin nocompression ram 0x1C22000 ble_ota.bin compression ram 0x1C22000 ble_app_ota.bin compression ram
/opt/utils/ota_loader \
    0x1C0A800 uart_update.bin compression ram \
    0x1C0A800 sd_update2.bin compression ram \
    0x1C0A800 usb_update2.bin compression ram \
    0x1C0A800 net_ota.bin compression ram \
    0x1C0A800 usb_hid_ota.bin compression ram \
    0x1C0A800 uart_user.bin compression ram \
    0x1C0A800 lcflash_ota.bin compression ram \

cat ota.bin > ota_temp.bin
#cat ota_temp.bin edr_ota2_ver.bin uart_update_ver.bin ble_ota_ver.bin ble_app_ota_ver.bin > ota.bin
cat ota_temp.bin \
    uart_update_ver.bin \
    sd_update2_ver.bin \
    usb_update2_ver.bin \
    net_ota_ver.bin \
    usb_hid_ota_ver.bin \
    uart_user_ver.bin \
    lcflash_ota_ver.bin \
    > ota.bin

if [ $3 = y ]; then
host-client -project ${PLATFORM}_file -f ota.bin $2.bin
fi
#fi
