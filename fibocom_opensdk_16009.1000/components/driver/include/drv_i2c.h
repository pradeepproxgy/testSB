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

#ifndef _DRV_I2C_H_
#define _DRV_I2C_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "fibocom.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief i2c bps enumeration
 */
typedef enum
{
    DRV_I2C_BPS_100K, ///< normal 100Kbps
    DRV_I2C_BPS_400K, ///< fast 400Kbps
    DRV_I2C_BPS_3P5M, ///< high speed 3.5Mbps
} drvI2cBps_t;

/**
 * @brief i2c slave definition
 */
typedef struct
{
    uint8_t addr_device;   ///< device address
    uint8_t addr_data;     ///< data high 8 bits
    uint8_t addr_data_low; ///< data low 8 bits
    bool reg_16bit;        ///< true:16bit reg address;false:8bit reg address
#ifdef CONFIG_FIBOCOM_BASE
    bool is_use;            // reg address or not
    bool restart_no_stop;   // restart without stop condition after sending reg address to read data, default as false
#endif
} drvI2cSlave_t;

struct drvI2cMaster;
#ifdef CONFIG_FIBOCOM_BASE
typedef uint32_t i2c_Handle;
#endif
/**
 * @brief the i2c master indicator
 */
typedef struct drvI2cMaster drvI2cMaster_t;

/**
 * @brief acquire the i2c master
 *
 * @param name      name of the i2c master
 * @param bps       the i2c speed
 * @return
 *      - (NULL)    fail
 *      - otherwise the i2c master instance
 */
drvI2cMaster_t *drvI2cMasterAcquire(uint32_t name, drvI2cBps_t bps);

/**
 * @brief release the i2c master
 *
 * @param i2c   the i2c master
 */
void drvI2cMasterRelease(drvI2cMaster_t *i2c);

/**
 * @brief i2c master send data
 *
 * @param i2c           the i2c master
 * @param slave         the i2c slave
 * @param data          data to send
 * @param length        data length
 * @return
 *      - true          success
 *      - false         fail
 */
bool drvI2cWrite(drvI2cMaster_t *i2c, const drvI2cSlave_t *slave, const uint8_t *data, uint32_t length);

/**
 * @brief i2c master get data
 *
 * @param i2c           the i2c master
 * @param slave         the i2c slave
 * @param buf           buffer to receive data
 * @param length        buffer length
 * @return
 *      - true          success
 *      - false         fail
 */
bool drvI2cRead(drvI2cMaster_t *i2c, const drvI2cSlave_t *slave, uint8_t *buf, uint32_t length);

#ifdef CONFIG_FIBOCOM_BASE
/**
 * @brief i2c master get data
 *
 * @param i2c           the i2c master
 * @param slave         the i2c slave
 * @param sbuf          buffer to send data
 * @param slength       send buffer length
 * @param rbuf          buffer to receive data
 * @param rlength       receive buffer length
 * @return
 *      - true          success
 *      - false         fail
 */
bool drvI2cRead_Ex(drvI2cMaster_t *d, const drvI2cSlave_t *slave, uint8_t *sbuf, uint32_t slength,uint8_t *rbuf, uint32_t rlength);
#endif

/**
 * @brief i2c master send raw byte
 *
 * @param i2c           the i2c master
 * @param data          raw byte to send
 * @param cmd_mask      command associated with this byte.
 * @return
 *      - true          success
 *      - false         fail
 */
bool drvI2cWriteRawByte(drvI2cMaster_t *i2c, uint8_t data, uint32_t cmd_mask);

/**
 * @brief i2c master read raw byte
 *
 * @param i2c           the i2c master
 * @param data          room for the byte
 * @param cmd_mask      the command mask required for the final phase of the
 *                      SCCB read cycle.
 * @return
 *      - true          success
 *      - false         fail
 */
bool drvI2cReadRawByte(drvI2cMaster_t *i2c, uint8_t *data, uint32_t cmd_mask);

#ifndef CONFIG_FIBOCOM_BASE
/**
 * @brief i2c1 soft reset
 *
 */
void _i2c1SoftReset(void);

/**
 * @brief i2c1 soft reset
 *
 */
void _i2c2SoftReset(void);

/**
 * @brief i2c1 soft reset
 *
 */
#endif
void _i2c3SoftReset(void);

#ifdef __cplusplus
}
#endif

#endif /*_DRV_I2C_H_*/
