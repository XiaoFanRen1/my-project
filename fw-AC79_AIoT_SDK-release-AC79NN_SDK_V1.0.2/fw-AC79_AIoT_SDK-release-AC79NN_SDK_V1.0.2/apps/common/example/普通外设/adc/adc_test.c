#include "generic/typedef.h"
#include "asm/clock.h"
#include "asm/adc_api.h"
#include "system/timer.h"
#include "asm/efuse.h"
#include "asm/p33.h"
#include "asm/power_interface.h"
#include "device/gpio.h"
#include "system/includes.h"
#include "app_config.h"

void c_main(void)
{

    /*测量AD通道*/
    gpio_set_die(IO_PORTA_07, 0);//关闭数字输入
    gpio_set_direction(IO_PORTA_07, 1);//将PA07通道引脚设为输入模式
    gpio_set_pull_down(IO_PORTA_07, 0);//取消下拉配置
    gpio_set_pull_up(IO_PORTA_07, 0);//取消上拉配置

    u16 i,j;
    adc_add_sample_ch(AD_CH_PA07);//添加通道

    os_time_dly(100);//延时

    i=adc_get_value(AD_CH_PA07);//测PA7引脚adc值
    printf("test_value = %d\n", i);

//    j=adc_get_voltage(AD_CH_PA07);//
//    printf("test_voltage_value = %d\n", j);

}
late_initcall(c_main);
