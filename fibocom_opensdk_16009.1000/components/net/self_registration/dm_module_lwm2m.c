/* Copyright (C) 2018 RDA Technologies Limited and/or its affiliates("RDA").
 * All rights reserved.
 *
 * This software is supplied "AS IS" without any warranties.
 * RDA assumes no responsibility or liability for the use of the software,
 * conveys no license or title under any patent, copyright, or mask work
 * right to the product. RDA reserves the right to make changes in the
 * software without notification.  RDA also make no representation or
 * warranty that such application will be suitable for the specified use
 * without further testing or modification.
 */

#include "at_response.h"

#include "nvm.h"
#include "netutils.h"
#include "lwm2m_api.h"
#include "dm_module.h"
#include "dm_endpoint.h"
#include "mipl_config.h"
#include "osi_log.h"
#include "fibocom.h"

static char dm_adapter_config_hex[] = {
    0x13, 0x00, 0x5f,
    0xf1, 0x00, 0x03,
    0xf2, 0x00, 0x48, 0x04, 0x00, 0x11, 0x00, 0x00, 0x05, 0x43,
    0x4d, 0x49, 0x4f, 0x54, 0x00, 0x04, 0x73, 0x64, 0x6b, 0x55, 0x00, 0x04, 0x73, 0x64, 0x6b, 0x50,
    0x00, 0x10, 0x31, 0x31, 0x37, 0x2e, 0x31, 0x36, 0x31, 0x2e, 0x32, 0x2e, 0x37, 0x3a, 0x35, 0x36, 0x38, 0x33, /*length 16 117.161.2.7*/
    0x00, 0x1a, 0x41, 0x75, 0x74, 0x68, 0x43, 0x6f, 0x64, 0x65, 0x3a, 0x3b, 0x55, 0x73,
    0x65, 0x72, 0x64, 0x61, 0x74, 0x61, 0x3a, 0x61, 0x62, 0x63, 0x31, 0x32, 0x33, 0x3b,
    0xf3, 0x00,
    0x11, 0xe4, 0x00, 0xc8, 0x00, 0x09, 0x73, 0x79, 0x73, 0x55, 0x44, 0x3a, 0x31, 0x32, 0x33};

static char dm_commercial_config_hex[] = {
    0x13, 0x00, 0x60,
    0xf1, 0x00, 0x03,
    0xf2, 0x00, 0x49, 0x04, 0x00, 0x11, 0x00, 0x00, 0x05, 0x43,
    0x4d, 0x49, 0x4f, 0x54, 0x00, 0x04, 0x73, 0x64, 0x6b, 0x55, 0x00, 0x04, 0x73, 0x64, 0x6b, 0x50,
    0x00, 0x11, 0x31, 0x31, 0x37, 0x2e, 0x31, 0x36, 0x31, 0x2e, 0x32, 0x2e, 0x34, 0x31, 0x3a, 0x35, 0x36, 0x38, 0x33, /*length 17 117.161.2.41*/
    0x00, 0x1a, 0x41, 0x75, 0x74, 0x68, 0x43, 0x6f, 0x64, 0x65, 0x3a, 0x3b, 0x55, 0x73,
    0x65, 0x72, 0x64, 0x61, 0x74, 0x61, 0x3a, 0x61, 0x62, 0x63, 0x31, 0x32, 0x33, 0x3b,
    0xf3, 0x00,
    0x11, 0xe4, 0x00, 0xc8, 0x00, 0x09, 0x73, 0x79, 0x73, 0x55, 0x44, 0x3a, 0x31, 0x32, 0x33};

#define STR_DM_EVENT_CODE(M) \
    ((M) == LWM2M_EVENT_BOOTSTRAP_START ? "EVENT_BOOTSTRAP_START" : ((M) == LWM2M_EVENT_BOOTSTRAP_SUCCESS ? "EVENT_BOOTSTRAP_SUCCESS" : ((M) == LWM2M_EVENT_BOOTSTRAP_FAILED ? "EVENT_BOOTSTRAP_FAILED" : ((M) == LWM2M_EVENT_CONNECT_SUCCESS ? "EVENT_CONNECT_SUCCESS" : ((M) == LWM2M_EVENT_CONNECT_FAILED ? "EVENT_CONNECT_FAILED" : ((M) == LWM2M_EVENT_REG_SUCCESS ? "EVENT_REG_SUCCESS" : ((M) == LWM2M_EVENT_REG_FAILED ? "EVENT_REG_FAILED" : ((M) == LWM2M_EVENT_REG_TIMEOUT ? "EVENT_REG_TIMEOUT" : ((M) == LWM2M_EVENT_LIFETIME_TIMEOUT ? "EVENT_LIFETIME_TIMEOUT" : ((M) == LWM2M_EVENT_STATUS_HALT ? "EVENT_STATUS_HALT" : ((M) == LWM2M_EVENT_UPDATE_SUCCESS ? "EVENT_UPDATE_SUCCESS" : ((M) == LWM2M_EVENT_UPDATE_FAILED ? "EVENT_UPDATE_FAILED" : ((M) == LWM2M_EVENT_UPDATE_TIMEOUT ? "EVENT_UPDATE_TIMEOUT" : ((M) == LWM2M_EVENT_UPDATE_NEED ? "EVENT_UPDATE_NEED" : "Unknown"))))))))))))))

static bool isAdapterPlatform = true;

static uint8_t g_self_register = 0;

static int8_t gDMConnectRef = -1;

#ifdef CONFIG_FIBOCOM_BASE
#define CHINA_MOBILE_DM_APPKEY "M100000041"
#define CHINA_MOBILE_DM_PASSWD "9buDsA45b5V4D08947F2uJ22oDg4F86M"
#endif

static void dm_server_ind_process(osiEvent_t *pEvent)
{
    uint8_t nSimId = (uint8_t)pEvent->param1;
    uint8_t ref = (uint8_t)pEvent->param2;
    osiEvent_t *lwm2m_ev = (osiEvent_t *)pEvent->param3;

    uint32_t result = lwm2m_ev->param1;
    uint32_t result_param1 = lwm2m_ev->param2;
    uint32_t result_param2 = lwm2m_ev->param3;

    OSI_LOGI(0x100080d1, "DM mipl_server_ind_process nEventId=%d", pEvent->id);
    OSI_LOGI(0x100080d2, "DM mipl_server_ind_process result=%d", result);
    OSI_LOGI(0x100080d3, "DM mipl_server_ind_process result_param1=%d", result_param1);
    OSI_LOGI(0x100080d4, "DM mipl_server_ind_process result_param2=%d", result_param2);

    switch (pEvent->id)
    {
    case EVETN_IND:
    {
        char refstr[50] = {0};

        if (result == LWM2M_EVENT_UPDATE_NEED)
            lwm2m_update(60, false, ref);
        sprintf(refstr, "%s", STR_DM_EVENT_CODE(result));
        atCmdRespSimUrcText(nSimId, refstr);
    }
    break;
    case OBJ_OPERATE_RSP_IND:
    case SERVER_REGCMD_RESULT_IND:
        break;
    default:
    {
        char refstr[50] = {0};
        sprintf(refstr, "+UNPROCESS EVENT:%ld", pEvent->id);
        atCmdRespSimUrcText(nSimId, refstr);
    }
    break;
    }

    free(lwm2m_ev);
    lwm2m_ev = NULL;
    free(pEvent);
}

static uint8_t dm_getIMEI(char *buffer, uint8_t maxlen)
{
    if (maxlen < 16)
        return 0;

    return nvmReadImeiStr(0, (nvmImeiStr_t *)buffer);
}
static uint8_t dm_getIMSI(char *buffer, uint8_t maxlen)
{
    if (maxlen < 16)
        return 0;
    if (!getSimImsi(0, (uint8_t *)buffer, &maxlen))
        return 0;
    return maxlen;
}

void dm_SdkInit(void *DMconfig)
{
    Options *g_opt = (Options *)DMconfig;
    memset(g_opt->szCMEI_IMEI, 0x00, sizeof(g_opt->szCMEI_IMEI));
    dm_getIMEI(g_opt->szCMEI_IMEI, sizeof(g_opt->szCMEI_IMEI));

    memset(g_opt->szIMSI, 0x00, sizeof(g_opt->szIMSI));
    dm_getIMSI(g_opt->szIMSI, sizeof(g_opt->szIMSI));
}

static bool dm_gethostport(char *url, char **host, char **port)
{
    char *p = NULL;
    ip_addr_t addr;

    static char s_host[20] = {0};
    static char s_port[6] = {0};
    memset(s_host, 0, sizeof(s_host));
    memset(s_port, 0, sizeof(s_port));

    if ((p = strstr(url, ":"))) //"183.230.40.39:5683"
    {
        strncpy(s_host, url, p - url);
        strcpy(s_port, p + 1);
    }
    else
    {
        *host = NULL;
        *port = NULL;
        return false;
    }

    if (!ipaddr_aton(s_host, &addr))
    {
        *host = NULL;
        *port = NULL;
        return false;
    }
    if (atoi(s_port) <= 0)
    {
        *host = NULL;
        *port = NULL;
        return false;
    }

    *host = s_host;
    *port = s_port;
    return true;
}

void dm_register(uint8_t nSimId)
{
    int ret;
    bool bRet;
    char cmdline[512] = {0};
    char *host = NULL;
    char *port = NULL;
    char *dm_pwd = NULL;
    char *deviceName = NULL;
    char *dmUpdeviceName = NULL;

    static bool dm_register_once = false;

    OSI_LOGI(0x100080ca, "dm_register");

    if(dm_register_once)
    {
        OSI_LOGE(0x100094d1, "dm had register before");
        return;
    }
    if (!(g_self_register & 0X07))
    {
        OSI_LOGE(0x100094c5, "dm_register DM doesn't register g_self_register = %d", g_self_register);
        return;
    }
    OSI_LOGI(0x100094c6, "dm_register g_self_register = %d", g_self_register);

    mipl_cfg_context_t *DMCfgCtx = malloc(sizeof(mipl_cfg_context_t));
    if (DMCfgCtx == NULL)
        return;

    if (isAdapterPlatform)
        bRet = mipl_config_init(DMCfgCtx, dm_adapter_config_hex, sizeof(dm_adapter_config_hex));
    else
        bRet = mipl_config_init(DMCfgCtx, dm_commercial_config_hex, sizeof(dm_commercial_config_hex));

    if (bRet != true)
    {
        OSI_LOGI(0x100080d5, "dm config init failed.");
        goto EXIT;
    }

    if (dm_gethostport((char *)DMCfgCtx->cfgNet.host.data, &host, &port) != true)
    {
        OSI_LOGXI(OSI_LOGPAR_S, 0x100080d6, "DM Invalid Host:%s", DMCfgCtx->cfgNet.host.data);
        goto EXIT;
    }

    Options dm_config = {0};
    memcpy(dm_config.szCMEI_IMEI, (char *)"CMEI_IMEI", strlen((char *)"CMEI_IMEI"));
    memcpy(dm_config.szIMSI, (char *)"IMSI", strlen((char *)"IMSI"));
    memcpy(dm_config.szDMv, (char *)"v2.0", strlen((char *)"v3.0"));
#ifndef CONFIG_FIBOCOM_BASE
    memcpy(dm_config.szAppKey, (char *)"M100000058", strlen((char *)"M100000058"));
    memcpy(dm_config.szPwd, (char *)"s2n7TgPkg3IT2oo51T630F3f6U5XJ6vj", strlen((char *)"s2n7TgPkg3IT2oo51T630F3f6U5XJ6vj"));

#else
    memcpy(dm_config.szAppKey, (char *)CHINA_MOBILE_DM_APPKEY, strlen((char *)CHINA_MOBILE_DM_APPKEY));
    memcpy(dm_config.szPwd, (char *)CHINA_MOBILE_DM_PASSWD, strlen((char *)CHINA_MOBILE_DM_PASSWD));
#endif
    dm_SdkInit(&dm_config);
    ret = genDmRegEndpointName("L", &deviceName, &dm_config);
    if (ret < 0)
    {
        OSI_LOGI(0x100080d7, "ERROR:Get device name error from IMEI/IMSI.");
        goto EXIT;
    }
    ret = genDmUpdateEndpointName("L", &dmUpdeviceName, &dm_config);
    if (ret < 0)
    {
        OSI_LOGI(0x100080d7, "ERROR:Get device name error from IMEI/IMSI.");
        goto EXIT;
    }

    dm_pwd = malloc(strlen((&dm_config)->szPwd) + 1);
    if (dm_pwd == NULL)
        goto EXIT;
    memset(dm_pwd, 0x00, strlen((&dm_config)->szPwd) + 1);
    memcpy(dm_pwd, (&dm_config)->szPwd, strlen((&dm_config)->szPwd));

    sprintf(cmdline, "-e %d -n %s -D %s -W %s -h %s -p %s -4",
            LWM2M_SERVER_ONENET, deviceName, dmUpdeviceName, dm_pwd, host, port);

    if (DMCfgCtx->cfgNet.bs_enabled == 1)
        strcat(cmdline, " -b");

    OSI_LOGXI(OSI_LOGPAR_S, 0x100080d8, "dm cmdline:%s", cmdline);
    gDMConnectRef = lwm2m_new_config((uint8_t *)cmdline);
    if (gDMConnectRef >= 0)
    {
        OSI_LOGI(0x100080d9, "dm new config success");
    }
    else
    {
        OSI_LOGI(0x100080da, "dm new config failed");
        goto EXIT;
    }

    lwm2mcfg_set_handler_ext(gDMConnectRef, atEngineGetThreadId(), (osiCallback_t)dm_server_ind_process, (void *)(uint32_t)nSimId, 0);

    if (lwm2m_register(gDMConnectRef, 60, 20) == LWM2M_RET_OK)
    {
        OSI_LOGI(0x100080db, "dm lwm2m_register success");
        dm_register_once = true;
    }
    else
    {
        OSI_LOGI(0x100080dc, "dm lwm2m_register failed");
    }

EXIT:
    if (DMCfgCtx != NULL)
        free(DMCfgCtx);

    if (deviceName != NULL)
        free(deviceName);

    if (dmUpdeviceName != NULL)
        free(dmUpdeviceName);

    if (dm_pwd != NULL)
        free(dm_pwd);
}

void  dm_register_init(uint8_t self_register)
{
    OSI_LOGI(0x100094ca, "dm register init:%d", self_register);
    g_self_register = self_register;
}

bool dm_platform_set(void *config)
{
    isAdapterPlatform = ((DM_PLAT_CONFIG_T *)config)->isAdapterPlatform;
    return true;
}

bool dm_platform_get(void *config)
{
    ((DM_PLAT_CONFIG_T *)config)->isAdapterPlatform = isAdapterPlatform;
    return true;
}



