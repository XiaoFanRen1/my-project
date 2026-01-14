#if defined CONFIG_DNS_ENC_ENABLE
        . = ALIGN(4);
        *(.jlsp_code)
        . = ALIGN(4);
        *(.jlsp_const)
        . = ALIGN(4);
        *(.dns_common_data)
        . = ALIGN(4);
        *(.dns_16k_data)
        . = ALIGN(4);
        *(.dns_8k_data)
        . = ALIGN(4);
        *(.dns_param_data_single)
        . = ALIGN(4);
        *(.dns_param_data_dual)
#endif

#if defined CONFIG_AEC_ENC_ENABLE
        . = ALIGN(4);
        *(.sms_code)
        . = ALIGN(4);
        *(.sms_sparse_code)
        . = ALIGN(4);
        *(.opcore_maskrom)
        . = ALIGN(4);
        *(.sms_const)
#endif

#if defined CONFIG_ADPCM_ENC_ENABLE
        . = ALIGN(4);
        *(.adpcm_encode_code)
        *(.adpcm_encode_sparse_code)
        *(.adpcm_encode_const)
        *(.adpcm_encode_sparse_const)
#endif

#if defined CONFIG_DTS_DEC_ENABLE
        . = ALIGN(4);
        *(.dts_dec_code)
        *(.dts_dec_const)
        *(.dts_dec_ff_const)
        *(.dts_dec_sparse_code)
        *(.dts_dec_sparse_const)
#endif

#if defined CONFIG_OPUS_DEC_ENABLE
        . = ALIGN(4);
        *(.wtgv2dec_code)
        *(.wtg_decv2_sparse_code)
        *(.wtgv2dec_const)
        *(.wtgv2dec_str)
#endif

#if defined CONFIG_MP3_DEC_ENABLE
        *(.mp3_dec_sparse_code)
        *(.mp3_dec_sparse_const)
        *(.mp3_dec_code)
        *(.mp3_dec_const)
#endif

#if TCFG_EQ_ENABLE
        . = ALIGN(4);
        *(.compressor_sparse_code)
        *(.compressor_sparse_const)
        *(.limiter_sparse_code)
        *(.limiter_sparse_const)
        *(.crossOver_sparse_code)
        *(.crossOver_sparse_const)
        *(.limiter_code)
        *(.limiter_const)
        *(.compressor_code)
        *(.compressor_const)
#endif

