/* Copyright (C) 2022 FIBOCOM Technologies Limited and/or its affiliates("FIBOCOM").
 * All rights reserved.
 *
 * This software is supplied "AS IS" without any warranties.
 * FIBOCOM assumes no responsibility or liability for the use of the software,
 * conveys no license or title under any patent, copyright, or mask work
 * right to the product. FIBOCOM reserves the right to make changes in the
 * software without notification.  FIBOCOM also make no representation or
 * warranty that such application will be suitable for the specified use
 * without further testing or modification.
 */

#define OSI_LOG_TAG OSI_MAKE_LOG_TAG('M', 'Y', 'A', 'P')

#include "fibo_opencpu.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

static HAL_I2C_BPS_T bsp1_1 = {2, 1};
UINT8 data[6];
UINT8 data1[2] = {0x21, 0x30};
i2c_Handle i2c_handle = 0;
int result = 0;
drvI2cSlave_t drv_i2c= {0x7E, 0, 0, true, true};
UINT8 data2[1] ={0x00};

static void i2c_thread_entry(void *param)
{
    fibo_textTrace("application thread enter, param 0x%x", param);

    fibo_gpio_mode_set(56, 0);  // mc669-i2c2
    fibo_gpio_mode_set(57, 0);  // mc669-i2c2
    fibo_textTrace("application thread enter, param 0x%x", param);

    result = fibo_i2c_open(bsp1_1, &i2c_handle);
    fibo_textTrace("i2c handle is %p", i2c_handle);
    fibo_textTrace("test_i2c open result %d", result);
    fibo_taskSleep(1000);

    result = fibo_i2c_Write(i2c_handle, drv_i2c, data1, 2);
    fibo_textTrace("test_i2c i2c_send_data result = %d", result);
    fibo_textTrace("test_i2c i2c_send_data data = %d, %d", data1[0], data1[1]);

    while(1)
    {
        fibo_taskSleep(1000);
        result = fibo_i2c_xfer(i2c_handle, drv_i2c, data, 6, data2, 1);
        fibo_textTrace("test_i2c i2c_get_data data = %d, %d", data[0], data[1]);
    }

    fibo_thread_delete();
}

void * appimg_enter(void *param)
{
    fibo_textTrace("application image enter, param 0x%x", param);

    fibo_thread_create(i2c_thread_entry, "i2c_custhread", 1024*4, NULL, OSI_PRIORITY_NORMAL);
    return 0;
}

void appimg_exit(void)
{
    fibo_textTrace("application image exit");
}

