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


#define OSI_LOG_TAG OSI_MAKE_LOG_TAG('F', 'I', 'B', 'O')

#include "osi_api.h"
#include "osi_log.h"
#include "osi_pipe.h"
#include "at_engine.h"
#include <string.h>
#include "fibo_opencpu.h"
static UINT32 g_pdp_active_sem = 0;

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
        case GAPP_SIG_PDP_ACTIVE_IND:
        {
            UINT8 cid = (UINT8)va_arg(arg, int);
            fibo_textTrace("sig_res_callback  cid = %d", cid);
            va_end(arg);
            if(g_pdp_active_sem)
            {
                fibo_sem_signal(g_pdp_active_sem);
                fibo_textTrace("[%s-%d]", __FUNCTION__, __LINE__);
            }
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

static void lbs_thread_entry(void *param)
{
    int ret = -1;
    UINT8 ip[50];
    memset(&ip, 0, sizeof(ip));
    reg_info_t reg_info;
    fibo_pdp_profile_t pdp_profile;
    UINT8 cid_status;
    fibo_lbs_res_t lbs_info;
    fibo_textTrace("[%s-%d]", __FUNCTION__, __LINE__);

    while(1)
    {
        fibo_reg_info_get(&reg_info, 0);
        if(reg_info.nStatus==1)
        {
            fibo_textTrace("[%s-%d] register network", __FUNCTION__, __LINE__);
            break;
        }
        fibo_taskSleep(1000);
    }
    memset(&pdp_profile, 0, sizeof(fibo_pdp_profile_t));
    pdp_profile.cid = 1;
    ret = fibo_pdp_active(1, &pdp_profile, 0);
    g_pdp_active_sem = fibo_sem_new(0);
    fibo_sem_wait(g_pdp_active_sem);
    fibo_sem_free(g_pdp_active_sem);

    if (0 == fibo_pdp_status_get(pdp_profile.cid, ip, &cid_status, 0))
    {
        fibo_textTrace("[%s-%d] ip = %s,cid_status=%d", __FUNCTION__, __LINE__, ip,cid_status);
    }
    while(1)
    {
        lbs_info.loc = malloc(LBS_LOC_MAX_LEN+1);
        lbs_info.desc = malloc(LBS_DESC_MAX_LEN+1);
        lbs_info.raw = malloc(LBS_RAWDATA_MAX_LEN+1);

        ret=fibo_lbs_get_gisinfo_v2(NULL, 6, 0, &lbs_info, 60*1000);
        fibo_textTrace("[%s-%d] ret=%d", __FUNCTION__, __LINE__, ret);

        if(0 == ret)
        {
            fibo_textTrace("[%s-%d] %s,%s", __FUNCTION__, __LINE__, lbs_info.loc, lbs_info.desc);
            fibo_textTrace("[%s-%d] %s", __FUNCTION__, __LINE__, lbs_info.raw);
        }
        free(lbs_info.loc);
        free(lbs_info.desc);
        free(lbs_info.raw);
        fibo_taskSleep(10 * 1000);
    }
    fibo_thread_delete();
}

static FIBO_CALLBACK_T user_callback = {
    .fibo_signal = sig_res_callback,
    .at_resp = at_res_callback};

void *appimg_enter(void *param)
{
    fibo_textTrace("application image enter, param 0x%x", param);
    //init
    fibo_thread_create(lbs_thread_entry, "lbs_demo", 1024*8*5, NULL, OSI_PRIORITY_NORMAL);
    return (void *)&user_callback;
}

void appimg_exit(void)
{
    fibo_textTrace("application image exit");
}

