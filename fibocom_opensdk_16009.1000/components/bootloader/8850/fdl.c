/* Copyright (C) 2019 RDA Technologies Limited and/or its affiliates("RDA").
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

#define OSI_LOCAL_LOG_TAG OSI_MAKE_LOG_TAG('F', 'D', 'L', '1')
// #define OSI_LOCAL_LOG_LEVEL OSI_LOG_LEVEL_DEBUG

#include "boot_fdl.h"
#include "boot_entry.h"
#include "boot_platform.h"
#include "boot_mem.h"
#include "boot_mmu.h"
#include "boot_spi_flash.h"
#include "hal_chip.h"
#include "boot_secure.h"
#include "boot_bsl_cmd.h"
#include "hal_config.h"
#include "cmsis_core.h"
#include "osi_log.h"
#include "osi_api.h"
#include "drv_names.h"
#include "diag_config.h"
#include "hwregs.h"
#include "cpio_parser.h"
#include "nvm.h"
#include "vfs.h"
#include "fs_mount.h"
#include "calclib/crc32.h"
#include "hal_adi_bus.h"
#include "8850/hal_rom_api.h"
#include <sys/reent.h>
#include <string.h>
#include <stdlib.h>
#include "fibocom.h"

// Work memory size in context
#define WORKMEM_SIZE (CONFIG_NVBIN_FIXED_SIZE < 0x10000 ? 0x10000 : CONFIG_NVBIN_FIXED_SIZE)
#ifdef CONFIG_FIBOCOM_BASE
#if defined(CONFIG_FIBOCUS_JIEKE_FEATURE)
#define PACKAGE_FILE_MAX_SIZE 0x100000
#else
#if defined(CONFIG_FIBOCOM_TTS_EXT_SUPPORT)
#define PACKAGE_FILE_MAX_SIZE 0x100000
#else
#define PACKAGE_FILE_MAX_SIZE 0x50000
#endif
#endif
#else
#define PACKAGE_FILE_MAX_SIZE 0x10000
#endif

#define FIXEDNV_ID_COUNT_MAX (64)

// Logic addresses, which should match the value used in pac file
#define ERASE_RUNNING_NV_LOGIC_ADDRESS 0xFE000001
#define PHASECHECK_LOGIC_ADDRESS 0xFE000002
#define FIXNV_LOGIC_ADDRESS 0xFE000003
#define PRE_PACK_FILE_LOGIC_ADDRESS 0xFE000004
#define DEL_APPIMG_LOGIC_ADDRESS 0xFE000005
#define FMT_FLASH_LOGIC_ADDRESS 0xFE000006

#define FLASH_ERASE_ALL 0x00000000
#define FLASH_ERASE_ALL_SIZE 0xFFFFFFFF

#define FLASH_PAGE_SIZE (256)
#define FLASH_SECTOR_SIZE (0x1000)

#define FDL_SIZE_4K (0x1000)

#define DELAY_POWEROFF_TIMEOUT (2000)

#define ERR_REP_RETURN(fdl, err) OSI_DO_WHILE0(fdlEngineSendRespNoData(fdl, err); return;)
#ifdef CONFIG_BOARD_WITH_EXT_FLASH
#define IS_FLASH_ADDRESS(address) (HAL_FLASH_DEVICE_NAME(address) == DRV_NAME_SPI_FLASH || HAL_FLASH_DEVICE_NAME(address) == DRV_NAME_SPI_FLASH_EXT)
#else
#define IS_FLASH_ADDRESS(address) (HAL_FLASH_DEVICE_NAME(address) == DRV_NAME_SPI_FLASH)
#endif

enum
{
    FS_PREPARE_WRITE_PHASECHEK,
    FS_PREPARE_WRITE_FIXEDNV,
    FS_PREPARE_WRITE_PREPACK,
    FS_PREPARE_READ_PHASECHECK,
    FS_PREPARE_READ_FIXEDNV,
    FS_PREPARE_ERASE_RUNNINGNV,
    FS_PREPARE_ERASE_PHASECHECK,
    FS_PREPARE_ERASE_APPIMG_FILE,
    FS_PREPARE_BACKUP_PHASE_CHECK,
};

typedef struct
{
    cpioStream_t *cpio;
    fdlDataDnld_t dnld;
    bool phasecheck_backed;
    bool secure_boot_enable;
    bool delay_poweroff;
    bool flash_write_whole;
    bool fixednv_written;
    unsigned device_type;
    fdlChannel_t *channel;
    unsigned max_packet_len;
    osiElapsedTimer_t connect_timer;
    phaseCheckHead_t phasecheck_back;
    char work_mem[WORKMEM_SIZE];
    char fixednv_write_data[CONFIG_NVBIN_FIXED_SIZE];
    unsigned fixednv_write_size;
    unsigned fixednv_read_size;
    char *flash_mem;
} fdlContext_t;

#if defined(CONFIG_FIBOCOM_BASE)
extern bool drvGetUId(osiBits64_t *id);
#endif

/**
 * Mount factory partition
 */
static bool prvMountFactory(fdlContext_t *d)
{
    if (!fsMountFsPath(CONFIG_FS_FACTORY_MOUNT_POINT, false))
        return false;

    return true;
}

/**
 * Prepare file system for various senario
 */
static bool prvFsPrepare(fdlContext_t *d, unsigned kind)
{
    switch (kind)
    {
    case FS_PREPARE_WRITE_PHASECHEK:
    case FS_PREPARE_WRITE_FIXEDNV:
        return fsMountWithFormatAll();

    case FS_PREPARE_WRITE_PREPACK:
    case FS_PREPARE_ERASE_RUNNINGNV:
    case FS_PREPARE_ERASE_APPIMG_FILE:
    case FS_PREPARE_READ_PHASECHECK:
    case FS_PREPARE_ERASE_PHASECHECK:
    case FS_PREPARE_READ_FIXEDNV:
        fsMountAll(); // ignore error
        return prvMountFactory(d);

    default:
        return false;
    }
}

/**
 * BSL_CMD_CONNECT, the timestamp is recorded.
 */
static void prvConnect(fdlEngine_t *fdl, fdlContext_t *d)
{
    OSI_LOGI(0x10009061, "FDL: connect");

    osiElapsedTimerStart(&d->connect_timer);
    fdlEngineSendRespNoData(fdl, BSL_REP_ACK);
}

/**
 * Flash download start. The common information start/total/received has
 * already initialized. Return response error code.
 */
static int prvFlashStart(fdlContext_t *d)
{
    fdlDataDnld_t *dn = &d->dnld;

    // Though it doesn't look reasonable, unaligned start address is supported.
    // In this case, the write address is not the same as erase address.
    dn->write_address = dn->start_address;
    dn->erase_address = OSI_ALIGN_DOWN(dn->start_address, FLASH_SECTOR_SIZE);

    unsigned faddress = HAL_FLASH_OFFSET(dn->erase_address);
    unsigned fend = HAL_FLASH_OFFSET(OSI_ALIGN_UP(dn->start_address + dn->total_size, FLASH_SECTOR_SIZE));
    int esize = bootSpiFlashEraseNoWait(dn->flash, faddress, fend - faddress);
    OSI_LOGV(0x10009062, "FDL2 flash erase no wait 0x%x/0x%x/%d", faddress, fend - faddress, esize);

    if (esize < 0)
    {
        dn->flash_stage = FLASH_STAGE_NONE;
        OSI_LOGE(0x10009063, "FDL: flash start failed 0x%x/0x%x", dn->start_address, dn->total_size);
        return -BSL_REP_DOWN_DEST_ERROR;
    }

    dn->erase_address += esize;
    dn->flash_stage = FLASH_STAGE_ERASE;
    dn->data_verify = crc32Init();
    return 0;
}

/**
 * Flash polling, called in FDL polling, midst and end. Don't access
 * \p dnld.received_size, rather the parameter should be used.
 */
static void prvFlashPolling(fdlContext_t *d, unsigned received)
{
    fdlDataDnld_t *dn = &d->dnld;

    if (dn->flash_stage == FLASH_STAGE_NONE ||
        dn->flash_stage == FLASH_STAGE_FINISH)
        return;

    if (dn->flash_stage == FLASH_STAGE_ERASE)
    {
        if (!bootSpiFlashIsDone(dn->flash))
            return;

        dn->flash_stage = FLASH_STAGE_WAIT_DATA;
    }
    else if (dn->flash_stage == FLASH_STAGE_WAIT_DATA)
    {
        int avail = received - (dn->write_address - dn->start_address);
        if (avail < FLASH_PAGE_SIZE || dn->write_address >= dn->erase_address)
            return;

        unsigned waddress = HAL_FLASH_OFFSET(dn->write_address);
        void *wdata = &d->work_mem[dn->write_address % WORKMEM_SIZE];
        int wsize = bootSpiFlashWriteNoWait(dn->flash, waddress, wdata, FLASH_PAGE_SIZE);
        OSI_LOGV(0x10009064, "FDL2 flash write no wait 0x%x/0x%x", waddress, wsize);

        dn->write_address += wsize;
        dn->flash_stage = FLASH_STAGE_WRITE;
    }
    else if (dn->flash_stage == FLASH_STAGE_WRITE)
    {
        if (!bootSpiFlashIsDone(dn->flash))
            return;

        if (dn->write_address < dn->erase_address)
        {
            dn->flash_stage = FLASH_STAGE_WAIT_DATA;
            return;
        }

        unsigned faddress = HAL_FLASH_OFFSET(dn->erase_address);
        unsigned fend = HAL_FLASH_OFFSET(OSI_ALIGN_UP(dn->start_address + dn->total_size, FLASH_SECTOR_SIZE));
        if (faddress >= fend)
        {
            dn->flash_stage = FLASH_STAGE_FINISH;
            return;
        }

        int esize = bootSpiFlashEraseNoWait(dn->flash, faddress, fend - faddress);
        OSI_LOGV(0x10009062, "FDL2 flash erase no wait 0x%x/0x%x/%d", faddress, fend - faddress, esize);
        dn->erase_address += esize;
        dn->flash_stage = FLASH_STAGE_ERASE;
    }
}

/**
 * Flash midst
 */
static void prvFlashMidst(fdlContext_t *d, const void *data, unsigned size)
{
    fdlDataDnld_t *dn = &d->dnld;

    dn->data_verify = crc32Update(dn->data_verify, data, size);

    // work_mem is reused as ring buffer. The index is address rather than
    // size. So, page write can always use linear buffer.
    unsigned mem_pos = (dn->start_address + dn->received_size) % WORKMEM_SIZE;
    unsigned tail = WORKMEM_SIZE - mem_pos;
    if (tail >= size)
    {
        memcpy(&d->work_mem[mem_pos], data, size);
    }
    else
    {
        memcpy(&d->work_mem[mem_pos], data, tail);
        memcpy(&d->work_mem[0], (const char *)data + tail, size - tail);
    }

    // Polling to make sure there are room for the next packet data.
    unsigned received = dn->received_size + size;
    unsigned received_address = dn->start_address + dn->received_size + size;
    while (received_address - dn->write_address > WORKMEM_SIZE - d->max_packet_len)
        prvFlashPolling(d, received);
}

/**
 * Flash end, write the remaining data and check crc.
 */
static int prvFlashEnd(fdlContext_t *d)
{
    fdlDataDnld_t *dn = &d->dnld;

    // size may be unaligned. So, polling to make sure the remaining data
    // less than a page.
    unsigned received_address = dn->start_address + dn->received_size;
    while (dn->write_address + FLASH_PAGE_SIZE <= received_address)
        prvFlashPolling(d, dn->received_size);

    // Write the remaining data, if exists.
    bootSpiFlashWaitDone(dn->flash);
    int remained = received_address - dn->write_address;
    if (remained > 0)
    {
        // It is possible that the remaining are located at unerased sector
        unsigned faddress = HAL_FLASH_OFFSET(dn->erase_address);
        unsigned fend = HAL_FLASH_OFFSET(OSI_ALIGN_UP(dn->start_address + dn->total_size, FLASH_SECTOR_SIZE));
        if (faddress < fend)
        {
            int esize = bootSpiFlashErase(dn->flash, faddress, fend - faddress);
            OSI_LOGV(0x10009065, "FDL2 flash erase 0x%x/0x%x/%d", faddress, fend - faddress, esize);
            dn->erase_address += esize;
        }

        unsigned waddress = HAL_FLASH_OFFSET(dn->write_address);
        void *wdata = &d->work_mem[dn->write_address % WORKMEM_SIZE];
        bootSpiFlashWrite(dn->flash, waddress, wdata, remained);
        OSI_LOGV(0x10009066, "FDL2 flash write 0x%x/0x%x", waddress, remained);
    }

    unsigned fstart = HAL_FLASH_OFFSET(dn->start_address);
    const void *fptr = bootSpiFlashMapAddress(dn->flash, fstart);
    unsigned fverify = crc32Calc(fptr, dn->total_size);

    dn->flash_stage = FLASH_STAGE_NONE;
    dn->flash = NULL;

    if (fverify != dn->data_verify)
    {
        OSI_LOGE(0x10009067, "FDL2 flash crc error, start/0x%x size/0x%x crc/0x%x calc/0x%x",
                 dn->start_address, dn->total_size, dn->data_verify, fverify);
        return -BSL_REP_VERIFY_ERROR;
    }
    return 0;
}

/**
 * Write flash after all data received.
 */
static int prvFlashWhole(fdlContext_t *d)
{
    unsigned start_addr = d->dnld.start_address;

    if (d->secure_boot_enable)
    {
        // flash image signature check
        if (!bootSimageCheckSign((const simageHeader_t *)(d->flash_mem)))
        {
            return -BSL_REP_SEC_VERIFY_ERROR;
        }
    }

    unsigned end_addr = start_addr + d->dnld.total_size;
    unsigned erase_offset = OSI_ALIGN_DOWN(HAL_FLASH_OFFSET(start_addr), FLASH_SECTOR_SIZE);
    unsigned erase_size = OSI_ALIGN_UP(HAL_FLASH_OFFSET(end_addr) - erase_offset, FLASH_SECTOR_SIZE);
    unsigned write_offset = HAL_FLASH_OFFSET(start_addr);
    unsigned write_size = d->dnld.total_size;

    if (!bootSpiFlashErase(d->dnld.flash, erase_offset, erase_size))
        return -BSL_REP_VERIFY_ERROR;
    if (!bootSpiFlashWrite(d->dnld.flash, write_offset, d->flash_mem, write_size))
        return -BSL_REP_VERIFY_ERROR;

    return 0;
}

/**
 * Post check after progressive flash write.
 */
static int prvFlashPostCheck(fdlContext_t *d)
{
    // The followings are only for secure boot
    if (!d->secure_boot_enable)
        return 0;

    if (!bootSimageCheckSign((const simageHeader_t *)(d->dnld.start_address)))
    {
        return -BSL_REP_SEC_VERIFY_ERROR;
    }
    return 0;
}

/**
 * Write one file from cpio.
 */
#if defined(CONFIG_FIBOCOM_HALUO_AT) && defined(CONFIG_FS_EXT_ENABLED)
extern char g_ExtMountName[64];
#endif
static int prvPackageFile(cpioFile_t *f)
{
    int ret = 0;
    char name[VFS_PATH_MAX + 1] = "/";
    bool isdir = f->mode & S_IFDIR;
    memcpy(name + 1, f->name, strlen(f->name) + 1);

    OSI_LOGXI(OSI_LOGPAR_SII, 0x10009f2f, "package file %s (%d/%u)", name, isdir, f->data_size);
#if defined(CONFIG_FIBOCOM_HALUO_AT) && defined(CONFIG_FS_EXT_ENABLED)
    if ((strlen(g_ExtMountName) != 0) && (memcmp(g_ExtMountName, name, strlen(g_ExtMountName)) == 0))
    {
        OSI_PRINTFI("package file %s (%s)", name, g_ExtMountName);
        return 0;
    }
#endif

#ifdef CONFIG_FIBOCOM_BASE

#if defined(CONFIG_FS_EXT_ENABLED)
    if ((strncasecmp("/ext", name, strlen("/ext")) == 0) && (bootSpiFlashOpen(DRV_NAME_SPI_FLASH_EXT) == NULL)) {
        OSI_LOGI(0, "FDL: find no ext flash, skip");
        return 0;
    }
#endif

    if (isdir)
    {
        ret = vfs_mkpath(name, 0);
        if (ret != 0) {
            OSI_LOGE(0, "FDL: err: prepack mkfilepath");
            return -1;
        }
    }
    else
    {
        ret = vfs_mkfilepath(name, 0);
        if (ret != 0) {
            OSI_LOGE(0, "FDL: err: prepack mkfilepath");
            return -1;
        }
        
        ret = vfs_file_write(name, f->data, f->data_size);
        if (ret < 0) {
            OSI_LOGE(0, "FDL: err: prepack file write");
            return -1;
        }
    }
    return 0;
#else
    if (isdir)
    {
        vfs_mkpath(name, 0);
    }
    else
    {
        vfs_mkfilepath(name, 0);
        ret = vfs_file_write(name, f->data, f->data_size);
    }
    return ret;
#endif
}

/**
 * BSL_CMD_START_DATA, return non-zero if response needed
 * When file system access is needed, it should be chcked here.
 */
static int prvDataStart(fdlEngine_t *fdl, fdlPacket_t *pkt, fdlContext_t *d)
{
    int result = 0;
    uint32_t *ptr = (uint32_t *)pkt->content;
    uint32_t start_addr = OSI_FROM_BE32(*ptr++);
    uint32_t file_size = OSI_FROM_BE32(*ptr++);

    OSI_LOGI(0x10009068, "FDL: data start, start_addr/0x%x, size/0x%x", start_addr, file_size);

    d->dnld.start_address = start_addr;
    d->dnld.total_size = file_size;
    d->dnld.received_size = 0;
    d->dnld.stage = SYS_STAGE_START;

    if (start_addr == PHASECHECK_LOGIC_ADDRESS)
    {
        if (file_size != sizeof(phaseCheckHead_t))
            return BSL_REP_DOWN_SIZE_ERROR;

        if (!prvFsPrepare(d, FS_PREPARE_WRITE_PHASECHEK))
            return BSL_REP_INCOMPATIBLE_PARTITION;

        return BSL_REP_ACK;
    }

    if (start_addr == FIXNV_LOGIC_ADDRESS)
    {
        if (file_size > CONFIG_NVBIN_FIXED_SIZE)
            return BSL_REP_DOWN_SIZE_ERROR;

        if (!prvFsPrepare(d, FS_PREPARE_WRITE_FIXEDNV))
            return BSL_REP_INCOMPATIBLE_PARTITION;

        return BSL_REP_ACK;
    }

    if (start_addr == PRE_PACK_FILE_LOGIC_ADDRESS)
    {
        if (!prvFsPrepare(d, FS_PREPARE_WRITE_PREPACK))
            return BSL_REP_INCOMPATIBLE_PARTITION;

        cpioStreamCfg_t cfg = {
            .file_size_max = PACKAGE_FILE_MAX_SIZE,
            .file_path_max = VFS_PATH_MAX,
        };
        d->cpio = cpioStreamCreate(&cfg);
        if (d->cpio == NULL)
            return BSL_PHONE_NOT_ENOUGH_MEMORY;

        return BSL_REP_ACK;
    }

    if (IS_FLASH_ADDRESS(start_addr))
    {
        bootSpiFlash_t *flash = bootSpiFlashOpen(HAL_FLASH_DEVICE_NAME(start_addr));
        if (flash == NULL)
            return BSL_REP_OPERATION_FAILED;

        d->dnld.flash = flash;
        d->flash_write_whole = false;

        if (start_addr == CONFIG_SPL_FLASH_ADDRESS)
            d->flash_write_whole = true;

        if (d->secure_boot_enable)
        {
#ifdef CONFIG_FDL_FLASH_SIGCHECK_BEFORE_WRITE
#ifndef CONFIG_BOOT_FLASH_DISABLE // modify by fibocom,@202540511 for 8850bm_gnss_plus project
            if (start_addr == CONFIG_APP_FLASH_ADDRESS ||
                start_addr == CONFIG_APPIMG_FLASH_ADDRESS ||
                start_addr == CONFIG_BOOT_FLASH_ADDRESS) //CQ:SPCSS01094074,mantis:MTC0598-588
#else
            if (start_addr == CONFIG_APP_FLASH_ADDRESS ||
                start_addr == CONFIG_APPIMG_FLASH_ADDRESS)
#endif
                d->flash_write_whole = true;
#endif
        }

        if (d->flash_write_whole)
        {
            d->flash_mem = malloc(d->dnld.total_size);
            if (d->flash_mem == NULL)
                return BSL_PHONE_NOT_ENOUGH_MEMORY;
        }
        else
        {
            if ((result = prvFlashStart(d)) < 0)
                return -result;
        }

        return BSL_REP_ACK;
    }

    return BSL_REP_DOWN_DEST_ERROR;
}

/**
 * BSL_CMD_MIDST_DATA, return non-zero if response needed
 */
static int prvDataMidst(fdlEngine_t *fdl, fdlPacket_t *pkt, fdlContext_t *d)
{
    uint16_t data_len = pkt->size;
    uint32_t start_addr = d->dnld.start_address;

    OSI_LOGD(0x10009069, "FDL: data midst, start_addr/0x%x, size/0x%x, received/0x%x, len/0x%x, stage/0x%x",
             d->dnld.start_address, d->dnld.total_size, d->dnld.received_size,
             data_len, d->dnld.stage);

    if ((d->dnld.stage != SYS_STAGE_START) && (d->dnld.stage != SYS_STAGE_GATHER))
    {
        d->dnld.flash_stage = FLASH_STAGE_NONE;
        d->dnld.flash = NULL;
        return BSL_REP_DOWN_NOT_START;
    }

    if (d->dnld.received_size + data_len > d->dnld.total_size)
    {
        d->dnld.flash_stage = FLASH_STAGE_NONE;
        d->dnld.flash = NULL;
        return BSL_REP_DOWN_SIZE_ERROR;
    }

    d->dnld.stage = SYS_STAGE_GATHER;

    if (start_addr == PRE_PACK_FILE_LOGIC_ADDRESS)
    {
        cpioStreamPushData(d->cpio, pkt->content, data_len);
        cpioFile_t *f;
        while ((f = cpioStreamPopFile(d->cpio)))
        {
#if defined(CONFIG_APPIMG_LOAD_FILE)
            if (d->secure_boot_enable && strcmp(f->name, &CONFIG_APPIMG_LOAD_FILE_NAME[1]) == 0)
            {
                OSI_LOGI(0x1000906a, "FDL: appimg file verify size %d", f->data_size);
                // TODO: appimg signature check
            }
#endif
#ifdef CONFIG_FIBOCOM_BASE
            if (prvPackageFile(f) != 0) {
                return BSL_WRITE_ERROR;
            }
#else
            if (prvPackageFile(f) < 0)
                return BSL_REP_DOWN_SIZE_ERROR;
#endif
            cpioStreamDestroyFile(d->cpio, f);
        }

        d->dnld.received_size += data_len;
        return BSL_REP_ACK;
    }

    if (IS_FLASH_ADDRESS(start_addr))
    {
        if (d->flash_write_whole)
            memcpy(&d->flash_mem[0] + d->dnld.received_size, pkt->content, data_len);
        else
            prvFlashMidst(d, pkt->content, data_len);

        d->dnld.received_size += data_len;
        return BSL_REP_ACK;
    }

    if (d->dnld.received_size + data_len > WORKMEM_SIZE)
        return BSL_REP_DOWN_SIZE_ERROR;

    memcpy(&d->work_mem[0] + d->dnld.received_size, pkt->content, data_len);
    d->dnld.received_size += data_len;
    return BSL_REP_ACK;
}

/**
 * BSL_CMD_END_DATA, return non-zero if response needed
 */
static int prvDataEnd(fdlEngine_t *fdl, fdlContext_t *d)
{
    int result = 0;
    uint32_t start_addr = d->dnld.start_address;

    OSI_LOGI(0x1000906b, "FDL: data end, start_addr/0x%x, size/0x%x, received/0x%x, stage/0x%x",
             d->dnld.start_address, d->dnld.total_size, d->dnld.received_size,
             d->dnld.stage);

    if ((d->dnld.stage != SYS_STAGE_START) && (d->dnld.stage != SYS_STAGE_GATHER))
    {
        d->dnld.flash_stage = FLASH_STAGE_NONE;
        d->dnld.flash = NULL;
        return BSL_REP_DOWN_NOT_START;
    }

    if (d->dnld.received_size != d->dnld.total_size)
    {
        d->dnld.flash_stage = FLASH_STAGE_NONE;
        d->dnld.flash = NULL;
        return BSL_REP_DOWN_SIZE_ERROR;
    }

    d->dnld.stage = SYS_STAGE_END;

    if (start_addr == PHASECHECK_LOGIC_ADDRESS)
    {
        phaseCheckHead_t *phase_check = (phaseCheckHead_t *)&d->work_mem[0];
        if (!nvmWritePhaseCheck(phase_check))
            return BSL_REP_OPERATION_FAILED;

        return BSL_REP_ACK;
    }

    if (start_addr == FIXNV_LOGIC_ADDRESS)
    {
        char *nvbin = &d->work_mem[0];
        if (nvmWriteFixedBin(nvbin, d->dnld.received_size) < 0)
            return BSL_REP_OPERATION_FAILED;

        d->fixednv_write_size = d->dnld.received_size;
        memcpy(d->fixednv_write_data, nvbin, d->fixednv_write_size);

        if (d->phasecheck_backed)
        {
            d->phasecheck_backed = false;
            if (!nvmWritePhaseCheck(&d->phasecheck_back))
                return BSL_REP_OPERATION_FAILED;
        }

        return BSL_REP_ACK;
    }

    if (start_addr == PRE_PACK_FILE_LOGIC_ADDRESS)
    {
        cpioStreamDestroy(d->cpio);
        d->cpio = NULL;
        return BSL_REP_ACK;
    }

    if (IS_FLASH_ADDRESS(start_addr))
    {
        if (d->flash_write_whole)
        {
            d->flash_write_whole = false;
            result = prvFlashWhole(d);
            free(d->flash_mem);
            d->flash_mem = NULL;

            return (result < 0) ? -result : BSL_REP_ACK;
        }

        if ((result = prvFlashEnd(d)) < 0)
            return -result;

        if ((result = prvFlashPostCheck(d)) < 0)
            return -result;

        return BSL_REP_ACK;
    }

    return BSL_REP_INVALID_CMD;
}

/**
 * BSL_CMD_NORMAL_RESET
 */
static void prvResetNormal(fdlEngine_t *fdl, fdlContext_t *d)
{
    OSI_LOGI(0x1000906c, "FDL: reset to normal");

    fdlEngineSendRespNoData(fdl, BSL_REP_ACK);
    osiDelayUS(2000);
    bootReset(BOOT_RESET_NORMAL);
}

/**
 * Polling for delayed poweroff
 */
static void prvPowerOffPolling(fdlContext_t *d)
{
    if (!d->delay_poweroff)
        return;

    if (BOOT_DNLD_FROM_UART(d->device_type))
    {
        unsigned ms = osiElapsedTime(&d->connect_timer);
        if (ms > DELAY_POWEROFF_TIMEOUT)
        {
            OSI_LOGI(0x1000906d, "FDL: poweroff timeout", ms);
            bootPowerOff();

            // ideally, we shouldn't come here
            OSI_LOGE(0x1000906e, "FDL: poweroff failed");
            d->delay_poweroff = false;
        }
    }

    if (BOOT_DNLD_FROM_USERIAL(d->device_type))
    {
        if (!d->channel->connected(d->channel))
        {
            OSI_LOGI(0x1000906f, "FDL: device not connected, poweroff");
            bootPowerOff();

            // ideally, we shouldn't come here
            OSI_LOGE(0x1000906e, "FDL: poweroff failed");
            d->delay_poweroff = false;
        }
    }
}

/**
 * BSL_CMD_POWER_OFF
 */
static void prvPowerOff(fdlEngine_t *fdl, fdlContext_t *d)
{
    OSI_LOGI(0x10009070, "FDL: poweroff");

    d->delay_poweroff = true;
    osiElapsedTimerStart(&d->connect_timer);
    fdlEngineSendRespNoData(fdl, BSL_REP_ACK);
}

/**
 * BSL_CMD_READ_FLASH, return non-zero if response needed
 */
static int prvReadFlash(fdlEngine_t *fdl, fdlPacket_t *pkt, fdlContext_t *d)
{
    uint32_t *ptr = (uint32_t *)pkt->content;
    uint32_t addr = OSI_FROM_BE32(*ptr++);
    uint32_t size = OSI_FROM_BE32(*ptr++);
    uint32_t offset = OSI_FROM_BE32(*ptr++); // not all address has this parameter

    if (offset == 0)
        OSI_LOGI(0x10009071, "FDL: read flash, addr/0x%x, size/0x%x, offset/0x%x", addr, size, offset);

    if (addr == PHASECHECK_LOGIC_ADDRESS)
    {
        if (!prvFsPrepare(d, FS_PREPARE_READ_PHASECHECK))
            return BSL_REP_INCOMPATIBLE_PARTITION;

        // reuse the working memory
        phaseCheckHead_t *phase_check = (phaseCheckHead_t *)&d->work_mem[0];
        memset(phase_check, 0, sizeof(phaseCheckHead_t));
        if (!nvmReadPhasecheck(phase_check))
            return BSL_REP_OPERATION_FAILED;

        fdlEngineSendRespData(fdl, BSL_REP_READ_FLASH, phase_check, sizeof(phaseCheckHead_t));
        return 0; // already send response
    }

    if (addr == FIXNV_LOGIC_ADDRESS)
    {
        // Assuming the read will be started with offset 0, and continuous
        char *nvbin = &d->work_mem[0];
        if (offset == 0)
        {
            if (!prvFsPrepare(d, FS_PREPARE_READ_FIXEDNV))
                return BSL_REP_INCOMPATIBLE_PARTITION;

            memset(nvbin, 0xff, CONFIG_NVBIN_FIXED_SIZE);
            int read_size = 0;
            if (d->fixednv_write_size > 0)
            {
                read_size = nvmReadUpdataFixedBin(d->fixednv_write_data, d->fixednv_write_size);
                if (read_size <= 0)
                    return BSL_REP_OPERATION_FAILED;

                memcpy(nvbin, d->fixednv_write_data, read_size);
            }
            else
            {
                read_size = nvmReadFixedBin(nvbin, CONFIG_NVBIN_FIXED_SIZE);
                if (read_size <= 0)
                    return BSL_REP_OPERATION_FAILED;
            }

            d->fixednv_read_size = read_size;
        }

        if (offset >= d->fixednv_read_size)
            return BSL_REP_INVALID_CMD;

        unsigned send_size = OSI_MIN(unsigned, d->fixednv_read_size - offset, size);
        fdlEngineSendRespData(fdl, BSL_REP_READ_FLASH, nvbin + offset, send_size);
        return 0; // already send response
    }

    if (IS_FLASH_ADDRESS(addr))
    {
        bootSpiFlash_t *flash = bootSpiFlashOpen(HAL_FLASH_DEVICE_NAME(addr));
        if (flash == NULL)
            return BSL_REP_OPERATION_FAILED;

        // valid flash offset not includes the end address
        if (!bootSpiFlashMapAddressValid(flash, (void *)(addr + size - 1)))
            return BSL_REP_INVALID_CMD;

        uint32_t flash_offset = HAL_FLASH_OFFSET(addr);
        const void *fdata = bootSpiFlashMapAddress(flash, flash_offset);
        fdlEngineSendRespData(fdl, BSL_REP_READ_FLASH, fdata, size);
        return 0; // already send response
    }

    return BSL_REP_INVALID_CMD;
}

/**
 * BSL_CMD_ERASE_FLASH, return non-zero if response needed
 */
static int prvEraseFlash(fdlEngine_t *fdl, fdlPacket_t *pkt, fdlContext_t *d)
{
    uint32_t *ptr = (uint32_t *)pkt->content;
    uint32_t addr = OSI_FROM_BE32(*ptr++);
    uint32_t size = OSI_FROM_BE32(*ptr++);

    OSI_LOGI(0x10009072, "FDL: erase flash, addr/0x%x, size/0x%x", addr, size);

    if (addr == ERASE_RUNNING_NV_LOGIC_ADDRESS)
    {
        if (!prvFsPrepare(d, FS_PREPARE_ERASE_RUNNINGNV))
            return BSL_REP_INCOMPATIBLE_PARTITION;

        nvmClearRunning(); // ignore error
        return BSL_REP_ACK;
    }

    if (addr == PHASECHECK_LOGIC_ADDRESS)
    {
        if (!prvFsPrepare(d, FS_PREPARE_ERASE_PHASECHECK))
            return BSL_REP_INCOMPATIBLE_PARTITION;

        nvmClearPhaseCheck(); // ignore error
        return BSL_REP_ACK;
    }

    if (addr == DEL_APPIMG_LOGIC_ADDRESS)
    {
#if defined(CONFIG_APPIMG_LOAD_FLASH) && defined(CONFIG_APPIMG_LOAD_FILE_NAME)
        if (!prvFsPrepare(d, FS_PREPARE_ERASE_APPIMG_FILE))
            return BSL_REP_INCOMPATIBLE_PARTITION;

        vfs_unlink(CONFIG_APPIMG_LOAD_FILE_NAME); // ignore error
#endif
        return BSL_REP_ACK;
    }

    if (addr == FMT_FLASH_LOGIC_ADDRESS)
    {
        // "size" is the flash block device name
        if (!fsMountFormatFlash(size))
            return BSL_REP_OPERATION_FAILED;
        return BSL_REP_ACK;
    }

    if (FLASH_ERASE_ALL == addr && FLASH_ERASE_ALL_SIZE == size)
    {
        // PC tool won't backup PhaseCheck. We will try to backup PhaseCheck
        // here. And if there is operation to write FIXEDNV, PhaseCheck will
        // be restored there.
        if (prvFsPrepare(d, FS_PREPARE_READ_PHASECHECK) &&
            nvmReadPhasecheck(&d->phasecheck_back))
            d->phasecheck_backed = true;

        fsUmountAll();

        bootSpiFlash_t *flash = bootSpiFlashOpen(DRV_NAME_SPI_FLASH);
        if (flash == NULL)
            return BSL_REP_OPERATION_FAILED;

        bootSpiFlashChipErase(flash);

#ifdef CONFIG_FS_EXT_ENABLED //merge patch by fibocom 20220608
        bootSpiFlash_t *flash_ext = bootSpiFlashOpen(DRV_NAME_SPI_FLASH_EXT);
#if defined(CONFIG_FIBOCOM_BASE)
        if (flash_ext != NULL)
        {
            bootSpiFlashChipErase(flash_ext);
        }
#else

        if (flash_ext == NULL)
            return BSL_REP_OPERATION_FAILED;

        bootSpiFlashChipErase(flash_ext);
#endif
#endif
        return BSL_REP_ACK;
    }

    if (IS_FLASH_ADDRESS(addr))
    {
        bootSpiFlash_t *flash = bootSpiFlashOpen(HAL_FLASH_DEVICE_NAME(addr));
        uint32_t flash_offset = HAL_FLASH_OFFSET(addr);
        if (!bootSpiFlashErase(flash, flash_offset, size))
            return BSL_REP_INVALID_CMD;
        return BSL_REP_ACK;
    }

    return BSL_REP_INVALID_CMD;
}

/**
 * BSL_CMD_CHANGE_BAUD
 */
static void prvSetBaud(fdlEngine_t *fdl, fdlPacket_t *pkt, fdlContext_t *d)
{
    uint32_t *ptr = (uint32_t *)pkt->content;
    uint32_t baud = OSI_FROM_BE32(*ptr++);

    OSI_LOGI(0x10009073, "FDL: change baud %d", baud);

    // This is special, ACK must be sent in old baud rate.
    fdlEngineSendRespNoData(fdl, BSL_REP_ACK);
    fdlEngineSetBaud(fdl, baud);
}

//20240531 add Start alick MTC0789-94  add tools can get chipid for liandi customer
static int prvHex2Char(uint8_t hex)
{
    if (hex <= 9)
        return (hex + '0');
    if (hex >= 0xa && hex <= 0xf)
        return (hex + 'A' - 10);
    return '0';
}

static void prvByte2Chars(uint8_t hex, uint8_t *ascii)
{
    uint8_t low_hex = hex & 0xf;
    uint8_t high_hex = (hex >> 4) & 0xf;
    ascii[0] = prvHex2Char(high_hex);
    ascii[1] = prvHex2Char(low_hex);
}
//20240531 add End alick MTC0789-94  add tools can get chipid for liandi customer

/**
 * Process packet, called by engine.
 */
static void prvProcessPkt(fdlEngine_t *fdl, fdlPacket_t *pkt, void *param)
{
    if (fdl == NULL || pkt == NULL)
        osiPanic();

    OSI_LOGV(0x10009074, "FDL: pkt type/0x%x, size/0x%x", pkt->type, pkt->size);

    fdlContext_t *d = (fdlContext_t *)param;
    int ret = 0;
    switch (pkt->type)
    {
    case BSL_CMD_CONNECT:
        prvConnect(fdl, d);
        break;

    case BSL_CMD_START_DATA:
        if ((ret = prvDataStart(fdl, pkt, d)) != 0)
            fdlEngineSendRespNoData(fdl, ret);
        break;

    case BSL_CMD_MIDST_DATA:
        if ((ret = prvDataMidst(fdl, pkt, d)) != 0)
            fdlEngineSendRespNoData(fdl, ret);
        break;

    case BSL_CMD_END_DATA:
        if ((ret = prvDataEnd(fdl, d)) != 0)
            fdlEngineSendRespNoData(fdl, ret);
        break;

    case BSL_CMD_NORMAL_RESET:
        prvResetNormal(fdl, d);
        break;

    case BSL_CMD_READ_FLASH:
        if ((ret = prvReadFlash(fdl, pkt, d)) != 0)
            fdlEngineSendRespNoData(fdl, ret);
        break;

    case BSL_CMD_ERASE_FLASH:
        if ((ret = prvEraseFlash(fdl, pkt, d)) != 0)
            fdlEngineSendRespNoData(fdl, ret);
        break;

    case BSL_CMD_POWER_OFF:
        prvPowerOff(fdl, d);
        break;

    case BSL_CMD_CHANGE_BAUD:
        prvSetBaud(fdl, pkt, d);
        break;

    case BSL_CMD_REPARTITION:
    case BSL_CMD_ENABLE_DEBUG_MODE:
        OSI_LOGI(0x10009075, "FDL: ignore packet type 0x%x", pkt->type);
        fdlEngineSendRespNoData(fdl, BSL_REP_ACK);
        break;

//20240531 add Start alick MTC0789-94  add tools can get chipid for liandi customer
    case BSL_CMD_READ_CHIP_UID:
        OSI_LOGI(0, "FDL: BSL_CMD_READ_CHIP_UID %x--%x", BSL_CMD_READ_CHIP_UID, BSL_REP_READ_CHIP_UID);
        UINT8 id[32];
        uint8_t u_id[8] = {};
        uint8_t u_id_ascii[16];

        drvGetUId((osiBits64_t *)&u_id);//no actual function, in order to link with libdriver.a
        memset(id,0,sizeof(id));
        for (int i = 0, j = 0; i < 8; i++)
        {
                prvByte2Chars(u_id[i], &u_id_ascii[j]);
                j += 2;
        }
        memcpy(id, u_id_ascii, sizeof(u_id_ascii));
        OSI_LOGE(0, "CHIPID[0 - 7]:%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x", id[0] , id[1], id[2],  id[3] , id[4] , id[5] , id[6] , id[7] );
        OSI_LOGE(0, "CHIPID[8 -15]:%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x", id[8] , id[9], id[10], id[11], id[12], id[13], id[14], id[15]);
        /*OSI_LOGE(0, "CHIPID[%s]", id);*/
        fdlEngineSendRespData(fdl, BSL_REP_READ_CHIP_UID, id, 16);
        break;
//20240531 add End alick MTC0789-94  add tools can get chipid for liandi customer

    default:
        OSI_LOGE(0x10009076, "FDL: unkonw packet 0x%x", pkt->type);
        fdlEngineSendRespNoData(fdl, BSL_REP_INVALID_CMD);
        break;
    }
}

/**
 * Polling. Called by engine
 */
static void prvPolling(fdlEngine_t *fdl, void *param)
{
    fdlContext_t *d = (fdlContext_t *)param;

    prvFlashPolling(d, d->dnld.received_size);
    prvPowerOffPolling(d);
}

/**
 * Start download
 */
static bool prvDnldStart(fdlEngine_t *fdl, unsigned devtype)
{
    fdlContext_t *d = calloc(1, sizeof(fdlContext_t));
    if (d == NULL)
        return false;

    halPmuExtFlashPowerOn();

    d->device_type = devtype;
    d->channel = fdlEngineGetChannel(fdl);
    d->max_packet_len = fdlEngineGetMaxPacketLen(fdl);
    d->secure_boot_enable = bootSecureBootEnable();

    fsMountSetScenario(FS_SCENRARIO_FDL);
    fdlEngineProcess(fdl, prvProcessPkt, prvPolling, d);
    free(d);
    return false;
}

#if defined(CONFIG_CHIP_8850_V3_BOARD) || defined(CONFIG_CHIP_8850_V4_BOARD) || defined(CONFIG_CHIP_8850_V111_BOARD)
void FdlbootSetUartIomux()
{
    /*

    hwp_iomux->gpio_16 = 0x18;    //UART2
    hwp_iomux->uart_2_cts = 0x10; // UART_4_RX
    hwp_iomux->pad_uart_2_cts &= ~IOMUX_PAD_UART_2_CTS_DRV(0x3);
    hwp_iomux->uart_2_rts = 0x10; // UART_4_TX
    hwp_iomux->pad_uart_2_cts |= IOMUX_PAD_UART_2_CTS_DRV(2);
    hwp_iomux->pad_uart_2_cts &= ~IOMUX_WPU;
    hwp_iomux->pad_uart_2_cts |= IOMUX_WPU;
    hwp_iomux->pad_uart_2_rts &= ~IOMUX_PAD_UART_2_RTS_DRV(0x3);
    hwp_iomux->pad_uart_2_rts |= IOMUX_PAD_UART_2_RTS_DRV(2);
    */
    hwp_iomux->gpio_2 = 0x1c;  //UART2_cts
    hwp_iomux->keyout_4 = 0xc; // UART_4_RX
    hwp_iomux->keyout_5 = 0xc; // UART_4_TX
    hwp_iomux->keyout_4 &= ~IOMUX_PAD_KEYOUT_4_DRV(0x3);
    hwp_iomux->keyout_4 |= IOMUX_PAD_KEYOUT_4_DRV(2);
    hwp_iomux->keyout_4 &= ~IOMUX_WPU;
    hwp_iomux->keyout_4 |= IOMUX_WPU;
    hwp_iomux->keyout_5 &= ~IOMUX_PAD_KEYOUT_5_DRV(0x3);
    hwp_iomux->keyout_5 |= IOMUX_PAD_KEYOUT_5_DRV(2);
}
#endif
#ifdef CONFIG_FIBOCOM_BASE
void fdl_gpio_init()//SPCSS01150065
{
    hwp_iomux->gpio_16 = IOMUX_FUNC_SEL(0);
    hwp_iomux->gpio_19 = IOMUX_FUNC_SEL(0);
    hwp_iomux->gpio_18 = IOMUX_FUNC_SEL(0);
    hwp_iomux->gpio_20 = IOMUX_FUNC_SEL(0);
    hwp_iomux->gpio_22 = IOMUX_FUNC_SEL(0);
    hwp_iomux->gpio_23 = IOMUX_FUNC_SEL(0);

    hwp_gpio2->gpio_oen_set_out = (1 << (23 - 16) | 1 << (22 - 16) | 1 << (20 - 16) | 1 << (19 - 16) | 1 << (18 - 16) | 1 << (16 - 16));
    hwp_gpio2->gpio_clr_reg = (1 << (23 - 16) | 1 << (22 - 16) | 1 << (20 - 16) | 1 << (19 - 16) | 1 << (18 - 16) | 1 << (16 - 16));
}

#endif
void bootStart(uint32_t param)
{
    OSI_CLEAR_SECTION(bss);

    halClockInit(HAL_CLOCK_INIT_FDL);
    halRamInit();

    bootMmuEnable();

    __FPU_Enable();
    _impure_ptr = _GLOBAL_REENT;

    extern uint32_t __sram_heap_start[];
    extern uint32_t __sram_heap_end[];
    extern uint32_t __ram_heap_start[];
    extern uint32_t __ram_heap_end[];
    unsigned sram_heap_size = OSI_PTR_DIFF(__sram_heap_end, __sram_heap_start);
    unsigned ram_heap_size = OSI_PTR_DIFF(__ram_heap_end, __ram_heap_start);
    bootHeapInit(__sram_heap_start, sram_heap_size, __ram_heap_start, ram_heap_size);
    bootHeapDefaultExtRam();

    uint32_t name = DRV_NAME_UART3;
    if (param == ROM_LOADPAR_DOWNLOAD_UART1)
        name = DRV_NAME_UART1;
    else if (param == ROM_LOADPAR_DOWNLOAD_UART2)
        name = DRV_NAME_UART2;
    else if (param == ROM_LOADPAR_DOWNLOAD_UART3)
        name = DRV_NAME_UART3;
    else if (param == ROM_LOADPAR_DOWNLOAD_UART4)
        name = DRV_NAME_UART4;
    else if (param == ROM_LOADPAR_DOWNLOAD_UART5)
        name = DRV_NAME_UART5;
    else if (param == ROM_LOADPAR_DOWNLOAD_UART6)
        name = DRV_NAME_UART6;
    else if (param == ROM_LOADPAR_DOWNLOAD_USERIAL)
        name = DRV_NAME_ACM0;

    bool trace_enable = false;
#ifdef CONFIG_FDL_LOG_ENABLED
    trace_enable = true;
#ifdef CONFIG_KERNEL_DIAG_TRACE
    if (name == CONFIG_DIAG_DEFAULT_UART)
        trace_enable = false;
#endif
#endif

#if defined(CONFIG_CHIP_8850_V3_BOARD) || defined(CONFIG_CHIP_8850_V4_BOARD) || defined(CONFIG_CHIP_8850_V111_BOARD)
    FdlbootSetUartIomux();
#endif
    bootTraceInit(trace_enable);

    OSI_LOGI(0x1000905c, "FDL: run %x", param);
    halAdiBusInit();
    bootResetPinEnable();
    bootPlatformInit();
    bootGetEfuseCtx();
    bootSpiFlashOpen(DRV_NAME_SPI_FLASH); // ensure accessible

    fdlChannel_t *ch;
    if (name == DRV_NAME_ACM0)
        ch = fdlOpenUsbSerial();
    else
        ch = fdlOpenUart(name, 0, false);
    fdlEngine_t *fdl = fdlEngineCreate(ch, CONFIG_FDL_PACKET_MAX_LEN);
#ifdef CONFIG_FIBOCOM_BASE
        fdl_gpio_init();
#endif
    fdlEngineIdentify(fdl, 0);
    fdlEngineSendVersion(fdl);
    prvDnldStart(fdl, param);

    // never return here
    osiPanic();
}
