#include "net_config.h"
#include "cJSON.h"
#include "netutils.h"
#include "mupnp/util/string.h"
#include "http_api.h"
#include "sockets.h"
#include "at_cfg.h"
#include <time.h>
#include "at_engine.h"
#include "osi_log.h"
#include "at_response.h"
#include "stdio.h"
#include "cfw.h"
#include "http_download.h"
#include "http_lunch_api.h"
#include "at_command.h"
#include "vfs.h"
#include "nvm.h"
#include "osi_api.h"

#define MBEDTLS_ERR_BASE64_BUFFER_TOO_SMALL               -0x002A  /**< Output buffer too small. */
#define MBEDTLS_ERR_BASE64_INVALID_CHARACTER              -0x002C  /**< Invalid character in input. */


#include <stdint.h>

static const unsigned char base64_enc_map[64] =
{
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
    'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
    'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd',
    'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x',
    'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', '+', '/'
};

#define BASE64_SIZE_T_MAX   ( (size_t) -1 ) /* SIZE_T_MAX is not standard */

/*
 * Encode a buffer into base64 format
 */
static int mbedtls_base64_encode( unsigned char *dst, size_t dlen, size_t *olen,
                   const unsigned char *src, size_t slen )
{
    size_t i, n;
    int C1, C2, C3;
    unsigned char *p;

    if( slen == 0 )
    {
        *olen = 0;
        return( 0 );
    }

    n = slen / 3 + ( slen % 3 != 0 );

    if( n > ( BASE64_SIZE_T_MAX - 1 ) / 4 )
    {
        *olen = BASE64_SIZE_T_MAX;
        return( MBEDTLS_ERR_BASE64_BUFFER_TOO_SMALL );
    }

    n *= 4;

    if( ( dlen < n + 1 ) || ( NULL == dst ) )
    {
        *olen = n + 1;
        return( MBEDTLS_ERR_BASE64_BUFFER_TOO_SMALL );
    }

    n = ( slen / 3 ) * 3;

    for( i = 0, p = dst; i < n; i += 3 )
    {
        C1 = *src++;
        C2 = *src++;
        C3 = *src++;

        *p++ = base64_enc_map[(C1 >> 2) & 0x3F];
        *p++ = base64_enc_map[(((C1 &  3) << 4) + (C2 >> 4)) & 0x3F];
        *p++ = base64_enc_map[(((C2 & 15) << 2) + (C3 >> 6)) & 0x3F];
        *p++ = base64_enc_map[C3 & 0x3F];
    }

    if( i < slen )
    {
        C1 = *src++;
        C2 = ( ( i + 1 ) < slen ) ? *src++ : 0;

        *p++ = base64_enc_map[(C1 >> 2) & 0x3F];
        *p++ = base64_enc_map[(((C1 & 3) << 4) + (C2 >> 4)) & 0x3F];

        if( ( i + 1 ) < slen )
             *p++ = base64_enc_map[((C2 & 15) << 2) & 0x3F];
        else *p++ = '=';

        *p++ = '=';
    }

    *olen = p - dst;
    *p = 0;

    return( 0 );
}

#define UEICCID_LEN 20
#define REG_HTTP_URL "http://zzhc.vnet.cn:9999/"
#define REG_HTTP_CONTENT "application/encrypted-json"
#define NV_SELF_REG "/nvm/dm_self_register.nv"
#define SOFTWARE_VER_NUM "BC60_1268D_V1.0"
#define RETRY_TIMER  (60*60*1000)

osiTimer_t *ghttpregtimer = NULL;
osiThread_t *dxhttpThread = NULL;
#ifdef CONFIG_NET_NBSELFREG_HTTP_SUPPORT
#define RETRY_NUM  3
#else
#define RETRY_NUM  10
#endif
static void do_dxregProcess(uint8_t nCid, uint8_t nSimId);

extern char *vnetregdata;

extern osiThread_t *netGetTaskID();

typedef struct {
    uint8_t simId;
    uint8_t nCid;
    uint8_t resultCode;
    uint8_t retryCount;
    uint8_t resultDesc[12];
    uint8_t SimIccid[20];
}reg_ctrl_t;

reg_ctrl_t reg_ctrl = {0};
nHttp_info *at_nHttp_reg = NULL;
bool is_dxreg_inited = false;

static bool isResultOk(reg_ctrl_t *reg_ctrl)
{
    return reg_ctrl->resultCode == 0 && strcmp((char *)reg_ctrl->resultDesc,"Success") == 0;
}

static void parseJson(char *pMsg, reg_ctrl_t *reg_ctrl)
{
    if (NULL == pMsg)
    {
        OSI_LOGI(0x1000a10b, "[VNET]CHN-CT parseJson: pMsg is NULL!!!\n");
        return;
    }
    cJSON *pJson = cJSON_Parse(pMsg);
    if (NULL == pJson)
    {
        OSI_LOGI(0x1000a10c, "[VNET]CHN-CT parseJson: pJson is NULL!!!\n");
        return;
    }
    char *regdatajs = cJSON_PrintUnformatted(pJson);
    OSI_LOGXI(OSI_LOGPAR_S, 0x1000a10d, "[VNET]CHN-CT parseJson: %s\n", regdatajs);

    cJSON *pSub = cJSON_GetObjectItem(pJson, "resultCode");
    if (NULL != pSub)
    {
        OSI_LOGXI(OSI_LOGPAR_S, 0x1000a10e, "[VNET]CHN-CT resultCode: %s\n", pSub->valuestring);
        reg_ctrl->resultCode = atoi(pSub->valuestring);
    }

    pSub = cJSON_GetObjectItem(pJson, "resultDesc");
    if (NULL != pSub)
    {
        OSI_LOGXI(OSI_LOGPAR_S, 0x1000a10f, "[VNET]CHN-CT resultDesc: %s\n", pSub->valuestring);
        strncpy((char *)reg_ctrl->resultDesc,pSub->valuestring,10);
    }
     free(regdatajs);
     if(NULL != pJson)
     {
         cJSON_Delete(pJson);
     }
}

OSI_UNUSED static int OutputTime(char *rsp, struct tm *tm)
{
    static const char *fmt1 = "20%02d-%02d-%02d %02d:%02d:%02d";
    static const char *fmt2 = "20%04d-%02d-%02d %02d:%02d:%02d";
    if (gAtSetting.csdf_auxmode == 1)
    {
        return sprintf(rsp, fmt1, (tm->tm_year + 1900) % 100, tm->tm_mon + 1, tm->tm_mday,
                       tm->tm_hour, tm->tm_min, tm->tm_sec);
    }
    else
    {
        return sprintf(rsp, fmt2, tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                       tm->tm_hour, tm->tm_min, tm->tm_sec);
    }
}

void httpreg_timeout(void *param)
{
    OSI_LOGI(0x1000a110, "[VNET]CHN-CT httpreg_timeout retryCount: %d\n", reg_ctrl.retryCount);
    do_dxregProcess(reg_ctrl.nCid, reg_ctrl.simId);
}

static char *genRegData(uint8_t simid)
{
    unsigned char *buffer;
    size_t buffer_len;
    size_t len;
    size_t out_len;
    cJSON *pJsonRoot = NULL;
    char cellID[10] = {0};;
    uint8_t imei[16] = {0};
    uint8_t imsi[16] = {0};
    uint8_t simiccid[21] = {0};
    uint8_t imsisize = 16;
    uint8_t iccid_len = 20;

    int nImeiLen = nvmReadImeiStr(simid, (nvmImeiStr_t *)imei);
    if (nImeiLen == -1)
    {
        OSI_LOGI(0x1000a111, "[VNET]CHN-CT nvmReadImei read error...");
        return NULL;
    }

    OSI_LOGXI(OSI_LOGPAR_S, 0x1000a112, "[VNET]CHN-CT NvmReadImeigetSimImei:%s", imei);
    getSimIccid(simid,simiccid,&iccid_len);
    getSimImsi(simid,imsi,&imsisize);

#ifdef CONFIG_NET_NBSELFREG_HTTP_SUPPORT
    pJsonRoot = cJSON_CreateObject();
    if (NULL == pJsonRoot)
        return NULL;
    cJSON_AddStringToObject(pJsonRoot, "REGVER", "5.0");
    cJSON_AddStringToObject(pJsonRoot, "MEID", (const char *)imei);
    cJSON_AddStringToObject(pJsonRoot, "MODEL", "BJS-BC60");
    cJSON_AddStringToObject(pJsonRoot, "SWVER", SOFTWARE_VER_NUM);    //软件版本号
    cJSON_AddStringToObject(pJsonRoot, "SIM1ICCID", (const char *)simiccid);
    cJSON_AddStringToObject(pJsonRoot, "SIM1LTEIMSI", (const char *)imsi);
#else
//get current time info
    int timeoffset = gAtSetting.timezone * CFW_TIME_ZONE_SECOND;
    time_t lt = osiEpochSecond() + timeoffset;
    struct tm ltm;
    gmtime_r(&lt, &ltm);
    char ts[48];
    OutputTime(ts, &ltm);
//get CELLID info
    uint8_t rat = CFW_NWGetStackRat(simid);
    if (rat == 4)
    {
        CFW_NET_INFO_T *pNetinfo = (CFW_NET_INFO_T *)malloc(sizeof(CFW_NET_INFO_T));
        memset(pNetinfo, 0, sizeof(CFW_NET_INFO_T));
        if (0 != CFW_GetNetinfo(pNetinfo, simid))
        {
            free(pNetinfo);
            OSI_LOGI(0x1000a113, "[VNET]CHN-CT genRegData NWGetStackRat = %d\n", rat);
            return NULL;
        }
        sprintf(cellID, "%ld", (uint32_t)((pNetinfo->nwCapabilityLte.lteScell.enodeBID << 8) + pNetinfo->nwCapabilityLte.lteScell.cellId));
        free(pNetinfo);
    }
    else
    {
        OSI_LOGI(0x1000a113, "[VNET]CHN-CT genRegData NWGetStackRat = %d\n", rat);
        return NULL;
    }
    pJsonRoot = cJSON_CreateObject();
    if (NULL == pJsonRoot)
        return NULL;
    cJSON_AddStringToObject(pJsonRoot, "REGVER", "5.0");
    cJSON_AddStringToObject(pJsonRoot, "MEID", "");
    cJSON_AddStringToObject(pJsonRoot, "MODEL", "BJS-BC60");
    cJSON_AddStringToObject(pJsonRoot, "SWVER", SOFTWARE_VER_NUM);    //软件版本号
    cJSON_AddStringToObject(pJsonRoot, "IMEI1", (char *)imei);
    cJSON_AddStringToObject(pJsonRoot, "SIM1CDMAIMSI", "");
    cJSON_AddStringToObject(pJsonRoot, "SIM1ICCID", (const char *)simiccid);
    cJSON_AddStringToObject(pJsonRoot, "SIM1LTEIMSI", (const char *)imsi);
    cJSON_AddStringToObject(pJsonRoot, "SIM1CELLID", (const char *)cellID);
    cJSON_AddStringToObject(pJsonRoot, "REGDATE", (const char *)ts);


#endif
    char *regdatajs = cJSON_PrintUnformatted(pJsonRoot);

    if (regdatajs == NULL)
    {
        OSI_LOGI(0x1000a114, "[VNET]CHN-CT JsonString error...");
        cJSON_Delete(pJsonRoot);
        return NULL;
    }
    else
        OSI_LOGXI(OSI_LOGPAR_S, 0x1000a115, "[VNET]CHN-CT genRegData :%s\n", regdatajs);
    out_len = strlen(regdatajs);

    mbedtls_base64_encode( NULL, 0, &buffer_len, (const unsigned char *)regdatajs, out_len);
    buffer = (unsigned char *)malloc(buffer_len);

    if (buffer != NULL && mbedtls_base64_encode(buffer, buffer_len, &len, (const unsigned char *)regdatajs, out_len) != 0)
    {
        OSI_LOGI(0x1000a116, "[VNET]CHN-CT base64 error...");
    }
    free(regdatajs);
    cJSON_Delete(pJsonRoot);
    return (char *)buffer;
}
static void dx_registercb(void *param)
{
    nHttp_info *nHttp_inforeg = (nHttp_info *)param;
    OSI_LOGI(0x1000a117, "[VNET]CHN-CT dx_registercb Entering at_nHttp_reg->CID= %d", at_nHttp_reg->CID);
    if (Http_postnreg(nHttp_inforeg) == false)
    {
        if (Term_Http(at_nHttp_reg) != true)
        {
            OSI_LOGI(0x1000a118, "[VNET]CHN-CT dx_registercb Term fail, please try again later\n");
            at_nHttp_reg = NULL;
            is_dxreg_inited = false;
            goto threadexit;
        }
        at_nHttp_reg = NULL;
        if(reg_ctrl.retryCount < RETRY_NUM)
        {
            reg_ctrl.retryCount++;
            OSI_LOGI(0x1000a119, "[VNET]CHN-CT dx_registercb  POSTREG error etryCount=%d\n", reg_ctrl.retryCount );
            ghttpregtimer = osiTimerCreate(netGetTaskID(), httpreg_timeout, NULL);
            if (ghttpregtimer == NULL)
            {
                OSI_LOGI(0x1000a11a, "[VNET]CHN-CT dx_registercb HTTPreg create TIMER failed \n");
                is_dxreg_inited = false;
                goto threadexit;
            }
            osiTimerStart(ghttpregtimer , RETRY_TIMER);
            goto threadexit;
        }
        else
        {
             OSI_LOGI(0x1000a11b, "[VNET]CHN-CT dx_registercboperation error retryCount = %d\n", reg_ctrl.retryCount );
             if(NULL != vnetregdata)
             {
                free(vnetregdata);
                vnetregdata = NULL;
             }
             is_dxreg_inited = false;
             goto tmdethexit;
        }
    }
    else
    {
         if (Term_Http(at_nHttp_reg) != true)
        {
            OSI_LOGI(0x1000a11c, "[VNET]CHN-CT dx_registercb Term_http fail\n");
            at_nHttp_reg = NULL;
            is_dxreg_inited = false;
            goto threadexit;
        }
         at_nHttp_reg = NULL;
        // response operation succuss
       //char *data = NULL;    //data 做一个全局变量下行下去得到返回的数据
       OSI_LOGI(0x1000a11d, "[VNET]CHN-CT dx_registercb postnreg success\n");
       parseJson(vnetregdata, &reg_ctrl);
       if(NULL != vnetregdata)
       {
           free(vnetregdata);
           vnetregdata = NULL;
       }
       if (isResultOk(&reg_ctrl))
       {
           uint8_t simiccid[UEICCID_LEN+1] = {0};
           uint8_t iccid_len = UEICCID_LEN;
           if (ghttpregtimer != NULL)
           {
               osiTimerStop(ghttpregtimer);
               osiTimerDelete(ghttpregtimer);
               ghttpregtimer = NULL;
           }
           OSI_LOGI(0x1000a11e, "[VNET]CHN-CT dx_registercb response operation succuss data right\n");
           getSimIccid(reg_ctrl.simId,simiccid,&iccid_len);
           vfs_file_write(NV_SELF_REG, simiccid, 20);
           is_dxreg_inited = false;
           //NV_SetUEIccid(simiccid,iccid_len,reg_ctrl->simId);
           goto threadexit;
       }
       else if (reg_ctrl.retryCount < RETRY_NUM)
       {
           reg_ctrl.retryCount++;
           OSI_LOGI(0x1000a11f, "[VNET]CHN-CT dx_registercb response operation succuss data error\n");
           ghttpregtimer = osiTimerCreate(netGetTaskID(), httpreg_timeout, NULL);
           if (ghttpregtimer == NULL)
           {
               OSI_LOGI(0x1000a120, "[VNET]CHN-CT dx_registercb HTTPreg create Timer failed\n");
               is_dxreg_inited = false;
               goto threadexit;
           }
           osiTimerStart(ghttpregtimer , RETRY_TIMER);
           goto threadexit;
           //COS_StartFunctionTimer(60*60*1000,do_dxregProcess,reg_ctrl->simId);
       }
       else
       {
            OSI_LOGI(0x1000a121, "[VNET]CHN-CT dx_registercb POSTREG success retryCount = %d", reg_ctrl.retryCount );
            if(NULL != vnetregdata)
            {
               free(vnetregdata);
               vnetregdata = NULL;
            }
            is_dxreg_inited = false;
            goto tmdethexit;
       }

    }
tmdethexit:
    if (ghttpregtimer != NULL)
    {
        //osiTimerStop(ghttpregtimer);
        osiTimerDelete(ghttpregtimer);
        ghttpregtimer = NULL;
    }
    if(dxhttpThread  != NULL)
    {
        dxhttpThread = NULL;
        OSI_LOGI(0x1000a122, "[VNET]CHN-CT dx_registercb will exit...\n");
        osiThreadExit();
    }
    return;

threadexit:
    if(dxhttpThread  != NULL)
    {
        dxhttpThread = NULL;
        OSI_LOGI(0x1000a122, "[VNET]CHN-CT dx_registercb will exit...\n");
        osiThreadExit();
    }
    return;
}

static void do_dxregProcess(uint8_t nCid, uint8_t nSimId)
{
    char *regdata = NULL;
    OSI_LOGI(0x1000a123, "[VNET]CHN-CT do_dxregProcess ncid = %d nSimid = %d\n", nCid, nSimId);
    if (netif_default == NULL)
        goto retry;

    uint8_t sim = (netif_default->sim_cid & 0xf0) >> 4;
    uint8_t dtCid = netif_default->sim_cid & 0x0f;
    if(sim != nSimId || dtCid  != nCid)
    {
        OSI_LOGI(0x1000a124, "[VNET]CHN-CT do_dxregProcess EXIT case sim != nSimId %d %d\n", sim, nSimId);
        return;
    }
    if(at_nHttp_reg != NULL)
    {
        OSI_LOGI(0x1000a125, "[VNET]CHN-CT do_dxregProcess#at_nHttp_reg is not NULL\n");
        Term_Http(at_nHttp_reg);
    }
    at_nHttp_reg = Init_Http();
    if (at_nHttp_reg == NULL)
    {
        OSI_LOGI(0x1000a126, "[VNET]CHN-CT Http_Init fail\n");
        goto retry;
    }
    regdata = genRegData(nSimId);
    if (regdata == NULL)
    {
        OSI_LOGI(0x1000a127, "[VNET]CHN-CT regdata error...\n");
        goto retry;
    }
    at_nHttp_reg->CID = nCid;
    OSI_LOGI(0x1000a128, "[VNET]CHN-CT do_dxregProcess at_nHttp_reg->CID=%d", at_nHttp_reg->CID);
    strcpy(at_nHttp_reg->url,  REG_HTTP_URL);
    strcpy(at_nHttp_reg->CONTENT_TYPE,  REG_HTTP_CONTENT );
    strcpy(at_nHttp_reg->user_data,  regdata );

    free(regdata);
    (at_nHttp_reg->cg_http_api)->nSIM = nSimId;
    (at_nHttp_reg->cg_http_api)->nCID = nCid;
    dxhttpThread = osiThreadCreate("dx_register", dx_registercb, at_nHttp_reg, OSI_PRIORITY_NORMAL,4096, 0);
    if(dxhttpThread == NULL)
    {
        OSI_LOGI(0x1000a129, "[VNET]CHN-CT httppost create THREAD fail\n");
        goto retry;
    }
    return;
retry:
    if (reg_ctrl.retryCount < RETRY_NUM)
    {
        reg_ctrl.retryCount++;
        OSI_LOGI(0x1000a12a, "[VNET]CHN-CT  do_dxregProcess error\n");
        ghttpregtimer = osiTimerCreate(netGetTaskID(), httpreg_timeout, NULL);
        if (ghttpregtimer == NULL)
        {
            OSI_LOGI(0x1000a12b, "[VNET]CHN-CT do_dxregProcess# do_dxregProcessTimer create failed\n");
            is_dxreg_inited = false;
            return;
        }
        osiTimerStart(ghttpregtimer , RETRY_TIMER);
    }
    else
    {
         if (ghttpregtimer != NULL)
         {
             //osiTimerStop(ghttpregtimer);
             osiTimerDelete(ghttpregtimer);
             ghttpregtimer = NULL;
         }
         is_dxreg_inited = false;
         return;
    }
}

bool sul_ZeroMemory8(void  *pBuf, uint32_t count)
{
    char *xs = pBuf;
    while (count--)
        *xs++ = 0x0;
    return 1;
}

extern bool fupdateIsBootAfterFota(void);
int vnet4gSelfRegister(uint8_t nCid, uint8_t nSimId)
{
    uint8_t tmpMode = 0;
    uint8_t operatorId[6];
    uint8_t *pOperName = NULL;
    uint8_t *pOperShortName = NULL;
    uint8_t ueiccid[UEICCID_LEN + 1] = {0};
    uint8_t simiccid[21] = {0};
    uint8_t iccid_len = UEICCID_LEN;
    uint8_t  nRet = 0;
    bool futa_status = fupdateIsBootAfterFota();
    if (futa_status == true)
    {
        OSI_LOGI(0x1000a0fd, "[UNISOC-4G]CHN-CT Self-Register ota finish\n");
        vfs_unlink(NV_SELF_REG);
        vfs_file_write(NV_SELF_REG, "0", 1);
    }

    if (!getSimIccid(nSimId,simiccid,&iccid_len))
    {
        OSI_LOGE(0x1000a9af, "[UNISOC-4G]CHN-CT get simiccid error exit...\n");
        return -1;
    }
    if (memcmp(reg_ctrl.SimIccid, simiccid,iccid_len) == 0)
    {
        OSI_LOGE(0x1000a9b0, "[UNISOC-4G]CHN-CT other pdp active not do self-reg...\n");
        return -1;
    }
    if (ghttpregtimer != NULL)         //上次未做完需要停掉
    {
        OSI_LOGI(0x1000a12c, "[VNET]CHN-CT vnet4gSelfRegister#REGTIMER is running EXIT\n");
        osiTimerStop(ghttpregtimer);
        osiTimerDelete(ghttpregtimer);
        ghttpregtimer = NULL;
        is_dxreg_inited = false;
    }
    if(is_dxreg_inited)
    {
        OSI_LOGI(0x1000a12d, "[VNET]CHN-CT vnet4gSelfRegister#register is processing do not repeat exit\n");
        return 0;
    }
    is_dxreg_inited = true;
    sul_ZeroMemory8(operatorId, 6);
    nRet = CFW_NwGetCurrentOperator(operatorId, &tmpMode, nSimId);
    if (nRet != 0)
    {
        is_dxreg_inited = false;
        return -1;
    }
    OSI_LOGXI(OSI_LOGPAR_IS, 0x1000a12e, "[VNET]CHN-CT vnet4gSelfRegister#CurrentOperator=%d %s\n", tmpMode, operatorId);
    nRet =  CFW_CfgNwGetOperatorName(operatorId, &pOperName, &pOperShortName);
    if (nRet != 0)
    {
        is_dxreg_inited = false;
        return -1;
    }
    OSI_LOGXI(OSI_LOGPAR_S, 0x1000a12f, "[VNET]CHN-CT vnet4gSelfRegister#OperatorName=%s\n", pOperName);
    if ((0 == strcmp("ChinaTelecom", (char*)pOperName))||(0 == strcmp("CHN-CT", (char*)pOperName)))
    {
        memcpy(reg_ctrl.SimIccid, simiccid, 20);
        int fd = vfs_open(NV_SELF_REG, 2);
        if (fd < 0)
        {
            OSI_LOGI(0x1000a130, "[VNET]CHN-CT vnet4gSelfRegister#file is NULL fd =%d\n", fd);
            vfs_file_write(NV_SELF_REG, "0", 1);
        }
        else
        {
            OSI_LOGI(0x1000a131, "[VNET]CHN-CT vnet4gSelfRegister#file open success fd =%d\n", fd);
            struct stat st = {};
            vfs_fstat(fd, &st);
            int file_size = st.st_size;
            if(file_size == 1 || file_size == 20)
            {
                vfs_read(fd, ueiccid, 20);
                OSI_LOGXI(OSI_LOGPAR_SI, 0x1000a132, "[VNET]CHN-CT vnet4gSelfRegister#file ueiccid =%s  file_size=%d\n", ueiccid, file_size);
                vfs_close(fd);
            }
            else
            {
                OSI_LOGI(0x1000a133, "[VNET]CHN-CT vnet4gSelfRegister#File size error =%d\n", file_size);
                vfs_unlink(NV_SELF_REG);
                vfs_file_write(NV_SELF_REG, "0", 1);
            }

        }
        // NV_GetUEIccid(ueiccid,iccid_len,simId);    //读flag   比较iccid
        if (memcmp(ueiccid, simiccid,iccid_len) == 0)
        {
             is_dxreg_inited = false;
             OSI_LOGI(0x1000a134, "[VNET]CHN-CT vnet4gSelfRegister# ueiccid same as simiccid success\n");
             return 0;
        }
        else
        {
            OSI_LOGXI(OSI_LOGPAR_S, 0x1000a135, "[VNET]CHN-CT vnet4gSelfRegister#simiccid = %s\n", simiccid);
            reg_ctrl.nCid = nCid;
            reg_ctrl.simId = nSimId;
            reg_ctrl.retryCount = 0;
            do_dxregProcess(nCid, nSimId);
        }
        return 1;
    }
    else
    {
        OSI_LOGI(0x1000a136, "[VNET]CHN-CT vnet4gSelfRegister#operator is not CHN-CT\n");
        is_dxreg_inited = false;
        return 0;
    }
}




