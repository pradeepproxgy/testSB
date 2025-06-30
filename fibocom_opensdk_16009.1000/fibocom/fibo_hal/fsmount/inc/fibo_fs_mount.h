/*
 * =====================================================================================
 *
 *       Filename:  fibo_hal_sdmmc.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  12/01/22 16:00:16
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *        Company:  
 *
 * =====================================================================================
 */

#ifndef __FIBO_FS_MOUNT_H__
#define __FIBO_FS_MOUNT_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "../../bdev/inc/fibo_hal_bdev.h"

#define PUBLIC
#define PRIVATE static

PUBLIC int32_t fibo_fs_mount(const char *base_path, fibo_hal_bdev_t *fibo_dev);

PUBLIC int32_t fibo_fs_umount(const char *base_path);

PUBLIC int32_t fibo_fs_format(const char *base_path, fibo_hal_bdev_t *fibo_dev);

#endif
