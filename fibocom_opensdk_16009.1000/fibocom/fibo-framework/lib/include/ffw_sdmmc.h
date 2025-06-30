#ifndef LIB_INCLUDE_FFW_SDMMC
#define LIB_INCLUDE_FFW_SDMMC

/**
* @file ffw_sdmmc.h
* @author sundaqing (sundaqing@fibocom.com)
* @brief sdmmc相关接口
* @version 0.1
* @date 2021-05-21
* 
* Copyright (c) 2021 Fibocom. All rights reserved
* 
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <ffw_plat_header.h>

/**
* @brief 创建设备系统资源
* 
* @param n 
* @return int32_t 
* @retval >= 0: 成功
* @retval < 0: 失败
 */
int32_t ffw_sdmmc_create(uint32_t n);

int32_t ffw_sdmmc_destroy(uint32_t n);

int32_t ffw_sdmmc_open(uint32_t n);

int32_t ffw_sdmmc_close(uint32_t n);

int32_t ffw_sdmmc_write(uint32_t n, uint32_t block_number, const void *buffer, uint32_t size);

int32_t ffw_sdmmc_read(uint32_t n, uint32_t block_number, void *buffer, uint32_t size);

uint32_t ffw_sdmmc_get_block_count(uint32_t n);

uint32_t ffw_sdmmc_get_block_size(uint32_t n);

uint32_t ffw_sdmmc_get_slot_num(void);


#endif /* LIB_INCLUDE_FFW_FS */