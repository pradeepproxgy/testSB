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


static void pmu_thread_entry(void *param)
{
    fibo_textTrace("application thread enter, param 0x%x", param);

    fibo_pmu_ctrl(0, true, true);  // open VLCD power domain
    fibo_pmu_setlevel(0, 0);  // set VLCD 1.8V

    fibo_pmu_setlevel(0, 1);  // set VLCD 3.2V
    fibo_thread_delete();
}

int appimg_enter(void *param)
{
    fibo_textTrace("application image enter, param 0x%x", param);

    fibo_thread_create(pmu_thread_entry, "pum_custhread", 1024, NULL, OSI_PRIORITY_NORMAL);
    return 0;
}

void appimg_exit(void)
{
    fibo_textTrace("application image exit");
}
