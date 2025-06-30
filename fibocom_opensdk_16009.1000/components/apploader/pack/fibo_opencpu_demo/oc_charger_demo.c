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

void drvChargerNoticeCBProc(UINT32 msg)
{
    switch (msg)
    {
        case FIBO_CHARGE_START_IND:
            fibo_textTrace("test FIBO_CHARGE_START_IND msg= %x", msg);
        break;
        case FIBO_CHARGE_END_IND:
            fibo_textTrace("test FIBO_CHARGE_END_IND msg= %x", msg);
        break;
        case FIBO_CHARGE_WARNING_IND:
            fibo_textTrace("test FIBO_CHARGE_WARNING_IND msg= %x", msg);
        break;
        case FIBO_CHARGE_SHUTDOWN_IND:
            fibo_textTrace("test FIBO_CHARGE_SHUTDOWN_IND msg= %x", msg);
        break;
        case FIBO_CHARGE_CHARGE_FINISH:
            fibo_textTrace("test FIBO_CHARGE_CHARGE_FINISH msg= %x", msg);
        break;
        case FIBO_CHARGE_CHARGE_DISCONNECT:
            fibo_textTrace("test FIBO_CHARGE_CHARGE_DISCONNECT msg= %x", msg);
        break;
        default :
            fibo_textTrace("test unknown msg= %x", msg);
        break;
    }
}

static void charger_thread_entry(void *param)
{
    INT32 ret = -1;
    INT32 i = 0;
    fibo_textTrace("test application thread enter, param 0x%x", param);

    fibo_textTrace("test sleep mode start,wait resigter 1min");
    fibo_taskSleep(10000);

    ret = fibo_charge_vbat_vol_get();
    fibo_textTrace("test fibo_charge_vbat_vol_get ret= %d", ret);
    fibo_taskSleep(3000);
    ret = fibo_charge_vbat_vol_percent_get();
    fibo_textTrace("test fibo_charge_vbat_vol_percent_get ret= %d", ret);
    fibo_taskSleep(3000);
    ret = fibo_charge_current_get();
    fibo_textTrace("test fibo_charge_current_get ret= %d", ret);
    fibo_taskSleep(3000);
    {
        fiboChargeParams_t test_conf = {4200, 1000, 4000, 3400, 3600};
        bool result = false;
        result = fibo_charge_config_set(&test_conf);
        fibo_textTrace("test fibo_charge_config_set ret= %d", result);
        fibo_charge_notice_cb_set(drvChargerNoticeCBProc);
        for(i = 0; i < 1000; i++)
        {
            ret = fibo_charge_is_full();
            fibo_textTrace("test fibo_charge_is_full ret= %d", ret);
            fibo_taskSleep(2000);
            ret = fibo_charge_vbat_vol_get();
            fibo_textTrace("test fibo_charge_vbat_vol_get ret= %d", ret);
            fibo_taskSleep(2000);
            ret = fibo_charge_current_get();
            fibo_textTrace("test fibo_charge_current_get ret= %d", ret);
        }
        fibo_taskSleep(120000);
    }
    fibo_thread_delete();
}

int appimg_enter(void *param)
{
    fibo_textTrace("application image enter, param 0x%x", param);

    prvInvokeGlobalCtors();

    fibo_thread_create(charger_thread_entry, "charger_custhread", 1024, NULL, OSI_PRIORITY_NORMAL);
    return 0;
}

void appimg_exit(void)
{
    fibo_textTrace("application image exit");
}
