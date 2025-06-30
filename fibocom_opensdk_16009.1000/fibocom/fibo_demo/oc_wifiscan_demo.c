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

UINT8 rrc_status = 0;

static void sig_res_callback(GAPP_SIGNAL_ID_T sig, va_list arg)
{
    switch (sig)
    {
        case GAPP_SIG_PDP_ACTIVE_IND:
        {
            UINT8 cid = (UINT8)va_arg(arg, int);
            fibo_textTrace("sig_res_callback  cid = %d", cid);
            va_end(arg);
        }
        break;

        case GAPP_SIG_PDP_RELEASE_IND:
        {
            UINT8 cid = (UINT8)va_arg(arg, int);
            fibo_textTrace("sig_res_callback  cid = %d", cid);
            va_end(arg);
        }
        break;
        case GAPP_SIG_PDP_ACTIVE_OR_DEACTIVE_FAIL_IND:
        {
            UINT8 cid = (UINT8)va_arg(arg, int);
            UINT8 state = (UINT8)va_arg(arg, int);
            fibo_textTrace("[%s-%d]cid = %d,state = %d", __FUNCTION__, __LINE__,cid,state);
            va_end(arg);
        }
        break;
        case GAPP_SIG_PDP_DEACTIVE_ABNORMALLY_IND:
        {
            UINT8 cid = (UINT8)va_arg(arg, int);
            fibo_textTrace("[%s-%d]cid = %d", __FUNCTION__, __LINE__,cid);
            va_end(arg);
        }
        break;
        case GAPP_SIG_RRC_STATUS_REPORT:
        {
            UINT8 conn = (UINT8)va_arg(arg, int);
            UINT8 state = (UINT8)va_arg(arg, int);
            UINT8 access = (UINT8)va_arg(arg, int);
            UINT8 sim = (UINT8)va_arg(arg, int);
            fibo_textTrace("[%s-%d]conn = %d,state=%d,access=%d,sim=%d", __FUNCTION__, __LINE__,conn,state,access,sim);
            va_end(arg);
            rrc_status = conn;
        }
        break;
        default:
            break;
    }
}

void wifiscan_refponse_cb(void *param)
{
    if(param != NULL)
    {
        fibo_wifiscan_res_t *ap_info = (fibo_wifiscan_res_t *)param;
        fibo_textTrace("[%s-%d] result=%d, ap_info->ap_num=%d", __FUNCTION__, __LINE__,ap_info->result ,ap_info->ap_num);
        for(int i = 0; i < ap_info->ap_num; i++)
        {
            fibo_textTrace("[%s-%d] ap_info[%d].addr=%s,ap_rssi=%d,ssid=%s", __FUNCTION__, __LINE__, i, ap_info->ap_info[i].ap_addr,ap_info->ap_info[i].ap_rssi,ap_info->ap_info[i].ap_name);
        }
    }
    else 
    {
        fibo_textTrace("[%s-%d],wifiscan fail", __FUNCTION__, __LINE__);
    }

    return;
}

static void at_res_callback(UINT8 *buf, UINT16 len)
{
    fibo_textTrace("FIBO <--%s", buf);
}

FIBO_CALLBACK_T user_callback = {
    .fibo_signal = sig_res_callback,
    .at_resp = at_res_callback};

void wifiscan_cus(void *p)
{
    fibo_textTrace("[%s-%d]", __FUNCTION__, __LINE__);
    if(!rrc_status)
        fibo_wifiscan_v2(0,wifiscan_refponse_cb);
}
static void wifiscan_thread_entry(void *param)
{
    fibo_textTrace("[%s-%d] param 0x%x",__FUNCTION__, __LINE__, param);
    fibo_taskSleep(10*1000);
    void * timer = fibo_timer_create(wifiscan_cus,NULL,0);
    if(timer != NULL)
    {
        fibo_timer_start(timer,20*1000,true);
    }
    while(1)
    {
        fibo_taskSleep(10000);
        fibo_textTrace("hello world");
    }
    fibo_thread_delete();
}

void * appimg_enter(void *param)
{
    fibo_textTrace("application image enter, param 0x%x", param);

    prvInvokeGlobalCtors();

    fibo_thread_create(wifiscan_thread_entry, "wifiscan_demo", 1024*4, NULL, OSI_PRIORITY_NORMAL);
    return (void *)&user_callback;
}

void appimg_exit(void)
{
    fibo_textTrace("application image exit");
}
