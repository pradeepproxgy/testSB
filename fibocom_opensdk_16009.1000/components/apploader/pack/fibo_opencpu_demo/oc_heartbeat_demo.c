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
#include "oc_gprs.h"

static void heartbeat_thread_entry(void *param)
{
    fibo_textTrace("application thread enter, param 0x%x", param);
    reg_info_t reg_info;
    while (1)
    {
        fibo_reg_info_get(&reg_info, 0);
        fibo_taskSleep(1000);
        fibo_textTrace("[fibocom %s-%d]", __FUNCTION__, __LINE__);
        if (reg_info.nStatus == 1)
        {
            fibo_pdp_active(1, NULL, 0);
            fibo_taskSleep(1000);
            fibo_textTrace("[fibocom %s-%d]", __FUNCTION__, __LINE__);
            break;
        }
    }

    fibo_enable_heartbeat(0, 0, 0, 0);
    fibo_heartbeat_param_set("111.231.250.105", 20000, 5, "31313131");
    fibo_enable_heartbeat(0, 0, 1, 0);
    osiThreadExit();
}

void *appimg_enter(void *param)
{
    fibo_textTrace("application image enter, param 0x%x", param);
    fibo_thread_create(heartbeat_thread_entry, "mythread", 1024 * 4, NULL, OSI_PRIORITY_NORMAL);
    return (void *)0;
}


void appimg_exit(void)
{
    fibo_textTrace("application image exit");
}
