#ifndef __PLNK_H__
#define __PLNK_H__

#include "generic/typedef.h"

/*! \addtogroup PLNK
 *  @ingroup HAL
 *  @brief	PLNK device api
 *  @{
 */

#define 	MAX_PLNK_NUM				2

/**
 * \name 输入通道选择
 * \{
 */
#define PLNK_MIC_MASK					(BIT(0) | BIT(1))
#define PLNK_CH_MIC_L					BIT(0)	//两个MIC共用DAT0也使用该宏
#define PLNK_CH_MIC_R					BIT(1)  //两个MIC共用DAT1也使用该宏
#define PLNK_CH_MIC_DOUBLE				(BIT(0) | BIT(1))
/* \} name */



struct plnk_platform_data {
    void (*port_remap_cb)(void);
    void (*port_unremap_cb)(void);
    u8 hw_channel;
    u8 clk_out;
    u8 high_gain;
    u8 sample_edge;
    u16 sr_points;
    u8 share_data_io;
    u8 dc_cancelling_filter;
    u32 dump_points_num;
};


/**
 * @brief 初始化PLNK
 *
 * @param sample_rate 采样率
 * @param channel 输入通道
 *
 * @return 0: 成功
 * @return -1: 失败
 */
int plnk_open(const struct plnk_platform_data *pd, u8 index);

/**
 * @brief 启动PLNK
 *
 * @return 0: 成功
 * @return -1: 失败
 */
int plnk_start(u8 index);

/**
 * @brief 停止PLNK
 *
 * @return 0: 成功
 * @return -1: 失败
 */
int plnk_stop(u8 index);

/**
 * @brief 关闭PLNK
 *
 * @return 0: 成功
 * @return -1: 失败
 */
int plnk_close(u8 index);

/**
 * @brief 设置PLNK采样率
 *
 * @param sample_rate 采样率
 *
 * @return 0: 成功
 * @return -1: 失败
 */
int plnk_set_sample_rate(int sample_rate, u8 index);

/**
 * @brief 设置PLNK中断处理函数中的回调函数，用于处理采集到的数据
 *
 * @param priv The pointer to the private data of the callback function
 * @param cb 回调函数指针
 */
void plnk_set_handler_cb(void *priv, void (*cb)(void *, u8 *data, int len), u8 index);

/**
 * @brief PLNK中断处理函数
 * @note  该函数用于注册中断
 */
void plnk_irq_handler(u8 index);

/*! @}*/


#endif

