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

static void sig_res_callback(GAPP_SIGNAL_ID_T sig, va_list arg)
{
    UINT8 sim  = 0;
    switch (sig)
    {
        case GAPP_SIG_CC_RING_IND:
        sim = (UINT8)va_arg(arg, int);
        char *num = (char *)va_arg(arg, char *);
        fibo_textTrace("sig_res_callback  sim = %d, num = %s ", sim, num);
        fibo_voice_answer_call(0);
        va_end(arg);
        break;

        case GAPP_SIG_CC_CONNECT_IND:
        fibo_textTrace("sig_res_callback GAPP_SIG_CC_CONNECT_IND sim = %d", sim);
        break;

        case GAPP_SIG_CC_ERROR_IND:
        fibo_textTrace("sig_res_callback GAPP_SIG_CC_ERROR_IND sim = %d", sim);
        break;

        case GAPP_SIG_CC_NOCARRIER_IND:
        fibo_textTrace("sig_res_callback GAPP_SIG_CC_NOCARRIER_IND sim = %d", sim);
        break;
        default:
            break;
    }
}

#if 1
static void make_call_demo(void *param)
{
    fibo_textTrace("application thread enter, param 0x%x", param);
    while(1)
    {
      fibo_taskSleep(1000);
      if(fibo_is_ims_register(0))
      {
          fibo_textTrace("[%s-%d] fibo_is_ims_register", __FUNCTION__, __LINE__);
          fibo_taskSleep(1000);
          break;
      }
    }

    char *num = "13572257661";
    fibo_voice_start_call(num,0);

    fibo_taskSleep(20000);

    while(1)
    {
        fibo_taskSleep(1000);
    }

    fibo_thread_delete();
}
#endif

static void answer_call_demo(void *param)
{
    fibo_textTrace("application thread enter, param 0x%x", param);
    while(1)
    {
      fibo_taskSleep(1000);
      if(fibo_is_ims_register(0))
      {
          fibo_textTrace("[%s-%d] fibo_is_ims_register", __FUNCTION__, __LINE__);
          fibo_taskSleep(1000);
          //break;
      }
    }


    fibo_thread_delete();
}

static void at_res_callback(UINT8 *buf, UINT16 len)
{
    fibo_textTrace("FIBO <--%s", buf);
}

static FIBO_CALLBACK_T user_callback = {
    .fibo_signal = sig_res_callback,
    .at_resp = at_res_callback};

int appimg_enter(void *param)
{
    fibo_textTrace("application image enter, param 0x%x", param);

    fibo_thread_create(make_call_demo, "mythread", 1024*8*5, NULL, OSI_PRIORITY_NORMAL);
    return (void *)&user_callback;
}

void appimg_exit(void)
{
    fibo_textTrace("application image exit");
}
