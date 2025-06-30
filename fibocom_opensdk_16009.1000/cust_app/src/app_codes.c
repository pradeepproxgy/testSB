#include "osi_api.h"
#include "osi_log.h"
#include "fibo_opencpu.h"
#include "didi_app.h"

#ifdef CONFIG_FIBOCOM_DIDI
void *at_main_uart_dispatch=NULL;
void *at_usb_dispatch=NULL;
#endif

static void sig_res_callback(GAPP_SIGNAL_ID_T sig, va_list arg)
{
    fibo_textTrace("sig_res_callback  sig = %d", sig);
}

static void at_res_callback(UINT8 *buf, UINT16 len)
{
    fibo_textTrace("FIBO <--%s", buf);
}

static FIBO_CALLBACK_T user_callback = {
    .fibo_signal = sig_res_callback,
    .at_resp = at_res_callback};

static void hello_world_demo()
{
    fibo_textTrace("application thread enter");

    for (int n = 0; n < 10; n++)
    {
        fibo_textTrace("app image hello world %d", n);
        fibo_taskSleep(500);
    }

    fibo_thread_delete();
}

void *app_image_start(void)
{
    fibo_textTrace("app codes enter");
    fibo_thread_create(hello_world_demo, "mythread", 10*1024, NULL, OSI_PRIORITY_NORMAL);
    return (void *)&user_callback;
}

