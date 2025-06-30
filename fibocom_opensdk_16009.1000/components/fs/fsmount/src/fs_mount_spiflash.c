
#include "osi_log.h"
#include "osi_api.h"
#include "fs_mount.h"
#include "fs_mount_spiflash.h"
#include "flash_block_device.h"
#include "partition_block_device.h"
#include "sffs_vfs.h"
#include "vfs.h"
#include "hal_config.h"
#include "calclib/crc32.h"
#include <string.h>
#include <stdlib.h>
#include <sys/queue.h>
#include "drv_spi.h"
#include "drv_names.h"

#define SPI_FLASH_WRITE_TEST 0
blockDevice_t *gSpiFlashBdev = NULL;

static int prvMountSffs(const char *base_path, blockDevice_t *fbdev)
{
    blockDevice_t *bdev = (blockDevice_t *)fbdev;

    int r = 0;
    r = sffsVfsMount(base_path, bdev, 4, 128, false);
    if (r != 0)
    {
        OSI_LOGI(0x1000a07a, "prvMountSffs mount failed!");
        return -2;
    }
    OSI_LOGI(0x1000a07b, "prvMountSffs mount success");

    return 0;
}

#if SPI_FLASH_WRITE_TEST
static void prvSpiFlashWriteTest(void)
{
    uint8_t sendbuf[256] = {
        0x12,
        0x34,
        0x45,
        0x56,
        0x78,
        0x90,
        0x12,
        0x34,
        0x56,
        0x78,
        0x90,
    };
    uint8_t *rBuf = calloc(1, 256);

    drvGeneralSpiFlashWriteTest(0x00000000, 256, sendbuf);
    drvGeneralSpiFlashRead(0x00000000, 256, rBuf);
    OSI_LOGI(0x1000a07c, "=======RB dump start=====");
    for (int j = 0; j < 256; j++)
    {
        OSI_LOGI(0x1000137c, "0x%x", rBuf[j]);
    }
    OSI_LOGI(0x1000a07d, "=======RB dump end======");
    if (memcmp(rBuf, sendbuf, 256) == 0)
    {
        OSI_LOGI(0x1000a07e, "prvSpiFlashWriteTest:  ok");
    }
    else
    {
        OSI_LOGI(0x1000a07f, "prvSpiFlashWriteTest:  failed");
    }
    free(rBuf);
}
#endif

static bool prvMountGeneralSpiFlash(bool format)
{
    drvGeneralSpiFlash_t *flash = drvGeneralSpiFlashOpen(DRV_NAME_SPI1);
    if (flash != NULL)
        OSI_LOGI(0x1000a080, "drvGeneralSpiFlashOpen open Success!");
    else
        return false;

#if SPI_FLASH_WRITE_TEST
    prvSpiFlashWriteTest();
#endif

    blockDevice_t *pSpiFlashBdev = generalSpiflashBlockDeviceCreateV2(flash, FLASH_START_ADDR, FLASH_MOUNT_SIZE, FLASH_EB_SIZE, FLASH_PB_SIZE, 0);
    OSI_LOGI(0x1000b8a9, "generalSpiflashBlockDeviceCreateV2 create Success!");

    if (format == 1)
    {
        if (sffsVfsMkfs(pSpiFlashBdev) != 0)
        {
            OSI_LOGI(0x1000a082, "general spi flash format failed!");
            return false;
        }
        OSI_LOGI(0x1000a083, "general spi flash format success!");
    }

    if (prvMountSffs(CONFIG_GENERAL_SPI_FLASH_MOUNT_POINT, pSpiFlashBdev) == 0)
    {
        OSI_LOGI(0x1000a084, "general spi flash mount success!");
        gSpiFlashBdev = pSpiFlashBdev;
        return true;
    }
    else
    {
        OSI_LOGI(0x1000a085, "general spi flash mount failed, try to format and mount!");

        if (sffsVfsMkfs(pSpiFlashBdev) != 0)
        {
            OSI_LOGI(0x1000a082, "general spi flash format failed!");
            return false;
        }
        else
        {
            if (prvMountSffs(CONFIG_GENERAL_SPI_FLASH_MOUNT_POINT, pSpiFlashBdev) != 0)
            {
                OSI_LOGI(0x1000a086, "general spi flash format and mount failed!");
                blockDeviceDestroy(pSpiFlashBdev);
                return false;
            }
            OSI_LOGI(0x1000a087, "general spi flash force format success!");
        }
    }
    gSpiFlashBdev = pSpiFlashBdev;
    return true;
}

void fsUmountGeneralSpiFlash()
{
    vfs_umount(CONFIG_GENERAL_SPI_FLASH_MOUNT_POINT);

    uint32_t critical = osiEnterCritical();
    blockDevice_t *bdev = gSpiFlashBdev;
    gSpiFlashBdev = NULL;
    osiExitCritical(critical);
    if (bdev)
    {
        blockDeviceDestroy(bdev);
    }
}

bool fsMountGeneralSpiFlash(bool format)
{
    return prvMountGeneralSpiFlash(format);
}
