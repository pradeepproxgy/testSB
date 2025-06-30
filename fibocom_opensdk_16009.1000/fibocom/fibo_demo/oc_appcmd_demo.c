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

void fibo_app_handle_at_callback(UINT8*data, UINT16 len)
{
    fibo_textTrace("cmd_buf:%s, %d", (char*)data, len);
    fibo_app_report_default_urc((const char*)data, len);
}

static void at_res_callback(UINT8 *buf, UINT16 len)
{
    fibo_textTrace("FIBO <--%s", buf);
}

static void fibo_app_thread_handle(void *param)
{
    fibo_app_handle_at_init(fibo_app_handle_at_callback);
    fibo_thread_delete();
}

static FIBO_CALLBACK_T user_callback = {
    .at_resp = at_res_callback};

void *appimg_enter(void *param)
{
    fibo_textTrace("application image enter, param 0x%x", param);
    //init
    fibo_thread_create(fibo_app_thread_handle, "app_handle_at_cus_thread", 1024, NULL, OSI_PRIORITY_NORMAL);
    return (void *)&user_callback;
}

void appimg_exit(void)
{
    fibo_textTrace(0, "application image exit");
}


