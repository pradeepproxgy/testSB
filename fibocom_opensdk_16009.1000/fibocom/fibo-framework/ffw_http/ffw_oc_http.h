#ifndef IDH_CODE_COMPONENTS_FIBOCOM_OPENCPU_OC_HTTP
#define IDH_CODE_COMPONENTS_FIBOCOM_OPENCPU_OC_HTTP

#include "ffw_types.h"
#include <stdint.h>
#include <stdbool.h>

typedef void (*on_progress_t)(uint32_t total, uint32_t recv_len, void *arg);
typedef void (*on_finish_t)(int result, void *arg);
typedef void (*on_data_callback_t)(int result, int status_code, void *data, uint32_t len, void *arg);
typedef struct fibo_http_option_s fibo_http_option_t;
typedef struct fibo_http_header_s fibo_http_header_t;

typedef struct
{
    char *endpoint;
    char *accessid;
    char *accesskey;
    int32_t iscname;
    char *bucket;
    char *object;
    char *filepath;
    char *md5;
    char *cbheader;
    char *cbheader_var;
} ossupload_t;

struct fibo_http_header_s
{
    const char *name;
    const char *value;
};

struct fibo_http_option_s
{ //follow 3xx when redir is true
    bool redir;
    //0 get, 1 post 2 put 3 delete 4 head
    uint8_t method;

    int8_t header_count;
    fibo_http_header_t *headers;

    void *arg;

    void (*on_connect)(void *arg);
    void (*on_data)(void *buf, uint32_t len, void *arg);
    void (*on_end)(void *arg);
    void (*on_error)(void *arg);
    void (*on_response)(void *arg);
};

/**
* @brief 
* 
* @param url 
* @param opt 
* @return int 
 */
int fibo_http_request(const char *url, fibo_http_option_t *opt);

/**
* @brief 
* 
* @param data 
* @param len 
* @return int 
 */
int fibo_http_send_data(uint8_t *data, uint32_t len);

/**
* @brief 
* 
* @param cb 
* @param arg 
* @return int 
 */
int fibo_http_each_resheader(void (*cb)(const char *name, const char *value, void *arg), void *arg);

/**
* @brief 
* 
* @return int 
 */
int fibo_http_statuscode();

/**
* @brief 
* 
* @param url 
* @param filename 
* @param headers 
* @param header_count 
* @return int 
 */
int fibo_http_upload_file(const char *url, const char *filename, fibo_http_header_t *headers, int header_count);

/**
* @brief 
* 
* @param url 
* @param method 
* @param filename 
* @param headers 
* @param header_count 
* @return int 
 */
int fibo_http_upload_file_method(const char *url, int method, const char *filename, fibo_http_header_t *headers, int header_count);

/**
* @brief 
* 
* @param url 
* @param buf 
* @param len 
* @param headers 
* @param header_count 
* @return int 
 */
int fibo_http_post_data(const char *url, void *buf, uint32_t len, fibo_http_header_t *headers, int header_count, void *res_buf, uint32_t res_buflen, uint32_t *res_len, int32_t timeout);

/**
* @brief 
* 
* @param url 
* @param filename 
* @param headers 
* @param header_count 
* @param on_progress 
* @param arg 
* @return int 
 */
int fibo_http_download_file(const char *url, char *filename, fibo_http_header_t *headers, int header_count, int32_t timeout, on_progress_t on_progress, void *arg);

/**
* @brief 
* 
* @param url 
* @param len_out 
* @param headers 
* @param header_count 
* @param timeout 
* @return uint32_t 
 */
int fibo_http_get_data_size(const char *url, uint32_t *len_out, fibo_http_header_t *headers, int header_count, int32_t timeout);

/**
* @brief 
* 
* @param url 
* @param buf 
* @param len 
* @param len_out 
* @param headers 
* @param header_count 
* @param timeout 
* @param on_progress 
* @param arg 
* @return int 
 */
int fibo_http_get_data(const char *url, void *buf, uint32_t len, uint32_t *len_out, fibo_http_header_t *headers, int header_count, int32_t timeout, on_progress_t on_progress, void *arg);

/**
* @brief 
* 
* @param url 
* @param b 
* @param headers 
* @param header_count 
* @param timeout 
* @param on_progress 
* @param arg 
* @return int 
 */
int fibo_http_get_data_buffer(const char *url, void *b, fibo_http_header_t *headers, int header_count, int32_t timeout, on_progress_t on_progress, void *arg);


/**
* @brief 
* 
* @param url 
* @param headers 
* @param header_count 
* @param timeout 
* @param cb 
* @param arg 
* @return int 
 */
int fibo_http_get_data_buffer_async(const char *url, fibo_http_header_t *headers, int header_count, int32_t timeout, on_data_callback_t cb, ffw_handle_t netif, void *arg);

/**
* @brief 
* 
* @param url 
* @param filename 
* @param headers 
* @param header_count 
* @param timeout 
* @param on_progress 
* @param on_fni 
* @param arg 
* @return int 
 */
int fibo_http_download_file1(const char *url, const char *filename, fibo_http_header_t *headers, int header_count, int32_t timeout, on_progress_t on_progress, on_finish_t on_fni, void *arg);


/**
* @brief 
* 
* @param hostname 
* @param appKeyId 
* @param appKeySecret 
* @param bucketName 
* @param objectName 
* @param filename 
* @return int 
 */
int fibo_http_put_oss_file(const char *hostname, const char *appKeyId, const char *appKeySecret, const char *bucketName, const char *objectName, const char *filename);


/**
* @brief 
* 
* @param url 
* @param buf 
* @param len 
* @param len_out 
* @param defheader 
* @param header_count 
* @param timeout 
* @param on_progress 
* @param arg 
* @return int 
 */
int fibo_http_get_data_defhead(const char *url, void *buf, uint32_t len, uint32_t *len_out, uint8_t *defheader, int header_count, int* response_code, int32_t timeout, on_progress_t on_progress, void *arg);


/**
* @brief 
* 
* @param url 
* @param buf 
* @param len 
* @param len_out 
* @param defheader 
* @param header_count 
* @param res_buf 
* @param res_buflen 
* @param res_len 
* @param timeout 
* @return int 
 */
int fibo_http_post_data_defhead(const char *url, void *buf, uint32_t len, uint8_t *defheader, int header_count, void *res_buf, uint32_t res_buflen, uint32_t *res_len, int* response_code, int32_t timeout);


int fibo_http_put_hb_oss_file(ossupload_t *upload);

#endif /* IDH_CODE_COMPONENTS_FIBOCOM_OPENCPU_OC_HTTP */
