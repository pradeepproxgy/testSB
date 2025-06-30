#ifndef FFW_TLS_FFW_TLS_API
#define FFW_TLS_FFW_TLS_API

/**
 * @file ffw_tls_api.h
 * @author sundaqing (sundaqing@fibocom.com)
 * @brief TLS的抽象层，根据不同的三方库实现，目前实现了mbedtls 和 cyclonetls
 * @version 0.1
 * @date 2021-05-30
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <stdint.h>
#include <ffw_transport_interface.h>

/// SSL 3.0
#define FFW_TLS_VER_SSL3_0 (1)

/// TLS 1.0
#define FFW_TLS_VER_TLS1_0 (2)

/// TLS 1.1
#define FFW_TLS_VER_TLS1_1 (3)

/// TLS 1.2
#define FFW_TLS_VER_TLS1_2 (4)

/// TLS 1.3
#define FFW_TLS_VER_TLS1_3 (5)

/// GMSSL v1
#define FFW_TLS_VER_GM1_1 (1)

/**
* @brief 创建一个TLS连接实例
* 
* @param[out] ctx 输出实例参数
* @return int
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_tls_api_context_init(void **ctx);

/**
* @brief 设置公钥和私钥
* 
* @param ctx TLS连接实例
* @param client_crt 公钥
* @param crt_size   公钥大小，如果是PEM格式的证书，该长度包含结束符'\0'
* @param client_key 私钥
* @param key_size   私钥大小，如果是PEM格式的证书，该长度包含结束符'\0'
* @param password 私钥密码，可以为NULL,表示没有密码
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_tls_api_set_cerfiticates(void *ctx, const void *client_crt, int crt_size, const void *client_key, int key_size, const char *password);

/**
* @brief 设置附加公钥和私钥
* 
* @param ctx TLS连接实例
* @param client_crt 公钥
* @param crt_size   公钥大小，如果是PEM格式的证书，该长度包含结束符'\0'
* @param client_key 私钥
* @param key_size   私钥大小，如果是PEM格式的证书，该长度包含结束符'\0'
* @param password 私钥密码，可以为NULL,表示没有密码
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_tls_api_set_ext_cerfiticates(void *ctx, const void *client_crt, int crt_size, const void *client_key, int key_size, const char *password);

/**
* @brief 设置CA证书，可以调用多次，设置多个ca
* 
* @param ctx 实例
* @param ca ca证书
* @param size ca证书大小，如果是PEM格式的证书，该长度包含结束符'\0'
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_tls_api_set_trustca(void *ctx, const void *ca, int size);

/**
* @brief 设置PSK id和key
* 
* @param ctx 实例
* @param psk pks key
* @param psk_len psk key 长度
* @param psk_identity psk id
* @param psk_identity_len psk id长度
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_tls_api_set_psk(void *ctx, const unsigned char *psk, uint32_t psk_len, const unsigned char *psk_identity, uint32_t psk_identity_len);

/**
* @brief 设置是否验证服务端证书
* 
* @param ctx 实例
* @param strict true: 验证 false: 不验证
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_tls_api_authmode(void *ctx, bool strict);

/**
* @brief 设置算法
* 
* @param ctx 实例
* @param cipher 算法列表，以0结束
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_tls_api_set_cipher(void *ctx, const int *cipher);

/**
* @brief 设置SNI
* 
* @param ctx 实例
* @param hostname SNI值 
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_tls_api_set_hostname(void *ctx, const char *hostname);

/**
* @brief 设置TLS版本
* 
* @param ctx 实例
* @param min 最小版本
* @param max 最大版本
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_tls_api_set_version(void *ctx, int32_t min, int max);

/**
* @brief 设置GM TLS版本
*
* @param ctx 实例
* @param min 最小版本
* @param max 最大版本
* @return int
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_tls_api_gm_set_version(void *ctx, int32_t min, int max);

/**
* @brief 设置是TCP还是UDP
* 
* @param ctx 实例
* @param tcp true: tcp false: udp
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_tls_api_set_udp_tcp(void *ctx, bool tcp);

/**
* @brief 获取连接的会话
* 
* @param ctx 实例
* @param[out] session_out session指针 
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_tls_api_get_session(void *ctx, void **session_out);

/**
* @brief 设置连接用的session
* 
* @param ctx 实例
* @param session session
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_tls_api_set_session(void *ctx, const void *session);

/**
* @brief 销毁session
* 
* @param session session
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_tls_api_free_session(void *session);

/**
* @brief 发送数据
* 
* @param ctx 实例
* @param buf 数据
* @param len 数据大小
* @param len_out 发送数据大小
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_tls_api_send(void *ctx, const void *buf, uint32_t len, uint32_t *len_out);

/**
* @brief 接受数据
* 
* @param ctx 实例
* @param buf 接受数据缓冲区
* @param len 缓冲区大小
* @param read_len 接收数据大小
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_tls_api_recv(void *ctx, void *buf, uint32_t len, uint32_t *read_len);

/**
* @brief 连接TLS服务器
* 
* @param sock 连接句柄
* @param timeout 超时时间，单位毫秒
* @param ctx TLS实例
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_tls_api_connect(ffw_transport_t *sock, uint32_t timeout, void *ctx);

/**
* @brief 异步连接TLS服务器
* 
* @param sock 连接句柄
* @param ctx TLS实例
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_tls_api_connect_async(ffw_transport_t *sock, void *ctx);

/**
* @brief 断开连接
* 
* @param ctx TLS实例
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_tls_api_disconnect(void *ctx);

/**
* @brief 销毁TLS实例
* 
* @param ctx 实例指针
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_tls_api_context_deinit(void *ctx);

/**
* @brief 设置是否验证过期证书和CN字段
* 
* @param ctx TLS实例
* @param verify_time 是否验证过期证书 
* @param verify_cn 是否验证CN字段
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_tls_api_verify_time_cn(void *ctx, bool verify_time, bool verify_cn);

/**
* @brief 获取所有支持算法列表
* 
* @return int* 算法列表
 */
const int *ffw_tls_api_list_ciphersuites();

/**
 * @brief 获取算法名称
 * 
 * @param id 算法ID
 * @return const char* 算法名称
 */
const char *ffw_tls_api_get_ciphersuite_name(const int id);

/**
* @brief TLS握手
* 
* @param ctx 实例
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_tls_api_handshake(void *ctx);

/**
* @brief 设置TLS日志级别
* 
* @param level 日志级别
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_tls_api_set_dbg(int level);

#endif /* FFW_TLS_FFW_TLS_API */
