/* Copyright (C) 2022 FIBOCOM Technologies Limited and/or its affiliates("FIBOCOM").
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


#define OSI_LOG_TAG OSI_MAKE_LOG_TAG('H', 'T', 'T', 'P')

#include "fibo_opencpu.h"
#include "oc_gprs.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <stdint.h>
static UINT32 g_test_sem;
static void prvInvokeGlobalCtors(void)
{
    extern void (*__init_array_start[])();
    extern void (*__init_array_end[])();

    size_t count = __init_array_end - __init_array_start;
    for (size_t i = 0; i < count; ++i)
        __init_array_start[i]();
}

INT32 http_file_print(const INT8 *pPath)
{
    INT32 fd = 0;
    UINT8 aBuff[1024] = {0};
    INT32 ret = 0;

    memset(aBuff, 0, 1024);
    fd = fibo_file_open(pPath, FS_O_RDONLY);
    if(fd == -1)
    {
        fibo_textTrace("%s,%d http file read error", __FUNCTION__, __LINE__);
        return 1;
    }

    ret = fibo_file_read(fd, aBuff, 1024);
    if(ret == -1)
    {
        fibo_textTrace("%s,%d http file read error", __FUNCTION__, __LINE__);
        fibo_file_close(fd);
        return ret;
    }

    fibo_file_close(fd);

    fibo_textTrace("%s,%d http len %d data %s", __FUNCTION__, __LINE__, strlen((const char *)aBuff), aBuff);
    return 0;
}

void read_header_cb(const int8_t *name, const int8_t *value)
{
    fibo_textTrace("@@@ HTTP response Header: %s : %s", name, value);
}

void read_body_cb(void *pHttpParam, int8_t *data, int32_t len)
{
    fibo_textTrace("@@@ HTTP response code          : %d", ((oc_http_param_t *)pHttpParam)->respCode);
    fibo_textTrace("@@@ HTTP response version       : %s", ((oc_http_param_t *)pHttpParam)->respVersion);
    fibo_textTrace("@@@ HTTP response reason phrase : %s", ((oc_http_param_t *)pHttpParam)->respReasonPhrase);
    fibo_textTrace("@@@ HTTP Recive Data : %d %s", len, data);
}

static void pdp_thread_entry(void *param)
{
    fibo_textTrace("application thread enter, param 0x%x", param);
    //const char *url = "http://182.92.122.159/1K.txt";
    const char url[] = "http://111.231.250.105";
    const char filepath[] = "/http/test";
    oc_http_param_t * pstHttpParam = NULL;
    INT32 i = 0;
    INT32 ret = 1;
    reg_info_t reg_info;
    fibo_pdp_profile_t pdp_profile;

    memset(&pdp_profile, 0, sizeof(fibo_pdp_profile_t));
    pdp_profile.cid = 1;
    memcpy(pdp_profile.nPdpType, "IP", strlen("IP"));

    const char Post_Head[] = "POST / HTTP/1.1\r\nAccept: application/json\r\nConnection: close\r\nContent-Type: application/json\r\nContent-Length: 29\r\nHost: 111.231.250.105\r\n\r\n";
    const char Post_Data[] = "{\"device_mac\":\"121212121212\"}";
    fibo_http_read_body_callback(read_body_cb);
    fibo_http_read_header_callback(read_header_cb);
    g_test_sem = fibo_sem_new(0);
    while(1)
    {
        fibo_reg_info_get(&reg_info, 0);
        fibo_taskSleep(1000);
        fibo_textTrace("[%s-%d]", __FUNCTION__, __LINE__);
        if (reg_info.nStatus == 1)
        {
            //ipv4
            fibo_pdp_active(1, &pdp_profile, 0);
            fibo_taskSleep(3000);
            break;
        }
    }

    for(i = 0; i < 1; i++)
    {
        /*=================================================*/
        /*         http head test, use user callback       */
        /*=================================================*/
        pstHttpParam = fibo_http_new();
        if(NULL == pstHttpParam)
        {
            goto EXIT;
        }
        /* prepare http get param */
        pstHttpParam->timeout = 30;
        pstHttpParam->respCode = 500;
        pstHttpParam->enHttpReadMethod = OC_USER_CALLBACK;
        if(strlen(filepath) <= OC_HTTP_FILE_PATH_LEN)
        {
            strncpy((char *)pstHttpParam->filepath, filepath, strlen(filepath));
        }

        memset(pstHttpParam->url, 0, OC_HTTP_URL_LEN + 1);
        if(strlen(url) <= OC_HTTP_URL_LEN)
        {
            strncpy((char *)pstHttpParam->url, url,strlen(url));
        }
        else
        {
            goto EXIT;
        }
        ret = fibo_http_head(pstHttpParam, NULL);
        if(ret != 0)
        {
            goto EXIT;
        }
        fibo_sem_wait(g_test_sem);
        if(OC_SAVE_FILE == pstHttpParam->enHttpReadMethod)
        {
            //http_file_print(filepath);
        }
        fibo_http_delete(pstHttpParam);
        pstHttpParam = NULL;

        /*=================================================*/
        /*        http get test, use user callback         */
        /*=================================================*/
        fibo_taskSleep(1000);
        pstHttpParam = fibo_http_new();
        if(NULL == pstHttpParam)
        {
            goto EXIT;
        }
        /* prepare http get param */
        pstHttpParam->timeout = 30;
        pstHttpParam->respCode = 500;
        pstHttpParam->enHttpReadMethod = OC_USER_CALLBACK;
        if(strlen(filepath) <= OC_HTTP_FILE_PATH_LEN)
        {
            strncpy((char *)pstHttpParam->filepath, filepath, strlen(filepath));
        }

        memset(pstHttpParam->url, 0, OC_HTTP_URL_LEN + 1);
        if(strlen(url) <= OC_HTTP_URL_LEN)
        {
            strncpy((char *)pstHttpParam->url, url, strlen(url));
        }
        else
        {
            goto EXIT;
        }
        ret = fibo_http_get(pstHttpParam, NULL);
        if(ret != 0)
        {
            goto EXIT;
        }
        fibo_sem_wait(g_test_sem);
        if(OC_SAVE_FILE == pstHttpParam->enHttpReadMethod)
        {
            //http_file_print(filepath);
        }
        fibo_http_delete(pstHttpParam);
        pstHttpParam = NULL;

        /*=================================================*/
        /*           http post test, save to file          */
        /*=================================================*/
        fibo_taskSleep(1000);
        /* http post */
        pstHttpParam = fibo_http_new();
        if(NULL == pstHttpParam)
        {
            goto EXIT;
        }
        /* prepare http get param */
        pstHttpParam->timeout = 30;
        pstHttpParam->respCode = 500;
        pstHttpParam->enHttpReadMethod = OC_SAVE_FILE;
        if(strlen(filepath) <= OC_HTTP_FILE_PATH_LEN)
        {
            strncpy((char *)pstHttpParam->filepath, filepath, strlen(filepath));
        }

        memset(pstHttpParam->url, 0, OC_HTTP_URL_LEN + 1);
        if(strlen((const char *)url) <= OC_HTTP_URL_LEN)
        {
            strncpy((char *)pstHttpParam->url, url, strlen(url));
        }
        else
        {
            goto EXIT;
        }
        ret= fibo_http_post(pstHttpParam, (uint8_t *)Post_Data, (uint8_t*)Post_Head);
        if(ret != 0)
        {
            goto EXIT;
        }
        fibo_sem_wait(g_test_sem);
        if(OC_SAVE_FILE == pstHttpParam->enHttpReadMethod)
        {
            //http_file_print(filepath);
        }
        fibo_http_delete(pstHttpParam);
        pstHttpParam = NULL;
    }
EXIT:
    fibo_http_delete(pstHttpParam);
    fibo_thread_delete();
    fibo_sem_free(g_test_sem);
}

static void sig_res_callback(GAPP_SIGNAL_ID_T sig, va_list arg)
{
    switch (sig)
    {
        case GAPP_SIG_HTTP_REQUEST_END:
        {
            fibo_sem_signal(g_test_sem);
            fibo_textTrace("http request finish sig: %d",sig);
            break;
        }
        case GAPP_SIG_HTTP_REQUEST_ERROR:
        {
            fibo_sem_signal(g_test_sem);
            fibo_textTrace("http request error sig: %d",sig);
            break;
        }
        case GAPP_SIG_HTTP_REQUEST_CONNECT:
            fibo_textTrace("http request connect sig: %d",sig);
            break;
        case GAPP_SIG_HTTP_REQUEST_RES:
            fibo_textTrace("http request res sig: %d",sig);
            break;
        default:
            break;
    }
}
static FIBO_CALLBACK_T user_callback = {
    .fibo_signal = sig_res_callback};

void *appimg_enter(void *param)
{
    fibo_textTrace("application image enter, param 0x%x", param);
    prvInvokeGlobalCtors();
    fibo_thread_create(pdp_thread_entry, "mythread", 1024 * 16, NULL, OSI_PRIORITY_NORMAL);
    return (void *)&user_callback;
}

void appimg_exit(void)
{
    fibo_textTrace("application image exit");
}
