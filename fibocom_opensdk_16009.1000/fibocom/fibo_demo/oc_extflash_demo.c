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

#include "osi_log.h"
#include "osi_api.h"
#include "fibo_opencpu.h"

static void prvInvokeGlobalCtors(void)
{
    extern void (*__init_array_start[])();
    extern void (*__init_array_end[])();

    size_t count = __init_array_end - __init_array_start;
    for (size_t i = 0; i < count; ++i)
        __init_array_start[i]();
}

static void extflash_thread_entry(void *param)
{
    fibo_taskSleep(5000);
    UINT32 ulId;
    UINT32 ulCapacity;
    if (!fibo_ext_flash_poweron(1, 0, 5))
    {
        fibo_textTrace("fibo_ext_flash_poweron fail");
    }
    fibo_textTrace("fibo_ext_flash 1");
    if (0 != fibo_ext_flash_init(1))
    {
        fibo_textTrace("fibo_ext_flash_init fail");
    }
    fibo_textTrace("fibo_ext_flash 2");
    if (0 != fibo_ext_flash_info(&ulId, &ulCapacity))
    {
        fibo_textTrace("fibo_ext_flash_info fail");
    }
    fibo_textTrace("fibo_ext_flash_info ID=0x%x Cap=0x%x.", ulId, ulCapacity);

    UINT8 *writeBuf = (UINT8 *)malloc(0x10000*(sizeof(UINT8)));
    if (writeBuf == NULL)
    {
        fibo_textTrace("fibo_ext_flash_info malloc fail");
    }
    for (UINT32 i = 0; i < 0x10000; i++)
    {
        writeBuf[i] = 0x55;
    }

    if (0 != fibo_ext_flash_erase(0, ulCapacity))
    {
        fibo_textTrace("fibo_ext_flash_erase fail");
    }

    fibo_taskSleep(2000);

    if (0 != fibo_ext_flash_write(0, writeBuf, 0x10000))
    {
        fibo_textTrace("fibo_ext_flash_write fail");
    }
    else
    {
        fibo_textTrace("fibo_ext_flash_write success");
    }

    while (1)
    {
        fibo_taskSleep(500);
    }
    fibo_textTrace("prvThreadEntry osiThreadExit");
    fibo_thread_delete();

}

int appimg_enter(void *param)
{
    fibo_textTrace("application image enter, param 0x%x", param);

    prvInvokeGlobalCtors();

    fibo_thread_create(extflash_thread_entry,"extflash_custhread", 1024, NULL, OSI_PRIORITY_NORMAL);
    return 0;
}

void appimg_exit(void)
{
    fibo_textTrace("application image exit");
}
