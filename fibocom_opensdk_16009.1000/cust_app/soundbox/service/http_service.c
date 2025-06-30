#include "http_service.h"
#include "fb_uart.h"

#include "event_signal.h"

#include "log_service.h"

HTTP_STATUS_TYPE_ agnss_http_download_status=HTTP_DEFAULT_STA;


static void cb_http_response_handle(void *pHttpParam, INT8 *data, INT32 len)
{
    fibo_taskSleep(1000);
    fibo_http_response_status_line((oc_http_param_t *)pHttpParam);

    fb_printf("@@@ HTTP response code          : %d\r\n", ((oc_http_param_t *)pHttpParam)->respCode);
    fb_printf("@@@ HTTP response version       : %s\r\n", ((oc_http_param_t *)pHttpParam)->respVersion);
    fb_printf("@@@ HTTP response reason phrase : %s\r\n", ((oc_http_param_t *)pHttpParam)->respReasonPhrase);

    fb_printf("@@@ HTTP Recive Data : %d %s", len, data);
}

static void cb_http_header_handle(const UINT8 *name, const UINT8 *value)
{
    fb_printf(" http Header callback: %s : %s\r\n", name, value);
}

//post save file    有些服务器不支持此方式    建议用get
#if 0

int http_download_file(char *url,char *local_file_path,int *download_file_size)
{
    int ret;
    int result=-1;
    int http_download_sta=HTTP_DEFAULT_STA;

    oc_http_param_t * pstHttpParam = NULL;

    fb_printf("http application thread enter \r\n");
    fibo_taskSleep(1000);
    /* http post */
    pstHttpParam = fibo_http_new();
    if(NULL == pstHttpParam)
    {
        goto exit;
    }
        /* c*/
    pstHttpParam->timeout = 30;
    pstHttpParam->cbReadBody = cb_http_response_handle;
    pstHttpParam->respCode = 500;
    pstHttpParam->bIsIpv6 = false;
    pstHttpParam->enHttpReadMethod = OC_SAVE_FILE;
    if(strlen(local_file_path) <= OC_HTTP_FILE_PATH_LEN)
    {
        strncpy((char *)pstHttpParam->filepath, local_file_path, strlen(local_file_path));
    }

    memset(pstHttpParam->url, 0, OC_HTTP_URL_LEN + 1);
    if(strlen((const char *)url) <= OC_HTTP_URL_LEN)
    {
        strncpy((char *)pstHttpParam->url, url, strlen(url));
    }
    else
    {
        goto exit;
    }
    ret=fibo_http_post(pstHttpParam, (UINT8 *)"000000", NULL);
    fb_printf("HTTP fibo_http_post      ret: %d\r\n", ret);
    if(ret!=0){
        fb_printf("HTTP fibo_http_post  exit\r\n");
            // http_download_status=HTTP_POST_FAIL;
        goto exit;
    }

    fibo_taskSleep(100);
    fb_printf("HTTP response code          : %d\r\n", pstHttpParam->respCode);
    fb_printf("HTTP response version       : %s\r\n", pstHttpParam->respVersion);
    fb_printf("HTTP response reason phrase : %s\r\n", pstHttpParam->respReasonPhrase);
    fb_printf("HTTP response body len      : %d\r\n", pstHttpParam->respContentLen);
    fb_printf("HTTP fibo_http_post      ret: %d\r\n", ret);
    fibo_taskSleep(100);
    if(strstr(pstHttpParam->respReasonPhrase,"OK"))
    {
        fb_printf("find file  ok\r\n");
        *download_file_size=pstHttpParam->respContentLen;
        http_download_sta=HTTP_FIND_FILE;
    }
    else
    {
        http_download_sta=HTTP_NO_FIND_FILE;
        result=-1;
        goto exit;
    }

    fb_printf("HTTP fibo_http_response_header_foreach00\r\n");
    fibo_taskSleep(100);
    ret=fibo_http_response_header_foreach(pstHttpParam, cb_http_header_handle);
    fb_printf("HTTP fibo_http_response_header_foreach      ret: %d\r\n", ret);
    fibo_taskSleep(100);
    if(http_download_sta==HTTP_FIND_FILE)
    {
        http_download_sta=HTTP_DOWLOAD_SUCCESS;
        fb_printf("agps get_agps_file_time:%d",get_agps_file_time());
        //save download time
        result=0;
    }
    else
    {
        http_download_sta=HTTP_DOWLOAD_FAIL;
        result=-1;
    }
    fb_printf("HTTP download end\r\n"); 
    fibo_taskSleep(100);

    exit:
    fibo_http_delete(pstHttpParam);
    return result;
}
#else
int http_download_file(char *url,char *local_file_path,int *download_file_size)
{
    int ret;
    int result=-1;
    int http_download_sta=HTTP_DEFAULT_STA;

    oc_http_param_t * pstHttpParam = NULL;

    fb_printf( "http application thread enter \r\n");
    fibo_taskSleep(1000);
    /* http get */
    pstHttpParam = fibo_http_new();
    if(NULL == pstHttpParam)
    {
        goto exit;
    }
        /* c*/
    pstHttpParam->timeout = 30;
    pstHttpParam->cbReadBody = cb_http_response_handle;
    pstHttpParam->respCode = 500;
    pstHttpParam->bIsIpv6 = false;
    pstHttpParam->enHttpReadMethod = OC_SAVE_FILE;
    if(strlen(local_file_path) <= OC_HTTP_FILE_PATH_LEN)
    {
        strncpy((char *)pstHttpParam->filepath, local_file_path, strlen(local_file_path));
    }

    memset(pstHttpParam->url, 0, OC_HTTP_URL_LEN + 1);
    if(strlen((const char *)url) <= OC_HTTP_URL_LEN)
    {
        strncpy((char *)pstHttpParam->url, url, strlen(url));
    }
    else
    {
        goto exit;
    }
    ret=fibo_http_get(pstHttpParam, NULL);

    fb_printf("HTTP fibo_http_get      ret: %d\r\n", ret);
    if(ret!=0){
        fb_printf("HTTP fibo_http_get  exit\r\n");
            // http_download_status=HTTP_POST_FAIL;
        goto exit;
    }

    fibo_taskSleep(100);
    fb_printf("HTTP response code          : %d\r\n", pstHttpParam->respCode);
    fb_printf("HTTP response version       : %s\r\n", pstHttpParam->respVersion);
    fb_printf("HTTP response reason phrase : %s\r\n", pstHttpParam->respReasonPhrase);
    fb_printf("HTTP response body len      : %d\r\n", pstHttpParam->respContentLen);
    fb_printf("HTTP fibo_http_get      ret: %d\r\n", ret);
    fibo_taskSleep(100);
    if(strstr((char *)pstHttpParam->respReasonPhrase,"OK"))
    {
        fb_printf("find file  ok\r\n");
        *download_file_size=pstHttpParam->respContentLen;
        http_download_sta=HTTP_FIND_FILE;
    }
    else
    {
        http_download_sta=HTTP_NO_FIND_FILE;
        result=-1;
        goto exit;
    }

    fb_printf("HTTP fibo_http_response_header_foreach00\r\n");
    fibo_taskSleep(100);
    ret=fibo_http_response_header_foreach(pstHttpParam, cb_http_header_handle);
    fb_printf("HTTP fibo_http_response_header_foreach      ret: %d\r\n", ret);
    fibo_taskSleep(100);
    if(http_download_sta==HTTP_FIND_FILE)
    {
        http_download_sta=HTTP_DOWLOAD_SUCCESS;
        //save download time
        result=0;
    }
    else
    {
        http_download_sta=HTTP_DOWLOAD_FAIL;
        result=-1;
    }
    fb_printf("HTTP download end\r\n"); 
    fibo_taskSleep(100);

    exit:
    fibo_http_delete(pstHttpParam);
    return result;
}
#endif
void cb_agnss_http_response_handle(void *pHttpParam, INT8 *data, INT32 len)
{
    fibo_taskSleep(1000);
    fibo_http_response_status_line((oc_http_param_t *)pHttpParam);

    fb_printf("@@@ http  test HTTP response code          : %d\r\n", ((oc_http_param_t *)pHttpParam)->respCode);
    fb_printf("@@@ http  test HTTP response version       : %s\r\n", ((oc_http_param_t *)pHttpParam)->respVersion);
    fb_printf("@@@ http  test HTTP response reason phrase : %s\r\n", ((oc_http_param_t *)pHttpParam)->respReasonPhrase);
    fb_printf("@@@ http  test HTTP Recive Data : %d %s", len, data);
}

void cb_agnss_http_header_handle(const UINT8 *name, const UINT8 *value)
{
    fb_printf(" http  test http Header callback: %s : %s\r\n", name, value);
}



HTTP_STATUS_TYPE_ get_agnss_http_status(void)
{
    return agnss_http_download_status;
}
void set_agnss_http_status(HTTP_STATUS_TYPE_ agnss_sta)
{
     agnss_http_download_status=agnss_sta;
}



