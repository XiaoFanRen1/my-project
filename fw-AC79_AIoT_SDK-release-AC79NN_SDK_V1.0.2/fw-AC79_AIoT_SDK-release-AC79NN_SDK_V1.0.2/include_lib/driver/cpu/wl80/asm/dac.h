#ifndef __DAC_H__
#define __DAC_H__

#include "generic/typedef.h"

#define DAC_44_1KHZ            0
#define DAC_48KHZ              1
#define DAC_32KHZ              2
#define DAC_22_05KHZ           3
#define DAC_24KHZ              4
#define DAC_16KHZ              5
#define DAC_11_025KHZ          6
#define DAC_12KHZ              7
#define DAC_8KHZ               8


#define FADE_DISABLE            0
#define FADE_VOL_OUT            1
#define FADE_VOL_IN             2
#define FADE_VOL_UPDOWN         3

struct dac_platform_data {
    u8 ldo_id;
    u8 pa_auto_mute;
    u8 pa_mute_port;
    u8 pa_mute_value;
    u8 differ_output;
    u8 hw_channel;
    u8 ch_num;
    u8 fade_enable;
    u16 mute_delay_ms;
    u16 fade_delay_ms;
    u16 sr_points;
};

int dac_open(const struct dac_platform_data *pd);

int dac_on(void);

int dac_off(void);

int dac_close(void);

void dac_set_volume(s16 volume);

int dac_set_sample_rate(u32 sample_rate);

void dac_irq_handler(void);

void dac_set_data_handler(void *priv, void (*handler)(void *, u8 *data, int len));

void dac_set_gain(u8 gain, u8 fade_opt);

void dac_direct_set_gain(u8 gain);

void dac_fade_inout_set(u8 enable);

int audio_dac_read_single_channel(u16 *read_pos, s16 points_offset, void *data, int len);

#endif
