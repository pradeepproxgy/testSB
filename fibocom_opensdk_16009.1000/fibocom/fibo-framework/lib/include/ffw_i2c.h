#ifndef FFW_HAL_I2C_H
#define FFW_HAL_I2C_H
#include "drv_i2c.h"
typedef int INT32;
typedef unsigned char UINT8;
typedef unsigned int UINT32;

typedef struct
{
   int i2c_name;
   int value;
}HAL_I2C_BPS_T;

INT32 ffw_i2c_open(HAL_I2C_BPS_T bps);
INT32 ffw_i2c_close(void);
INT32 ffw_i2c_send_data(UINT32 slaveAddr, UINT32 memAddr, bool is16bit, UINT8 *pData, UINT8 datalen);
INT32 ffw_i2c_get_data(UINT32 slaveAddr, UINT32 memAddr, bool is16bit, UINT8 *pData, UINT8 datalen);
INT32 ffw_hal_i2c_open(HAL_I2C_BPS_T bps, i2c_Handle *i2c_handle);
INT32 ffw_hal_i2c_close(i2c_Handle *i2c_handle);
INT32 ffw_hal_i2c_Write(i2c_Handle i2c_handle, drvI2cSlave_t idAddress, UINT8 *Buffer, UINT8 bufferlen);
INT32 ffw_hal_i2c_Read(i2c_Handle i2c_handle, drvI2cSlave_t idAddress, UINT8 *Buffer, UINT8 bufferlen);
INT32 ffw_hal_i2c_xfer(i2c_Handle i2c_handle, drvI2cSlave_t idAddress, UINT8 *send_Buffer, UINT8 send_bufferlen, UINT8 *recv_Buffer, UINT8 recv_bufferlen);

#endif
