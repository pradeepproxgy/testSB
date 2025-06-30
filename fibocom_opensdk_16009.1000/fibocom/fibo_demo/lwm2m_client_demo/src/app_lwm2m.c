/*
 * Fibocom OpenCPU LWM2M Client Demo
 *
 * This demo illustrates how to implement a simple LWM2M client using the Fibocom OpenCPU platform.
 * It demonstrates initializing the platform, handling AT command responses, managing signal callbacks,
 * and running a basic LWM2M client application.
 *
 * Dependencies:
 * - Fibocom OpenCPU SDK
 * - LWM2M library
 *
 * Note: This code is intended for demonstration purposes and may require adjustments for production use.
 */
#include "osi_api.h"
#include "osi_log.h"
#include "fibo_opencpu.h"
#include "app_lwm2m.h"
#include "app_pdp.h"

extern int app_lwm2m_demo();
static fibo_signal_t g_signal_callback[GAPP_SIG_MAX] = {NULL};

static void at_res_callback(UINT8 *buf, UINT16 len)
{
    APP_LOG_INFO("FIBO <--%s", buf);
}

static void sig_res_callback(GAPP_SIGNAL_ID_T sig, va_list arg)
{
    // Log the received signal
    APP_LOG_INFO("sig_res_callback sig = %d", sig);

    // If a callback function is registered for this signal, call it
    if (sig < GAPP_SIG_MAX && g_signal_callback[sig] != NULL)
    {
        g_signal_callback[sig](sig, arg);
    }
}

int app_register_sig_callback(GAPP_SIGNAL_ID_T sig, fibo_signal_t cb)
{
    // Check if the signal ID is valid and register the callback
    if (sig < GAPP_SIG_MAX)
    {
        g_signal_callback[sig] = cb;
        return 0;
    }
    else
    {
        // Return error if signal ID is invalid
        return -1;
    }
}


static FIBO_CALLBACK_T user_callback = {
    .fibo_signal = sig_res_callback,
    .at_resp = at_res_callback};


static void lwm2m_client_demo()
{
    // Log entry into the application thread
    fibo_textTrace("application thread enter");

    // Initialize PDP context and activate it
    app_pdp_init();
    app_pdp_active(0, CID_DEFAULT, APP_PDP_IPTYPE_IPV4V6, NULL, NULL, NULL, 0);

    // Start the LWM2M client demo
    app_lwm2m_demo();

    // Delete the thread after the demo is complete
    fibo_thread_delete();
}

void *app_image_start(void)
{
    fibo_textTrace("app codes enter");
    // Create a new thread to run the LWM2M client demo
    fibo_thread_create(lwm2m_client_demo, "lwm2m_client_demo", 16*1024, NULL, OSI_PRIORITY_NORMAL);
    // Return a pointer to the structure containing callback functions
    return (void *)&user_callback;
}
