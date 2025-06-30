/**
* @file ffw_httpc.h
* @author sundaqing (sundaqing@fibocom.com)
* @brief HTTP简易客户端
* @version 0.1
* @date 2021-04-13
* 
* @copyright Copyright (c) 2021
* 
 */

#ifndef FFW_HTTP_FFW_HTTPC
#define FFW_HTTP_FFW_HTTPC

#include <stdint.h>
#include <stdbool.h>
#include <limits.h>

#include "ffw_url.h"
#include "ffw_buffer.h"
#include "ffw_list.h"
#include "ffw_transport_interface.h"

/// HTTP scheme
#define DEFAULT_HTTP_SCHEME "http"

/// http default port
#define DEFAULT_HTTP_PORT (80)

/// https default port
#define DEFAULT_HTTPS_PORT (443)

/// defaut buffer size
#define FFW_HTTP_DEFAULT_BUFF_SIZE (1024)

/// 默认超时时间
#define HTTP_CLIENT_DEFAULT_TIMEOUT 15000

/// 缓存的事件个数
#define FFW_HTTPC_MAX_EVENT (4)

/// type of ffw_http_header_s
typedef struct ffw_http_header_s ffw_http_header_t;

/// type of ffw_httpc_s
typedef struct ffw_httpc_s ffw_httpc_t;

/**
* @brief 连接回调函数定义
* 
* @param c HTTP实例
* @param arg 回调上下文
 */
typedef void (*on_connect_t)(ffw_httpc_t *c, void *arg);

/**
* @brief 数据回调函数定义
* 
* @param c HTTP实例
* @param buf 数据
* @param len 数据长度
* @param arg 回调上下文
 */
typedef void (*on_data_t)(ffw_httpc_t *c, void *buf, uint32_t len, void *arg);

/**
* @brief 请求结束回调函数定义
* 
* @param c HTTP实例
* @param arg 回调上下文
 */
typedef void (*on_end_t)(ffw_httpc_t *c, void *arg);

/**
* @brief 请求错误回调函数定义
* 
* @param c HTTP实例
* @param arg 回调上下文
 */
typedef void (*on_error_t)(ffw_httpc_t *c, void *arg);

/**
* @brief 回应头部接收完毕回调函数定义
* 
* @param c HTTP实例
* @param arg 回调上下文
 */
typedef void (*on_response_t)(ffw_httpc_t *c, void *arg);

/**
* @brief post 发送函数
* 
* @param c HTTP实例
* @param arg 回调上下文
 */
typedef void (*on_send_t)(ffw_httpc_t *c, void *arg);
/**
* @brief http请求方法
* 
 */
typedef enum ffw_http_method_s
{
    ///GET
    FFW_HTTP_GET,

    ///POST
    FFW_HTTP_POST,

    ///HEAD
    FFW_HTTP_HEAD,

    ///DELETE
    FFW_HTTP_DELETE,

    ///PUT
    FFW_HTTP_PUT
} ffw_http_method_t;

/**
* @brief http请求过程状态
* 
 */
typedef enum ffw_http_stat_s
{
    ///初始状态值
    FFW_HTTP_STAT_INIT = 0,

    ///等待回应
    FFW_HTTP_STAT_RESLINE,

    ///收到请求行CR字符
    FFW_HTTP_STAT_RESLINE_ALMOST_END,

    ///开始接收回应头部
    FFW_HTTP_STAT_HEADER,

    ///即将开始解析回应头部
    FFW_HTTP_STAT_HEADER_NAME_ALMOST_START,

    ///接收回应头部的名字部分
    FFW_HTTP_STAT_HEADER_NAME_START,

    ///接收头部名字部分结束
    FFW_HTTP_STAT_HEADER_NAME_END,

    ///接收头部名字的CR
    FFW_HTTP_STAT_HEADER_VALUE_ALMOST_START,

    ///接收头部值部分
    FFW_HTTP_STAT_HEADER_VALUE_START,

    ///接收头部值的CR字符
    FFW_HTTP_STAT_HEADER_VALUE_ALMOST_END,

    ///接收头部值部分结束
    FFW_HTTP_STAT_HEADER_VALUE_END,

    /// 头部最后一行CR收到
    FFW_HTTP_STAT_ALMOST_BODY,

    ///需要重定向
    FFW_HTTP_STAT_REDIR,

    ///开始接收数据部分
    FFW_HTTP_STAT_BODY,

    ///开始接收chunk-encoding 块的大小部分
    FFW_HTTP_STAT_CHUNK_SIZE,

    ///块的大小部分CR接收到
    FFW_HTTP_STAT_CHUNK_SIZE_ALMOST_END,

    ///接收chunk-encoding的数据
    FFW_HTTP_STAT_CHUNK_DATA,

    ///chunk块数据接收即将完毕
    FFW_HTTP_STAT_CHUNK_DATA_ALMOST_END,

    ///chunk块数据接收完毕
    FFW_HTTP_STAT_CHUNK_DATA_END,

    ///请求结束
    FFW_HTTP_STAT_FINISH
} ffw_http_stat_t;

/**
* @brief http header 定义 
* 
 */
struct ffw_http_header_s
{
    ///名字部分
    ffw_buffer_t name;

    ///值部分
    ffw_buffer_t value;

    ///链表
    struct list_head link;
};

/**
* @brief http请求句柄
* 
 */
struct ffw_httpc_s
{
    ///是否已经初始化
    bool init;

    ///是否需要读取body部分的数据，如果为false，那么回应的header读取完成之后就结束了。
    bool read_body;

    ///请求的url
    const char *url;

    ///同步锁
    void *lock;

    ///信号量
    void *binary_sem;

    ///解析之后的url
    ffw_url_t u;

    ///请求方法
    ffw_http_method_t method;

    ///resevse
    const char *auth_user;

    ///resevse
    const char *auth_passwd;

    ///连续重定向的次数
    int redir_count;

    ///重定向时，用于存储需要重定向的url
    ffw_buffer_t location; // for 3xx

    ///请求头部的键值对链表
    struct list_head req_headers;

    ///请求数据的大小
    int32_t req_content_length;

    ///可以直接设置http格式的头部字符串
    char *raw_request_header;
    
#if defined(CONFIG_FIBOCOM_UNICOM_ESIM_ENABLE)
    //自定义http头部字符串
    char *custom_header;
#endif
    ///回应的状态码
    int status_code;

    /// 主版本
    int ver1;

    /// 此版本
    int ver2;

    /// code 对应的描述
    char mesg[256];

    ///response raw header
    ffw_buffer_t raw_headers;

    ///response raw line
    ffw_buffer_t res_line;

    ///解析时，临时存储header
    ffw_http_header_t *tmp_header;

    ///解析之后的回应头部，键值对链表
    struct list_head res_headers;

    ///TCP层读取数据的缓冲区
    ffw_buffer_t recv_buf; //running buffer

    ///请求数据的大小
    int32_t content_length;

    ///当前已经收到的数据的大小
    int32_t recv_length;

    ///请求的数据格式是否是Chunk-Encode
    bool recv_chunk;

    ///当前chunk的大小
    int32_t chunk_size;

    ///已经接受的chunk的大小
    int32_t recv_chunk_size;

    ///是否需要自动重定向
    bool redir;

    ///当前状态
    ffw_http_stat_t st;

    ///处理最终结果，1为成功，0为失败，同步的情况下有用
    int http_req_result;

    ///TCP连接的回调函数
    on_connect_t on_connect;

    ///收到回应后的回调函数
    on_response_t on_response;

    ///收到数据的回调函数
    on_data_t on_data;

    ///请求结束的回调函数
    on_end_t on_end;

    ///出错的回调函数
    on_error_t on_error;

    //发送函数
    on_send_t on_send;

    ///回调函数的用户数据
    void *arg;

    ///是否开启流控
    bool xon;

    ///接收数据的缓冲区大小
    uint32_t xon_buffsize;

    /// 中断HTTP请求
    bool abort;

    ///TCP的连接句柄
    ffw_transport_t transport;

    /// 压缩解压缩句柄
    void *compress_handle;

    /// TLS 配置ID
    int config_id;

    /// 使用的网卡，可以是网卡名字或者网卡句柄
    ffw_handle_t netif;

    /// 缓存的事件
    on_response_t evt_notify[FFW_HTTPC_MAX_EVENT];

    /// 缓存的事件个数
    int evt_count;

    /// AF_INET FOR ipv4 AND AF_INET6 FOR IPV6
    int family;
};

/**
* @brief 获取请求方法对应的字符串，如果GET POST PUT DELETE
* 
* @param method 数字形式的请求方法
* @return const char* 字符串形式的请求方法
 */
const char *ffw_httpc_get_method_str(ffw_http_method_t method);

/**
* @brief 初始化一个http请求句柄
* 
* @param c 句柄结构体
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_httpc_init(ffw_httpc_t *c);

/**
* @brief 初始化一个http请求句柄
* 
* @param c 句柄结构体
* @param buffer_size 每次读取数据的缓冲区大小
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_httpc_init1(ffw_httpc_t *c, int32_t buffer_size);

/**
* @brief 销毁http请求句柄，每次请求结束之后，都要调用此接口。 @c ffw_httpc_wait_finish 调用完之后，或者 @c on_end 及 @c on_error 调用之后都需要抵用这个接口
* 
* @param c 句柄结构体
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_httpc_deinit(ffw_httpc_t *c);

/**
* @brief 设置当前请求是否需要读取http的数据部分
* 
* @param c HTTP请求句柄
* @param read_body 
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_httpc_read_body(ffw_httpc_t *c, bool read_body);

/**
* @brief 发起一次HTTP异步请求
* 
* @param c HTTP请求句柄
* @param method 请求方法，参考 @c ffw_http_method_t
* @param url 请求的URL
* @param redir 是否自动重定向
* @param arg 异步回调上下文
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_httpc_request(ffw_httpc_t *c, ffw_http_method_t method, const char *url, bool redir, void *arg);

/**
* @brief 通过url发起HTTP请求
* 
* @param c 请求句柄
* @param method 请求方法
* @param url url
* @param redir 是否自动重定向
* @param timeout 连接超时时间，单位为ms
* @param arg 
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_httpc_request_timeout(ffw_httpc_t *c, ffw_http_method_t method, const char *url, bool redir, int timeout, void *arg);


/**
* @brief 通过url发起HTTP请求同步接口
* 
* @param c 请求句柄
* @param method 请求方法
* @param url url
* @param redir 是否自动重定向
* @param timeout 连接超时时间，单位为ms
* @param arg 
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_httpc_request_timeout_sync(ffw_httpc_t *c, ffw_http_method_t method, const char *url, bool redir, int timeout, void *arg);

/**
* @brief 发送HTTP请求数据，此接口必须在 @c ffw_httpc_request 调用之后调用。
* 
* @param c 请求句柄
* @param data 发送的数据
* @param len 发送数据的长度
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_httpc_send_data(ffw_httpc_t *c, uint8_t *data, uint32_t len);

/**
* @brief 发送HTTP请求数据，此接口必须在 @c ffw_httpc_request 调用之后调用。
* 
* @param c 请求句柄
* @param data 发送的数据
* @param len 发送数据的长度
* @param sendlen 已发送数据的长度
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_httpc_send_data1(ffw_httpc_t *c, uint8_t *data, uint32_t len, uint32_t *sendlen);

/**
* @brief 读取HTTP数据
* 
* @param c 
* @param timeout 
* @return int 
*/
int ffw_httpc_read_data(ffw_httpc_t *c, int timeout);
/**
* @brief 在HTTP请求之前，添加请求头部，须在 @c ffw_httpc_request_timeout 或者 @c ffw_httpc_request 之前调用
* 
* @param c 请求句柄
* @param name 请求头名字
* @param value 请求头值
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_httpc_add_header(ffw_httpc_t *c, const char *name, const char *value);

/**
* @brief 获取HTTP请求回应特定头部的值
* 
* @param c 请求句柄
* @param name 头部名字
* @param[out] value 头部值
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_httpc_get_res_header(const ffw_httpc_t *c, const char *name, const char **value);

/**
* @brief 遍历回应头部，需要在 @c on_res 调用之后 或者请求结束之后调用
* 
* @param c 请求句柄
* @param cb 回调函数
* @param arg 回调用户函数
* @return int
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败  
 */
int ffw_httpc_each_resheader(const ffw_httpc_t *c, void (*cb)(const char *name, const char *value, void *arg), void *arg);

/**
* @brief 以HTTP协议的格式输出请求回应的header， @c buf 输出存储数据的缓冲区，@c len 表示 @c buf 中数据的大小
* 
* @param c http连接句柄
* @param[out] buf 缓冲区
* @param[out] len 缓冲区的
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_httpc_dump_header(const ffw_httpc_t *c, const char **buf, int32_t *len);

/**
* @brief 
* 
* @param c 
* @param timeout 
* @return int 
 */
int ffw_httpc_wait_finish(ffw_httpc_t *c, int timeout);

/**
* @brief 设置接收缓冲区的大小。如果缓冲区大小 @c xon_bufsize 值为0，则会停止从tcp缓冲区读取数据，如果再次设置缓冲区大小且 @c xon_bufsize 大于0，则继续接收数据。
* 
* @param c http连接句柄
* @param xon_bufsize 缓冲区的大小
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败  
 */
int ffw_httpc_recv_xon(ffw_httpc_t *c, uint32_t xon_bufsize);

/**
* @brief 获取请求回应码
* 
* @param c http连接句柄
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_httpc_status_code(ffw_httpc_t *c);

/**
* @brief 按照http的协议格式，设置http请求头部，这个接口不能和 @c ffw_httpc_add_header 同时使用
* 
* @param c http连接句柄
* @param content http的头部数据
* @param len 数据长度
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_httpc_add_line_header(ffw_httpc_t *c, const uint8_t *content, int32_t len);

#if defined(CONFIG_FIBOCOM_UNICOM_ESIM_ENABLE)
/**
* @brief 按照http的协议格式，设置http请求头部，这个接口不能和 @c ffw_httpc_add_header 同时使用
* 
* @param c http连接句柄
* @param content http的头部数据
* @param len 数据长度
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_httpc_add_custom_header(ffw_httpc_t *c, const uint8_t *content, int32_t len);
#endif
/**
* @brief 设置HTTP请求需要发送数据的长度，需要在发起请求之前设置
* 
* @param c http连接句柄
* @param len 数据长度
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_httpc_set_request_content_length(ffw_httpc_t *c, int32_t len);

/**
* @brief 获取请求数据的长度
* 
* @param c http连接句柄
* @return int 数据的长度
 */
int ffw_httpc_content_length(ffw_httpc_t *c);

/**
* @brief 中断HTTP请求
* 
* @param c  http连接句柄
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_httpc_abort(ffw_httpc_t *c);

/**
* @brief 等待http请求头部发送完毕，完成之后可以发送数据，也就是
* 
* @param c http连接句柄
* @param timeout 超时时间
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
* @retval FFW_R_TIMEOUT：等待超时 
 */
int ffw_httpc_wait_header_send(ffw_httpc_t *c, int timeout);

#endif /* FFW_HTTP_FFW_HTTPC */
