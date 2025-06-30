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
#include "vfs.h"
#include "netutils.h"
#include "lwm2m_api.h"
#include "dm_module.h"
#include "dm_endpoint.h"
#include "osi_log.h"

#define Default_Server_Domain "m.fxltsbl.com"
#define Default_Serevr_IP "117.161.2.41"
#define Default_Server_Port "5683"
#define COAP_Max_Transmit_Wait 46

static uint8_t dm_cus_cfg_file[] = "/dm_cus.nv";

static DM_CUS_CONFIG_T gDmCusCfg = {
        .DM_mode = false
    };

#define RES_DM_DEV_INFO                  6601
#define RES_DM_APP_INFO                  6602
#define RES_DM_MAC                       6603
#define RES_DM_ROM                       6604
#define RES_DM_RAM                       6605
#define RES_DM_CPU                       6606
#define RES_DM_SYS_VERSION               6607
#define RES_DM_FIRM_VERSION              6608
#define RES_DM_FIRM_NAME                 6609
#define RES_DM_VOLTE                     6610
#define RES_DM_NET_TYPE                  6611
#define RES_DM_NET_ACCT                  6612
#define RES_DM_PHONE                     6613
#define RES_DM_LOCATION                  6614

static uint32_t gDMCusResID[] = {RES_DM_DEV_INFO, \
                            RES_DM_APP_INFO, \
                            RES_DM_MAC, \
                            RES_DM_ROM, \
                            RES_DM_RAM, \
                            RES_DM_CPU, \
                            RES_DM_SYS_VERSION, \
                            RES_DM_FIRM_VERSION, \
                            RES_DM_FIRM_NAME, \
                            RES_DM_VOLTE, \
                            RES_DM_NET_TYPE, \
                            RES_DM_NET_ACCT, \
                            RES_DM_PHONE, \
                            RES_DM_LOCATION};

static DM_CUS_CONFIG_EX_T gDmCusCfgEX = {
    .APP_info = {'N','U','L','L'},
    .MAC = {'N','U','L','L'},
    .ROM = {'N','U','L','L'},
    .RAM = {'N','U','L','L'},
    .CPU = {'N','U','L','L'},
    .osSysVer = {'N','U','L','L'},
    .swVer = {'N','U','L','L'},
    .swname = {'N','U','L','L'},
    .VoLTE = {'N','U','L','L'},
    .net_type = {'N','U','L','L'},
    .account = {'N','U','L','L'},
    .phoneNum = {'N','U','L','L'},
    .location = {'N','U','L','L'}
};

static uint8_t dm_cus_cfg_ex_file[] = "/dm_cus_ex.nv";

static DM_CUS_STATE_E gDmCusState = DM_CUS_STATE_INITIAL;
static osiWork_t* gDmCusWork = NULL;

#define STR_DM_EVENT_CODE(M) \
    ((M) == LWM2M_EVENT_BOOTSTRAP_START ? "EVENT_BOOTSTRAP_START" : ((M) == LWM2M_EVENT_BOOTSTRAP_SUCCESS ? "EVENT_BOOTSTRAP_SUCCESS" : ((M) == LWM2M_EVENT_BOOTSTRAP_FAILED ? "EVENT_BOOTSTRAP_FAILED" : ((M) == LWM2M_EVENT_CONNECT_SUCCESS ? "EVENT_CONNECT_SUCCESS" : ((M) == LWM2M_EVENT_CONNECT_FAILED ? "EVENT_CONNECT_FAILED" : ((M) == LWM2M_EVENT_REG_SUCCESS ? "EVENT_REG_SUCCESS" : ((M) == LWM2M_EVENT_REG_FAILED ? "EVENT_REG_FAILED" : ((M) == LWM2M_EVENT_REG_TIMEOUT ? "EVENT_REG_TIMEOUT" : ((M) == LWM2M_EVENT_LIFETIME_TIMEOUT ? "EVENT_LIFETIME_TIMEOUT" : ((M) == LWM2M_EVENT_STATUS_HALT ? "EVENT_STATUS_HALT" : ((M) == LWM2M_EVENT_UPDATE_SUCCESS ? "EVENT_UPDATE_SUCCESS" : ((M) == LWM2M_EVENT_UPDATE_FAILED ? "EVENT_UPDATE_FAILED" : ((M) == LWM2M_EVENT_UPDATE_TIMEOUT ? "EVENT_UPDATE_TIMEOUT" : ((M) == LWM2M_EVENT_UPDATE_NEED ? "EVENT_UPDATE_NEED" : "Unknown"))))))))))))))


static int8_t gDMCusConnectRef = -1;

#define DM_CUS_URC_ENABLE 0

static bool at_write_to_file(uint8_t *filename, void *contextData, uint32_t contextLen)
{
    int fd = -1;

    OSI_LOGXI(OSI_LOGPAR_SI, 0X0, "enter at_write_to_file. file: %s, contextLen:%d", filename, contextLen);
    
    fd = vfs_open((const char *)filename, O_WRONLY | O_TRUNC);
    if (fd < 0)
    {
        fd = vfs_open((const char *)filename, O_CREAT | O_WRONLY | O_TRUNC);
        if (fd < 0)
        {
            OSI_LOGXI(OSI_LOGPAR_SI, 0X0, "[%s]open failed, h: %d", filename, fd);
            return false;
        }
    }
    if (vfs_write(fd, (uint8_t *)contextData, contextLen) != contextLen)
    {
        OSI_LOGXI(OSI_LOGPAR_S, 0X0, "[%s]write failed", filename);
        vfs_close(fd);
        return false;
    }
    vfs_close(fd);
    return true;
}

static bool at_read_from_file(uint8_t *filename, void *buffer, int length)
{
    int fd = -1;
    int result;
    int32_t fileSize = 0;

    OSI_LOGXI(OSI_LOGPAR_S, 0X0, "filename:%s", filename);

    fd = vfs_open((const char *)filename, O_RDONLY);
    if (fd < 0)
    {
        OSI_LOGXI(OSI_LOGPAR_SI, 0X0, "[%s]open failed, h: %d", filename, fd);
        return false;
    }

    struct stat st = {0};
    result = vfs_fstat(fd, &st);
    if (result != 0)
    {
        OSI_LOGXI(OSI_LOGPAR_SI, 0x1000983d, "[%s]fstat error, result: %d", filename, result);
        vfs_close(fd);
        return false;
    }

    fileSize = st.st_size;
    if (fileSize != length)
    {
        OSI_LOGXI(OSI_LOGPAR_SI, 0x1000983e, "[%s]file size(%d) error", filename, fileSize);
        vfs_close(fd);
        return false;
    }
    
    memset(buffer, 0, length);
    result = vfs_read(fd, buffer, length);
    if (result != length)
    {
        OSI_LOGXI(OSI_LOGPAR_SI, 0x1000983f, "[%s]file read(%d) error", filename, result);
        vfs_close(fd);
        return false;
    }

    vfs_close(fd);
    return true;
}

static uint32_t read_dm_device_info(int resId, char **outbuff)
{
    *outbuff = NULL;

    char buff[256] = { 0 };
    int  buflen = sizeof(buff);

    switch (resId)
    {
        case RES_DM_DEV_INFO://devinfo
            snprintf(buff,buflen,"%s", "");
            break;
        case RES_DM_APP_INFO://appinfo
            snprintf(buff,buflen,gDmCusCfgEX.APP_info);
            break;
        case RES_DM_MAC://mac
            snprintf(buff,buflen,gDmCusCfgEX.MAC);
            break;
        case RES_DM_ROM://rom
            snprintf(buff,buflen,gDmCusCfgEX.ROM);
            break;
        case RES_DM_RAM://ram
            snprintf(buff,buflen,gDmCusCfgEX.RAM);
            break;
        case RES_DM_CPU://CPU
            snprintf(buff,buflen,gDmCusCfgEX.CPU);
            break;
        case RES_DM_SYS_VERSION://SYS VERSION
            snprintf(buff,buflen,gDmCusCfgEX.osSysVer);
            break;
        case RES_DM_FIRM_VERSION://FIRMWARE VERSION
            snprintf(buff,buflen,gDmCusCfgEX.swVer);
            break;
        case RES_DM_FIRM_NAME://FIRMWARE NAME
            snprintf(buff,buflen,gDmCusCfgEX.swname);
            break;
        case RES_DM_VOLTE://Volte
            snprintf(buff,buflen,gDmCusCfgEX.VoLTE);
            break;
        case RES_DM_NET_TYPE://NetType
            snprintf(buff,buflen,gDmCusCfgEX.net_type);
            break;
        case RES_DM_NET_ACCT://Net Account
            snprintf(buff,buflen,gDmCusCfgEX.account);
            break;
        case RES_DM_PHONE://PhoneNumber
            snprintf(buff,buflen,gDmCusCfgEX.phoneNum);
            break;
        case RES_DM_LOCATION://Location
            snprintf(buff,buflen,gDmCusCfgEX.location);
            break;
        default:
            return 0;
    }

    *outbuff = (char*)malloc(strlen(buff)+1);
    memset(*outbuff, 0x00, strlen(buff)+1);
    memcpy(*outbuff, buff, strlen(buff));

    return strlen(*outbuff);
}


static uint8_t get_dm_encode_info(uint16_t resId, char **outbuff, char *pwd)
{
    uint32_t result = 0;
    char *szvalbuf = NULL;

    *outbuff = NULL;

    do
    {
        result = read_dm_device_info(resId, &szvalbuf);
        if (result == 0 || szvalbuf == NULL)
        {
            result = 0;
            break;
        }

        if (strlen((char *)szvalbuf) == 0)
        {
            result = 0;
            break;
        }

        int32_t outlen = dm_read_info_encode(szvalbuf, outbuff, pwd);
        if (outlen < 0)
        {
            result = 0;
            break;
        }
        result = strlen(*outbuff);
    }while(0);

    if (szvalbuf)
    {
        free(szvalbuf);
        szvalbuf = NULL;
    }

    OSI_LOGI(0x100094cb, "get_dm_encode_info get device info successfully");

    return result;
}

static void dm_cus_server_ind_process(osiEvent_t *pEvent)
{
    uint8_t i = 0;
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
        sprintf(refstr, "%s", STR_DM_EVENT_CODE(result));
#if DM_CUS_URC_ENABLE
        atCmdRespSimUrcText(nSimId, refstr);
#else
        OSI_LOGXI(OSI_LOGPAR_IS, 0x10009840, "dm: nSimId[%d], %s", nSimId, refstr);
#endif
        if (result == LWM2M_EVENT_UPDATE_NEED)
            lwm2m_update((gDmCusCfg.update_period + COAP_Max_Transmit_Wait), false, ref);
        else if (result == LWM2M_EVENT_CONNECT_SUCCESS)
            gDmCusState = DM_CUS_STATE_CONNECT_SUCC;
        else if (result == LWM2M_EVENT_CONNECT_FAILED)
            gDmCusState = DM_CUS_STATE_CONNECT_FAILED;
        else if (result == LWM2M_EVENT_REG_SUCCESS)
            gDmCusState = DM_CUS_STATE_REG_SUCC;
        else if (result == LWM2M_EVENT_REG_FAILED)
            gDmCusState = DM_CUS_STATE_REG_FAILED;
        else if (result == LWM2M_EVENT_REG_TIMEOUT)
            gDmCusState = DM_CUS_STATE_REG_TIMEOUT;
        else if (result == LWM2M_EVENT_LIFETIME_TIMEOUT)
            gDmCusState = DM_CUS_STATE_LIFETIME_TIMEOUT;
        else if (result == LWM2M_EVENT_UPDATE_SUCCESS)
            gDmCusState = DM_CUS_STATE_UPDATE_SUCC;
        else if (result == LWM2M_EVENT_UPDATE_FAILED)
            gDmCusState = DM_CUS_STATE_UPDATE_FAILED;
        else if (result == LWM2M_EVENT_UPDATE_TIMEOUT)
            gDmCusState = DM_CUS_STATE_UPDATE_TIMEOUT;

    }
    break;
    case OBJ_READ_IND:
    {
        uint32_t msgid = result;
        read_param_t *readparam = (read_param_t *)result_param1;
        char refstr[100] = {0};
        char *response = NULL;
        sprintf(refstr, "+MIPLREAD:%d,%ld,%d,%d,%d", ref, msgid, readparam->objid, (readparam->insid == 0xFFFF ? -1 : readparam->insid),
                (readparam->resid == 0xFFFF ? -1 : readparam->resid));
#if DM_CUS_URC_ENABLE
        atCmdRespSimUrcText(nSimId, refstr);
#else
        OSI_LOGXI(OSI_LOGPAR_IS, 0x10009840, "dm: nSimId[%d], %s", nSimId, refstr);
#endif
        if(readparam->resid != 0xFFFF)
        {
            if(get_dm_encode_info(readparam->resid, &response, gDmCusCfg.password) > 0)
            {
                lwm2m_read_rsp_onenet(msgid, readparam->objid, readparam->insid, readparam->resid, \
                                                lwm2m_string, (uint8_t *)response, strlen(response), \
                                                 ONENET_RESPONSE_READ, ref);
                free(response);
            }
        }
        else
        {
            for(i = 0; i<sizeof(gDMCusResID)/sizeof(uint32_t); i++)
            {
                uint32_t onenet_result = ONENET_NOTIFY_CONTINUE;
                if(i == (sizeof(gDMCusResID)/sizeof(uint32_t) - 1))
                    onenet_result = ONENET_RESPONSE_READ;
                if(get_dm_encode_info(gDMCusResID[i], &response, gDmCusCfg.password) > 0)
                {
                    OSI_LOGI(0x1000a0fc, "dm: get_dm_encode_info  resid = %d\n", gDMCusResID[i]);
                    lwm2m_read_rsp_onenet(msgid, readparam->objid, readparam->insid, gDMCusResID[i], \
                                                    lwm2m_string, (uint8_t *)response, strlen(response), \
                                                     onenet_result, ref);
                    free(response);
                }
            }
        }

        free(readparam);
    }
    break;
    case OBJ_OPERATE_RSP_IND:
    case SERVER_REGCMD_RESULT_IND:
    break;
    case SERVER_QUIT_IND:
    {
        gDmCusState = DM_CUS_STATE_INITIAL;
#if DM_CUS_URC_ENABLE
        atCmdRespSimUrcText(nSimId, "SERVER_QUIT_IND");
#else
        OSI_LOGXI(OSI_LOGPAR_IS, 0x10009840, "dm: nSimId[%d], %s", nSimId, "SERVER_QUIT_IND");
#endif
    }
    break;
    default:
    {
        char refstr[50] = {0};
        sprintf(refstr, "+UNPROCESS EVENT:%ld", pEvent->id);
#if DM_CUS_URC_ENABLE
        atCmdRespSimUrcText(nSimId, refstr);
#else
        OSI_LOGXI(OSI_LOGPAR_IS, 0x10009840, "dm: nSimId[%d], %s", nSimId, refstr);
#endif
    }
    break;
    }

    free(lwm2m_ev);
    lwm2m_ev = NULL;
    free(pEvent);
}

/*
static int8_t dm_setIMEI(char *buffer, uint8_t maxlen)
{
    if (maxlen < 16)
        return 0;

    return nvmWriteImeiStr(0, (nvmImeiStr_t *)buffer);
}

static uint8_t dm_getIMEI(char *buffer, uint8_t maxlen)
{
    if (maxlen < 16)
        return 0;

    return nvmReadImeiStr(0, (nvmImeiStr_t *)buffer);
}
*/

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
    strcpy(g_opt->szCMEI_IMEI, gDmCusCfg.tml_IMEI);

    memset(g_opt->szIMSI, 0x00, sizeof(g_opt->szIMSI));
    dm_getIMSI(g_opt->szIMSI, sizeof(g_opt->szIMSI));
}

static char * dm_Severdns(char *host, char *port)
{
    char *addressIP = NULL;
    struct addrinfo hints;
    struct addrinfo *servinfo = NULL;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if (0 != getaddrinfo(host, port, &hints, &servinfo) || servinfo == NULL) 
        return NULL;

    if (servinfo->ai_addr != NULL)
    {
        if(servinfo->ai_addr->sa_family == PF_INET)
        {
            struct sockaddr_in *addr = (struct sockaddr_in *)servinfo->ai_addr;
            addressIP = inet_ntoa(addr->sin_addr);
        }
            
        if (servinfo->ai_addr->sa_family == PF_INET6)
        {
            struct sockaddr_in6 *addr = (struct sockaddr_in6 *)servinfo->ai_addr;
            addressIP = inet6_ntoa(addr->sin6_addr);
        }
    }

    freeaddrinfo(servinfo);
    return addressIP;
}

static void dm_register_callback(void *param)
{
    int ret;
    char cmdline[512] = {0};
    char *host = NULL;
    char port[6] = {0};
    char *dm_pwd = NULL;
    char *deviceName = NULL;
    char *dmUpdeviceName = NULL;
    char *ternimal_type[3] = {"I", "L", "A"};

    OSI_LOGI(0x100080ca, "dm_register");

    Options dm_config = {0};
    memcpy(dm_config.szCMEI_IMEI, (char *)"CMEI_IMEI", strlen((char *)"CMEI_IMEI"));
    memcpy(dm_config.szIMSI, (char *)"IMSI", strlen((char *)"IMSI"));
    memcpy(dm_config.szDMv, (char *)"v2.0", strlen((char *)"v2.0"));
    strncpy(dm_config.szAppKey, gDmCusCfg.APP_key, (64-1)); //M100000058
    strncpy(dm_config.szPwd, gDmCusCfg.password, (64-1));   //s2n7TgPkg3IT2oo51T630F3f6U5XJ6vj

    dm_SdkInit(&dm_config);
    ret = genDmRegEndpointName(ternimal_type[gDmCusCfg.if_type], &deviceName, &dm_config);
    if (ret < 0)
    {
        OSI_LOGI(0x100080d7, "ERROR:Get device name error from IMEI/IMSI.");
        goto EXIT;
    }
    ret = genDmUpdateEndpointName(ternimal_type[gDmCusCfg.if_type], &dmUpdeviceName, &dm_config);
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

    if(gDmCusCfg.Server_type == 1)
    {
        host = gDmCusCfg.Server_IP;
        snprintf(port, sizeof(port), "%d", gDmCusCfg.Server_Port);
    }
    else
    {
        host = dm_Severdns(Default_Server_Domain, Default_Server_Port); 
        if(host == NULL)
            host = Default_Serevr_IP;
        else
            gDmCusState = DM_CUS_STATE_DNS_SUCC;

        strcpy(port, Default_Server_Port);
    }

    sprintf(cmdline, "-e %d -n %s -D %s -W %s -h %s -p %s -4",
            LWM2M_SERVER_ONENET, deviceName, dmUpdeviceName, dm_pwd, host, port);

    // customized read resource
    strcat(cmdline, " -d");

    // bootstrap
    //strcat(cmdline, " -b");

    OSI_LOGXI(OSI_LOGPAR_S, 0x100080d8, "dm cmdline:%s", cmdline);
    gDMCusConnectRef = lwm2m_new_config((uint8_t *)cmdline);
    if (gDMCusConnectRef >= 0)
    {
        OSI_LOGI(0x100080d9, "dm new config success");
    }
    else
    {
        OSI_LOGI(0x100080da, "dm new config failed");
        goto EXIT;
    }

    lwm2mcfg_set_handler_ext(gDMCusConnectRef, atEngineGetThreadId(), (osiCallback_t)dm_cus_server_ind_process, param, 0);
/*lifetime value calculation method
      *1) <93 (lifetime * ((float)lifetime / 120))
      *2) >93&&<186 lifetime + 62
      *3) >186  lifetime + 46
      * 24h == 86400s + 46
*/
    if (lwm2m_register(gDMCusConnectRef, (gDmCusCfg.update_period + COAP_Max_Transmit_Wait), 20) == LWM2M_RET_OK)
    {
        OSI_LOGI(0x100080db, "dm lwm2m_register success");
    }
    else
    {
        OSI_LOGI(0x100080dc, "dm lwm2m_register failed");
    }

EXIT:
    
    if (deviceName != NULL)
        free(deviceName);

    if (dmUpdeviceName != NULL)
        free(dmUpdeviceName);

    if (dm_pwd != NULL)
        free(dm_pwd);
    
    osiWorkDelete(gDmCusWork);
}

void dm_register(uint8_t nSimId)
{
    uint32_t param = (uint32_t)nSimId;

    if(gDmCusCfg.DM_mode && gDmCusState == DM_CUS_STATE_INITIAL)
    {
        gDmCusWork = osiWorkCreate(dm_register_callback, NULL, (void *)param);
        osiWorkEnqueue(gDmCusWork, osiSysWorkQueueLowPriority());
    }
}

bool dm_platform_set(void *config)
{
    DM_CUS_CONFIG_T *DmCusCfg = (DM_CUS_CONFIG_T *)config;

    if(at_write_to_file(dm_cus_cfg_file, (void*)DmCusCfg, sizeof(DM_CUS_CONFIG_T)) == false)
    {
        OSI_LOGI(0x100094cc, "dm cus write config failed");
        return false;
    }

    return true;
}

bool dm_platform_get(void *config)
{
    DM_CUS_CONFIG_T *DmCusCfg = (DM_CUS_CONFIG_T *)config;

    if (at_read_from_file(dm_cus_cfg_file, (uint8_t *)DmCusCfg, sizeof(DM_CUS_CONFIG_T)) == false)
    {
        OSI_LOGI(0x100094cd, "dm cus read config failed, use the default value");
        memcpy((void*)DmCusCfg, &gDmCusCfg, sizeof(DM_CUS_CONFIG_T));
    }

    return true;
}

bool dm_platform_setEX(void *config)
{
    gDmCusCfgEX = *((DM_CUS_CONFIG_EX_T *)config);
    if(at_write_to_file(dm_cus_cfg_ex_file, (void*)config, sizeof(DM_CUS_CONFIG_EX_T)) == false)
    {
        OSI_LOGI(0x100094ce, "dm cus ex write config failed");
        return false;
    }

    return true;
}

bool dm_platform_getEX(void *config)
{
    if (at_read_from_file(dm_cus_cfg_ex_file, config, sizeof(DM_CUS_CONFIG_EX_T)) == false)
    {
        OSI_LOGI(0x100094cf, "dm cus ex read config failed, use the default value");
        *((DM_CUS_CONFIG_EX_T *)config) = gDmCusCfgEX;
    }

    return true;
}


bool dm_state_get(void *state)
{
    ((DM_CUS_STATE_T *)state)->dm_state = gDmCusState;
    ((DM_CUS_STATE_T *)state)->dm_current_config = &gDmCusCfg;
    return true;
}

void dm_register_init(uint8_t self_register)
{
    OSI_LOGI(0x100094ca, "dm register init:%d", self_register);

    if (at_read_from_file(dm_cus_cfg_file, (uint8_t *)&gDmCusCfg, sizeof(DM_CUS_CONFIG_T)) == false)
    {
        OSI_LOGI(0x100094cd, "dm cus read config failed, use the default value");

        memset(&gDmCusCfg, 0, sizeof(DM_CUS_CONFIG_T));
        strcpy(gDmCusCfg.Server_IP, Default_Serevr_IP);
        gDmCusCfg.Server_Port = atoi(Default_Server_Port);
        gDmCusCfg.update_period = 86400;
    }

    if (at_read_from_file(dm_cus_cfg_ex_file, (uint8_t *)&gDmCusCfgEX, sizeof(DM_CUS_CONFIG_EX_T)) == false)
    {
        OSI_LOGI(0x100094d0, "dm cus read configex failed, use the default value");
    }
}


