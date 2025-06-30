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
#include "mutil_app_common.h"

extern void test_printf(void);

static void prvInvokeGlobalCtors(void)
{
    extern void (*__init_array_start[])();
    extern void (*__init_array_end[])();

    size_t count = __init_array_end - __init_array_start;
    for (size_t i = 0; i < count; ++i)
        __init_array_start[i]();
}


INT32 sys_thread_create(void *pvTaskCode, INT8 *pcName, UINT32 usStackDepth, void *pvParameters, UINT32 uxPriority)
{
    fibo_thread_create(pvTaskCode, pcName, usStackDepth, pvParameters, uxPriority);
    return 0;

}
void sys_thread_delete(void)
{
    fibo_thread_delete();
    return;

}
void *sys_malloc(UINT32 size)
{
     return fibo_malloc(size);
}
INT32 sys_free(void *buffer)
{
     fibo_free(buffer);
     return 0 ;
}

void sys_taskSleep(UINT32 msec)
{
     fibo_taskSleep(msec);
     return;
}

INT32 sys_test_201(void)
{
   // do nothing
   fibo_textTrace("hello world,sys_test_201");
   return 0;
}


static APP_SERRICES_T api_for_3rd_app = {

sys_thread_create,
sys_thread_delete,
sys_malloc,
sys_free,
sys_taskSleep,
sys_test_201

};

static void sig_res_callback(GAPP_SIGNAL_ID_T sig, va_list arg)
{
    switch (sig)
    {
        //fibo_PDPActive  ip address resopnse event
        case GAPP_SIG_PDP_ACTIVE_ADDRESS:
        {
            UINT8 cid = (UINT8)va_arg(arg, int);
            char *addr = (char *)va_arg(arg, char *);
            if(addr != NULL)
            {
                fibo_textTrace("sig_res_callback  cid = %d, addr = %s ", cid, addr);
            }
            else 
            {
                fibo_textTrace("[%s-%d]", __FUNCTION__, __LINE__);
            }
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

static FIBO_CALLBACK_T user_callback = {
    .fibo_signal = sig_res_callback,
    .at_resp = at_res_callback};



static THIRD_APP_RECV_SIG_CALLBACK_T *third_app_signal_ind = NULL;

void third_app_dispatch(THIRD_APP_SIGNAL_ID_T sig, ...)
{
    va_list arg;
    if (third_app_signal_ind && third_app_signal_ind->fibo_signal)
    {
        va_start(arg, sig);
        third_app_signal_ind->fibo_signal(sig, arg);
        va_end(arg);
    }
}

static void mutil_app1_thread_entry(void *param)
{
    fibo_textTrace("application thread enter, param 0x%x", param);
    //srand(100);

    for (int n = 0; n < 10; n++)
    {
        fibo_textTrace("hello world %d", n);
        fibo_taskSleep(1000);
    }

    char *pt = (char *)fibo_malloc(512);
    if (pt != NULL)
    {
        fibo_textTrace("malloc address %u", (unsigned int)pt);
        fibo_free(pt);
    }
    //test_printf();
    third_app_signal_ind = (THIRD_APP_RECV_SIG_CALLBACK_T *)fibo_app_start(0x410000,0x50000,0xF40000,0x40000,(void *)&api_for_3rd_app);

    fibo_taskSleep(10000);

    third_app_dispatch(GAPP_SIG_TEST1);
    fibo_taskSleep(20000);
    fibo_thread_delete();
}

void * appimg_enter(void *param)
{
    fibo_textTrace("application image enter, param 0x%x", param);

    prvInvokeGlobalCtors();

    fibo_thread_create(mutil_app1_thread_entry, "mythread", 1024*4, NULL, OSI_PRIORITY_NORMAL);
    return (void *)&user_callback;
}

void appimg_exit(void)
{
    fibo_textTrace("application image exit");
}
