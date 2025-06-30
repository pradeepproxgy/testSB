#include "app_pdp.h"
#include <sys/types.h>

#define APP_MAX_CID (6)

typedef enum
{
    APP_PDP_STATUS_DEACTIVED,
    APP_PDP_STATUS_ACTIVED,
    APP_PDP_STATUS_ACTIVING,
    APP_PDP_STATUS_DEACTIVING,
    APP_PDP_STATUS_ACTIVED_BUT_DEATTACHED
} app_pdp_status_t;

typedef struct
{
    UINT32 sem;
    int cid;
    int iptype;
    app_pdp_status_t status;
    char ipv4[32];
    char ipv6[32];
} app_pdp_info_t;

static UINT32 g_sig_reg_sem = 0;
static UINT32 g_sig_reg_lock = 0;
static app_pdp_info_t g_app_pdp_info[APP_MAX_CID] = {0};
static const char *g_app_iptypestr[] = {"", "IP", "IPV6", "IPV4V6"};

static void pdp_sig_res_ok_callback(GAPP_SIGNAL_ID_T sig, va_list arg)
{
    UINT8 cid;
    cid = (UINT8)va_arg(arg, int);
    va_end(arg);
    APP_LOG_INFO("GAPP_SIG_PDP_ACTIVE_IND cid=%d", cid);

    fibo_mutex_lock(g_sig_reg_lock);
    if (cid < APP_MAX_CID)
    {
        if (g_app_pdp_info[cid].status == APP_PDP_STATUS_ACTIVING)
        {
            // g_app_pdp_info[cid].status = APP_PDP_STATUS_ACTIVED;
        }
        else
        {
            APP_LOG_INFO("GAPP_SIG_PDP_RELEASE_IND invalid status g_app_pdp_info[%d].status=%d", cid, g_app_pdp_info[cid].status);
        }
    }
    fibo_mutex_unlock(g_sig_reg_lock);
}

static void pdp_sig_active_callback(GAPP_SIGNAL_ID_T sig, va_list arg)
{
    UINT8 cid;
    cid = (UINT8)va_arg(arg, int);
    char *addr = (char *)va_arg(arg, char *);
    va_end(arg);
    APP_LOG_INFO("sig_res_callback  cid = %d, addr = %s ", cid, addr);

    fibo_mutex_lock(g_sig_reg_lock);
    if (cid < APP_MAX_CID)
    {
        g_app_pdp_info[cid].status = APP_PDP_STATUS_ACTIVED;
        char *spiltchar = strstr(addr, ",");
        if (spiltchar == NULL)
        {
            if (strstr(addr, ":") != NULL) /// IPV6
            {
                strcpy(g_app_pdp_info[cid].ipv6, addr);
            }
            else
            {
                strcpy(g_app_pdp_info[cid].ipv4, addr);
            }
        }
        else
        {
            spiltchar[0] = 0;
            strcpy(g_app_pdp_info[cid].ipv4, addr);
            strcpy(g_app_pdp_info[cid].ipv6, spiltchar + 1);
        }

        APP_LOG_INFO("ipv4=%s ipv6=%s", g_app_pdp_info[cid].ipv4, g_app_pdp_info[cid].ipv6);
    }
    fibo_mutex_unlock(g_sig_reg_lock);
}

static void pdp_sig_release_callback(GAPP_SIGNAL_ID_T sig, va_list arg)
{
    UINT8 cid;
    cid = (UINT8)va_arg(arg, int);
    va_end(arg);

    fibo_mutex_lock(g_sig_reg_lock);
    APP_LOG_INFO("GAPP_SIG_PDP_DEACTIVE_ABNORMALLY_IND cid=%d", cid);
    if (cid < APP_MAX_CID)
    {
        if (g_app_pdp_info[cid].status != APP_PDP_STATUS_DEACTIVED)
        {
            g_app_pdp_info[cid].status = APP_PDP_STATUS_DEACTIVED;
            fibo_sem_signal(g_app_pdp_info[cid].sem);
        }
        else
        {
            APP_LOG_INFO("GAPP_SIG_PDP_DEACTIVE_ABNORMALLY_IND invalid status g_app_pdp_info[%d].status=%d", cid, g_app_pdp_info[cid].status);
        }
    }
    fibo_mutex_unlock(g_sig_reg_lock);
}

static void pdp_sig_deactive_callback(GAPP_SIGNAL_ID_T sig, va_list arg)
{
    UINT8 cid;
    cid = (UINT8)va_arg(arg, int);
    va_end(arg);

    fibo_mutex_lock(g_sig_reg_lock);
    APP_LOG_INFO("GAPP_SIG_PDP_RELEASE_IND cid=%d", cid);
    if (cid < APP_MAX_CID)
    {
        if (g_app_pdp_info[cid].status == APP_PDP_STATUS_DEACTIVING)
        {
            g_app_pdp_info[cid].status = APP_PDP_STATUS_DEACTIVED;
            fibo_sem_signal(g_app_pdp_info[cid].sem);
        }
        else
        {
            APP_LOG_INFO("GAPP_SIG_PDP_RELEASE_IND invalid status g_app_pdp_info[%d].status=%d", cid, g_app_pdp_info[cid].status);
        }
    }
    fibo_mutex_unlock(g_sig_reg_lock);
}

static void pdp_sig_active_or_deactive_fail_callback(GAPP_SIGNAL_ID_T sig, va_list arg)
{
    UINT8 cid;
    cid = (UINT8)va_arg(arg, int);
    va_end(arg);

    fibo_mutex_lock(g_sig_reg_lock);
    APP_LOG_INFO("GAPP_SIG_PDP_ACTIVE_OR_DEACTIVE_FAIL_IND cid=%d", cid);
    if (cid < APP_MAX_CID)
    {
        if (g_app_pdp_info[cid].status == APP_PDP_STATUS_ACTIVING)
        {
            g_app_pdp_info[cid].status = APP_PDP_STATUS_DEACTIVED;
            fibo_sem_signal(g_app_pdp_info[cid].sem);
        }
        else if (g_app_pdp_info[cid].status == APP_PDP_STATUS_DEACTIVING)
        {
            g_app_pdp_info[cid].status = APP_PDP_STATUS_ACTIVED;
            fibo_sem_signal(g_app_pdp_info[cid].sem);
        }
        else
        {
            APP_LOG_INFO("GAPP_SIG_PDP_RELEASE_IND invalid status g_app_pdp_info[%d].status=%d", cid, g_app_pdp_info[cid].status);
        }
    }
    fibo_mutex_unlock(g_sig_reg_lock);
}

static bool g_network_status = false; // false: 没驻网 true： 驻网成功
static void pdp_sig_res_reg_callback(GAPP_SIGNAL_ID_T sig, va_list arg)
{
    fibo_mutex_lock(g_sig_reg_lock);
    UINT8 status = (UINT8)va_arg(arg, int);
    UINT32 cellid = (UINT32)va_arg(arg, int);
    UINT16 tac = (UINT16)va_arg(arg, int);
    UINT8 access = (UINT8)va_arg(arg, int);
    APP_LOG_INFO("status = %d,cellid=%x,tac=%x,access=%d", status, cellid, tac, access);
    va_end(arg);

    g_network_status = status;
    if (!g_network_status)
    {
        for (int i = 0; i < APP_MAX_CID; i++)
        {
            if (g_app_pdp_info[i].status == APP_PDP_STATUS_ACTIVED)
            {
                g_app_pdp_info[i].status = APP_PDP_STATUS_ACTIVED_BUT_DEATTACHED;
                fibo_sem_signal(g_app_pdp_info[i].sem);
            }
        }
    }

    fibo_mutex_unlock(g_sig_reg_lock);
    fibo_sem_signal(g_sig_reg_sem);
}

static bool g_rrc_status = false; /// false: rrc断开 true: rrc连接
static void pdp_sig_rrc_report_callback(GAPP_SIGNAL_ID_T sig, va_list arg)
{
    fibo_mutex_lock(g_sig_reg_lock);
    UINT8 conn = (UINT8)va_arg(arg, int);
    UINT8 state = (UINT8)va_arg(arg, int);
    UINT8 access = (UINT8)va_arg(arg, int);
    UINT8 sim = (UINT8)va_arg(arg, int);
    APP_LOG_INFO("conn = %d,state=%d,access=%d,sim=%d", conn, state, access, sim);
    va_end(arg);
    g_rrc_status = conn;
    fibo_mutex_unlock(g_sig_reg_lock);
    fibo_sem_signal(g_sig_reg_sem);
}

int app_pdp_init()
{
    for (int i = 0; i < APP_MAX_CID; i++)
    {
        g_app_pdp_info[i].sem = fibo_sem_new(0);
        if (g_app_pdp_info[i].sem == 0)
        {
            APP_LOG_INFO("sem new fail");
            goto error;
        }
    }

    g_sig_reg_lock = fibo_mutex_create();
    if (g_sig_reg_lock == 0)
    {
        APP_LOG_INFO("mutex new fail");
        goto error;
    }

    g_sig_reg_sem = fibo_sem_new(0);
    if (g_sig_reg_sem == 0)
    {
        APP_LOG_INFO("sem new fail");
        goto error;
    }

    app_register_sig_callback(GAPP_SIG_PDP_ACTIVE_IND, pdp_sig_res_ok_callback);
    app_register_sig_callback(GAPP_SIG_PDP_ACTIVE_ADDRESS, pdp_sig_active_callback);
    app_register_sig_callback(GAPP_SIG_PDP_RELEASE_IND, pdp_sig_deactive_callback);
    app_register_sig_callback(GAPP_SIG_PDP_ACTIVE_OR_DEACTIVE_FAIL_IND, pdp_sig_active_or_deactive_fail_callback);
    app_register_sig_callback(GAPP_SIG_PDP_DEACTIVE_ABNORMALLY_IND, pdp_sig_release_callback);
    app_register_sig_callback(GAPP_SIG_REG_STATUS_IND, pdp_sig_res_reg_callback);
    app_register_sig_callback(GAPP_SIG_RRC_STATUS_REPORT, pdp_sig_rrc_report_callback);

    return 0;

error:
    for (int i = 0; i < APP_MAX_CID; i++)
    {
        if (g_app_pdp_info[i].sem != 0)
        {
            fibo_sem_free(g_app_pdp_info[i].sem);
            g_app_pdp_info[i].sem = 0;
        }
    }
    fibo_sem_free(g_sig_reg_sem);
    g_sig_reg_sem = 0;
    fibo_mutex_delete(g_sig_reg_lock);
    g_sig_reg_lock = 0;

    return -1;
}

int app_pdp_is_active(int cid)
{
    if (cid >= APP_MAX_CID)
    {
        APP_LOG_INFO("invlid cid %d", cid);
        return -1;
    }
    fibo_mutex_lock(g_sig_reg_lock);
    int ret = g_app_pdp_info[cid].status == APP_PDP_STATUS_ACTIVED;
    fibo_mutex_unlock(g_sig_reg_lock);
    return ret;
}

int app_pdp_get_ipaddress(int cid, char **ipv4, char **ipv6)
{
    if (cid >= APP_MAX_CID)
    {
        APP_LOG_INFO("invlid cid %d", cid);
        return -1;
    }

    fibo_mutex_lock(g_sig_reg_lock);
    if (g_app_pdp_info[cid].status != APP_PDP_STATUS_ACTIVED)
    {
        APP_LOG_INFO("cid %d not active", cid);
        fibo_mutex_unlock(g_sig_reg_lock);
        return -1;
    }

    if (ipv4)
    {
        *ipv4 = g_app_pdp_info[cid].ipv4;
    }
    if (ipv6)
    {
        *ipv6 = g_app_pdp_info[cid].ipv6;
    }
    fibo_mutex_unlock(g_sig_reg_lock);
    return 0;
}

int app_pdp_active(int simid, int cid, app_pdp_iptype_t iptype, char *apn, char *apnuser, char *apnpasswd, app_pdp_authproto_t auth_proto)
{
    int ret = 0;
    fibo_pdp_profile_t tmpprofile = {0};
    fibo_pdp_profile_t *pdp_profile = &tmpprofile;

    if (cid >= APP_MAX_CID)
    {
        APP_LOG_INFO("invlid cid %d", cid);
        return 0;
    }

    fibo_mutex_lock(g_sig_reg_lock);
    if (g_app_pdp_info[cid].status == APP_PDP_STATUS_ACTIVED)
    {
        fibo_mutex_unlock(g_sig_reg_lock);
        APP_LOG_INFO("pdp %d have actived", cid);
        return 0;
    }

    if (cid == 1) /// 如果cid为1，则使用默认承载
    {
        if (apn) /// 改变默认承载配置
        {
            APP_LOG_INFO("set default apn simid=%d apn=%s will cfun0 cfun1", simid, apn);

            /// 设置默认承载的配置
            fibo_initial_pdp_set(1, iptype, apn, auth_proto, apnuser, apnpasswd, simid);

            /// 对于LTE来说，要生效，必须重新驻网
            g_network_status = false;
            fibo_cfun_set(0, simid); /// cfun 0
            fibo_cfun_set(1, simid); /// cfun 1
        }
        pdp_profile = NULL;
    }
    else
    {
        pdp_profile->cid = cid;
        strcpy((char *)pdp_profile->nPdpType, g_app_iptypestr[iptype]);
        if (apn)
        {
            strcpy((char *)pdp_profile->apn, apn);
        }

        if (apnuser && apnpasswd)
        {
            strcpy((char *)pdp_profile->apnUser, apnuser);
            strcpy((char *)pdp_profile->apnPwd, apnpasswd);
            pdp_profile->nAuthProt = auth_proto;
        }
    }

    APP_WAIT_EXPECT_STATUS(g_sig_reg_lock, g_sig_reg_sem, !g_network_status, g_network_status);
    APP_LOG_INFO("network register ok");

    ret = fibo_pdp_active(1, pdp_profile, simid);
    if (ret == 0)
    {

        g_app_pdp_info[cid].iptype = iptype;
        g_app_pdp_info[cid].status = APP_PDP_STATUS_ACTIVING;
        APP_WAIT_EXPECT_STATUS(g_sig_reg_lock, g_app_pdp_info[cid].sem, g_app_pdp_info[cid].status == APP_PDP_STATUS_ACTIVING, g_app_pdp_info[cid].status);
    }

    ret = (g_app_pdp_info[cid].status == APP_PDP_STATUS_ACTIVED) ? 0 : -1;
    fibo_mutex_unlock(g_sig_reg_lock);

    return ret;
}

int app_pdp_deactive(int cid)
{
    int simid = 0;
    if (cid >= APP_MAX_CID)
    {
        APP_LOG_INFO("invlid cid %d", cid);
        return -1;
    }

    fibo_mutex_lock(g_sig_reg_lock);

    if (g_app_pdp_info[cid].status != APP_PDP_STATUS_ACTIVED && g_app_pdp_info[cid].status != APP_PDP_STATUS_ACTIVED_BUT_DEATTACHED)
    {
        APP_LOG_INFO("deactive cid%d fail", cid);
        fibo_mutex_unlock(g_sig_reg_lock);
        return -1;
    }

    int ret = fibo_pdp_release(0, cid, simid);
    if (ret != 0)
    {
        APP_LOG_INFO("deactive cid%d fail", cid);
        fibo_mutex_unlock(g_sig_reg_lock);
        return -1;
    }
    g_app_pdp_info[cid].status = APP_PDP_STATUS_DEACTIVING;
    APP_WAIT_EXPECT_STATUS(g_sig_reg_lock, g_app_pdp_info[cid].sem, g_app_pdp_info[cid].status == APP_PDP_STATUS_DEACTIVING, g_app_pdp_info[cid].status);
    ret = (g_app_pdp_info[cid].status == APP_PDP_STATUS_DEACTIVED) ? 0 : -1;
    fibo_mutex_unlock(g_sig_reg_lock);

    return ret;
}

int app_pdp_get_rrc_status()
{
    return g_rrc_status;
}

int app_pdp_wait_rrc_idle()
{
    fibo_mutex_lock(g_sig_reg_lock);
    APP_WAIT_EXPECT_STATUS(g_sig_reg_lock, g_sig_reg_sem, g_rrc_status, g_rrc_status);
    fibo_mutex_unlock(g_sig_reg_lock);
    return g_rrc_status;
}

int app_pdp_check_pdp_release(int cid)
{
    int ret = 0;
    if (cid >= APP_MAX_CID)
    {
        APP_LOG_INFO("invlid cid %d", cid);
        return -1;
    }

    fibo_mutex_lock(g_sig_reg_lock);
    APP_WAIT_EXPECT_STATUS_TIMEOUT(g_sig_reg_lock, g_app_pdp_info[cid].sem, (g_app_pdp_info[cid].status != APP_PDP_STATUS_DEACTIVED) && (g_app_pdp_info[cid].status != APP_PDP_STATUS_ACTIVED_BUT_DEATTACHED), g_app_pdp_info[cid].status, 10 * 1000);

    if (g_app_pdp_info[cid].status == APP_PDP_STATUS_ACTIVED_BUT_DEATTACHED)
    {
        APP_LOG_INFO("deattached release pdp cid%d", cid);
        fibo_mutex_unlock(g_sig_reg_lock);
        app_pdp_deactive(cid);
        fibo_mutex_lock(g_sig_reg_lock);
    }
    fibo_mutex_unlock(g_sig_reg_lock);
    return ret; /// pdp release
}
