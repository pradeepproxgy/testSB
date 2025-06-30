/**
 * @file oc_http.h
 * @author zhaoziqi (ziqi.zhao@fibocom.com)
 * @brief
 * @version 1.0
 * @date 2022-02-21
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef OC_HTTP_H_
#define OC_HTTP_H_

#include <stdbool.h>
#include <stdint.h>
#include "fibocom.h"
#include "fibo_opencpu_comm.h"

#ifdef CONFIG_FIBOCOM_LONG_URL_SUPPORT
#define OC_HTTP_URL_LEN 256
#else
#define OC_HTTP_URL_LEN 1024
#endif
#define OC_HTTP_FILE_PATH_LEN 64

typedef enum
{
    OC_USER_CALLBACK = 0,
    OC_SAVE_FILE = 1
} oc_http_read_method_e;


// http callback
typedef void (*http_read_body_callback)(void *pstHttpParam, int8_t *data, int32_t len);

typedef struct _ocHttpParam
{
    uint8_t url[OC_HTTP_URL_LEN + 1];            /* Http URL */
    uint32_t timeout;                            /* Request timeout */
    uint8_t filepath[OC_HTTP_FILE_PATH_LEN + 1]; /* Http read filepath */
    oc_http_read_method_e enHttpReadMethod;      /* Http is use userdef callback */
    int32_t respContentLen;                      /* Http response body len */
    int32_t respCode;                            /* Http response code */
    uint8_t *respVersion;                        /* Http response version */
    uint8_t *respReasonPhrase;                   /* Http response reason phrase */
    bool    bIsIpv6;                             /* Http is Ipv6, default is Ipv4 */
    http_read_body_callback cbReadBody;          /* Http response callback */
    const char *longurl;
} oc_http_param_t;

typedef void (*oc_htpp_data_callback_t)(int status_code, int total_size, const void *data, int len, void *arg);

typedef struct
{
    const char *name;
    const char *value;
}oc_http_header_t;

typedef struct
{
    oc_htpp_data_callback_t data_callback;
    void *arg;
}oc_http_context_t;

typedef void (*http_read_header_callback)(const int8_t *name, const int8_t *value);

extern oc_http_param_t *fibo_http_new(void);
extern void fibo_http_delete(oc_http_param_t *pstHttpParam);
extern INT32 fibo_http_get(oc_http_param_t *pstHttpParam, uint8_t *pUserDefHeader);
extern INT32 fibo_http_head(oc_http_param_t *pstHttpParam, uint8_t *pUserDefHeader);
extern INT32 fibo_http_post(oc_http_param_t *pstHttpParam, uint8_t *pPostData, uint8_t *pUserDefHeader);
extern void fibo_http_read_body_callback(http_read_body_callback callback);
extern void fibo_http_read_header_callback(http_read_header_callback callback);
extern INT32 fibo_http_response_header_foreach(oc_http_param_t *pstHttpParam,http_read_header_callback callback);
extern INT32 fibo_http_response_status_line(oc_http_param_t *pstHttpParam);
extern INT32 fibo_http_post_ex(oc_http_param_t *pstHttpParam, UINT8 *pPostData,UINT8 *pUserDefHeader,size_t length);
extern INT32 fibo_http_put(oc_http_param_t *pstHttpParam, uint8_t *pPutData, uint8_t *pUserDefHeader);

// stream audio play url of http
extern INT32 fibo_audio_url_play(UINT8 *url, void *playback, void *arg);
extern INT32 fibo_audio_url_stop(void);
extern INT32 fibo_audio_url_status(void);

extern int fibo_http_put_oss_file(const char *hostname, const char *appKeyId, const char *appKeySecret, const char *bucketName, const char *objectName, const char *filename);

extern int fibo_http_get_sync(const char *url, oc_http_header_t *headers, int header_num, int timeout, oc_htpp_data_callback_t data_callback, void *arg);
#endif // OC_HTTP_H_