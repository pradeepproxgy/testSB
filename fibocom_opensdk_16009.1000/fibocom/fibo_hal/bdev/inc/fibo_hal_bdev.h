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

#ifndef __FIBO_HAL_BDEV_H__
#define __FIBO_HAL_BDEV_H__

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>

#define PUBLIC
#define PRIVATE static

enum fibo_hal_bdev_type_e
{
    FIBO_HAL_BDEV_NONE = 0,
    FIBO_HAL_BDEV_SDMMC,
    FIBO_HAL_BDEV_MAX_NUM,
};

typedef enum fibo_hal_bdev_type_e fibo_hal_bdev_type_t;

struct fibo_hal_bdev_s
{
    uint32_t type;
    void *bdev;
};

typedef struct fibo_hal_bdev_s fibo_hal_bdev_t;

PUBLIC fibo_hal_bdev_t *fibo_hal_bdev_create(uint32_t dev_type, ...);

PUBLIC int32_t fibo_hal_bdev_destroy(fibo_hal_bdev_t *);

#endif
