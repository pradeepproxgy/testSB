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

extern void test_printf(void);

static void prvInvokeGlobalCtors(void)
{
    extern void (*__init_array_start[])();
    extern void (*__init_array_end[])();

    size_t count = __init_array_end - __init_array_start;
    for (size_t i = 0; i < count; ++i)
        __init_array_start[i]();
}

static void inner_thread_entry(void *param)
{
    int ret;
    UINT8 data[64] = {0};
    fibo_textTrace("application thread enter, param 0x%x", param);

    fibo_inner_flash_init();

    ret = fibo_inner_flash_read(0x340000-0x1000, data, sizeof(data));
    if (ret == 0)
    {
        fibo_textTrace("read OK : data = %s", data);
    }

    memset(data, 0, sizeof(data));
    strcpy((char*)data, "chaixiaoqiang is OK,d affjksjkfskfsfsdfjk");
    fibo_inner_flash_erase(0x340000-0x1000, 0x1000);
    ret = fibo_inner_flash_write(0x340000-0x1000, data, sizeof(data));
    if (ret == 0)
    {
        fibo_textTrace("write OK : data = %s", data);
    }

    ret = fibo_inner_flash_read(0x340000-0x1000, data, sizeof(data));
    if (ret == 0)
    {
        fibo_textTrace("read OK : data = %s", data);
    }

    fibo_thread_delete();
}

void * appimg_enter(void *param)
{
    fibo_textTrace("application image enter, param 0x%x", param);

    prvInvokeGlobalCtors();

    fibo_thread_create(inner_thread_entry, "inner_custhread", 1024 * 4, NULL, OSI_PRIORITY_NORMAL);
    return 0;
}

void appimg_exit(void)
{
    fibo_textTrace("application image exit");
}
