#include "app_wifiscan.h"
#include "app_pdp.h"

#define MC66X

static UINT32 g_sig_wifi_sem = 0;
static UINT32 g_sig_wifi_lock = 0;
static int g_wifi_status = 0; /// 0:busy 1:idle
static fibo_wifiscan_res_t *g_wifiscan_result = NULL;

static void wifiscan_res_cb(void *param)
{
    fibo_mutex_lock(g_sig_wifi_lock);
    if (param != NULL)
    {
        fibo_wifiscan_res_t *ap_info = (fibo_wifiscan_res_t *)param;
        *g_wifiscan_result = *ap_info;
    }
    else
    {
        APP_LOG_INFO("wifiscan fail");
    }
    g_wifi_status = 0;
    fibo_mutex_unlock(g_sig_wifi_lock);
    fibo_sem_signal(g_sig_wifi_sem);

    return;
}

int app_wifiscan_init()
{
    g_sig_wifi_sem = fibo_sem_new(0);
    if (g_sig_wifi_sem == 0)
    {
        APP_LOG_INFO("sem new fail");
        goto error;
    }

    g_sig_wifi_lock = fibo_mutex_create();
    if (g_sig_wifi_lock == 0)
    {
        APP_LOG_INFO("mutex new fail");
        goto error;
    }

    return 0;

error:
    fibo_mutex_delete(g_sig_wifi_lock);
    g_sig_wifi_lock = 0;
    fibo_sem_free(g_sig_wifi_sem);
    g_sig_wifi_sem = 0;
    return -1;
}

int app_wifiscan(fibo_wifiscan_res_t *ap_info)
{
    int ret = 0;
    int simid = 0;

#ifdef MC66X // 如果是MC66X, wifi和LTE共享射频，需要等待LTE释放RRC，才能使用WFIFSCAN
    app_pdp_wait_rrc_idle();
#endif

    fibo_mutex_lock(g_sig_wifi_lock);
    APP_WAIT_EXPECT_STATUS(g_sig_wifi_lock, g_sig_wifi_sem, g_wifi_status != 0, g_wifi_status);

    g_wifiscan_result = ap_info;
    g_wifi_status = 1;
    ret = fibo_wifiscan_v2(simid, wifiscan_res_cb);
    if (ret != 0)
    {
        g_wifi_status = 0;
    }
    APP_WAIT_EXPECT_STATUS(g_sig_wifi_lock, g_sig_wifi_sem, g_wifi_status != 0, g_wifi_status);
    fibo_mutex_unlock(g_sig_wifi_lock);

    return ret;
}

void app_wifiscan_demo()
{
    fibo_wifiscan_res_t ap_info = {0};
    int ret = app_wifiscan(&ap_info);
    if (ret == 0)
    {
        APP_LOG_INFO("result=%d, ap_info->ap_num=%d", ap_info.result, ap_info.ap_num);
        for (int i = 0; i < ap_info.ap_num; i++)
        {
            APP_LOG_INFO("ap_info[%d].addr=%s,ap_rssi=%d,ssid=%s", i, ap_info.ap_info[i].ap_addr, ap_info.ap_info[i].ap_rssi, ap_info.ap_info[i].ap_name);
        }
    }
}