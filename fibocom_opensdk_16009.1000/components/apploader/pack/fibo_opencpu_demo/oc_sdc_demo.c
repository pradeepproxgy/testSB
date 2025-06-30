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
#include "oc_adc.h"
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

static void sdc_thread_entry(void *param)
{
    bool ret = false;
    INT8 ret1 = 0;
    int n = 0;
    unsigned long total = 0, used = 0, rest = 0;
    fibo_textTrace("application thread enter, param 0x%x", param);

    ret = fibo_mount_sdc(0, "/cemmc");
    if(!ret)
    {
        ret = fibo_format_sdc(0);
        fibo_textTrace("fibo_format_sdc ,ret=%d", ret);
        ret = fibo_mount_sdc(0, "/cemmc");
        fibo_textTrace("fibo_mount_sdc,ret=%d", ret);
    }
    ret1 = fibo_show_sdcard_space(&total, &used, &rest);
    fibo_textTrace("fibo_show_sdcard_space ,ret=%d %lu %lu %lu", ret1, total, used, rest);
    fibo_umount_sdc("/cemmc");

    fibo_thread_delete();
}

int appimg_enter(void *param)
{
    fibo_textTrace("application image enter, param 0x%x", param);

    prvInvokeGlobalCtors();

    fibo_thread_create(sdc_thread_entry, "sdc_custhread", 1024*10, NULL, OSI_PRIORITY_NORMAL);
    return 0;
}

void appimg_exit(void)
{
    fibo_textTrace("application image exit");
}
