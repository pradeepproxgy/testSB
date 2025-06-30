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
#include "drv_spi.h"
#include "drv_ifc.h"
#include "drv_names.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "osi_log.h"
#include "osi_api.h"
#include "hal_chip.h"
#include "hal_iomux.h"
#include "osi_compiler.h"
#include "drv_config.h"

drvSpiMaster_t *pGeneralSpiMaster = NULL;
#define SPI_SECTOR_SIZE 4096
#define SPI_SECTORS_PER_BLOCK 8
#define SPI_SECTOR_NUM 4096

#define SPI_BLOCK_NUM 512

#define SPI_PAGE_SIZE 256
#define SPI_PAGE_SHIFT 8
#define SPI_PAGE_NUM_PER_SECTOR (SPI_SECTOR_SIZE >> SPI_PAGE_SHIFT)

#define cmd_JedecDeviceID 0x9f
#define cmd_SectorEarse 0x20
#define cmd_ReadData 0x03
#define cmd_WriteEnable 0x06
#define cmd_PageProgram 0x02
#define cmd_ReadStatusReg 0x05
#define cmd_BlockErase_32k 0x52
#define cmd_BlockErase_64k 0xd8
#ifdef CONFIG_GENERAL_SPI_LC_FLASH
#define cmd_4Byte_SectorEarse 0x21
#define cmd_4Byte_ReadData 0x13
#define cmd_4Byte_PageProgram 0x12
#define cmd_4Byte_BlockErase_64k 0xdc
#endif

#define SIZE_4K (4 * 1024)
#define SIZE_8K (8 * 1024)
#define SIZE_16K (16 * 1024)
#define SIZE_32K (32 * 1024)
#define SIZE_64K (64 * 1024)
#define PAGE_SIZE (256)
#ifdef CONFIG_GENERAL_SPI_LC_FLASH
#define SIZE_16M (SIZE_16K * 1024)
#endif

static uint8_t OSI_ALIGNED(32) dummpdata[4096] = {0};

#define CONFIG_GER_SPI_PHY_ADDRESS 0x70000000
#define WAIT_FINISH_TIME 128000000 //US

struct drvGeneralSpiFlash
{
    bool opened;
    bool xip_protect;
    unsigned name;
    unsigned capacity;
    uintptr_t base_address;
    osiMutex_t *lock;
    uint32_t block_prohibit[8];
};

static drvGeneralSpiFlash_t gDrvGeneralSpiFlashCtx[] = {
    {
        .opened = false,
        .xip_protect = false,
        .name = DRV_NAME_SPI1,
        .capacity = FLASH_MOUNT_SIZE,
        .base_address = CONFIG_GER_SPI_PHY_ADDRESS,
    }};

drvGeneralSpiFlash_t *prvFlashGetByName(uint32_t name)
{
    for (unsigned n = 0; n < OSI_ARRAY_SIZE(gDrvGeneralSpiFlashCtx); n++)
    {
        if (gDrvGeneralSpiFlashCtx[n].name == name)
            return &gDrvGeneralSpiFlashCtx[n];
    }
    return NULL;
}

#ifdef CONFIG_SOC_8910
void drvGeneralSpiFlashInit(uint32_t name)
{
    uint8_t OSI_ALIGNED(16) cmd = 0x9F;
    uint8_t OSI_ALIGNED(16) sDataBuf[3] = {0};
    uint8_t OSI_ALIGNED(16) flashIDBuf[3] = {0};

#if 1 //SPI1 ;
    hwp_iomux->pad_gpio_9_cfg_reg = 0x01;
    hwp_iomux->pad_gpio_10_cfg_reg = 0x01;
    hwp_iomux->pad_gpio_11_cfg_reg = 0x01;
    hwp_iomux->pad_gpio_12_cfg_reg = 0x01;
#else //SPI2
    hwp_iomux->pad_gpio_0_cfg_reg = 0x02;
    hwp_iomux->pad_gpio_1_cfg_reg = 0x02;
    hwp_iomux->pad_gpio_2_cfg_reg = 0x02;
    hwp_iomux->pad_gpio_3_cfg_reg = 0x02;
#endif
    drvSpiConfig_t cfg = {
        0,
    };
    cfg.name = name;
    cfg.inputEn = true;
    cfg.baud = 30000000;
    cfg.cpol = SPI_CPOL_LOW;
    cfg.cpha = SPI_CPHA_1Edge;
    cfg.input_sel = SPI_DI_1;
    cfg.transmode = SPI_DIRECT_POLLING;
    cfg.cs_polarity0 = SPI_CS_ACTIVE_LOW;
    cfg.framesize = 8;
    pGeneralSpiMaster = drvSpiMasterAcquire(cfg);
    drvSpiPinControl(pGeneralSpiMaster, SPI_CS0_CTRL, FORCE_0_CTRL);
    drvSpiWrite(pGeneralSpiMaster, SPI_I2C_CS0, &cmd, 1);
    if (!drvSpiRead(pGeneralSpiMaster, SPI_I2C_CS0, sDataBuf, flashIDBuf, 3))
    {
        OSI_LOGI(0x10009fcb, "SPI drvSpiRead ERROR line %d", __LINE__);
        return;
    }
    drvSpiPinControl(pGeneralSpiMaster, SPI_CS0_CTRL, SPI_CTRL);
    OSI_LOGI(0x1000aa25, "General SPI read FlashID: %02x,%02x,%02x\r\n", flashIDBuf[0], flashIDBuf[1], flashIDBuf[2]);
}
#elif defined(CONFIG_SOC_8850)
void drvGeneralSpiFlashInit(uint32_t name)
{
    uint8_t OSI_ALIGNED(16) cmd[3] = {0x9F, 0, 0};

    REG_IOMUX_SDMMC1_DATA_6_T spi_1_di_1 = {0};
    spi_1_di_1.b.func_sel = 4;
    hwp_iomux->sdmmc1_data_7 = spi_1_di_1.v; //di_1
    hwp_iomux->sdmmc1_data_6 = spi_1_di_1.v; //di_1
    hwp_iomux->sdmmc1_data_5 = spi_1_di_1.v; //dio_0
    hwp_iomux->sdmmc1_data_4 = spi_1_di_1.v; //cs0
    hwp_iomux->sdmmc1_data_3 = spi_1_di_1.v; //clk

    halPmuSetPowerLevel(HAL_POWER_SD, 1800);
    halPmuSwitchPower(HAL_POWER_SD, true, true);

    drvSpiConfig_t cfg = {
        .baud = 1000000,
        .clk_source = CLK_SOURCE_XTAL26,
        .cpha = SPI_CPHA_1Edge,
        .cpol = SPI_CPOL_LOW,
        .framesize = 8,
        .name = DRV_NAME_SPI1,
        .lsb = false,
        .work_mode = SPI_WORK_STANDARD,
        .transmode = SPI_DIRECT_POLLING,
        .inputEn = true,
        .input_sel = SPI_DI_1,
        .cs_polarity0 = SPI_CS_ACTIVE_LOW,
    };
    pGeneralSpiMaster = drvSpiMasterAcquire(cfg);

    osiThreadSleep(1000);
    drvSpiRegsConfig(pGeneralSpiMaster);

    OSI_LOGXI(OSI_LOGPAR_SI, 0x10009fb4, "[spi test] - in %s,line - %d", __func__, __LINE__);
    drvSpiCsConfig(pGeneralSpiMaster, SPI_I2C_CS0, true);
    drvSpiWrite(pGeneralSpiMaster, cmd, 1);
    cmd[0] = 0x0;
    if (!drvSpiRead(pGeneralSpiMaster, cmd, 3))
    {
        OSI_LOGI(0x10009fcb, "SPI drvSpiRead ERROR line %d", __LINE__);
        return;
    }
    drvSpiCsConfig(pGeneralSpiMaster, SPI_I2C_CS0, false);
    OSI_LOGI(0x1000aa25, "General SPI read FlashID: %02x,%02x,%02x\r\n", cmd[0], cmd[1], cmd[2]);
}
#endif

#ifdef CONFIG_GENERAL_SPI_LC_FLASH
/**
 * \brief enter 4-byte address mode
 */
void drvGeneralSpiFlash4ByteAddressMode(drvGeneralSpiFlash_t *d)
{
    if (d->capacity < SIZE_16M)
        return;

    OSI_LOGI(0x1000aa26, "SPI: enable 4Byte address");

    char OSI_ALIGNED(16) data[1];
    char OSI_ALIGNED(16) readData[2] = {0};
    uint32_t startTick = 0, checkTick = 0;
    startTick = osiUpTime();
#ifdef CONFIG_SOC_8910
    char OSI_ALIGNED(16) sDataBuf[1] = {0};
#endif
    char sr3_need = 0x1; //ads = 1
    do
    {
        data[0] = 0xb7;
#ifdef CONFIG_SOC_8910
        drvSpiPinControl(pGeneralSpiMaster, SPI_CS0_CTRL, FORCE_0_CTRL);
        drvSpiWrite(pGeneralSpiMaster, SPI_I2C_CS0, data, 1);
        drvSpiPinControl(pGeneralSpiMaster, SPI_CS0_CTRL, SPI_CTRL);
#elif defined(CONFIG_SOC_8850)
        drvSpiCsConfig(pGeneralSpiMaster, SPI_I2C_CS0, true);
        drvSpiWrite(pGeneralSpiMaster, &data, 1);
        drvSpiCsConfig(pGeneralSpiMaster, SPI_I2C_CS0, false);
#endif
        data[0] = 0x15;
#ifdef CONFIG_SOC_8910
        drvSpiPinControl(pGeneralSpiMaster, SPI_CS0_CTRL, FORCE_0_CTRL);
        drvSpiWrite(pGeneralSpiMaster, SPI_I2C_CS0, data, 1);
        drvSpiRead(pGeneralSpiMaster, SPI_I2C_CS0, sDataBuf, readData, 1);
        drvSpiPinControl(pGeneralSpiMaster, SPI_CS0_CTRL, SPI_CTRL);
#elif defined(CONFIG_SOC_8850)
        drvSpiCsConfig(pGeneralSpiMaster, SPI_I2C_CS0, true);
        drvSpiWrite(pGeneralSpiMaster, &data, 1);
        drvSpiRead(pGeneralSpiMaster, &readData, 1);
        drvSpiCsConfig(pGeneralSpiMaster, SPI_I2C_CS0, false);
#endif
        checkTick = osiUpTime();
        if ((checkTick - startTick) > 6000)
        {
            OSI_LOGI(0x1000aa27, "\r\n****Error:prv4ByteAddressModeEn Timeout*****\r\n");
            break;
        }

    } while ((readData[0] & (0x1)) == sr3_need);
}
#endif

drvGeneralSpiFlash_t *drvGeneralSpiFlashOpen(uint32_t name)
{
    drvGeneralSpiFlash_t *d = prvFlashGetByName(name);
    if (d == NULL)
        return NULL;

    uint32_t critical = osiEnterCritical();
    if (!d->opened)
    {
        drvGeneralSpiFlashInit(d->name);
        d->opened = true;
#ifdef CONFIG_GENERAL_SPI_LC_FLASH
        drvGeneralSpiFlash4ByteAddressMode(d);
#endif
        d->lock = osiMutexCreate();
    }
    osiExitCritical(critical);
    return d;
}

void drvGeneralSpiFlashWriteEnable(void)
{
    uint8_t OSI_ALIGNED(16) cmd = cmd_WriteEnable;
#ifdef CONFIG_SOC_8910
    drvSpiPinControl(pGeneralSpiMaster, SPI_CS0_CTRL, FORCE_0_CTRL);
    drvSpiWrite(pGeneralSpiMaster, SPI_I2C_CS0, &cmd, 1);
    drvSpiPinControl(pGeneralSpiMaster, SPI_CS0_CTRL, SPI_CTRL);
#elif defined(CONFIG_SOC_8850)
    drvSpiCsConfig(pGeneralSpiMaster, SPI_I2C_CS0, true);
    drvSpiWrite(pGeneralSpiMaster, &cmd, 1);
    drvSpiCsConfig(pGeneralSpiMaster, SPI_I2C_CS0, false);
#endif
}

int drvGeneralSpiFlashFinish(void)
{
    uint8_t OSI_ALIGNED(16) cmd = cmd_ReadStatusReg;
    uint8_t OSI_ALIGNED(16) status;
#ifdef CONFIG_SOC_8910
    uint8_t OSI_ALIGNED(16) dummydata;
#endif
    uint64_t startTime = 0, endTime = 0, elapsedTime = 0;

    osiElapsedTimer_t elapsed;
    osiElapsedTimerStart(&elapsed);
    startTime = osiElapsedTimeUS(&elapsed);
    do
    {
#ifdef CONFIG_SOC_8910
        drvSpiPinControl(pGeneralSpiMaster, SPI_CS0_CTRL, FORCE_0_CTRL);
        drvSpiWrite(pGeneralSpiMaster, SPI_I2C_CS0, &cmd, 1);
        drvSpiRead(pGeneralSpiMaster, SPI_I2C_CS0, &dummydata, &status, 1);
        drvSpiPinControl(pGeneralSpiMaster, SPI_CS0_CTRL, SPI_CTRL);
#elif defined(CONFIG_SOC_8850)
        drvSpiCsConfig(pGeneralSpiMaster, SPI_I2C_CS0, true);
        drvSpiWrite(pGeneralSpiMaster, &cmd, 1);
        drvSpiRead(pGeneralSpiMaster, &status, 1);
        drvSpiCsConfig(pGeneralSpiMaster, SPI_I2C_CS0, false);
#endif
        endTime = osiElapsedTimeUS(&elapsed);
        elapsedTime = endTime - startTime;
        if (elapsedTime > WAIT_FINISH_TIME)
        {
            OSI_LOGXI(OSI_LOGPAR_D, 0x10009fcd, "drvGeneralSpiFlashFinish %lld ", elapsedTime);
            return false;
        }
        if ((status & 0x01) == 1)
            osiDelayUS(64);
    } while ((status & 0x01) == 1);
    return true;
}

static void prvPageProgram(
    drvGeneralSpiFlash_t *d, uint32_t offset,
    const uint8_t *data, size_t size)
{
}

static void prvPageProgramNoXipLocked(
    drvGeneralSpiFlash_t *d, uint32_t offset,
    const uint8_t *data, size_t size)
{
    uint8_t OSI_ALIGNED(16) cmd[5] = {0};
    uint8_t cmd_len = 4;

    //erase one block
    drvGeneralSpiFlashErase(d, offset, size);
    drvGeneralSpiFlashWriteEnable();
    if (size > PAGE_SIZE)
    {
        OSI_LOGI(0x1000aa28, "prvPageProgramNoXipLocked offset = 0x%x , size = 0x%x", offset, size);
        size = PAGE_SIZE;
    }
#ifdef CONFIG_GENERAL_SPI_LC_FLASH
    cmd[0] = cmd_4Byte_PageProgram;
    cmd[1] = (offset & 0xff000000) >> 24;
    cmd[2] = (offset & 0xff0000) >> 16;
    cmd[3] = (offset & 0xff00) >> 8;
    cmd[4] = offset & 0xff;

    cmd_len = 5;
#else
    cmd[0] = cmd_PageProgram;
    cmd[1] = (offset & 0xff0000) >> 16;
    cmd[2] = (offset & 0xff00) >> 8;
    cmd[3] = offset & 0xff;
#endif

#ifdef CONFIG_SOC_8910
    drvSpiPinControl(pGeneralSpiMaster, SPI_CS0_CTRL, FORCE_0_CTRL);
    drvSpiWrite(pGeneralSpiMaster, SPI_I2C_CS0, cmd, cmd_len);
    drvSpiWrite(pGeneralSpiMaster, SPI_I2C_CS0, data, size);
    drvSpiPinControl(pGeneralSpiMaster, SPI_CS0_CTRL, SPI_CTRL);
#elif defined(CONFIG_SOC_8850)
    drvSpiCsConfig(pGeneralSpiMaster, SPI_I2C_CS0, true);
    drvSpiWrite(pGeneralSpiMaster, cmd, cmd_len);
    drvSpiWrite(pGeneralSpiMaster, data, size);
    drvSpiCsConfig(pGeneralSpiMaster, SPI_I2C_CS0, false);
#endif
    if (!drvGeneralSpiFlashFinish())
    {
        OSI_LOGI(0x10009fce, "drvGeneralSpiFlashWrite Fail time out in line %d", __LINE__);
        return;
    }
}

static void prvErase(drvGeneralSpiFlash_t *d, uint32_t offset, size_t size)
{
}

static void prvEraseNoXipLocked(drvGeneralSpiFlash_t *d, uint32_t offset, size_t size)
{
    uint8_t OSI_ALIGNED(16) cmd[5] = {
        0,
    };
    uint8_t cmd_len = 4;
    drvGeneralSpiFlashWriteEnable();
    if (!drvGeneralSpiFlashFinish())
    {
        OSI_LOGI(0x10009fcf, "drvGeneralSpiFlashErase fail time out in line %d", __LINE__);
        return;
    }

#ifdef CONFIG_GENERAL_SPI_LC_FLASH
    if (size == SIZE_64K)
        cmd[0] = cmd_4Byte_BlockErase_64k;
    else if (size == SIZE_32K)
        cmd[0] = cmd_BlockErase_32k;
    else
        cmd[0] = cmd_4Byte_SectorEarse;

    cmd[1] = (offset & 0xff000000) >> 24;
    cmd[2] = (offset & 0xff0000) >> 16;
    cmd[3] = (offset & 0xff00) >> 8;
    cmd[4] = offset & 0xff;

    cmd_len = 5;
#else
    if (size == SIZE_64K)
        cmd[0] = cmd_BlockErase_64k;
    else if (size == SIZE_32K)
        cmd[0] = cmd_BlockErase_32k;
    else
        cmd[0] = cmd_SectorEarse;

    cmd[1] = (offset & 0xff0000) >> 16;
    cmd[2] = (offset & 0xff00) >> 8;
    cmd[3] = offset & 0xff;
#endif

#ifdef CONFIG_SOC_8910
    drvSpiPinControl(pGeneralSpiMaster, SPI_CS0_CTRL, FORCE_0_CTRL);
    drvSpiWrite(pGeneralSpiMaster, SPI_I2C_CS0, cmd, cmd_len);
    drvSpiPinControl(pGeneralSpiMaster, SPI_CS0_CTRL, SPI_CTRL);
#elif defined(CONFIG_SOC_8850)
    drvSpiCsConfig(pGeneralSpiMaster, SPI_I2C_CS0, true);
    drvSpiWrite(pGeneralSpiMaster, cmd, cmd_len);
    drvSpiCsConfig(pGeneralSpiMaster, SPI_I2C_CS0, false);
#endif
    if (!drvGeneralSpiFlashFinish())
    {
        OSI_LOGI(0x10009fcf, "drvGeneralSpiFlashErase fail time out in line %d", __LINE__);
        return;
    }
}

int drvGeneralSpiFlashRead(uint32_t offset, uint32_t size, void *pbuf)
{
    uint8_t OSI_ALIGNED(16) cmd[5] = {0};
    bool bret = true;
    uint8_t cmd_len = 4;

#ifdef CONFIG_GENERAL_SPI_LC_FLASH
    cmd[0] = cmd_4Byte_ReadData;
    cmd[1] = (offset & 0xff000000) >> 24;
    cmd[2] = (offset & 0xff0000) >> 16;
    cmd[3] = (offset & 0xff00) >> 8;
    cmd[4] = offset & 0xff;

    cmd_len = 5;
#else
    cmd[0] = cmd_ReadData;
    cmd[1] = (offset & 0xff0000) >> 16;
    cmd[2] = (offset & 0xff00) >> 8;
    cmd[3] = offset & 0xff;
#endif

    if (dummpdata[0] == 0)
        memset(dummpdata, 0xff, sizeof(dummpdata));
#ifdef CONFIG_SOC_8910
    drvSpiPinControl(pGeneralSpiMaster, SPI_CS0_CTRL, FORCE_0_CTRL);
    drvSpiWrite(pGeneralSpiMaster, SPI_I2C_CS0, cmd, cmd_len);
    drvSpiRead(pGeneralSpiMaster, SPI_I2C_CS0, dummpdata, pbuf, size);
    drvSpiPinControl(pGeneralSpiMaster, SPI_CS0_CTRL, SPI_CTRL);
#elif defined(CONFIG_SOC_8850)
    drvSpiCsConfig(pGeneralSpiMaster, SPI_I2C_CS0, true);
    drvSpiWrite(pGeneralSpiMaster, cmd, cmd_len);
    drvSpiRead(pGeneralSpiMaster, pbuf, size);
    drvSpiCsConfig(pGeneralSpiMaster, SPI_I2C_CS0, false);
#endif

    return bret;
}

bool drvGeneralSpiFlashWrite(drvGeneralSpiFlash_t *d, uint32_t offset, const void *data, size_t size)
{
    if (data == NULL || offset + size > d->capacity)
    {
        OSI_LOGI(0x1000aa29, "drvGeneralSpiFlashWrite Start, offset= 0x%x, data=0x%x, size=0x%x, capacity=0x%x", offset, data, size, d->capacity);
        return false;
    }
    osiMutexLock(d->lock);
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
    osiMutexUnlock(d->lock);
    return true;
}

bool drvGeneralSpiFlashErase(drvGeneralSpiFlash_t *d, uint32_t offset, size_t size)
{
    if (offset + size > d->capacity ||
        !OSI_IS_ALIGNED(offset, SIZE_4K) ||
        !OSI_IS_ALIGNED(size, SIZE_4K))
        return false;

    osiMutexLock(d->lock);
    if (d->xip_protect)
    {
        while (size > 0)
        {
            if (OSI_IS_ALIGNED(offset, SIZE_64K) && size >= SIZE_64K)
            {
                prvErase(d, offset, SIZE_64K);
                offset += SIZE_64K;
                size -= SIZE_64K;
            }
            else if (OSI_IS_ALIGNED(offset, SIZE_32K) && size >= SIZE_32K)
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
            if (OSI_IS_ALIGNED(offset, SIZE_64K) && size >= SIZE_64K)
            {
                prvEraseNoXipLocked(d, offset, SIZE_64K);
                offset += SIZE_64K;
                size -= SIZE_64K;
            }
            else if (OSI_IS_ALIGNED(offset, SIZE_32K) && size >= SIZE_32K)
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

    osiMutexUnlock(d->lock);
    return true;
}

bool drvGeneralSpiFlashWriteEnableTest(void)
{
    char OSI_ALIGNED(16) data[1];
    char OSI_ALIGNED(16) readData[2] = {0};
    bool bret = true;
    uint32_t startTick = 0, checkTick = 0;
    startTick = osiUpTime();
#ifdef CONFIG_SOC_8910
    char OSI_ALIGNED(16) sDataBuf[1] = {0};
#endif

    do
    {
#ifdef CONFIG_SOC_8910
        data[0] = cmd_WriteEnable;
        drvSpiPinControl(pGeneralSpiMaster, SPI_CS0_CTRL, FORCE_0_CTRL);
        drvSpiWrite(pGeneralSpiMaster, SPI_I2C_CS0, data, 1);
        drvSpiPinControl(pGeneralSpiMaster, SPI_CS0_CTRL, SPI_CTRL);
        data[0] = cmd_ReadStatusReg;
        drvSpiPinControl(pGeneralSpiMaster, SPI_CS0_CTRL, FORCE_0_CTRL);
        drvSpiWrite(pGeneralSpiMaster, SPI_I2C_CS0, data, 1);
        drvSpiRead(pGeneralSpiMaster, SPI_I2C_CS0, sDataBuf, readData, 1);
        drvSpiPinControl(pGeneralSpiMaster, SPI_CS0_CTRL, SPI_CTRL);
#elif defined(CONFIG_SOC_8850)
        data[0] = cmd_WriteEnable;
        drvSpiCsConfig(pGeneralSpiMaster, SPI_I2C_CS0, true);
        drvSpiWrite(pGeneralSpiMaster, data, 1);
        drvSpiCsConfig(pGeneralSpiMaster, SPI_I2C_CS0, false);
        data[0] = cmd_ReadStatusReg;
        drvSpiCsConfig(pGeneralSpiMaster, SPI_I2C_CS0, true);
        drvSpiWrite(pGeneralSpiMaster, data, 1);
        drvSpiRead(pGeneralSpiMaster, readData, 1);
        drvSpiCsConfig(pGeneralSpiMaster, SPI_I2C_CS0, false);
#endif
        checkTick = osiUpTime();
        if ((checkTick - startTick) > 6000)
        {
            OSI_LOGI(0x1000aa2a, "\r\n****Error:drvGeneralSpiFlashWriteEnableTest Timeout*****\r\n");
            bret = false;
            break;
        }
    } while ((readData[0] & (0x1 << 1)) == 0);

    return bret;
}

bool drvGeneralSpiFlashWriteDisableTest(void)
{
    char OSI_ALIGNED(16) data[1];
    char OSI_ALIGNED(16) readData[2] = {0};
    bool bret = true;
    uint32_t startTick = 0, checkTick = 0;
#ifdef CONFIG_SOC_8910
    char OSI_ALIGNED(16) sDataBuf[1] = {0};
#endif

    startTick = osiUpTime();
    do
    {
#ifdef CONFIG_SOC_8910
        data[0] = 0x04;
        drvSpiPinControl(pGeneralSpiMaster, SPI_CS0_CTRL, FORCE_0_CTRL);
        drvSpiWrite(pGeneralSpiMaster, SPI_I2C_CS0, data, 1);
        drvSpiPinControl(pGeneralSpiMaster, SPI_CS0_CTRL, SPI_CTRL);
        data[0] = cmd_ReadStatusReg;
        drvSpiPinControl(pGeneralSpiMaster, SPI_CS0_CTRL, FORCE_0_CTRL);
        drvSpiWrite(pGeneralSpiMaster, SPI_I2C_CS0, data, 1);
        drvSpiRead(pGeneralSpiMaster, SPI_I2C_CS0, sDataBuf, readData, 1);
        drvSpiPinControl(pGeneralSpiMaster, SPI_CS0_CTRL, SPI_CTRL);
#elif defined(CONFIG_SOC_8850)
        data[0] = 0x04;
        drvSpiCsConfig(pGeneralSpiMaster, SPI_I2C_CS0, true);
        drvSpiWrite(pGeneralSpiMaster, data, 1);
        drvSpiCsConfig(pGeneralSpiMaster, SPI_I2C_CS0, false);
        data[0] = cmd_ReadStatusReg;
        drvSpiCsConfig(pGeneralSpiMaster, SPI_I2C_CS0, true);
        drvSpiWrite(pGeneralSpiMaster, data, 1);
        drvSpiRead(pGeneralSpiMaster, readData, 1);
        drvSpiCsConfig(pGeneralSpiMaster, SPI_I2C_CS0, false);
#endif
        checkTick = osiUpTime();
        if ((checkTick - startTick) > 60000)
        {
            OSI_LOGI(0x1000aa2b, "\r\n****Error:drvGeneralSpiFlashWriteDisableTest Timeout*****\r\n");
            bret = false;
            break;
        }
    } while ((readData[0] & ((0x1 << 1))) == (0x1 << 1));
    return bret;
}

bool drvGeneralSpiFlashWriteCompleteTest(void)
{
    char OSI_ALIGNED(16) data[1];
    char OSI_ALIGNED(16) readData[2] = {0};
    bool bret = true;
    uint32_t startTick = 0, checkTick = 0;
    startTick = osiUpTime();
#ifdef CONFIG_SOC_8910
    char OSI_ALIGNED(16) sDataBuf[1] = {0};
#endif
    do
    {
#ifdef CONFIG_SOC_8910
        data[0] = cmd_ReadStatusReg;
        drvSpiPinControl(pGeneralSpiMaster, SPI_CS0_CTRL, FORCE_0_CTRL);
        drvSpiWrite(pGeneralSpiMaster, SPI_I2C_CS0, data, 1);
        drvSpiRead(pGeneralSpiMaster, SPI_I2C_CS0, sDataBuf, readData, 1);
        drvSpiPinControl(pGeneralSpiMaster, SPI_CS0_CTRL, SPI_CTRL);
#elif defined(CONFIG_SOC_8850)
        data[0] = cmd_ReadStatusReg;
        drvSpiCsConfig(pGeneralSpiMaster, SPI_I2C_CS0, true);
        drvSpiWrite(pGeneralSpiMaster, data, 1);
        drvSpiRead(pGeneralSpiMaster, readData, 1);
        drvSpiCsConfig(pGeneralSpiMaster, SPI_I2C_CS0, false);
#endif
        checkTick = osiUpTime();
        if ((checkTick - startTick) > 60000)
        {
            OSI_LOGI(0x1000aa2c, "Error:drvGeneralSpiFlashWriteCompleteTest Timeout");
            bret = false;
            break;
        }
    } while ((readData[0] & (0x01)));
    return bret;
}

int drvGeneralSpiFlashEraseTest(uint32_t offset, uint32_t size)
{
    char OSI_ALIGNED(16) data[5] = {0};
    bool ret = false;
    uint8_t data_len = 4;
    unsigned int addr = offset;
    OSI_LOGI(0x10009fd4, "drvGeneralSpiFlashEraseTest:erase start\r\n");
    do
    {
        if (size > 0x100)
        {
            OSI_LOGE(0x1000aa2d, "drvGeneralSpiFlashEraseTest:Error size %d!", size);
            break;
        }
        ret = drvGeneralSpiFlashWriteEnableTest();
        if (!ret)
        {
            break;
        }

#ifdef CONFIG_GENERAL_SPI_LC_FLASH
        data[0] = cmd_SectorEarse;
        data[1] = (addr >> 24) & 0xff;
        data[2] = (addr >> 16) & 0xff;
        data[3] = (addr >> 8) & 0xff;
        data[4] = (addr & 0xff);
        data_len = 5;
#else
        data[0] = cmd_SectorEarse;
        data[1] = (addr >> 16) & 0xff;
        data[2] = (addr >> 8) & 0xff;
        data[3] = (addr & 0xff);
#endif

#ifdef CONFIG_SOC_8910
        drvSpiPinControl(pGeneralSpiMaster, SPI_CS0_CTRL, FORCE_0_CTRL);
        drvSpiWrite(pGeneralSpiMaster, SPI_I2C_CS0, data, data_len);
        drvSpiPinControl(pGeneralSpiMaster, SPI_CS0_CTRL, SPI_CTRL);
#elif defined(CONFIG_SOC_8850)
        drvSpiCsConfig(pGeneralSpiMaster, SPI_I2C_CS0, true);
        drvSpiWrite(pGeneralSpiMaster, data, data_len);
        drvSpiCsConfig(pGeneralSpiMaster, SPI_I2C_CS0, false);
#endif
        if (drvGeneralSpiFlashWriteCompleteTest() == false)
            break;

        if (drvGeneralSpiFlashWriteDisableTest() == false)
            break;
        ret = true;

    } while (0);
    OSI_LOGI(0x10009fd5, "drvGeneralSpiFlashEraseTest: erase end\r\n");

    return ret;
}

int drvGeneralSpiFlashWriteTest(uint32_t offset, uint32_t size, const void *buf)
{
    char OSI_ALIGNED(16) data[5] = {0};
    uint32_t addr = 0;
    uint32_t pageNo = 0; //256 bytes per page
    uint8_t data_len = 4;
    bool bret = true;
    uint32_t sectorID = offset / SPI_SECTOR_SIZE;
#ifdef CONFIG_GENERAL_SPI_LC_FLASH
    uint32_t max_sectorID = 4096 * 2; // 32M
#else
    uint32_t max_sectorID = 4096; //16M
#endif

    OSI_LOGI(0x10009fd0, "spi_flash_write (0x%x) size/%d", buf, size);
    do
    {
        if ((sectorID < 0) || (sectorID >= max_sectorID))
        {
            bret = false;
            break;
        }

        //erase one block
        if (!drvGeneralSpiFlashEraseTest(offset, size))
        {
            OSI_LOGI(0x10009fd1, "drvGeneralSpiFlashWriteTest:erase block fail\r\n");
        }

        OSI_LOGI(0x10009fd2, "drvGeneralSpiFlashWriteTest:write start\r\n");
        drvGeneralSpiFlashWriteEnableTest();
        //READ LOOP
        addr = offset;
        addr = addr + (pageNo << 8);
#ifdef CONFIG_GENERAL_SPI_LC_FLASH
        data[0] = cmd_4Byte_PageProgram;
        data[1] = (addr >> 24) & 0xff;
        data[2] = (addr >> 16) & 0xff;
        data[3] = (addr >> 8) & 0xff;
        data[4] = (addr & 0xff);

        data_len = 5;
#else
        data[0] = cmd_PageProgram;
        data[1] = (addr >> 16) & 0xff;
        data[2] = (addr >> 8) & 0xff;
        data[3] = (addr & 0xff);
#endif

#ifdef CONFIG_SOC_8910
        drvSpiPinControl(pGeneralSpiMaster, SPI_CS0_CTRL, FORCE_0_CTRL);
        drvSpiWrite(pGeneralSpiMaster, SPI_I2C_CS0, data, data_len);
        drvSpiWrite(pGeneralSpiMaster, SPI_I2C_CS0, buf, SPI_PAGE_SIZE);
        drvSpiPinControl(pGeneralSpiMaster, SPI_CS0_CTRL, SPI_CTRL);
#elif defined(CONFIG_SOC_8850)
        drvSpiCsConfig(pGeneralSpiMaster, SPI_I2C_CS0, true);
        drvSpiWrite(pGeneralSpiMaster, data, data_len);
        drvSpiWrite(pGeneralSpiMaster, buf, SPI_PAGE_SIZE);
        drvSpiCsConfig(pGeneralSpiMaster, SPI_I2C_CS0, false);
#endif
        drvGeneralSpiFlashWriteDisableTest();

    } while (0);
    OSI_LOGI(0x10009fd3, "drvGeneralSpiFlashWriteTest:write end\r\n");
    return bret;
}
