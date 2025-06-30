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

#include "drv_spi_flash.h"
#include "hal_spi_flash.h"
#include "hal_spi_flash_defs.h"
#include "hal_chip.h"
#include "hwregs.h"
#include "osi_api.h"
#include "osi_log.h"
#include "osi_profile.h"
#include "osi_byte_buf.h"
#ifdef CONFIG_CPU_ARM
#include "cmsis_core.h"
#endif
#include <string.h>
#ifdef CONFIG_TFM_SUPPORT
#include "tfm_func_redef.h"
#endif
#include "fibocom.h"
#include "hal_chip.h"

#ifdef CONFIG_FIBOCOM_BASE
extern uint8_t spiflash_is_4line;
#endif
#define FLASHRAM_CODE OSI_SECTION_LINE(.ramtext.flashdrv)
#define FLASHRAM_API FLASHRAM_CODE OSI_NO_INLINE

// From experiments, 2us is enough. However, it is better to follow datasheet
#define DELAY_AFTER_SUSPEND (20) // us

// From experiments, if the time from resume to suspend is too short,
// the erase will take huge time to finish. Though in most cases there
// doesn't exist soo many interrupt, it is needed to avoid worst case.
#define MIN_ERASE_PROGRAM_TIME (100)        // us
#define MIN_ERASE_PROGRAM_RESUME_TIME (150) // us

#define FLASH_MAP_ADDRESS(d, offset) (const void *)REG_ACCESS_ADDRESS((uintptr_t)d->base_address + offset)
#define FLASH_ADDRESS_S_TO_NS(adr) ((void *)((uint32_t)(adr) & (~(0x1UL << 28))))

#if defined(CONFIG_SOC_8910) && defined(CONFIG_SUPPORT_LC_FLASH)
#define size_16M 0x1000000
#define size_32M 0x2000000
#endif

enum
{
    FLASH_NO_ERASE_PROGRAM,
    FLASH_ERASE,
    FLASH_PROGRAM,
    FLASH_ERASE_SUSPEND,
    FLASH_PROGRAM_SUSPEND
};

struct drvSpiFlash
{
    bool opened;
    bool xip_protect;
    bool suspend_disable;
    unsigned name;
    halSpiFlash_t flash;
    uintptr_t base_address;
    osiMutex_t *lock; // suspend will cause thread switch
#if defined(CONFIG_SOC_8910) && defined(CONFIG_SUPPORT_LC_FLASH)
    uint32_t block_prohibit[32]; // 16M/64K/32bit_per_word
#else
    uint32_t block_prohibit[8];
#endif
#ifdef CONFIG_FIBOCOM_BASE
    osiPmSource_t *pm_source;
#endif
};

#if defined(CONFIG_SOC_8910) || defined(CONFIG_SOC_8811) || defined(CONFIG_SOC_8850) || defined(CONFIG_SOC_8821)
static drvSpiFlash_t gDrvSpiFlashCtx[] = {
    {
        .opened = false,
        .xip_protect = true,
        .name = DRV_NAME_SPI_FLASH,
        .flash.hwp = (uintptr_t)hwp_spiFlash,
        .base_address = CONFIG_NOR_PHY_ADDRESS,
    },
    {
        .opened = false,
        .xip_protect = true,
        .name = DRV_NAME_SPI_FLASH_EXT,
        .flash.hwp = (uintptr_t)hwp_spiFlashExt,
        .base_address = CONFIG_NOR_EXT_PHY_ADDRESS,
    },
};
#endif

#ifdef CONFIG_SOC_8821
extern uint8_t NL1cSysNtnCheckDlTmAdvState();
#endif


#ifdef CONFIG_FIBOCOM_BASE
static uint32_t g_regVal_spi_config = 0x00;
static uint32_t g_regVal_cgm_spiflash2_sel_cfg = 0x00;
static uint32_t g_regVal_cfg_clk_spiflash2 = 0x00;



static void _spiFlashExtSuspend(void *ctx, osiSuspendMode_t mode)
{
    g_regVal_spi_config = *(volatile uint32_t *)(&hwp_spiFlashExt->spi_config);
    g_regVal_cgm_spiflash2_sel_cfg = *(volatile uint32_t *)(&hwp_apClk->cgm_spiflash2_sel_cfg);
    g_regVal_cfg_clk_spiflash2 = *(volatile uint32_t *)(&hwp_apApb->cfg_clk_spiflash2);
}

static void _spiFlashExtResume(void *ctx, osiSuspendMode_t mode, uint32_t source)
{
    halApplyRegisters((uint32_t)&hwp_spiFlashExt->spi_config, g_regVal_spi_config, REG_APPLY_END);
    halApplyRegisters((uint32_t)&hwp_apClk->cgm_spiflash2_sel_cfg, g_regVal_cgm_spiflash2_sel_cfg, REG_APPLY_END);
    halApplyRegisters((uint32_t)&hwp_apApb->cfg_clk_spiflash2, g_regVal_cfg_clk_spiflash2, REG_APPLY_END);
}

static const osiPmSourceOps_t _spiFlashExtPmOps = {
    .suspend = _spiFlashExtSuspend,
    .resume = _spiFlashExtResume,
};
#endif

/**
 * Find flash instance by name
 */
OSI_FORCE_INLINE static inline drvSpiFlash_t *prvFlashGetByName(uint32_t name)
{
    for (unsigned n = 0; n < OSI_ARRAY_SIZE(gDrvSpiFlashCtx); n++)
    {
        if (gDrvSpiFlashCtx[n].name == name)
            return &gDrvSpiFlashCtx[n];
    }
    return NULL;
}

/**
 * Flash API protection acquire, it is different for XIP/NON-XIP
 */
OSI_FORCE_INLINE static uint32_t prvProtectAquire(drvSpiFlash_t *d)
{
    osiMutexLock(d->lock);
    return (d->xip_protect) ? osiEnterCritical() : 0;
}

/**
 * Flash API protection release, it is different for XIP/NON-XIP
 */
OSI_FORCE_INLINE static void prvProtectRelease(drvSpiFlash_t *d, uint32_t critical)
{
    if (d->xip_protect)
        osiExitCritical(critical);
    osiMutexUnlock(d->lock);
}

#ifndef CONFIG_TFM_SUPPORT
/**
 * Whether address is soft write protected
 */
OSI_FORCE_INLINE static bool prvIsWriteProhibit(drvSpiFlash_t *d, uint32_t address)
{
    unsigned block = address / SIZE_64K; // 64KB block
    return osiBitmapIsSet(d->block_prohibit, block);
}

/**
 * Whether range is soft write protected
 */
FLASHRAM_CODE static bool prvIsRangeWriteProhibit(drvSpiFlash_t *d, uint32_t address, unsigned size)
{
    uint32_t critical = osiEnterCritical();
    for (unsigned n = 0; n < size; n += SIZE_64K)
    {
        if (prvIsWriteProhibit(d, address + n))
        {
            osiExitCritical(critical);
            return true;
        }
    }
    osiExitCritical(critical);
    return false;
}

/**
 * Disable AHB read of flash controller
 */
OSI_FORCE_INLINE static void prvDisableAhbRead(drvSpiFlash_t *d)
{
#if defined(CONFIG_SOC_8910) || defined(CONFIG_SOC_8850)
    // not use AHB_READ_DISABLE feature
#elif defined(CONFIG_SOC_8811) || defined(CONFIG_SOC_8821)
    HWP_SPI_FLASH_T *hwp = (HWP_SPI_FLASH_T *)d->flash.hwp;
    REG_SPI_FLASH_AHB_FLASH_CTRL_T ahb_flash_ctrl;
    REG_FIELD_CHANGE1(hwp->ahb_flash_ctrl, ahb_flash_ctrl,
                      ahb_read, SPI_FLASH_AHB_READ_V_DISABLE);
#else
    HWP_SPI_FLASH_T *hwp = (HWP_SPI_FLASH_T *)d->flash.hwp;
    REG_SPI_FLASH_SPI_CS_SIZE_T spi_cs_size;
    REG_FIELD_CHANGE1(hwp->spi_cs_size, spi_cs_size,
                      ahb_read_disable, 1);
#endif
}

/**
 * Enable AHB read of flash controller
 */
OSI_FORCE_INLINE static void prvEnableAhbRead(drvSpiFlash_t *d)
{
#if defined(CONFIG_SOC_8910) || defined(CONFIG_SOC_8850)
    // There are "hidden" instruction prefetch and data preload
    // during flash erase/program. If it happens, the data
    // is 0, and invalidate I-Cache D-Cache will remove it.
    // And the performance impact is acceptable.
#if defined(CONFIG_SOC_8850)
    halSpiFlashMedClear(&d->flash);
#endif
    L1C_CleanInvalidateDCacheAll();
    L1C_InvalidateICacheAll();
#elif defined(CONFIG_SOC_8811) || defined(CONFIG_SOC_8821)
    HWP_SPI_FLASH_T *hwp = (HWP_SPI_FLASH_T *)d->flash.hwp;
    REG_SPI_FLASH_AHB_FLASH_CTRL_T ahb_flash_ctrl;
    REG_FIELD_CHANGE1(hwp->ahb_flash_ctrl, ahb_flash_ctrl,
                      ahb_read, SPI_FLASH_AHB_READ_V_ENABLE);
#else
    HWP_SPI_FLASH_T *hwp = (HWP_SPI_FLASH_T *)d->flash.hwp;
    REG_SPI_FLASH_SPI_CS_SIZE_T spi_cs_size;
    REG_FIELD_CHANGE1(hwp->spi_cs_size, spi_cs_size,
                      ahb_read_disable, 0);
#endif
}

/**
 * Page program with XIP
 */
FLASHRAM_CODE static void prvPageProgram(
    drvSpiFlash_t *d, uint32_t offset,
    const uint8_t *data, size_t size)
{
#ifdef CONFIG_SOC_8821
    OSI_POLL_WAIT(NL1cSysNtnCheckDlTmAdvState() == false);
#endif

    uint32_t critical = osiEnterCritical();

    osiProfileEnter(PROFCODE_FLASH_PROGRAM);
    prvDisableAhbRead(d);

    halSpiFlashPrepareEraseProgram(&d->flash, offset, size);
    halSpiFlashPageProgram(&d->flash, offset, data, size);
    osiDelayUS(MIN_ERASE_PROGRAM_TIME); // Wait a while

    for (;;)
    {
        if (halSpiFlashIsWipFinished(&d->flash))
        {
            halSpiFlashFinishEraseProgram(&d->flash);
            prvEnableAhbRead(d);

            osiProfileExit(PROFCODE_FLASH_PROGRAM);
            osiExitCritical(critical);
            return;
#if defined(CONFIG_MC661_CN_19_70_JIEKE)
        } else {
            if ((d->name == DRV_NAME_SPI_FLASH_EXT) && (d->flash.suspend_en == false)) {
                prvEnableAhbRead(d);
                osiExitCritical(critical);
                osiDelayUS(5); // avoid CPU can't take interrupt
                critical = osiEnterCritical();
                prvDisableAhbRead(d);
            }
#endif
        }

        if (d->flash.suspend_en && !d->suspend_disable && osiIrqPending() && !osiIsPanic())
        {
            halSpiFlashProgramSuspend(&d->flash);

            osiDelayUS(DELAY_AFTER_SUSPEND); // Wait a while
            halSpiFlashWaitWipFinish(&d->flash);
            prvEnableAhbRead(d);
            osiExitCritical(critical);

            osiDelayUS(5); // avoid CPU can't take interrupt
#ifdef CONFIG_SOC_8821
            OSI_POLL_WAIT(NL1cSysNtnCheckDlTmAdvState() == false);
#endif
            critical = osiEnterCritical();

            prvDisableAhbRead(d);
            halSpiFlashProgramResume(&d->flash);
            osiDelayUS(MIN_ERASE_PROGRAM_RESUME_TIME); // Wait a while
        }
    }
}

/**
 * Erase with XIP
 */
FLASHRAM_CODE static void prvErase(drvSpiFlash_t *d, uint32_t offset, size_t size)
{
#ifdef CONFIG_SOC_8821
    OSI_POLL_WAIT(NL1cSysNtnCheckDlTmAdvState() == false);
#endif
    uint32_t critical = osiEnterCritical();

    osiProfileEnter(PROFCODE_FLASH_ERASE);
    prvDisableAhbRead(d);

    halSpiFlashPrepareEraseProgram(&d->flash, offset, size);
    halSpiFlashErase(&d->flash, offset, size);
    osiDelayUS(MIN_ERASE_PROGRAM_TIME); // Wait a while

    for (;;)
    {
        if (halSpiFlashIsWipFinished(&d->flash))
        {
            halSpiFlashFinishEraseProgram(&d->flash);
            prvEnableAhbRead(d);

            osiProfileExit(PROFCODE_FLASH_ERASE);
            osiExitCritical(critical);
            return;
#if defined(CONFIG_MC661_CN_19_70_JIEKE)
        } else {
            if ((d->name == DRV_NAME_SPI_FLASH_EXT) && (d->flash.suspend_en == false)) {
                prvEnableAhbRead(d);
                osiExitCritical(critical);
                osiDelayUS(5); // avoid CPU can't take interrupt
                critical = osiEnterCritical();
                prvDisableAhbRead(d);
            }
#endif
        }

        if (d->flash.suspend_en && !d->suspend_disable && osiIrqPending() && !osiIsPanic())
        {
            halSpiFlashEraseSuspend(&d->flash);

            osiDelayUS(DELAY_AFTER_SUSPEND); // Wait a while
            halSpiFlashWaitWipFinish(&d->flash);
            prvEnableAhbRead(d);
            osiExitCritical(critical);

            osiDelayUS(5); // avoid CPU can't take interrupt
#ifdef CONFIG_SOC_8821
            OSI_POLL_WAIT(NL1cSysNtnCheckDlTmAdvState() == false);
#endif
            critical = osiEnterCritical();

            prvDisableAhbRead(d);
            halSpiFlashEraseResume(&d->flash);
            osiDelayUS(MIN_ERASE_PROGRAM_RESUME_TIME); // Wait a while
        }
    }
}

/**
 * Page program without XIP
 */
FLASHRAM_CODE static void prvEraseNoXipLocked(drvSpiFlash_t *d, uint32_t offset, size_t size)
{
    prvDisableAhbRead(d);
    halSpiFlashPrepareEraseProgram(&d->flash, offset, size);
    halSpiFlashErase(&d->flash, offset, size);
    halSpiFlashWaitWipFinish(&d->flash);
    halSpiFlashFinishEraseProgram(&d->flash);
    prvEnableAhbRead(d);
}

/**
 * Erase without XIP
 */
FLASHRAM_CODE static void prvPageProgramNoXipLocked(
    drvSpiFlash_t *d, uint32_t offset,
    const uint8_t *data, size_t size)
{
    prvDisableAhbRead(d);
    halSpiFlashPrepareEraseProgram(&d->flash, offset, size);
    halSpiFlashPageProgram(&d->flash, offset, data, size);
    halSpiFlashWaitWipFinish(&d->flash);
    halSpiFlashFinishEraseProgram(&d->flash);
    prvEnableAhbRead(d);
}
#endif

FLASHRAM_CODE void drvSpiFlashSRCheck(drvSpiFlash_t *d)
{
#ifdef CONFIG_TFM_SUPPORT
        tfmPlatformFlashSRCheck(d->name);
#else
    do
    {
        osiDelayUS(200 * 1000);
        if (halSpiFlashIsWipFinished(&d->flash))
        {
            prvEnableAhbRead(d);
            osiDelayUS(20 * 1000);
           break;
        }
    } while (1);
#endif
    //Direct restart required, the code should be executed in SRAM or PSRAM
    // TODO
}

/**
 * Open flash driver
 */
FLASHRAM_API drvSpiFlash_t *drvSpiFlashOpen(uint32_t name)
{
    drvSpiFlash_t *d = prvFlashGetByName(name);
    if (d == NULL)
        return NULL;

    uint32_t critical = osiEnterCritical();

    if (!d->opened)
    {
#ifdef CONFIG_TFM_SUPPORT
        if (tfmPlatformFlashOpen(name))
        {
            d->opened = true;
            d->lock = osiMutexCreate();
        }
        else
        {
            d = NULL;
        }
#else
        #ifdef CONFIG_FIBOCOM_BASE
        if (name == DRV_NAME_SPI_FLASH_EXT)
        {
            extern uint32_t halExtSpiFlashReadID(halSpiFlash_t * d);
            uint32_t mid =halExtSpiFlashReadID(&d->flash);
            OSI_LOGI(0, "ExtFlash id 0x%x",mid);
            if (mid == 0)
            {
                osiExitCritical(critical);
                return NULL;
            }
            else
            {
                extern int fibo_judge_flashID(unsigned mid);
                int found = fibo_judge_flashID(mid);
                if(found < 0)
                {
                    osiExitCritical(critical);
                    return NULL;
                }
            }
        }
        if (name == DRV_NAME_SPI_FLASH_EXT) {
            if (d->pm_source != NULL) {
                osiPmSourceDelete(d->pm_source);
                d->pm_source = NULL;
            }
            
            d->pm_source = osiPmSourceCreate(name, &_spiFlashExtPmOps, d);
            if (d->pm_source == NULL) {
                return NULL;
            }
        }
        #endif

        halSpiFlashInit(&d->flash);
        d->opened = true;

        #ifdef CONFIG_FIBOCOM_BASE
        if ((d->name == DRV_NAME_SPI_FLASH_EXT) && (d->flash.suspend_en == false)) {
            // for flash which is un-supported suspend, to response to system interrupt, 
            // although it can not run in XiP mode.
            d->xip_protect = false;
        }
        #endif
        d->lock = osiMutexCreate();

        OSI_LOGI(0x10007e52, "open flash %4c 0x%x", d->name, d->flash.mid);
#endif
    }

    osiExitCritical(critical);
    return d;
}

#if defined(CONFIG_SOC_8910) && defined(CONFIG_SUPPORT_LC_FLASH)
void drvSpiFlash4ByteAddressMode(drvSpiFlash_t *d, uint32_t offset)
{
    if (offset < size_16M)
        return;

    uint32_t critical = osiEnterCritical();
    halSpiFlash4ByteAddressMode(&d->flash);
    REG_SPI_FLASH_SPI_256_512_FLASH_CONFIG_T flash_config = {
        .b.four_byte_addr = 1,
    };
    hwp_spiFlashExt->spi_256_512_flash_config = flash_config.v;

    if (offset >= size_32M)
    {
        REG_SPI_FLASH_SPI_CS_SIZE_T spi_cs_size1 = {
            .b.sel1_flash_1 = 1,
        };

        hwp_spiFlashExt->spi_cs_size = spi_cs_size1.v;
    }
    else if (offset >= size_16M)
    {
        REG_SPI_FLASH_SPI_CS_SIZE_T spi_cs_size1 = {
            .b.sel_flash_1 = 1,
        };

        hwp_spiFlashExt->spi_cs_size = spi_cs_size1.v;
    }

    osiExitCritical(critical);
    return;
}

void drvSpiFlashDisenable4ByteAddressMode(drvSpiFlash_t *d, uint32_t offset)
{
    if (offset < size_16M)
        return;

    uint32_t critical = osiEnterCritical();
    halSpiFlashDisenable4ByteAddressMode(&d->flash);
    REG_SPI_FLASH_SPI_256_512_FLASH_CONFIG_T flash_config = {
        .b.four_byte_addr = 0,
    };
    hwp_spiFlashExt->spi_256_512_flash_config = flash_config.v;

    if (offset >= size_32M)
    {
        REG_SPI_FLASH_SPI_CS_SIZE_T spi_cs_size1 = {
            .b.sel1_flash_1 = 0,
        };
        hwp_spiFlashExt->spi_cs_size = spi_cs_size1.v;
    }
    else if (offset >= size_16M)
    {
        REG_SPI_FLASH_SPI_CS_SIZE_T spi_cs_size1 = {
            .b.sel_flash_1 = 0,
        };
        hwp_spiFlashExt->spi_cs_size = spi_cs_size1.v;
    }

    osiExitCritical(critical);
    return;
}
#endif
/**
 * Set whether XIP protection is needed
 */
bool drvSpiFlashSetXipEnabled(drvSpiFlash_t *d, bool enable)
{
#ifdef CONFIG_TFM_SUPPORT
        bool ret;
        ret = tfmPlatformFlashSetXipEnabled(d->name, enable);
        if (ret)
        {
            d->xip_protect = enable;
        }
        return ret;
#else
    if (d == NULL || d->name == DRV_NAME_SPI_FLASH)
        return false;

    d->xip_protect = enable;
    return true;
#endif
}

/**
 * Set whether to enable suspend
 */
bool drvSpiFlashSetSuspendEnabled(drvSpiFlash_t *d, bool enable)
{
#ifdef CONFIG_TFM_SUPPORT
        return tfmPlatformFlashSetSuspendEnabled(d->name, enable);
#else
    if (d == NULL)
        return false;

    d->suspend_disable = !enable;
    return true;
#endif
}

/**
 * Set range of soft write protection
 */
void drvSpiFlashSetRangeWriteProhibit(drvSpiFlash_t *d, uint32_t start, uint32_t end)
{
#ifdef CONFIG_TFM_SUPPORT
        tfmPlatformFlashSetRangeWriteProhibit(d->name, start, end);
#else
    if (start > d->flash.capacity || end > d->flash.capacity)
        return;

    unsigned block_start = OSI_ALIGN_UP(start, SIZE_64K) / SIZE_64K;
    unsigned block_end = OSI_ALIGN_DOWN(end, SIZE_64K) / SIZE_64K;
    for (unsigned block = block_start; block < block_end; block++)
        osiBitmapSet(d->block_prohibit, block);
#endif
}

/**
 * Clear range of soft write protection
 */
void drvSpiFlashClearRangeWriteProhibit(drvSpiFlash_t *d, uint32_t start, uint32_t end)
{
#ifdef CONFIG_TFM_SUPPORT
        tfmPlatformFlashClearRangeWriteProhibit(d->name, start, end);
#else
    if (start > d->flash.capacity || end > d->flash.capacity)
        return;

    unsigned block_start = OSI_ALIGN_UP(start, SIZE_64K) / SIZE_64K;
    unsigned block_end = OSI_ALIGN_DOWN(end, SIZE_64K) / SIZE_64K;
    for (unsigned block = block_start; block < block_end; block++)
        osiBitmapClear(d->block_prohibit, block);
#endif
}

/**
 * Get flash manufacture id
 */
uint32_t drvSpiFlashGetID(drvSpiFlash_t *d)
{
#ifdef CONFIG_TFM_SUPPORT
    return tfmPlatformFlashGetID(d->name);
#else
    return d->flash.mid;
#endif
}

/**
 * Get flash capacity in bytes
 */
uint32_t drvSpiFlashCapacity(drvSpiFlash_t *d)
{
#ifdef CONFIG_TFM_SUPPORT
    return tfmPlatformFlashCapacity(d->name);
#else
    return d->flash.capacity;
#endif
}

/**
 * Map flash address to CPU accessible address
 */
const void *drvSpiFlashMapAddress(drvSpiFlash_t *d, uint32_t offset)
{
#ifdef CONFIG_TFM_SUPPORT
    uint32_t ret = (uint32_t)tfmPlatformFlashMapAddress(d->name, offset);
    if (ret == 0)
        return NULL;
    else
        return FLASH_ADDRESS_S_TO_NS(ret);
#else
    if (offset >= d->flash.capacity)
        return NULL;
    return FLASH_MAP_ADDRESS(d, offset);
#endif
}

/**
 * Read data from flash
 */
bool drvSpiFlashRead(drvSpiFlash_t *d, uint32_t offset, void *data, uint32_t size)
{
#if defined(CONFIG_SOC_8910) && defined(CONFIG_SUPPORT_LC_FLASH)
    drvSpiFlash4ByteAddressMode(d, offset);
#endif
#ifdef CONFIG_TFM_SUPPORT
    return tfmPlatformFlashRead(d->name, offset, data, size);
#else
    if (size == 0)
        return true;
    if (d == NULL || data == NULL || offset + size > d->flash.capacity)
        return false;

    const void *fl = FLASH_MAP_ADDRESS(d, offset);
    memcpy(data, fl, size);
#if defined(CONFIG_SOC_8910) && defined(CONFIG_SUPPORT_LC_FLASH)
    drvSpiFlashDisenable4ByteAddressMode(d, offset);
#endif
    return true;
#endif
}

/**
 * Read data from flash, and check with provided data
 */
bool drvSpiFlashReadCheck(drvSpiFlash_t *d, uint32_t offset, const void *data, uint32_t size)
{
#ifdef CONFIG_TFM_SUPPORT
    return tfmPlatformFlashReadCheck(d->name, offset, data, size);
#else
    if (size == 0)
        return true;
    if (d == NULL || data == NULL || offset + size > d->flash.capacity)
        return false;
#if defined(CONFIG_SOC_8910) && defined(CONFIG_SUPPORT_LC_FLASH)
    drvSpiFlash4ByteAddressMode(d, offset);
#endif
    const void *fl = FLASH_MAP_ADDRESS(d, offset);
    bool flag = false;
    if (memcmp(data, fl, size) == 0)
        flag = true;
#if defined(CONFIG_SOC_8910) && defined(CONFIG_SUPPORT_LC_FLASH)
    drvSpiFlashDisenable4ByteAddressMode(d, offset);
#endif
    return flag;
#endif
}

/**
 * Write to flash
 */
FLASHRAM_API bool drvSpiFlashWrite(drvSpiFlash_t *d, uint32_t offset, const void *data, size_t size)
{
#ifdef CONFIG_TFM_SUPPORT
    bool ret;
    osiMutexLock(d->lock); // exclusive write
    ret = tfmPlatformFlashWrite(d->name, offset, data, size);
    osiMutexUnlock(d->lock);
    return ret;
#else
    uint32_t o_offset = offset;
    size_t o_size = size;

    if (data == NULL || offset + size > d->flash.capacity)
        return false;

    if (prvIsRangeWriteProhibit(d, offset, size))
    {
        OSI_LOGI(0x10007e53, "flash write protected range %4c offset/0x%x size/0x%x",
                 d->name, offset, size);
        return false;
    }

    osiMutexLock(d->lock); // exclusive write
#if defined(CONFIG_SOC_8910) && defined(CONFIG_SUPPORT_LC_FLASH)
    drvSpiFlash4ByteAddressMode(d, offset);
#endif
    if (d->xip_protect)
    {
        while (size > 0)
        {
            uint32_t next_page = OSI_ALIGN_DOWN(offset + PAGE_SIZE, PAGE_SIZE);
            uint32_t bsize = OSI_MIN(unsigned, next_page - offset, size);

            prvPageProgram(d, offset, data, bsize);
            data = (const char *)data + bsize;
            offset += bsize;
            size -= bsize;
        }
    }
    else
    {
        while (size > 0)
        {
            uint32_t next_page = OSI_ALIGN_DOWN(offset + PAGE_SIZE, PAGE_SIZE);
            uint32_t bsize = OSI_MIN(unsigned, next_page - offset, size);

            prvPageProgramNoXipLocked(d, offset, data, bsize);
            data = (const char *)data + bsize;
            offset += bsize;
            size -= bsize;
        }
    }
#if defined(CONFIG_SOC_8910) && defined(CONFIG_SUPPORT_LC_FLASH)
    drvSpiFlashDisenable4ByteAddressMode(d, o_offset);
#endif
    osiMutexUnlock(d->lock);
    osiDCacheInvalidate(FLASH_MAP_ADDRESS(d, o_offset), o_size);
    return true;
#endif
}

/**
 * Erase flash range
 */
FLASHRAM_API bool drvSpiFlashErase(drvSpiFlash_t *d, uint32_t offset, size_t size)
{
#ifdef CONFIG_TFM_SUPPORT
        bool ret;
        osiMutexLock(d->lock);
        ret = tfmPlatformFlashErase(d->name, offset, size);
        osiMutexUnlock(d->lock);
        return ret;
#else
    uint32_t o_offset = offset;
    size_t o_size = size;

    if (offset + size > d->flash.capacity ||
        !OSI_IS_ALIGNED(offset, SIZE_4K) ||
        !OSI_IS_ALIGNED(size, SIZE_4K))
        return false;

    if (prvIsRangeWriteProhibit(d, offset, size))
    {
        OSI_LOGW(0x10007e54, "flash erase protected range %4c offset/0x%x size/0x%x",
                 d->name, offset, size);
        return false;
    }

    osiMutexLock(d->lock);
#if defined(CONFIG_SOC_8910) && defined(CONFIG_SUPPORT_LC_FLASH)
    drvSpiFlash4ByteAddressMode(d, offset);
#endif
    if (d->xip_protect)
    {
        while (size > 0)
        {
            bool force_4k = false;
            if (d->flash.force_4k_first_block && offset < SIZE_64K)
                force_4k = true;
            if (d->flash.force_4k_last_block && offset >= d->flash.capacity - SIZE_64K)
                force_4k = true;

            if (!force_4k && OSI_IS_ALIGNED(offset, SIZE_64K) && size >= SIZE_64K)
            {
                prvErase(d, offset, SIZE_64K);
                offset += SIZE_64K;
                size -= SIZE_64K;
            }
            else if (!force_4k && OSI_IS_ALIGNED(offset, SIZE_32K) && size >= SIZE_32K)
            {
                prvErase(d, offset, SIZE_32K);
                offset += SIZE_32K;
                size -= SIZE_32K;
            }
            else
            {
                prvErase(d, offset, SIZE_4K);
                offset += SIZE_4K;
                size -= SIZE_4K;
            }
        }
    }
    else
    {
        while (size > 0)
        {
            bool force_4k = false;
            if (d->flash.force_4k_first_block && offset < SIZE_64K)
                force_4k = true;
            if (d->flash.force_4k_last_block && offset >= d->flash.capacity - SIZE_64K)
                force_4k = true;

            if (!force_4k && OSI_IS_ALIGNED(offset, SIZE_64K) && size >= SIZE_64K)
            {
                prvEraseNoXipLocked(d, offset, SIZE_64K);
                offset += SIZE_64K;
                size -= SIZE_64K;
            }
            else if (!force_4k && OSI_IS_ALIGNED(offset, SIZE_32K) && size >= SIZE_32K)
            {
                prvEraseNoXipLocked(d, offset, SIZE_32K);
                offset += SIZE_32K;
                size -= SIZE_32K;
            }
            else
            {
                prvEraseNoXipLocked(d, offset, SIZE_4K);
                offset += SIZE_4K;
                size -= SIZE_4K;
            }
        }
    }
#if defined(CONFIG_SOC_8910) && defined(CONFIG_SUPPORT_LC_FLASH)
    drvSpiFlashDisenable4ByteAddressMode(d, o_offset);
#endif
    osiMutexUnlock(d->lock);
    osiDCacheInvalidate(FLASH_MAP_ADDRESS(d, o_offset), o_size);
    return true;
#endif
}

/**
 * Chip erase
 */
FLASHRAM_API bool drvSpiFlashChipErase(drvSpiFlash_t *d)
{
#ifdef CONFIG_TFM_SUPPORT
        bool ret;
        uint32_t critical = prvProtectAquire(d);
        ret = tfmPlatformFlashChipErase(d->name);
        prvProtectRelease(d, critical);
        return ret;
#else
    if (d == NULL || prvIsRangeWriteProhibit(d, 0, d->flash.capacity))
        return false;

    uint32_t critical = prvProtectAquire(d);

    halSpiFlashPrepareEraseProgram(&d->flash, 0, d->flash.capacity);
    halSpiFlashChipErase(&d->flash);
    halSpiFlashWaitWipFinish(&d->flash);
    halSpiFlashFinishEraseProgram(&d->flash);

    prvProtectRelease(d, critical);
    return true;
#endif
}

/**
 * Read flash status register
 */
FLASHRAM_API uint16_t drvSpiFlashReadSR(drvSpiFlash_t *d)
{
    if (d == NULL)
        return 0;

    uint16_t sr;
    uint32_t critical = prvProtectAquire(d);
#ifdef CONFIG_TFM_SUPPORT
    sr = tfmPlatformFlashReadSR(d->name);
#else
    sr = halSpiFlashReadSR(&d->flash);
#endif
    prvProtectRelease(d, critical);
    return sr;
}

/**
 * write flash status register
 */
FLASHRAM_API void drvSpiFlashWriteSR(drvSpiFlash_t *d, uint16_t sr)
{
    if (d == NULL)
        return;

    uint32_t critical = prvProtectAquire(d);
#ifdef CONFIG_TFM_SUPPORT
    tfmPlatformFlashWriteSR(d->name, sr);
#else
    halSpiFlashWriteSR(&d->flash, sr);
#endif
    prvProtectRelease(d, critical);
}

/**
 * Enter deep power down mode
 */
FLASHRAM_API void drvSpiFlashDeepPowerDown(drvSpiFlash_t *d)
{
    uint32_t critical = prvProtectAquire(d);
#ifdef CONFIG_TFM_SUPPORT
    tfmPlatformFlashDeepPowerDown(d->name);
#else
    halSpiFlashDeepPowerDown(&d->flash);
#endif
    prvProtectRelease(d, critical);
}


/**
 * Release deep power down mode
 */
FLASHRAM_API void drvSpiFlashReleaseDeepPowerDown(drvSpiFlash_t *d)
{
    uint32_t critical = prvProtectAquire(d);
#ifdef CONFIG_TFM_SUPPORT
    tfmPlatformFlashReleaseDeepPowerDown(d->name);
#else
    halSpiFlashReleaseDeepPowerDown(&d->flash);
#endif
    prvProtectRelease(d, critical);
}


/**
 * Read unique ID
 */
FLASHRAM_API int drvSpiFlashReadUniqueId(drvSpiFlash_t *d, uint8_t *id)
{
    if (d == NULL || id == NULL)
        return -1;

    int size;
    uint32_t critical = prvProtectAquire(d);
#ifdef CONFIG_TFM_SUPPORT
    size = tfmPlatformFlashReadUniqueId(d->name, id);
#else
    size = halSpiFlashReadUniqueId(&d->flash, id);
#endif
    prvProtectRelease(d, critical);
    return size;
}


/**
 * Read CPID
 */
FLASHRAM_API uint16_t drvSpiFlashReadCpId(drvSpiFlash_t *d)
{
    if (d == NULL)
        return 0;

    uint16_t id;
    uint32_t critical = prvProtectAquire(d);
#ifdef CONFIG_TFM_SUPPORT
    id = tfmPlatformFlashReadCpId(d->name);
#else
    id = halSpiFlashReadCpId(&d->flash);
#endif
    prvProtectRelease(d, critical);
    return id;
}


/**
 * Read security registers
 */
FLASHRAM_API bool drvSpiFlashReadSecurityRegister(
    drvSpiFlash_t *d, uint8_t num, uint16_t address,
    void *data, uint32_t size)
{
    if (size == 0)
        return true;
    if (d == NULL || data == NULL)
        return false;
#ifdef CONFIG_TFM_SUPPORT
    uint32_t critical = prvProtectAquire(d);
    bool ret = tfmPlatformFlashReadSecurityRegister(d->name, num, address, data, size);
    prvProtectRelease(d, critical);
    return ret;
#else

    unsigned address_end = address + size;
    while (address < address_end)
    {
        uint32_t read_end = OSI_MIN(uint32_t, OSI_ALIGN_DOWN(address + 4, 4), address_end);
        uint32_t read_size = OSI_MIN(uint32_t, read_end - address, 4);

        uint32_t critical = prvProtectAquire(d);
        bool ok = halSpiFlashReadSecurityRegister(&d->flash, num, address, data, read_size);
        prvProtectRelease(d, critical);

        if (!ok)
            return false;

        address += read_size;
        data = (char *)data + read_size;
    }
    return true;
#endif
}

/**
 * Read security registers
 */
FLASHRAM_API bool drvSpiFlashProgramSecurityRegister(
    drvSpiFlash_t *d, uint8_t num, uint16_t address,
    const void *data, uint32_t size)
{
    if (size == 0)
        return true;
    if (d == NULL || data == NULL)
        return false;
#ifdef CONFIG_TFM_SUPPORT
    uint32_t critical = prvProtectAquire(d); // avoid read, and there are no suspend
    bool ret = tfmPlatformFlashProgramSecurityRegister(d->name, num, address, data, size);
    prvProtectRelease(d, critical);
    return ret;
#else

    unsigned address_end = address + size;
    while (address < address_end)
    {
        uint32_t block_end = OSI_MIN(uint32_t, OSI_ALIGN_DOWN(address + 128, 128), address_end);
        unsigned block_size = OSI_MIN(unsigned, block_end - address, 128);

        uint32_t critical = prvProtectAquire(d); // avoid read, and there are no suspend
        halSpiFlashWriteEnable(&d->flash);
        bool ok = halSpiFlashProgramSecurityRegister(&d->flash, num, address, data, block_size);
        halSpiFlashWaitWipFinish(&d->flash);
        prvProtectRelease(d, critical);

        if (!ok)
            return false;

        address += block_size;
        data = (char *)data + block_size;
    }

    return true;
#endif
}

/**
 * Erase security register
 */
FLASHRAM_API bool drvSpiFlashEraseSecurityRegister(drvSpiFlash_t *d, uint8_t num)
{
    if (d == NULL)
        return false;

#ifdef CONFIG_TFM_SUPPORT
    uint32_t critical = prvProtectAquire(d); // avoid read, and there are no suspend
    bool ok = tfmPlatformFlashEraseSecurityRegister(d->name, num);
    prvProtectRelease(d, critical);
    return ok;
#else
    uint32_t critical = prvProtectAquire(d); // avoid read, and there are no suspend
    halSpiFlashWriteEnable(&d->flash);
    bool ok = halSpiFlashEraseSecurityRegister(&d->flash, num);
    halSpiFlashWaitWipFinish(&d->flash);
    prvProtectRelease(d, critical);
    return ok;
#endif
}


/**
 * Lock security register
 */
FLASHRAM_API bool drvSpiFlashLockSecurityRegister(drvSpiFlash_t *d, uint8_t num)
{
    if (d == NULL)
        return false;

    bool ok;
    uint32_t critical = prvProtectAquire(d);
#ifdef CONFIG_TFM_SUPPORT
    ok = tfmPlatformFlashLockSecurityRegister(d->name, num);
#else
    ok = halSpiFlashLockSecurityRegister(&d->flash, num);
#endif
    prvProtectRelease(d, critical);
    return ok;
}


/**
 * Is security register locked
 */
FLASHRAM_API bool drvSpiFlashIsSecurityRegisterLocked(drvSpiFlash_t *d, uint8_t num)
{
    if (d == NULL)
        return false;

    bool ok;
    uint32_t critical = prvProtectAquire(d);
#ifdef CONFIG_TFM_SUPPORT
    ok = tfmPlatformFlashIsSecurityRegisterLocked(d->name, num);
#else
    ok = halSpiFlashIsSecurityRegisterLocked(&d->flash, num);
#endif
    prvProtectRelease(d, critical);
    return ok;
}

/**
 * Read SFDP
 */
FLASHRAM_API bool drvSpiFlashReadSFDP(drvSpiFlash_t *d, unsigned address, void *data, unsigned size)
{
    if (size == 0)
        return true;
    if (d == NULL || data == NULL)
        return false;

    bool ok;
    uint32_t critical = prvProtectAquire(d);
#ifdef CONFIG_TFM_SUPPORT
    ok = tfmPlatformFlashReadSFDP(d->name, address, data, size);
#else
    ok = halSpiFlashReadSFDP(&d->flash, address, data, size);
#endif
    prvProtectRelease(d, critical);
    return ok;
}


/**
 * Get flash properties
 */
halSpiFlash_t *drvSpiFlashGetProp(drvSpiFlash_t *d)
{
#ifdef CONFIG_TFM_SUPPORT
    tfmPlatformFlashGetProp(d->name, (void *)(&d->flash));
#endif
    return &d->flash;
}


/**
 * Unique id length
 */
int drvSpiFlashUidLength(drvSpiFlash_t *d)
{
    if (d == NULL)
        return 0;

    switch (d->flash.uid_type)
    {
    case HAL_SPI_FLASH_UID_4BH_8:
        return 8;

    case HAL_SPI_FLASH_UID_SFDP_80H_12:
        return 12;

    case HAL_SPI_FLASH_UID_4BH_16:
    case HAL_SPI_FLASH_UID_SFDP_94H_16:
    case HAL_SPI_FLASH_UID_SFDP_194H_16:
        return 16;

    default:
        return 0;
    }
}
#ifdef CONFIG_FIBOCOM_BASE
void ext_flash_mode_set(uint8_t mode)
{
   if(mode == 1)
       spiflash_is_4line = 1;
   else if(mode == 0)
       spiflash_is_4line = 0;
}
#endif
