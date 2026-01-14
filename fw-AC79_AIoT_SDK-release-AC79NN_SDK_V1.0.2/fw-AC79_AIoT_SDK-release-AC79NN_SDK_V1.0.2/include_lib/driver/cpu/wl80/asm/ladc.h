#ifndef __LADC_H__
#define __LADC_H__

#include "generic/typedef.h"



/* ADC采样率设置 */
#define ADC_SAMPRATE_44_1KHZ           0
#define ADC_SAMPRATE_48KHZ             1
#define ADC_SAMPRATE_32KHZ             2
#define ADC_SAMPRATE_22_05KHZ          3
#define ADC_SAMPRATE_24KHZ             4
#define ADC_SAMPRATE_16KHZ             5
#define ADC_SAMPRATE_11_025KHZ         6
#define ADC_SAMPRATE_12KHZ             7
#define ADC_SAMPRATE_8KHZ              8


/* MIC通道选择 */										  // N   P
#define LADC_CH_MIC0_P  				BIT(0)			  //PA1 PA2
#define LADC_CH_MIC0_N  				BIT(1)			  //PA1 PA2
#define LADC_CH_MIC0_P_N  				(BIT(0) | BIT(1)) //PA1 PA2
#define LADC_CH_MIC1_P  				BIT(2)			  //PH8 PH9
#define LADC_CH_MIC1_N  				BIT(3)			  //PH8 PH9
#define LADC_CH_MIC1_P_N  				(BIT(2) | BIT(3)) //PH8 PH9
#define LADC_CH_MIC2_P  				BIT(4)			  //PA4 PA3
#define LADC_CH_MIC2_N  				BIT(5)			  //PA4 PA3
#define LADC_CH_MIC2_P_N  				(BIT(4) | BIT(5)) //PA4 PA3
#define LADC_CH_MIC3_P  				BIT(6)			  //PH5 PH6
#define LADC_CH_MIC3_N  				BIT(7)			  //PH5 PH6
#define LADC_CH_MIC3_P_N  				(BIT(6) | BIT(7)) //PH5 PH6

/* LINEIN通道选择 */
#define LADC_CH_AUX0  					BIT(0)	  //PA0
#define LADC_CH_AUX1  					BIT(1)	  //PH7
#define LADC_CH_AUX2  					BIT(2)	  //PA5
#define LADC_CH_AUX3  					BIT(3)	  //PH4

struct adc_platform_data {
    u8 mic_channel;
    u8 linein_channel;
    u8 mic_ch_num;
    u8 linein_ch_num;
    u8 fake_channel;
    u8 fake_ch_num;
    u8 isel;
    u8 all_channel_open;
    u16 sr_points;
};

void adc_multiplex_set_gain(const char *source, u8 channel_bit_map, u8 gain);

void adc_multiplex_open(const char *source, u8 channel_bit_map);

int adc_open(const char *source, struct adc_platform_data *pd);

int adc_set_sample_rate(int sample_rate);

int adc_start(const char *source);

void adc_irq_handler(void);

int adc_stop(void);

int adc_close(void);

void adc_set_data_handler(void *priv, void (*handler)(void *, u8 *data, int len));

int adc_set_gain(const char *source, u8 gain);

void linein_to_fdac(u8 enable_dacl, u8 enable_dacr);

void mic_to_fdac(u8 channel_bit_map, u8 enable_dacl, u8 enable_dacr);

void adc_early_init(struct adc_platform_data *pd);

void vcm_early_init(u32 delay_ms);

void linein_to_fdac_high_gain(u8 enable);

#endif










