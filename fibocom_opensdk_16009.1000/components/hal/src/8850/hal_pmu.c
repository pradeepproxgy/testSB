
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

#define OSI_LOCAL_LOG_LEVEL OSI_LOG_LEVEL_DEBUG

#include "hwregs.h"
#include "osi_api.h"
#include "osi_log.h"
#include "hal_adi_bus.h"
#include "hal_chip.h"
#include "hal_config.h"
#include "drv_config.h"
#include "hal_iomux.h"
#include "hal_gpio.h"
#include "kernel_config.h"
#include "modem_lte_config.h"
#include <stdlib.h>
#include <string.h>
#include "drv_efuse_pmic.h"
#include "openCPU_config.h"
#include "fibocom.h"
#ifdef CONFIG_FIBOCOM_BASE
#include "didi_app.h"
#endif

#define PWR_WR_PROT_MAGIC (0x6e7f)
#define PSM_WR_PROT_MAGIC (0x454e)
#define RTC_ALARM_UNLOCK_MAGIC (0xa5)

#define DAY_SECONDS (24 * 60 * 60)
#define HOUR_SECONDS (60 * 60)
#define MIN_SECONDS (60)
#define WEEK_DAYS (7)

#define PSM_MIN_CNT (5)
#define PSM_MODE_REG (hwp_pmicPsm->reserved_2)
#define PMU_BOOT_MODE_REG (hwp_pmicRtcAna->por_rst_monitor)
#define PSM_EPOCH_HIGH (hwp_pmicPsm->reserved_5)
#define PSM_EPOCH_LOW (hwp_pmicPsm->reserved_6)
#define PMIC_AC (0x8850a002)

#define HAL_POWER_SD_H OSI_MAKE_TAG('S', 'D', 'H', ' ')

bool halPmuIsVCoreFirstTimeEnterDeep(void);

static uint32_t prvGetPmicChipId(void);

static uint32_t prvPsmCntUs(void);

#define PSM_CNT_US (prvPsmCntUs())



#ifdef CONFIG_FIBOCOM_BASE
static int bltc_en_inuse = 0;// This flag is because sink1 and sink2 use same enable bit to control, if sink1 or sink2 is in use this bit must be enabled
extern void osiSetUpTimeUS(int64_t us);
extern uint8_t spiflash_is_4line;
extern bool fibo_get_psm_mode(void);
extern int fibo_get_customize_info(void);

#if defined CONFIG_SUPPORT_BATTERY_CHARGER
extern bool fibo_getChargeSwitch(void);
#endif
#endif
typedef struct
{
    uint32_t power_id;
    int32_t step;
    int32_t min_uV;
    int32_t max_uV;
} powerLevelConfigInfo_t;

typedef enum
{
    CLOCK_26M_TSX,
    CLOCK_26M_DCXO,
    CLOCK_26M_TCXO,
    CLOCK_26M_UNIDENTIFIED
} CLOCK_26M_TYPE_T;

static bool bWakeupFromDeep = false;
//for openCPU PSM mode, ap system doesn't use
static bool gIsOpenCPUPmMode = false;
static bool bDeepVcoreFlag = false;

uint32_t key_ref = 0xe;
static const powerLevelConfigInfo_t pmic_power_level_table[] = {
    {.power_id = HAL_POWER_SIM0, .step = 25000, .min_uV = 1625000, .max_uV = 3225000},
    {.power_id = HAL_POWER_SIM1, .step = 25000, .min_uV = 1625000, .max_uV = 3225000},
    {.power_id = HAL_POWER_BUCK_PA, .step = 25000, .min_uV = 400000, .max_uV = 3500000},
    {.power_id = HAL_POWER_USB, .step = 25000, .min_uV = 1625000, .max_uV = 3225000},
    {.power_id = HAL_POWER_CAMA, .step = 25000, .min_uV = 1625000, .max_uV = 3225000},
    {.power_id = HAL_POWER_VIO33, .step = 25000, .min_uV = 1925000, .max_uV = 3500000},
    {.power_id = HAL_POWER_SD, .step = 25000, .min_uV = 1525000, .max_uV = 2300000},
    {.power_id = HAL_POWER_SD_H, .step = 25000, .min_uV = 2725000, .max_uV = 3500000},
    {.power_id = HAL_POWER_LP18, .step = 25000, .min_uV = 1625000, .max_uV = 3225000},
    {.power_id = HAL_POWER_LCD, .step = 25000, .min_uV = 1625000, .max_uV = 3225000},
    {.power_id = HAL_POWER_RF12, .step = 12500, .min_uV = 812500, .max_uV = 1600000},
    {.power_id = HAL_POWER_RF15, .step = 12500, .min_uV = 1400000, .max_uV = 1887500},
    {.power_id = HAL_POWER_CAMD, .step = 12500, .min_uV = 1400000, .max_uV = 2187500},
    {.power_id = HAL_POWER_SPIMEM, .step = 12500, .min_uV = 1400000, .max_uV = 2187500},
    {.power_id = HAL_POWER_MEM, .step = 12500, .min_uV = 1400000, .max_uV = 2187500},
    {.power_id = HAL_POWER_VIO18, .step = 12500, .min_uV = 1400000, .max_uV = 2187500},
    {.power_id = HAL_POWER_ANALOG, .step = 12500, .min_uV = 1400000, .max_uV = 2187500},
    {.power_id = HAL_POWER_KEYLED, .step = 100000, .min_uV = 2800000, .max_uV = 3500000},

};

PMD_LDO_SETTINGS_T g_pmdLdoSettings =
    {{
        .fmEnable = false,
        .btEnable = false,
        .keyEnable = false,
        .tscEnable = false,
        .vLcdEnable = true,
        .vCamEnable = true,
        .vMicEnable = true,
        .vIbrEnable = true,
        .vRfEnable = true,
        .vAbbEnable = true,
        .vMmcEnable = true,
        .vSim4Enable = false,
        .vSim3Enable = false,
        .vSim2Enable = false,
        .vSim1Enable = false,
    }};

PMD_LDO_PROFILE_SETTING5_T g_pmdLdoProfileSetting5 =
    {{
        // NOTE:
        //   vRgbLed & vAbb bits are located in RDA_ADDR_LDO_ACTIVE_SETTING5
        //   only, and they are valid for all profiles
        .vRgbLedIbit = 4,
        .vRgbLedVsel = 4,
#ifdef ABB_HP_DETECT
        // Workaround:
        // Headphone detection will encounter errors if vAbbIbit>=3
        .vAbbIbit = 2,
#else
        .vAbbIbit = 4,
#endif
        // End of NOTE
        .vUsbIbit = 4,
        .vSim4Is1_8 = true,
        .vSim3Is1_8 = true,
        .vSim2Is1_8 = true,
        .vSim1Is1_8 = true,
    }};

//patch by SPCSS01010144
static bool bPowerByResetPin = false;


static uint32_t prvGetPmicChipId(void)
{
    uint32_t anaChipId = (((uint32_t)halAdiBusRead(&hwp_pmicAna->chip_id_high) << 16) | ((uint32_t)halAdiBusRead(&hwp_pmicAna->chip_id_low) & 0xFFFF));
    return anaChipId;
}

//BM ab 78125 less
//BM ac 78145 less
//DG ab 78125 less ???
//DG ac 78145 less ???
//XTAL 78125
static uint32_t prvPsmCntUs(void)
{
    uint32_t psm_cnt_us = 0;

    REG_PMIC_RTC_ANA_CLK32KLESS_CTRL0_T clk32kless_ctrl0;
    clk32kless_ctrl0.v = halAdiBusRead(&hwp_pmicRtcAna->clk32kless_ctrl0);
    if ((clk32kless_ctrl0.b.rtc_mode == 1) && (prvGetPmicChipId() == PMIC_AC))
        psm_cnt_us = (uint32_t)78145; //less
    else
        psm_cnt_us = (uint32_t)78125; //xtal

    return psm_cnt_us;
}

static bool prvGetPowerLevelTableMap(uint32_t power_id, powerLevelConfigInfo_t *info)
{
    int32_t i = 0;

    int32_t count = sizeof(pmic_power_level_table) / sizeof(pmic_power_level_table[0]);
    for (i = 0; i < count; i++)
    {
        if (pmic_power_level_table[i].power_id == power_id)
        {
            memcpy(info, &pmic_power_level_table[i], sizeof(powerLevelConfigInfo_t));
            return true;
        }
    }

    return false;
}

static int32_t prvVoltageSettingValue(uint32_t power_id, uint32_t mv)
{
#define DIV_ROUND_UP(n, d) (((n) + (d)-1) / (d))
    powerLevelConfigInfo_t info;
    uint32_t power_level_id;
    uint16_t level_offset;
    power_level_id = power_id;
    level_offset = 0;
    if (power_level_id == HAL_POWER_SD)
    {
        if (mv > 2500)
        {
            power_level_id = HAL_POWER_SD_H;
            level_offset = 32;
        }
    }

    if (!prvGetPowerLevelTableMap(power_level_id, &info))
        return -1;

    int32_t uv = mv * 1000;
    int32_t min_uV = info.min_uV;
    int32_t max_uV = info.max_uV;
    int32_t step = info.step;

    OSI_LOGD(0x100092c5, "power_id = %4c, step = %d, min_uV = %d, max_uV = %d",
             power_level_id, info.step, info.min_uV, info.max_uV);

    if ((uv < min_uV) || (uv > max_uV))
        return -1;

    int32_t value = DIV_ROUND_UP((int)(uv - min_uV), step);

    return value + level_offset;
}

static void prvUnlockPowerReg(void)
{
#ifdef CONFIG_SOC_8850
#ifndef CONFIG_8850_FPGA_BOARD
    halAdiBusWrite(&hwp_pmicRtcAna->pwr_wr_prot_value, PWR_WR_PROT_MAGIC);
    OSI_LOOP_WAIT(halAdiBusRead(&hwp_pmicRtcAna->pwr_wr_prot_value) == 0x8000);
#endif
#endif
}

OSI_UNUSED static void prvUnlockPsmReg(void)
{
#ifndef CONFIG_8850_FPGA_BOARD

    halAdiBusWrite(&hwp_pmicPsm->psm_reg_wr_protect, PSM_WR_PROT_MAGIC);
    OSI_LOOP_WAIT(halAdiBusRead(&hwp_pmicPsm->psm_reg_wr_protect) == 1);
#endif
}

static void prvResetPinEnable(void)
{
#ifdef CONFIG_SOC_8850
    REG_PMIC_RTC_ANA_POR_7S_CTRL_T por_7s_ctrl;
    REG_ADI_CHANGE1(hwp_pmicRtcAna->por_7s_ctrl, por_7s_ctrl,
                    ext_rstn_mode, 1);

    REG_PMIC_RTC_ANA_SWRST_CTRL0_T swrst_ctrl0 = {halAdiBusRead(&hwp_pmicRtcAna->swrst_ctrl0)};
    swrst_ctrl0.b.ext_rstn_pd_en = 1;
    swrst_ctrl0.b.pb_7s_rst_pd_en = 1;
    swrst_ctrl0.b.reg_rst_pd_en = 1;
    swrst_ctrl0.b.wdg_rst_pd_en = 0;
    halAdiBusWrite(&hwp_pmicRtcAna->swrst_ctrl0, swrst_ctrl0.v);

    REG_PMIC_RTC_ANA_SWRST_CTRL1_T swrst_ctrl1 = {};
    swrst_ctrl1.b.sw_rst_vio33_pd_en = 0;
    swrst_ctrl1.b.sw_rst_usb_pd_en = 0;
    swrst_ctrl1.b.sw_rst_rf15_pd_en = 0;
    swrst_ctrl1.b.sw_rst_ana_pd_en = 0;
    swrst_ctrl1.b.sw_rst_rf12_pd_en = 0;
    swrst_ctrl1.b.sw_rst_dcxo_pd_en = 0;
    swrst_ctrl1.b.sw_rst_mem_pd_en = 1;
    swrst_ctrl1.b.sw_rst_dcdccore_pd_en = 0;
    swrst_ctrl1.b.sw_rst_dcdcgen_pd_en = 0;
    swrst_ctrl1.b.sw_rst_vio18_pd_en = 0;
    swrst_ctrl1.b.sw_rst_spimem_pd_en = 0;
    halAdiBusWrite(&hwp_pmicRtcAna->swrst_ctrl1, swrst_ctrl1.v);
#endif
}
void prvPinDeepSleepDisable(void)
{
    //G2
    hwp_iomux->pad_rfdig_gpio_7 &= ~0x7E000;
    hwp_iomux->pad_rfdig_gpio_6 &= ~0x7E000;
    hwp_iomux->pad_rfdig_gpio_5 &= ~0x7E000;
    hwp_iomux->pad_rfdig_gpio_4 &= ~0x7E000;
    hwp_iomux->pad_rfdig_gpio_3 &= ~0x7E000;
    hwp_iomux->pad_rfdig_gpio_2 &= ~0x7E000;
    hwp_iomux->pad_rfdig_gpio_1 &= ~0x7E000;
    hwp_iomux->pad_rfdig_gpio_0 &= ~0x7E000;

    //G4
    hwp_iomux->pad_keyin_4 &= ~0x7E000;
    hwp_iomux->pad_keyout_5 &= ~0x7E000;
    hwp_iomux->pad_keyin_5 &= ~0x7E000;
    hwp_iomux->pad_keyout_4 &= ~0x7E000;
    hwp_iomux->pad_uart_1_rts &= ~0x7E000;
    hwp_iomux->pad_uart_1_txd &= ~0x7E000;
    hwp_iomux->pad_uart_1_rxd &= ~0x7E000;
    hwp_iomux->pad_uart_1_cts &= ~0x7E000;
    hwp_iomux->pad_gpio_0 &= ~0x7E000;
    hwp_iomux->pad_gpio_3 &= ~0x7E000;
    hwp_iomux->pad_gpio_2 &= ~0x7E000;
    hwp_iomux->pad_gpio_1 &= ~0x7E000;
    hwp_iomux->pad_gpio_7 &= ~0x7E000;
    hwp_iomux->pad_gpio_6 &= ~0x7E000;
    hwp_iomux->pad_gpio_5 &= ~0x7E000;
    hwp_iomux->pad_gpio_4 &= ~0x7E000;
    hwp_iomux->pad_adi_sda &= ~0x7E000;
    hwp_iomux->pad_adi_scl &= ~0x7E000;
    hwp_iomux->pad_resetb &= ~0x7E000;
    hwp_iomux->pad_osc_32k &= ~0x7E000;
    hwp_iomux->pad_pmic_ext_int &= ~0x7E000;
    hwp_iomux->pad_chip_pd &= ~0x7E000;
    hwp_iomux->pad_clk26m_pmic &= ~0x7E000;
    hwp_iomux->pad_sim_1_rst &= ~0x7E000;
    hwp_iomux->pad_sim_1_dio &= ~0x7E000;
    hwp_iomux->pad_sim_1_clk &= ~0x7E000;
    hwp_iomux->pad_sim_0_rst &= ~0x7E000;
    hwp_iomux->pad_sim_0_dio &= ~0x7E000;
    hwp_iomux->pad_sim_0_clk &= ~0x7E000;

    //G5
    hwp_iomux->pad_sw_clk &= ~0x7E000;
    hwp_iomux->pad_sw_dio &= ~0x7E000;
    hwp_iomux->pad_debug_host_tx &= ~0x7E000;
    hwp_iomux->pad_debug_host_rx &= ~0x7E000;
    hwp_iomux->pad_debug_host_clk &= ~0x7E000;
    //hwp_iomux->pad_camera_rst_l &= ~0x7E000;
    hwp_iomux->pad_spi_camera_sck &= ~0x7E000;
    hwp_iomux->pad_spi_camera_si_1 &= ~0x7E000;
    hwp_iomux->pad_spi_camera_si_0 &= ~0x7E000;
    //hwp_iomux->pad_camera_ref_clk &= ~0x7E000;
    //hwp_iomux->pad_camera_pwdn &= ~0x7E000;
    //hwp_iomux->pad_i2s_sdat_i &= ~0x7E000;
    //hwp_iomux->pad_i2s1_sdat_o &= ~0x7E000;
    //hwp_iomux->pad_i2s1_lrck &= ~0x7E000;
    //hwp_iomux->pad_i2s1_bck &= ~0x7E000;
    //hwp_iomux->pad_i2s1_mclk &= ~0x7E000;

    //G1
    hwp_iomux->pad_i2c_m2_scl &= ~0x7E000;
    hwp_iomux->pad_i2c_m2_sda &= ~0x7E000;
    hwp_iomux->pad_nand_sel &= ~0x7E000;
    hwp_iomux->pad_keyout_3 &= ~0x7E000;
    hwp_iomux->pad_keyout_2 &= ~0x7E000;
    hwp_iomux->pad_keyout_1 &= ~0x7E000;
    hwp_iomux->pad_keyout_0 &= ~0x7E000;
    hwp_iomux->pad_keyin_3 &= ~0x7E000;
    hwp_iomux->pad_keyin_2 &= ~0x7E000;
    hwp_iomux->pad_keyin_1 &= ~0x7E000;
    hwp_iomux->pad_keyin_0 &= ~0x7E000;
    hwp_iomux->pad_lcd_rstb &= ~0x7E000;
    hwp_iomux->pad_lcd_fmark &= ~0x7E000;
    hwp_iomux->pad_spi_lcd_select &= ~0x7E000;
    hwp_iomux->pad_spi_lcd_cs &= ~0x7E000;
    hwp_iomux->pad_spi_lcd_clk &= ~0x7E000;
    hwp_iomux->pad_spi_lcd_sdc &= ~0x7E000;
    hwp_iomux->pad_spi_lcd_sio &= ~0x7E000;
    hwp_iomux->pad_sdmmc1_rst &= ~0x7E000;
    hwp_iomux->pad_sdmmc1_data_7 &= ~0x7E000;
    hwp_iomux->pad_sdmmc1_data_6 &= ~0x7E000;
    hwp_iomux->pad_sdmmc1_data_5 &= ~0x7E000;
    hwp_iomux->pad_sdmmc1_data_4 &= ~0x7E000;
    hwp_iomux->pad_sdmmc1_data_3 &= ~0x7E000;
    hwp_iomux->pad_sdmmc1_data_2 &= ~0x7E000;
    hwp_iomux->pad_sdmmc1_data_1 &= ~0x7E000;
    hwp_iomux->pad_sdmmc1_data_0 &= ~0x7E000;
    hwp_iomux->pad_sdmmc1_cmd &= ~0x7E000;
    hwp_iomux->pad_sdmmc1_clk &= ~0x7E000;

    //G3
    //hwp_iomux->pad_uart_2_rts &= ~0x7E000;
    hwp_iomux->pad_uart_2_cts &= ~0x7E000;
    //hwp_iomux->pad_uart_2_txd &= ~0x7E000;
    hwp_iomux->pad_uart_2_rxd &= ~0x7E000;
    hwp_iomux->pad_i2c_m1_sda &= ~0x7E000;
    hwp_iomux->pad_i2c_m1_scl &= ~0x7E000;
    hwp_iomux->pad_gpio_23 &= ~0x7E000;
    hwp_iomux->pad_gpio_22 &= ~0x7E000;
    hwp_iomux->pad_gpio_21 &= ~0x7E000;
    hwp_iomux->pad_gpio_20 &= ~0x7E000;
    hwp_iomux->pad_gpio_19 &= ~0x7E000;
    hwp_iomux->pad_gpio_18 &= ~0x7E000;
    hwp_iomux->pad_gpio_17 &= ~0x7E000;
    hwp_iomux->pad_gpio_16 &= ~0x7E000;
    hwp_iomux->pad_m_spi_d_3 &= ~0x7E000;
    hwp_iomux->pad_m_spi_d_2 &= ~0x7E000;
    hwp_iomux->pad_m_spi_d_1 &= ~0x7E000;
    hwp_iomux->pad_m_spi_d_0 &= ~0x7E000;
    hwp_iomux->pad_m_spi_cs &= ~0x7E000;
    hwp_iomux->pad_m_spi_clk &= ~0x7E000;
}

void prvPinPropertySet(void)
{
    hwp_iomux->pad_chip_pd &= ~0x0000F;
    hwp_iomux->pad_chip_pd |= 0x7E001;

    hwp_iomux->pad_adi_sda &= ~0x0000F;
    hwp_iomux->pad_adi_scl &= ~0x0000F;

    hwp_iomux->pad_adi_sda |= 0x7E003;
    hwp_iomux->pad_adi_scl |= 0x7E001;

    hwp_iomux->pad_uart_2_rts |= 0x8;
    hwp_iomux->pad_uart_2_txd |= 0x8;
    hwp_iomux->pad_gpio_17 |= 0x80;
}

static void prvEnableVDDCoreDrop(void)
{
    if (halPmuIsVCoreFirstTimeEnterDeep() && bWakeupFromDeep == true)
    {
        REG_PMIC_RTC_ANA_SLP_DCDC_PD_CTRL_T slp_dcdc_pd_ctrl;
        REG_ADI_CHANGE1(hwp_pmicRtcAna->slp_dcdc_pd_ctrl, slp_dcdc_pd_ctrl, slp_dcdccore_drop_en, 1);

        bWakeupFromDeep = false;
    }
}

static void prvModifyVDDCoreDropVol(void)
{
    if (bDeepVcoreFlag == true)
        return;

    if (clock26mId == CLOCK_26M_TCXO)
    {
        // pm1 vcore set to 0.75v ,  0xf0 = 0.75v , 0x100 = 0.8v
        REG_PMIC_RTC_ANA_DCDC_CORE_SLP_CTRL1_T dcdc_core_slp_ctrl1 = {
            .b.pm1_dcdc_core_vosel_ds_sw = 0xf0}; //0xf0 = 0.75v , 0x100 = 0.8v
        halAdiBusWrite(&hwp_pmicRtcAna->dcdc_core_slp_ctrl1, dcdc_core_slp_ctrl1.v);

        REG_PMIC_RTC_ANA_DCDC_CORE_SLP_CTRL0_T dcdc_core_slp_ctrl0 = {
            .b.pm1_dcdc_core_slp_step_delay = 3,
            .b.pm1_dcdc_core_slp_step_num = 3,
            .b.pm1_dcdc_core_slp_step_vol = 0xc,
            .b.dcdc_core_slp_step_en = 1};
        halAdiBusWrite(&hwp_pmicRtcAna->dcdc_core_slp_ctrl0, dcdc_core_slp_ctrl0.v);

        // pm2 vcore set to 0.675v ,  0x6409 = 0.625v, 0x6c09 = 0.675v
        halAdiBusWrite(&hwp_pmicRtcAna->slp_ldo_lp_ctrl1, 0x6c09);

        REG_PMIC_RTC_ANA_RESERVED_REG1_T reserved_reg1 = {
            .b.pm2_dcdc_core_slp_step_vol = 0x9};
        halAdiBusWrite(&hwp_pmicRtcAna->reserved_reg1, reserved_reg1.v);
    }
    bDeepVcoreFlag = true;
}

void halPmuExtFlashPowerOn(void)
{
#ifdef CONFIG_BOARD_WITH_EXT_FLASH
    // Usually, iomux, power with correct voltage, clock should be
    // configured here. There are examples below, and the default is
    // external flash on vio, 83MHz.

#if defined(CONFIG_FIBOCUS_JIEKE_FEATURE) && defined(CONFIG_MC661_CN_19_70_JIEKE)
    halIomuxSetFunction(HAL_IOMUX_FUN_GPIO_33_PAD_KEYOUT_1);
    halGpioSetOutput(33,1);//19-87 3V3_EN for boot
#endif

#define EXT_FLASH_EXAMPLE_ON_VIO_83M

#ifdef EXT_FLASH_EXAMPLE_ON_VIO_83M
#ifdef CONFIG_FIBOCOM_EXTFLASH_FROM_LCD
#ifdef CONFIG_FIBOCOM_EXTFLASH_3_0V
        halIomuxSetFunction(HAL_IOMUX_FUN_GPIO_18_PAD_GPIO_18);
        halIomuxSetFunction(HAL_IOMUX_FUN_GPIO_19_PAD_GPIO_19);
        halIomuxSetFunction(HAL_IOMUX_FUN_GPIO_20_PAD_GPIO_20);
        halIomuxSetFunction(HAL_IOMUX_FUN_GPIO_21_PAD_GPIO_21);
        halIomuxSetFunction(HAL_IOMUX_FUN_GPIO_22_PAD_GPIO_22);
        halIomuxSetFunction(HAL_IOMUX_FUN_GPIO_23_PAD_GPIO_23);

        halGpioSetInput(18);
        halGpioSetInput(19);
        halGpioSetInput(20);
        halGpioSetInput(21);
        halGpioSetInput(22);
        halGpioSetInput(23);

        *(&hwp_iomux->pad_gpio_0) = (0 << 2) ;
        hwp_gpio1->gpio_oen_set_out = (1 << 0);
        hwp_gpio1->gpio_set_reg = (1 << 0);

        // config power
        halPmuSwitchPower(HAL_POWER_LCD, true, true);
        halPmuSetPowerLevel(HAL_POWER_LCD, POWER_LEVEL_3200MV);

        // config pin
#ifndef CONFIG_SPECIAL_SPIFLASH_SUPPORT_TWO_LINE
        hwp_iomux->pad_lcd_fmark = 0x6fe000;
        hwp_iomux->pad_spi_lcd_select = 0x6fe000;
#endif
        hwp_iomux->pad_spi_lcd_cs = 0x6fe000;
        hwp_iomux->pad_spi_lcd_clk = 0x6fe000;
        hwp_iomux->pad_spi_lcd_sdc = 0x6fe000;
        hwp_iomux->pad_spi_lcd_sio = 0x6fe000;

        halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_CLK_PAD_SPI_LCD_SIO);
        halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_CS_PAD_SPI_LCD_SDC);
        halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_SIO_0_PAD_SPI_LCD_CLK);
        halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_SIO_1_PAD_SPI_LCD_CS);
#ifndef CONFIG_SPECIAL_SPIFLASH_SUPPORT_TWO_LINE
        halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_SIO_2_PAD_SPI_LCD_SELECT);
        halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_SIO_3_PAD_LCD_FMARK);
#endif
        //enable spiflash en
        REGT_FIELD_CHANGE(hwp_apApb->clk_ap_en0, REG_AP_APB_CLK_AP_EN0_T, clk_en_spiflash2_fr, 1);
        //sel spiflash clksource apa5 500M
        REGT_FIELD_CHANGE(hwp_apClk->cgm_spiflash2_sel_cfg, REG_AP_CLK_CGM_SPIFLASH2_SEL_CFG_T, cgm_spiflash2_sel, 0x4);
        //div apa5 500M to 6 200M for spiflash 500M -> 166M
        REGT_FIELD_CHANGE(hwp_apApb->cfg_clk_spiflash2, REG_AP_APB_CFG_CLK_SPIFLASH2_T, spiflash2_freq, 0x5);

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
#elif defined(CONFIG_FIBOCOM_EXTFLASH_1_8V)
        halIomuxSetFunction(HAL_IOMUX_FUN_GPIO_18_PAD_GPIO_18);
        halIomuxSetFunction(HAL_IOMUX_FUN_GPIO_19_PAD_GPIO_19);
        halIomuxSetFunction(HAL_IOMUX_FUN_GPIO_20_PAD_GPIO_20);
        halIomuxSetFunction(HAL_IOMUX_FUN_GPIO_21_PAD_GPIO_21);
        halIomuxSetFunction(HAL_IOMUX_FUN_GPIO_22_PAD_GPIO_22);
        halIomuxSetFunction(HAL_IOMUX_FUN_GPIO_23_PAD_GPIO_23);
  
        halGpioSetInput(18);
        halGpioSetInput(19);
        halGpioSetInput(20);
        halGpioSetInput(21);
        halGpioSetInput(22);
        halGpioSetInput(23);
        halPmuSwitchPower(HAL_POWER_LCD, true, true);
        halPmuSetPowerLevel(HAL_POWER_LCD, POWER_LEVEL_1800MV);
 #ifndef CONFIG_SPECIAL_SPIFLASH_SUPPORT_TWO_LINE
       hwp_iomux->pad_lcd_fmark = 0x6fe000;
       hwp_iomux->pad_spi_lcd_select = 0x6fe000;
  #endif
       hwp_iomux->pad_spi_lcd_cs = 0x6fe000;
       hwp_iomux->pad_spi_lcd_clk = 0x6fe000;
       hwp_iomux->pad_spi_lcd_sdc = 0x6fe000;
       hwp_iomux->pad_spi_lcd_sio = 0x6fe000;
  
       halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_CLK_PAD_SPI_LCD_SIO);
       halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_CS_PAD_SPI_LCD_SDC);
       halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_SIO_0_PAD_SPI_LCD_CLK);
       halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_SIO_1_PAD_SPI_LCD_CS);
#ifndef CONFIG_SPECIAL_SPIFLASH_SUPPORT_TWO_LINE
        halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_SIO_2_PAD_SPI_LCD_SELECT);
       halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_SIO_3_PAD_LCD_FMARK);
#endif
        //enable spiflash en
        REGT_FIELD_CHANGE(hwp_apApb->clk_ap_en0, REG_AP_APB_CLK_AP_EN0_T, clk_en_spiflash2_fr, 1);
        //sel spiflash clksource apa5 500M
        REGT_FIELD_CHANGE(hwp_apClk->cgm_spiflash2_sel_cfg, REG_AP_CLK_CGM_SPIFLASH2_SEL_CFG_T, cgm_spiflash2_sel, 0x4);
        //div apa5 500M to 6 200M for spiflash 500M -> 166M
        REGT_FIELD_CHANGE(hwp_apApb->cfg_clk_spiflash2, REG_AP_APB_CFG_CLK_SPIFLASH2_T, spiflash2_freq, 0xb);

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
#endif
#elif defined(CONFIG_FIBOCOM_EXTFLASH_FROM_MMC)
    // cancel GPIO21 pin init as flash function
    halIomuxSetFunction(HAL_IOMUX_FUN_GPIO_21_PAD_GPIO_21);
    // pin function
    halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_CLK_PAD_SDMMC1_DATA_2);
    halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_CS_PAD_SDMMC1_DATA_3);
    halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_SIO_0_PAD_SDMMC1_DATA_4);
    halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_SIO_1_PAD_SDMMC1_DATA_5);

    // pin driving stength
    halIomuxSetPadDriving(HAL_IOMUX_FUN_SPI_FLASH1_CLK_PAD_SDMMC1_DATA_2, 8); // range: 0~15
    halIomuxSetPadDriving(HAL_IOMUX_FUN_SPI_FLASH1_CS_PAD_SDMMC1_DATA_3, 8);
    halIomuxSetPadDriving(HAL_IOMUX_FUN_SPI_FLASH1_SIO_0_PAD_SDMMC1_DATA_4, 8);
    halIomuxSetPadDriving(HAL_IOMUX_FUN_SPI_FLASH1_SIO_1_PAD_SDMMC1_DATA_5, 8);

    #ifndef CONFIG_SPECIAL_SPIFLASH_SUPPORT_TWO_LINE
    // pin function
    halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_SIO_2_PAD_SDMMC1_DATA_6);
    halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_SIO_3_PAD_SDMMC1_DATA_7);
    
    // pin driving stength
    halIomuxSetPadDriving(HAL_IOMUX_FUN_SPI_FLASH1_SIO_2_PAD_SDMMC1_DATA_6, 8);
    halIomuxSetPadDriving(HAL_IOMUX_FUN_SPI_FLASH1_SIO_3_PAD_SDMMC1_DATA_7, 8);
    #endif
    
    // config power
    #ifdef CONFIG_FIBOCOM_EXTFLASH_3_0V
    halPmuSwitchPower(HAL_POWER_SD, true, true);
    halPmuSetPowerLevel(HAL_POWER_SD, POWER_LEVEL_3200MV);
    #else
    halPmuSwitchPower(HAL_POWER_SD, true, true);
    halPmuSetPowerLevel(HAL_POWER_SD, POWER_LEVEL_1800MV);
    #endif
    
    //enable spiflash en
    REGT_FIELD_CHANGE(hwp_apApb->clk_ap_en0, REG_AP_APB_CLK_AP_EN0_T, clk_en_spiflash2_fr, 1);
    //sel spiflash clksource apa5 500M
    REGT_FIELD_CHANGE(hwp_apClk->cgm_spiflash2_sel_cfg, REG_AP_CLK_CGM_SPIFLASH2_SEL_CFG_T, cgm_spiflash2_sel, 0x4);
    //div apa5 500M to 6 200M for spiflash 500M -> 166M
    REGT_FIELD_CHANGE(hwp_apApb->cfg_clk_spiflash2, REG_AP_APB_CFG_CLK_SPIFLASH2_T, spiflash2_freq, 0xb);

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

#else
#ifndef CONFIG_SPECIAL_SPIFLASH_SUPPORT_TWO_LINE

        hwp_iomux->pad_gpio_18 = 0x1fe000;
        hwp_iomux->pad_gpio_19 = 0x1fe000;
        hwp_iomux->pad_gpio_20 = 0x1fe000;
        hwp_iomux->pad_gpio_21 = 0x1fe000;
        hwp_iomux->pad_gpio_22 = 0x1fe000;
        hwp_iomux->pad_gpio_23 = 0x1fe000;
        halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_CLK_PAD_GPIO_18);
        halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_CS_PAD_GPIO_19);
        halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_SIO_0_PAD_GPIO_20);
        halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_SIO_1_PAD_GPIO_21);
        halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_SIO_2_PAD_GPIO_22);
        halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_SIO_3_PAD_GPIO_23);
#else
        hwp_iomux->pad_gpio_18 = 0x1fe000;
        hwp_iomux->pad_gpio_19 = 0x1fe000;
        hwp_iomux->pad_gpio_20 = 0x1fe000;
        hwp_iomux->pad_gpio_21 = 0x1fe000;
        halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_CLK_PAD_GPIO_18);
        halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_CS_PAD_GPIO_19);
        halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_SIO_0_PAD_GPIO_20);
        halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_SIO_1_PAD_GPIO_21);
#endif
        //enable spiflash en
        REGT_FIELD_CHANGE(hwp_apApb->clk_ap_en0, REG_AP_APB_CLK_AP_EN0_T, clk_en_spiflash2_fr, 1);
        //sel spiflash clksource apa5 500M
        REGT_FIELD_CHANGE(hwp_apClk->cgm_spiflash2_sel_cfg, REG_AP_CLK_CGM_SPIFLASH2_SEL_CFG_T, cgm_spiflash2_sel, 0x4);
        //div apa5 500M to 6 200M for spiflash 500M -> 166M
#if defined(CONFIG_MC665_CN_31_00_FYKJ)
        REGT_FIELD_CHANGE(hwp_apApb->cfg_clk_spiflash2, REG_AP_APB_CFG_CLK_SPIFLASH2_T, spiflash2_freq, 0x9);
#else
        REGT_FIELD_CHANGE(hwp_apApb->cfg_clk_spiflash2, REG_AP_APB_CFG_CLK_SPIFLASH2_T, spiflash2_freq, 0xb);
#endif

        REG_SPI_FLASH_SPI_CONFIG_T spi_config = {
#ifdef  CONFIG_SPECIAL_SPIFLASH_SUPPORT_TWO_LINE
            .b.quad_mode = 0,
            #if defined(CONFIG_MC665_CN_31_00_FYKJ)
            .b.clk_divider = 2,
            #else
            .b.clk_divider = 6,
            #endif
#else
            .b.quad_mode = 1,
            .b.clk_divider = 2,
#endif
            .b.sample_delay = 2,
        };
        hwp_spiFlashExt->spi_config = spi_config.v;
#endif
#endif // EXT_FLASH_EXAMPLE_ON_VIO_83M
#else
#if !(defined  CONFIG_MC665_CN_19_90_HANHUIWS || defined CONFIG_MC665_EU_19_90_HANHUIWS)
    halIomuxSetFunction(HAL_IOMUX_FUN_GPIO_18_PAD_GPIO_18);
    halIomuxSetFunction(HAL_IOMUX_FUN_GPIO_19_PAD_GPIO_19);
    halIomuxSetFunction(HAL_IOMUX_FUN_GPIO_20_PAD_GPIO_20);
    halIomuxSetFunction(HAL_IOMUX_FUN_GPIO_21_PAD_GPIO_21);
    halIomuxSetFunction(HAL_IOMUX_FUN_GPIO_22_PAD_GPIO_22);
    halIomuxSetFunction(HAL_IOMUX_FUN_GPIO_23_PAD_GPIO_23);

    halGpioSetInput(18);
    hwp_iomux->pad_gpio_18_set = 0x40;
    halGpioSetInput(19);
    hwp_iomux->pad_gpio_19_set = 0x40;
    halGpioSetInput(20);
    hwp_iomux->pad_gpio_20_set = 0x40;
    halGpioSetInput(21);
    hwp_iomux->pad_gpio_21_set = 0x40;
    halGpioSetInput(22);
    hwp_iomux->pad_gpio_22_set = 0x40;
    halGpioSetInput(23);
#endif
#endif
}


#ifdef CONFIG_FIBOCOM_BASE
bool fibo_halPmuExtFlashPowerOn(uint8_t pinsel, uint8_t val, uint8_t div)
{
    // div: 9  spiclk:1G/4/2   = 125MHz
    // div: 8  spiclk:1G/4.5/2 = 111MHz
    // div: 7  spiclk:1G/5/2   = 100MHz
    // div: 6  spiclk:1G/5.5/2 = 90MHz
    // div: 5  spiclk:1G/6/2   = 83MHz
    // div: 4  spiclk:1G/7/2   = 71MHz
    // div: 3  spiclk:1G/8/2   = 62MHz
    // div: 2  spiclk:1G/12/2  = 41MHz
    // div: 1  spiclk:1G/16/2  = 31MHz
    if ((div > 9)||(div == 0))
    {
        return false;
    }
    if (pinsel == 0) //V_LCD_18_33
    {
        if (val==0) //1.8v
        {
            hwp_iomux->pad_lcd_fmark = 0x6fe000;
            hwp_iomux->pad_spi_lcd_select = 0x6fe000;
            hwp_iomux->pad_spi_lcd_cs = 0x6fe000;
            hwp_iomux->pad_spi_lcd_clk = 0x6fe000;
            hwp_iomux->pad_spi_lcd_sdc = 0x6fe000;
            hwp_iomux->pad_spi_lcd_sio = 0x6fe000;

            halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_CLK_PAD_SPI_LCD_SIO);
            halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_CS_PAD_SPI_LCD_SDC);
            halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_SIO_0_PAD_SPI_LCD_CLK);
            halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_SIO_1_PAD_SPI_LCD_CS);

            if (spiflash_is_4line) {
                halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_SIO_2_PAD_SPI_LCD_SELECT);
                halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_SIO_3_PAD_LCD_FMARK);
            }

            // config power
            halPmuSwitchPower(HAL_POWER_LCD, true, true);
            halPmuSetPowerLevel(HAL_POWER_LCD, POWER_LEVEL_1800MV);

            osiDelayUS(300);

            //enable spiflash en
            REGT_FIELD_CHANGE(hwp_apApb->clk_ap_en0, REG_AP_APB_CLK_AP_EN0_T, clk_en_spiflash2_fr, 1);
            //sel spiflash clksource apa5 500M
            REGT_FIELD_CHANGE(hwp_apClk->cgm_spiflash2_sel_cfg, REG_AP_CLK_CGM_SPIFLASH2_SEL_CFG_T, cgm_spiflash2_sel, 0x4);
            //div apa5 500M to 6 200M for spiflash 500M -> 166M
            REGT_FIELD_CHANGE(hwp_apApb->cfg_clk_spiflash2, REG_AP_APB_CFG_CLK_SPIFLASH2_T, spiflash2_freq, 0xa); //æ¶éè¿é«å¯¼è´è¯»åæ¶åºé®é¢ï¼åºç°dump

            REG_SPI_FLASH_SPI_CONFIG_T spi_config = {
                //.b.quad_mode = 1,
                .b.sample_delay = 2,
                .b.clk_divider = 2,
            };

            OSI_LOGI(0,"EXT_FLASH spiflash_is_4line is %d",spiflash_is_4line);
            if(spiflash_is_4line) {
                spi_config.b.quad_mode = 1;
            }
            else {
                spi_config.b.quad_mode = 0;
            }
            hwp_spiFlashExt->spi_config = spi_config.v;

        }
        else if (val == 1) //3.2v
        {
            // config power
            halPmuSwitchPower(HAL_POWER_LCD, true, true);
            halPmuSetPowerLevel(HAL_POWER_LCD, POWER_LEVEL_3200MV);
            // config pin
            hwp_iomux->pad_lcd_fmark = 0x6fe000;
            hwp_iomux->pad_spi_lcd_select = 0x6fe000;
            hwp_iomux->pad_spi_lcd_cs = 0x6fe000;
            hwp_iomux->pad_spi_lcd_clk = 0x6fe000;
            hwp_iomux->pad_spi_lcd_sdc = 0x6fe000;
            hwp_iomux->pad_spi_lcd_sio = 0x6fe000;

            halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_CLK_PAD_SPI_LCD_SIO);
            halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_CS_PAD_SPI_LCD_SDC);
            halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_SIO_0_PAD_SPI_LCD_CLK);
            halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_SIO_1_PAD_SPI_LCD_CS);

            if (spiflash_is_4line) {
                halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_SIO_2_PAD_SPI_LCD_SELECT);
                halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_SIO_3_PAD_LCD_FMARK);
            }
            osiDelayUS(300);
            //enable spiflash en
            REGT_FIELD_CHANGE(hwp_apApb->clk_ap_en0, REG_AP_APB_CLK_AP_EN0_T, clk_en_spiflash2_fr, 1);
            //sel spiflash clksource apa5 500M
            REGT_FIELD_CHANGE(hwp_apClk->cgm_spiflash2_sel_cfg, REG_AP_CLK_CGM_SPIFLASH2_SEL_CFG_T, cgm_spiflash2_sel, 0x4);
            //div apa5 500M to 6 200M for spiflash 500M -> 166M
            #ifdef CONFIG_FIBOCUS_JIEKE_FEATURE
                REGT_FIELD_CHANGE(hwp_apApb->cfg_clk_spiflash2, REG_AP_APB_CFG_CLK_SPIFLASH2_T, spiflash2_freq, 0x5);
            #else
                REGT_FIELD_CHANGE(hwp_apApb->cfg_clk_spiflash2, REG_AP_APB_CFG_CLK_SPIFLASH2_T, spiflash2_freq, 0x6);
            #endif
            REG_SPI_FLASH_SPI_CONFIG_T spi_config = {
                //.b.quad_mode = 1,
                .b.sample_delay = 2,
                .b.clk_divider = 2,
            };
            OSI_LOGI(0,"EXT_FLASH spiflash_is_4line is %d",spiflash_is_4line);
            if(spiflash_is_4line) {
                spi_config.b.quad_mode = 1;
            }
            else {
                spi_config.b.quad_mode = 0;
            }
            hwp_spiFlashExt->spi_config = spi_config.v;
        }
        else
        {
            return false;
        }
    }
    else if (pinsel == 1) //pcm VDDIO_18_2
    {
        hwp_iomux->pad_gpio_18 = 0x1fe000;
        hwp_iomux->pad_gpio_19 = 0x1fe000;
        hwp_iomux->pad_gpio_20 = 0x1fe000;
        hwp_iomux->pad_gpio_21 = 0x1fe000;
        hwp_iomux->pad_gpio_22 = 0x1fe000;
        hwp_iomux->pad_gpio_23 = 0x1fe000;
        halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_CLK_PAD_GPIO_18);
        halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_CS_PAD_GPIO_19);
        halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_SIO_0_PAD_GPIO_20);
        halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_SIO_1_PAD_GPIO_21);
        halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_SIO_2_PAD_GPIO_22);
        halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_SIO_3_PAD_GPIO_23);
        //enable spiflash en
        REGT_FIELD_CHANGE(hwp_apApb->clk_ap_en0, REG_AP_APB_CLK_AP_EN0_T, clk_en_spiflash2_fr, 1);
        //sel spiflash clksource apa5 500M
        REGT_FIELD_CHANGE(hwp_apClk->cgm_spiflash2_sel_cfg, REG_AP_CLK_CGM_SPIFLASH2_SEL_CFG_T, cgm_spiflash2_sel, 0x4);
        //div apa5 500M to 6 200M for spiflash 500M -> 166M
        REGT_FIELD_CHANGE(hwp_apApb->cfg_clk_spiflash2, REG_AP_APB_CFG_CLK_SPIFLASH2_T, spiflash2_freq, 0xb);
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
    }
    else if (pinsel == 2) //VMMC_18_30
    {
        hwp_iomux->pad_gpio_18 = 0x1fe000;
        hwp_iomux->pad_gpio_19 = 0x1fe000;
        hwp_iomux->pad_gpio_20 = 0x1fe000;
        hwp_iomux->pad_gpio_21 = 0x1fe000;
        halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_CLK_PAD_GPIO_18);
        halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_CS_PAD_GPIO_19);
        halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_SIO_0_PAD_GPIO_20);
        halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_SIO_1_PAD_GPIO_21);
        //enable spiflash en
        REGT_FIELD_CHANGE(hwp_apApb->clk_ap_en0, REG_AP_APB_CLK_AP_EN0_T, clk_en_spiflash2_fr, 1);
        //sel spiflash clksource apa5 500M
        REGT_FIELD_CHANGE(hwp_apClk->cgm_spiflash2_sel_cfg, REG_AP_CLK_CGM_SPIFLASH2_SEL_CFG_T, cgm_spiflash2_sel, 0x4);
        //div apa5 500M to 6 200M for spiflash 500M -> 166M
        REGT_FIELD_CHANGE(hwp_apApb->cfg_clk_spiflash2, REG_AP_APB_CFG_CLK_SPIFLASH2_T, spiflash2_freq, 0x9);
        REG_SPI_FLASH_SPI_CONFIG_T spi_config = {
#ifdef  CONFIG_SPECIAL_SPIFLASH_SUPPORT_TWO_LINE
            .b.quad_mode = 0,
            .b.clk_divider = 2,
#endif
            .b.sample_delay = 2,
        };
        hwp_spiFlashExt->spi_config = spi_config.v;
    }
    else
    {
        return false;
    }
    return true;
}

bool fibo_halPmuExtFlashPowerOn_v2(uint8_t pinsel, uint8_t val, uint8_t div)
{
    // div: 9  spiclk:500M/2/2     = 125MHz
    // div: 8  spiclk:500M/2.5/2   = 100MHz
    // div: 7  spiclk:500M/2.5/2   = 100MHz
    // div: 6  spiclk:500M/3/2     = 83MHz
    // div: 5  spiclk:500M/3/2     = 83MHz
    // div: 4  spiclk:500M/3.5/2   = 71MHz
    // div: 3  spiclk:500M/4/2     = 62MHz
    // div: 2  spiclk:500M/6/2     = 41MHz
    // div: 1  spiclk:500M/8/2     = 31MHz

    uint32_t freq_div = 0;

    if ((pinsel != 0) && (pinsel != 1) && (pinsel != 2)) 
    {
        OSI_LOGE(0,"EXT_FLASH: err: invaild pinsel: %d", pinsel);
        return false;
    }

    if ((val != 0) && (val != 1)) 
    {
        OSI_LOGE(0,"EXT_FLASH: err: invaild val: %d", val);
        return false;
    }

    switch (div) 
    {
    case 1:
        freq_div = 3;
        break;

    case 2:
        freq_div = 5;
        break;

    case 3:
        freq_div = 9;
        break;

    case 4:
        freq_div = 0xa;
        break;

    case 5:
    case 6:
        freq_div = 0xb;
        break;

    case 7:
    case 8:
        freq_div = 0xc;
        break;

    case 9:
        freq_div = 0xd;
        break;

    default:
        OSI_LOGE(0,"EXT_FLASH: err: invaild div: %d", div);
        return false;
    }

    if (pinsel == 0) //V_LCD_18_33
    {
        // config pin
        hwp_iomux->pad_spi_lcd_cs = 0x6fe000;
        hwp_iomux->pad_spi_lcd_clk = 0x6fe000;
        hwp_iomux->pad_spi_lcd_sdc = 0x6fe000;
        hwp_iomux->pad_spi_lcd_sio = 0x6fe000;

        halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_CLK_PAD_SPI_LCD_SIO);
        halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_CS_PAD_SPI_LCD_SDC);
        halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_SIO_0_PAD_SPI_LCD_CLK);
        halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_SIO_1_PAD_SPI_LCD_CS);

        if (spiflash_is_4line) {
            hwp_iomux->pad_lcd_fmark = 0x6fe000;
            hwp_iomux->pad_spi_lcd_select = 0x6fe000;

            halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_SIO_2_PAD_SPI_LCD_SELECT);
            halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_SIO_3_PAD_LCD_FMARK);
        }

        // config power
        halPmuSwitchPower(HAL_POWER_LCD, true, true);
        if (val==0) //1.8v
        {
            halPmuSetPowerLevel(HAL_POWER_LCD, POWER_LEVEL_1800MV);
        }
        else if (val == 1) //3.2v
        {
            halPmuSetPowerLevel(HAL_POWER_LCD, POWER_LEVEL_3200MV);
        }
        osiDelayUS(300);
    }
    else if (pinsel == 1) //pcm VDDIO_18_2
    {
        hwp_iomux->pad_gpio_18 = 0x1fe000;
        hwp_iomux->pad_gpio_19 = 0x1fe000;
        hwp_iomux->pad_gpio_20 = 0x1fe000;
        hwp_iomux->pad_gpio_21 = 0x1fe000;

        halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_CLK_PAD_GPIO_18);
        halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_CS_PAD_GPIO_19);
        halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_SIO_0_PAD_GPIO_20);
        halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_SIO_1_PAD_GPIO_21);

        if (spiflash_is_4line) {
            hwp_iomux->pad_gpio_22 = 0x1fe000;
            hwp_iomux->pad_gpio_23 = 0x1fe000;

            halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_SIO_2_PAD_GPIO_22);
            halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_SIO_3_PAD_GPIO_23);
        }
    }
    else if (pinsel == 2) // sdmmc pins used as flash pins
    {
        // cancel GPIO21 pin init as flash function
        halIomuxSetFunction(HAL_IOMUX_FUN_GPIO_21_PAD_GPIO_21);

        // pin function
        halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_CLK_PAD_SDMMC1_DATA_2);
        halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_CS_PAD_SDMMC1_DATA_3);
        halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_SIO_0_PAD_SDMMC1_DATA_4);
        halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_SIO_1_PAD_SDMMC1_DATA_5);
        
        // pin driving stength
        halIomuxSetPadDriving(HAL_IOMUX_FUN_SPI_FLASH1_CLK_PAD_SDMMC1_DATA_2, 8); // range: 0~15
        halIomuxSetPadDriving(HAL_IOMUX_FUN_SPI_FLASH1_CS_PAD_SDMMC1_DATA_3, 8);
        halIomuxSetPadDriving(HAL_IOMUX_FUN_SPI_FLASH1_SIO_0_PAD_SDMMC1_DATA_4, 8);
        halIomuxSetPadDriving(HAL_IOMUX_FUN_SPI_FLASH1_SIO_1_PAD_SDMMC1_DATA_5, 8);
        
        if (spiflash_is_4line) {
            // pin function
            halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_SIO_2_PAD_SDMMC1_DATA_6);
            halIomuxSetFunction(HAL_IOMUX_FUN_SPI_FLASH1_SIO_3_PAD_SDMMC1_DATA_7);
            
            // pin driving stength
            halIomuxSetPadDriving(HAL_IOMUX_FUN_SPI_FLASH1_SIO_2_PAD_SDMMC1_DATA_6, 8);
            halIomuxSetPadDriving(HAL_IOMUX_FUN_SPI_FLASH1_SIO_3_PAD_SDMMC1_DATA_7, 8);
        }
        
        // config power
        halPmuSwitchPower(HAL_POWER_SD, true, true);
        if (val==0) //1.8v
        {
            halPmuSetPowerLevel(HAL_POWER_SD, POWER_LEVEL_1800MV);
        }
        else if (val == 1) //3.2v
        {
            halPmuSetPowerLevel(HAL_POWER_SD, POWER_LEVEL_3200MV);
            
        }
        osiDelayUS(300);
    }

    //enable spiflash en
    REGT_FIELD_CHANGE(hwp_apApb->clk_ap_en0, REG_AP_APB_CLK_AP_EN0_T, clk_en_spiflash2_fr, 1);
    //sel spiflash clksource apa5 500M
    REGT_FIELD_CHANGE(hwp_apClk->cgm_spiflash2_sel_cfg, REG_AP_CLK_CGM_SPIFLASH2_SEL_CFG_T, cgm_spiflash2_sel, 0x4);
    //div apa5 500M to 6 200M for spiflash 500M -> 166M
    REGT_FIELD_CHANGE(hwp_apApb->cfg_clk_spiflash2, REG_AP_APB_CFG_CLK_SPIFLASH2_T, spiflash2_freq, freq_div);

#ifdef CONFIG_SPECIAL_SPIFLASH_SUPPORT_TWO_LINE
    REG_SPI_FLASH_SPI_CONFIG_T spi_config = {
        .b.quad_mode = 0,
        .b.sample_delay = 2,
        .b.clk_divider = 2,
    };
#else
    OSI_LOGI(0,"EXT_FLASH: is 4line: %d",spiflash_is_4line);

    REG_SPI_FLASH_SPI_CONFIG_T spi_config = {
        .b.sample_delay = 2,
        .b.clk_divider = 2,
    };

    if(spiflash_is_4line) {
        spi_config.b.quad_mode = 1;
    }
    else {
        spi_config.b.quad_mode = 0;
    }
#endif
    hwp_spiFlashExt->spi_config = spi_config.v;

    return true;
}
#endif




#ifdef CONFIG_BOOT_SMPL_ENABLE
static bool prvSmplEnable(void)
{
}
#endif

void halPmuEnableDropProtect(void)
{
    REG_PMIC_RTC_ANA_SMPL_CTRL0_T anasmpl_ctrl0;
    anasmpl_ctrl0.b.smpl_mode = 0x1935;
    prvUnlockPowerReg();
    halAdiBusWrite(&hwp_pmicRtcAna->smpl_ctrl0, anasmpl_ctrl0.v);
}

uint32_t halPmuReadPsmInitCnt(void)
{
    uint32_t cnt_init = halAdiBusRead(&hwp_pmicPsm->psm_cnt_update_l_value);
    uint32_t cnt;
    OSI_LOOP_WAIT((cnt = halAdiBusRead(&hwp_pmicPsm->psm_cnt_update_l_value)) != cnt_init);

    uint32_t cnt_h = halAdiBusRead(&hwp_pmicPsm->psm_cnt_update_h_value);
    uint32_t cnt_l = halAdiBusRead(&hwp_pmicPsm->psm_cnt_update_l_value);
    if (cnt_l != cnt)
        cnt_h = halAdiBusRead(&hwp_pmicPsm->psm_cnt_update_h_value);
    return (cnt_h << 16) | cnt_l;
}

int64_t halPmuRtcReadSecond(void)
{
    uint16_t prev_sec = halAdiBusRead(&hwp_pmicRtc->rtc_sec_cnt_value) & 0x3F;
    uint16_t day, hour, min, sec;
    for (;;)
    {
        day = halAdiBusRead(&hwp_pmicRtc->rtc_day_cnt_value) & 0xffff;
        hour = halAdiBusRead(&hwp_pmicRtc->rtc_hrs_cnt_value) & 0x3f;
        min = halAdiBusRead(&hwp_pmicRtc->rtc_min_cnt_value) & 0x3f;
        sec = halAdiBusRead(&hwp_pmicRtc->rtc_sec_cnt_value) & 0x3f;
        if (sec == prev_sec)
            break;
        prev_sec = sec;
    }
    return day * DAY_SECONDS + hour * HOUR_SECONDS + min * MIN_SECONDS + sec;
}

void halPmuCheckPsm(void)
{
    REG_PMIC_RTC_ANA_MODULE_EN0_T module_en0;

    // enable PSM module
    REG_ADI_CHANGE1(hwp_pmicRtcAna->module_en0, module_en0, psm_topa_en, 1);

    uint32_t psm_magic = halAdiBusRead(&PSM_MODE_REG);
    OSI_LOGD(0x100092c6, "pmu: psm magic 0x%x", psm_magic);
    OSI_LOGI(0, "pmu: psm_ctrl 0x%x", halAdiBusRead(&hwp_pmicPsm->psm_ctrl));
    OSI_LOGI(0, "pmu: psm_status 0x%x", halAdiBusRead(&hwp_pmicPsm->psm_status));
    OSI_LOGI(0, "pmu: psm_fsm_status 0x%x", halAdiBusRead(&hwp_pmicPsm->psm_fsm_status));

    int64_t epoch;
    if (psm_magic == OSI_SHUTDOWN_PSM_SLEEP || psm_magic == OSI_SHUTDOWN_POWER_OFF)
    {
        OSI_LOGI(0x1000a9d1, "PSM_CNT_US %d", PSM_CNT_US);
        int64_t psm_cnt = halPmuReadPsmInitCnt();
        int64_t sleep_time = psm_cnt * PSM_CNT_US + 1320000;
        osiSetUpTimeUS(psm_cnt * PSM_CNT_US);

        if (psm_magic == OSI_SHUTDOWN_PSM_SLEEP)
            osiSetBootMode(OSI_BOOTMODE_PSM_RESTORE);

        OSI_LOGD(0x100092c7, "pmu: wakeup cnt/%u", (uint32_t)psm_cnt);

        // clear it, and it will only be set before PSM power off
        halAdiBusWrite(&PSM_MODE_REG, 0);
        const uint16_t epoch_l = halAdiBusRead(&PSM_EPOCH_LOW) & 0xffff;
        const uint16_t epoch_h = halAdiBusRead(&PSM_EPOCH_HIGH) & 0xffff;
        epoch = ((int64_t)((epoch_h << 16) | epoch_l) + CONFIG_KERNEL_MIN_UTC_SECOND) * 1000 * 1000 +
                sleep_time;
        osiSetEpochTimeUS(epoch);
    }
    else if (psm_magic == OSI_SHUTDOWN_OPENCPU_PSM_SLEEP)
    {
        osiSetBootMode(OSI_BOOTMODE_OPENCPU_PSM_RESTORE);
    }
    else
    {
        epoch = (halPmuRtcReadSecond() + CONFIG_KERNEL_MIN_UTC_SECOND) * 1000;
        osiSetEpochTime(epoch); //use this prevent out of range
    }

    // disable PSM module
    REG_ADI_CHANGE1(hwp_pmicRtcAna->module_en0, module_en0, psm_topa_en, 0);
}

void halBootCauseMode(void)
{
    REG_PMIC_RTC_ANA_POR_7S_CTRL_T rtc_ana_por_7s_ctrl;
    REG_PMIC_RTC_ANA_POR_SRC_FLAG_T rtc_ana_por_src_flag;
    REG_PMIC_RTC_ANA_MODULE_EN0_T rtc_ana_module_en0;
    REG_PMIC_RTC_ANA_RTC_CLK_EN0_T rtc_ana_rtc_clk_en0;
    REG_PMIC_RTC_ANA_SWRST_CTRL0_T rtc_ana_swrst_ctrl0;
    REG_PMIC_ANA_CHGR_STATUS_T ana_chgr_status;
    REG_PMIC_RTC_ANA_POR_OFF_FLAG_T ana_por_off_flag;

    ana_por_off_flag.v = halAdiBusRead(&hwp_pmicRtcAna->por_off_flag);
    ana_chgr_status.v = halAdiBusRead(&hwp_pmicAna->chgr_status);
    rtc_ana_swrst_ctrl0.v = halAdiBusRead(&hwp_pmicRtcAna->swrst_ctrl0);
    rtc_ana_por_src_flag.v = halAdiBusRead(&hwp_pmicRtcAna->por_src_flag);
    OSI_LOGI(0x100092c8, "pmu: boot por_src_flag 0x%08x, swrst_ctrl0 0x%x",
             rtc_ana_por_src_flag.v, rtc_ana_swrst_ctrl0.b.reg_rst_en);
    OSI_LOGI(0x1000a9cd, "pmu: boot por_off_flag 0x%08x", ana_por_off_flag.v);

    REG_ADI_CHANGE6(hwp_pmicRtcAna->por_off_flag, ana_por_off_flag,
                    otp_chip_pd_flag_clr, 1,
                    hw_chip_pd_flag_clr, 1,
                    sw_chip_pd_flag_clr, 1,
                    sw_chip_pd_flag_clr, 1,
                    uvlo_chip_pd_flag_clr, 1,
                    por_chip_pd_flag_clr, 1);

    if (rtc_ana_swrst_ctrl0.b.reg_rst_en == 0)
    {
        if (rtc_ana_por_src_flag.v & (1 << 11))
            osiSetBootCause(OSI_BOOTCAUSE_PIN_RESET);

        if ((rtc_ana_por_src_flag.v & (1 << 12)) ||
            (rtc_ana_por_src_flag.v & (1 << 8)))
            osiSetBootCause(OSI_BOOTCAUSE_PWRKEY);

        if (rtc_ana_por_src_flag.v & (1 << 6))
            osiSetBootCause(OSI_BOOTCAUSE_ALARM);

#ifdef CONFIG_FIBOCOM_BASE
 //MTC0683-210
        if ((rtc_ana_por_src_flag.v & (1 << 10)) ||
            (rtc_ana_por_src_flag.v & (1 << 5))  ||
            (rtc_ana_por_src_flag.v & (1 << 4)))
            osiSetBootCause(OSI_BOOTCAUSE_CHARGE);
#endif
    }
    else
    {
        // clear soft reset
        REG_ADI_CHANGE1(hwp_pmicRtcAna->swrst_ctrl0, rtc_ana_swrst_ctrl0, reg_rst_en, 0);
    }

    if (ana_chgr_status.b.chgr_int && ana_chgr_status.b.chgr_on)
        osiSetBootCause(OSI_BOOTCAUSE_CHARGE);

    REG_ADI_CHANGE1(hwp_pmicRtcAna->por_src_flag, rtc_ana_por_src_flag,
                    reg_soft_rst_flag_clr, 1); // clear bit14

    REG_ADI_CHANGE4(hwp_pmicRtcAna->por_7s_ctrl, rtc_ana_por_7s_ctrl,
                    pbint_7s_flag_clr, 1, // clear bit12
                    ext_rstn_flag_clr, 1, // clear bit11
                    chgr_int_flag_clr, 1, // clear bit10
                    pbint_flag_clr, 1     // clear bit8
    );

    osiDelayUS(100);

    // clear the clear bits
    REG_ADI_CHANGE4(hwp_pmicRtcAna->por_7s_ctrl, rtc_ana_por_7s_ctrl,
                    pbint_7s_flag_clr, 0,
                    ext_rstn_flag_clr, 0,
                    chgr_int_flag_clr, 0,
                    pbint_flag_clr, 0);

    // enable watchdog to check watchdog reset
    halAdiBusWrite(&hwp_pmicWdt->wdg_ctrl, 0); // clear wdg_run
    REG_ADI_CHANGE1(hwp_pmicRtcAna->module_en0, rtc_ana_module_en0, wdg_en, 1);
    REG_ADI_CHANGE1(hwp_pmicRtcAna->rtc_clk_en0, rtc_ana_rtc_clk_en0, rtc_wdg_en, 1);

    REG_PMIC_WDT_WDG_INT_RAW_T wdt_wdg_int_raw;
    wdt_wdg_int_raw.v = halAdiBusRead(&hwp_pmicWdt->wdg_int_raw);
    if (wdt_wdg_int_raw.b.wdg_rst_raw)
    {
        osiSetBootCause(OSI_BOOTCAUSE_WDG);

        REG_PMIC_WDT_WDG_INT_CLR_T wdg_int_clr;
        REG_ADI_CHANGE1(hwp_pmicWdt->wdg_int_clr, wdg_int_clr, wdg_rst_clr, 1);
        OSI_LOGI(0x10009077, "pmic wdt reset sucess");
    }

    // disable watchdog and it should re-enable if used
    REG_ADI_CHANGE1(hwp_pmicRtcAna->module_en0, rtc_ana_module_en0, wdg_en, 0);
    REG_ADI_CHANGE1(hwp_pmicRtcAna->rtc_clk_en0, rtc_ana_rtc_clk_en0, rtc_wdg_en, 0);

    osiBootMode_t boot_mode = osiGetBootMode();
    uint32_t boot_mode_val = halAdiBusRead(&PMU_BOOT_MODE_REG);

    REG_LPS_APB_RESET_CAUSE_T reset_cause = {hwp_lpsApb->reset_cause}; //this register not same as 8910
    rtc_ana_por_src_flag.v = halAdiBusRead(&hwp_pmicRtcAna->por_src_flag);
    OSI_LOGD(0x100092c9, "pmu: boot mode magic 0x%x, por_src_flag 0x%x, wdg_int_raw 0x%x reset_cause 0x%x, boot_cause 0x%x",
             boot_mode_val, rtc_ana_por_src_flag.v, wdt_wdg_int_raw.v, reset_cause.v, osiGetBootCauses());

    if (boot_mode == OSI_BOOTMODE_NORMAL)
    {
        switch (boot_mode_val)
        {
        case OSI_SHUTDOWN_PANIC:
            // Panic reset is a special case. The information is stored in
            // boot mode register, but it is a boot cause.
            osiSetBootCause(OSI_BOOTCAUSE_PANIC);
            break;

        case OSI_BOOTMODE_DOWNLOAD:
        case OSI_BOOTMODE_CALIB:
        case OSI_BOOTMODE_CALIB_POST:
        case OSI_BOOTMODE_NB_CALIB:
        case OSI_BOOTMODE_BBAT:
        case OSI_BOOTMODE_UPGRADE:
        case OSI_BOOTMODE_PSM_RESTORE:
            osiSetBootMode(boot_mode_val);
            break;
        case OSI_BOOTMODE_OPENCPU_PSM_RESTORE:
            osiSetBootMode(boot_mode_val);
            break;
        default:
            break;
        }
    }
    halAdiBusWrite(&PMU_BOOT_MODE_REG, 0);
}
#ifdef CONFIG_FIBOCOM_BASE
void halPmuBatCrashVolSetting(uint8_t vol)
{
    REG_PMIC_RTC_ANA_POWON_CTRL_T powon_ctrl = {};
    powon_ctrl.v = halAdiBusRead(&hwp_pmicRtcAna->powon_ctrl);
    powon_ctrl.b.rg_vbat_crash_v = vol;
    halAdiBusWrite(&hwp_pmicRtcAna->powon_ctrl, powon_ctrl.v);
}
#endif
void halPmuInit(unsigned whence)
{
    // pmic config is same as cp, as 8910
    REGT_FIELD_CHANGE(hwp_idleLps->ap_lps_sig_time, REG_IDLE_LPS_AP_LPS_SIG_TIME_T, ap_t1, 55, ap_t2, 3, ap_t3, 10, ap_t4, 15);

    REGT_FIELD_CHANGE(hwp_idleLps->pm2_off_time, REG_IDLE_LPS_PM2_OFF_TIME_T, n1, 16, n2, 4, n3, 2, n4, 1);

    REGT_FIELD_CHANGE(hwp_idleLps->pm2_on_time, REG_IDLE_LPS_PM2_ON_TIME_T, p2, 2, p3, 2, p4, 2);

    REGT_FIELD_CHANGE(hwp_idleLps->pm2_on_off_time, REG_IDLE_LPS_PM2_ON_OFF_TIME_T, n5, 4, n6, 16, p5, 1, p6, 32);

    REGT_FIELD_CHANGE(hwp_idleLps->sleep_prot_time, REG_IDLE_LPS_SLEEP_PROT_TIME_T, sleep_prot_time, 39);

    halAdiBusWrite(&hwp_pmicRtcAna->reserved_reg4, 0x3026);
#ifdef CONFIG_FIBOCOM_BASE
//2024-08-01, kangmengjiang, MTC0745-440.pmic determines that the shutdown voltage of power failure is raised to
//improve the stability of CPU operation in scenarios where power is quickly cut off for SPCSS01351167.
    halPmuBatCrashVolSetting(3);
//2024-08-01, kangmengjiang, MTC0745-440.End.
#endif
    // pm2 relative register
    hwp_adiMst->adi_chanel_en = 0x8;    //idst_aon
    hwp_adiMst->event1_waddr = 0x918;   //pmic reserved reg6 write 1
    hwp_adiMst->event1_wdata = 0x10000; //write 1
    hwp_aonClk->cgm_adi_sel_cfg = 0x1;  //xtal_26m

    REGT_FIELD_CHANGE(hwp_lpsApb->aon_ahb_lp_ctrl, REG_LPS_APB_AON_AHB_LP_CTRL_T, lps_ahb_dslp_ctrl_en, 1);

    hwp_lpsApb->cfg_io_deep_sleep = 0x30fff;
    hwp_lpsApb->aon_iram_ctrl = 0x5407;
    prvPinDeepSleepDisable();
    prvPinPropertySet();

    prvUnlockPowerReg();
#ifndef CONFIG_OPENCPU_ENABLED
    REG_PMIC_RTC_ANA_KPLED_CTRL0_T keled_ctrl_ref;
    drvEfusePmicOpen();
    drvEfusePmicRead(13, &key_ref);
    key_ref = (key_ref >> 8) & 0x1f;
    REG_ADI_CHANGE1(hwp_pmicRtcAna->kpled_ctrl0, keled_ctrl_ref, rg_ldo_kpled_reftrim, key_ref);
    drvEfusePmicClose();
#endif
    halPmuSwitchPower(HAL_POWER_KEYLED, true, true);
    halAdiBusWrite(&hwp_pmicAna->vcore_ctrl3, 0xc00);

#if 0
    halPmuSwitchPower(HAL_POWER_RF12, true, false);
    halPmuSwitchPower(HAL_POWER_RF15, true, false);
#endif
    halPmuSwitchPower(HAL_POWER_DCDC_VRF, true, false);

    halAdiBusWrite(&hwp_pmicRtcAna->pm2_pd_en, 0x7917);

    REG_PMIC_RTC_ANA_CLK32KLESS_CTRL0_T clk32kless_ctrl0;
    clk32kless_ctrl0.v = halAdiBusRead(&hwp_pmicRtcAna->clk32kless_ctrl0);
    if (clk32kless_ctrl0.b.rtc_mode == 1)
    {
        hwp_lpsApb->rc26m_pu_ctrl = 0x1;
        halPmuSwitchPower(HAL_POWER_DCXO, true, true);
        halPmuSwitchPowerPm2(HAL_POWER_DCXO, true);
    }
    else
    {
        halPmuSwitchPower(HAL_POWER_DCXO, true, false);
        halPmuSwitchPowerPm2(HAL_POWER_DCXO, false);
    }

    REG_PMIC_RTC_ANA_VGEN_CTRL0_T vgen_ctrl0;
    vgen_ctrl0.v = halAdiBusRead(&hwp_pmicRtcAna->vgen_ctrl0);
    vgen_ctrl0.b.slp_ldo_mem_powersel_en = 1;
    vgen_ctrl0.b.pm2_ldo_mem_powersel = 1;
    halAdiBusWrite(&hwp_pmicRtcAna->vgen_ctrl0, vgen_ctrl0.v);
    halPmuSwitchPowerPm2(HAL_POWER_MEM, false);
    halPmuSwitchPowerPm2(HAL_POWER_DCDC_GEN, false);
    halPmuSwitchPowerPm2(HAL_POWER_VIO33, false);

#ifdef CONFIG_BOOT_SMPL_ENABLE
    if (!prvSmplEnable())
        OSI_LOGI(0x1000907e, "SMPL enable fail");
#endif
    prvResetPinEnable();

#ifdef CONFIG_SOC_8850
#ifdef CONFIG_FLASH_LP_POWER_DOWN
    halPmuSwitchPower(HAL_POWER_SPIMEM, true, false);
#else
    halPmuSwitchPower(HAL_POWER_SPIMEM, true, true);
#endif

#ifdef CONFIG_SOC_8850
    //kpled ldo ctrl
    hwp_mailbox->sysmail28 |= 0x1;
    hwp_mailbox->sysmail28 |= 0x2;
#endif
#if defined(CONFIG_FIBOCOM_MC660) && defined(CONFIG_FIBOCOM_BLE_SUPPORT)
    halPmuSwitchPower(HAL_POWER_SD, true, false);
    halPmuSetPowerLevel(HAL_POWER_SD, 1800);
#elif defined CONFIG_MC660_CN_26_90
    halPmuSwitchPower(HAL_POWER_SD, true, false);
#else
    halPmuSwitchPower(HAL_POWER_SD, false, false);
#endif

#ifdef CONFIG_FIBOCUS_JIEKE_FEATURE
    halPmuSwitchPower(HAL_POWER_LCD, true, true);
#elif defined (CONFIG_FIBOCOM_DIDI)
    halPmuSwitchPower(HAL_POWER_LCD, true, true);
#elif defined (CONFIG_FIBOCOM_VBUS_PIN_ONOFF)
    halPmuSwitchPower(HAL_POWER_LCD, true, false);
#else
    halPmuSwitchPower(HAL_POWER_LCD, false, false);
#endif
#ifdef CONFIG_MC665_CN_36_90_QDTIANYAN
    halPmuSwitchPower(HAL_POWER_VIO33, true, false);
#endif
#ifdef CONFIG_FIBOCOM_BASE
    if (1 == fibo_get_customize_info())
    {
        halPmuSwitchPower(HAL_POWER_VIO33, true, false);
    }
    else
    {
#ifdef CONFIG_FIBOCOM_INNER_CODEC_SUPPORT
        halPmuSwitchPower(HAL_POWER_VIO33, false, false);
#else
        halPmuSwitchPower(HAL_POWER_VIO33, true, false);
#endif
    }
#endif

#ifdef CONFIG_MC669_CN_25_99_XIAOAN
    halPmuSwitchPower(HAL_POWER_VIO33, false, false);  //power for codec of inner, close by default.
#endif
    halPmuSwitchPower(HAL_POWER_CAMD, false, false);
    if (whence == HAL_CLOCK_INIT_OPENCPU)
    {
        hwp_pwrctrl->rf_pwr_ctrl = 0;
        halPmuSwitchPower(HAL_POWER_RF12, true, false);
        halPmuSwitchPower(HAL_POWER_RF15, true, false);
        halPmuSwitchPower(HAL_POWER_USB, false, false);
        hwp_pwrctrl->usb_pwr_ctrl = 0;
        OSI_POLL_WAIT(hwp_pwrctrl->usb_pwr_stat == 0x2);
        hwp_lpsApb->cfg_por_usb_phy = 0xf;
        hwp_aonClk->cgm_usb_ahb_sel_cfg = 0x0;
        REGT_FIELD_CHANGE(hwp_sysCtrl->aon_clock_en1,
                          REG_SYS_CTRL_AON_CLOCK_EN1_T, usb_ref_en, 0);
        REG_PMIC_ANA_SIM_VPA_CTRL0_T sim_vpa_ctrl0;
        REG_ADI_CHANGE1(hwp_pmicAna->sim_vpa_ctrl0, sim_vpa_ctrl0,
                        rg_vpa_pd, 1);
        REG_PMIC_RTC_ANA_RESERVED_REG1_T reserved_reg1;
        REG_ADI_CHANGE1(hwp_pmicAna->reserved_reg1, reserved_reg1,
                        pm1_dvdd_en, 1);
        halAdiBusWrite(&hwp_pmicRtcAna->kpled_ctrl0, 0xb0);
    }
    else
    {
        halPmuSwitchPower(HAL_POWER_USB, true, false);
    }

    halPmuSwitchPower(HAL_POWER_AVDD18, true, false);
#ifdef CONFIG_CAMA_POWER_ON_INIT
    halPmuSwitchPower(HAL_POWER_CAMA, true, false);
#else
    halPmuSwitchPower(HAL_POWER_CAMA, false, false);
#endif
    halPmuSwitchPower(HAL_POWER_VIBR, false, false);
#ifdef CONFIG_FIBOCOM_BASE
    halPmuSwitchPower(HAL_POWER_BACK_LIGHT, false, false);
#endif
#ifdef CONFIG_BOARD_KPLED_USED_FOR_RF_SWITCH
    halPmuSwitchPower(HAL_POWER_KEYLED, true, false);
#else
    halPmuSwitchPower(HAL_POWER_KEYLED, false, false);
#endif

#ifdef CONFIG_FIBOCOM_BASE
    halPmuSwitchPower(HAL_POWER_SIM1, true, true);
#endif

    halAdiBusWrite(&hwp_pmicRtcAna->slp_ldo_lp_ctrl0, 0x7fff); // all '1'
    halAdiBusWrite(&hwp_pmicRtcAna->slp_ldo_lp_ctrl1, 0x6409); // all '1'
    halAdiBusWrite(&hwp_pmicRtcAna->slp_dcdc_lp_ctrl, 0x16);   // all '1'

    REG_PMIC_RTC_ANA_SLP_LDO_PD_CTRL1_T slp_ldo_pd_ctrl1;
    slp_ldo_pd_ctrl1.v = halAdiBusRead(&hwp_pmicRtcAna->slp_ldo_pd_ctrl1);
    slp_ldo_pd_ctrl1.b.pm1_ldocp_pd_en = 0;
    slp_ldo_pd_ctrl1.b.pm1_ldomem_pd_en = 0;
    slp_ldo_pd_ctrl1.b.ldo_xtl_en = 0;
    slp_ldo_pd_ctrl1.b.slp_io_en = 0;
    slp_ldo_pd_ctrl1.b.slp_ldo_pd_en = 1;
    halAdiBusWrite(&hwp_pmicRtcAna->slp_ldo_pd_ctrl1, slp_ldo_pd_ctrl1.v);

    REG_PMIC_RTC_ANA_DCDC_CORE_SLP_CTRL0_T dcdc_core_slp_ctrl0 = {
        .b.pm1_dcdc_core_slp_step_delay = 3,
        .b.pm1_dcdc_core_slp_step_num = 3,
        .b.pm1_dcdc_core_slp_step_vol = 0x10,
        .b.dcdc_core_slp_step_en = 1};
    halAdiBusWrite(&hwp_pmicRtcAna->dcdc_core_slp_ctrl0, dcdc_core_slp_ctrl0.v);

    REG_PMIC_RTC_ANA_DCDC_CORE_SLP_CTRL1_T dcdc_core_slp_ctrl1 = {
        .b.pm1_dcdc_core_vosel_ds_sw = 0xe0};
    halAdiBusWrite(&hwp_pmicRtcAna->dcdc_core_slp_ctrl1, dcdc_core_slp_ctrl1.v);

    halAdiBusWrite(&hwp_pmicRtcAna->ldo_vlg_sel0, 0x40);
    halAdiBusWrite(&hwp_pmicRtcAna->dcdc_ctrl1, 0x300);

    REG_PMIC_RTC_ANA_DCDC_VLG_SEL_T dcdc_vlg_sel;
    REG_ADI_CHANGE2(hwp_pmicRtcAna->dcdc_vlg_sel, dcdc_vlg_sel,
                    dcdc_core_nor_sw_sel, 1,
                    dcdc_core_slp_sw_sel, 1);

    REG_PMIC_RTC_ANA_RESERVED_REG1_T reserved_reg1 = {
        .b.pm2_dcdc_core_slp_step_vol = 0x9};
    halAdiBusWrite(&hwp_pmicRtcAna->reserved_reg1, reserved_reg1.v);

    REG_PMIC_RTC_ANA_RESERVED_REG2_T reserved_reg2 = {
        .b.ulp_cycle_sel0 = 2};
    halAdiBusWrite(&hwp_pmicRtcAna->reserved_reg2, reserved_reg2.v);

    REG_PMIC_ANA_VRF_CTRL0_T vrf_ctrl0 = {
        .b.rg_vrf_curlimit_r = 0x2};

    halAdiBusWrite(&hwp_pmicAna->vrf_ctrl0, vrf_ctrl0.v);

#if 0
    REG_PMIC_RTC_ANA_RESERVED_REG5_T reserved_reg5 = {
        .b.pm2_dcdc_core_slp_step_num = 4,
        .b.pm2_dcdc_core_slp_step_delay = 3};
    halAdiBusWrite(&hwp_pmicRtcAna->reserved_reg5, reserved_reg5.v);
#else
    halAdiBusWrite(&hwp_pmicRtcAna->reserved_reg5, 0x77f3);
#endif

    // rtc enable
    REG_PMIC_RTC_ANA_MODULE_EN0_T module_en0;
    REG_PMIC_RTC_ANA_RTC_CLK_EN0_T rtc_clk_en0;
    REG_PMIC_RTC_RTC_SPG_UPD_T rtc_spg_upd;
    halAdiBusBatchChange(
        &hwp_pmicRtcAna->module_en0,
        REG_FIELD_MASKVAL1(module_en0, rtc_en, 1),
        &hwp_pmicRtcAna->rtc_clk_en0,
        REG_FIELD_MASKVAL1(rtc_clk_en0, rtc_rtc_en, 1),
        &hwp_pmicRtc->rtc_int_en,
        HAL_ADI_BUS_OVERWITE(0),
        &hwp_pmicRtc->rtc_int_clr,
        HAL_ADI_BUS_OVERWITE(0xffff),
        &hwp_pmicRtc->rtc_spg_upd,
        REG_FIELD_MASKVAL1(rtc_spg_upd, rtc_almlock_upd, RTC_ALARM_UNLOCK_MAGIC),
        HAL_ADI_BUS_CHANGE_END);

    // check PSM module for boot cause/mode
    halPmuCheckPsm();

    halPmuEnableDropProtect();

    // check boot cause and mode
    halBootCauseMode();

    #ifndef CONFIG_FIBOCOM_BASE //SPCSS01180620  MTC0605-164
    if ((osiGetBootCauses() & OSI_BOOTCAUSE_PIN_RESET) == 0)
    {
        REG_PMIC_RTC_ANA_SLP_DCDC_PD_CTRL_T slp_dcdc_pd_ctrl;
        REG_ADI_CHANGE1(hwp_pmicRtcAna->slp_dcdc_pd_ctrl, slp_dcdc_pd_ctrl, slp_dcdccore_drop_en, 1);
    }
    else
    #endif
        bPowerByResetPin = true;

#endif
}

bool halPmuSwitchPower(uint32_t id, bool enabled, bool lp_enabled)
{
    REG_PMIC_RTC_ANA_POWER_PD_SW0_T ldo_sd_pd_sw0;
    REG_PMIC_RTC_ANA_POWER_PD_SW1_T ldo_sd_pd_sw1;
    REG_PMIC_RTC_ANA_SLP_LDO_PD_CTRL0_T slp_ldo_lp_pd_ctrl0;
    REG_PMIC_RTC_ANA_SLP_LDO_PD_CTRL1_T slp_ldo_lp_pd_ctrl1;
    REG_PMIC_ANA_SIM_VPA_CTRL0_T vpa_sim_ctrl0;
    REG_PMIC_BLTC_BLTC_CTL_T bltc_ctrl_t;
    REG_PMIC_ANA_MODULE_EN0_T ana_module_en0;
    REG_PMIC_ANA_KPLED_CTRL1_T kp_led_ctrl1;
    REG_PMIC_BLTC_BLTC_PD_CTRL_T bltc_pd_ctrl;
    REG_PMIC_BLTC_BLTC_R_ISAT_T rgb_r_isat;
    REG_PMIC_BLTC_BLTC_G_ISAT_T rgb_g_isat;
    REG_PMIC_BLTC_BLTC_B_ISAT_T rgb_b_isat;
    REG_PMIC_RTC_ANA_POWER_PD_SW1_T ldo_lcd_reg0;
    REG_PMIC_RTC_ANA_KPLED_CTRL0_T kpled_ctrl0;
    REG_PMIC_RTC_ANA_SLP_DCDC_PD_CTRL_T slp_dcdc_pd_c;
    REG_PMIC_RTC_ANA_SLP_LDO_PD_CTRL1_T slp_ldo_pd_ctr1;
    REG_PMIC_RTC_ANA_VGEN_CTRL0_T vgen_ctrl0;
#ifdef CONFIG_FIBOCOM_BASE
    REG_PMIC_ANA_LDO_SIM_CTRL1_T ldo_sim_ctrl1; //remove by fibocom for  compile warning at 20230417
#endif
    switch (id)
    {
    case HAL_POWER_MEM:
        prvUnlockPowerReg();
        REG_ADI_CHANGE1(hwp_pmicRtcAna->power_pd_sw0, ldo_sd_pd_sw0,
                        da_ldo_mem_pd, enabled ? 0 : 1);

        REG_ADI_CHANGE1(hwp_pmicRtcAna->slp_ldo_pd_ctrl1, slp_ldo_lp_pd_ctrl1,
                        pm1_ldomem_pd_en, lp_enabled ? 0 : 1);

        REG_ADI_CHANGE2(hwp_pmicRtcAna->vgen_ctrl0, vgen_ctrl0,
                        slp_ldo_mem_powersel_en, enabled ? 1 : 0, pm2_ldo_mem_powersel, enabled ? 1 : 0);
        break;

    case HAL_POWER_VIO18:
        prvUnlockPowerReg();
        REG_ADI_CHANGE1(hwp_pmicRtcAna->power_pd_sw0, ldo_sd_pd_sw0,
                        da_ldo_vio18_pd, enabled ? 0 : 1);
        REG_ADI_CHANGE1(hwp_pmicRtcAna->slp_ldo_pd_ctrl0, slp_ldo_lp_pd_ctrl0,
                        pm1_ldovio18_pd_en, lp_enabled ? 0 : 1);
        break;

    case HAL_POWER_VIO33:
        prvUnlockPowerReg();
        REG_ADI_CHANGE1(hwp_pmicRtcAna->power_pd_sw0, ldo_sd_pd_sw0,
                        da_ldo_vio33_pd, enabled ? 0 : 1);
        REG_ADI_CHANGE1(hwp_pmicRtcAna->slp_ldo_pd_ctrl0, slp_ldo_lp_pd_ctrl0,
                        pm1_ldovio33_pd_en, lp_enabled ? 0 : 1);
        break;

    case HAL_POWER_DCXO:
        prvUnlockPowerReg();
        REG_ADI_CHANGE1(hwp_pmicRtcAna->power_pd_sw0, ldo_sd_pd_sw0,
                        da_ldo_dcxo_pd, enabled ? 0 : 1);
        REG_ADI_CHANGE1(hwp_pmicRtcAna->slp_ldo_pd_ctrl0, slp_ldo_lp_pd_ctrl0,
                        pm1_ldodcxo_pd_en, lp_enabled ? 0 : 1);
        break;

    case HAL_POWER_AVDD18:
        prvUnlockPowerReg();
        REG_ADI_CHANGE1(hwp_pmicRtcAna->power_pd_sw0, ldo_sd_pd_sw0,
                        da_ldo_ana_pd, enabled ? 0 : 1);
        REG_ADI_CHANGE1(hwp_pmicRtcAna->slp_ldo_pd_ctrl0, slp_ldo_lp_pd_ctrl0,
                        slp_ldoana_pd_en, lp_enabled ? 0 : 1);
        break;

    case HAL_POWER_CAMD:
        prvUnlockPowerReg();
        REG_ADI_CHANGE1(hwp_pmicRtcAna->power_pd_sw1, ldo_sd_pd_sw1,
                        da_ldo_camd_pd, enabled ? 0 : 1);
        REG_ADI_CHANGE1(hwp_pmicRtcAna->slp_ldo_pd_ctrl0, slp_ldo_lp_pd_ctrl0,
                        slp_ldocamd_pd_en, lp_enabled ? 0 : 1);
        break;

    case HAL_POWER_CAMA:
        prvUnlockPowerReg();
        REG_ADI_CHANGE1(hwp_pmicRtcAna->power_pd_sw1, ldo_sd_pd_sw1,
                        da_ldo_cama_pd, enabled ? 0 : 1);
        REG_ADI_CHANGE1(hwp_pmicRtcAna->slp_ldo_pd_ctrl0, slp_ldo_lp_pd_ctrl0,
                        slp_ldocama_pd_en, lp_enabled ? 0 : 1);
        break;

    case HAL_POWER_LCD:
        prvUnlockPowerReg();
        REG_ADI_CHANGE1(hwp_pmicRtcAna->power_pd_sw1, ldo_lcd_reg0,
                        da_ldo_lcd_pd, enabled ? 0 : 1);
        REG_ADI_CHANGE1(hwp_pmicRtcAna->slp_ldo_pd_ctrl0, slp_ldo_lp_pd_ctrl0,
                        slp_ldolcd_pd_en, lp_enabled ? 0 : 1);
        break;

    case HAL_POWER_USB:
        prvUnlockPowerReg();
        REG_ADI_CHANGE1(hwp_pmicRtcAna->power_pd_sw0, ldo_sd_pd_sw0,
                        da_ldo_usb33_pd, enabled ? 0 : 1);
        REG_ADI_CHANGE1(hwp_pmicRtcAna->slp_ldo_pd_ctrl0, slp_ldo_lp_pd_ctrl0,
                        pm1_ldousb_pd_en, lp_enabled ? 0 : 1);
        break;

    case HAL_POWER_SD:
        prvUnlockPowerReg();
        REG_ADI_CHANGE1(hwp_pmicRtcAna->power_pd_sw0, ldo_sd_pd_sw0,
                        da_ldo_mmc_pd, enabled ? 0 : 1);

        REG_ADI_CHANGE1(hwp_pmicRtcAna->slp_ldo_pd_ctrl0, slp_ldo_lp_pd_ctrl0,
                        slp_ldommc_pd_en, lp_enabled ? 0 : 1);
        break;

    case HAL_POWER_EMM:
        prvUnlockPowerReg();
        REG_ADI_CHANGE1(hwp_pmicRtcAna->power_pd_sw0, ldo_sd_pd_sw0,
                        ldo_emm_pd, enabled ? 0 : 1);
        break;
    case HAL_POWER_CP:
        prvUnlockPowerReg();
        REG_ADI_CHANGE1(hwp_pmicRtcAna->power_pd_sw0, ldo_sd_pd_sw0,
                        ldo_cp_pd, enabled ? 0 : 1);

        REG_ADI_CHANGE1(hwp_pmicRtcAna->slp_ldo_pd_ctrl1, slp_ldo_pd_ctr1,
                        pm1_ldocp_pd_en, lp_enabled ? 0 : 1);
        break;

    case HAL_POWER_SIM0:
        // controlled by cp
#ifdef CONFIG_MODEM_LTE_LIB
        prvUnlockPowerReg();
        REG_ADI_CHANGE1(hwp_pmicAna->sim_vpa_ctrl0, vpa_sim_ctrl0,
                        da_ldo_sim0_pd, enabled ? 0 : 1);
        //REG_ADI_CHANGE1(hwp_pmicAna->sim_vpa_ctrl0, vpa_sim_ctrl0,
        //                rg_vpa_lp_en, lp_enabled ? 1 : 0);
#endif
        break;

    case HAL_POWER_SIM1:
        // controlled by cp
#ifdef CONFIG_MODEM_LTE_LIB
        prvUnlockPowerReg();
        REG_ADI_CHANGE1(hwp_pmicAna->sim_vpa_ctrl0, vpa_sim_ctrl0,
                        da_ldo_sim1_pd, enabled ? 0 : 1);
#ifdef CONFIG_FIBOCOM_BASE
        REG_ADI_CHANGE1(hwp_pmicAna->ldo_sim_ctrl1, ldo_sim_ctrl1,
                        slp_ldosim1_pd_en, lp_enabled ? 0 : 1);
#endif

#endif
        break;

    case HAL_POWER_KEYLED:
        prvUnlockPowerReg();
        REG_ADI_CHANGE1(hwp_pmicRtcAna->kpled_ctrl0, kpled_ctrl0, rg_ldo_kpled_reftrim, key_ref);

        REG_ADI_CHANGE2(hwp_pmicBltc->bltc_ctl, bltc_ctrl_t,
                        b_sel, 1, b_sw, 1);

        REG_ADI_CHANGE1(hwp_pmicRtcAna->kpled_ctrl0, kpled_ctrl0,
                        rg_ldo_kpled_pd, enabled ? 0 : 1);
        REG_ADI_CHANGE1(hwp_pmicRtcAna->kpled_ctrl0, kpled_ctrl0,
                        rg_kpled_pd, 1);

        //halAdiBusWrite(&hwp_pmicRtcAna->kpled_ctrl0, 0x90);
        REG_ADI_CHANGE1(hwp_pmicAna->kpled_ctrl1, kp_led_ctrl1, rg_ldo_kpled_v, 0);

        REG_ADI_CHANGE1(hwp_pmicRtcAna->slp_ldo_pd_ctrl0, slp_ldo_lp_pd_ctrl0,
                        slp_ldokpled_pd_en, lp_enabled ? 0 : 1);

        break;
    case HAL_POWER_VPA:
        prvUnlockPowerReg();
        REG_ADI_CHANGE1(hwp_pmicAna->sim_vpa_ctrl0, vpa_sim_ctrl0,
                        rg_vpa_pd, enabled ? 0 : 1);
        REG_ADI_CHANGE1(hwp_pmicAna->sim_vpa_ctrl0, vpa_sim_ctrl0,
                        rg_vpa_lp_en, lp_enabled ? 1 : 0);

        break;

    case HAL_POWER_BACK_LIGHT:
        prvUnlockPowerReg();
        REG_ADI_CHANGE1(hwp_pmicAna->module_en0, ana_module_en0, bltc_en, 1);
        REG_ADI_CHANGE4(hwp_pmicBltc->bltc_ctl, bltc_ctrl_t,
                        g_sel, enabled ? 1 : 0, g_sw, enabled ? 1 : 0,
                        r_sel, enabled ? 1 : 0, r_sw, enabled ? 1 : 0);

        REG_ADI_CHANGE4(hwp_pmicBltc->bltc_ctl, bltc_ctrl_t,
                        wled_sel, enabled ? 1 : 0, wled_sw, enabled ? 1 : 0,
                        b_sel, enabled ? 1 : 0, b_sw, enabled ? 1 : 0);

        REG_ADI_CHANGE2(hwp_pmicBltc->bltc_pd_ctrl, bltc_pd_ctrl,
                        hw_pd, 0,
                        sw_pd, enabled ? 0 : 1);

        REG_ADI_CHANGE1(hwp_pmicBltc->bltc_r_isat, rgb_r_isat, isat, 0x20);
        REG_ADI_CHANGE1(hwp_pmicBltc->bltc_g_isat, rgb_g_isat, isat, 0x20);
        REG_ADI_CHANGE1(hwp_pmicBltc->bltc_b_isat, rgb_b_isat, isat, 0x20);
        //merge patch by fibocom�?0220317
        REG_ADI_CHANGE1(hwp_pmicAna->module_en0, ana_module_en0, bltc_en, enabled ? 1 : 0);
        break;
#ifdef CONFIG_FIBOCOM_BASE
    case HAL_POWER_BACK_LIGHT_1:  //  rgb_b control
        prvUnlockPowerReg();
        if(0 == bltc_en_inuse)//for first time we need disable sink2 pin, tince them in same pm control filed
        {
            REG_ADI_CHANGE1(hwp_pmicAna->module_en0, ana_module_en0, bltc_en, 1);
            REG_ADI_CHANGE4(hwp_pmicBltc->bltc_ctl, bltc_ctrl_t,
                        g_sel, 0, g_sw, 0,
                        r_sel, 0, r_sw, 0);
            REG_ADI_CHANGE4(hwp_pmicBltc->bltc_ctl, bltc_ctrl_t,
                        wled_sel, 0, wled_sw, 0,
                        b_sel, 0, b_sw, 0);
            REG_ADI_CHANGE1(hwp_pmicAna->module_en0, ana_module_en0, bltc_en, 0);
        }
        REG_ADI_CHANGE1(hwp_pmicAna->module_en0, ana_module_en0, bltc_en, 1);
        REG_ADI_CHANGE4(hwp_pmicBltc->bltc_ctl, bltc_ctrl_t,
                        wled_sel, enabled ? 1 : 0, wled_sw, enabled ? 1 : 0,
                        b_sel, enabled ? 1 : 0, b_sw, enabled ? 1 : 0);
        REG_ADI_CHANGE1(hwp_pmicBltc->bltc_b_isat, rgb_b_isat, isat, 0x20);
        if(enabled)
        {
            bltc_en_inuse += 1;
            REG_ADI_CHANGE2(hwp_pmicBltc->bltc_pd_ctrl, bltc_pd_ctrl,
                        hw_pd, 0,
                        sw_pd, enabled ? 0 : 1);
            REG_ADI_CHANGE1(hwp_pmicAna->module_en0, ana_module_en0, bltc_en, enabled ? 1 : 0);
        }
        else
        {
            if(bltc_en_inuse >= 1)
            {
                bltc_en_inuse -= 1;
            }
            if(bltc_en_inuse)
            {
                OSI_PRINTFI("SINK1 pin is in use, not close bltc_en !!!bltc_en_inuse= %d ", bltc_en_inuse);
            }
            else
            {
                bltc_en_inuse = 0;
                REG_ADI_CHANGE2(hwp_pmicBltc->bltc_pd_ctrl, bltc_pd_ctrl,
                        hw_pd, 0,
                        sw_pd, enabled ? 0 : 1);
                REG_ADI_CHANGE1(hwp_pmicAna->module_en0, ana_module_en0, bltc_en, enabled ? 1 : 0);
            }
        }
        break;
#endif
    case HAL_POWER_BUCK_PA:
        prvUnlockPowerReg();
        REG_ADI_CHANGE1(hwp_pmicAna->sim_vpa_ctrl0, vpa_sim_ctrl0,
                        rg_vpa_pd, enabled ? 0 : 1);
        // no setting for lp
        break;

    case HAL_POWER_SPIMEM:
        prvUnlockPowerReg();
        REG_ADI_CHANGE1(hwp_pmicRtcAna->power_pd_sw0, ldo_sd_pd_sw0,
                        da_ldo_spimem_pd, enabled ? 0 : 1);
        REG_ADI_CHANGE1(hwp_pmicRtcAna->slp_ldo_pd_ctrl0, slp_ldo_lp_pd_ctrl0,
                        slp_ldospimem_pd_en, lp_enabled ? 0 : 1);
        break;

    case HAL_POWER_ANALOG:
        prvUnlockPowerReg();
        REG_ADI_CHANGE1(hwp_pmicRtcAna->power_pd_sw0, ldo_sd_pd_sw0,
                        da_ldo_ana_pd, enabled ? 0 : 1);
        REG_ADI_CHANGE1(hwp_pmicRtcAna->slp_ldo_pd_ctrl0, slp_ldo_lp_pd_ctrl0,
                        slp_ldoana_pd_en, lp_enabled ? 0 : 1);
        break;
    case HAL_POWER_RF12:
        prvUnlockPowerReg();
        REG_ADI_CHANGE1(hwp_pmicRtcAna->power_pd_sw0, ldo_sd_pd_sw0,
                        da_ldo_rf12_pd, enabled ? 0 : 1);
        REG_ADI_CHANGE1(hwp_pmicRtcAna->slp_ldo_pd_ctrl0, slp_ldo_lp_pd_ctrl0,
                        slp_ldorf12_pd_en, lp_enabled ? 0 : 1);
        break;
    case HAL_POWER_RF15:
        prvUnlockPowerReg();
        REG_ADI_CHANGE1(hwp_pmicRtcAna->power_pd_sw1, ldo_sd_pd_sw1,
                        da_ldo_rf15_pd, enabled ? 0 : 1);
        REG_ADI_CHANGE1(hwp_pmicRtcAna->slp_ldo_pd_ctrl0, slp_ldo_lp_pd_ctrl0,
                        slp_ldorf15_pd_en, lp_enabled ? 0 : 1);
        break;

    case HAL_POWER_LP18:
        prvUnlockPowerReg();
        REG_ADI_CHANGE1(hwp_pmicRtcAna->power_pd_sw0, ldo_sd_pd_sw0,
                        da_ldo_lp18_pd, enabled ? 0 : 1);
        REG_ADI_CHANGE1(hwp_pmicRtcAna->slp_ldo_pd_ctrl0, slp_ldo_lp_pd_ctrl0,
                        pm1_ldolp18_pd_en, lp_enabled ? 0 : 1);
        break;

    case HAL_POWER_DCDC_GEN:
        prvUnlockPowerReg();
        REG_ADI_CHANGE1(hwp_pmicRtcAna->power_pd_sw0, ldo_sd_pd_sw0,
                        da_vgen_pd, enabled ? 0 : 1);

        REG_ADI_CHANGE1(hwp_pmicRtcAna->slp_dcdc_pd_ctrl, slp_dcdc_pd_c,
                        pm1_dcdcgen_pd_en, lp_enabled ? 0 : 1);
        break;
    case HAL_POWER_DCDC_VRF:
        prvUnlockPowerReg();
        REG_ADI_CHANGE1(hwp_pmicRtcAna->power_pd_sw0, ldo_sd_pd_sw0,
                        da_vrf_pd, enabled ? 0 : 1);
        REG_ADI_CHANGE1(hwp_pmicRtcAna->slp_dcdc_pd_ctrl, slp_dcdc_pd_c,
                        slp_dcdcvrf_pd_en, lp_enabled ? 0 : 1);
        break;

    case HAL_POWER_CORE:
        prvUnlockPowerReg();
        REG_ADI_CHANGE1(hwp_pmicRtcAna->power_pd_sw0, ldo_sd_pd_sw0,
                        da_vcore_pd, lp_enabled ? 1 : 0);

        REG_ADI_CHANGE1(hwp_pmicRtcAna->slp_dcdc_pd_ctrl, slp_dcdc_pd_c,
                        slp_dcdccore_drop_en, lp_enabled ? 0 : 1);
        break;

    default:
        // ignore silently
        break;
    }

    return true;
}

bool halPmuSwitchPowerPm2(uint32_t id, bool pm2)
{

    REG_PMIC_RTC_ANA_PM2_PD_EN_T rtc_ana_pm2_en;
    REG_PMIC_RTC_ANA_RESERVED_REG5_T reserved_reg5;
    prvUnlockPowerReg();

    switch (id)
    {
    case HAL_POWER_VIO18:
        REG_ADI_CHANGE1(hwp_pmicRtcAna->pm2_pd_en, rtc_ana_pm2_en,
                        pm2_ldovio18_pd_en, pm2 ? 0 : 1);
        break;
    case HAL_POWER_VIO33:
        REG_ADI_CHANGE1(hwp_pmicRtcAna->pm2_pd_en, rtc_ana_pm2_en,
                        pm2_ldovio33_pd_en, pm2 ? 0 : 1);
        break;

    case HAL_POWER_DCXO:
        REG_ADI_CHANGE1(hwp_pmicRtcAna->pm2_pd_en, rtc_ana_pm2_en,
                        pm2_ldodcxo_pd_en, pm2 ? 0 : 1);
        break;

    case HAL_POWER_LP18:
        REG_ADI_CHANGE1(hwp_pmicRtcAna->pm2_pd_en, rtc_ana_pm2_en,
                        pm2_ldolp18_pd_en, pm2 ? 0 : 1);
        break;

    case HAL_POWER_DCDC_GEN:
        REG_ADI_CHANGE1(hwp_pmicRtcAna->pm2_pd_en, rtc_ana_pm2_en,
                        pm2_dcdcgen_pd_en, pm2 ? 0 : 1);
        break;

    case HAL_POWER_CORE:
        REG_ADI_CHANGE1(hwp_pmicRtcAna->pm2_pd_en, rtc_ana_pm2_en,
                        pm2_dcdccore_pd_en, pm2 ? 0 : 1);
        break;

    case HAL_POWER_MEM:
        REG_ADI_CHANGE1(hwp_pmicRtcAna->reserved_reg5, reserved_reg5,
                        pm2_ldomem_lp_en, pm2 ? 0 : 1);
        break;

    default:
        // ignore silently
        break;
    }

    return true;
}

bool halPmuSetCamFlashLevel(uint8_t level)
{
    // TODO:
    return true;
}

bool halPmuSetPowerLevel(uint32_t id, uint32_t mv)
{
    REG_PMIC_ANA_LDO_SIM_VOSEL_T ldo_sim0_vosel;
    REG_PMIC_ANA_LDO_SIM_VOSEL_T ldo_sim1_vosel;
    REG_PMIC_ANA_VPA_CTRL1_T ldo_pa_vosel;

    REG_PMIC_ANA_LDO_VOSEL1_T ldo_vosel_1;
    REG_PMIC_ANA_LDO_VOSEL3_T ldo_vosel_3;
    REG_PMIC_ANA_LDO_VOSEL4_T ldo_vosel_4;
    REG_PMIC_ANA_LDO_RF12_CTRL_T ldo_rf12_vsel;
    REG_PMIC_ANA_LDO_RF15_CTRL_T ldo_rf15_vsel;
    REG_PMIC_ANA_LDO_CAMD_CTRL_T ldo_camd_vsel;
    REG_PMIC_ANA_LDO_SPIMEM_CTRL_T ldo_spimem_vsel;
    REG_PMIC_ANA_LDO_VGEN_CTRL1_T ldo_vgen1_vsel;
    REG_PMIC_ANA_LDO_VIO18_CTRL_T ldo_vio18_vsel;
    REG_PMIC_ANA_LDO_ANA_CTRL_T ldo_ana_vsel;
    REG_PMIC_ANA_KPLED_CTRL1_T key_ldo_ctrl1_v;

    int32_t level = prvVoltageSettingValue(id, mv);

    if (level < 0)
        return false;

    switch (id)
    {
    case HAL_POWER_SIM0:
        REG_ADI_CHANGE1(hwp_pmicAna->ldo_sim_vosel, ldo_sim0_vosel, rg_ldo_sim0_vosel, level);
        break;

    case HAL_POWER_SIM1:
        REG_ADI_CHANGE1(hwp_pmicAna->ldo_sim_vosel, ldo_sim1_vosel, rg_ldo_sim1_vosel, level);
        break;

    case HAL_POWER_BUCK_PA:
        REG_ADI_CHANGE1(hwp_pmicAna->vpa_ctrl1, ldo_pa_vosel, rg_vpa_vosel, level);
        break;

    case HAL_POWER_USB:
        REG_ADI_CHANGE1(hwp_pmicAna->ldo_vosel1, ldo_vosel_1, rg_ldo_usb33_vosel, level);
        break;

    case HAL_POWER_CAMA:
        REG_ADI_CHANGE1(hwp_pmicAna->ldo_vosel1, ldo_vosel_1, rg_ldo_cama_vosel, level);
        break;

    case HAL_POWER_VIO33:
        REG_ADI_CHANGE1(hwp_pmicAna->ldo_vosel3, ldo_vosel_3, rg_ldo_vio33_vosel, level);
        break;

    case HAL_POWER_SD:
        REG_ADI_CHANGE1(hwp_pmicAna->ldo_vosel3, ldo_vosel_3, rg_ldo_mmc_vosel, level);
        break;

    case HAL_POWER_LP18:
        REG_ADI_CHANGE1(hwp_pmicAna->ldo_vosel4, ldo_vosel_4, rg_ldo_lp18_vosel, level);
        break;

    case HAL_POWER_LCD:
        REG_ADI_CHANGE1(hwp_pmicAna->ldo_vosel4, ldo_vosel_4, rg_ldo_lcd_vosel, level);
        break;

    case HAL_POWER_RF12:
        REG_ADI_CHANGE1(hwp_pmicAna->ldo_rf12_ctrl, ldo_rf12_vsel, rg_ldo_rf12_v, level);
        break;

    case HAL_POWER_RF15:
        REG_ADI_CHANGE1(hwp_pmicAna->ldo_rf15_ctrl, ldo_rf15_vsel, rg_ldo_rf15_v, level);
        break;
    case HAL_POWER_CAMD:
        REG_ADI_CHANGE1(hwp_pmicAna->ldo_camd_ctrl, ldo_camd_vsel, rg_ldo_camd_v, level);
        break;
    case HAL_POWER_SPIMEM:
        REG_ADI_CHANGE1(hwp_pmicAna->ldo_spimem_ctrl, ldo_spimem_vsel, rg_ldo_spimem_v, level);
        break;
    case HAL_POWER_MEM:
        REG_ADI_CHANGE1(hwp_pmicAna->ldo_vgen_ctrl1, ldo_vgen1_vsel, rg_ldo_mem_v, level);
        break;
    case HAL_POWER_VIO18:
        REG_ADI_CHANGE1(hwp_pmicAna->ldo_vio18_ctrl, ldo_vio18_vsel, rg_ldo_vio18_v, level);
        break;
    case HAL_POWER_ANALOG:
        REG_ADI_CHANGE1(hwp_pmicAna->ldo_ana_ctrl, ldo_ana_vsel, rg_ldo_ana_v, level);
        break;

    case HAL_POWER_KEYLED:
        REG_ADI_CHANGE1(hwp_pmicAna->kpled_ctrl1, key_ldo_ctrl1_v, rg_ldo_kpled_v, level);
        break;

    default:
        break;
    }

    return true;
}

OSI_NO_RETURN void halShutdown(int mode, int64_t wake_uptime)
{
    OSI_LOGD(0x100092ca, "psm: shutdown mode/0x%x wake/%u", mode, (unsigned)wake_uptime);

#ifdef CONFIG_SHUTDOWN_NORMAL_POWER_OFF
    if (mode == OSI_SHUTDOWN_POWER_OFF)
    {
        //need to set dcxo lp,otherwise 400ua electric leakage when power off
        REG_RF_DIG_RTC_REG_C4_REG_T reg_c4_reg = {};
        reg_c4_reg.b.xtal_osc_ibit_n = 8;
        reg_c4_reg.b.xtal26m_plls1_en = 1;
        reg_c4_reg.b.xtal26m_plls2_en = 1;
        reg_c4_reg.b.xtal26m_pllcal_en = 1;
        reg_c4_reg.b.enable_clk_26m = 1;
        reg_c4_reg.b.xtal26m_interface_en = 1;
        reg_c4_reg.b.enable_clk_26m_lp_uart = 1;
        hwp_rfDigRtc->reg_c4_reg = reg_c4_reg.v;

        REG_RF_DIG_RTC_REG_CC_REG_T reg_cc_reg = {};
        reg_cc_reg.b.xtal_reg_bit = 8;
        hwp_rfDigRtc->reg_cc_reg = reg_cc_reg.v;

        REG_RF_DIG_RTC_REG_C8_REG_T reg_c8_reg = {};
        reg_c8_reg.b.xtal_fixi_bit_n = 16;
        reg_c8_reg.b.xtal_fixi_bit_l = 1;
        // reg_c8_reg.b.xtal26m_ts_en = 1;???
        hwp_rfDigRtc->reg_c8_reg = reg_c8_reg.v;

        prvUnlockPowerReg();
        REG_PMIC_RTC_ANA_POWER_PD_HW_T power_pd_hw = {};
        power_pd_hw.b.pwr_off_seq_en = 1;
        halAdiBusWrite(&hwp_pmicRtcAna->power_pd_hw, power_pd_hw.v);

        OSI_DEAD_LOOP;
    }
#endif

    if (mode == OSI_SHUTDOWN_POWER_OFF)
    {
#if defined CONFIG_FIBOCOM_BASE && defined CONFIG_SUPPORT_BATTERY_CHARGER
        bool charge_power = fibo_getChargeSwitch();
#endif
        halPmuPsmPrepare();
        halAdiBusWrite(&PSM_MODE_REG, mode);
        bool pwrkey_en = false;
        bool wakeup_en = false;
#ifdef CONFIG_PWRKEY_POWERUP
        pwrkey_en = true;
#else
        pwrkey_en = false;
#endif
#ifdef CONFIG_WAKEUP_PIN_POWERUP
        wakeup_en = true;
#endif

        const int64_t epoch_sec = osiEpochSecond();
        if (epoch_sec < CONFIG_KERNEL_MIN_UTC_SECOND)
            osiPanic();

        const uint32_t epoch_sec_off = epoch_sec - CONFIG_KERNEL_MIN_UTC_SECOND;
        halAdiBusBatchChange(&PSM_EPOCH_HIGH,
                             HAL_ADI_BUS_OVERWITE(epoch_sec_off >> 16),
                             &PSM_EPOCH_LOW,
                             HAL_ADI_BUS_OVERWITE(epoch_sec_off & 0xffff),
                             HAL_ADI_BUS_CHANGE_END);

        // configure psm each function enable signal
        REG_PMIC_PSM_PSM_CTRL_T psm_ctrl = {
            .b.psm_en = 1,
            .b.rtc_pwr_on_timeout_en = 0,
            .b.ext_int_pwr_en = wakeup_en ? 1 : 0,
            .b.pbint1_pwr_en = pwrkey_en ? 1 : 0,
            .b.pbint2_pwr_en = 0,
#ifdef CONFIG_CHARGER_POWERUP
      #ifdef CONFIG_FIBOCOM_BASE
        #ifdef CONFIG_SUPPORT_BATTERY_CHARGER
            #ifdef CONFIG_FIBOCUS_JIEKE_FEATURE
                .b.charger_pwr_en = 1,
            #else
                .b.charger_pwr_en = charge_power,
            #endif
        #else
            .b.charger_pwr_en = 0,
        #endif
      #else
            .b.charger_pwr_en = 1,
      #endif
#else
            .b.charger_pwr_en = 0,
#endif
            .b.psm_cnt_alarm_en = 0,
            .b.psm_cnt_alm_en = 0,
            .b.psm_software_reset = 0,
            .b.psm_cnt_update = 1,
            .b.psm_cnt_en = 1,
            .b.psm_status_clr = 0,
            .b.psm_cal_en = 1,
            .b.rtc_32k_clk_sel = 0, // 32k less
        };

        if (wake_uptime != INT64_MAX)
        {
            int64_t delta = (wake_uptime - osiUpTime()) * 1000 / PSM_CNT_US;
            uint32_t cnt = (delta < PSM_MIN_CNT) ? PSM_MIN_CNT : (uint32_t)delta;

            OSI_LOGD(0x100092cb, "psm: sleep cnt/%u", cnt);

            // configure psm normal wakeup time
            halAdiBusWrite(&hwp_pmicPsm->psm_cnt_l_th, cnt & 0xffff);
            halAdiBusWrite(&hwp_pmicPsm->psm_cnt_h_th, cnt >> 16);

            psm_ctrl.b.psm_cnt_alm_en = 1;
        }
        //halAdiBusWrite(&hwp_pmicPsm->psm_alarm_cnt_l_th, 0x880);
        //halAdiBusWrite(&hwp_pmicPsm->psm_alarm_cnt_h_th, 0x0);

        //halAdiBusWrite(&hwp_pmicPsm->psm_cnt_interval_th, 0x2);
        //halAdiBusWrite(&hwp_pmicPsm->psm_cnt_interval_phase, 0x2);

        halAdiBusWrite(&hwp_pmicPsm->pbint_1s_thd, 0x235);
        halAdiBusWrite(&hwp_pmicPsm->rtc_reserved1, 0);

        REG_PMIC_RTC_ANA_CLK32KLESS_CTRL0_T clk32kless_ctrl0;
        clk32kless_ctrl0.v = halAdiBusRead(&hwp_pmicRtcAna->clk32kless_ctrl0);
        if (clk32kless_ctrl0.b.rtc_mode == 0)
        {
            psm_ctrl.b.psm_cal_en = 0;
        }

        halAdiBusWrite(&hwp_pmicPsm->psm_ctrl, psm_ctrl.v);
        //halAdiBusWrite(&hwp_pmicPsm->psm_ctrl, 0x1641);

        REG_PMIC_PSM_PSM_STATUS_T psm_psm_status;
        REG_ADI_WAIT_FIELD_NEZ(psm_psm_status, hwp_pmicPsm->psm_status, psm_cnt_update_vld);
        halAdiBusWrite(&(hwp_pmicPsm->reserved_4), halAdiBusRead(&hwp_pmicPsm->psm_fsm_status));

        psm_ctrl.b.psm_status_clr = 1;
        halAdiBusWrite(&hwp_pmicPsm->psm_ctrl, psm_ctrl.v);
        osiDelayUS(20000);
        psm_ctrl.b.psm_status_clr = 0;
        halAdiBusWrite(&hwp_pmicPsm->psm_ctrl, psm_ctrl.v);
        // power off rtc
        REG_PMIC_RTC_ANA_POWER_PD_HW_T rtc_ana_power_pd_hw;
        rtc_ana_power_pd_hw.b.pwr_off_seq_en = 1;
        halAdiBusWrite(&hwp_pmicRtcAna->power_pd_hw, rtc_ana_power_pd_hw.v);
    }
    else if (mode == OSI_SHUTDOWN_FORCE_DOWNLOAD)
    {
        // boot_mode will be checked in ROM
        REG_LPS_APB_CFG_BOOT_MODE_T cfg_boot_mode = {hwp_lpsApb->cfg_boot_mode};
        cfg_boot_mode.b.boot_mode_pin |= 1;
        cfg_boot_mode.b.boot_mode_pin &= ~1; // force download, clear product test
        cfg_boot_mode.b.boot_mode_sw = 0;
        hwp_lpsApb->cfg_boot_mode = cfg_boot_mode.v;

        REG_LPS_APB_RESET_SYS_SOFT_T reset_enable = {hwp_lpsApb->reset_sys_soft};
        reset_enable.b.chip_soft_reset = 1;
        hwp_lpsApb->reset_sys_soft = reset_enable.v;

        OSI_DEAD_LOOP;
    }
    else
    {
        //config rst_timer delay to max value for reboot stability
        REG_PMIC_PSM_POR_TIMER_THD_T psm_por_timer_thd;
        REG_ADI_CHANGE1(hwp_pmicPsm->por_timer_thd, psm_por_timer_thd, ext_rst_timer_thd, 0x3FF);
        osiDelayUS(10);

        halAdiBusWrite(&PMU_BOOT_MODE_REG, mode);

        REG_PMIC_RTC_ANA_SWRST_CTRL0_T swrst_ctrl0;
        swrst_ctrl0.v = halAdiBusRead(&hwp_pmicRtcAna->swrst_ctrl0);
        swrst_ctrl0.b.reg_rst_en = 1;
        swrst_ctrl0.b.reg_rst_pd_en = 1;
        halAdiBusWrite(&hwp_pmicRtcAna->swrst_ctrl0, swrst_ctrl0.v);
        #ifdef CONFIG_FIBOCUS_VODAFU_FEATURE
        REG_PMIC_RTC_ANA_SWRST_CTRL1_T swrst_ctrl1 = {};
        swrst_ctrl1.b.sw_rst_vio33_pd_en = 0;
        swrst_ctrl1.b.sw_rst_usb_pd_en = 0;
        swrst_ctrl1.b.sw_rst_rf15_pd_en = 0;
        swrst_ctrl1.b.sw_rst_ana_pd_en = 0;
        swrst_ctrl1.b.sw_rst_rf12_pd_en = 0;
        swrst_ctrl1.b.sw_rst_dcxo_pd_en = 0;
        swrst_ctrl1.b.sw_rst_mem_pd_en = 1; //modified by SPCSS01105291, change PSRAM from reset mode to power-off mode
        swrst_ctrl1.b.sw_rst_dcdccore_pd_en = 0;
        swrst_ctrl1.b.sw_rst_dcdcgen_pd_en = 0;
        swrst_ctrl1.b.sw_rst_vio18_pd_en = 1;
        swrst_ctrl1.b.sw_rst_spimem_pd_en = 0;
        halAdiBusWrite(&hwp_pmicRtcAna->swrst_ctrl1, swrst_ctrl1.v);
        #endif
        REG_PMIC_RTC_ANA_SOFT_RST_HW_T soft_rst_hw;
        soft_rst_hw.b.reg_soft_rst_sw = 1;
        halAdiBusWrite(&hwp_pmicRtcAna->soft_rst_hw, soft_rst_hw.v);
    }

    OSI_DEAD_LOOP;
}

void halPmuWakeFromDeep(void)
{
    bWakeupFromDeep = true;
}

bool halPmuIsVCoreFirstTimeEnterDeep(void)
{
    return !(REGT_ADI_FIELD_GET(hwp_pmicRtcAna->slp_dcdc_pd_ctrl, REG_PMIC_RTC_ANA_SLP_DCDC_PD_CTRL_T, slp_dcdccore_drop_en));
}

void halPmuEnterPm1(void)
{
    halPmuSwitchPower(HAL_POWER_SPIMEM, true, true);
#ifdef CONFIG_OPENCPU_ENABLED
    halPmuSwitchPower(HAL_POWER_EMM, false, false);
#endif
    prvModifyVDDCoreDropVol();
}

void halPmuAbortPm1(void)
{
}

void halPmuExitPm1(void)
{
    prvEnableVDDCoreDrop();
#ifdef CONFIG_OPENCPU_ENABLED
    halPmuSwitchPower(HAL_POWER_EMM, true, false);
#endif
}

void halPmuEnterPm2(void)
{
    //gpio 1 used as hp det, it is pulled up to vdd18, in pm2 mode, this pin will generate interrupt and wakeup sys.
    //we need mask interrupt.
    hwp_gpio1->gpint_ctrl_r_clr_reg = 2;
    hwp_gpio1->gpint_ctrl_f_clr_reg = 2;
    hwp_lpsApb->cfg_por_usb_phy = 0xf;

    halPmuSwitchPower(HAL_POWER_EMM, false, false);

    // pm2 relative register
    hwp_adiMst->adi_chanel_en = 0x8;    //idst_aon
    hwp_adiMst->event1_waddr = 0x918;   //pmic reserved reg6 write 1
    hwp_adiMst->event1_wdata = 0x10000; //write 1
    hwp_aonClk->cgm_adi_sel_cfg = 0x1;  //xtal_26m

    halPmuSwitchPower(HAL_POWER_SPIMEM, true, false);

    prvModifyVDDCoreDropVol();
}

void halPmuAbortPm2(void)
{
    //re-enable gpio 1 interrupt.
    hwp_gpio1->gpint_ctrl_r_set_reg = 2;
    hwp_gpio1->gpint_ctrl_f_set_reg = 2;
}

void halPmuExitPm2(void)
{
    //re-enable gpio 1 interrupt.
    hwp_gpio1->gpint_ctrl_r_set_reg = 2;
    hwp_gpio1->gpint_ctrl_f_set_reg = 2;
    hwp_lpsApb->cfg_por_usb_phy = 0x0;
    halPmuSwitchPower(HAL_POWER_EMM, true, false);

    halAdiBusWrite(&hwp_pmicAna->vcore_ctrl3, 0xc00);

    prvEnableVDDCoreDrop();
    // if wake from pm2, clear pmic reg6 (pm2_en)
    halAdiBusWrite(&hwp_pmicRtcAna->reserved_reg6, 0);
}

void halPmuSet7sReset(bool enable)
{
    REG_PMIC_RTC_ANA_POR_7S_CTRL_T por_7s_ctrl;

    if (enable)
        REG_ADI_CHANGE1(hwp_pmicRtcAna->por_7s_ctrl, por_7s_ctrl,
                        pbint_7s_rst_disable, 0);
    else
        REG_ADI_CHANGE1(hwp_pmicRtcAna->por_7s_ctrl, por_7s_ctrl,
                        pbint_7s_rst_disable, 1);
}

void halPmuPsmPrepare(void)
{
    //REG_PMIC_RTC_ANA_SOFT_RST0_T soft_rst0;
    //REG_PMIC_PSM_PSM_CTRL_T psm_ctrl;
    //REG_PMIC_PSM_PSM_RC_CLK_DIV_T psm_rc_clk_div;
    //REG_PMIC_PSM_PSM_STATUS_T psm_status;
    // REG_PMIC_PSM_PSM_REG_WR_PROTECT_T psm_reg_wr_protect;
    // REG_PMIC_PSM_PSM_26M_CAL_DN_TH_T psm_26m_cal_dn_th;
    // REG_PMIC_PSM_PSM_26M_CAL_UP_TH_T psm_26m_cal_up_th;
    //REG_PMIC_PSM_PSM_32K_CAL_TH_T psm_32k_cal_th;
    //REG_PMIC_PSM_RTC_PWR_OFF_TH1_T rtc_pwr_off_th1;
    //REG_PMIC_PSM_RTC_PWR_OFF_TH2_T rtc_pwr_off_th2;
    //REG_PMIC_PSM_RTC_PWR_OFF_TH3_T rtc_pwr_off_th3;
    //REG_PMIC_PSM_RTC_PWR_ON_TH1_T rtc_pwr_on_th1;
    //REG_PMIC_PSM_RTC_PWR_ON_TH2_T rtc_pwr_on_th2;
    //REG_PMIC_PSM_RTC_PWR_ON_TH3_T rtc_pwr_on_th3;
    // REG_PMIC_PSM_PSM_CNT_INTERVAL_TH_T psm_cnt_interval_th;
    // REG_PMIC_PSM_PSM_CNT_INTERVAL_PHASE_T psm_cnt_interval_phase;

    // psm pclk enable
    halAdiBusWrite(&hwp_pmicRtcAna->module_en0, 0x12);
    //rtc soft_reset
    halAdiBusWrite(&hwp_pmicRtcAna->soft_rst0, 0x20);
    //powkey_time
    osiDelayUS(10);
    halAdiBusWrite(&hwp_pmicRtcAna->soft_rst0, 0x00);

    //config psm register
    //REG_PMIC_PSM_PSM_REG_WR_PROTECT_T
    halAdiBusWrite(&hwp_pmicPsm->psm_reg_wr_protect, PSM_WR_PROT_MAGIC);

    // psm apb soft_reset
    halAdiBusWrite(&hwp_pmicPsm->psm_ctrl, 0x100);
    osiDelayUS(10);
    // open PSM protect register
    halAdiBusWrite(&hwp_pmicPsm->psm_reg_wr_protect, PSM_WR_PROT_MAGIC);

    halAdiBusWrite(&PSM_MODE_REG, OSI_SHUTDOWN_PSM_SLEEP);

    // psm module soft_reset, auto clear
    // halAdiBusWrite(&hwp_pmicPsm->psm_ctrl, 0xc00);
    halAdiBusWrite(&hwp_pmicPsm->psm_ctrl, 0x800);
    // halAdiBusWrite(&hwp_pmicPsm->psm_ctrl, 0x0);

    REG_PMIC_RTC_ANA_CLK32KLESS_CTRL0_T clk32kless_ctrl0;
    clk32kless_ctrl0.v = halAdiBusRead(&hwp_pmicRtcAna->clk32kless_ctrl0);
    if (clk32kless_ctrl0.b.rtc_mode == 0)
    {
        REG_PMIC_PSM_CLK_32K_XTAL_CALIBRA_SEL_T clk_32k_xtal_cal_sel;
        REG_ADI_CHANGE1(hwp_pmicPsm->clk_32k_xtal_calibra_sel, clk_32k_xtal_cal_sel, clk_32k_xtal_calibra_sel, 0);

        REG_PMIC_PSM_XTAL_POR_1ST_CLK_SEL_T xtal_por_1st_clk;
        REG_ADI_CHANGE1(hwp_pmicPsm->xtal_por_1st_clk_sel, xtal_por_1st_clk, xtal_por_1st_clk_sel, 0);

        halAdiBusWrite(&hwp_pmicPsm->psm_rc_clk_div, 0x470);
    }
    else
    {
        REG_PMIC_PSM_CLK_32K_XTAL_CALIBRA_SEL_T clk_32k_xtal_calibra_sel;
        REG_ADI_CHANGE1(hwp_pmicPsm->clk_32k_xtal_calibra_sel, clk_32k_xtal_calibra_sel, psm_reg_xtal32k_pon, 0);

        halAdiBusWrite(&hwp_pmicPsm->psm_rc_clk_div, 0x4f0);
    }

    halAdiBusWrite(&hwp_pmicPsm->psm_32k_cal_th, 0x208);

    halAdiBusWrite(&hwp_pmicPsm->psm_ctrl, 0x400);
   
    #if defined CONFIG_FIBOCOM_BASE
   extern uint8_t fibo_get_cpof_status(void);
         if((fibo_get_psm_mode() == true)||(1==fibo_get_cpof_status()))
         {
             #if defined CONFIG_FIBOCOM_MC669 //MC669 used gpio 7
             // gpio 7 for PSM ext wakeup, need  config as pull-up
             halAdiBusWrite(&hwp_pmicPsm->ext_xtl_en_hl, 0xff);
             halAdiBusWrite(&hwp_pmicPinReg->ext_xtl_en7, 0x146);
             #else //MC665 used  gpio4
             // gpio 4 for PSM ext wakeup, need  config as pull-up
             halAdiBusWrite(&hwp_pmicPsm->ext_xtl_en_hl, 0xff);
             halAdiBusWrite(&hwp_pmicPinReg->ext_xtl_en4, 0x146);
             #endif
         }
    #else
        #ifdef CONFIG_8850CM_TARGET
            // gpio 3 for PSM ext wakeup, need  config as pull-up, gpio4 config as Hi-Z state
            halAdiBusWrite(&hwp_pmicPsm->ext_xtl_en_hl, 0xf7);
            halAdiBusWrite(&hwp_pmicPinReg->ext_xtl_en3, 0x18A);
            halAdiBusWrite(&hwp_pmicPinReg->ext_xtl_en4, 0x020);
        #else
            // gpio 4 for PSM ext wakeup, need  config as pull-up, gpio3 config as Hi-Z state
            halAdiBusWrite(&hwp_pmicPsm->ext_xtl_en_hl, 0xef);
            halAdiBusWrite(&hwp_pmicPinReg->ext_xtl_en4, 0x18A);
            halAdiBusWrite(&hwp_pmicPinReg->ext_xtl_en3, 0x020);
        #endif
    #endif

    //other gpio need config as Hi-Z state
    halAdiBusWrite(&hwp_pmicPinReg->ext_xtl_en0, 0x020);
    halAdiBusWrite(&hwp_pmicPinReg->ext_xtl_en1, 0x020);
    halAdiBusWrite(&hwp_pmicPinReg->ext_xtl_en2, 0x020);
#if defined CONFIG_FIBOCOM_BASE
    halAdiBusWrite(&hwp_pmicPinReg->ext_xtl_en3, 0x020);
#endif
    halAdiBusWrite(&hwp_pmicPinReg->ext_xtl_en5, 0x020);
    halAdiBusWrite(&hwp_pmicPinReg->ext_xtl_en6, 0x020);

    #if defined CONFIG_FIBOCOM_BASE
        if((fibo_get_psm_mode() == true)||(1==fibo_get_cpof_status()))
        {
            #if defined CONFIG_FIBOCOM_MC669
              halAdiBusWrite(&hwp_pmicPinReg->ext_xtl_en4, 0x020);
            #else
              halAdiBusWrite(&hwp_pmicPinReg->ext_xtl_en7, 0x020);
            #endif
        }
        else
        {
            halAdiBusWrite(&hwp_pmicPinReg->ext_xtl_en4, 0x020);
            halAdiBusWrite(&hwp_pmicPinReg->ext_xtl_en7, 0x020);
        }
    #else
            halAdiBusWrite(&hwp_pmicPinReg->ext_xtl_en7, 0x020);
    #endif

    osiDelayUS(2000);
    halAdiBusWrite(&hwp_pmicPsm->psm_ctrl, 0x600);
    REG_PMIC_PSM_PSM_STATUS_T psm_psm_status;
    REG_ADI_WAIT_FIELD_NEZ(psm_psm_status, hwp_pmicPsm->psm_status, psm_cnt_update_vld);
    halAdiBusWrite(&(hwp_pmicPsm->reserved_4), halAdiBusRead(&hwp_pmicPsm->psm_fsm_status));
    halAdiBusWrite(&hwp_pmicPsm->psm_ctrl, 0x400);
    osiDelayUS(50000);
    halAdiBusWrite(&hwp_pmicPsm->psm_26m_cal_dn_th, 0x0);
    halAdiBusWrite(&hwp_pmicPsm->psm_26m_cal_up_th, 0xffff);
    halAdiBusWrite(&hwp_pmicPsm->rtc_pwr_off_th1, 0x1810);
    halAdiBusWrite(&hwp_pmicPsm->rtc_pwr_off_th2, 0x2820);
    halAdiBusWrite(&hwp_pmicPsm->rtc_pwr_off_th3, 0x40);

    halAdiBusWrite(&hwp_pmicPsm->rtc_pwr_on_th1, 0x2004);
    halAdiBusWrite(&hwp_pmicPsm->rtc_pwr_on_th2, 0x0506);
    halAdiBusWrite(&hwp_pmicPsm->rtc_pwr_on_th3, 0x48ff);

    // configure psm cal interval
    halAdiBusWrite(&hwp_pmicPsm->psm_cnt_interval_th, 0x6);

    // configure psm cal phase, <=psm_cnt_interval_th
    halAdiBusWrite(&hwp_pmicPsm->psm_cnt_interval_phase, 0x6);
}

#if defined CONFIG_FIBOCOM_BASE 
int _bootGetPowerOnCause(void)
{    
    uint32_t boot_causes = osiGetBootCauses();
    if(boot_causes & OSI_BOOTCAUSE_WDG)
    {
        return OSI_BOOTCAUSE_WDG;
    }
    else if(boot_causes & OSI_BOOTCAUSE_PIN_RESET)
    {
        return OSI_BOOTCAUSE_PIN_RESET;
    }
    else if(boot_causes & OSI_BOOTCAUSE_PWRKEY)
    {
        return OSI_BOOTCAUSE_PWRKEY;
    }
    else if(boot_causes & OSI_BOOTCAUSE_PANIC)
    {
        return OSI_BOOTCAUSE_PANIC;
    }
    else if(boot_causes & OSI_BOOTCAUSE_CHARGE)
    {
        return OSI_BOOTCAUSE_CHARGE;
    }
    else if(boot_causes & OSI_BOOTCAUSE_ALARM)
    {
        return OSI_BOOTCAUSE_ALARM;
    }
    else
    {
        return OSI_BOOTCAUSE_UNKNOWN;
    }
}

#endif

/**
 * Distinguish between 'ap system psm' and 'openCPU system psm'
 * gIsOpenCPUPmMode true : is 'openCPU system psm'
 * gIsOpenCPUPmMode false : is 'ap system psm'
 */
void halSetOpenCPUPmMode(void)
{
    gIsOpenCPUPmMode = true;
}

/**
 * Distinguish between 'ap system psm' and 'openCPU system psm'
 * gIsOpenCPUPmMode true : is 'openCPU system psm'
 * gIsOpenCPUPmMode false : is 'ap system psm'
 */
bool halGetOpenCPUPmMode(void)
{
    return gIsOpenCPUPmMode;
}



void halPsmSleep(int64_t wake_uptime)
{
    OSI_LOGD(0x100092cc, "psm sleep, wake/%u", (unsigned)wake_uptime);
#if defined CONFIG_FIBOCOM_BASE && defined CONFIG_SUPPORT_BATTERY_CHARGER
    bool charge_power = fibo_getChargeSwitch();
#endif
    if (true == halGetOpenCPUPmMode())
        halAdiBusWrite(&PSM_MODE_REG, OSI_SHUTDOWN_OPENCPU_PSM_SLEEP);
    else
        halAdiBusWrite(&PSM_MODE_REG, OSI_SHUTDOWN_PSM_SLEEP);

    bool pwrkey_en = false;
    bool wakeup_en = true;
#ifdef CONFIG_PWRKEY_WAKEUP_PSM
    pwrkey_en = true;
#endif

#ifdef CONFIG_OPENCPU_ENABLED
    OSI_LOGI(0x1000a9d2, "halPsmSleep gIsOpenCPUPmMode /%u", gIsOpenCPUPmMode);
#endif
    if (false == halGetOpenCPUPmMode())
    {
        const int64_t epoch_sec = osiEpochSecond();
        if (epoch_sec < CONFIG_KERNEL_MIN_UTC_SECOND)
            osiPanic();

        const uint32_t epoch_sec_off = epoch_sec - CONFIG_KERNEL_MIN_UTC_SECOND;
        halAdiBusBatchChange(&PSM_EPOCH_HIGH,
                             HAL_ADI_BUS_OVERWITE(epoch_sec_off >> 16),
                             &PSM_EPOCH_LOW,
                             HAL_ADI_BUS_OVERWITE(epoch_sec_off & 0xffff),
                             HAL_ADI_BUS_CHANGE_END);
    }
    // configure psm each function enable signal
    REG_PMIC_PSM_PSM_CTRL_T psm_ctrl = {
        .b.psm_en = 1,
        .b.rtc_pwr_on_timeout_en = 0,
        .b.ext_int_pwr_en = wakeup_en ? 1 : 0,
        .b.pbint1_pwr_en = pwrkey_en ? 1 : 0,
        .b.pbint2_pwr_en = 0,
#ifdef CONFIG_CHARGER_POWERUP
    #ifdef CONFIG_FIBOCOM_BASE
        #ifdef CONFIG_SUPPORT_BATTERY_CHARGER
        .b.charger_pwr_en = charge_power,
        #else
        .b.charger_pwr_en = 0,
        #endif
    #else
        .b.charger_pwr_en = 1,
    #endif
#else
        .b.charger_pwr_en = 0,
#endif
        .b.psm_cnt_alarm_en = 0,
        .b.psm_cnt_alm_en = 0,
        .b.psm_software_reset = 0,
        .b.psm_cnt_update = 1,
        .b.psm_cnt_en = 1,
        .b.psm_status_clr = 0,
        .b.psm_cal_en = 1,
        .b.rtc_32k_clk_sel = 0, // 32k less
    };

    if (wake_uptime != INT64_MAX)
    {
        int64_t delta = (wake_uptime - osiUpTime()) * 1000 / PSM_CNT_US;
        uint32_t cnt = (delta < PSM_MIN_CNT) ? PSM_MIN_CNT : (uint32_t)delta;

        OSI_LOGD(0x100092cb, "psm: sleep cnt/%u", cnt);

        // configure psm normal wakeup time
        halAdiBusWrite(&hwp_pmicPsm->psm_cnt_l_th, cnt & 0xffff);
        halAdiBusWrite(&hwp_pmicPsm->psm_cnt_h_th, cnt >> 16);

        psm_ctrl.b.psm_cnt_alm_en = 1;
    }

    halAdiBusWrite(&hwp_pmicPsm->pbint_1s_thd, 0x235);
    halAdiBusWrite(&hwp_pmicPsm->rtc_reserved1, 0);

    REG_PMIC_RTC_ANA_CLK32KLESS_CTRL0_T clk32kless_ctrl0;
    clk32kless_ctrl0.v = halAdiBusRead(&hwp_pmicRtcAna->clk32kless_ctrl0);
    if (clk32kless_ctrl0.b.rtc_mode == 0)
    {
        psm_ctrl.b.psm_cal_en = 0;
    }

    halAdiBusWrite(&hwp_pmicPsm->psm_ctrl, psm_ctrl.v);

    REG_PMIC_PSM_PSM_STATUS_T psm_status;
    REG_ADI_WAIT_FIELD_NEZ(psm_status, hwp_pmicPsm->psm_status, psm_cnt_update_vld);
    psm_ctrl.b.psm_status_clr = 1;
    halAdiBusWrite(&hwp_pmicPsm->psm_ctrl, psm_ctrl.v);
    // configure psm status
    REG_ADI_CHANGE6(hwp_pmicPsm->psm_status, psm_status,
                    ext_int_mask, 1,
                    pbint1_int_mask, 1,
                    pbint2_int_mask, 1,
                    charger_int_mask, 1,
                    psm_req_int_mask, 1,
                    alarm_req_int_mask, 1);
    osiDelayUS(20000);
    psm_ctrl.b.psm_status_clr = 0;
    halAdiBusWrite(&hwp_pmicPsm->psm_ctrl, psm_ctrl.v);
    // power off rtc
    halAdiBusWrite(&hwp_pmicRtcAna->power_pd_hw, 1);
}

static uint8_t g_sim_existent_flag = 0x3; /*every-bit for one sim,1:existent,0:non-existent;default sim all are existent*/
uint8_t hal_SimExistentGetFromIPC(UINT8 sim_index)
{
    return (g_sim_existent_flag & (1 << sim_index));
}

#define UIP8850_SIM_1V8_REG_BIT (0x7)
#define UIP8850_SIM_3V0_REG_BIT (0x37)
#define UIP8850_SIM1_VOSEL_OFFSET (10)
#define UIP8850_SIM2_VOSEL_OFFSET (0)
#define UIP8850_SIM_VOSEL_MASK (0x3F)

uint32_t foursim_SetSimVoltageAP(
    uint8_t voltClass0,
    uint8_t voltClass1,
    uint8_t voltClass2,
    uint8_t voltClass3,
    uint16_t pmdSimSelect,
    uint8_t SimIndex)
{
    OSI_LOGD(0x10006ae1, "pmd_FourSimSetVoltageClass: class1 = %d , class2 = %d  class3 = %d , class4 = %d ",
             voltClass0, voltClass1, voltClass2, voltClass3);

    uint16_t simSelMask = 0;

    uint32_t scStatus = osiEnterCritical();

    if ((voltClass0 == 3) || (!hal_SimExistentGetFromIPC(0))) //Set the Sim voltage to  0 V or sim card is non-existent
    {
        g_pmdLdoSettings.vSim1Enable = false;
    }
    else
    {
        g_pmdLdoSettings.vSim1Enable = true;
        simSelMask |= (1 << 0);
    }

    switch (voltClass0)
    {
    case 3:
        break;

    case 2:
        g_pmdLdoProfileSetting5.vSim1Is1_8 = true;
        break;

    case 1:
        g_pmdLdoProfileSetting5.vSim1Is1_8 = false;
        break;

    case 0:
        g_pmdLdoProfileSetting5.vSim1Is1_8 = false;
        break;

    default:
        OSI_LOGD(0x1000a9d3, "Assert! sim0 voltage: invalid voltage class %d", voltClass0);
        osiPanic();
        break;
    }

    if ((voltClass1 == 3) || (!hal_SimExistentGetFromIPC(1)))
    {
        g_pmdLdoSettings.vSim2Enable = false;
    }
    else
    {
        g_pmdLdoSettings.vSim2Enable = true;
        simSelMask |= (1 << 1);
    }

    switch (voltClass1)
    {
    case 3:
        break;

    case 2:
        g_pmdLdoProfileSetting5.vSim2Is1_8 = true;
        break;

    case 1:
        g_pmdLdoProfileSetting5.vSim2Is1_8 = false;
        break;

    case 0:
        g_pmdLdoProfileSetting5.vSim2Is1_8 = false;
        break;

    default:
        OSI_LOGD(0x1000a9d4, "sim1 voltage: invalid voltage class %d", voltClass0);
        osiPanic();
        break;
    }

    if ((voltClass2 == 3) || (!hal_SimExistentGetFromIPC(2)))
    {
        g_pmdLdoSettings.vSim3Enable = false;
    }
    else
    {
        g_pmdLdoSettings.vSim3Enable = true;
        simSelMask |= (1 << 2);
    }

    switch (voltClass2)
    {
    case 3:
        break;

    case 2:
        g_pmdLdoProfileSetting5.vSim3Is1_8 = true;
        break;

    case 1:
        g_pmdLdoProfileSetting5.vSim3Is1_8 = false;
        break;

    case 0:
        g_pmdLdoProfileSetting5.vSim3Is1_8 = false;
        break;

    default:
        OSI_LOGD(0x1000a9d5, "sim2 voltage: invalid voltage class %d", voltClass0);
        osiPanic();
        break;
    }

    if ((voltClass3 == 3) || (!hal_SimExistentGetFromIPC(3)))
    {
        g_pmdLdoSettings.vSim4Enable = false;
    }
    else
    {
        g_pmdLdoSettings.vSim4Enable = true;
        simSelMask |= (1 << 3);
    }

    switch (voltClass3)
    {
    case 3:
        break;

    case 2:
        g_pmdLdoProfileSetting5.vSim4Is1_8 = true;
        break;

    case 1:
        g_pmdLdoProfileSetting5.vSim4Is1_8 = false;
        break;

    case 0:
        g_pmdLdoProfileSetting5.vSim4Is1_8 = false;
        break;

    default:
        OSI_LOGD(0x1000a9d6, "sim3 voltage: invalid voltage class %d", voltClass0);
        osiPanic();
        break;
    }

    if ((simSelMask & pmdSimSelect) == 0)
    {
        //        hal_SimApDisableSci();
        hal_SimDisableSci();
    }

    uint16_t sim_vosel_val;
    if (SimIndex == 0) //current is sim1
    {
        if (g_pmdLdoSettings.vSim1Enable == false)
        {
            halPmuSwitchPower(HAL_POWER_SIM0, false, false); /*关闭sim1电源*/
        }
        else
        {
            if (g_pmdLdoProfileSetting5.vSim1Is1_8 == true)
            {
                sim_vosel_val = 1800; /*??sim1????.8V*/
            }
            else
            {
                sim_vosel_val = 3000; /*??sim1????V*/
            }
            halPmuSetPowerLevel(HAL_POWER_SIM0, sim_vosel_val);
            halPmuSwitchPower(HAL_POWER_SIM0, true, false); /*??sim1??*/
        }
    }
    else if (SimIndex == 1) //current is sim2
    {
        if (g_pmdLdoSettings.vSim2Enable == false)
        {
            halPmuSwitchPower(HAL_POWER_SIM1, false, false); /*??sim2??*/
        }
        else
        {
            if (g_pmdLdoProfileSetting5.vSim2Is1_8 == true)
            {
                sim_vosel_val = 1800; /*??sim2????.8V*/
            }
            else
            {
                sim_vosel_val = 3000; /*??sim2????V*/
            }
            halPmuSetPowerLevel(HAL_POWER_SIM1, sim_vosel_val);
            halPmuSwitchPower(HAL_POWER_SIM1, true, false); /*??sim2??*/
        }
    }

    osiExitCritical(scStatus);

    osiDelayUS(2000); //delay 2ms, waiting vsim stable

    scStatus = osiEnterCritical();
    if ((simSelMask & pmdSimSelect) != 0)
    {
        //        hal_SimApEnableSci();
        hal_SimEnableSci();
    }

    osiExitCritical(scStatus);

    return 0;
}

void pmd_SimLdoClose(uint8_t sim_card)
{
    //power off sim
    if (0 == sim_card)
    {
        halPmuSwitchPower(HAL_POWER_SIM0, false, false); /*关闭sim1电源*/
    }
    else
    {
        halPmuSwitchPower(HAL_POWER_SIM1, false, false); /*关闭sim2电源*/
    }
}

uint8_t drv_SetSimEnable(uint8_t sim_Index, uint8_t switch_Num)
{
    if (sim_Index > 3)
    {
        return 1;
    }

    // if (pmd_SelectSimCard(number))
    PMD_LDO_SETTINGS_T simEnMask;
    simEnMask.reg = 0;

    uint32_t scStatus = osiEnterCritical();

    //g_pmdSimSelect = 0;

    if (sim_Index == 0)
    {
        //g_pmdSimSelect |= HAL_ABB_SIM_SELECT_SIM0;
        simEnMask.vSim1Enable = true;
    }
    else if (sim_Index == 1)
    {
        //g_pmdSimSelect |= HAL_ABB_SIM_SELECT_SIM1;
        simEnMask.vSim2Enable = true;
    }
    else if (sim_Index == 2)
    {
        //g_pmdSimSelect |= HAL_ABB_SIM_SELECT_SIM2;
        simEnMask.vSim3Enable = true;
    }
    else if (sim_Index == 3)
    {
        //g_pmdSimSelect |= HAL_ABB_SIM_SELECT_SIM3;
        simEnMask.vSim4Enable = true;
    }

    if ((simEnMask.reg & g_pmdLdoSettings.reg) == 0)
    {
        hal_SimDisableSci();
    }

    //hal_AbbEnableSim(g_pmdSimSelect);

    //    if (sim_Index != 0xff)
    //    {
    //        hal_SimSetSimIndex(sim_Index);
    //    }

    if ((simEnMask.reg & g_pmdLdoSettings.reg) != 0)
    {
        hal_SimEnableSci();
    }
    osiExitCritical(scStatus);
    return 0;
}
