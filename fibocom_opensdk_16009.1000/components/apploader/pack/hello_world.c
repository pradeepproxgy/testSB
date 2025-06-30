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

#ifdef CONFIG_SUPPORT_ALIPAY 
#include "alipay_macro_def.h"
#include "alipay_iot_coll_biz.h"
#include "alipay_iot_signer.h"
#include "ant_log.h"
#include "alipay_iot_sdk.h"
#include "ant_os.h"
#include "ant_event.h"
#include "ant_clib.h"
#include "ant_mem.h"

#define ALI_SDK_QUEUE_L1 (10)
#define ALI_SDK_QUEUE_L2 (20)
#endif

static void prvInvokeGlobalCtors(void)
{
    extern void (*__init_array_start[])();
    extern void (*__init_array_end[])();

    size_t count = __init_array_end - __init_array_start;
    for (size_t i = 0; i < count; ++i)
        __init_array_start[i]();
}

#ifdef CONFIG_SUPPORT_ALIPAY 
static void fibo_alipay_sdk_create(int priority)
{
    int ret = 0;

    fibo_textTrace("%s",__func__);

    ret = alipay_iot_coll_cache_open(DATA_QUEUE_TYPE_MDAP, ALI_SDK_QUEUE_L1, ALI_SDK_QUEUE_L2);
    fibo_textTrace("alipay_iot_coll_cache_open ret = %d", ret);
}
#endif

static void hello_world_demo(void *param)
{
    fibo_textTrace("application thread enter, param 0x%x", param);

#ifdef CONFIG_SUPPORT_ALIPAY 
    fibo_alipay_sdk_create(OSI_PRIORITY_NORMAL);
#endif

    for (int n = 0; n < 10; n++)
    {
        fibo_textTrace("hello world %d", n);
        fibo_taskSleep(500);
    }

    fibo_thread_delete();
}

int appimg_enter(void *param)
{
    fibo_textTrace("application image enter, param 0x%x", param);

    prvInvokeGlobalCtors();

    fibo_thread_create(hello_world_demo, "mythread", 1024, NULL, OSI_PRIORITY_NORMAL);
    return 0;
}

void appimg_exit(void)
{
    fibo_textTrace("application image exit");
}
