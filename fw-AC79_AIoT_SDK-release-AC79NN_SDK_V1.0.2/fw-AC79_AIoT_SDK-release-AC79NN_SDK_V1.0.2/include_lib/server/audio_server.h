#ifndef AUDIO_SERVER_H
#define AUDIO_SERVER_H

#include "fs/fs.h"
#include "server/audio_dev.h"

enum {
    AUDIO_SERVER_EVENT_CURR_TIME = 0x20,
    AUDIO_SERVER_EVENT_END,
    AUDIO_SERVER_EVENT_ERR,
    AUDIO_SERVER_EVENT_SPEAK_START,
    AUDIO_SERVER_EVENT_SPEAK_STOP,
};

enum {
    AUDIO_ATTR_REAL_TIME   = BIT(0),
    AUDIO_ATTR_LR_SUB      = BIT(1),
    AUDIO_ATTR_PS_EN       = BIT(2),
    AUDIO_ATTR_LR_ADD      = BIT(3),
    AUDIO_ATTR_DECRYPT_DEC = BIT(4),
    AUDIO_ATTR_FADE_INOUT  = BIT(5),
    AUDIO_ATTR_EQ_EN       = BIT(6),
    AUDIO_ATTR_DRC_EN      = BIT(7),
    AUDIO_ATTR_EQ32BIT_EN  = BIT(8),
    AUDIO_ATTR_BT_AAC_EN   = BIT(9),
};

#define AUDIO_REQ_DEC                  0
#define AUDIO_REQ_ENC                  1
#define AUDIO_REQ_GET_INFO             2
#define AUDIO_REQ_IOCTL                4
#define AUDIO_REQ_WIRELESS_SYNC        5

#define AUDIO_DEC_OPEN                 0
#define AUDIO_DEC_START                1
#define AUDIO_DEC_PAUSE                2
#define AUDIO_DEC_STOP                 3
#define AUDIO_DEC_FF                   4
#define AUDIO_DEC_FR                   5
#define AUDIO_DEC_GET_BREAKPOINT       6
#define AUDIO_DEC_PP                   7
#define AUDIO_DEC_SET_VOLUME           8
#define AUDIO_DEC_DIGITAL_GAIN_SET     9
#define AUDIO_DEC_PS_PARM_SET          10
#define AUDIO_DEC_GET_STATUS           11

#define AUDIO_ENC_OPEN                 4
#define AUDIO_ENC_START                5
#define AUDIO_ENC_PAUSE                6
#define AUDIO_ENC_STOP                 7
#define AUDIO_ENC_CLOSE                8
#define AUDIO_ENC_SET_VOLUME           9
#define AUDIO_ENC_GET_STATUS		   10
#define AUDIO_ENC_PP				   11


//dac输出模式
#define AUDIO_OUTPUT_ORIG_CH           0 //按原始声道输出
#define AUDIO_OUTPUT_STEREO            1 //按立体声
#define AUDIO_OUTPUT_L_CH              2 //只输出原始声道的左声道
#define AUDIO_OUTPUT_R_CH              3 //只输出原始声道的右声道
#define AUDIO_OUTPUT_MONO_LR_CH        4 //输出左右合成的单声道

struct audio_cbuf_t {
    void *cbuf;
    void *wr_sem;
    void *rd_sem;
    volatile int end;
};

struct stream_codec_info {
    int time;
    int frame_num;
    u32 frame_len;
    int frame_points;
    int sequence_number;
    u32 sample_rate;
    u8  channel;
};

struct audio_dec_breakpoint {
    int len;
    u32 fptr;
    u8 *data;	//ape格式断点最大2036字节
};

struct audio_finfo {
    u8 channel;
    u8 name_code; /* 0:ansi, 1:unicode_le, 2:unicode_be*/
    int sample_rate;
    int bit_rate;
    int total_time;
};

struct audio_ioctl {
    u32 cmd;
    void *priv;
};

/*
 * 指定位置播放
 * 设置后跳到start_time开始播放，
 * 播放到dest_time后如果callback_func存在，则调用callback_func，
 * 可以在callback_func回调中实现对应需要的动作
 */
struct audio_dest_time_play_param {
    u32 start_time;	// 要跳转过去播放的起始时间。单位：ms
    u32 dest_time;	// 要跳转过去播放的目标时间。单位：ms
    u32(*callback_func)(void *priv);	// 到达目标时间后回调
    void *callback_priv;	// 回调参数
};

struct audio_vfs_ops {
    void *(*fopen)(const char *path, const char *mode);
    int (*fread)(void *file, void *buf, u32 len);
    int (*fwrite)(void *file, void *buf, u32 len);
    int (*fseek)(void *file, u32 offset, int seek_mode);
    int (*ftell)(void *file);
    int (*flen)(void *file);
    int (*fclose)(void *file);
};

struct audio_dec_output_ops {
    void *(*get_buf)(void *priv, u32 *len);
    void (*put_buf)(void *priv, void *buf, u32 len);
    const struct audio_vfs_ops *vfs_ops;
};

struct audio_decoder_ops {
    const char *name;
    void *(*open)(FILE *file, void *priv,
                  const struct audio_dec_output_ops *ops, struct audio_dec_breakpoint *);
    void *(*try_open)(FILE *file, void *priv,
                      const struct audio_dec_output_ops *ops, struct audio_dec_breakpoint *);
    int (*get_audio_info)(void *, struct audio_finfo *info);
    int (*get_play_time)(void *);
    int (*fast_forward)(void *, int step_s);
    int (*fast_rewind)(void *, int step_s);
    int (*get_breakpoint)(void *, struct audio_dec_breakpoint *);
    int (*start)(void *);
    int (*close)(void *);
    int (*channel_switch)(void *, u8);
    int (*stream_info_scan)(void *, struct stream_codec_info *info, void *data, int len);
    int (*stop)(void *);
    int (*ioctl)(void *, u32 cmd, void *parm);
};

// 解码器命令
enum {
    AUDIO_IOCTRL_CMD_SET_BREAKPOINT_A = 0x08,	// 设置复读A点
    AUDIO_IOCTRL_CMD_SET_BREAKPOINT_B,			// 设置复读B点
    AUDIO_IOCTRL_CMD_SET_BREAKPOINT_MODE,		// 设置AB点复读模式

    AUDIO_IOCTRL_CMD_REPEAT_PLAY = 0x90,		// 设置循环播放
    AUDIO_IOCTRL_CMD_SET_DEC_SR = 0x91,         // 设置采样率或者码率
    AUDIO_IOCTRL_CMD_SET_DEST_PLAYPOS = 0x93,	// 设置指定位置播放
    AUDIO_IOCTRL_CMD_GET_PLAYPOS = 0x94,		// 获取毫秒级时间
};

struct audio_dec_req {
    u8 cmd;
    u8 status;
    u8 channel;
    u8 volume;
    u8 priority;
    u8 speedV; // >80是变快，<80是变慢，建议范围：30到130
    u16 pitchV;	// >32768是音调变高，<32768音调变低，建议范围20000到50000
    u16 attr;
    u8 digital_gain_mul;
    u8 digital_gain_div;
    u32 output_buf_len;
    u32 orig_sr;
    u32 force_sr;
    u32 sample_rate;
    u32 ff_fr_step;
    u32 total_time;
    u32 play_time;
    void *output_buf;
    FILE *file;
    const char *dec_type;
    const char *sample_source;
    struct audio_dec_breakpoint *bp;
    const struct audio_vfs_ops *vfs_ops;
    void *eq_attr;
    void *eq_hdl;
    struct audio_cbuf_t *virtual_audio;
    int (*dec_callback)(u8 *buf, u32 len, u32 sample_rate, u8 ch_num);
    int (*dec_sync)(void *priv, u32 data_size, u16 *in_rate, u16 *out_rate);
};

struct wireless_sync_ops {
    void *(*open)(void);
    void (*set_handler)(void *, void *priv, void (*handler)(void *, int *, int));
    u32(*time)(u8 type, u32 time);
    int (*send)(void *, void *buf, u32 len);
    int (*master)(u8 type);
    u8(*online)(u8);
    void (*role_lock)(void *, u8);
    void (*close)(void *);
};

struct audio_enc_req;

struct audio_package_ops {
    const char *name;
    const char *data_fmt;
    void *(*open)(void *file, const struct audio_vfs_ops *, struct audio_enc_req *);
    int (*start)(void *file, const struct audio_vfs_ops *vfs_ops);
    int (*input)(void *file, u8 *data, int len, const struct audio_vfs_ops *vfs_ops);
    int (*close)(void *file, const struct audio_vfs_ops *vfs_ops);
};

struct audio_enc_req {
    u8 cmd;
    u8 status;
    u8 channel;
    u8 channel_bit_map;
    u8 volume;
    u8 priority;
    u8 use_vad : 1;
    u8 vad_auto_refresh : 1;
    u8 direct2dac : 1;
    u8 high_gain : 1;
    u8 amr_src : 1;
    u8 aec_enable : 1;
    u8 ch_data_exchange : 1;
    u8 no_header : 1;

    u8 vir_data_wait : 1;
    u8 no_auto_start : 1;
    u8 sample_depth : 3;
    u8 dns_enable : 1;
    u8 reserve : 2;
    u16 vad_start_threshold;	//ms
    u16 vad_stop_threshold;		//ms
    u16 frame_size;
    u16 frame_head_reserve_len;
    u32 bitrate;
    u32 output_buf_len;
    u32 sample_rate;
    u32 msec;
    FILE *file;
    u8 *output_buf;
    const char *format;         //编码格式
    const char *sample_source;
    const struct audio_vfs_ops *vfs_ops;
    u32(*read_input)(u8 *buf, u32 len);
    void *aec_attr;
};

/*
 * 音频（无线）立体同步
 */
struct audio_wireless_sync {
    u8 cmd;
    u8 top_percent; //上限比例
    u8 start_percent; //启动比例限值
    u8 bottom_percent; //下限比例
    u8 protocol; //音频包含协议
    const struct wireless_sync_ops *ops;
};

union audio_req {
    struct audio_wireless_sync sync;
    struct audio_dec_req dec;
    struct audio_finfo info;
    struct audio_ioctl ioctl;
    struct audio_enc_req enc;
};


#define REGISTER_AUDIO_DECODER(ops) \
        const struct audio_decoder_ops ops SEC_USED(.audio_decoder)

extern const struct audio_decoder_ops audio_decoder_begin[];
extern const struct audio_decoder_ops audio_decoder_end[];

#define list_for_each_audio_decoder(p) \
    for (p = audio_decoder_begin; p < audio_decoder_end; p++)



#define REGISTER_AUDIO_PACKAGE(ops) \
        const struct audio_package_ops ops SEC_USED(.audio_package)

extern const struct audio_package_ops audio_package_begin[];
extern const struct audio_package_ops audio_package_end[];

#define list_for_each_audio_package(p) \
    for (p = audio_package_begin; p < audio_package_end; p++)


#endif
