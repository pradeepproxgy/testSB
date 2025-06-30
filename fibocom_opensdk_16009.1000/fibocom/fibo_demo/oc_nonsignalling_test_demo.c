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


#define OSI_LOG_TAG OSI_MAKE_LOG_TAG('T', 'T', 'S', 'Y')

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

static void nonsignalling_test_thread_entry(void *param)
{
    fibo_textTrace("application thread enter, param 0x%x", param);
    CFW_SIM_ID nSimID = 0;
    CFW_NW_NST_TX_PARA_CONFIG nst_para_config;
    memset(&nst_para_config, 0, sizeof(CFW_NW_NST_TX_PARA_CONFIG));
    nst_para_config.band = 900;
    nst_para_config.band_indicator = 100;
    nst_para_config.bw = 0;
    nst_para_config.channel = 100;
    nst_para_config.onoff = 1;
    nst_para_config.pwr = 100;
    nst_para_config.TxRx = 100;
    nst_para_config.TxRxmode = 100;
    fibo_cfun_set(0, nSimID);
    fibo_set_mcal_dev(1,nSimID);
    fibo_set_mcal(&nst_para_config,nSimID);
    fibo_set_mcal_dev(0,nSimID);
    while(1)
    {
       fibo_taskSleep(10*1000);
       fibo_textTrace("non signalling testing");
    }
    fibo_thread_delete();
}

void *appimg_enter(void *param)
{
    fibo_textTrace("application image enter, param 0x%x", param);
    prvInvokeGlobalCtors();
    fibo_thread_create(nonsignalling_test_thread_entry, "mythread", 1024 * 4, NULL, OSI_PRIORITY_NORMAL);
    return 0;
}

void appimg_exit(void)
{
    fibo_textTrace("application image exit");
}
