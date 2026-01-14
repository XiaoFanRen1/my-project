//#include "app_config.h"
//#include "system/includes.h"
//#include "asm/pwm.h"
//#include "device/device.h"
//
///*********************************PWM_LED设备例子****************************************************
//  注意：初始化之后，只支持单通道选择
//  PWM通过dev_write写，返回值为成功0，失败非0；
//  PWM通过dev_read读，返回值为通道的占空比值(0-100%)；
//  PWM通过dev_ioctl控制，形参arg为PWM通道；
//************************************************************************************************/
//static void pwm_led_test(void)
//{
//    int ret;
//    u32 duty;
//    u32 channel;
//	void *pwm_dev_handl = NULL;
//    struct pwm_platform_data pwm = {0};
//
//    /*1.open ，open之后的PWM通道最多支持同一个通道的H和L设置，不能进行多通道的设置*/
//    pwm_dev_handl = dev_open("pwm1", &pwm);//打开PWM1设备，传参可以获取board.c配置的参数
//    if (!pwm_dev_handl) {
//        printf("open pwm err !!!\n\n");
//        return ;
//    }
//    printf("pwm : ch=0x%x,duty=%2f%%,pbit=%d,freq=%dHz\n", pwm.pwm_ch, pwm.duty, pwm.point_bit, pwm.freq);
//    os_time_dly(200);
///*open PWM设备之后就会初始化PWM，PWM相关参数为board.c配置，在不需要更改参数时，只需要open就行，不需要进行以下操作*/
//    #if 0
//    /*write and read 配置占空比*/
//
//    pwm.pwm_ch = PWMCH0_H | PWMCH0_L;//配置通道
//    pwm.duty = 80;//配置占空比
//    dev_write(pwm_dev_handl, (void *)&pwm, 0);
//    ret = dev_read(pwm_dev_handl, (void *)&pwm, 0);//dev_read函数返回值为占空比（不带小数点）0-100
//    printf("pwm0 read duty : %d \n", ret);
//    os_time_dly(200);
//#endif
//
//     /*2.ioctl配置占空比*/
//    int i;
//	while(1)
//	{
//		//值范围：0~100  灯渐亮
//		for( i = 0; i<= 100 ; i++)
//		{
//		    pwm.duty = i;
//            dev_ioctl(pwm_dev_handl, PWM_SET_DUTY, (u32)&pwm);//设置占空比
//            os_time_dly(10);
//
//		}
//		//灯渐灭
//		for( i = 100; i>= 0 ; i--)
//		{
//			pwm.duty = i;
//            dev_ioctl(pwm_dev_handl, PWM_SET_DUTY, (u32)&pwm);//设置占空比
//            os_time_dly(10);
//		}
//	}
//
//}
//static int c_main(void)
//{
//    os_task_create(pwm_led_test, NULL, 12, 1000, 0, "pwm_led_test");
//    return 0;
//}
//late_initcall(pwm_led_test);
