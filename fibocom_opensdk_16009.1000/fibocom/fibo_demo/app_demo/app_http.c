#include "app_http.h"

#define APP_HTTP_HEADER_MAX_SIZE (1024)

typedef enum
{
    APP_HTTP_REQUEST_IDLE,
    APP_HTTP_REQUEST_ING,
    APP_HTTP_REQUEST_OK,
    APP_HTTP_REQUEST_ERROR
} app_http_status_t;

typedef enum
{
    APP_HTTP_METHOD_GET,
    APP_HTTP_METHOD_POST,
    APP_HTTP_METHOD_HEAD,
} app_http_method_t;

static char *app_method_str[] = {"GET", "POST", "HEAD"};
static UINT32 g_sig_http_sem = 0;
static UINT32 g_sig_http_lock = 0;
static oc_http_param_t *g_pstHttpParam = NULL;

static app_http_status_t g_http_status = APP_HTTP_REQUEST_IDLE;

static UINT8 *g_httpres_data = NULL;
static UINT32 g_httpres_datasize = 0;
static UINT32 g_httpres_datalen = 0;

static INT32 g_http_file_fd = -1;

static void append_data(void *data, int len)
{
    if (g_httpres_data == NULL)
    {
        g_httpres_datasize = g_httpres_datalen + len + 1;
        g_httpres_data = fibo_malloc(g_httpres_datasize);
    }

    if (g_httpres_datalen + len + 1 > g_httpres_datasize)
    {
        g_httpres_datasize = (g_httpres_datalen + len + 1) * 2;
        UINT8 *httpres_datatmp = fibo_malloc(g_httpres_datasize);
        if (g_httpres_datalen > 0)
        {
            memcpy(httpres_datatmp, g_httpres_data, g_httpres_datalen);
        }
        memcpy(httpres_datatmp + g_httpres_datalen, data, len);
        g_httpres_datalen += len;
        fibo_free(g_httpres_data);
        g_httpres_data = httpres_datatmp;
    }
    else
    {
        memcpy(g_httpres_data + g_httpres_datalen, data, len);
        g_httpres_datalen += len;
    }
    g_httpres_data[g_httpres_datalen] = 0;
}

static void check_http_is_busy()
{
    APP_WAIT_EXPECT_STATUS(g_sig_http_lock, g_sig_http_sem, g_http_status != APP_HTTP_REQUEST_IDLE, g_http_status);
    g_httpres_data = NULL;
    g_httpres_datasize = 0;
    g_httpres_datalen = 0;
}

static char *http_header_dump(app_http_method_t method, const char *url, const app_http_header_t *headers, int headers_cnt)
{
    if (headers_cnt == 0)
    {
        return NULL;
    }

    char *http_header = fibo_malloc(APP_HTTP_HEADER_MAX_SIZE);
    if (http_header == NULL)
    {
        APP_LOG_INFO("http header malloc fail");
        return NULL;
    }

    const char *uri = NULL;
    const char *hostname = strstr(url, "//");
    if (hostname != NULL)
    {
        uri = strstr(hostname + sizeof("//") - 1, "/");
    }
    if (uri == NULL)
    {
        uri = "/";
    }
    int offset = 0;
    offset += snprintf(http_header + offset, APP_HTTP_HEADER_MAX_SIZE - offset, "%s %s HTTP/1.1\r\n", app_method_str[method], uri);
    for (int i = 0; i < headers_cnt; i++)
    {
        if (APP_HTTP_HEADER_MAX_SIZE > offset)
        {
            offset += snprintf(http_header + offset, APP_HTTP_HEADER_MAX_SIZE - offset, "%s: %s\r\n", headers[i].name, headers[i].value);
        }
        else
        {
            APP_LOG_INFO("http header too long");
        }
    }
    snprintf(http_header + offset, APP_HTTP_HEADER_MAX_SIZE - offset, "%s", "\r\n");
    return http_header;
}

static void http_body_callback(void *pstHttpParam, int8_t *data, int32_t len)
{
    oc_http_param_t *httphandle = (oc_http_param_t *)pstHttpParam;
    append_data(data, len);
    if (httphandle->respContentLen == g_httpres_datalen)
    {
        APP_LOG_INFO("http recv finish");
    }
}

static void http_savefile_callback(void *pstHttpParam, int8_t *data, int32_t len)
{
    oc_http_param_t *httphandle = (oc_http_param_t *)pstHttpParam;
    g_httpres_datalen += len;
    if (g_http_file_fd > 0)
    {
        fibo_file_write(g_http_file_fd, (UINT8 *)data, len);
        fibo_taskSleep(50); /// 如果下载比较大的话，需要sleep，平台限制，否则会崩溃(被狗咬)
    }

    if (httphandle->respContentLen == g_httpres_datalen)
    {
        APP_LOG_INFO("write file finish");
    }
}

static void http_sig_request_ok_callback(GAPP_SIGNAL_ID_T sig, va_list arg)
{
    APP_LOG_INFO("http request respCode=%d", g_pstHttpParam->respCode);
    fibo_mutex_lock(g_sig_http_lock);
    if (g_http_status == APP_HTTP_REQUEST_ING)
    {
        g_http_status = (g_pstHttpParam->respCode / 100 == 2) ? APP_HTTP_REQUEST_OK : APP_HTTP_REQUEST_ERROR;
    }
    else
    {
        APP_LOG_INFO("invalid status %d", g_http_status);
    }
    fibo_mutex_unlock(g_sig_http_lock);
    fibo_sem_signal(g_sig_http_sem);
}

static void http_sig_request_fail_callback(GAPP_SIGNAL_ID_T sig, va_list arg)
{
    APP_LOG_INFO("http request fail");
    fibo_mutex_lock(g_sig_http_lock);
    if (g_http_status == APP_HTTP_REQUEST_ING)
    {
        g_http_status = APP_HTTP_REQUEST_ERROR;
    }
    fibo_mutex_unlock(g_sig_http_lock);
    fibo_sem_signal(g_sig_http_sem);
}

static void http_sig_request_res_callback(GAPP_SIGNAL_ID_T sig, va_list arg)
{
    APP_LOG_INFO("http request res ok");
    if (g_pstHttpParam->respContentLen == 0)
    {
        APP_LOG_INFO("NO Content-Length, g_pstHttpParam->respContentLen is invalid");
    }
    fibo_sem_signal(g_sig_http_sem);
}

int app_http_init()
{
    g_sig_http_sem = fibo_sem_new(0);
    if (g_sig_http_sem == 0)
    {
        APP_LOG_INFO("sem new fail");
        goto error;
    }

    g_sig_http_lock = fibo_mutex_create();
    if (g_sig_http_lock == 0)
    {
        APP_LOG_INFO("mutex new fail");
        goto error;
    }

    app_register_sig_callback(GAPP_SIG_HTTP_REQUEST_END, http_sig_request_ok_callback);
    app_register_sig_callback(GAPP_SIG_HTTP_REQUEST_ERROR, http_sig_request_fail_callback);
    app_register_sig_callback(GAPP_SIG_HTTP_REQUEST_RES, http_sig_request_res_callback);
    return 0;

error:
    fibo_mutex_delete(g_sig_http_lock);
    g_sig_http_lock = 0;
    fibo_sem_free(g_sig_http_sem);
    g_sig_http_sem = 0;
    return -1;
}

static int app_http_request(app_http_method_t method, const char *filename, const char *url, const app_http_header_t *headers, int headers_cnt, int timeout, const char *data, UINT32 datalen, void **dataout, UINT32 *dataoutlen)
{
    int ret = 0;
    check_http_is_busy();

    if (strlen(url) > OC_HTTP_URL_LEN)
    {
        APP_LOG_INFO("url(%s) too long", url);
        return -1;
    }

    g_pstHttpParam = fibo_http_new();
    if (NULL == g_pstHttpParam)
    {
        APP_LOG_INFO("new http fail");
        return -1;
    }

    /* prepare http get param */
    g_pstHttpParam->timeout = timeout;
    g_pstHttpParam->respCode = 500;
    g_pstHttpParam->enHttpReadMethod = OC_USER_CALLBACK;
    strcpy((char *)g_pstHttpParam->url, url);
    if (filename != NULL)
    {
        g_http_file_fd = fibo_file_open(filename, (O_CREAT | O_RDWR));
        if (-1 == g_http_file_fd)
        {
            APP_LOG_INFO("open file fail %s", filename);
            fibo_http_delete(g_pstHttpParam);
            g_pstHttpParam = NULL;
            return -1;
        }
        fibo_http_read_body_callback(http_savefile_callback);
    }
    else
    {
        fibo_http_read_body_callback(http_body_callback);
    }

    char *http_header = http_header_dump(method, url, headers, headers_cnt);
    g_http_status = APP_HTTP_REQUEST_ING;
    if (method == APP_HTTP_METHOD_GET)
    {
        ret = fibo_http_get(g_pstHttpParam, (UINT8 *)http_header);
    }
    else if (method == APP_HTTP_METHOD_GET)
    {
        ret = fibo_http_get(g_pstHttpParam, (UINT8 *)http_header);
    }
    else if (method == APP_HTTP_METHOD_POST)
    {
        ret = fibo_http_post_ex(g_pstHttpParam, (UINT8 *)data, (UINT8 *)http_header, datalen);
    }
    else
    {
        g_http_status = APP_HTTP_REQUEST_IDLE;
        APP_LOG_INFO("invalid method %d", method);
        ret = -1;
    }
    if (ret != 0)
    {
        fibo_http_delete(g_pstHttpParam);
        g_pstHttpParam = NULL;
        APP_LOG_INFO("http get fail");
        fibo_free(http_header);
        fibo_file_close(g_http_file_fd);
        g_http_file_fd = -1;
        _http_status = APP_HTTP_REQUEST_IDLE;
        return -1;
    }
    fibo_free(http_header);

    g_http_status = APP_HTTP_REQUEST_ING;
    APP_WAIT_EXPECT_STATUS(g_sig_http_lock, g_sig_http_sem, g_http_status == APP_HTTP_REQUEST_ING, g_http_status);

    if (g_http_file_fd > 0)
    {
        fibo_file_close(g_http_file_fd);
        g_http_file_fd = -1;
    }

    if (dataout)
    {
        *dataout = g_httpres_data;
        *dataoutlen = g_httpres_datalen;
    }
    fibo_http_delete(g_pstHttpParam);
    g_pstHttpParam = NULL;

    g_httpres_data = NULL;
    g_httpres_datasize = 0;
    g_httpres_datalen = 0;

    ret = (g_http_status == APP_HTTP_REQUEST_OK) ? 0 : -1;
    g_http_status = APP_HTTP_REQUEST_IDLE;
    return ret;
}

int app_http_request_get(const char *url, const app_http_header_t *headers, int headers_cnt, int timeout, void **dataout, UINT32 *dataoutlen)
{
    fibo_mutex_lock(g_sig_http_lock);
    int ret = app_http_request(APP_HTTP_METHOD_GET, NULL, url, headers, headers_cnt, timeout, NULL, 0, dataout, dataoutlen);
    fibo_mutex_unlock(g_sig_http_lock);
    return ret;
}

int app_http_request_get_file(const char *url, const app_http_header_t *headers, int headers_cnt, int timeout, const char *filename)
{
    fibo_mutex_lock(g_sig_http_lock);
    int ret = app_http_request(APP_HTTP_METHOD_GET, filename, url, headers, headers_cnt, timeout, NULL, 0, NULL, NULL);
    fibo_mutex_unlock(g_sig_http_lock);
    return ret;
}

int app_http_request_post(const char *url, const app_http_header_t *headers, int headers_cnt, int timeout, const char *data, UINT32 datalen, void **dataout, UINT32 *dataoutlen)
{
    fibo_mutex_lock(g_sig_http_lock);
    int ret = app_http_request(APP_HTTP_METHOD_POST, NULL, url, headers, headers_cnt, timeout, data, datalen, dataout, dataoutlen);
    fibo_mutex_unlock(g_sig_http_lock);
    return ret;
}

void app_http_demo()
{
    int ret = 0;
    int timeout = 10 * 1000;
    char *resdata = NULL;
    UINT32 resdatalen = 0;

    // //// 最基本的HTTP GET
    // ret = app_http_request_get("https://www.baidu.com", NULL, 0, timeout, (void **)&resdata, &resdatalen);
    // if (ret == 0)
    // {
    //     APP_LOG_INFO("res %d %s", resdatalen, resdata);
    // }
    // else
    // {
    //     APP_LOG_INFO("res fail");
    // }
    // fibo_free(resdata);
    // /////

    // //// 带有自定义的HTTP GET
    // app_http_header_t headers[] = {{"User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/108.0.0.0 Safari/537.36 Edg/108.0.1462.46"},
    //                                {"Host", "www.qq.com"}};
    // ret = app_http_request_get("https://new.qq.com/rain/a/20240116A03V2T00", headers, sizeof(headers) / sizeof(headers[0]), timeout, (void **)&resdata, &resdatalen);
    // if (ret == 0)
    // {
    //     APP_LOG_INFO("res %d %s", resdatalen, resdata);
    // }
    // else
    // {
    //     APP_LOG_INFO("res fail");
    // }
    // fibo_free(resdata);
    // ///// END

    // //// 下载数据到文件
    // ret = app_http_request_get_file("https://www.qq.com", NULL, 0, timeout, "/httptestfile.html");
    // if (ret == 0)
    // {
    //     APP_LOG_INFO("save file ok");
    // }
    // else
    // {
    //     APP_LOG_INFO("res fail");
    // }

    while (true)
    {
        const char postdata[] = "{\"appId\":\"b2b89edb0da5a06c864f63797bbe57e2\",\"deviceName\":\"865403063246607\",\"productKey\":\"k02ttH697n4\",\"nonceStr\":\"22162305124019024266146319491872\",\"timestamp\":\"2024-01-29 17:11:29\",\"sign\":\"7BAA4BB917E024D7020019C612C6DD3B\"}";
        //// 带有自定义的HTTP GET
        app_http_header_t headers_post[] = {{"Host", "campus-share.taobao.com"},
                                            {"Content-Type", "application/json"},
                                            {"Content-Length", "224"}};
        ret = app_http_request_post("https://campus-share.taobao.com/share/iot/queryDeviceDetail", headers_post, sizeof(headers_post) / sizeof(headers_post[0]), timeout, postdata, strlen(postdata), (void **)&resdata, &resdatalen);
        if (ret == 0)
        {
            APP_LOG_INFO("res %d %s", resdatalen, resdata);
        }
        else
        {
            APP_LOG_INFO("res fail");
        }
        fibo_free(resdata);
        fibo_taskSleep(10 * 1000);
    }
}