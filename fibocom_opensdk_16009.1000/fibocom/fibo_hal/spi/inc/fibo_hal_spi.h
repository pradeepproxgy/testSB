/*
 * =====================================================================================
 *
 *       Filename:  fibo_hal_spi.h
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

#ifndef __FIBO_HAL_SPI_H__
#define __FIBO_HAL_SPI_H__

#include "osi_log.h"
#include "drv_spi.h"
#include "fibo_opencpu_comm.h"
#define PUBLIC
#define PRIVATE static

#define TAG    "fibo_hal_spi"
#define FIBO_HAL_SPI_LOG(format, ...) OSI_PRINTFI("[%s:%s:%d] " format, TAG, __FUNCTION__, __LINE__, ##__VA_ARGS__)

INT32 fibo_hal_spi_open(drvSpiConfig_t *cfg, SPIHANDLE *spiFd);
INT32 fibo_hal_spi_close(SPIHANDLE *spiFd);
INT32 fibo_hal_spi_send(SPIHANDLE spiFd, SPI_IOC spiIoc, void *sendaddr, uint32_t size);
INT32 fibo_hal_spi_recv(SPIHANDLE spiFd, SPI_IOC spiIoc, void *readaddr, uint32_t size);
INT32 fibo_hal_spi_send_rcv(SPIHANDLE spiFd, SPI_IOC spiIoc, void *sendaddr, void *readaddr, uint32_t size);
INT32 fibo_hal_spi_pinctrl(SPIHANDLE spiFd, drvSpiPinCtrl pinctrl, drvSpiPinState pinstate);

#endif
