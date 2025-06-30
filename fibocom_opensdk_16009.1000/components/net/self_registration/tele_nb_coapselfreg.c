#include "net_config.h"

#include "cJSON.h"
#include "netutils.h"
#include "sockets.h"
#include "at_cfg.h"
#include <time.h>
#include "osi_log.h"
#include "stdio.h"
#include "cfw.h"
#include "vfs.h"
#include "nvm.h"
#include "osi_api.h"
#include "coap_async_api.h"
#include <stdint.h>
#include "drv_rtc.h"
#include "drv_i2c.h"
#include "drv_names.h"
#include <time.h>
#include "fupdate.h"

#define MBEDTLS_ERR_BASE64_BUFFER_TOO_SMALL               -0x002A  /**< Output buffer too small. */
#define MBEDTLS_ERR_BASE64_INVALID_CHARACTER              -0x002C  /**< Invalid character in input. */


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
#define REG_COAP_URL "http://zzhc.vnet.cn/nb"
#define REG_COAP_CONTENT "application/json"
#define NV_NB_COAP_SELF_REG "/nvm/nbcoap_self_register.nv"
#define SOFTWARE_VER_NUM "BC60_1268D_V1.0"
#define RETRY_REGTIMER  (60*60*1000)

extern bool g_bCoapDoing;      // coapzizhuce
osiTimer_t *gcoapregtimer = NULL;

#ifdef CONFIG_NBSELFREG_CUS_SUPPORT
#define AT_UNREG OSI_MAKE_TAG('A', 'T', 'R', 'G')
#define NV_UNICOM_LAST_REGTIME "/nvm/nbcoap_last_regtimer.nv"
#define NV_NB_SELFREG_FLAG  "/nvm/nb_ynself_register.nv"
#define SELF_TEST_CYCLE 28  //This value can only be set to 27 ~30 ;
extern int interal_timer_flag;

typedef struct
{
    char text[CONFIG_ATS_ALARM_TEXT_LEN + 1];
    uint8_t has_timezone;
    int8_t timezone;
    uint8_t silent;
} atUnregInfo_t;
#endif
static void do_dxnbregProcess(uint8_t nCid, uint8_t nSimId);



extern osiThread_t *netGetTaskID();

typedef void (*coap_selfreg_data_cb_t)(unsigned int uti, char *code, int len);
typedef struct {
    uint8_t simId;
    uint8_t nCid;
    uint8_t resultCode;
    uint8_t retryCount;
    uint8_t resultDesc[12];
    int OperatorFlag;
    coap_status_t status;
    coap_selfreg_data_cb_t cb;
}reg_ctrl_t;

reg_ctrl_t *reg_ctrl = NULL;
bool is_dxnbreg_inited = false;
int nbselfreg_flag = 0;
#ifdef CONFIG_NBSELFREG_CUS_SUPPORT

//extern void atCmdRespDefUrcText(const char *text);

static void _calaUnicomRegCB(drvRtcAlarm_t *alarm, void *unused)
{
    //atUnregInfo_t *info = (atUnregInfo_t *)alarm->info;
    //char rsp[80];
    OSI_LOGI(0x10009b74, "[CIOT]UNICOM Self-Register 28th days timeout\n");
    //sprintf(rsp, "+CALU: %ld, \"%s\"", alarm->name, info->text);

    //atCmdRespDefUrcText(rsp);
    if(reg_ctrl  != NULL)
    {
         OSI_LOGI(0x10009b75, "[CIOT]UNICOM Self-Register enter re-registration\n");
         if (NULL != gcoapregtimer)
         {
             osiTimerStop(gcoapregtimer);
             osiTimerDelete(gcoapregtimer);
             gcoapregtimer = NULL;
         }
         if(is_dxnbreg_inited)
         {
             OSI_LOGI(0x10009b76, "[CIOT]UNICOM Self-Register is doing exit this connection\n");
             return;
         }
         is_dxnbreg_inited = true;
         do_dxnbregProcess(reg_ctrl->nCid,  reg_ctrl->simId);
    }
}

void UnicomRegInit()
{
    OSI_LOGI(0x10009b77, "[CIOT]UNICOM Self-Register init\n");
    drvRtcAlarmOwnerSetCB(AT_UNREG, NULL,  _calaUnicomRegCB);
    int count = drvRtcGetAlarmCount(AT_UNREG);
    if (count != 0)
        drvRtcRemoveAlarm(AT_UNREG, 1);

}

static void timereg_get_save()
{
    static const char *fmt1 = "%02d/%02d/%02d,%02d:%02d:%02d";
    char rsp[48];
    uint8_t mon[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    uint8_t monleap[12] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int64_t setAlarmSec = 0;
    int timeoffset = gAtSetting.timezone * CFW_TIME_ZONE_SECOND;
    time_t lt = osiEpochSecond() + timeoffset;
    struct tm ltm, tm;
    gmtime_r(&lt, &ltm);
    tm.tm_sec = ltm.tm_sec;
    tm.tm_min = ltm.tm_min;
    tm.tm_hour = ltm.tm_hour;
    tm.tm_isdst = 0;
    if((ltm.tm_year + 1900) % 4 == 0)
    {
         if(ltm.tm_mday + SELF_TEST_CYCLE > monleap[ltm.tm_mon])
         {
             if((ltm.tm_mon == 0) && ((ltm.tm_mday + SELF_TEST_CYCLE) > (monleap[ltm.tm_mon] + monleap[ltm.tm_mon + 1])))
             {
                  tm.tm_mday = SELF_TEST_CYCLE - monleap[ltm.tm_mon + 1] - (monleap[ltm.tm_mon] - ltm.tm_mday);
                  tm.tm_mon = ltm.tm_mon + 2;
                  tm.tm_year = ltm.tm_year ;
                  goto Continue;
             }
             tm.tm_mday = SELF_TEST_CYCLE - (monleap[ltm.tm_mon] - ltm.tm_mday);
             if((ltm.tm_mon + 1) == 12)
             {
                  tm.tm_mon = 1;
                  tm.tm_year = ltm.tm_year + 1;
             }
             else
             {
                 tm.tm_mon = ltm.tm_mon + 1;
                 tm.tm_year = ltm.tm_year;
             }

         }
         else
         {
             tm.tm_mday= ltm.tm_mday + SELF_TEST_CYCLE;
             tm.tm_mon = ltm.tm_mon;
             tm.tm_year = ltm.tm_year;
         }
    }
    else
    {
        if(ltm.tm_mday + SELF_TEST_CYCLE > mon[ltm.tm_mon])
         {
             if((ltm.tm_mon == 0) && ((ltm.tm_mday + SELF_TEST_CYCLE) > (monleap[ltm.tm_mon] + monleap[ltm.tm_mon + 1])))
             {
                  tm.tm_mday = SELF_TEST_CYCLE - monleap[ltm.tm_mon + 1] - (monleap[ltm.tm_mon] - ltm.tm_mday);
                  tm.tm_mon = ltm.tm_mon + 2;
                  tm.tm_year = ltm.tm_year ;
                  goto Continue;
             }
             tm.tm_mday = SELF_TEST_CYCLE - (mon[ltm.tm_mon] - ltm.tm_mday);
             if((ltm.tm_mon + 1) == 12)
             {
                  tm.tm_mon = 0;
                  if(ltm.tm_year + 1901 >= 2100)
                  {
                       tm.tm_sec = 0;
                       tm.tm_min = 0;
                       tm.tm_hour = 0;
                       tm.tm_mday= 29;
                       tm.tm_mon = 1;
                       tm.tm_year = 100;
                  }
                  else
                  {
                       tm.tm_year = ltm.tm_year + 1;
                  }
             }
             else
             {
                  tm.tm_mon = ltm.tm_mon + 1;
                  tm.tm_year = ltm.tm_year;
             }

         }
         else
         {
             tm.tm_mday= ltm.tm_mday + SELF_TEST_CYCLE;
             tm.tm_mon = ltm.tm_mon;
             tm.tm_year = ltm.tm_year;
         }
    }
Continue:
    setAlarmSec = mktime(&tm) - timeoffset;
    atUnregInfo_t info = {0};
    strcpy(info.text, "unreg");
    info.has_timezone = false;

    info.timezone = gAtSetting.timezone;
    info.silent = 0;
    drvRtcSetAlarm(AT_UNREG, 1, &info, sizeof(info), setAlarmSec, true);
    OSI_LOGI(0x10009b78, "[CIOT]UNICOM Self-Register save_time: %02d/%02d/%02d,%02d:%02d:%02d\n",(ltm.tm_year + 1900) % 100, ltm.tm_mon + 1, ltm.tm_mday,
                       ltm.tm_hour, ltm.tm_min, ltm.tm_sec);
    sprintf(rsp, fmt1, (tm.tm_year + 1900) % 100, tm.tm_mon + 1, tm.tm_mday,
                       tm.tm_hour, tm.tm_min,  tm.tm_sec);
    vfs_unlink(NV_UNICOM_LAST_REGTIME);
    vfs_file_write(NV_UNICOM_LAST_REGTIME, rsp, 17);

}

static void setstartreg_time(struct tm *tm)
{
     int64_t setAlarmSec = 0;
     int timeoffset = gAtSetting.timezone * CFW_TIME_ZONE_SECOND;
     tm->tm_year = tm->tm_year + 100;
     tm->tm_mon = tm->tm_mon - 1;
     OSI_LOGI(0x10009b79, "[CIOT]UNICOM Self-Register set_start_regtime: %d/%02d/%02d,%02d:%02d:%02d\n", tm->tm_year, tm->tm_mon,
         tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
     setAlarmSec = mktime(tm) - timeoffset;
     atUnregInfo_t info = {0};
     strcpy(info.text, "unreg");
     info.has_timezone = false;

     info.timezone = gAtSetting.timezone;
     info.silent = 0;
     drvRtcSetAlarm(AT_UNREG, 1, &info, sizeof(info), setAlarmSec, true);
}

static int time_roundfind()
{
    static const char *fmt1 = "%02d/%02d/%02d,%02d:%02d:%02d";
    int regedtimer = 0;
    char timeStr[28] ={0};
    int timeoffset = gAtSetting.timezone * CFW_TIME_ZONE_SECOND;
    time_t lt = osiEpochSecond() + timeoffset;
    struct tm ltm;
    gmtime_r(&lt, &ltm);
    int fd = vfs_open(NV_UNICOM_LAST_REGTIME, 2);
    if (fd < 0)
    {
        OSI_LOGI(0x10009b7a, "[CIOT]UNICOM Self-Register time_roundfind open failed %d\n", fd);
        timereg_get_save();
        goto End;
     }
     else
     {
        OSI_LOGI(0x10009b7b, "[CIOT]UNICOM Self-Register time_roundfind open success\n");
        struct stat st = {};
        vfs_fstat(fd, &st);
        int file_size = st.st_size;
        if(file_size == 17)
        {
            OSI_LOGI(0x10009b7c, "[CIOT]UNICOM Self-Register time file size: %d\n", file_size);
            vfs_read(fd, timeStr, 17);
            OSI_LOGXI(OSI_LOGPAR_S, 0x10009b7d, "[CIOT]UNICOM Self-Register time: %s\n", timeStr);
            vfs_close(fd);
        }
        else
        {
            OSI_LOGI(0x10009b7e, "[CIOT]UNICOM Self-Register flie size error: %d\n", file_size);
            timereg_get_save();
            goto End;
        }
     }

    struct tm tm;
    memset(&tm, 0x00, sizeof(tm));
    size_t slen = strlen(timeStr);
    if (slen != 17 )
    {
         OSI_LOGI(0x10009b7f, "[CIOT]UNICOM Self-Register time_roundfind length error!!\n");
          goto End;
    }
    int n = sscanf(timeStr, fmt1, &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
                       &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
    if (n != 6)
    {
         OSI_LOGI(0x10009b80, "[CIOT]UNICOM Self-Register time_roundfind n= %d\n", n);
          goto End;
    }
    int tmtotmon = tm.tm_year *12 + tm.tm_mon;

    int ltmtotmon = ((ltm.tm_year + 1900) % 100)*12 + ltm.tm_mon + 1;
    OSI_LOGI(0x10009b81, "[CIOT]UNICOM Self-Register system time %d-%d-%d  current time %d-%d-%d\n",((ltm.tm_year + 1900) % 100), ltm.tm_mon + 1, ltm.tm_mday, tm.tm_year, tm.tm_mon, tm.tm_mday);
    if(ltmtotmon > tmtotmon)
    {
        regedtimer = 1;
    }
    else if(ltmtotmon == tmtotmon)
    {
         if(ltm.tm_mday >= tm.tm_mday)
         {
             regedtimer = 1;
         }
         else
         {
             OSI_LOGI(0x10009b82, "[CIOT]UNICOM Self-Register ltm.tm_mday < tm.tm_mday!!\n");
             regedtimer = 0;
             setstartreg_time(&tm);
         }
    }
    else
    {
        OSI_LOGI(0x10009b83, "[CIOT]UNICOM Self-Register ltmtotmon < tmtotmon!!\n");
        regedtimer = 0;
        setstartreg_time(&tm);
    }
    return regedtimer;

End:
    return -1;

}

uint8_t  get_regflag()
{
    char flagStr[20] = {0};
    uint8_t flag = 0;
    int fd = vfs_open(NV_NB_SELFREG_FLAG, 2);
    if (fd < 0)
    {
        OSI_LOGI(0x10009b84, "[CIOT]CT/UNICOM Self-Register get_regflag vfs_open failed\n");
        vfs_file_write(NV_NB_SELFREG_FLAG, "0", 1);
     }
     else
     {
        OSI_LOGI(0x10009b85, "[CIOT]CT/UNICOM Self-Register get_regflag open success\n");
        struct stat st = {};
        vfs_fstat(fd, &st);
        int file_size = st.st_size;
        if(file_size == 1)
        {
            OSI_LOGI(0x10009b86, "[CIOT]CT/UNICOM Self-Register flag_file_size = 1\n");
            vfs_read(fd, flagStr, 1);
            flag = flagStr[0] - '0';
            vfs_close(fd);
            return flag;
        }
        else
        {
            OSI_LOGI(0x10009b87, "[CIOT]CT/UNICOM Self-Register flag_file_size error\n");
            vfs_unlink(NV_NB_SELFREG_FLAG);
            vfs_file_write(NV_NB_SELFREG_FLAG, "0", 1);
        }
     }
     return 0;
}
void  coap_register_init()
{
    OSI_LOGI(0x10009b88, "[CIOT]CT/UNICOM Self-Register reg_init\n");
    nbselfreg_flag = get_regflag();
    if(nbselfreg_flag == 2)
        UnicomRegInit();
}
#else
extern bool fupdateIsBootAfterFota(void);
void  coap_register_init(int flag)
{
    OSI_LOGI(0x10009b88, "[CIOT]CT/UNICOM Self-Register reg_init\n");
    nbselfreg_flag = flag;
    bool futa_status = fupdateIsBootAfterFota();
    if (futa_status == true)
    {
        OSI_LOGI(0x10009b89, "[CIOT]CT/UNICOM Self-Register ota finish\n");
        vfs_unlink(NV_NB_COAP_SELF_REG);
        vfs_file_write(NV_NB_COAP_SELF_REG, "0", 1);
    }
}
#endif

static bool isResultOk(reg_ctrl_t *reg_ctrl)
{
    return reg_ctrl->resultCode == 0 && strcmp((char *)reg_ctrl->resultDesc,"Success") == 0;
}

static void parseJson(char *pMsg, reg_ctrl_t *reg_ctrl)
{
    if (NULL == pMsg)
    {
        OSI_LOGI(0x10009b8a, "[CIOT]CT/UNICOM Self-Register pMsg is NULL!\n");
        return;
    }
    cJSON *pJson = cJSON_Parse(pMsg);
    if (NULL == pJson)
    {
        OSI_LOGI(0x10009b8b, "[CIOT]CT/UNICOM Self-Register pJson is NULL!\n");
        return;
    }
    char *regdatajs = cJSON_PrintUnformatted(pJson);
    OSI_LOGXI(OSI_LOGPAR_S, 0x10009b8c, "[CIOT]CT/UNICOM Self-Register parseJson: %s\n", regdatajs);

    cJSON *pSub = cJSON_GetObjectItem(pJson, "resultCode");
    if (NULL != pSub)
    {
        OSI_LOGXI(OSI_LOGPAR_S, 0x10009b8d, "[CIOT]CT/UNICOM Self-Register resultCode: %s\n", pSub->valuestring);
        reg_ctrl->resultCode = atoi(pSub->valuestring);
    }

    pSub = cJSON_GetObjectItem(pJson, "resultDesc");
    if (NULL != pSub)
    {
        OSI_LOGXI(OSI_LOGPAR_S, 0x10009b8e, "[CIOT]CT/UNICOM Self-Register resultDesc: %s\n", pSub->valuestring);
        strncpy((char *)reg_ctrl->resultDesc,pSub->valuestring,10);
    }
     free(regdatajs);
}


static char *genRegData(uint8_t simid)
{
    unsigned char *buffer;
    size_t buffer_len;
    size_t len;
    size_t out_len;
    cJSON *pJsonRoot = NULL;
    pJsonRoot = cJSON_CreateObject();
    if (NULL == pJsonRoot)
        return NULL;
    uint8_t imei[16] = {0};
    uint8_t imsi[16] = {0};
    uint8_t simiccid[21] = {0};
    uint8_t imsisize = 16;
    uint8_t iccid_len = 20;

    getSimIccid(simid,simiccid,&iccid_len);
    getSimImsi(simid,imsi,&imsisize);
    int nImeiLen = nvmReadImeiStr(simid, (nvmImeiStr_t *)imei);
    if (nImeiLen == -1)
         OSI_LOGI(0x10009b8f, "[CIOT]CT/UNICOM Self-Register nvmReadImei read error...\n");
    OSI_LOGXI(OSI_LOGPAR_S, 0x10009b90, "[CIOT]CT/UNICOM Self-Register NvmReadImeigetSimImei:%s\n", imei);
    cJSON_AddStringToObject(pJsonRoot, "REGVER", "2");
    cJSON_AddStringToObject(pJsonRoot, "MEID", (const char *)imei);
    if(reg_ctrl->OperatorFlag)
        cJSON_AddStringToObject(pJsonRoot, "MODEL", "BJS-BC60");
    else
        cJSON_AddStringToObject(pJsonRoot, "MODELSMS", "BJS-BC60");
    cJSON_AddStringToObject(pJsonRoot, "SWVER", SOFTWARE_VER_NUM);    //软件版本号
    cJSON_AddStringToObject(pJsonRoot, "SIM1ICCID", (const char *)simiccid);
    cJSON_AddStringToObject(pJsonRoot, "SIM1LTEIMSI", (const char *)imsi);

    char *regdatajs = cJSON_PrintUnformatted(pJsonRoot);

    if (regdatajs == NULL)
    {
        OSI_LOGI(0x10009b91, "[CIOT]CT/UNICOM Self-Register JsonString error...\n");
        cJSON_Delete(pJsonRoot);
        return NULL;
    }
    else
        OSI_LOGXI(OSI_LOGPAR_S, 0x10009b92, "[CIOT]CT/UNICOM Self-Register genRegData :%s\n", regdatajs);
    out_len = strlen(regdatajs);

    mbedtls_base64_encode( NULL, 0, &buffer_len, (const unsigned char *)regdatajs, out_len);
    buffer = (unsigned char *)malloc(buffer_len);

    if (buffer != NULL && mbedtls_base64_encode(buffer, buffer_len, &len, (const unsigned char *)regdatajs, out_len) != 0)
    {
        OSI_LOGI(0x10009b93, "[CIOT]CT/UNICOM Self-Register base64 error...\n");
    }
    free(regdatajs);
    cJSON_Delete(pJsonRoot);
    return (char *)buffer;
}


void coapreg_timeout(void *param)
{
    OSI_LOGI(0x10009b94, "[CIOT]CT/UNICOM Self-Register timeout retryCount: %d\n", reg_ctrl->retryCount);
    do_dxnbregProcess(reg_ctrl->nCid,  reg_ctrl->simId);
}
static void coap_user_cb(coap_client_t *client, coap_method_t method, bool result)
{
    OSI_LOGI(0x10009b95, "[CIOT]CT/UNICOM Self-Register coap_user_cb (%d):%d\n", method, result);
#ifdef CONFIG_NBSELFREG_CUS_SUPPORT
    interal_timer_flag = 0;
#endif
    //reg_ctrl_t *reg_ctrl = (reg_ctrl_t *)coap_client_getUserData(client);
    if (result == true && isResultOk(reg_ctrl))
    {
        uint8_t simiccid[UEICCID_LEN+1] = {0};
        uint8_t iccid_len = UEICCID_LEN;
        reg_ctrl->status = coap_client_getStatus(client);
        if (NULL != gcoapregtimer)
        {
            osiTimerStop(gcoapregtimer);
            osiTimerDelete(gcoapregtimer);
            gcoapregtimer = NULL;
        }
	 OSI_LOGI(0x10009b96, "[CIOT]CT/UNICOM Self-Register succuss and save ICCID\n");
        getSimIccid(reg_ctrl->simId,simiccid,&iccid_len);
        vfs_file_write(NV_NB_COAP_SELF_REG, simiccid, 20);
        reg_ctrl->retryCount = 0;
#ifdef CONFIG_NBSELFREG_CUS_SUPPORT
        if(reg_ctrl->OperatorFlag == 0)
            timereg_get_save();
#endif
    }
    else if (reg_ctrl->retryCount < 3)
    {
        reg_ctrl->retryCount++;
        gcoapregtimer = osiTimerCreate(netGetTaskID(), coapreg_timeout, NULL);
        if (gcoapregtimer == NULL)
        {
            OSI_LOGI(0x10009b97, "[CIOT]CT/UNICOM Self-Register create timer failed\n");
            is_dxnbreg_inited = false;
            g_bCoapDoing = false;
            if(NULL != client)
                coap_client_free(client);
            return;
        }
        osiTimerStart(gcoapregtimer, RETRY_REGTIMER);
    }
    else
    {
        if (NULL != gcoapregtimer)
        {
            osiTimerStop(gcoapregtimer);
            osiTimerDelete(gcoapregtimer);
            gcoapregtimer = NULL;
        }
        reg_ctrl->retryCount = 0;
    }
    is_dxnbreg_inited = false;
    coap_client_free(client);
    g_bCoapDoing = false;
#ifdef CONFIG_NBSELFREG_CUS_SUPPORT
        if(reg_ctrl->OperatorFlag == 0)
            timereg_get_save();
#endif
}

static void coap_incoming_data_cb(coap_client_t *client, const unsigned char *data, short unsigned int len, unsigned char num)
{
    OSI_LOGXI(OSI_LOGPAR_IIS, 0x10009b98, "[CIOT]CT/UNICOM Self-Register incoming_data(%d:%d): %s\n", num, len, data);
    //reg_ctrl = (reg_ctrl_t *)coap_client_getUserData(client);
    parseJson((char *)data, reg_ctrl);
}



static void do_dxnbregProcess(uint8_t nCid, uint8_t nSimId)
{
     char *regdata = NULL;
     bool resparam = true;
     if(g_bCoapDoing == true)
     {
           OSI_LOGI(0x10009b99, "[CIOT]CT/UNICOM Self-Register start ...\n");
           goto retry;
     }
     g_bCoapDoing = true;
     if (netif_default == NULL)
        goto retry;
     regdata = genRegData(nSimId);
     if (NULL == regdata)
     {
         OSI_LOGI(0x10009b9a, "[CIOT]CT/UNICOM Self-Register regdata error...\n");
         goto retry;
     }
     coap_client_t *client = coap_client_new(netGetTaskID(), coap_user_cb, NULL);
     if (client == NULL)
     {
        OSI_LOGI(0x10009b9b, "[CIOT]CT/UNICOM Self-Register create client error...\n");
        free(regdata);
        goto retry;
     }
    reg_ctrl->nCid = nCid;
    reg_ctrl->simId = nSimId;
    reg_ctrl->status = coap_client_getStatus(client);
    coap_client_setParams(client, ((unsigned char *)("-t application/json")));
    coap_client_setParams(client, ((unsigned char *)("-p 5683")));
    coap_client_setIndataCbk(client, ((coap_incoming_data_cb_t)coap_incoming_data_cb));
    coap_client_setPayload(client, (unsigned char *)regdata, strlen(regdata));
#ifdef CONFIG_NBSELFREG_CUS_SUPPORT
    interal_timer_flag = 1;
#endif
    if(reg_ctrl->OperatorFlag)
        resparam = coap_async_post(client, ((unsigned char *)("coap://zzhc.vnet.cn:5683/nb")), NULL, true); //tele_dx_self_register
    else
        resparam = coap_async_post(client,((unsigned char *)("coap://114.255.193.236:5683/")),NULL,true); //unicom self register
    reg_ctrl->status = coap_client_getStatus(client);
    if(false == resparam)
    {
        g_bCoapDoing = false;
        is_dxnbreg_inited = false;
        coap_client_free(client);
#ifdef CONFIG_NBSELFREG_CUS_SUPPORT
        interal_timer_flag = 0;
        free(regdata);
        goto retry;
#endif
    }
    free(regdata);
    return;

retry:
    if (reg_ctrl->retryCount < 3)
    {
        reg_ctrl->retryCount++;
        gcoapregtimer = osiTimerCreate(netGetTaskID(), coapreg_timeout, NULL);
        if (gcoapregtimer == NULL)
        {
            OSI_LOGI(0x10009b9c, "[CIOT]CT/UNICOM Self-Register Timer create failed++\n");
            is_dxnbreg_inited = false;
            g_bCoapDoing = false;
	     return;
        }
        osiTimerStart(gcoapregtimer, RETRY_REGTIMER);
    }
    else
    {
        if (NULL != gcoapregtimer)
        {
            osiTimerStop(gcoapregtimer);
            osiTimerDelete(gcoapregtimer);
            gcoapregtimer = NULL;
        }
        reg_ctrl->retryCount = 0;
    }
    is_dxnbreg_inited = false;
    g_bCoapDoing = false;
#ifdef CONFIG_NBSELFREG_CUS_SUPPORT
    if(reg_ctrl->OperatorFlag == 0)
        timereg_get_save();
#endif
    return;
}

bool sul_ZeroMemory8(void  *pBuf, uint32_t count)
{
    char *xs = pBuf;
    while (count--)
        *xs++ = 0x0;
    return 1;
}

int con_operator(uint8_t nSimId)
{
    uint8_t tmpMode = 0;
    uint8_t operatorId[6];
    uint8_t *pOperName = NULL;
    uint8_t *pOperShortName = NULL;
    int  operater_flag = -1;
    uint8_t  nRet = 0;

    sul_ZeroMemory8(operatorId, 6);
    nRet = CFW_NwGetCurrentOperator(operatorId, &tmpMode, nSimId);
    if (nRet != 0)
    {
        is_dxnbreg_inited = false;
        return -1;
    }
    OSI_LOGXI(OSI_LOGPAR_IS, 0x10009b9d, "[CIOT]CT/UNICOM Self-Register operatorId=%d %s\n", tmpMode, operatorId);
    nRet =  CFW_CfgNwGetOperatorName(operatorId, &pOperName, &pOperShortName);
    if (nRet != 0)
    {
        is_dxnbreg_inited= false;
        return -1;
    }
    OSI_LOGXI(OSI_LOGPAR_S, 0x10009b9e, "[CIOT]CT/UNICOM Self-Register pOperName=%s\n", pOperName);
    if ((0 == strcmp("ChinaTelecom", (char*)pOperName))||(0 == strcmp("CHN-CT", (char*)pOperName)))
    {
        operater_flag = 1;
        OSI_LOGI(0x10009b9f, "[CIOT]CT/UNICOM Self-Register is CTCC: %d\n", operater_flag);
    }
    else if(0 == strcmp("CHN-UNICOM", (char*)pOperName))
    {
        operater_flag = 0;
        OSI_LOGI(0x10009ba0, "[CIOT]CT/UNICOM Self-Register is China Unicom: %d\n", operater_flag);
    }
    else
    {
       operater_flag = -1;
       OSI_LOGI(0x10009ba1, "[CIOT]CT/UNICOM Self-Register operator cannot self-register: %d\n", operater_flag);
    }
    return operater_flag;

}

int vnetNBSelfRegister(uint8_t nCid, uint8_t nSimId)
{
#ifdef CONFIG_NBSELFREG_CUS_SUPPORT
    int count = drvRtcGetAlarmCount(AT_UNREG);
    if (count != 0)
        drvRtcRemoveAlarm(AT_UNREG, 1);
#endif
    if(nbselfreg_flag == 0)
    {
         OSI_LOGI(0x10009ba2, "[CIOT]CT/UNICOM Self-Register closed#exit flag=%d\n", nbselfreg_flag);
         return -1;
    }

    uint8_t ueiccid[UEICCID_LEN + 1] = {0};
    int operater_flag = -1;
    OSI_LOGI(0x10009ba3, "[CIOT]CT/UNICOM Self-Register entering\n");
    if (gcoapregtimer != NULL)         //上次未做完需要停掉
    {
        OSI_LOGI(0x10009ba4, "[CIOT]CT/UNICOM Self-Register#gcoapregtimer not stop\n");
        osiTimerStop(gcoapregtimer);
        osiTimerDelete(gcoapregtimer);
        gcoapregtimer = NULL;
        is_dxnbreg_inited = false;
    }
    if(is_dxnbreg_inited)
    {
        OSI_LOGI(0x10009ba5, "[CIOT]CT/UNICOM Self-Register is in process to exit this connection\n");
        return 0;
    }
    is_dxnbreg_inited = true;

    operater_flag = con_operator(nSimId);
#ifdef CONFIG_NBSELFREG_CUS_SUPPORT
    if((operater_flag == 1 && nbselfreg_flag == 1) || (operater_flag == 0 && nbselfreg_flag == 2))
#else
    if(operater_flag == 1 || operater_flag == 0)
#endif
    {
        uint8_t simiccid[21] = {0};
        uint8_t iccid_len = UEICCID_LEN;
        if (!getSimIccid(nSimId,simiccid,&iccid_len))
        {
            is_dxnbreg_inited = false;
            return -1;
        }
        int fd = vfs_open(NV_NB_COAP_SELF_REG, 2);
        if (fd < 0)
        {
            OSI_LOGI(0x10009ba6, "[CIOT]CT/UNICOM Self-Register failed to open file %d\n", fd);
            vfs_file_write(NV_NB_COAP_SELF_REG, "0", 1);
        }
        else
        {
            OSI_LOGI(0x10009ba7, "[CIOT]CT/UNICOM Self-Register opened the file successfully\n");
            struct stat st = {};
            vfs_fstat(fd, &st);
            int file_size = st.st_size;
            if(file_size == 1 || file_size == 20)
            {
                OSI_LOGI(0x10009ba8, "[CIOT]CT/UNICOM Self-Register ICCID_SAVE_file_size is %d\n", file_size);
                vfs_read(fd, ueiccid, 20);
                OSI_LOGXI(OSI_LOGPAR_S, 0x10009ba9, "[CIOT]CT/UNICOM Self-Register ueiccid:%s\n", ueiccid);
                vfs_close(fd);
            }
            else
            {
                OSI_LOGI(0x10009baa, "[CIOT]CT/UNICOM Self-Register ICCID_SAVE_file_size error:%d\n", file_size);
                vfs_unlink(NV_NB_COAP_SELF_REG);
                vfs_file_write(NV_NB_COAP_SELF_REG, "0", 1);
            }

        }
        // NV_GetUEIccid(ueiccid,iccid_len,simId);    //读flag   比较iccid
        if (memcmp(ueiccid, simiccid,iccid_len) == 0)
        {
#ifdef CONFIG_NBSELFREG_CUS_SUPPORT
             //OSI_LOGI(0, "[CIOT]CT/UNICOM Self-Register ueiccid same as simiccid register success\n");
             if(operater_flag == 0)
             {
                   if( reg_ctrl != NULL)
                   {
                        free(reg_ctrl);
                        reg_ctrl = NULL;
                   }
                   reg_ctrl = (reg_ctrl_t *)malloc(sizeof(reg_ctrl_t));
                   if(NULL == reg_ctrl)
                   {
                       OSI_LOGI(0x10009bab, "[CIOT]CT/UNICOM Self-Register reg_ctrl malloc error\n");
                       is_dxnbreg_inited = false;
                       return -1;
                   }
                   reg_ctrl->retryCount = 0;
                   reg_ctrl->OperatorFlag = operater_flag;
                   int regtimer = time_roundfind();
                   if(regtimer == 1)
                   {
                        OSI_LOGI(0x10009bac, "[CIOT]CT/UNICOM Self-Register initiated due to more than 30 days\n");
                        do_dxnbregProcess(nCid, nSimId);
                        return 1;
                   }
                   reg_ctrl->nCid = nCid;
                   reg_ctrl->simId = nSimId;

             }
#endif
             is_dxnbreg_inited = false;
             OSI_LOGI(0x10009bad, "[CIOT]CT/UNICOM Self-Register ueiccid same as simiccid register success\n");
             return 0;
        }
        else
        {
            OSI_LOGXI(OSI_LOGPAR_S, 0x10009bae, "[CIOT]CT/UNICOM Self-Register simiccid: %s\n", simiccid);
            reg_ctrl = (reg_ctrl_t *)malloc(sizeof(reg_ctrl_t));
            if(NULL == reg_ctrl)
            {
                OSI_LOGI(0x10009baf, "[CIOT]CT/UNICOM Self-Register reg_ctrl malloc error");
                is_dxnbreg_inited = false;
                return -1;
            }
            reg_ctrl->retryCount = 0;
            reg_ctrl->OperatorFlag = operater_flag;
            do_dxnbregProcess(nCid, nSimId);
        }
        return 1;
    }
    else
    {
        OSI_LOGI(0x10009bb0, "[CIOT]CT/UNICOM Self-Register operator platform is not CT/UNICOM");
        is_dxnbreg_inited = false;
        return 0;
    }
}
