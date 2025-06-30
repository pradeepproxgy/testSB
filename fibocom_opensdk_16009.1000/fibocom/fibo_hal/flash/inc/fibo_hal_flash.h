/*
 * =====================================================================================
 *
 *       Filename:  fibo_hal_flash.h
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

#ifndef __FIBO_HAL_FLASH_H__
#define __FIBO_HAL_FLASH_H__

#include "osi_log.h"

#define PUBLIC
#define PRIVATE static

#define TAG    "fibo_hal_flash"
#define FIBO_HAL_FLASH_LOG(format, ...) OSI_PRINTFI("[%s:%s:%d] " format, TAG, __FUNCTION__, __LINE__, ##__VA_ARGS__)


#endif

