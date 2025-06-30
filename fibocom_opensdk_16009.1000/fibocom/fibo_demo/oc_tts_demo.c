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


#define OSI_LOG_TAG OSI_MAKE_LOG_TAG('T', 'T', 'S', 'Y')

#include "fibo_opencpu.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

extern void test_printf(void);

static void prvInvokeGlobalCtors(void)
{
    extern void (*__init_array_start[])();
    extern void (*__init_array_end[])();

    size_t count = __init_array_end - __init_array_start;
    for (size_t i = 0; i < count; ++i)
        __init_array_start[i]();
}

static void tts_thread_entry(void *param)
{
    fibo_textTrace("application thread enter, param 0x%x", param);

    INT8 *pUtf8 = "E8BF99E9878CE698AF545453E6B58BE8AF95"; /*UTF8 This is the TTS test*/
    INT8 *pGb2312 = "D5E2C0EFCAC7545453B2E2CAD4"; /*GB2312 This is the TTS test*/
    INT8 *pUnicode = "D98FCC912F665400540053004B6DD58B"; /* UNICODE This is the TTS test */

    fibo_taskSleep(5000);
    if(GAPP_TTS_SUCCESS == fibo_tts_start((const UINT8 *)pUtf8, CTTS_STRING_ENCODE_TYPE_UTF8))
    {
        fibo_textTrace("TTS UTF8 play success");
    }
    else
    {
        fibo_textTrace("TTS UTF8 play error");
    }

    while(fibo_tts_is_playing())
    {
        fibo_textTrace("TTS wait play!");
        fibo_taskSleep(1000);
    }

    if(GAPP_TTS_SUCCESS == fibo_tts_start((const UINT8 *)pGb2312, CTTS_STRING_ENCODE_TYPE_GB2312))
    {
        fibo_textTrace("TTS GB2312 play success");
    }
    else
    {
        fibo_textTrace("TTS GB2312 play error");
    }

    while(fibo_tts_is_playing())
    {
        fibo_textTrace("TTS wait play!");
        fibo_taskSleep(1000);
    }

    if(GAPP_TTS_SUCCESS == fibo_tts_start((const UINT8 *)pUnicode, CTTS_STRING_ENCODE_TYPE_UNICODE))
    {
        fibo_textTrace("TTS UNICODE play success");
    }
    else
    {
        fibo_textTrace("TTS UNICODE play error");
    }

    fibo_thread_delete();
}
static void sig_res_callback(GAPP_SIGNAL_ID_T sig, va_list arg)
{
    switch (sig)
    {
    case GAPP_SIG_TTS_END:
    {
        fibo_textTrace("ttsfinish", 0);
    }
    default:
    {
        break;
    }
    }
    fibo_textTrace("test %d",sig);
}
static FIBO_CALLBACK_T user_callback = {
    .fibo_signal = sig_res_callback};

void *appimg_enter(void *param)
{
    fibo_textTrace("application image enter, param 0x%x", param);
    prvInvokeGlobalCtors();
    fibo_thread_create(tts_thread_entry, "mythread", 1024 * 4, NULL, OSI_PRIORITY_NORMAL);
    return (void *)&user_callback;
}

void appimg_exit(void)
{
    fibo_textTrace("application image exit");
}
