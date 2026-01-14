//*********************************************************************************//
// Module name : csfr.h                                                            //
// Description : q32DSP core sfr define                                            //
// By Designer : zequan_liu                                                        //
// Dat changed :                                                                   //
//*********************************************************************************//

#ifndef __Q32DSP_CSFR__
#define __Q32DSP_CSFR__


#define __RW      volatile       // read write
#define __RO      volatile const // only read
#define __WO      volatile       // only write

#undef __u32
#define __u32     unsigned int
#define map_adr(grp, adr)  ((64 * grp + adr) * 4)   // grp(0x0-0xff), adr(0x0-0x3f)

//*********************************************************************************
//
// hcore control sfr
//
//*********************************************************************************

#define csfr_base    0x1ee0000

//............. 0x0000 - 0x00ff............
typedef struct {
    __RO __u32(SOFT_INT);
    __WO __u32(SOFT_SET);
    __WO __u32(SOFT_CLR);
    __RW __u32(CON);
} JL_CMNG_TypeDef;

#define JL_CMNG_BASE                   (csfr_base + map_adr(0x00, 0x00))
#define JL_CMNG                        ((JL_CMNG_TypeDef *)JL_CMNG_BASE)

//............. 0x0100 - 0x01ff............
typedef struct {
    __RW __u32(CON);
    __RW __u32(KEY);
} JL_SDTAP_TypeDef;

#define JL_SDTAP_BASE                  (csfr_base + map_adr(0x01, 0x00))
#define JL_SDTAP                       ((JL_SDTAP_TypeDef *)JL_SDTAP_BASE)


//............. 0x0200 - 0x02ff............
typedef struct {
    __RW __u32(MBISTCTL);
    __RO __u32(MBISTSOGO);
} JL_MBIS_TypeDef;

#define JL_MBIS_BASE                   (csfr_base + map_adr(0x02, 0x00))
#define JL_MBIS                        ((JL_MBIS_TypeDef *)JL_MBIS_BASE)

//............. 0x0300 - 0x03ff............
typedef struct {
    __RW __u32(CON);
    __RW __u32(PAGE_END);     //page number - 1
} JL_HMEM_TypeDef;

#define JL_HMEM_BASE                    (csfr_base + map_adr(0x03, 0x00))
#define JL_HMEM                         ((JL_HMEM_TypeDef *)JL_HMEM_BASE)

//............. 0x0400 - 0x04ff............
typedef struct {
    __RW __u32(CON);
    __RW __u32(CADR);
    __RW __u32(ACC0L);
    __RW __u32(ACC0H);
    __RW __u32(ACC1L);
    __RW __u32(ACC1H);
    __RW __u32(CONST);
    __RW __u32(TEST1);
} JL_FFT_TypeDef;

#define JL_FFT_BASE                    (csfr_base + map_adr(0x04, 0x00))
#define JL_FFT                         ((JL_FFT_TypeDef *)JL_FFT_BASE)




//*********************************************************************************
//
// corex2 sfr
//
//*********************************************************************************

//---------------------------------------------//
// corex2 define
//---------------------------------------------//

#define corex2_sfr_offset   0x400
#define corex2_sfr_base    (csfr_base + 0xe000)
#define corex2(n)          ((JL_TypeDef_corex2     *)(corex2_sfr_base + corex2_sfr_offset*n))

typedef struct {
    /* 00 */  __RW __u32(C0_CON);
    /* 01 */  __RW __u32(C1_CON);
    /* 02 */  __RW __u32(CACHE_CON);
    /* 03 */  __RW __u32(DCACHE_WAY);
    /* 04 */  __RW __u32(ICACHE_WAY);
    /* 05 */  __RW __u32(RING_OSC);
    /*    */  __RO __u32(REV_80_3[0x80 - 0x5 - 1]);

    /* 80 */  __RW __u32(C0_IF_UACNTL);
    /* 81 */  __RW __u32(C0_IF_UACNTH);
    /* 82 */  __RW __u32(C0_RD_UACNTL);
    /* 83 */  __RW __u32(C0_RD_UACNTH);
    /* 84 */  __RW __u32(C0_WR_UACNTL);
    /* 85 */  __RW __u32(C0_WR_UACNTH);
    /* 86 */  __RW __u32(C0_TL_CKCNTL);
    /* 87 */  __RW __u32(C0_TL_CKCNTH);
    /* 88 */  __RW __u32(C1_IF_UACNTL);
    /* 89 */  __RW __u32(C1_IF_UACNTH);
    /* 8a */  __RW __u32(C1_RD_UACNTL);
    /* 8b */  __RW __u32(C1_RD_UACNTH);
    /* 8c */  __RW __u32(C1_WR_UACNTL);
    /* 8d */  __RW __u32(C1_WR_UACNTH);
    /* 8e */  __RW __u32(C1_TL_CKCNTL);
    /* 8f */  __RW __u32(C1_TL_CKCNTH);

    /* 90 */  __RW __u32(DBG_WR_EN);
    /* 91 */  __RO __u32(DBG_MSG);
    /* 92 */  __RW __u32(DBG_MSG_CLR);
    /*    */  __RO __u32(REV_9c_92[0xa0 - 0x92 - 1]);

    /* a0 */  __RW __u32(WR_LIMIT0_H);
    /* a1 */  __RW __u32(WR_LIMIT1_H);
    /* a2 */  __RW __u32(WR_LIMIT2_H);
    /* a3 */  __RW __u32(WR_LIMIT3_H);
    /* a4 */  __RW __u32(WR_LIMIT4_H);
    /* a5 */  __RW __u32(WR_LIMIT5_H);
    /* a6 */  __RW __u32(WR_LIMIT6_H);
    /* a7 */  __RW __u32(WR_LIMIT7_H);
    /*    */  __RO __u32(REV_b0_a7[0xb0 - 0xa7 - 1]);

    /* b0 */  __RW __u32(WR_LIMIT0_L);
    /* b1 */  __RW __u32(WR_LIMIT1_L);
    /* b2 */  __RW __u32(WR_LIMIT2_L);
    /* b3 */  __RW __u32(WR_LIMIT3_L);
    /* b4 */  __RW __u32(WR_LIMIT4_L);
    /* b5 */  __RW __u32(WR_LIMIT5_L);
    /* b6 */  __RW __u32(WR_LIMIT6_L);
    /* b7 */  __RW __u32(WR_LIMIT7_L);
    /*    */  __RO __u32(REV_c0_b7[0xc0 - 0xb7 - 1]);

    /* c0 */  __RW __u32(WR_ALLOW_ID0);
    /* c1 */  __RW __u32(WR_ALLOW_ID1);
    /* c2 */  __RW __u32(WR_ALLOW_ID2);
    /* c3 */  __RW __u32(WR_ALLOW_ID3);
    /* c4 */  __RW __u32(WR_ALLOW_ID4);
    /* c5 */  __RW __u32(WR_ALLOW_ID5);
    /* c6 */  __RW __u32(WR_ALLOW_ID6);
    /* c7 */  __RW __u32(WR_ALLOW_ID7);
    /*    */  __RO __u32(REV_d0_c7[0xd0 - 0xc7 - 1]);

    /* d0 */  __RW __u32(DBG_EN);
    /* d1 */  __RW __u32(DBG_CON);
    /* d2 */  __RW __u32(C0_WR_LIMIT_EN);
    /* d3 */  __RW __u32(C1_WR_LIMIT_EN);
    /* d4 */  __RW __u32(PRP_WR_LIMIT_EN);
    /* d5 */  __RW __u32(PRP_WR_ALLOW_EN);
    /* d6 */  __RW __u32(PRP_OUTLIM_ERR_EN);
    /*    */  __RO __u32(REV_d8_d5[0xd8 - 0xd6 - 1]);

    /* d8 */  __RO __u32(C0_WR_LIMIT_ERR_NUM);
    /* d9 */  __RO __u32(C1_WR_LIMIT_ERR_NUM);
    /* da */  __RO __u32(PRP_WR_LIMIT_ERR_NUM);
    /* db */  __RO __u32(PRP_WR_LIMIT_ID);
    /* dc */  __RO __u32(AXI_RD_INV_ID);
    /* dd */  __RO __u32(AXI_WR_INV_ID);
    /* de */  __RO __u32(PRP_MMU_ERR_WID);
    /* df */  __RO __u32(PRP_MMU_ERR_RID);
    /*    */  __RO __u32(REV_e0_de[0xe0 - 0xdf - 1]);

    /* e0 */  __RW __u32(PC_LIMIT0_H);
    /* e1 */  __RW __u32(PC_LIMIT0_L);
    /* e2 */  __RW __u32(PC_LIMIT1_H);
    /* e3 */  __RW __u32(PC_LIMIT1_L);

} JL_TypeDef_corex2;


//*********************************************************************************
//
// q32DSP_sfr
//
//*********************************************************************************

//---------------------------------------------//
// q32DSP define
//---------------------------------------------//

#define q32DSP_sfr_offset   0x200
#define q32DSP_sfr_base    (csfr_base + 0xf000)

#define q32DSP_cpu_base    (q32DSP_sfr_base + 0x00)
#define q32DSP_mpu_base    (q32DSP_sfr_base + 0x80)

#define q32DSP(n)          ((JL_TypeDef_q32DSP     *)(q32DSP_sfr_base + q32DSP_sfr_offset*n))
#define q32DSP_mpu(n)      ((JL_TypeDef_q32DSP_MPU *)(q32DSP_mpu_base + q32DSP_sfr_offset*n))

//---------------------------------------------//
// q32DSP core sfr
//---------------------------------------------//

typedef struct {
    /* 00 */  __RO __u32(DR00);
    /* 01 */  __RO __u32(DR01);
    /* 02 */  __RO __u32(DR02);
    /* 03 */  __RO __u32(DR03);
    /* 04 */  __RO __u32(DR04);
    /* 05 */  __RO __u32(DR05);
    /* 06 */  __RO __u32(DR06);
    /* 07 */  __RO __u32(DR07);
    /* 08 */  __RO __u32(DR08);
    /* 09 */  __RO __u32(DR09);
    /* 0a */  __RO __u32(DR10);
    /* 0b */  __RO __u32(DR11);
    /* 0c */  __RO __u32(DR12);
    /* 0d */  __RO __u32(DR13);
    /* 0e */  __RO __u32(DR14);
    /* 0f */  __RO __u32(DR15);

    /* 10 */  __RO __u32(RETI);
    /* 11 */  __RO __u32(RETE);
    /* 12 */  __RO __u32(RETX);
    /* 13 */  __RO __u32(RETS);
    /* 14 */  __RO __u32(SR04);
    /* 15 */  __RO __u32(PSR);
    /* 16 */  __RO __u32(CNUM);
    /* 17 */  __RO __u32(SR07);
    /* 18 */  __RO __u32(SR08);
    /* 19 */  __RO __u32(SR09);
    /* 1a */  __RO __u32(SR10);
    /* 1b */  __RO __u32(ICFG);
    /* 1c */  __RO __u32(USP);
    /* 1d */  __RO __u32(SSP);
    /* 1e */  __RO __u32(SP);
    /* 1f */  __RO __u32(PCRS);

    /* 20 */  __RW __u32(BPCON);
    /* 21 */  __RW __u32(BSP);
    /* 22 */  __RW __u32(BP0);
    /* 23 */  __RW __u32(BP1);
    /* 24 */  __RW __u32(BP2);
    /* 25 */  __RW __u32(BP3);
    /* 26 */  __WO __u32(CMD_PAUSE);
    /*    */  __RO __u32(REV_30_26[0x30 - 0x26 - 1]);

    /* 30 */  __RW __u32(PMU_CON);
    /*    */  __RO __u32(REV_34_30[0x34 - 0x30 - 1]);
    /* 34 */  __RW __u32(EMU_CON);
    /* 35 */  __RW __u32(EMU_MSG);
    /* 36 */  __RW __u32(EMU_SSP_H);
    /* 37 */  __RW __u32(EMU_SSP_L);
    /* 38 */  __RW __u32(EMU_USP_H);
    /* 39 */  __RW __u32(EMU_USP_L);
    /*    */  __RO __u32(REV_3b_39[0x3b - 0x39 - 1]);
    /* 3b */  __RW __u32(TTMR_CON);
    /* 3c */  __RW __u32(TTMR_CNT);
    /* 3d */  __RW __u32(TTMR_PRD);
    /* 3e */  __RW __u32(BANK_CON);
    /* 3f */  __RW __u32(BANK_NUM);

    /* 40 */  __RW __u32(ICFG00);
    /* 41 */  __RW __u32(ICFG01);
    /* 42 */  __RW __u32(ICFG02);
    /* 43 */  __RW __u32(ICFG03);
    /* 44 */  __RW __u32(ICFG04);
    /* 45 */  __RW __u32(ICFG05);
    /* 46 */  __RW __u32(ICFG06);
    /* 47 */  __RW __u32(ICFG07);
    /* 48 */  __RW __u32(ICFG08);
    /* 49 */  __RW __u32(ICFG09);
    /* 4a */  __RW __u32(ICFG10);
    /* 4b */  __RW __u32(ICFG11);
    /* 4c */  __RW __u32(ICFG12);
    /* 4d */  __RW __u32(ICFG13);
    /* 4e */  __RW __u32(ICFG14);
    /* 4f */  __RW __u32(ICFG15);

    /* 50 */  __RW __u32(ICFG16);
    /* 51 */  __RW __u32(ICFG17);
    /* 52 */  __RW __u32(ICFG18);
    /* 53 */  __RW __u32(ICFG19);
    /* 54 */  __RW __u32(ICFG20);
    /* 55 */  __RW __u32(ICFG21);
    /* 56 */  __RW __u32(ICFG22);
    /* 57 */  __RW __u32(ICFG23);
    /* 58 */  __RW __u32(ICFG24);
    /* 59 */  __RW __u32(ICFG25);
    /* 5a */  __RW __u32(ICFG26);
    /* 5b */  __RW __u32(ICFG27);
    /* 5c */  __RW __u32(ICFG28);
    /* 5d */  __RW __u32(ICFG29);
    /* 5e */  __RW __u32(ICFG30);
    /* 5f */  __RW __u32(ICFG31);

    /* 60 */  __RO __u32(IPND0);
    /* 61 */  __RO __u32(IPND1);
    /* 62 */  __RO __u32(IPND2);
    /* 63 */  __RO __u32(IPND3);
    /* 64 */  __RO __u32(IPND4);
    /* 65 */  __RO __u32(IPND5);
    /* 66 */  __RO __u32(IPND6);
    /* 67 */  __RO __u32(IPND7);
    /* 68 */  __WO __u32(ILAT_SET);
    /* 69 */  __WO __u32(ILAT_CLR);
    /* 6a */  __RW __u32(IPMASK);
    /*    */  __RO __u32(REV_70_6a[0x70 - 0x6a - 1]);

    /* 70 */  __RW __u32(ETM_CON);
    /* 71 */  __RO __u32(ETM_PC0);
    /* 72 */  __RO __u32(ETM_PC1);
    /* 73 */  __RO __u32(ETM_PC2);
    /* 74 */  __RO __u32(ETM_PC3);
    /* 75 */  __RW __u32(WP0_ADRH);
    /* 76 */  __RW __u32(WP0_ADRL);
    /* 77 */  __RW __u32(WP0_DATH);
    /* 78 */  __RW __u32(WP0_DATL);
    /* 79 */  __RW __u32(WP0_PC);
} JL_TypeDef_q32DSP;


#define TICK_CON                (q32DSP(0)->TTMR_CON)
#define TICK_PRD                (q32DSP(0)->TTMR_PRD)
#define TICK_CNT                (q32DSP(0)->TTMR_CNT)

#define SOFT_CLEAR_PENDING      (q32DSP(0)->ILAT_CLR)

#define CPU_MSG                 (q32DSP(0)->EMU_MSG)
#define CPU_CON                 (q32DSP(0)->EMU_CON)
#undef __u32
//*********************************************************************************//
//                                                                                 //
//                               end of this module                                //
//                                                                                 //
//*********************************************************************************//
#endif
