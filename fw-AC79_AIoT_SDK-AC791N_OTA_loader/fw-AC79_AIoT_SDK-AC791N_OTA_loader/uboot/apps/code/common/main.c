#include "config.h"
#include "boot.h"
#include "p33_interface.h"
#include "gpio.h"
#include "reset.h"
#include "memory.h"
#include "p33.h"
#include "asm/wdt.h"
#include "uart.h"
#include "sys_timer.h"
#include "csfr.h"

#include "dev_update/updata_mode.h"
#include "asm/clock_hw.h"

#define UART_UPDATE_STR	"UARTUPDATE"

//变量定义
static struct boot_info_t g_boot_info SEC(.boot_bss);
struct SYS_CFG boot_sys_cfg SEC(.boot_bss);

extern u32 malloc_pool[];
extern u32 malloc_pool_size;
//变量定义END

//函数声明
extern void maskrom_uart_info_save(u8 *info);
extern bool updata_check(void);
extern void local_irq_disable(void);
extern void irq_disable(int index);
extern void bredr_bd_close();
extern void update_loader_ram_record_check(void);
extern u32 get_reset_cfg_from_isd_config(u8 *reset_cfg);
extern void power_reset_source_dump(void);
extern void flash_fs_v2_update_init(void);
extern u32 get_power_pin(u32 *gpio, u8 *level);
extern u32 get_power_pin_multi(u32 *gpio, u8 *level, u32 cnt);
extern u32 get_uart_debug_info(u32 *uart_tx, u32 *uart_baud);
extern void g_flash_dec_key_init(u16 key);
extern u32 get_update_jump_flag();
extern void ram_protect_close(void);
extern void ll_hci_destory(void);
extern u32 update_exif_addr_get(void);
extern void ex_cfg_start_addr_set(u32 addr);
extern void dynamic_mem_init(void *malloc_pool, int malloc_size);
extern void udelay(u32 nus);
extern void switch_to_hrc(void);
extern void clr_long_press_reset_timer(void);

extern u8 bz_flash_deal(u32 flash_id);
extern void uart_close_all(void);
extern u32 get_updata_ram_end_addr(void);
extern u8 *update_param_ext_get(UPDATA_PARM *p, u8 ext_type);
extern void update_enter_jump_maskrom(void);
//函数声明END

void ota_vlvd_sel(int lvd)
{
    P33_VLVD_OE(0);
    udelay(1);
    P33_VLVD_EN(0);
    VLVD_SEL(lvd);
    udelay(1);
    P33_VLVD_EN(1);
    P33_VLVD_OE(1);
}

#define PP_N  3//最多记录power_pin脚数
typedef struct  {
    u32 gpio[PP_N];
    u8 level[PP_N];
} PowerPin;
static PowerPin pp_handle = {0};

static u8 vlvd_reg = 0;

static u8 *latch_flag = NULL;
u8 jump_mode_check()
{
    if (latch_flag == NULL) {
        UPDATA_PARM *p;
        p = (u8 *)get_updata_ram_end_addr(); //定位到ram1最后地址
        latch_flag = update_param_ext_get(p, EXT_JUMP_FLAG);
    }
    if (latch_flag) {
        printf(">>>[test]:flag = %d\n", latch_flag[0]);
        return latch_flag[0];
    }
    printf(">>>[test]:latch_flag == NULL\n");
    return 0;
}

bool get_share_osc_en(void)
{
    UPDATA_PARM *p;
    u8 *share_osc_en = NULL;
    p = (u8 *)get_updata_ram_end_addr(); //定位到ram1最后地址
    share_osc_en = update_param_ext_get(p, EXT_SHARE_OSC_EN);
    if (share_osc_en == NULL) {
        return FALSE;
    }
    printf("%s %d\n", __func__, *share_osc_en);
    return (*share_osc_en == 1) ? TRUE : FALSE;
}

void chip_restart(void)
{
    /* 寄存器不会复位 */
#if (0 == UART_UPDATE_ONLY_TEST_MODE)
    u8 latch_reset = jump_mode_check();
    if (latch_reset == 0) {
        if ((latch_flag == NULL) && get_update_jump_flag()) {
            ram_protect_close();
#if(BT_UPDATA_MODULE_CONTROL == 1)
            puts("bredr_bd_close\n");
            bredr_bd_close();
#endif
#if((BT_UPDATA_MODULE_CONTROL  == 2) || BLE_GATT_UPDATA_MODULE_CONTROL)
            ll_hci_destory();
#endif
            u8 i = 0;
            local_irq_disable();
            for (i = 0; i < 64; i++) {
                irq_disable(i);
            }
            switch_to_hrc();
            uart_close_all();
            update_enter_jump_maskrom();
        }
    }
    /* if (get_update_jump_flag()) { */
    if (latch_reset) {
        puts("latch reset...\n");

        void latch_reset(void);
        latch_reset();
    } else
#endif
    {
        printf(">>>[test]:vlvd_reg = 0x%x\n", vlvd_reg);
        ota_vlvd_sel(vlvd_reg);
        uart_close_all();
        cpu_reset();
    }
}

/* extern u32 _text_begin; */
/* extern u32 _text_size; */
/* extern u32 ram_code_end; */
AT(.volatile_ram_code)
u32 get_osc_clk_freq(void)
{
#if UART_UPDATA_MODULE_CONTROL || UART_UPDATA_USER_MODULE_CONTROL
    return 24 * 1000 * 1000L;
#else
    return boot_sys_cfg.osc_freq;
#endif
}

//串口升级从maskrom跳转，其余升级从uboot跳转
void maskrom_uboot_info_store(void *pSDF)
{
#if !(UART_UPDATA_MODULE_CONTROL)
    memcpy((u8 *)&g_boot_info, (u8 *)pSDF, sizeof(struct boot_info_t));
    memcpy((u8 *)&boot_sys_cfg, (u8 *)(((struct boot_info_t *)pSDF)->sys_cfg), sizeof(struct SYS_CFG));
#else
    maskrom_uart_info_save((u8 *)pSDF);
#endif
}

extern void uart_update_set_gpio(u8 *gpio);
#if (defined(UPDATE_EFUSE) && (UPDATE_EFUSE == 1))
extern UPDATA_RESULT efuse_update(void);
extern u32 p33_read_page0(void);
#endif
extern void wdt_close(void);
void uboot_main(void *pSDF, u32 addr, u32 prevApp)
{
    u8 upadta_flag = 0;
    u32 uart_debug_io = 0, uart_debug_baud = 0;
    u32 pwr_gpio;
    u8  pwr_level;
    u32 p33_page0 = 0;
    APP_RUN_RES run_res;
    DEV_UPDATA_VARS updata_res;


    /* if (jump_mode_check()) { */
    /*     void latch_ulock(void); */
    /*     latch_ulock(); */
    /* } */

#if (defined(UPDATE_EFUSE) && (UPDATE_EFUSE == 1))  // 不确定其它芯片是否存在该问题

    wdt_close(); // 防止从sdk跳转到ota前没有关闭wdt，这里手动关闭，避免升级时出现复位
#endif
    uart_close_all();           //关闭所有串口，有些cpu reset之后寄存器不为0可能影响串口初始化
    /* if (jump_mode_check()) { */
    /*     LSB_CLK_DIV(0); */
    /* } */
    /* clk_out(IO_PORTA_03, LSB_CLK_OUT); */
    uart_init(UART_DEBUG_IO, NULL, UART_DEBUG_BAUD);
    bool  check_res = updata_check();


    /* printf(">>>[test]:CLOCK 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", JL_CLOCK->PWR_CON, JL_CLOCK->HTC_CON, JL_CLOCK->SYS_DIV, JL_CLOCK->CLK_CON0, JL_CLOCK->CLK_CON1, JL_CLOCK->CLK_CON2, JL_CLOCK->CLK_CON3, JL_CLOCK->CLK_CON4, JL_CLOCK->CLK_CON5); */

    /* printf(">>>[test]: PLL 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", JL_PLL->PLL_INTF0, JL_PLL->PLL_INTF1, JL_PLL->PLL_CON0, JL_PLL->PLL_CON1, JL_PLL->PLL_CON2, JL_PLL->PLL_CON3, JL_PLL->PLL_CON4); */

    vlvd_reg = p33_rx_1byte(P3_VLVD_CON);
    vlvd_reg &= BIT(3) | BIT(4) | BIT(5);
    vlvd_reg >>= 3;
    printf(">>>[test]:vlvd_reg = 0x%x\n", vlvd_reg);
#if defined(CONFIG_CPU_BD29)
    ota_vlvd_sel(VLVD_SEL_260V);
#else
    ota_vlvd_sel(VLVD_SEL_26V);
#endif

#if USB_HOST_MODULE_CONTROL
    VDDIOM_VOL_SEL(0x7); //操作usb io须提高vddio电压(20210127_weijiaba)
#endif

    sys_init(pSDF);


#if !(UART_UPDATA_MODULE_CONTROL ||UART_UPDATA_USER_MODULE_CONTROL)
    printf("pSDF:%x\n", pSDF);
    printf("osc_clk:%d uart_ux:%s rx%s\n", boot_sys_cfg.osc_freq, boot_sys_cfg.uttx, boot_sys_cfg.utrx);
#endif

    put_buf((u8 *)&g_boot_info, sizeof(struct boot_info_t));
    printf("osc_clk:%d uart_ux:%s rx%s\n", boot_sys_cfg.osc_freq, boot_sys_cfg.uttx, boot_sys_cfg.utrx);

#ifdef UART_DEBUG
    power_reset_source_dump();
#endif

    //put_buf((u8 *)boot_sys_cfg.reset_pin, sizeof(boot_sys_cfg.reset_pin));
    //long_press_reset_io_init(boot_sys_cfg.reset_pin[0], boot_sys_cfg.reset_pin[1] - '0');

#if (defined(UPDATE_EFUSE) && (UPDATE_EFUSE == 1))
    pll_init(0);
    p11_clk_init();  // 初始化p11时钟
#endif
    //<spi初始化检测，重新初始化SPI并提速
    spi_reinit();

#if defined(CONFIG_CPU_BR40)
#else
    void spi0flash_remove_protect(void);
    spi0flash_remove_protect();
#endif

#if BZ_FLASH_ENABLE
    u32 flash_id = get_flash_id();
    u8 ret = bz_flash_deal(flash_id);
    if (ret == 2) {
        printf("bz_flash_deal ok = %x\n", ret) ;
    } else if (ret == 1) {
        flash_id = -1;//紧用于报错
    }
#endif

    g_flash_dec_key_init(g_boot_info.chip_key);

    dynamic_mem_init(malloc_pool, malloc_pool_size);
#if FLASH_FRAMEWORK_VERSION_V2_EN && (0 == UART_UPDATE_ONLY_TEST_MODE) && (0 == BT_UPDATE_ONLY_TEST_MODE)
#if (defined(UPDATE_EFUSE) && (UPDATE_EFUSE == 1))
    // 因为pll_init会改变系统时钟,使得SPI时钟大于24M,导致个别芯片会读写SPI出错,所以将pll_init提前
    /* pll_init(0); */
    /* p11_clk_init();  // 初始化p11时钟 */

    p33_page0 = p33_read_page0();
    p33_page0 &= 0x00ff0000;
    /* put_buf(&p33_page0, 4); */
    // 判断是否需要走升级flash流程
    if (((JL_SYSTEM->CHIP_ID & (~0x7200)) && (get_flash_id() == 0xffffff)) || ((p33_page0 >> 16) & BIT(5))) { // 0x7200 br40 chip id; ((p33_page0 >> 16) & BIT(5))代码跑rom/flash
        // 只升级 efuse 不初始化flash
    } else
#endif
    {
        flash_fs_v2_update_init();

        get_uart_debug_info(&uart_debug_io, &uart_debug_baud);
        if (uart_debug_baud != 0) {     //当读取INI文件波特率不为0，根据ini文件的配置重新初始化串口
            printf("uart_reinit:%s %d\n", &uart_debug_io, uart_debug_baud);
#if (BZ_FLASH_ENABLE == 0)
            gpio_set_pull_up(get_gpio(UART_DEBUG_IO), 0);           //恢复output_channel对默认串口输出IO的占用
            gpio_set_pull_down(get_gpio(UART_DEBUG_IO), 0);
#endif
            uart_init((char *)&uart_debug_io, NULL, uart_debug_baud);
        }

#endif
    }
    puts("\n\n\n***********br23_ota_loader_app_ota "__DATE__ " " __TIME__"***********\n");
    deg_printf("addr ==  %08x prevApp == %x\n", addr, prevApp);


    wdt_init(WDT_8S);

#if BLE_GATT_UPDATA_MODULE_CONTROL || SPP_UPDATA_MODULE_CONTROL || USB_HID_MODULE_CONTROL
    ex_cfg_start_addr_set(update_exif_addr_get());
#endif

#if NET_UPDATA_MODULE_CONTROL /*在set_updata_result函数清除升级的地址前保存boot_info数据*/
    void boot_info_save(void *info);
    boot_info_save(&g_boot_info);
#endif

    set_updata_result(UPDATA_DEV_ERR);

    clr_long_press_reset_timer();

    sys_timer_var_init();
#if (defined(UPDATE_EFUSE) && (UPDATE_EFUSE == 1))
    if ((JL_SYSTEM->CHIP_ID & (~0x7200)) && (get_flash_id() == 0xffffff) || ((p33_page0 >> 16) & BIT(5))) { // 0x7200 is br40 chip id
        efuse_printf("updata_res_result:%x\n", updata_res.updata_result);
        updata_res.updata_result = 0;
        uart_update_set_gpio(uart_update_default_io);
        updata_res.updata_result = efuse_update();  // 没有flash直接升级efuse
        goto UPDAT_RESULT;
    }
#endif
    /* u8 ex_api_code_load(void); */
    /* ex_api_code_load(); */
    /* void debug_bredr_table(void); */
    /* debug_bredr_table(); */

    /*********************debug_function*****************/
    //debug_function
    /* extern void bt_updata_debug(void); */
    /* bt_updata_debug(); */

    /* extern void bt_updata_reinit(void); */
    /* bt_updata_reinit(); */
    /* set_updata_type(USB_UPDATA); */
    /* set_updata_type(SD1_UPDATA); */
    /*********************debug_function*****************/
#if NET_UPDATA_MODULE_CONTROL
    int wifi_init(void);
    wifi_init();
#endif

#if (SD_MODULE_CONTROL || USB_HID_MODULE_CONTROL || BLE_UPDATA_SUPPORT_CONNECT || USER_NORFLASH_UPDATA_MODULE_CONTROL || USER_LC_FLASH_UPDATA_MODULE_CONTROL || DEV_NORFLASH_UPDATA_MODULE_CONTROL || USB_HOST_MODULE_CONTROL )
    update_loader_ram_record_check();
    check_res = updata_check();
#elif (BT_UPDATA_MODULE_CONTROL && (defined(CONFIG_CPU_BR23) || defined(CONFIG_CPU_BR25))) //这里23\25需要读ldo trim和share_en
    update_loader_ram_record_check();
    updata_check(); //不修改check_res返回值，不影响bt_reinit流程;
#elif (UART_UPDATA_USER_MODULE_CONTROL)
    if (!check_res) {
        update_loader_ram_record_check();
    }
#endif

#if UART_UPDATA_MODULE_CONTROL && (0 == UART_UPDATE_ONLY_TEST_MODE)
    u8 reset_cfg[3];
    if (get_reset_cfg_from_isd_config(reset_cfg)) {
        put_buf(reset_cfg, sizeof(reset_cfg));
        reset_pin_init(reset_cfg[0], reset_cfg[1], reset_cfg[2]);
    }
#endif
#if (0 == UART_UPDATE_ONLY_TEST_MODE)
    if (get_power_pin_multi(&pp_handle.gpio[0], &pp_handle.level[0], PP_N)) {
        for (int i = 0; i < PP_N; i++) {
            printf(">>>[test]:gpio[%d] = %d\n", i, pp_handle.gpio[i]);
            if (pp_handle.gpio[i] == 0) {
                break;
            }
            gpio_set_direction(pp_handle.gpio[i], 0);
            gpio_set_die(pp_handle.gpio[i], 1);
            //先输出低是为了释放该io的电容的电，达到快速拉低目的,防止部分触摸ic掉电一半再上电状态异常;
            gpio_set_output_value(pp_handle.gpio[i], 0);
            udelay(100);
            gpio_set_output_value(pp_handle.gpio[i], pp_handle.level[i]);
            gpio_set_hd0(pp_handle.gpio[i], 1);
            gpio_set_hd(pp_handle.gpio[i], 1);
        }
    }
#endif
    while (1) {
        //step 1 : check parameter of updata is valid
#if (UART_UPDATA_MODULE_CONTROL || UART_UPDATA_USER_MODULE_CONTROL)
        u8 gpio[8] = {0};
#if (0 == UART_UPDATE_ONLY_TEST_MODE)
        u32 get_uart_update_pin(u8 * gpio);
        if (get_uart_update_pin(gpio)) {
            uart_update_set_gpio(gpio);
        } else
#endif
        {
            uart_update_set_gpio(uart_update_default_io);
        }
        deg_printf_buf((u8 *)UART_UPDATE_FLAG_ADDR, 10);

        if (TRUE) {
#else
        if (TRUE == check_res) {
#endif

            printf("updata_res_result:%x\n", updata_res.updata_result);
            updata_res.updata_result = 0;
            puts("***********check_updata_parm_ok***************\n");
            //check dev online and enter updata
            updata_mode(pSDF, &updata_res, addr);
UPDAT_RESULT:
            //result check
            if (DEV_UPDATA_SUCC == (updata_res.updata_result)) {
                deg_puts("***********run_app***************\n");
                set_updata_result(UPDATA_SUCCESSFULLY);
                chip_restart();
            } else if (updata_res.updata_result == DEV_UPDATA_KEY_ERR) {
                deg_puts("***********key_err***************\n");
                set_updata_result(UPDATA_KEY_ERR);
                chip_restart();
            } else {
                deg_printf("********updata_err:0x%x***********\n", updata_res.updata_result);
                set_updata_result(UPDATA_DEV_ERR);
                chip_restart();
            }
        } else {
            puts("********updata_nothine***********\n");
            updata_res.updata_result = DEV_POWER_BT_UPDATA;
            updata_mode(pSDF, &updata_res, addr);

            if (DEV_UPDATA_SUCC == (updata_res.updata_result)) {
                deg_puts("***********run_app***************\n");
                set_updata_result(UPDATA_SUCCESSFULLY);
                chip_restart();
            } else if (!updata_check_updata_result()) {
                set_updata_result(UPDATA_NON);
            }
        }

        //step 3 : run_app
        clr_updata_info();
        cpu_reset();
    }
}

