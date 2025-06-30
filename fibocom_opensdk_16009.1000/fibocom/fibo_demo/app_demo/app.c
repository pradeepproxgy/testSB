#include "app.h"
#include "app_image.h"
#include "app_pdp.h"
#include "app_http.h"
#include "app_ftp.h"
#include "app_mqtt.h"
#include "app_socket.h"
#include "app_wifiscan.h"

static fibo_signal_t g_signal_callback[GAPP_SIG_MAX] = {NULL};

static void at_res_callback(UINT8 *buf, UINT16 len)
{
    APP_LOG_INFO("FIBO <--%s", buf);
}

static void sig_res_callback(GAPP_SIGNAL_ID_T sig, va_list arg)
{
    APP_LOG_INFO("sig_res_callback sig = %d", sig);

    if (sig < GAPP_SIG_MAX && g_signal_callback[sig] != NULL)
    {
        g_signal_callback[sig](sig, arg);
    }
}

static FIBO_CALLBACK_T user_callback = {
    .fibo_signal = sig_res_callback,
    .at_resp = at_res_callback};

static int init()
{
    app_pdp_init();

    app_http_init();

    app_ftp_init();

    app_mqtt_init();

    app_socket_init();

    app_wifiscan_init();

    return 0;
}

static void app_enter_task(void (*cb)())
{
    APP_LOG_INFO("application task enter");

    set_app_ver(app_ver);
    
    init();

    cb();
}

FIBO_CALLBACK_T *app_init(void (*cb)())
{
    APP_LOG_INFO("app image init");
    fibo_thread_create(app_enter_task, "apptask", 10 * 1024, cb, OSI_PRIORITY_NORMAL);
    return &user_callback;
}

int app_register_sig_callback(GAPP_SIGNAL_ID_T sig, fibo_signal_t cb)
{
    if (sig < GAPP_SIG_MAX)
    {
        g_signal_callback[sig] = cb;
        return 0;
    }
    else
    {
        return -1;
    }
}