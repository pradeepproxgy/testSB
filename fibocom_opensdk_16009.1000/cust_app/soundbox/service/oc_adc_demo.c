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

static void prvInvokeGlobalCtors(void)
{
    extern void (*__init_array_start[])();
    extern void (*__init_array_end[])();

    size_t count = __init_array_end - __init_array_start;
    for (size_t i = 0; i < count; ++i)
        __init_array_start[i]();
}

static void adc_thread_entry(void *param)
{
    fibo_textTrace("application thread enter, param 0x%x", param);
    INT32 ret = 0;
    UINT32 data = 0;
    hal_adc_channel_t channel = 0;
    UINT32 scale = 3;

    fibo_hal_adc_init();

//    fibo_textTrace("test fibo_hal_adc_get_data channel %d", channel);
   while(1){
    fibo_hal_adc_get_data_polling(channel, &data);
    fibo_textTrace("test fibo_hal_adc_get_data_polling data=%d", data);
    fibo_taskSleep(1000);
   }
    fibo_thread_delete();
}
void adc_interface()
{

//    prvInvokeGlobalCtors();
   fibo_thread_create(adc_thread_entry, "adc_custhread", 1024*10, NULL, OSI_PRIORITY_NORMAL);
    return 0;
}

/*void appimg_exit(void)
{
    fibo_textTrace("application image exit");
}*/
