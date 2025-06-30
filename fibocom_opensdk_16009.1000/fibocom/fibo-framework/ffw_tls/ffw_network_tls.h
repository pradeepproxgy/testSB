#ifndef FFW_TLS_FFW_NETWORK_TLS
#define FFW_TLS_FFW_NETWORK_TLS

/**
* @file ffw_network_tls.h
* @author sundaqing (sundaqing@fibocom.com)
* @brief TLS客户端实现
* @version 0.1
* @date 2021-06-01
* 
* Copyright (c) 2021 Fibocom. All rights reserved
* 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ffw_transport_interface.h"
#include "ffw_network.h"
#include "ffw_file.h"
#include "ffw_sslconf.h"

#define FFW_SSL_CA_DIRNAME "/usr" FFW_FS_SEP "ca"
#define FFW_SSL_KEY_DIRNAME "/usr" FFW_FS_SEP "key"


/// type of ffw_net_tls_handler_s
typedef struct ffw_net_tls_handler_s ffw_net_tls_handler_t;

/// type of ffw_net_tls_option_s
typedef struct ffw_net_tls_option_s ffw_net_tls_option_t;

/**
* @brief TLS的承载协议
* 
 */
enum ffw_net_tls_type_s
{
    /// TCP OVER TLS
    FFW_TLS_TCP = 0,

    /// UDP OVER TLS
    FFW_TLS_UDP,
};

/// type of ffw_net_tls_type_s
typedef enum ffw_net_tls_type_s ffw_net_tls_type_t;

/**
* @brief TLS实例定义
* 
 */
struct ffw_net_tls_handler_s
{
    ///The ID of TLS connection
    uint32_t id;

    ///TLS 连接句柄
    void *ssl_ctx;

    ///是否已经初始化
    bool init;

    ///当前状态
    ffw_transport_status_t status;

    /// TLS承载协议
    ffw_net_tls_type_t type;

    ///TCP OR UDP 连接句柄
    ffw_transport_t tcp_transport;

    ///mebedtls 错误回调函数
    int (*on_mbedtls_error)(uint32_t id, int err_code);

    ///是否使用TLS回话复用
    bool session_reused;

    /// this session will be used when @c session_reused is true
    void *session;

    ///如果是异步连接，连接上报回调函数
    transport_on_connect_t conn_cb;

    ///回调上下文
    void *conn_arg;

    ///是否启用sni功能
    bool sni_enable;

    ///SNI值，这个是必须在连接期间都有效
    const char *sni;

    /// 当前tls的transport句柄
    ffw_transport_t *t;
};

/**
* @brief TLS初始化用的选项
* 
 */
struct ffw_net_tls_option_s
{
    ///The ID of TLS connection
    uint32_t id;

    ///The ID of TCP connection
    uint32_t id_tcp;

    ///文件形式的CA证书
    const char *root_ca_file;

    ///CA证书
    const ffw_cert_t *root_ca;

    ///证书的个数
    int ca_num;

    ///是否使用PSK模式
    bool psk_mode;

    ///客户端公钥
    const ffw_cert_t *client_cert;

    ///客户端私钥
    const ffw_cert_t *client_key;

    ///文件形式的客户端公钥
    const char *client_certpath;

    ///文件形式的客户端私钥
    const char *client_keypath;

    ///文件形式的客户端私钥密码，可以为空
    const char *client_keypass;

    ///客户端附加公钥
    const ffw_cert_t *client_certext;

    ///客户端附加私钥
    const ffw_cert_t *client_keyext;

    ///文件形式的客户端附加公钥
    const char *client_certextpath;

    ///文件形式的客户端附加私钥
    const char *client_keyextpath;

    ///文件形式的客户端附加私钥密码，可以为空
    const char *client_keyextpass;

    ///PSK ID
    const uint8_t *psk_id;

    ///psk id的长度
    int psk_idlen;

    ///PSk KEY
    const uint8_t *psk_key;

    ///PSK KEY的长度
    int psk_keylen;

    ///TLS的版本
    int ver;

    /// TLS承载协议
    ffw_net_tls_type_t type;

    ///算法列表
    const int *ciphersuites;

    ///mebedtls 错误回调函数
    int (*on_mbedtls_error)(uint32_t id, int err_code);

    ///是否忽略过期证书
    int32_t ignore_cert_expire;

    ///是否检查证书的CN字段
    int32_t ignore_hostname_check;

    ///是否验证服务器证书
    bool verify_strict;

    ///是否使用TLS回话复用
    bool session_reused;

    /// this session will be used when @c session_reused is true
    void *session;

    ///是否启用sni功能
    bool sni_enable;

    ///SNI值，这个是必须在连接期间都有效
    const char *sni;
};

/**
* @brief 初始化TLS连接句柄
* 
* @param c 句柄
* @param opt 参数选项
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_net_tls_init(ffw_net_tls_handler_t *c, const ffw_net_tls_option_t *opt);

/**
* @brief 销毁TLS句柄
* 
* @param h 句柄
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_net_tls_deinit(ffw_net_tls_handler_t *h);

/**
 * @brief 同步连接TLS
 * 
 * @param h 句柄
 * @param host 目的地址
 * @param port 端口号
 * @param timeout 超时时间，单位毫秒
 * @return int 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 */
int ffw_net_tls_connect(ffw_net_tls_handler_t *h, const char *host, uint16_t port, int timeout);

/**
* @brief 异步连接TLS
* 
* @param h 句柄
* @param family AF_INET 或者 AF_INET6 
* @param lport 源端口
* @param host 对端地址
* @param port 对端端口号
* @param timeout 超时时间，单位毫秒
* @param conn_cb 连接完成回调函数
* @param arg 回调上下文
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_net_tls_connect_async(ffw_net_tls_handler_t *h, int family, uint16_t lport, const char *host, uint16_t port, int timeout, transport_on_connect_t conn_cb, void *arg);

/**
 * @brief 发送数据，保证全部发送，或者失败
 * 
 * @param h 句柄
 * @param buf 数据缓冲区
 * @param len 数据长度
 * @return int 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 */
int ffw_net_tls_send(ffw_net_tls_handler_t *h, const void *buf, uint32_t len);

/**
 * @brief 发送数据，不保证全部发送
 * 
 * @param h 句柄
 * @param buf 数据缓冲区
 * @param len 数据长度
 * @param[out] send_len 已发送的数据长度
 * @return int 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 */
int ffw_net_tls_send1(ffw_net_tls_handler_t *h, const void *buf, uint32_t len, uint32_t *send_len);

/**
* @brief 接收数据
* 
* @param h 句柄
* @param buf 数据缓冲区
* @param len @c buf 的长度
* @param[out] read_len 接收数据的长度 
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_net_tls_recv(ffw_net_tls_handler_t *h, void *buf, uint32_t len, uint32_t *read_len);

/**
 * @brief 断开TLS连接
 * 
 * @param h 句柄
 * @return int 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 */
int ffw_net_tls_disconnect(ffw_net_tls_handler_t *h);

/**
* @brief 设置TLS的日志级别，数字越大，日志越多，一般为5即可
* 
* @param level 日志级别
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_net_tls_set_dbg(int level);

/**
* @brief 获取上一次连接的TLS回话信息
* 
* @param h 句柄
* @param[out] session 回话内容 
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_net_tls_transport_get_session(ffw_transport_t *h, void **session);

/**
* @brief 设置TLS回话信息
* 
* @param h 句柄
* @param session 回话内容 
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_net_tls_transport_set_session(ffw_transport_t *h, const void *session);

/**
* @brief 实现了 ::ffw_transport_t 接口，用于建立基于TCP的TLS连接
* 
* @param t 连接实例
* @param opt 选项参数
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_net_tls_transport_init(ffw_transport_t *t, const ffw_net_tls_option_t *opt);

/**
* @brief 实现了 ::ffw_transport_t 接口, XGD使用这个接口，ca和证书根据id来获取
* 
* @param t 连接实例
* @param type 承载协议
* @param id 连接ID
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_net_tls_transport_init1(ffw_transport_t *t, ffw_net_tls_type_t type, uint32_t id);

/**
* @brief 实现了 ::ffw_transport_t 接口, 标准版本使用这个接口，ca和证书只获取id为0的证书
* 
* @param t 连接实例
* @param type 承载协议
* @param id 连接ID
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_net_tls_transport_init2(ffw_transport_t *t, ffw_net_tls_type_t type, uint32_t id);

/**
* @brief 实现了 ::ffw_transport_t 接口, 魔方使用这个接口，ca和证书根据id来获取
* 
* @param t  连接实例
* @param type 承载协议
* @param id 连接ID
* @param ssl_id TLS配置ID
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_net_tls_transport_init3(ffw_transport_t *t, ffw_net_tls_type_t type, uint32_t id, uint32_t ssl_id);

/**
* @brief 实现了 ::ffw_transport_t 接口, psk认证方式
* 
* @param t 连接实例
* @param type 承载协议
* @param id 连接ID
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_net_tls_transport_psk_init(ffw_transport_t *t, ffw_net_tls_type_t type, uint32_t id);

/**
* @brief get session
* 
* @param h 句柄
* @param session_out session
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_net_tls_get_session(ffw_net_tls_handler_t *h, void **session_out);

/**
* @brief set session
* 
* @param h 句柄
* @param session session
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_net_tls_set_session(ffw_net_tls_handler_t *h, const void *session);

/**
* @brief 释放session
* 
* @param session session
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_net_tls_free_session(void *session);

/// TCP OVER TLS PSK MODE with connect id 0
#define ffw_net_ttls_transport_psk_init(t) ffw_net_tls_transport_psk_init(t, FFW_TLS_TCP, 0)

/// UDP OVER TLS PSK MODE with connect id 0
#define ffw_net_dtls_transport_psk_init(t) ffw_net_tls_transport_psk_init(t, FFW_TLS_UDP, 0)

/// TCP OVER TLS with connect id 0
#define ffw_net_ttls_transport_init(t) ffw_net_tls_transport_init1(t, FFW_TLS_TCP, 0)

/// UDP OVER TLS with connect id 0
#define ffw_net_dtls_transport_init(t) ffw_net_tls_transport_init1(t, FFW_TLS_UDP, 0)

/// TCP OVER TLS with connect id
#define ffw_net_ttls_transport_init1(t, id) ffw_net_tls_transport_init1(t, FFW_TLS_TCP, id)

/// UDP OVER TLS with connect id
#define ffw_net_dtls_transport_init1(t, id) ffw_net_tls_transport_init1(t, FFW_TLS_UDP, id)

/// TCP OVER TLS
#define ffw_net_ttls_transport_init2(t, id) ffw_net_tls_transport_init2(t, FFW_TLS_TCP, id)

/// UDP OVER TLS
#define ffw_net_dtls_transport_init2(t, id) ffw_net_tls_transport_init2(t, FFW_TLS_UDP, id)

#endif /* FFW_TLS_FFW_NETWORK_TLS */
