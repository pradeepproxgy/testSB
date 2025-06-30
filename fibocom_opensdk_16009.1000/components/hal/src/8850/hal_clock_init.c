/* Copyright (C) 2018 RDA Technologies Limited and/or its affiliates("RDA").
 * All rights reserved.
 *
 * This software is supplied "AS IS" without any warranties.
 * RDA assumes no responsibility or liability for the use of the software,
 * conveys no license or title under any patent, copyright, or mask work
 * right to the product. RDA reserves the right to make changes in the
 * software without notification.  RDA also make no representation or
 * warranty that such application will be suitable for the specified use
 * without further testing or modification.
 */

#include "hwregs.h"
#include "hal_chip.h"
#include "hal_ram_cfg.h"
#include "hal_hwspinlock.h"
#include "drv_config.h"
#include "osi_api.h"
//#include "connectivity_config.h"
#include "osi_tick_unit.h"
#include "hal_config.h"
#include "openCPU_config.h"

#define MR0 (0)
#define MR1 (1)
#define MR2 (2)
#define MR3 (3)
#define MR4 (4)
#define MR5 (5)
#define MR6 (6)
#define MR7 (7)
#define MR8 (8)
#define DMC_MR_WRITE_CMD(mr, val) (DMC400_DIRECT_CMD_MRS | ((val) << 8) | (mr))
#define DMC_MR_READ_CMD(mr, val) (DMC400_DIRECT_CMD_MRR | (mr))

#define QOS_AON 1
#define QOS_AP_AXIDMA 1
#define QOS_EMMC 1
#define QOS_LZMA 1
#define QOS_APA5 2
#define QOS_CE 2
#define QOS_USB 2
#define QOS_MED 2
#define QOS_CPIFC 3
#define QOS_APIFC 3
#define QOS_GOUDA 3
#define QOS_GNSS 4
#define QOS_CP_AXIDMA 5
#define QOS_F8 6
#define QOS_CPA5 7
#define WRITE_PRIO 3
#define READ_FILL_PRIO 3

extern void halPmuWakeFromDeep(void); //patch by SPCSS01010144

#ifdef __PSRAM_AUTO__
#define HAL_EFUSE_SIGNLE_BLOCK_PRODUCT_TYPE_SECTION (189)
#define PSRAM_CONFIG_SHIFT (7)
#define PSRAM_CONFIG_MASK (0xf << PSRAM_CONFIG_SHIFT)

#define PSRAM_TYPE_REG (hwp_idleLps->lps_res5)
#define PSRAM_FREQ_REG (hwp_idleLps->lps_res4)

extern void halEfuseOpen(void);
extern bool halEfuseSingleRead(uint32_t block_index, uint32_t *val);
extern void halEfuseClose(void);

uint8_t prvGetPsramConfig(void)
{
    uint32_t PsramConfig = 0;
    uint8_t PsramCheckID = DEFAULT_PROD_FLAG;

    halEfuseOpen();
    bool ReadStatus = halEfuseSingleRead(HAL_EFUSE_SIGNLE_BLOCK_PRODUCT_TYPE_SECTION, &PsramConfig);
    halEfuseClose();

    if (ReadStatus)
    {
        PsramConfig = (PsramConfig & PSRAM_CONFIG_MASK) >> PSRAM_CONFIG_SHIFT;
        switch (PsramConfig)
        {
        case 0:
#ifdef CONFIG_8850BM_C1
            PsramCheckID = WIN955_PROD_FLAG;
#else
            PsramCheckID = DEFAULT_PROD_FLAG;
#endif
            break;
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
            PsramCheckID = PsramConfig;
            break;
        default:
#ifdef CONFIG_8850BM_C1
            PsramCheckID = WIN955_PROD_FLAG;
#else
            PsramCheckID = DEFAULT_PROD_FLAG;
#endif
            break;
        }
    }
    else
#ifdef CONFIG_8850BM_C1
        PsramCheckID = WIN955_PROD_FLAG;
#else
        PsramCheckID = DEFAULT_PROD_FLAG;
#endif
    return PsramCheckID;
}
#endif

static void prvDelayUS(uint32_t us)
{
    uint32_t start = halTimerTick32();
    uint32_t ticks = OSI_US_TO_HWTICK_U32(us);
    OSI_LOOP_WAIT(((unsigned)(halTimerTick32() - start)) > ticks);
}

static void prvDmc400QosConfig(void)
{
    REG_AP_APB_CFG_QOS0_T cfg_qos_apa5_ce_emmc_lzma = {
        .b.ap_a5_arqos = QOS_APA5,
        .b.ap_a5_awqos = QOS_APA5,
        .b.ce_arqos = QOS_CE,
        .b.ce_awqos = QOS_CE,
        .b.emmc_arqos = QOS_EMMC,
        .b.emmc_awqos = QOS_EMMC,
        .b.lzma_arqos = QOS_LZMA,
        .b.lzma_awqos = QOS_LZMA,
    };
    hwp_apApb->cfg_qos0 = cfg_qos_apa5_ce_emmc_lzma.v;

    REG_AP_APB_CFG_QOS1_T cfg_gd_usb_ce_apifc_aon = {
        .b.gouda_arqos = QOS_GOUDA,
        .b.gouda_awqos = QOS_GOUDA,
        .b.usb_awqos = QOS_USB,
        .b.usb_awqos = QOS_USB,
        .b.ap_ifc_arqos = QOS_APIFC,
        .b.ap_ifc_awqos = QOS_APIFC,
    };
    hwp_apApb->cfg_qos1 = cfg_gd_usb_ce_apifc_aon.v;

    REG_AP_APB_CFG_QOS2_T cfg_med_apaxidma = {
        .b.med_arqos = QOS_MED,
        .b.med_awqos = QOS_MED,
        .b.ap_axidma_arqos = QOS_AP_AXIDMA,
        .b.ap_axidma_awqos = QOS_AP_AXIDMA,
    };
    hwp_apApb->cfg_qos2 = cfg_med_apaxidma.v;
#if 0
    REG_CP_GLB_SYSCTRL01_T cfg_cpa5_f8_cpaxidma = {
        .b.awqos_cp_a5 = QOS_CPA5,
        .b.arqos_cp_a5 = QOS_CPA5,
        .b.awqos_f8 = QOS_F8,
        .b.arqos_f8 = QOS_F8,
        .b.awqos_axidma = QOS_CP_AXIDMA,
        .b.arqos_axidma = QOS_CP_AXIDMA,
        .b.awqos_cp_ifc = QOS_CPIFC,
        .b.arqos_cp_ifc = QOS_CPIFC,
    };
    hwp_cpGlb->sysctrl01 = cfg_cpa5_f8_cpaxidma.v;
#endif
    REG_SYS_CTRL_CFG_AON_QOS_T aon_qos = {
        .b.awqos_aon = QOS_AON,
        .b.arqos_aon = QOS_AON,
    };
    hwp_sysCtrl->cfg_aon_qos = aon_qos.v;
#if 0
    REG_GNSS_SYS_GNSS_AXI_QOS_T gnss_qos = {
        .b.gnss_arqos = QOS_GNSS,
        .b.gnss_awqos = QOS_GNSS,
    };
    hwp_gnssSys->gnss_axi_qos = gnss_qos.v;
#endif
    hwp_dmcCtrl->qos0_control = 0xf00;
    hwp_dmcCtrl->qos1_control = 0xe01;
    hwp_dmcCtrl->qos2_control = 0xd02;
    hwp_dmcCtrl->qos3_control = 0xc03;
    hwp_dmcCtrl->qos4_control = 0xb04;
    hwp_dmcCtrl->qos5_control = 0xa05;
    hwp_dmcCtrl->qos6_control = 0x906;
    hwp_dmcCtrl->qos7_control = 0x807;
    hwp_dmcCtrl->qos8_control = 0x708;
    hwp_dmcCtrl->qos9_control = 0x608;
    hwp_dmcCtrl->qos10_control = 0x50a;
    hwp_dmcCtrl->qos11_control = 0x40b;
    hwp_dmcCtrl->qos12_control = 0x30c;
    hwp_dmcCtrl->qos13_control = 0x20d;
    hwp_dmcCtrl->qos14_control = 0x10e;
    hwp_dmcCtrl->qos15_control = 0x00f;

    REG_DMC400_WRITE_PRIORITY_CONTROL_T write_priority_control = {
        .b.write_threshold_en = 1,
        .b.write_fill_priority_1_16ths = 0,
        .b.write_fill_priority_2_16ths = 0,
        .b.write_fill_priority_3_16ths = 0,
        .b.write_fill_priority_4_16ths = 0,
        .b.write_fill_priority_5_16ths = 0,
        .b.write_fill_priority_6_16ths = 0,
        .b.write_fill_priority_7_16ths = 0,
    };
    hwp_dmcCtrl->write_priority_control = write_priority_control.v;

    REG_DMC400_READ_PRIORITY_CONTROL_T read_priority_control = {
        .b.read_escalation = 1,
        .b.read_in_burst_prioritisation = 1,
        .b.read_fill_priority_1_16ths = 0,
        .b.read_fill_priority_2_16ths = 0,
        .b.read_fill_priority_3_16ths = 0,
        .b.read_fill_priority_4_16ths = 0,
        .b.read_fill_priority_5_16ths = 0,
        .b.read_fill_priority_6_16ths = 0,
        .b.read_fill_priority_7_16ths = 0,
    };
    hwp_dmcCtrl->read_priority_control = read_priority_control.v;
}

static void pll_cpst0(void)
{
    unsigned int rdata;
    unsigned int locked = 0;
    unsigned int dll_cnt = 0;
    int i = 0;
    int m;

    hwp_psramPhy->psram_drf_train_cfg = 0x1;
    hwp_psramPhy->psram_drf_cfg_dll_mode_f0 = 0x2;
    hwp_psramPhy->psram_rf_cfg_dll_ads0 = PSRAM_PHY_RF_DLL_LOCK_WAIT_ADS0(10) | PSRAM_PHY_RF_DLL_PD_CNT_ADS0(5) | PSRAM_PHY_RF_DL_CPST_THR_ADS0(8) | PSRAM_PHY_RF_DLL_EN_ADS0 | PSRAM_PHY_RF_DL_CPST_AUTO_REF_EN_ADS0 | PSRAM_PHY_RF_DL_CPST_EN_ADS0;
    hwp_psramPhy->psram_rf_cfg_dll_ads1 = 0x400;
    while ((locked == 0) && (dll_cnt < 0xff) && (i < 1000))
    {
        for (m = 0; m < 20; m++)
        {
            __asm__("nop");
        }
        rdata = hwp_psramPhy->psram_rfdll_status_dll_ads0;
        locked = rdata & PSRAM_PHY_RFDLL_LOCKED_ADS0;
        dll_cnt = rdata & PSRAM_PHY_RFDLL_CNT_ADS0(0xff);
        i++;
    }
    if (locked == PSRAM_PHY_RFDLL_LOCKED_ADS0)
    {
        //rprintf("PSRAM adslice0 DLL locked!" );
    }
    else if (dll_cnt == 0xff)
    {
        //rprintf("PSRAM adslice0 DLL saturated!" );
    }
    else
    {
        //rprintf("PSRAM adslice0 DLL saturated!" );
    }
    prvDelayUS(5);
    hwp_psramPhy->psram_rf_cfg_dll_ads0 = 0x0;
}

static void pll_cpst1(void)
{
    unsigned int rdata;
    unsigned int locked = 0;
    unsigned int dll_cnt = 0;
    int i = 0;
    int m;

    hwp_psramPhy->psram_drf_train_cfg = 0x1;
    hwp_psramPhy->psram_drf_cfg_dll_mode_f0 = 0x2;
    hwp_psramPhy->psram_rf_cfg_dll_ads1 = PSRAM_PHY_RF_DLL_LOCK_WAIT_ADS1(10) | PSRAM_PHY_RF_DLL_PD_CNT_ADS1(5) | PSRAM_PHY_RF_DL_CPST_THR_ADS1(8) | PSRAM_PHY_RF_DLL_EN_ADS1 | PSRAM_PHY_RF_DL_CPST_AUTO_REF_EN_ADS1 | PSRAM_PHY_RF_DL_CPST_EN_ADS1;
    while ((locked == 0) && (dll_cnt < 0xff) && (i < 1000))
    {
        for (m = 0; m < 20; m++)
        {
            __asm__("nop");
        }
        rdata = hwp_psramPhy->psram_rfdll_status_dll_ads1;
        locked = rdata & PSRAM_PHY_RFDLL_LOCKED_ADS1;
        dll_cnt = rdata & PSRAM_PHY_RFDLL_CNT_ADS1(0xff);
        i++;
    }
    if (locked == PSRAM_PHY_RFDLL_LOCKED_ADS1)
    {
        //rprintf("PSRAM adslice1 DLL locked!" );
    }
    else if (dll_cnt == 0xff)
    {
        //rprintf("PSRAM adslice1 DLL saturated!" );
    }
    else
    {
        //rprintf("PSRAM adslice1 DLL saturated!" );
    }
    prvDelayUS(5);
    hwp_psramPhy->psram_rf_cfg_dll_ads1 = 0x0;
}

/**
 * PSRAM exit half sleep, a low pulse of CE with width tXHPS, and wait tXHS.
 */
static inline void prvPsramExitHalfSleep(void)
{
    // select iomux for software
    hwp_psramPhy->psram_rf_cfg_phy_iomux_oe_wr_ads0 = 0x00110000;
    hwp_psramPhy->psram_rf_cfg_phy_iomux_oe_wr_ads1 = 0x00110000;
    hwp_psramPhy->psram_rf_cfg_phy_iomux_out_wr_ads0 = 0x00100000;
    hwp_psramPhy->psram_rf_cfg_phy_iomux_out_wr_ads1 = 0x00100000;
    hwp_psramPhy->psram_rf_cfg_phy_iomux_sel_wr_ads0 = 0x00110000;
    hwp_psramPhy->psram_rf_cfg_phy_iomux_sel_wr_ads1 = 0x00110000;

    // exit half sleep
    hwp_psramPhy->psram_rf_cfg_phy_iomux_out_wr_ads0 = 0x00000000;
    hwp_psramPhy->psram_rf_cfg_phy_iomux_out_wr_ads1 = 0x00000000;

    prvDelayUS(4); // tXPHS

    // release clock
    hwp_psramPhy->psram_rf_cfg_phy_iomux_out_wr_ads0 = 0x00100000;
    hwp_psramPhy->psram_rf_cfg_phy_iomux_out_wr_ads1 = 0x00100000;

    prvDelayUS(150); // tXHS

    // select iomux for hardware
    hwp_psramPhy->psram_rf_cfg_phy_iomux_sel_wr_ads0 = 0x00000000;
    hwp_psramPhy->psram_rf_cfg_phy_iomux_sel_wr_ads1 = 0x00000000;
    hwp_psramPhy->psram_rf_cfg_phy_iomux_oe_wr_ads0 = 0x00000000;
    hwp_psramPhy->psram_rf_cfg_phy_iomux_oe_wr_ads1 = 0x00000000;
    hwp_psramPhy->psram_rf_cfg_phy_iomux_out_wr_ads0 = 0x00000000;
    hwp_psramPhy->psram_rf_cfg_phy_iomux_out_wr_ads1 = 0x00000000;

    prvDelayUS(2);
}

static void prvFlashClkInit(void)
{
    REG_AP_CLK_CGM_SPIFLASH1_SEL_CFG_T CFG = {0};
    REG_AP_APB_CLK_AP_EN0_T clken = {hwp_apApb->clk_ap_en0};
    REG_AP_APB_CFG_CLK_SPIFLASH1_T ap_clk = {};

    ap_clk.b.spiflash1_freq = 0xc;
    hwp_apApb->cfg_clk_spiflash1 = ap_clk.v;

    clken.b.clk_en_spiflash1_fr = 1;
    hwp_apApb->clk_ap_en0 = clken.v;
    CFG.b.cgm_spiflash1_sel = 0x4;
    hwp_apClk->cgm_spiflash1_sel_cfg = CFG.v;

    REG_SPI_FLASH_SPI_CONFIG_T SPI_CFG = {0};
    SPI_CFG.b.clk_divider = 0x2;
    SPI_CFG.b.sample_delay = 0x2;
    SPI_CFG.b.quad_mode = 1;
    hwp_spiFlash->spi_config = SPI_CFG.v;

#ifdef CONFIG_FIBOCOM_BASE
#ifdef CONFIG_BOARD_WITH_EXT_FLASH
    // external flash on vio, 83MHz.
    //sel spiflash clksource apa5 500M
    hwp_apClk->cgm_spiflash2_sel_cfg = 4;

#ifdef CONFIG_FIBOCOM_EXTFLASH_FROM_LCD
#ifdef CONFIG_FIBOCOM_EXTFLASH_3_0V
    //div apa5 500M to 6 200M for spiflash 500M -> 166M
    hwp_apApb->cfg_clk_spiflash2 = 0x5;
#elif defined(CONFIG_FIBOCOM_EXTFLASH_1_8V)
    //div apa5 500M to 6 200M for spiflash 500M -> 166M
    hwp_apApb->cfg_clk_spiflash2 = 0xb;
#endif
    REG_SPI_FLASH_SPI_CONFIG_T spi_config = {
#ifdef  CONFIG_SPECIAL_SPIFLASH_SUPPORT_TWO_LINE
        .b.quad_mode = 0,
#else
        .b.quad_mode = 1,
#endif
        .b.sample_delay = 2,
        .b.clk_divider = 2,
    };
    hwp_spiFlashExt->spi_config = spi_config.v;
#else
    hwp_apApb->cfg_clk_spiflash2 = 0xb;

    REG_SPI_FLASH_SPI_CONFIG_T spi_config = {
#ifdef  CONFIG_SPECIAL_SPIFLASH_SUPPORT_TWO_LINE
        .b.quad_mode = 0,
        .b.clk_divider = 6,
#else
        .b.quad_mode = 1,
        .b.clk_divider = 2,
#endif
        .b.sample_delay = 2,
    };
    hwp_spiFlashExt->spi_config = spi_config.v;
#endif
#endif
#else
#ifdef CONFIG_BOARD_WITH_EXT_FLASH
    // external flash on vio, 83MHz.
    //sel spiflash2 clksource apa5 500M
    hwp_apClk->cgm_spiflash2_sel_cfg = 4;

    // div apa5 500M to 3.5 for spiflash2 500M -> 142M
    hwp_apApb->cfg_clk_spiflash2 = 0xa;

    REG_SPI_FLASH_SPI_CONFIG_T spi_config = {
        .b.quad_mode = 1,
        .b.sample_delay = 2,
        .b.clk_divider = 2,
    };
    hwp_spiFlashExt->spi_config = spi_config.v;

#else
#ifdef CONFIG_SPI_NAND_DRIVER_SUPPORT
    // external spi nand flash on vio
    //config spi nand flash oper mode
#define SPI_NAND_QUAD_MODE

    //enable nand sel
    REGT_FIELD_CHANGE(hwp_spiFlashExt->spi_nand_config, REG_SPI_FLASH_SPI_NAND_CONFIG_T, ram_read_cmd, 0x3, nand_sel, 1);
    //enable spiflash en
    REGT_FIELD_CHANGE(hwp_apApb->clk_ap_en0, REG_AP_APB_CLK_AP_EN0_T, clk_en_spiflash2_fr, 1);
    //sel spiflash clksource apa5 500M
    REGT_FIELD_CHANGE(hwp_apClk->cgm_spiflash2_sel_cfg, REG_AP_CLK_CGM_SPIFLASH2_SEL_CFG_T, cgm_spiflash2_sel, 0x4);
    //div apa5 500M to 2.5 200M for spiflash 500M -> 200M (0xc)
    //cfg_clk_spiflash2 :
    //0x3 =7.0, 0x4 =6.5, 0x5 =6.0, 0x6 =5.5, 0x7 =5.0, 0x8 =4.5.0, 0x9 =4
    //0xa =3.5, 0xb =3.0, 0xc =2.5, 0xd =2.0, 0xe =1.5, 0xf=1.0
    REGT_FIELD_CHANGE(hwp_apApb->cfg_clk_spiflash2, REG_AP_APB_CFG_CLK_SPIFLASH2_T, spiflash2_freq, 0xc);

#ifdef SPI_NAND_QUAD_MODE
    REGT_FIELD_CHANGE(hwp_spiFlashExt->spi_config, REG_SPI_FLASH_SPI_CONFIG_T, quad_mode, 1, sample_delay, 4, clk_divider, 3); // 200M -> 66M
#else
    REGT_FIELD_CHANGE(hwp_spiFlashExt->spi_config, REG_SPI_FLASH_SPI_CONFIG_T, quad_mode, 0, sample_delay, 4, clk_divider, 3); // 200M -> 66M
#endif
#endif
#endif
#endif

    REG_IOMUX_PAD_M_SPI_D_3_T iocfg_d3 = {};
    REG_IOMUX_PAD_M_SPI_D_2_T iocfg_d2 = {};
    REG_IOMUX_PAD_M_SPI_D_1_T iocfg_d1 = {};
    REG_IOMUX_PAD_M_SPI_D_0_T iocfg_d0 = {};
    REG_IOMUX_PAD_M_SPI_CS_T iocfg_cs = {};
    REG_IOMUX_PAD_M_SPI_CLK_T iocfg_cLK = {};
    iocfg_d3.b.dslp_en = 63;
    iocfg_d3.b.drv = 5;
    hwp_iomux->pad_m_spi_d_3 = iocfg_d3.v;
    iocfg_d2.b.dslp_en = 63;
    iocfg_d2.b.drv = 5;
    hwp_iomux->pad_m_spi_d_2 = iocfg_d2.v;
    iocfg_d1.b.dslp_en = 63;
    iocfg_d1.b.drv = 5;
    hwp_iomux->pad_m_spi_d_1 = iocfg_d1.v;
    iocfg_d0.b.dslp_en = 63;
    iocfg_d0.b.drv = 5;
    hwp_iomux->pad_m_spi_d_0 = iocfg_d0.v;
    iocfg_cs.b.dslp_en = 63;
    iocfg_cs.b.drv = 5;
    hwp_iomux->pad_m_spi_cs = iocfg_cs.v;
    iocfg_cLK.b.dslp_en = 63;
    iocfg_cLK.b.drv = 5;
    hwp_iomux->pad_m_spi_clk = iocfg_cLK.v;
}

OSI_UNUSED static void prvPsramWarmInit(void)
{
#if defined(CONFIG_USE_PSRAM) && defined(CONFIG_PSRAM_LP_HALF_SLEEP)
    hwp_pubApb->slp_ctrl |= 0x80000;
    hwp_lpsApb->cfg_psram_half_slp |= 1;
#endif
    hwp_pubApb->cgm_psram = 0xa2;
    hwp_pubApb->slp_ctrl &= 0xFFFBF100;
    // DMC config for PSRAM
    hwp_pwrctrl->psram_hold_ctrl = 1;
    // hwp_dmcCtrl->memc_cmd = 0x0;
    if ((hwp_dmcCtrl->memc_status & 0x3) == 3)
    {
        hwp_dmcCtrl->memc_cmd = 0x0;
    }
    prvDmc400QosConfig();
#ifdef __PSRAM_AUTO__
    prvPsramDmc400Config(PSRAM_TYPE_REG);
#else
    prvPsramDmc400Config();
#endif

    hwp_dmcCtrl->memc_cmd = 0x3;
    //hwp_pwrctrl->psram_hold_ctrl = 0;
    // PSRAM PHY config
#ifdef __PSRAM_AUTO__
    prvPsramPhyConfig(PSRAM_TYPE_REG);
#else
    prvPsramPhyConfig();
#endif

    hwp_psramPhy->psram_rf_cfg_phy = 0x0;
    pll_cpst0();
    prvDelayUS(10);
    pll_cpst1();
    prvDelayUS(10);
    hwp_psramPhy->psram_rf_cfg_phy = 0x3;
    hwp_pwrctrl->psram_hold_ctrl = 0;
    // PSRAM PAD config
    //psram_pad_config();

#ifdef CONFIG_PSRAM_LP_HALF_SLEEP
    hwp_psramPhy->psram_rf_cfg_phy_iomux_oe_wr_ads0 = 0x00110000;  // rf_phy_io_csn_oe_ads0=1,  rf_phy_io_clk_oe_ads0=1
    hwp_psramPhy->psram_rf_cfg_phy_iomux_out_wr_ads0 = 0x00100000; // rf_phy_io_csn_out_ads0=1, rf_phy_io_clk_out_ads0=0
    hwp_psramPhy->psram_rf_cfg_phy_iomux_sel_wr_ads0 = 0x00110000; // rf_phy_io_csn_sel_ads0=1, rf_phy_io_clk_sel_ads0=1
    // select iomux for software ads1
    hwp_psramPhy->psram_rf_cfg_phy_iomux_oe_wr_ads1 = 0x00110000;  // rf_phy_io_csn_oe_ads1=1,  rf_phy_io_clk_oe_ads1=1
    hwp_psramPhy->psram_rf_cfg_phy_iomux_out_wr_ads1 = 0x00100000; // rf_phy_io_csn_out_ads1=1, rf_phy_io_clk_out_ads1=0
    hwp_psramPhy->psram_rf_cfg_phy_iomux_sel_wr_ads1 = 0x00110000; // rf_phy_io_csn_sel_ads1=1, rf_phy_io_clk_sel_ads1=1
    // gate clock
    hwp_psramPhy->psram_rf_cfg_phy_iomux_out_wr_ads0 = 0x00100000; // rf_phy_io_csn_out_ads0=1, rf_phy_io_clk_out_ads0=0
    hwp_psramPhy->psram_rf_cfg_phy_iomux_out_wr_ads1 = 0x00100000; // rf_phy_io_csn_out_ads1=1, rf_phy_io_clk_out_ads1=0
    // exit Half Sleep mode
    hwp_psramPhy->psram_rf_cfg_phy_iomux_out_wr_ads0 = 0x00000000; // rf_phy_io_csn_out_ads0=0, rf_phy_io_clk_out_ads0=0
    hwp_psramPhy->psram_rf_cfg_phy_iomux_out_wr_ads1 = 0x00000000; // rf_phy_io_csn_out_ads1=0, rf_phy_io_clk_out_ads1=0
    prvDelayUS(1);                                                 //weak up ce low hold time (60-2000ns)
    hwp_psramPhy->psram_rf_cfg_phy_iomux_out_wr_ads0 = 0x00100000; // rf_phy_io_csn_out_ads0=1, rf_phy_io_clk_out_ads0=0
    hwp_psramPhy->psram_rf_cfg_phy_iomux_out_wr_ads1 = 0x00100000; // rf_phy_io_csn_out_ads1=1, rf_phy_io_clk_out_ads1=0

    // release clock
    prvDelayUS(150);                                               //(ce low to first posedge clk > 150us)
    hwp_psramPhy->psram_rf_cfg_phy_iomux_out_wr_ads0 = 0x00100000; // rf_phy_io_csn_out_ads0=1, rf_phy_io_clk_out_ads0=0
    hwp_psramPhy->psram_rf_cfg_phy_iomux_out_wr_ads1 = 0x00100000; // rf_phy_io_csn_out_ads1=1, rf_phy_io_clk_out_ads1=0

    // select iomux for hardware
    hwp_psramPhy->psram_rf_cfg_phy_iomux_sel_wr_ads0 = 0x00000000; // rf_phy_io_csn_sel_ads010, rf_phy_io_clk_sel_ads010
    hwp_psramPhy->psram_rf_cfg_phy_iomux_oe_wr_ads0 = 0x00000000;  // rf_phy_io_csn_oe_ads010,  rf_phy_io_clk_oe_ads010
    hwp_psramPhy->psram_rf_cfg_phy_iomux_out_wr_ads0 = 0x00000000; // rf_phy_io_csn_out_ads010, rf_phy_io_clk_out_ads010
    hwp_psramPhy->psram_rf_cfg_phy_iomux_sel_wr_ads1 = 0x00000000; // rf_phy_io_csn_sel_ads010, rf_phy_io_clk_sel_ads010
    hwp_psramPhy->psram_rf_cfg_phy_iomux_oe_wr_ads1 = 0x00000000;  // rf_phy_io_csn_oe_ads010,  rf_phy_io_clk_oe_ads010
    hwp_psramPhy->psram_rf_cfg_phy_iomux_out_wr_ads1 = 0x00000000; // rf_phy_io_csn_out_ads010, rf_phy_io_clk_out_ads010
                                                                   //#500us;

    if ((hwp_dmcCtrl->memc_status & 0x3) == 3)
    {
        hwp_dmcCtrl->memc_cmd = 0x0;
    }
    // Power up PSRAM with global reset
    prvPsramWarmPor(); //FOR APSRAM64
    //psram_power_up_with_grst();
    hwp_dmcCtrl->memc_cmd = 0x3;

    prvDelayUS(150);
    //*(volatile unsigned int*) REG_PWRCTRL_PSRAM_HOLD_CTRL = 0;
    //rdata = *(volatile unsigned int*)0x5160300c;
    //*(volatile unsigned int*)0x5160300c = rdata | 0x40100; //for psram slp enable
    hwp_pwrctrl->psram_hold_ctrl = 0;

#elif defined(__PSRAM_AUTO__)
    // pm2 switch psram power between vmem and lps1.8, cause WIN955 lose CR0 config
    // re-config CR0 after wakeup from pm2
    if (PSRAM_TYPE_REG == WIN955_PROD_FLAG)
    {
        if (hwp_dmcCtrl->memc_status == DMC400_MEMC_STATUS_READY)
        {
            hwp_dmcCtrl->memc_cmd = DMC400_MEMC_CMD_CONFIG;
            OSI_LOOP_WAIT(hwp_dmcCtrl->memc_status == DMC400_MEMC_STATUS_CONFIG);
        }
        hwp_dmcCtrl->direct_cmd = 0x0; //NOP
        prvDelayUS(5);
        hwp_dmcCtrl->direct_cmd = 0x10000100; //config CR0
        prvDelayUS(5);
        hwp_dmcCtrl->memc_cmd = DMC400_MEMC_CMD_GO;
        OSI_LOOP_WAIT(hwp_dmcCtrl->memc_status == DMC400_MEMC_STATUS_READY);
    }
#endif
    hwp_pubApb->slp_ctrl |= 0x40F00;
}

void halRamInit(void)
{
#ifdef __PSRAM_AUTO__
    PSRAM_TYPE_REG = prvGetPsramConfig();
    if (PSRAM_TYPE_REG == WIN955_PROD_FLAG)
    {
        // WIN955 PSRAM run with 233MHz
        REGT_FIELD_WRITE(hwp_analogG1->analog_mpll_apll_int_value,
                         REG_ANALOG_G1_ANALOG_MPLL_APLL_INT_VALUE_T,
                         analog_mpll_apll_kint, 887258,
                         analog_mpll_apll_nint, 35);
        PSRAM_FREQ_REG = hwp_analogG1->analog_mpll_apll_int_value;
    }
#endif

    // Withdraw AP sleep request to PUB
#if defined(CONFIG_8850_FPGA_BOARD)
    REGT_FIELD_WRITE(hwp_pwrctrl->pub_pwr_ctrl_clr,
                     REG_PWRCTRL_PUB_PWR_CTRL_T,
                     ap_pol, 1);
    OSI_POLL_WAIT(hwp_pwrctrl->pub_pwr_stat == 3);
    prvRamPhyPadCfg();
    prvDmc400QosConfig();
    prvRamDmcCfg();
    prvRamPor();
    prvRamBootCfg();
#elif defined(CONFIG_SOC_8850)
    /*
    REGT_FIELD_WRITE(hwp_pubApb->cgm_psram,
                     REG_PUB_APB_CGM_PSRAM_T,
                     sel_2x, 5);
*/
    //unsigned int rdata;
    //rdata = *(volatile unsigned int*)0x5160300c;
    //*(volatile unsigned int*)0x5160300c = rdata & 0xFFFBF100; //0x40100for psram slp enable
    REGT_FIELD_WRITE(hwp_pwrctrl->pub_pwr_ctrl_clr,
                     REG_PWRCTRL_PUB_PWR_CTRL_T,
                     ap_pol, 1);
    //wait stable
    OSI_POLL_WAIT(hwp_pwrctrl->pub_pwr_stat == 3);
#if defined(CONFIG_USE_PSRAM) && defined(CONFIG_PSRAM_LP_HALF_SLEEP)
    hwp_pubApb->slp_ctrl |= 0x80000;
    hwp_lpsApb->cfg_psram_half_slp |= 1;
#endif
    hwp_pubApb->cgm_psram = 0xa2;
    hwp_pubApb->slp_ctrl &= 0xFFFBF100;
    // DMC config for PSRAM
    hwp_pwrctrl->psram_hold_ctrl = 1;
    // hwp_dmcCtrl->memc_cmd = 0x0;
    if ((hwp_dmcCtrl->memc_status & 0x3) == 3)
    {
        hwp_dmcCtrl->memc_cmd = 0x0;
    }
    prvDmc400QosConfig();

#ifdef __PSRAM_AUTO__
    prvPsramDmc400Config(PSRAM_TYPE_REG);
#else
    prvPsramDmc400Config();
#endif

    hwp_dmcCtrl->memc_cmd = 0x3;
    //hwp_pwrctrl->psram_hold_ctrl = 0;

    // PSRAM PHY config
#ifdef __PSRAM_AUTO__
    prvPsramPhyConfig(PSRAM_TYPE_REG);
#else
    prvPsramPhyConfig();
#endif
    hwp_psramPhy->psram_rf_cfg_phy = 0x0;
    pll_cpst0();
    prvDelayUS(10);
    pll_cpst1();
    prvDelayUS(10);
    hwp_psramPhy->psram_rf_cfg_phy = 0x3;
    hwp_pwrctrl->psram_hold_ctrl = 0;
    // PSRAM PAD config
    //psram_pad_config();
    hwp_dmcCtrl->memc_cmd = 0x0;
    // Power up PSRAM with global reset
#ifdef __PSRAM_AUTO__
    prvPsramPor(PSRAM_TYPE_REG);
#else
    prvPsramPor();
#endif
    //psram_power_up_with_grst();
    hwp_dmcCtrl->memc_cmd = 0x3;
    //*(volatile unsigned int*) REG_PWRCTRL_PSRAM_HOLD_CTRL = 0;
    //rdata = *(volatile unsigned int*)0x5160300c;
    //*(volatile unsigned int*)0x5160300c = rdata | 0x40100; //for psram slp enable
    hwp_pwrctrl->psram_hold_ctrl = 0;
    hwp_pubApb->slp_ctrl |= 0x40F00;
#endif
    hwp_idleLps->lps_res8 = (hwp_idleLps->lps_res8 | 0x1);
}

void halClockInit(unsigned whence)
{
    if (whence != HAL_CLOCK_INIT_OPENCPU)
    {
        // choose apll500M, aon defalut HW ctrl
        REGT_FIELD_CHANGE(hwp_apClk->cgm_ap_a5_sel_cfg,
                          REG_AP_CLK_CGM_AP_A5_SEL_CFG_T, cgm_ap_a5_sel, 5);

        // choose apll200M
        REGT_FIELD_CHANGE(hwp_aonClk->cgm_aon_ahb_sel_cfg,
                          REG_AON_CLK_CGM_AON_AHB_SEL_CFG_T, cgm_aon_ahb_sel, 7);

        REGT_FIELD_CHANGE(hwp_apClk->cgm_ap_a5_div_cfg,
                          REG_AP_CLK_CGM_AP_A5_DIV_CFG_T, cgm_ap_a5_div, 0);

        REGT_FIELD_CHANGE(hwp_apClk->cgm_ap_bus_div_cfg,
                          REG_AP_CLK_CGM_AP_BUS_DIV_CFG_T, cgm_ap_bus_div, 1);

        REGT_FIELD_CHANGE(hwp_aonClk->cgm_aon_ahb_div_cfg,
                          REG_AON_CLK_CGM_AON_AHB_DIV_CFG_T, cgm_aon_ahb_div, 0);
    }

    if (whence == HAL_CLOCK_INIT_OPENCPU)
    {
        // choose xtal26m, aon defalut HW ctrl
        REGT_FIELD_CHANGE(hwp_apClk->cgm_ap_a5_sel_cfg,
                          REG_AP_CLK_CGM_AP_A5_SEL_CFG_T, cgm_ap_a5_sel, 1);

        REGT_FIELD_CHANGE(hwp_apClk->cgm_ap_a5_div_cfg,
                          REG_AP_CLK_CGM_AP_A5_DIV_CFG_T, cgm_ap_a5_div, 0);

        REGT_FIELD_CHANGE(hwp_apClk->cgm_ap_bus_div_cfg,
                          REG_AP_CLK_CGM_AP_BUS_DIV_CFG_T, cgm_ap_bus_div, 1);
    }
    // close debug function, use clk_mode and clk_en control
    hwp_apApb->cgm_gate_auto_sel0 = 0xffffffff;
    hwp_apApb->cgm_gate_auto_sel1 = 0xffffffff;
    hwp_apApb->cgm_gate_auto_sel2 = 0xffffffff;
    hwp_apApb->cgm_gate_auto_sel3 = 0xffffffff;

    hwp_apApb->clk_ap_mode0 = 0; //clk_core auto gating
    hwp_apApb->clk_ap_mode1 = 0; //ip clk auto gating
    hwp_apApb->clk_ap_mode2 = 0;

    // debug host
    #ifndef CONFIG_FIBOCOM_BASE
    REGT_FIELD_CHANGE(hwp_iomux->debug_host_tx, REG_IOMUX_DEBUG_HOST_TX_T, func_sel, 2);
    #endif
    REGT_FIELD_CHANGE(hwp_iomux->debug_host_rx, REG_IOMUX_DEBUG_HOST_RX_T, func_sel, 2);

    if (whence == HAL_CLOCK_INIT_OPENCPU)
    {
        // enable wakeup sources
        REG_IDLE_LPS_AP_AWK_EN_T ap_awk_en = {
            .b.ap_awk0_en = 1, // pmic
            .b.ap_awk1_en = 0, // uart1
            .b.ap_awk2_en = 1, // keypad
            .b.ap_awk3_en = 1, // gpio1
            .b.ap_awk4_en = 1, // gpt1
            .b.ap_awk5_en = 0, // uart1_rx
            .b.ap_awk6_en = 1, // mailbox_ap
            .b.ap_awk7_en = 1, // mailbox_cp
            // .b.ap_awk8_en = 1, // uart2
            // .b.ap_awk9_en = 1, // uart3
            .b.ap_awk10_en = 1, // gpio2
            .b.ap_awk11_en = 1, // gpt2_irq0
            .b.ap_awk12_en = 1, // gpt2_irq1
            .b.ap_awk13_en = 1, // gpt2_irq2
            // .b.ap_awk14_en = 1, // uart2_rx
            // .b.ap_awk15_en = 1, // uart3_rx
            .b.ap_awk16_en = 1, // usb
            .b.ap_awk17_en = 1, // spi2
            //.b.ap_awk18_en = 1, // usb se0 irq
            .b.ap_awk19_en = 1, // rtc_timer
            .b.ap_p1_awk_en = 1
            // .b.ap_t1_awk_en = 1,
            // .b.ap_t2_awk_en = 1,
            // .b.ap_t3_awk_en = 1,
            // .b.ap_t4_awk_en = 1,
            // .b.ap_t5_awk_en = 1,
            // .b.ap_t6_awk_en = 1,
        };
        hwp_idleLps->ap_awk_en = ap_awk_en.v;
    }

    if (whence != HAL_CLOCK_INIT_OPENCPU)
    {
        // enable wakeup sources
        REG_IDLE_LPS_AP_AWK_EN_T ap_awk_en = {
            .b.ap_awk0_en = 1, // pmic
            .b.ap_awk1_en = 1, // uart1
            .b.ap_awk2_en = 1, // keypad
            .b.ap_awk3_en = 1, // gpio1
            .b.ap_awk4_en = 1, // gpt1
            .b.ap_awk5_en = 1, // uart1_rx
            .b.ap_awk6_en = 1, // mailbox_ap
        .b.ap_awk7_en = 1, // mailbox_cp
        // .b.ap_awk8_en = 1, // uart2
#ifdef CONFIG_FIBOCOM_BASE
        .b.ap_awk9_en = 1, // uart3
#else
        // .b.ap_awk9_en = 1, // uart3
#endif
        .b.ap_awk10_en = 1, // gpio2
        .b.ap_awk11_en = 1, // gpt2_irq0
            .b.ap_awk12_en = 1, // gpt2_irq1
            .b.ap_awk13_en = 1, // gpt2_irq2
            // .b.ap_awk14_en = 1, // uart2_rx
            // .b.ap_awk15_en = 1, // uart3_rx
            .b.ap_awk16_en = 1, // usb
            .b.ap_awk17_en = 1, // spi2
            //.b.ap_awk18_en = 1, // usb se0 irq
            .b.ap_awk19_en = 1, // rtc_timer
            .b.ap_p1_awk_en = 1
            // .b.ap_t1_awk_en = 1,
            // .b.ap_t2_awk_en = 1,
            // .b.ap_t3_awk_en = 1,
            // .b.ap_t4_awk_en = 1,
            // .b.ap_t5_awk_en = 1,
            // .b.ap_t6_awk_en = 1,
        };
        hwp_idleLps->ap_awk_en_set = ap_awk_en.v;
    }

    REG_IDLE_LPS_AP_AWK_EN1_T ap_awk_en1 = {
        .b.ap_t7_awk_en = 1,
        .b.ap_t8_awk_en = 1,
        .b.ap_t9_awk_en = 1};
    hwp_idleLps->ap_awk_en1_set = ap_awk_en1.v;

    REGT_FIELD_CHANGE(hwp_idleLps->ap_inten, REG_IDLE_LPS_AP_INTEN_T, ap_sys_awk_irq_to_ap_en, 1);
    // hwp_idleTimer->slow_sys_clk_sel_hwen = 1;
    // hwp_idleTimer->slow_clk_sel_hwen = 1;
    if (whence != HAL_CLOCK_INIT_OPENCPU)
    {
        prvFlashClkInit();

        halHwspinlockAcquireInCritical(HAL_HWSPINLOCK_ID_CPSLEEP);

        for (uintptr_t address = (uintptr_t)&hwp_mailbox->sysmail0;
             address <= (uintptr_t)&hwp_mailbox->sysmail95; address += 4)
            *(volatile unsigned *)address = 0;
        halHwspinlockReleaseInCritical(HAL_HWSPINLOCK_ID_CPSLEEP);
    }
}

void halRamWakeInit(void)
{
#ifdef __PSRAM_AUTO__
    if (PSRAM_TYPE_REG == WIN955_PROD_FLAG)
    {
        hwp_analogG1->analog_mpll_apll_int_value = PSRAM_FREQ_REG;
    }
#endif

#ifdef CONFIG_8850_FPGA_BOARD
    OSI_POLL_WAIT((hwp_pwrctrl->pub_pwr_stat & 0x2) == 0x2);
    // Withdraw AP sleep request to PUB
    REGT_FIELD_WRITE(hwp_pwrctrl->pub_pwr_ctrl_clr,
                     REG_PWRCTRL_PUB_PWR_CTRL_T,
                     ap_pol, 1);
    OSI_POLL_WAIT(hwp_pwrctrl->pub_pwr_stat == 3);
    if (hwp_idleLps->lps_res8 != 0)
    {
    }
    else
    {
#if defined(CONFIG_USE_PSRAM) && defined(CONFIG_PSRAM_LP_HALF_SLEEP)
        prvRamPhyPadCfg();
        prvPsramExitHalfSleep();
        prvRamPor();
        prvDmc400QosConfig();
        prvRamDmcCfg();
        prvDelayUS(100);
        hwp_pwrctrl->psram_hold_ctrl = 0;
        prvDelayUS(100);
#else
        prvRamWakeCfg();
#endif
        halPmuWakeFromDeep();//patch by SPCSS01010144
    }
    hwp_idleLps->lps_res8 = (hwp_idleLps->lps_res8 | 0x1);
#else
#if defined(CONFIG_USE_PSRAM)
#ifndef CONFIG_8850CM_TARGET
    // Withdraw AP sleep request to PUB
    REGT_FIELD_WRITE(hwp_pwrctrl->pub_pwr_ctrl_clr,
                     REG_PWRCTRL_PUB_PWR_CTRL_T,
                     ap_pol, 1);
#else
    hwp_pwrctrl->pub_pwr_ctrl_clr = 0x3;
    //hwp_idleLps->cp_sig_en = 0x3f;//need double check ?+#endif
#endif
    //wait stable
    OSI_POLL_WAIT(hwp_pwrctrl->pub_pwr_stat == 3);

    if (hwp_idleLps->lps_res8 == 0)
    {
        prvPsramWarmInit();
        halPmuWakeFromDeep();
#ifdef CONFIG_8850CM_TARGET
        //cp sys power on
#ifdef CONFIG_OPENCPU_ENABLED
        if (!halGetOpenCPUPmMode())
#endif
        {
            hwp_pwrctrl->cp_pwr_ctrl = 1;
            OSI_LOOP_WAIT(hwp_pwrctrl->cp_pwr_stat == 3);
        }
#endif
    }
    hwp_idleLps->lps_res8 = (hwp_idleLps->lps_res8 | 0x1);
#endif
#endif
}

void halRamSuspend(void)
{
#if defined(CONFIG_USE_PSRAM) && defined(CONFIG_PSRAM_LP_HALF_SLEEP)
    hwp_pubApb->slp_ctrl |= 0x80000;
    hwp_lpsApb->cfg_psram_half_slp |= 1;
    prvDelayUS(1);
    hwp_dmcCtrl->memc_cmd = 0x0;
    hwp_pwrctrl->pub_pwr_ctrl = 0x3;
    OSI_POLL_WAIT(hwp_pwrctrl->pub_pwr_stat != 0x2);
#endif
}
