#include "logic_include.h"
#include "usb/device/usb_stack.h"
#include "usb/device/custom_hid.h"
#include "usb_common_def.h"
#include "usb_std_class_def.h"

extern void wdt_clear();
static u8 test_buf[64];
static u8 cntt = 1;
void usb_start_1()
{
    printf("youbaibai usb_start\n");
    usb_device_mode(0, CUSTOM_HID_CLASS);
}

static void timer_send()
{
    printf("youbaibai %s\n", __func__);
    memset(test_buf, cntt, cntt);
    custom_hid_tx_data(0, test_buf, cntt);
    cntt++;
    if (cntt > 64) {
        cntt = 0;
    }
}

static void custom_hid_rx_handler(void *priv, u8 *buf, u32 len)
{
    printf("%s\n", __func__);
    put_buf(buf, len);
}
void usb_custom_hid_test()
{
    printf("youbaibai usb_custom_hid_test\n");

    u32 cnt = 0;
    usb_start_1();

    custom_hid_set_rx_hook(NULL, custom_hid_rx_handler);




    while (1) {
        cnt ++;
        if (cnt == 10000000) {
            cnt = 0;
            putchar('o');
            wdt_clear();

            timer_send();
        }
    }
}

void usb_delay(void)
{
    u32 cnt = 0;
    cnt ++;
    if (cnt == 10000000) {
        cnt = 0;
        custom_hid_tx_data(0, NULL, 0);
    }
}
/**
 * @brief  用户自定义usb升级设备控制入口
 * @param: none
 * @return:见UPDATA_RESULT(uboot/include_lib/lib_include.h)
 **/
u32 user_usb_update_control(void)
{
    puts("user_usb_update_control\n");
    /* usb_custom_hid_test(); //hid测试用 */
    return 1;
}
