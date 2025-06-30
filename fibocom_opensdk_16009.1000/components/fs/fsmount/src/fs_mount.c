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

#include "osi_log.h"
#include "osi_api.h"
#include "fs_mount.h"
#include "drv_spi_flash.h"
#include "flash_block_device.h"
#include "partition_block_device.h"
#include "sffs_vfs.h"
#include "vfs.h"
#include "hal_config.h"
#include "calclib/crc32.h"
#include <string.h>
#include <stdlib.h>
#include <sys/queue.h>

#define SUBPART_MAX_COUNT (8)
#define PARTINFO_SIZE_MAX (64 * 1024)
#define HEADER_VERSION (0x100)
#define HEADER_MAGIC OSI_MAKE_TAG('P', 'R', 'T', 'I')
#define TYPE_FBDEV2 OSI_MAKE_TAG('F', 'B', 'D', '2')
#define TYPE_SUBPART OSI_MAKE_TAG('B', 'P', 'R', 'T')
#define TYPE_SFFS OSI_MAKE_TAG('S', 'F', 'F', 'S')

#ifdef CONFIG_FIBOCOM_BASE
#ifdef CONFIG_BOARD_WITH_EXT_FLASH
#ifdef CONFIG_FS_EXT_ENABLED
bool g_bExtFlashisExist=true;
#endif
#endif
#endif

#ifdef CONFIG_FS_EXT_ENABLED
#define DEVICE_FEXT OSI_MAKE_TAG('F', 'E', 'X', 'T')
uint8_t g_ucExtFsmountStatus =0;
#if defined(CONFIG_FIBOCOM_HALUO_AT)
char g_ExtMountName[64] = {0};
#endif
#endif


#define MOUNT_ONLY (0)
#define MOUNT_FORMAT_ON_FAIL (1)
#define MOUNT_FORCE_FORMAT (2)

enum
{
    PARTINFO_BOOTLOADER_RO = (1 << 0),
    PARTINFO_APPLICATION_RO = (1 << 1),
    PARTINFO_BOOTLOADER_IGNORE = (1 << 2),
    PARTINFO_APPLICATION_IGNORE = (1 << 3),
    PARTINFO_BOOTLOADER_FORMAT_ON_FAIL = (1 << 4),
    PARTINFO_APPLICATION_FORMAT_ON_FAIL = (1 << 5)
};

typedef struct
{
    uint32_t magic;
    uint32_t crc;
    uint32_t size;
    uint32_t version;
} descHeader_t;

typedef struct
{
    uint32_t type;
    uint32_t flash;
    uint32_t name;
    uint32_t offset;
    uint32_t size;
    uint32_t eb_size;
    uint32_t pb_size;
    uint32_t flags;
} descFbdev_t;

typedef struct
{
    uint32_t type;
    uint32_t device;
    uint32_t name;
    uint32_t offset;
    uint32_t count;
    uint32_t flags;
} descSubpart_t;

typedef struct
{
    uint32_t type;
    uint32_t device;
    char mount[64];
    uint32_t reserve_block;
    uint32_t flags;
} descSffs_t;

typedef struct bdevRegistry
{
    /**/ SLIST_ENTRY(bdevRegistry) iter;
    uint32_t type;
    uint32_t name;
    blockDevice_t *bdev;
#ifdef CONFIG_FIBOCOM_BASE
    char mountpoint[64];
#endif
} bdevRegistry_t;

static SLIST_HEAD(, bdevRegistry) gBdevList = SLIST_HEAD_INITIALIZER(gBdevList);
static fsMountScenario_t gFsScenario = FS_SCENRARIO_APP;
extern const unsigned gPartInfo[];

/**
 * Register block device to global list
 */
static bool prvRegisterBlockDevice(uint32_t type, uint32_t name, blockDevice_t *bdev)
{
    bdevRegistry_t *desc = calloc(1, sizeof(bdevRegistry_t));
    if (desc == NULL)
    {
        OSI_LOGE(0x10006ffd, "failed to add block device %4c", name);
        return false;
    }

    desc->type = type;
    desc->name = name;
    desc->bdev = bdev;
    SLIST_INSERT_HEAD(&gBdevList, desc, iter);
    return true;
}

/**
 * Unregister block device to global list, if exists
 */
static void prvUnregisterBlockDevice(blockDevice_t *bdev)
{
    bdevRegistry_t *desc;
    SLIST_FOREACH(desc, &gBdevList, iter)
    {
        if (desc->bdev == bdev)
        {
            SLIST_REMOVE(&gBdevList, desc, bdevRegistry, iter);
            free(desc);
            return;
        }
    }
}

#ifdef CONFIG_FIBOCOM_BASE
static bool prvBlockDeviceAddMountPoint(const char mountpoint[], blockDevice_t *bdev)
{
    bdevRegistry_t *desc;
    SLIST_FOREACH(desc, &gBdevList, iter)
    {
        if (desc->bdev == bdev)
        {
            strcpy(desc->mountpoint, mountpoint);
            return true;
        }
    }
    return false;
}

bool fsRegisterBlockDevice(const char mountpoint[], blockDevice_t *bdev)
{
    if (prvRegisterBlockDevice(TYPE_FBDEV2, 0, bdev) != true) {
        return false;
    }
    
    if (prvBlockDeviceAddMountPoint(mountpoint, bdev) != true) {
        return false;
    }
    
    return true;
}

void fsUnregisterBlockDevice(const char mountpoint[])
{
    bdevRegistry_t *desc;
    SLIST_FOREACH(desc, &gBdevList, iter)
    {
        if (strcmp(mountpoint, desc->mountpoint) == 0)
        {
            SLIST_REMOVE(&gBdevList, desc, bdevRegistry, iter);
            free(desc);
            return;
        }
    }
}

blockDevice_t *fsGetBlockDeviceByMountPoint(const char mountpoint[])
{
    bdevRegistry_t *desc;
    SLIST_FOREACH(desc, &gBdevList, iter)
    {
        if (strcmp(mountpoint, desc->mountpoint) == 0)
            return desc->bdev;
    }
    return NULL;
}

#endif

/**
 * Find from created block device list by name
 */
blockDevice_t *fsFindBlockDeviceByName(unsigned name)
{
    bdevRegistry_t *desc;
    SLIST_FOREACH(desc, &gBdevList, iter)
    {
        if (desc->name == name)
            return desc->bdev;
    }
    return NULL;
}

/**
 * Get all block device names
 */
int fsGetBlockDeviceNames(unsigned names[], unsigned count)
{
    int total = 0;
    bdevRegistry_t *desc;
    SLIST_FOREACH(desc, &gBdevList, iter)
    {
        names[total++] = desc->name;
    }
    return total;
}

/**
 * Helper for description type
 */
OSI_FORCE_INLINE static unsigned prvDescType(const void *ptr)
{
    return *(const unsigned *)ptr;
}

/**
 * Helper for description size, -1 on error
 */
static int prvDescSize(unsigned type)
{
    if (type == TYPE_FBDEV2)
        return sizeof(descFbdev_t);
    if (type == TYPE_SUBPART)
        return sizeof(descSubpart_t);
    if (type == TYPE_SFFS)
        return sizeof(descSffs_t);
    return -1;
}

/**
 * Find flash block device decription by name
 */
static const descFbdev_t *prvFindFbdevDescByName(unsigned name)
{
    uintptr_t ptr = (uintptr_t)gPartInfo;
    descHeader_t *header = (descHeader_t *)OSI_PTR_INCR_POST(ptr, sizeof(descHeader_t));
    uintptr_t ptr_end = (uintptr_t)gPartInfo + header->size;

    while (ptr < ptr_end)
    {
        uint32_t type = *(uint32_t *)ptr;
        if (type == TYPE_FBDEV2)
        {
            const descFbdev_t *desc = (const descFbdev_t *)ptr;
            if (desc->name == name)
                return desc;
        }
        ptr += prvDescSize(type);
    }
    return NULL;
}

/**
 * Find SFFS decription by mount point name
 */
static const descSffs_t *prvFindSffsDescByMount(const char *mount)
{
    uintptr_t ptr = (uintptr_t)gPartInfo;
    descHeader_t *header = (descHeader_t *)OSI_PTR_INCR_POST(ptr, sizeof(descHeader_t));
    uintptr_t ptr_end = (uintptr_t)gPartInfo + header->size;

    while (ptr < ptr_end)
    {
        uint32_t type = *(uint32_t *)ptr;
        if (type == TYPE_SFFS)
        {
            const descSffs_t *desc = (const descSffs_t *)ptr;
            if (strncmp(desc->mount, mount, 64) == 0)
                return desc;
        }
        ptr += prvDescSize(type);
    }
    return NULL;
}

/**
 * Find SFFS decription by device name
 */
static const descSffs_t *prvFindSffsDescByDeviceName(unsigned device)
{
    uintptr_t ptr = (uintptr_t)gPartInfo;
    descHeader_t *header = (descHeader_t *)OSI_PTR_INCR_POST(ptr, sizeof(descHeader_t));
    uintptr_t ptr_end = (uintptr_t)gPartInfo + header->size;

    while (ptr < ptr_end)
    {
        uint32_t type = *(uint32_t *)ptr;
        if (type == TYPE_SFFS)
        {
            const descSffs_t *desc = (const descSffs_t *)ptr;
            if (desc->device == device)
                return desc;
        }
        ptr += prvDescSize(type);
    }
    return NULL;
}

/**
 * Find sub-partitions by parent device name
 */
static int prvFindSubpartDescByParentName(const descSubpart_t *descs[], unsigned device)
{
    int count = 0;
    uintptr_t ptr = (uintptr_t)gPartInfo;
    descHeader_t *header = (descHeader_t *)OSI_PTR_INCR_POST(ptr, sizeof(descHeader_t));
    uintptr_t ptr_end = (uintptr_t)gPartInfo + header->size;

    while (ptr < ptr_end)
    {
        uint32_t type = *(uint32_t *)ptr;
        if (type == TYPE_SUBPART)
        {
            const descSubpart_t *desc = (const descSubpart_t *)ptr;
            if (desc->device == device)
                descs[count++] = desc;
        }
        ptr += prvDescSize(type);
    }
    return count;
}

/**
 * Helper for read-only from description flags
 */
static bool prvFlagReadOnly(unsigned flags)
{
    return (gFsScenario == FS_SCENRARIO_APP && (flags & PARTINFO_APPLICATION_RO)) ||
           (gFsScenario == FS_SCENRARIO_BOOTLOADER && (flags & PARTINFO_BOOTLOADER_RO));
}

/**
 * Helper for ignore from description flags
 */
static bool prvFlagIgnored(unsigned flags)
{
    return (gFsScenario == FS_SCENRARIO_APP && (flags & PARTINFO_APPLICATION_IGNORE)) ||
           (gFsScenario == FS_SCENRARIO_BOOTLOADER && (flags & PARTINFO_BOOTLOADER_IGNORE));
}

/**
 * Helper for format-on-fail from description flags
 */
static bool prvFlagFormatOnFail(unsigned flags)
{
    return (gFsScenario == FS_SCENRARIO_APP && (flags & PARTINFO_APPLICATION_FORMAT_ON_FAIL)) ||
           (gFsScenario == FS_SCENRARIO_BOOTLOADER && (flags & PARTINFO_BOOTLOADER_FORMAT_ON_FAIL));
}

/**
 * Mount SFFS. The block device should be valid.
 */
static bool prvMountSffs(const descSffs_t *desc, blockDevice_t *bdev, unsigned opt)
{
    if (opt == MOUNT_FORCE_FORMAT)
    {
        if (sffsVfsMkfs(bdev) != 0)
        {
            OSI_LOGE(0x10007002, "failed to format");
            return false;
        }
    }

    bool read_only = prvFlagReadOnly(desc->flags);
    if (sffsVfsMount(desc->mount, bdev, 4, desc->reserve_block, read_only) == 0)
    #ifdef CONFIG_FIBOCOM_BASE
    {
        return prvBlockDeviceAddMountPoint(desc->mount, bdev);
    }
    #else
        return true;
    #endif

    if (opt != MOUNT_FORMAT_ON_FAIL)
    {
        OSI_LOGXE(OSI_LOGPAR_S, 0x10007000, "failed to mount %s", desc->mount);
        return false;
    }

    if (sffsVfsMkfs(bdev) != 0)
    {
        OSI_LOGE(0x10007002, "failed to format");
        return false;
    }

    if (sffsVfsMount(desc->mount, bdev, 4, desc->reserve_block, read_only) != 0)
    {
        OSI_LOGXE(OSI_LOGPAR_S, 0x10007000, "failed to mount %s", desc->mount);
        return false;
    }
#ifdef CONFIG_FIBOCOM_BASE
        return prvBlockDeviceAddMountPoint(desc->mount, bdev);
#else
        return true;
#endif
}

/**
 * Create FBDEV2 if not exists. To reduce possibility to lose erase count
 * information, FORCE_FORMAT is treated as FORMAT_ON_FAIL.
 */
static bool prvCreateFbdev2(const descFbdev_t *desc, unsigned opt)
{
    if (fsFindBlockDeviceByName(desc->name) != NULL)
        return true;

    drvSpiFlash_t *flash = drvSpiFlashOpen(desc->flash);
    if (flash == NULL)
    {
        OSI_LOGE(0x10006ff9, "failed to open spi flash %4c", desc->flash);
        return false;
    }

#ifdef CONFIG_FIBOCOM_EXTFS_MOUNT_SIZE_ADAPTIVE // mount size self-adapting
    uint32_t mount_size = 0;

    if (desc->name == DEVICE_FEXT) {
        mount_size = drvSpiFlashCapacity(flash);
    } else {
        mount_size = desc->size;
    }

    blockDevice_t *fbdev = flashBlockDeviceCreateV2(
        flash, desc->offset, mount_size,
        desc->eb_size, desc->pb_size, false);
#else
    blockDevice_t *fbdev = flashBlockDeviceCreateV2(
        flash, desc->offset, desc->size,
        desc->eb_size, desc->pb_size, false);
#endif

    if (fbdev != NULL)
        goto created;

    if (opt == MOUNT_ONLY)
    {
        OSI_LOGE(0x10006ffa, "failed to mount flash block device %4c", desc->name);
        return false;
    }

#ifdef CONFIG_FIBOCOM_EXTFS_MOUNT_SIZE_ADAPTIVE // mount size self-adapting
    if (!flashBlockDeviceFormatV2(
            flash, desc->offset, mount_size,
            desc->eb_size, desc->pb_size))
#else
    if (!flashBlockDeviceFormatV2(
            flash, desc->offset, desc->size,
            desc->eb_size, desc->pb_size))
#endif
    {
        OSI_LOGE(0x10006ffc, "failed to format flash block device %4c", desc->name);
        return false;
    }

#ifdef CONFIG_FIBOCOM_EXTFS_MOUNT_SIZE_ADAPTIVE // mount size self-adapting
    fbdev = flashBlockDeviceCreateV2(
        flash, desc->offset, mount_size,
        desc->eb_size, desc->pb_size, false);
#else
    fbdev = flashBlockDeviceCreateV2(
        flash, desc->offset, desc->size,
        desc->eb_size, desc->pb_size, false);
#endif

    if (fbdev == NULL)
    {
        OSI_LOGE(0x10006ffa, "failed to mount flash block device %4c", desc->name);
        return false;
    }

created:
    if (!prvRegisterBlockDevice(desc->type, desc->name, fbdev))
    {
        blockDeviceDestroy(fbdev);
        return false;
    }
    return true;
}

/**
 * Create sub-partition block device if not created.
 */
static bool prvCreateSubpart(const descSubpart_t *desc)
{
    if (fsFindBlockDeviceByName(desc->name) != NULL)
        return true;

    blockDevice_t *parent = fsFindBlockDeviceByName(desc->device);
    if (parent == NULL)
    {
        OSI_LOGE(0x10006ffe, "failed to find block device %4c", desc->device);
        return false;
    }

    blockDevice_t *bdev = partBlockDeviceCreate(parent, desc->offset, desc->count);
    if (bdev == NULL)
    {
        OSI_LOGE(0x10006fff, "failed to create sub partition %4c", desc->name);
        return false;
    }

    if (!prvRegisterBlockDevice(desc->type, desc->name, bdev))
        return false;

    return true;
}

/**
 * Format a file system
 */
bool fsMountFormat(const char *name)
{
    if (name == NULL || gFsScenario == FS_SCENRARIO_UNKNOWN)
        return false;

    const descSffs_t *fsdesc = prvFindSffsDescByMount(name);
    if (fsdesc == NULL)
        return false;

    vfs_umount(fsdesc->mount); // unmount and ignore return value

    const descFbdev_t *bdesc = prvFindFbdevDescByName(fsdesc->device);
    if (bdesc == NULL)
        return false;

    if (!prvCreateFbdev2(bdesc, MOUNT_FORCE_FORMAT))
        return false;

    blockDevice_t *bdev = fsFindBlockDeviceByName(fsdesc->device);
    return prvMountSffs(fsdesc, bdev, MOUNT_FORCE_FORMAT);
}

bool fsMountFsPath(const char *path, bool format_on_fail)
{
    if (path == NULL || gFsScenario == FS_SCENRARIO_UNKNOWN)
        return false;

    if (vfs_mount_handle(path) != NULL)
        return true;

    const descSffs_t *fsdesc = prvFindSffsDescByMount(path);
    if (fsdesc == NULL)
        return false;

    const descFbdev_t *bdesc = prvFindFbdevDescByName(fsdesc->device);
    if (bdesc == NULL)
        return false;

    // TODO: when sub-partition are used, it is needed to check sub-partition
    unsigned opt = format_on_fail ? MOUNT_FORMAT_ON_FAIL : MOUNT_ONLY;
    if (!prvCreateFbdev2(bdesc, opt))
        return false;

    blockDevice_t *bdev = fsFindBlockDeviceByName(fsdesc->device);
    return prvMountSffs(fsdesc, bdev, opt);
}

/**
 * Format all file systems on flash block device
 */
bool fsMountFormatFlash(unsigned name)
{
    if (gFsScenario == FS_SCENRARIO_UNKNOWN)
        return false;

    const descFbdev_t *fbdesc = prvFindFbdevDescByName(name);
    if (fbdesc == NULL)
    {
        OSI_LOGE(0x10006ffe, "failed to find block device %4c", name);
        return false;
    }

    const descSffs_t *fsdesc = prvFindSffsDescByDeviceName(fbdesc->name);
    if (fsdesc != NULL)
        return fsMountFormat(fsdesc->mount);

    const descSubpart_t *subdescs[SUBPART_MAX_COUNT];
    int subcount = prvFindSubpartDescByParentName(subdescs, fbdesc->name);

    for (int n = 0; n < subcount; n++)
    {
        const descSubpart_t *subdesc = subdescs[n];
        const descSffs_t *subfsdesc = prvFindSffsDescByDeviceName(subdesc->name);
        if (subfsdesc != NULL)
            vfs_umount(subfsdesc->mount);

        blockDevice_t *bdev = fsFindBlockDeviceByName(subdesc->name);
        if (bdev != NULL)
        {
            prvUnregisterBlockDevice(bdev);
            blockDeviceDestroy(bdev);
        }
    }

    if (!prvCreateFbdev2(fbdesc, MOUNT_FORCE_FORMAT))
        return false;

    for (int n = 0; n < subcount; n++)
    {
        const descSubpart_t *subdesc = subdescs[n];
        if (!prvCreateSubpart(subdesc))
            return false;

        blockDevice_t *subbdev = fsFindBlockDeviceByName(subdesc->name);
        const descSffs_t *subfsdesc = prvFindSffsDescByDeviceName(subdesc->name);
        if (subfsdesc == NULL)
            continue;

        if (!prvMountSffs(subfsdesc, subbdev, MOUNT_FORCE_FORMAT))
            return false;
    }
    return true;
}

#define DRV_NAME_FS_EXT OSI_MAKE_TAG('F', 'E', 'X', 'T')     // SFL1 //merge patch by fibocom 20220608


/**
 * Mount all, optionally format on fail.
 */
static OSI_FORCE_INLINE bool prvMountAll(bool format_on_fail)
{
    bool need_mount_ext = false;
#ifdef CONFIG_FS_EXT_ENABLED //merge patch by fibocom 20220608
    need_mount_ext = true;
#endif

    if (gFsScenario == FS_SCENRARIO_UNKNOWN)
        return false;

    uintptr_t ptr = (uintptr_t)gPartInfo;
    descHeader_t *header = (descHeader_t *)OSI_PTR_INCR_POST(ptr, sizeof(descHeader_t));
    if (header->magic != HEADER_MAGIC ||
        header->version != HEADER_VERSION ||
        header->size > PARTINFO_SIZE_MAX)
    {
        OSI_LOGE(0x10006ff6, "invalid partition description header");
        return false;
    }

    if (header->crc != crc32Calc((const char *)gPartInfo + 8, header->size - 8))
    {
        OSI_LOGE(0x10006ff7, "invalid partition description crc");
        return false;
    }

    uintptr_t ptr_end = (uintptr_t)gPartInfo + header->size;
    while (ptr < ptr_end)
    {
        uint32_t type = *(uint32_t *)ptr;
        if (type == TYPE_FBDEV2)
        {
            const descFbdev_t *desc = (const descFbdev_t *)OSI_PTR_INCR_POST(ptr, sizeof(descFbdev_t));
            if (ptr > ptr_end)
            {
                OSI_LOGE(0x10006ff8, "invalid partition description exceed end");
                return false;
            }

            if (prvFlagIgnored(desc->flags) || (desc->name == DRV_NAME_FS_EXT && need_mount_ext == false)) //merge patch by fibocom 20220608
                continue;
#ifdef CONFIG_FIBOCOM_BASE
#ifdef CONFIG_BOARD_WITH_EXT_FLASH
#ifdef CONFIG_FS_EXT_ENABLED
            if ((desc->name == DEVICE_FEXT) && (g_bExtFlashisExist == false))
            {
                g_ucExtFsmountStatus=1;
                OSI_LOGI(0, "prvMountAll: extflash CreateFbdev2: ExtFlash is not exist.");
                continue;
            }
#endif
#endif
#endif

            unsigned opt = MOUNT_ONLY;
            if (format_on_fail || prvFlagFormatOnFail(desc->flags))
                opt = MOUNT_FORMAT_ON_FAIL;

            if (!prvCreateFbdev2(desc, opt))
#ifdef CONFIG_FS_EXT_ENABLED
                {
                    if (desc->name == DEVICE_FEXT)
                    {
                        g_ucExtFsmountStatus=2;
                        OSI_LOGI(0, "prvMountAll: extflash CreateFbdev2: failed.");
                        continue;
                    }
                    else
                    {
                        return false;
                    }
                }
#else
                return false;
#endif
        }
        else if (type == TYPE_SUBPART)
        {
            descSubpart_t *desc = (descSubpart_t *)OSI_PTR_INCR_POST(ptr, sizeof(descSubpart_t));
            if (ptr > ptr_end)
            {
                OSI_LOGE(0x10006ff8, "invalid partition description exceed end");
                return false;
            }

            if (prvFlagIgnored(desc->flags))
                continue;

            if (!prvCreateSubpart(desc))
                return false;
        }
        else if (type == TYPE_SFFS)
        {
            descSffs_t *desc = (descSffs_t *)OSI_PTR_INCR_POST(ptr, sizeof(descSffs_t));
            if (ptr > ptr_end)
            {
                OSI_LOGE(0x10006ff8, "invalid partition description exceed end");
                return false;
            }

            if (prvFlagIgnored(desc->flags) || (desc->device == DRV_NAME_FS_EXT && need_mount_ext == false)) //merge patch by fibocom 20220608
                continue;

#ifdef CONFIG_FIBOCOM_BASE
#ifdef CONFIG_BOARD_WITH_EXT_FLASH
#ifdef CONFIG_FS_EXT_ENABLED
            if ((desc->device == DEVICE_FEXT) && (g_bExtFlashisExist == false))
            {
                OSI_LOGI(0, "prvMountAll: extflash MountSffs: ExtFlash is not exist.");
                continue;
            }

            if ((desc->device == DEVICE_FEXT) && (g_ucExtFsmountStatus == 2))
            {
#if defined(CONFIG_FIBOCOM_HALUO_AT)
                OSI_PRINTFI("prvMountAll: extflash MountSffs: failed.mount:%s", desc->mount);
                memcpy(g_ExtMountName, desc->mount, strlen(desc->mount));
#else
                OSI_LOGI(0, "prvMountAll: extflash MountSffs: failed.");
#endif
                continue;
            }
#endif
#endif
#endif

            if (vfs_mount_handle(desc->mount) != NULL)
                continue;

            blockDevice_t *bdev = fsFindBlockDeviceByName(desc->device);
            if (bdev == NULL)
            {
                OSI_LOGE(0x10006ffe, "failed to find block device %4c", desc->device);
                return false;
            }

            unsigned opt = MOUNT_ONLY;
            if (format_on_fail || prvFlagFormatOnFail(desc->flags))
                opt = MOUNT_FORMAT_ON_FAIL;

            if (!prvMountSffs(desc, bdev, opt))
#ifdef CONFIG_FS_EXT_ENABLED
                {
                    if (desc->device == DEVICE_FEXT)
                    {
                        g_ucExtFsmountStatus=3;
                        OSI_LOGI(0, "prvMountAll: extflash prvMountSffs: failed.");
                        continue;
                    }
                    else
                    {
                        return false;
                    }
                }
                else
                {
                    if (desc->device == DEVICE_FEXT)
                    {
                        g_ucExtFsmountStatus=5;
                        OSI_LOGI(0, "prvMountAll: extflash prvMountSffs: successed.");
                    }
                }
#else
                    return false;
#endif

        }
        else
        {
            OSI_LOGE(0x10007004, "invalid partition description type %4c", type);
            return false;
        }
    }

    return true;
}

void fsMountSetScenario(fsMountScenario_t scenario)
{
    gFsScenario = scenario;
}

bool fsMountAll(void)
{
    return prvMountAll(false);
}

bool fsMountWithFormatAll(void)
{
    return prvMountAll(true);
}

void fsUmountAll(void)
{
    vfs_umount_all();

    while (!SLIST_EMPTY(&gBdevList))
    {
        bdevRegistry_t *desc = SLIST_FIRST(&gBdevList);
        SLIST_REMOVE_HEAD(&gBdevList, iter);

        blockDeviceDestroy(desc->bdev);
        free(desc);
    }
}

void fsRemountFactory(unsigned flags)
{
#ifdef CONFIG_FS_FACTORY_MOUNT_POINT
    if (vfs_remount(CONFIG_FS_FACTORY_MOUNT_POINT, flags) < 0)
    {
        OSI_LOGXE(OSI_LOGPAR_SI, 0x10007005, "failed to remount %s, flags/%d",
                  CONFIG_FS_FACTORY_MOUNT_POINT, flags);
    }
#endif
}
