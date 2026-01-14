#ifndef __IIS_H__
#define __IIS_H__

#include "generic/typedef.h"


#define DIGITAL_VOL_CTRL        0		/*!< 是否采用数字音量缩放   */
#define MAX_IIS_VOL             31		/*!< 数字音量阶梯表最大阶梯数   */


#define IIS_PORTA          0
#define IIS_PORTC          1
#define IIS_PORTG          2


#define	MAX_IIS_NUM			2


struct iis_platform_data {
    u8 channel_in;
    u8 channel_out;
    u8 port_sel;
    u8 data_width;
    u8 mclk_output;
    u8 slave_mode;
    u8 update_edge;
    u8 f32e;
    u8 keep_alive;
    u8 sel;
    u16 dump_points_num;
    u16 sr_points;
};


void iis_channel_on(u8 channel, u8 index);

void iis_channel_off(u8 channel, u8 index);

int iis_open(struct iis_platform_data *pd, u8 index);

void iis_close(u8 index);

int iis_set_sample_rate(int sample_rate, u8 index);

void iis_set_enc_data_handler(void *priv, void (*cb)(void *, u8 *data, int len, u8), u8 index);

void iis_set_dec_data_handler(void *priv, void (*cb)(void *, u8 *data, int len, u8), u8 index);

void iis_irq_handler(u8 index);

void iis_set_vol(u8 vol, u8 index);

#endif

