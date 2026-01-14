
UPDATA_SIZE = 0x80;
UPDATE_BEG = 0x1c7fe00 - 128;

ISR_BASE = 0x1c7fe00;

MEMORY
{
    /*起始地址0x1C0A800和uboot相关，详情uboot的ram.ld，0x1c02000+34*1024 = 0x1C0A800，故OTA_LOADER起始地址：0x1C0A800 */
    /*总长度：0x1C0A800 + 465K + 1K +3K，务必 < 0x1C80000*/
    comm(rx)            : ORIGIN = 0x1C0A800, 				LENGTH = 465K
	ex_code(rx)			: ORIGIN = 0x1C0A800 + 465K,		LENGTH = 1K
	reserved(rx)		: ORIGIN = 0x1C0A800 + 465K + 1K,	LENGTH = 3K
}


ENTRY(_uboot_start);

SECTIONS
{
    . = ORIGIN(comm);
    .text ALIGN(4):
    {
		/*text*/
        *startup.o(.text*)
        *(.text*)

		*(.lmp_release)
		poweroff_text_begin = . ;
		*(.poweroff_text)
		poweroff_text_end = . ;
		loop_detect_handler_begin = .;
		KEEP(*(.loop_detect_region))
		loop_detect_handler_end = .;

		sync_time_detect_handler_begin = .;
		KEEP(*(.sync_time_loop_detect_region))
		sync_time_detect_handler_end = .;

		*(.bredr)
		bredr_text_begin = .;
		*(.bredr_irq_text)
		bredr_text_end   = .;

		ble_text_begin = .;
		*(.ble_irq_text)
		ble_text_end   = .;

		/*data*/
        *(.rodata*)

		*(.fs_update_const)
		*(.fs_update_code)

		*(.bt_update_const)
		*(.bt_update_code)

		*(.cfg_update_const)
		*(.cfg_update_code)

		*(.spi0flash_const)
		*(.spi0flash_code)
		*(.spi0_code)

		*(.volatile_ram)
		*(.volatile_ram_code)
		*(.non_volatile_ram)
		*(.non_volatile_ram_code)

		*(.bredr_irq_code)
		*(.bredr_irq)
		*(.bredr_irq_const)
		*(.hwi_const)
		*(.hwi_code)
		. = ALIGN(4);
		_early_initcall_begin = .;
		PROVIDE(early_initcall_begin = .);
		KEEP(*(.early.initcall))
			_early_initcall_end = .;
		PROVIDE(early_initcall_end = .);

		*(.bt_updata_ram_code)

		*(.reset_pin_code)
		*(.reset_pin_const)

		_bredr_nv_mem_begin = .;
		*(.ram1_data)
		_bredr_nv_mem_end = .;
		*(.fs_update_data)
		*(.bt_update_data)
		*(.cfg_update_data)
		*(.spi0flash_data)
		*(.hwi_data)
		. = ALIGN(4);
		*(*.rf.text.cache.L2.rf_trim)

		PROVIDE(__fsymtab_start = .);
		KEEP(*(.FSymTab))
		PROVIDE(__fsymtab_end = .);
		PROVIDE(__vsymtab_start = .);
		KEEP(*(.VSymTab))
		PROVIDE(__vsymtab_end = .);
		PROVIDE(__rtatcmdtab_start = .);
		KEEP(*(.RtAtCmdTab))
		PROVIDE(__rtatcmdtab_end = .);

		__rf_trim_movable_region_start = .;
		*(.movable.region.1);
		__rf_trim_movable_region_end = .;
		*(.flushinv_icache)
		*(.movable.stub.1)
		*(*.text.cache.L1)
		*(*.text.cache.L2)
		*(*.text.const.cache.L2)
		_os_begin = .;
		PROVIDE(os_begin = .);
		*(.os_code)
		*(.os_const)
		*(.os_str)
		*(.os_critical_code)
		*(.os_port_code)
		*(.os_port_const)
		_os_end = .;
		PROVIDE(os_end = .);	


        *(.data*)
		. = ALIGN(4);
		/* . = LENGTH(comm); */
    } > comm

	.bss (NOLOAD) :SUBALIGN(4)
	{
		*(.boot_bss)
		*(.memp_memory_x)
		. = ALIGN(4);
	} > comm

	.bss2 (NOLOAD) :SUBALIGN(4)
	{
		*(.sram)
	} > comm

	.bss3 (NOLOAD):SUBALIGN(4)
	{
		*(.bss)
		*(.fs_update_bss)
		*(.bt_update_bss)
		*(.cfg_update_bss)
		*(.spi0flash_bss)
		*(.hwi_bss)
		*(.ram1_bss)
		*(.mask_ram0)
		*(.update_buf)
		*(.bredr_rxtx_bulk)
		*(.os_port_bss)
		*(.os_bss)

		*(.cpu0_ustack)
		_cpu0_sstack_begin = .;
		PROVIDE(cpu0_sstack_begin = .);
		*(.cpu0_sstack)
		_cpu0_sstack_end = .;
		PROVIDE(cpu0_sstack_end = .);
	} > comm

	.bss1 (NOLOAD) :SUBALIGN(4)
	{
		*(.bd_base)
		*(.comm_rf_para)
		*(.bd_base1)
	} > comm


	. = ORIGIN(reserved);
	.ver_tag ALIGN(4):
	{
		KEEP(*(.version_tag1))
		KEEP(*(.version_tag2))
		KEEP(*(.version_tag3))
		. = ALIGN(4);
	} > reserved

	.ex_code (NOLOAD) :SUBALIGN(4)
	{
	} > ex_code


	_bss_begin = ADDR(.bss ) ;
	_bss_size  = SIZEOF(.bss);
    _bss1_begin = ADDR(.bss1 ) ;
    _bss1_size  = SIZEOF(.bss1);
    _bss2_begin = ADDR(.bss2 ) ;
    _bss2_size  = SIZEOF(.bss2);

	_bss3_begin = ADDR(.bss3);
	_bss3_size = SIZEOF(.bss3);

	_bss_total_size = _bss_size + _bss1_size + _bss2_size + _bss3_size;
	UPDATA_BREDR_BASE_BEG = _bss1_begin;
	_text_begin = ADDR(.text);
    _text_size = SIZEOF(.text);
	_text_end = ADDR(.text)+SIZEOF(.text);

	_ex_code_begin = ADDR(.ex_code);
	_ex_code_size = SIZEOF(.ex_code);
	_ex_code_end = _ex_code_begin + _ex_code_size;

	_version_begin = ADDR(.ver_tag);
	_version_size = SIZEOF(.ver_tag);
	_version_end = _version_begin + _version_size;
}



