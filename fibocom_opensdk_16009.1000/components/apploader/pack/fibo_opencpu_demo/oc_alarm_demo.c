/* Copyright (C) 2018 FIBOCOM Technologies Limited and/or its affiliates("FIBOCOM").
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

static void hello_world_demo(void *param)
{
    char *a = "14:08:00";
    char *b = "2,4,6,";
    //char *b = "0";

    //char *a = "22/09/27,10:25:44";
    //char *b = "EMPTY";
    fibo_textTrace("application thread enter, param 0x%x", param);

    fibo_taskSleep(2000);

    int ret= fibo_set_alarm(a,1,b,0);
    //int ret = fibo_del_alarm(1);
    fibo_textTrace("alarm ret =%d",ret);
}

int appimg_enter(void *param)
{
    fibo_textTrace("application image enter, param 0x%x", param);

    prvInvokeGlobalCtors();

    //fibo_thread_create("mythread", hello_world_demo, NULL, OSI_PRIORITY_NORMAL, 1024, 0);
    fibo_thread_create(hello_world_demo, "mythread", 1024 * 4, NULL, OSI_PRIORITY_NORMAL);
    return 0;
}

void appimg_exit(void)
{
    fibo_textTrace("application image exit");
}
