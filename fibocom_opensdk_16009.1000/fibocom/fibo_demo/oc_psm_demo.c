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

static void oc_psm_demo(void *param)
{
    fibo_textTrace("application thread enter, param 0x%x", param);
    fibo_taskSleep(5000);
    if(fibo_psm_mode(1,"01000001","00000001") == true)
    {
         fibo_textTrace("set psm suc");
        fibo_thread_delete();
    }
}

static void sig_res_callback(GAPP_SIGNAL_ID_T sig, va_list arg)
{
    switch (sig)
    {
        case GAPP_SIG_PSM_STATUS_IND:
        {
            UINT8 psm_status = (UINT8)va_arg(arg, int);
            fibo_textTrace("[%s-%d]psm_status = %d,0 is psm in,1 is psm out", __FUNCTION__, __LINE__,psm_status);
            va_end(arg);
        }
        break;
        default:
            break;
    }
}

static void at_res_callback(UINT8 *buf, UINT16 len)
{
    fibo_textTrace("FIBO <--%s", buf);
}

FIBO_CALLBACK_T user_callback = {
    .fibo_signal = sig_res_callback,
    .at_resp = at_res_callback};

int appimg_enter(void *param)
{
    fibo_textTrace("application image enter, param 0x%x", param);

    prvInvokeGlobalCtors();

    osiThreadCreate("mythread", oc_psm_demo, NULL, OSI_PRIORITY_NORMAL, 1024, 0);
    return 0;
}

void appimg_exit(void)
{
    fibo_textTrace("application image exit");
}

