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

#include "hal_config.h"
#include "openCPU_config.h"
#include "boot_config.h"
#include "boot_platform.h"
#include "boot_fdl.h"
#include "hal_chip.h"
#include "boot_debuguart.h"
#include "boot_mem.h"
#include "boot_entry.h"
#include "cmsis_core.h"
#include "boot_secure.h"
#include "boot_spi_flash.h"
#include "boot_vfs.h"
#include "boot_mmu.h"
#include "boot_timer.h"
#include "boot_irq.h"
#include "boot_bsl_cmd.h"
#include "boot_pdl.h"
#include "hal_adi_bus.h"
#include "hal_spi_flash.h"
#include "flash_block_device.h"
#include "fupdate.h"
#include "fupdate_security.h"
#include "fs_mount.h"
#include "osi_log.h"
#include "osi_api.h"
#include "drv_names.h"
#include "calclib/simage.h"
#include <sys/reent.h>
#include "string.h"
#include "boot_efuse.h"
#include "hwregs.h"
#include "af_fota.h"

#include "app_config.h"
#ifdef CONFIG_APP_COMPRESSED_SUPPORT
#include "hal_config.h"
#include "cmsis_core.h"
#include "hal_chip_config.h"
#include "calclib/simage.h"
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include "lzmar.h"
#include "hal_lzma.h"
#endif
#ifdef CONFIG_FIBOCOM_BASE
#include "atr_config.h"
#include <stdlib.h>
#ifdef CONFIG_FIBOCOM_FW_UPDATE_LOGO_SUPPORT
#include "fibo_logo.h"
extern fibo_logo_cfg_t g_fibo_logo_cfg;
#endif
#endif
#ifdef CONFIG_FIBOCOM_SUPPORT_TWO_SPL
#include "calclib/unlzma.h"
#endif

#define ROM_LOADPAR_NORMAL (0)
#define ROM_LOADPAR_FROM_BOOT2 (0x10)
#define FLASH_SECTOR_SIZE (0x1000)
#define FLASH_PAGE_SIZE (256)
#define SLAVE_ACCESS_NO (0)
#define SLAVE_ACCESS_SECURE_ONLY (1)
#define SLAVE_ACCESS_NONSECURE_ONLY (2)
#define SLAVE_ACCESS_SECURE_NONESECURE (3)

#define AON_WDT_HZ (32768)
#define RESTART_WDT_MARGIN_TIME (1800000)
#define WDT_RESET_MIN (10)
#define PMU_BOOT_MODE_REG (hwp_pmicRtcAna->por_rst_monitor)

#ifdef CONFIG_FIBOCOM_SUPPORT_TWO_SPL
extern unsigned gBoot2CompData[];
extern unsigned gBoot2CompSize[];
#endif

#ifdef CONFIG_TRUSTZONE_SUPPORT
#define MASTER_ID_APA5_CODE (0)
#define MASTER_ID_APA5_DATA (1)
#define MASTER_ID_CE_SEC (4)
#define MASTER_ID_EMMC (6)
#define MASTER_ID_USB (9)
#define MASTER_ID_DAP (60)
#define MASTER_ID_FDMA (61)

static inline void firewallEnableMaster(volatile uint32_t *reg, unsigned id)
{
    unsigned regn = id / 32;
    unsigned mask = 1 << (id % 32);
    reg[regn] |= mask;
}

static inline void firewallDisableMaster(volatile uint32_t *reg, unsigned id)
{
    unsigned regn = id / 32;
    unsigned mask = 1 << (id % 32);
    reg[regn] &= ~mask;
}
#endif

#if defined(CONFIG_FIBOCOM_EXTFLASH_FOTA) || defined(CONFIG_FIBOCOM_APP_IN_EXTFLASH_FOTA)
#include "fibocom.h"
const char *gFupdatePackFileName = FUPDATE_PACK_FILE_NAME;
const char *gFupdateStageFileName = FUPDATE_STAGE_FILE_NAME;
const char *gFupdateTempFileName = FUPDATE_TEMP_FILE_NAME;
const char* gFupdateSignFileName = FUPDATE_SIGN_FILE_NAME;
#endif

static void bootSetFirewall()
{
    // unset firewall set by sysrom
    hwp_memFwSpiflash1->seg_0_first_addr = 0xffffffff;
    hwp_memFwSpiflash1->seg_0_last_addr = 0;
    hwp_memFwAonImem->seg_0_first_addr = 0xffffffff;
    hwp_memFwAonImem->seg_0_last_addr = 0;
    hwp_memFwApImem->seg_0_first_addr = 0xffffffff;
    hwp_memFwApImem->seg_0_last_addr = 0;
    hwp_slvFwApAhb->id0_first_addr_0 = 0xffffffff;
    hwp_slvFwApAhb->id0_last_addr_0 = 0;
    hwp_slvFwLpsIfc->id0_first_addr_0 = 0xffffffff;
    hwp_slvFwLpsIfc->id0_last_addr_0 = 0;
    hwp_slvFwApIfc->id0_first_addr_0 = 0xffffffff;
    hwp_slvFwApIfc->id0_last_addr_0 = 0;
#ifdef CONFIG_TRUSTZONE_SUPPORT
    // memeory firewall for AON IMEM address
    firewallEnableMaster(&hwp_memFwAonImem->seg_0_mst_r_id0, MASTER_ID_APA5_CODE);
    firewallEnableMaster(&hwp_memFwAonImem->seg_0_mst_r_id0, MASTER_ID_APA5_DATA);
    firewallEnableMaster(&hwp_memFwAonImem->seg_0_mst_r_id0, MASTER_ID_DAP);
    firewallEnableMaster(&hwp_memFwAonImem->seg_0_mst_r_id0, MASTER_ID_FDMA);
    hwp_memFwAonImem->seg_0_first_addr = (CONFIG_AON_SRAM_PHY_ADDRESS) >> 10;
    hwp_memFwAonImem->seg_0_last_addr = (CONFIG_AON_SRAM_PHY_ADDRESS + 0x2000 - 1) >> 10;
#endif
    // unset flash,uart,keypad, usb firewall
    REGT_FIELD_CHANGE(hwp_slvFwApAhb->rd_sec_1, REG_SLV_FW_AP_AHB_RD_SEC_1_T,
                      spiflash1_reg_rd_sec, SLAVE_ACCESS_SECURE_NONESECURE);
    REGT_FIELD_CHANGE(hwp_slvFwApAhb->wr_sec_1, REG_SLV_FW_AP_AHB_WR_SEC_1_T,
                      spiflash1_reg_wr_sec, SLAVE_ACCESS_SECURE_NONESECURE);

    REGT_FIELD_CHANGE(hwp_slvFwApAhb->rd_sec_1, REG_SLV_FW_AP_AHB_RD_SEC_1_T,
                      med_rd_sec, SLAVE_ACCESS_SECURE_NONESECURE);
    REGT_FIELD_CHANGE(hwp_slvFwApAhb->wr_sec_1, REG_SLV_FW_AP_AHB_WR_SEC_1_T,
                      med_wr_sec, SLAVE_ACCESS_SECURE_NONESECURE);

    REGT_FIELD_CHANGE(hwp_slvFwLpsIfc->rd_sec_0, REG_SLV_FW_LPS_IFC_RD_SEC_0_T,
                      keypad_rd_sec, SLAVE_ACCESS_SECURE_NONESECURE);
    REGT_FIELD_CHANGE(hwp_slvFwLpsIfc->wr_sec_0, REG_SLV_FW_LPS_IFC_WR_SEC_0_T,
                      keypad_wr_sec, SLAVE_ACCESS_SECURE_NONESECURE);

    REGT_FIELD_CHANGE(hwp_slvFwApIfc->rd_sec_0, REG_SLV_FW_AP_IFC_RD_SEC_0_T,
                      uart4_rd_sec, SLAVE_ACCESS_SECURE_NONESECURE);
    REGT_FIELD_CHANGE(hwp_slvFwApIfc->wr_sec_0, REG_SLV_FW_AP_IFC_WR_SEC_0_T,
                      uart4_wr_sec, SLAVE_ACCESS_SECURE_NONESECURE);

    REGT_FIELD_CHANGE(hwp_slvFwLpsIfc->rd_sec_0, REG_SLV_FW_LPS_IFC_RD_SEC_0_T,
                      uart1_rd_sec, SLAVE_ACCESS_SECURE_NONESECURE);
    REGT_FIELD_CHANGE(hwp_slvFwLpsIfc->wr_sec_0, REG_SLV_FW_LPS_IFC_WR_SEC_0_T,
                      uart1_wr_sec, SLAVE_ACCESS_SECURE_NONESECURE);
    REGT_FIELD_CHANGE(hwp_slvFwApAhb->rd_sec_1, REG_SLV_FW_AP_AHB_RD_SEC_1_T,
                      usb_rd_sec, SLAVE_ACCESS_SECURE_NONESECURE);
    REGT_FIELD_CHANGE(hwp_slvFwApAhb->wr_sec_1, REG_SLV_FW_AP_AHB_WR_SEC_1_T,
                      usb_wr_sec, SLAVE_ACCESS_SECURE_NONESECURE);

    // unset emmc firewall
    REGT_FIELD_CHANGE(hwp_slvFwApAhb->rd_sec_0, REG_SLV_FW_AP_AHB_RD_SEC_0_T,
                      emmc_rd_sec, SLAVE_ACCESS_SECURE_NONESECURE);
    REGT_FIELD_CHANGE(hwp_slvFwApAhb->wr_sec_0, REG_SLV_FW_AP_AHB_WR_SEC_0_T,
                      emmc_wr_sec, SLAVE_ACCESS_SECURE_NONESECURE);

    REGT_FIELD_CHANGE(hwp_mstCtrlAp->rd_sec_0, REG_MST_CTRL_AP_RD_SEC_0_T,
                      emmc_rd_sec, SLAVE_ACCESS_SECURE_NONESECURE);
    REGT_FIELD_CHANGE(hwp_mstCtrlAp->wr_sec_0, REG_MST_CTRL_AP_WR_SEC_0_T,
                      emmc_wr_sec, SLAVE_ACCESS_SECURE_NONESECURE);
    REGT_FIELD_CHANGE(hwp_slvFwApAhb->rd_sec_1, REG_SLV_FW_AP_AHB_RD_SEC_1_T,
                      ce_sec_rd_sec, SLAVE_ACCESS_SECURE_NONESECURE);
    REGT_FIELD_CHANGE(hwp_slvFwApAhb->wr_sec_1, REG_SLV_FW_AP_AHB_WR_SEC_1_T,
                      ce_sec_wr_sec, SLAVE_ACCESS_SECURE_NONESECURE);
}

static void bootUsbParamCalibration()
{
    uint32_t usb_cfg;
    uint8_t tuneotg = 0;
    uint8_t tfregres = 0;
    uint8_t tfhsres = 0;
    bool low = bootEfuseRead(true, HAL_EFUSE_DOUBLE_BLOCK_USB, &usb_cfg);
    if (low && (usb_cfg & 0x1) == 1)
    {
        tfhsres = usb_cfg >> 4;
        tfregres = usb_cfg >> 9;
        tuneotg = usb_cfg >> 1;
        hwp_analogG2->analog_usb20_usb20_trimming &= ~ANALOG_G2_ANALOG_USB20_USB20_TFHSRES(0x1f);
        hwp_analogG2->analog_usb20_usb20_trimming &= ~ANALOG_G2_ANALOG_USB20_USB20_TFREGRES(0x3f);
        hwp_analogG2->analog_usb20_usb20_trimming &= ~ANALOG_G2_ANALOG_USB20_USB20_TUNEOTG(0x7);
        hwp_analogG2->analog_usb20_usb20_trimming |= ANALOG_G2_ANALOG_USB20_USB20_TFHSRES(tfhsres);
        hwp_analogG2->analog_usb20_usb20_trimming |= ANALOG_G2_ANALOG_USB20_USB20_TFREGRES(tfregres);
        hwp_analogG2->analog_usb20_usb20_trimming |= ANALOG_G2_ANALOG_USB20_USB20_TUNEOTG(tuneotg);
    }
}

OSI_UNUSED static bool bootWdtStart(uint32_t reset_ms)
{
    if (reset_ms < WDT_RESET_MIN)
        return false;

    uint32_t reload = (reset_ms * (uint64_t)AON_WDT_HZ) / 1000;

    REG_PMIC_RTC_ANA_MODULE_EN0_T module_en0;
    REG_PMIC_RTC_ANA_RTC_CLK_EN0_T rtc_clk_en0;
    REG_PMIC_WDT_WDG_CTRL_T wdg_ctrl;
    halAdiBusBatchChange(
        &hwp_pmicWdt->wdg_ctrl,
        HAL_ADI_BUS_OVERWITE(0), // wdg_run
        &hwp_pmicRtcAna->module_en0,
        REG_FIELD_MASKVAL1(module_en0, wdg_en, 1),
        &hwp_pmicRtcAna->rtc_clk_en0,
        REG_FIELD_MASKVAL1(rtc_clk_en0, rtc_wdg_en, 1),
        &hwp_pmicWdt->wdg_load_high,
        HAL_ADI_BUS_OVERWITE(reload >> 16),
        &hwp_pmicWdt->wdg_load_low,
        HAL_ADI_BUS_OVERWITE(reload & 0xffff),
        &hwp_pmicWdt->wdg_ctrl,
        REG_FIELD_MASKVAL2(wdg_ctrl, wdg_open, 1, wdg_rst_en, 1),
        HAL_ADI_BUS_CHANGE_END);

    return true;
}

OSI_UNUSED static void bootWdtStop()
{
    REG_PMIC_RTC_ANA_MODULE_EN0_T module_en0;
    REG_PMIC_RTC_ANA_RTC_CLK_EN0_T rtc_clk_en0;
    halAdiBusBatchChange(&hwp_pmicWdt->wdg_ctrl, HAL_ADI_BUS_OVERWITE(0), // wdg_run
                         &hwp_pmicRtcAna->module_en0, REG_FIELD_MASKVAL1(module_en0, wdg_en, 0),
                         &hwp_pmicRtcAna->rtc_clk_en0, REG_FIELD_MASKVAL1(rtc_clk_en0, rtc_wdg_en, 0),
                         HAL_ADI_BUS_CHANGE_END);
}

static void bootWdtInit()
{
#ifdef CONFIG_WDT_ENABLE
    bootWdtStart(RESTART_WDT_MARGIN_TIME);
#else
    // WDT is enabled by default. Later complete WDT management is needed.
    bootWdtStop();
#endif
}

OSI_UNUSED static bool prvCheckNormalPowerUp()
{
#ifndef CONFIG_CHARGER_POWERUP
    return (bootPowerOnCause() != OSI_BOOTCAUSE_CHARGE);
#endif
    return true;
}

void bootSetUartIomux()
{
#if defined(CONFIG_CHIP_8850_V3_BOARD) || defined(CONFIG_CHIP_8850_V4_BOARD) || defined(CONFIG_CHIP_8850_V111_BOARD)
    hwp_iomux->gpio_2 = 0x1c;  //UART2_cts
    hwp_iomux->keyout_4 = 0xc; // UART_4_RX
    hwp_iomux->keyout_5 = 0xc; // UART_4_TX
    hwp_iomux->keyout_4 &= ~IOMUX_PAD_KEYOUT_4_DRV(0x3);
    hwp_iomux->keyout_4 |= IOMUX_PAD_KEYOUT_4_DRV(2);
    hwp_iomux->keyout_4 &= ~IOMUX_WPU;
    hwp_iomux->keyout_4 |= IOMUX_WPU;
    hwp_iomux->keyout_5 &= ~IOMUX_PAD_KEYOUT_5_DRV(0x3);
    hwp_iomux->keyout_5 |= IOMUX_PAD_KEYOUT_5_DRV(2);
#else
    hwp_iomux->gpio_16 = 0x18;    //UART2
    hwp_iomux->uart_2_cts = 0x10; // UART_4_RX
    hwp_iomux->uart_2_rts = 0x10; // UART_4_TX
    hwp_iomux->pad_uart_2_cts &= ~IOMUX_PAD_UART_2_CTS_DRV(0x3);
    hwp_iomux->pad_uart_2_cts |= IOMUX_PAD_UART_2_CTS_DRV(2);
    hwp_iomux->pad_uart_2_cts &= ~IOMUX_WPU;
    hwp_iomux->pad_uart_2_cts |= IOMUX_WPU;
    hwp_iomux->pad_uart_2_rts &= ~IOMUX_PAD_UART_2_RTS_DRV(0x3);
    hwp_iomux->pad_uart_2_rts |= IOMUX_PAD_UART_2_RTS_DRV(2);
#endif
}

OSI_UNUSED static bool prvFlashCopy(uint32_t fromaddr, uint32_t toaddr, uint32_t psize)
{
    //OSI_LOGI(0xffffffff, "dual boot copy from 0x%x to 0x%x, size 0x%x", fromaddr, toaddr, psize);

    bootSpiFlash_t *flash = bootSpiFlashOpen(DRV_NAME_SPI_FLASH);
    if (flash == NULL)
        return false;

    uint32_t to_off = HAL_FLASH_OFFSET(toaddr);
    uint32_t esize = OSI_ALIGN_UP(psize, FLASH_SECTOR_SIZE);
    if (!bootSpiFlashErase(flash, to_off, esize))
        return false;

    // bootSpiFlashWrite data can't be located in the same flash
    // fromaddr should be always sector aligned
    uint8_t ramdata[FLASH_PAGE_SIZE];
    const uint8_t *data = (const uint8_t *)fromaddr;
    while (psize > 0)
    {
        unsigned wr = OSI_MIN(unsigned, psize, FLASH_PAGE_SIZE);
        memcpy(ramdata, data, wr);

        if (!bootSpiFlashWrite(flash, to_off, ramdata, wr))
            return false;

        psize -= wr;
        to_off += wr;
        data += wr;
    }
    return true;
}

#ifndef CONFIG_FIBOCOM_SUPPORT_TWO_SPL
#ifndef CONFIG_FIBOCOM_XINGUODU
#ifdef CONFIG_DUAL_BOOTLOADER_ENABLE
static void prvDualSplCheck(uint32_t param)
{
    if (param == ROM_LOADPAR_NORMAL)
    {
        const simageHeader_t *header = (const simageHeader_t *)CONFIG_SPL1_FLASH_ADDRESS;
        if (header->image_size > CONFIG_SPL_SIZE)
            osiPanic();

        if (memcmp((void *)CONFIG_SPL1_FLASH_ADDRESS,
                   (void *)CONFIG_SPL2_FLASH_ADDRESS,
                   header->image_size) == 0)
            return;

        if (!prvFlashCopy(CONFIG_SPL1_FLASH_ADDRESS,
                          CONFIG_SPL2_FLASH_ADDRESS,
                          header->image_size))
            osiPanic();
    }
    else if (param == ROM_LOADPAR_FROM_BOOT2)
    {
        const simageHeader_t *header = (const simageHeader_t *)CONFIG_SPL2_FLASH_ADDRESS;
        if (header->image_size > CONFIG_SPL_SIZE)
            osiPanic();

        // Most likely, they are different. However, comparing them anyway.
        if (memcmp((void *)CONFIG_SPL2_FLASH_ADDRESS,
                   (void *)CONFIG_SPL1_FLASH_ADDRESS,
                   header->image_size) == 0)
            return;

        if (!prvFlashCopy(CONFIG_SPL2_FLASH_ADDRESS,
                          CONFIG_SPL1_FLASH_ADDRESS,
                          header->image_size))
            osiPanic();
    }
}
#endif
#endif

static void prvFirmwareUpdatePreProgress(int block_count, int block)
{
    fupdateContentType_t type = FUPDATE_CONTENT_INVALID;

    type = fupdateGetUpdatingType();
    if (type == FUPDATE_CONTENT_FLASH)
    {
        fupdateFlashContent_t flash_content = {};
        bool result = fupdateGetFlashUpdatingContent(&flash_content);
        OSI_LOGI(0x10009078, "FUPDATE PreProgress ret %d flash name/offset %4c/0x%x",
                 result, flash_content.device_name, flash_content.offset);
    }
    else
    {
        OSI_LOGI(0x10009079, "FUPDATE PreProgress current type %d", type);
    }
}

static void prvFirmwareUpdateProgress(int block_count, int block)
{
#ifdef CONFIG_FIBOCOM_FUPDATE_REPORT
#ifdef CONFIG_FIBOCOM_AT_QUE_FEATURE
    char send_data[24] = "\r\n+QIND: \"FOTA\",";
    char send_data2[16] = "\"UPDATING\",";
    char send_data3[6] = "\r\n";
    int per = block*100/block_count;
    char per_t[4] = {0};
    itoa(per,per_t,10);
    strcat(send_data2,per_t);

    fdlChannel_t *ch = fdlOpenUart(DRV_NAME_UART1, CONFIG_ATR_DEFAULT_UART_BAUD, true);
    if(block == 1)
    {
        ch->write(ch, send_data, strlen(send_data));
        ch->flush(ch);
        ch->write(ch, "\"START\"\r\n", sizeof("\"START\"\r\n")-1);
        ch->flush(ch);
    }
    ch->write(ch, send_data, strlen(send_data));
    ch->flush(ch);
    ch->write(ch, send_data2, strlen(send_data2));
    ch->flush(ch);
    ch->write(ch, send_data3, strlen(send_data3));
    ch->flush(ch);
#else
    char send_data[8] = "GTOTA ";
    char send_data2[10] = "UPDATE ";
    char send_data3[6] = "%\r\n";
    int per = block*100/block_count;
    char per_t[4] = {0};
    itoa(per,per_t,10);
    strcat(send_data2,per_t);

    fdlChannel_t *ch = fdlOpenUart(DRV_NAME_UART1, CONFIG_ATR_DEFAULT_UART_BAUD, true);
    if(block == 1)
    {
        ch->write(ch, send_data, strlen(send_data));
        ch->flush(ch);
        ch->write(ch, "START\r\n", sizeof("START\r\n")-1);
        ch->flush(ch);
    }
    ch->write(ch, send_data, strlen(send_data));
    ch->flush(ch);
    ch->write(ch, send_data2, strlen(send_data2));
    ch->flush(ch);
    ch->write(ch, send_data3, strlen(send_data3));
    ch->flush(ch);
#endif
#endif
    OSI_LOGI(0x10009056, "FUPDATE block: %d/%d", block, block_count);

#ifdef CONFIG_FIBOCOM_FW_UPDATE_LOGO_SUPPORT
    if (block != 0) { // fota upgrade start
        if (block < block_count) {
            if ((block & 1) == 1) {
                fibo_logo_display(g_fibo_logo_cfg.update_logo_path[0], g_fibo_logo_cfg.update_logo_bgcolor);
            } else {
                fibo_logo_display(g_fibo_logo_cfg.update_logo_path[1], g_fibo_logo_cfg.update_logo_bgcolor);
            }
        }else if (block_count == block) { // fota upgrade end
            fibo_logo_release();
        }
    }
#endif
}

OSI_UNUSED static fupdateResult_t prvFirmwareUpdate(void)
{
    int fota_appfw_flag_fd = -1;

    fsMountSetScenario(FS_SCENRARIO_BOOTLOADER);
    if (!fsMountAll())
        return FUPDATE_RESULT_NOT_READY;

    fota_appfw_flag_fd = vfs_open(FILE_FOTA_APPFW_FLAG, O_RDONLY);
    OSI_LOGI(0, "fota_appfw_flag_fd: %d", fota_appfw_flag_fd);
    if(-1 != fota_appfw_flag_fd)
    {
        vfs_close(fota_appfw_flag_fd);
        vfs_unlink(FUPDATE_PACK_FILE_NAME);
        vfs_unlink(FUPDATE_SIGN_FILE_NAME);
        vfs_rmdir(CONFIG_FS_FOTA_DATA_DIR);
        vfs_unlink(FILE_FOTA_BIN);
        vfs_rmdir(DENTRY_APPFOTA);
        vfs_unlink(FILE_FOTA_APPFW_FLAG);
    }
    bool check = fupdateCheckSignedPack();
    if (!check)
    {
        OSI_LOGE(0x10009057, "FUPDATE sign check failed");
        fsUmountAll();
        return FUPDATE_RESULT_NOT_READY;
    }

#ifdef CONFIG_FUPDATE_CHECK_SECURE_VERSION
    if (!fupdateCheckSecureVersion())
    {
        OSI_LOGE(0x1000abad, "FUPDATE CheckSecureVersion fail.");
        fsUmountAll();
        return FUPDATE_RESULT_NOT_READY;
    }
#endif
#ifdef CONFIG_FUPDATE_SUPPORT_CHECK_TMP_SPACE
//TRUSTZONE has no space check in spl because of code size limit
#ifndef CONFIG_TRUSTZONE_SUPPORT
    if (fupdateGetStatus() == FUPDATE_STATUS_READY)
    {
        check = fupdateIsTmpSpaceEnough();
        if (!check)
        {
            fsUmountAll();
            return FUPDATE_RESULT_CANNT_START;
        }
    }
#endif
#endif

    fupdateResult_t result = fupdateRunV2(prvFirmwareUpdatePreProgress, prvFirmwareUpdateProgress);

    OSI_LOGI(0x10009058, "FUPDATE: %d", result);
    if (result == FUPDATE_RESULT_FAILED)
        bootReset(BOOT_RESET_NORMAL);
    else if (result == FUPDATE_RESULT_FINISHED)
        fupdateSetBootAfterFota(FUPDATE_RESULT_FINISHED);

// Umount FS After load ap compressed image.
#if !defined(CONFIG_APP_COMPRESSED_SUPPORT)
    fsUmountAll();
#endif

    return result;
}

static bool splVerifySimage(simageHeader_t *header, unsigned max_size)
{
    if (!simageValid(header, max_size))
        return false;

    return bootSimageCheckSign(header);
}

// check the signatures of SML, TOS, Boot, App, openCPU
static simageHeader_t *splCheckSignatures()
{
    simageHeader_t *header_go = NULL;
    simageHeader_t *header_verify = NULL;

#ifndef CONFIG_BOOT_FLASH_DISABLE
    // boot
    header_verify = (simageHeader_t *)CONFIG_APP_RAM_ADDRESS;
    memcpy(header_verify, (void *)CONFIG_BOOT_FLASH_ADDRESS, CONFIG_BOOT_FLASH_SIZE);
    if (!splVerifySimage(header_verify, CONFIG_BOOT_FLASH_SIZE))
        return NULL;
    header_go = header_verify;
#endif
    // app
    header_verify = (simageHeader_t *)CONFIG_APP_FLASH_ADDRESS;
    if (!splVerifySimage(header_verify, CONFIG_APP_FLASH_SIZE))
        return NULL;
#ifdef CONFIG_BOOT_FLASH_DISABLE
#ifdef CONFIG_OPENCPU_BOOT_FROM_BOOTLOADER
    header_verify = (simageHeader_t *)CONFIG_OPENCPU_FLASH_ADDRESS;
#endif
    header_go = header_verify;
#endif

#ifdef CONFIG_OPENCPU_ENABLED
    // openCPU
    header_verify = (simageHeader_t *)CONFIG_OPENCPU_FLASH_ADDRESS;
    if (!splVerifySimage(header_verify, CONFIG_OPENCPU_FLASH_SIZE))
        return NULL;
#endif

#ifdef CONFIG_TRUSTZONE_SUPPORT
    // sml
    header_verify = (simageHeader_t *)CONFIG_TZ_SML_FLASH_ADDRESS;
    if (!splVerifySimage(header_verify, CONFIG_TZ_SML_FLASH_SIZE))
        return NULL;
    memcpy((void *)header_verify->data_entry,
           (void *)(CONFIG_TZ_SML_FLASH_ADDRESS + header_verify->data_offset),
           header_verify->data_size);
    header_go = header_verify;

    // tos
    header_verify = (simageHeader_t *)CONFIG_TZ_TOS_FLASH_ADDRESS;
    if (!splVerifySimage(header_verify, CONFIG_TZ_TOS_FLASH_SIZE))
        return NULL;
    memcpy((void *)header_verify->data_entry,
           (void *)(CONFIG_TZ_TOS_FLASH_ADDRESS + header_verify->data_offset),
           header_verify->data_size);
#endif

    return header_go;
}
#endif

#if defined(CONFIG_APP_COMPRESSED_SUPPORT)

#define MAX_FILE_SIZE (128 * 1024)
#define MAX_BLOCK_SIZE (128 * 1024)
#define ALIGN_UP(n, a) (((n) + (a)-1) & ~((a)-1))

enum
{
    E_METHOD_SOFT = 1,
    E_METHOD_HARD = 2
};

static int g_decMethod = 0;

int prvGetDecMethod(char *data)
{
    int dict_size = halLzmaDictSize(data);
    if (dict_size == 1024)
        return E_METHOD_HARD;
    else if (dict_size > 1024)
        return E_METHOD_SOFT;
    else
        return 0;
}

static int prvGetLoadAddr(const char *name)
{
    void *bs = NULL;
    void *bf = NULL;
    void *bsAlign = NULL;
    void *bfAlign = NULL;
    int fd = 0;

    bf = malloc(MAX_FILE_SIZE);
    if (NULL == bf)
        goto fail;
    bfAlign = (void *)ALIGN_UP((uint32_t)bf, 32);

    bs = malloc(MAX_BLOCK_SIZE);
    if (NULL == bs)
    {
        goto fail;
    }
    bsAlign = (void *)ALIGN_UP((uint32_t)bs, 32);

    char part_name[128] = {0};
    strcpy(part_name, name);
    strcat(part_name, ".000");

    //OSI_LOGXI(OSI_LOGPAR_S, 0, "part_name: %s", part_name);

    fd = vfs_open(part_name, O_RDONLY);
    if (fd < 0)
    {
        OSI_LOGI(0, "open err");
        goto fail;
    }
    size_t fsize = vfs_read(fd, bfAlign, MAX_FILE_SIZE);
    //OSI_LOGXI(OSI_LOGPAR_SSI, 0, "ap: %s %s %d", name, part_name, fsize);

    if (fsize < 0)
    {
        goto fail;
    }

    if (0 == (g_decMethod = prvGetDecMethod((char *)bfAlign)))
        goto fail;

    if (g_decMethod == E_METHOD_SOFT)
    {
        int data_size = lzmarDataSize(bfAlign);
        int dsize = lzmarDecompress(bfAlign, fsize, bsAlign, (size_t)data_size);
        if (dsize != data_size)
        {
            goto fail;
        }
    }
    else
    {
        int data_size = halLzmaDecompressFile(bfAlign, bsAlign);
        if (data_size < 0)
        {
            goto fail;
        }
    }

    const simageHeader_t *header = (simageHeader_t *)bsAlign;
    //OSI_LOGI(0, "prvGetLoadAddr addr:0x%x", header->data_load);

    vfs_close(fd);
    free(bf);
    free(bs);

    return header->data_load;

fail:
    //OSI_LOGI(0, "prvGetLoadAddr fail");
    if (fd > 0)
        vfs_close(fd);
    if (bf)
        free(bf);
    if (bs)
        free(bs);
    return -1;
}

static int prvSoftLzma3Load(const char *name, void *output)
{
    void *file_buf = malloc(MAX_FILE_SIZE);
    if (NULL == file_buf)
        return -1;

    void *dest_buf = malloc(MAX_BLOCK_SIZE);
    if (NULL == dest_buf)
    {
        free(file_buf);
        return -1;
    }
    void *destAlign = NULL;
    destAlign = (void *)ALIGN_UP((uint32_t)dest_buf, 32);

    char *ps = (char *)output;
    unsigned out_size = 0;
    char part_name[128] = {0};
    int fd = 0;

    for (int n = 0;; n++)
    {
        strcpy(part_name, name);
        unsigned fname_len = strlen(part_name);
        part_name[fname_len] = '.';
        part_name[fname_len + 1] = '0' + n / 100;
        part_name[fname_len + 2] = '0' + (n % 100) / 10;
        part_name[fname_len + 3] = '0' + n % 10;
        part_name[fname_len + 4] = '\0';

        fd = vfs_open(part_name, O_RDONLY);
        if (fd < 0)
            break;

        size_t fsize = vfs_read(fd, file_buf, MAX_FILE_SIZE);
        //OSI_LOGXI(OSI_LOGPAR_SSI, 0, "cp: %s %s %d", name, part_name, fsize);
        vfs_close(fd);

        if (fsize < 0)
        {
            goto fail;
        }
        int dsize = 0;
        if (g_decMethod == E_METHOD_SOFT)
        {
            int data_size = lzmarDataSize(file_buf);
            dsize = lzmarDecompress(file_buf, fsize, destAlign, (size_t)data_size);
            if (dsize != data_size)
            {
                goto fail;
            }
        }
        else
        {
            dsize = halLzmaDecompressFile(file_buf, destAlign);
            if (dsize < 0)
            {
                goto fail;
            }
        }
        memcpy(ps, destAlign, dsize);
        ps += dsize;
        out_size += dsize;
    }

    free(file_buf);
    free(dest_buf);
    return out_size;

fail:
    if (fd > 0)
        vfs_close(fd);
    if (file_buf)
        free(file_buf);
    if (dest_buf)
        free(dest_buf);
    return -1;
}

bool prvLoadAppImg()
{
    int out = 0;
    char apbin[64] = {0};

    sprintf(apbin, CONFIG_FS_MODEM_MOUNT_POINT "/ap_compress.img");

    OSI_LOGXI(OSI_LOGPAR_S, 0, "LoadApImg apbin:%s", apbin);

    uint32_t addr = 0;
    if ((addr = prvGetLoadAddr(apbin)) <= 0)
        return false;

    if ((uint32_t)addr < CONFIG_APP_RAM_ADDRESS ||
        (uint32_t)addr > CONFIG_APP_RAM_ADDRESS + CONFIG_APP_RAM_SIZE)
        return false;

    uint32_t start = addr - SIMAGE_HEADER_SIZE;
    if ((out = prvSoftLzma3Load(apbin, (void *)start)) <= 0)
        return false;

    L1C_InvalidateICacheAll();
    L1C_CleanDCacheAll();
    __DSB();
    __ISB();

    //OSI_LOGI(0, "LoadApImg apbin: ok");
    const simageHeader_t *header = (simageHeader_t *)start;
    if (!simageValid(header, out))
        return false;

    OSI_LOGI(0, "LoadApImg apbin: check ok");

    return true;
}

#endif

#ifndef CONFIG_FIBOCOM_SUPPORT_TWO_SPL
static void prvSetFlashWriteProhibit(void)
{
    // ATTENTION: This will set write prohibit for bootloader
    //
    // If there are requiement (though not reasonable) to change bootloader
    // the followings should be changed. And when there are more regions are
    // known never changed, more regions can be added.
#ifndef CONFIG_BOOT_FLASH_DISABLE
    bootSpiFlash_t *flash = bootSpiFlashOpen(HAL_FLASH_DEVICE_NAME(CONFIG_BOOT_FLASH_ADDRESS));
    bootSpiFlashSetRangeWriteProhibit(flash, HAL_FLASH_OFFSET(CONFIG_BOOT_FLASH_ADDRESS),
                                      HAL_FLASH_OFFSET(CONFIG_BOOT_FLASH_ADDRESS) + CONFIG_BOOT_FLASH_SIZE);
#endif
}
#endif

#ifdef CONFIG_TRUSTZONE_SUPPORT
extern uint32_t __warmboot_start;
extern uint32_t __warmboot_end;
extern void WarmbootEntry(void);
#endif
void bootStart(uint32_t param)
{
    OSI_CLEAR_SECTION(bss);
#ifndef CONFIG_FIBOCOM_SUPPORT_TWO_SPL
    bootSpiFlashOpen(DRV_NAME_SPI_FLASH); // ensure accessible
#endif
    halClockInit(HAL_CLOCK_INIT_BOOTLOADER);
    bootSetFirewall();
    bootUsbParamCalibration();

    halRamInit();
    bootMmuEnable();

    __FPU_Enable();
#ifndef CONFIG_FIBOCOM_SUPPORT_TWO_SPL
    _REENT_INIT_PTR_ZEROED(_impure_ptr);
#endif
    halAdiBusInit();
    bootResetPinEnable();
    bootWdtInit();
#ifndef CONFIG_FIBOCOM_SUPPORT_TWO_SPL
    bootGetEfuseCtx();
#endif
#if 0
    if (bootIsFromPsmSleep())
    {
#ifdef CONFIG_TRUSTZONE_SUPPORT
            simageHeader_t *header = bootloaderJumpImageEntry();
            if(header == NULL)
                goto failed;
            SmlJumpImageEntry();
#else
            simageHeader_t *header = (simageHeader_t *)CONFIG_APP_FLASH_ADDRESS;
            simageJumpEntry(header, OSI_SHUTDOWN_PSM_SLEEP); // never return
#endif
    }
#endif

#ifndef CONFIG_FIBOCOM_SUPPORT_TWO_SPL
#ifdef CONFIG_OPENCPU_ENABLED
    uint32_t boot_mode = halAdiBusRead(&PMU_BOOT_MODE_REG);
    if (boot_mode == OSI_SHUTDOWN_OPENCPU_SWITCH)
    {
        halAdiBusWrite(&PMU_BOOT_MODE_REG, 0);
        simageHeader_t *header = (simageHeader_t *)CONFIG_OPENCPU_FLASH_ADDRESS;
        simageJumpEntry(header, boot_mode);
    }
#endif

    if (!prvCheckNormalPowerUp())
    {
        osiDelayUS(1000 * 10);
        bootPowerOff();
    }
#endif
    bool trace_enable = false;
#ifdef CONFIG_BOOT_LOG_ENABLED
    trace_enable = true;
#endif
    bootSetUartIomux();
    bootTraceInit(trace_enable);
    bootPlatformInit();

    extern uint32_t __sram_heap_start[];
    extern uint32_t __sram_heap_end[];
    extern uint32_t __ram_heap_start[];
    extern uint32_t __ram_heap_end[];
    unsigned sram_heap_size = OSI_PTR_DIFF(__sram_heap_end, __sram_heap_start);
    unsigned ram_heap_size = OSI_PTR_DIFF(__ram_heap_end, __ram_heap_start);
    bootHeapInit(__sram_heap_start, sram_heap_size, __ram_heap_start, ram_heap_size);
    bootHeapDefaultExtRam();

#ifndef CONFIG_FIBOCOM_SUPPORT_TWO_SPL
    halPmuExtFlashPowerOn();
#ifdef CONFIG_FIBOCOM_BASE
//2024-08-01, kangmengjiang, MTC0745-440.pmic determines that the shutdown voltage of power failure is raised to
//improve the stability of CPU operation in scenarios where power is quickly cut off for SPCSS01351167.
    halPmuBatCrashVolSetting(3);
//2024-08-01, kangmengjiang, MTC0745-440.End.
#endif
    fupdateResult_t result = prvFirmwareUpdate();
    if (result == FUPDATE_RESULT_FINISHED)
        bootReset(BOOT_RESET_NORMAL);

    #ifndef CONFIG_FIBOCOM_XINGUODU
    #ifdef CONFIG_DUAL_BOOTLOADER_ENABLE
    prvDualSplCheck(param);//img less than 80K 
    #endif
    #endif
	
#if defined(CONFIG_APP_COMPRESSED_SUPPORT)
    if (!prvLoadAppImg())
        goto failed;
    fsUmountAll();
#endif

    prvSetFlashWriteProhibit();
#endif
#ifndef CONFIG_FIBOCOM_SUPPORT_TWO_SPL
    if (!bootUpdateVersion())
        goto failed;
    simageHeader_t *header = splCheckSignatures();
    if (header == NULL)
        goto failed;
#else
    void *comp_data = gBoot2CompData;
    unsigned comp_size = (unsigned)gBoot2CompSize;
    long out_size, dec_bufsize;

    if (lzmaDecompressProbe(comp_data, comp_size, &out_size, &dec_bufsize) < 0)
        goto failed;

    // LZMA working buffer is memory after decompressed data, with an extra cache line
    void *dec_buf = (void *)(CONFIG_RAM_PHY_ADDRESS +
                             OSI_ALIGN_UP(out_size, CONFIG_CACHE_LINE_SIZE) +
                             CONFIG_CACHE_LINE_SIZE);
    void *spl2_addr = (void *)CONFIG_RAM_PHY_ADDRESS;

    if (lzmaDecompressMemStatic(comp_data, comp_size, spl2_addr, dec_buf, NULL) < 0)
        goto failed;

    simageHeader_t *header = (simageHeader_t *)spl2_addr;
#endif

    simageJumpEntry(header, 0); // never return

failed:
#ifdef CONFIG_TRUSTZONE_SUPPORT
    if (__warmboot_start == __warmboot_end)
    { //never get here
        WarmbootEntry();
    }
#endif
    OSI_DEAD_LOOP;
}
