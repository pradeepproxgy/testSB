#ifndef LIB_INCLUDE_FFW_SSL_CONF
#define LIB_INCLUDE_FFW_SSL_CONF

/**
 * @file ffw_sslconf.h
 * @author sundaqing (sundaqing@fibocom.com)
 * @brief SSL相关配置 SOCKET相关配置 用户自定义DNS服务器相关配置
 * @version 0.1
 * @date 2021-05-30
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <stdint.h>
#include <stdbool.h>

/// 证书类型定义
typedef struct ffw_cert_s
{
    /// 证书数据
    uint8_t *data;

    /// 证书数据大小
    int size;
} ffw_cert_t;

/// 公钥类型证书
#define FFW_CERT_FILE (0)

/// 私钥类型证书
#define FFW_KEY_FILE (1)

/// CA证书
#define FFW_TRUST_FILE (2)

/// 附加公钥类型证书
#define FFW_CERT_EXT_FILE (3)

/// 附加私钥类型证书
#define FFW_KEY_EXT_FILE (4)

#ifndef FFW_SSL_SOCKET_ID_MAX
/// 最大的SOCKET ID
#define FFW_SSL_SOCKET_ID_MAX (16)
#endif

#ifndef FFW_SSL_CIPHER_ID_MIN
/// 最小算法ID
#define FFW_SSL_CIPHER_ID_MIN (1)
#endif

#ifndef FFW_SSL_CIPHER_ID_MAX
/// 最大算法ID
#define FFW_SSL_CIPHER_ID_MAX (138)
#endif

///所有版本都支持
#define FFW_SSL_VER_ALL (0)

///SSL3.0
#define FFW_SSL_VER_SSL3_0 (1)

///TLS1.0
#define FFW_SSL_VER_TLS1_0 (2)

///TLS1.1
#define FFW_SSL_VER_TLS1_1 (3)

///TLS1.2
#define FFW_SSL_VER_TLS1_2 (4)

///GMSSLv1
#define FFW_SSL_VER_GM1_1 (5)

///除了SSL3.0以外的版本
#define FFW_SSL_VER_NO_SSL3_0 (6)

/**
* @brief 初始化配置数据
* 
* @return int
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_sslconf_init();

/**
* @brief 添加SSL证书
* 
* @param id 配置ID
* @param type 证书类型 ::FFW_CERT_FILE 公钥 ::FFW_KEY_FILE 私钥 ::FFW_TRUST_FILE CA证书
* @param data 证书数据
* @param size 数据大小，如果是PEM证书，此大小包括结束符'\0'
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_sslconf_add_file(uint32_t id, int type, const void *data, int size);

/**
* @brief 添加SSL证书,如果是私钥，可以设置私钥的密码
* 
* @param id 配置ID
* @param type 证书类型 ::FFW_CERT_FILE 公钥 ::FFW_KEY_FILE 私钥 ::FFW_TRUST_FILE CA证书
* @param data 证书数据
* @param size 数据大小，如果是PEM证书，此大小包括结束符'\0'
* @param key_password 私钥的密码，可以为NULL,只有 @c type 为 ::FFW_KEY_FILE 时有效
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_sslconf_add_file1(uint32_t id, int type, const void *data, int size, const char *key_password);

/**
* @brief 获取证书数据
*
* @param id 配置ID 
* @param[out] cert_file 公钥证书
* @param[out] key_file 私钥证书
* @param[out] trustfile CA证书
* @param[out] trustfile_num CA证书的个数
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_sslconf_get_file(uint32_t id, const ffw_cert_t **cert_file, const ffw_cert_t **key_file, const ffw_cert_t **trustfile, int *trustfile_num);

/**
 * @brief 获取附件证书数据
 * 
 * @param id 配置ID 
 * @param cert_file 公钥证书
 * @param key_file 私钥证书
 * @param key_filepasswd 私钥证书密码
 * @return int 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 */
int ffw_sslconf_get_ext_file(uint32_t id, const ffw_cert_t **cert_file, const ffw_cert_t **key_file, const char **key_filepasswd);

/**
* @brief 获取证书数据，可以获取私钥证书的密码
* 
* @param id 配置ID
* @param[out] cert_file 公钥证书
* @param[out] key_file 私钥证书
* @param[out] key_filepasswd 私钥证书密码
* @param[out] trustfile CA证书
* @param[out] trustfile_num CA证书的个数
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_sslconf_get_file1(uint32_t id, const ffw_cert_t **cert_file, const ffw_cert_t **key_file, const char **key_filepasswd, const ffw_cert_t **trustfile, int *trustfile_num);

/**
* @brief 设置SSL证书，证书是文件的形式
* 
* @param id 配置ID
* @param type 证书类型 ::FFW_CERT_FILE 公钥 ::FFW_KEY_FILE 私钥 ::FFW_TRUST_FILE CA证书 
* @param filename 证书文件
* @param password 证书密码，只有 @c type 为 ::FFW_KEY_FILE 有效
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_sslconf_add_file_from_file(uint32_t id, int type, const char *filename, const char *password);

/**
 * @brief 获取SSL配置文件路径
 * 
 * @param id 配置ID
 * @param cert_file 公钥文件 
 * @param key_file 私钥文件
 * @param key_filepasswd 私钥密码
 * @param trustfile CA证书文件
 * @return int 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 */
int ffw_sslconf_get_filepath(uint32_t id, const char **cert_file, const char **key_file, const char **key_filepasswd, const char **trustfile);

/**
 * @brief 获取附加SSL配置文件路径
 * 
 * @param id 配置ID
 * @param cert_file 公钥文件
 * @param key_file 私钥文件
 * @param key_filepasswd 私钥密码
 * @return int 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 */
int ffw_sslconf_get_ext_filepath(uint32_t id, const char **cert_file, const char **key_file, const char **key_filepasswd);

/**
* @brief 是否验证服务端模式
* 
* @param id 配置ID
* @param mode true: 验证服务器证书 false: 不验证服务器
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_sslconf_set_mode(uint32_t id, bool mode);

/**
 * @brief 获取SSL的验证模式
 * 
 * @param id 配置ID
 * @return true 验证服务器证书
 * @return false 不验证服务器证书
 */
bool ffw_sslconf_get_mode(uint32_t id);

/**
* @brief 是否使用session复用
* 
* @param id 配置ID
* @param sw 是否使用session复用
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_sslconf_set_session_multiplex(uint32_t id, bool sw);

/**
* @brief 获取是否使用session复用
* 
* @param id 配置ID
* @param[out] sw 返回值，是否使用session复用 
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_sslconf_get_session_multiplex(uint32_t id, bool *sw);

/**
* @brief 设置SSL的错误码
* 
* @param id 配置ID
* @param err 错误码
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_sslconf_set_err(uint32_t id, int err);

/**
* @brief 获取SSL错误码
* 
* @param id 配置ID
* @return int 错误码
 */
int ffw_sslconf_get_err(uint32_t id);

/**
* @brief 设置SSL的版本号，支持的版本号有 
*        ::FFW_SSL_VER_ALL ::FFW_SSL_VER_SSL3_0 
*        ::FFW_SSL_VER_TLS1_0 ::FFW_SSL_VER_TLS1_1
*        ::FFW_SSL_VER_TLS1_2 ::FFW_SSL_VER_NO_SSL3_0
* 
* @param id 配置ID
* @param ver 版本号
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_sslconf_set_ver(uint32_t id, int ver);

/**
* @brief 获取当前SSL的版本号
* 
* @return int 版本号
 */
int ffw_sslconf_get_ver(uint32_t id);

/**
 * @brief 设置算法启用或者停用
 * 
 * @param id 配置ID
 * @param cipher_id 算法ID，从1开始 
 * @param use 是否启用
 * @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_sslconf_set_cipherlist(uint32_t id, int cipher_id, bool use);

/**
* @brief 设置算法，算法ID是SSL定义的ID
* 
* @param id 配置ID
* @param cipher_attr 算法数组 
* @param count 算法个数
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_sslconf_set_cipherlist_directly(uint32_t id, int *cipher_attr, int count);

/**
* @brief 
* 
* @param id
* @param out 
* @param min 
* @param max 
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_sslconf_get_cipher_id_list(uint32_t id, const bool **out, int *min, int *max);

/**
* @brief 获取 @ffw_sslconf_set_cipherlist_directly 接口设置的SSL算法，对应ec200 QISSLCFG
* 
* @param id 
* @param cipher_attr 
* @param count 
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_sslconf_get_cipher_list(uint32_t id, int **cipher_attr, int *count);

/**
* @brief 
* 
* @return const int* 
 */
const int *ffw_sslconf_get_cipherlist(uint32_t id);

/**
* @brief 获取SSL版本对应的字符串，调试使用
* 
* @param ver 版本号
* @return const char* 版本号对应的字符串 
 */
const char *ffw_sslconf_sslver_str(int ver);

/**
* @brief 设置SSL的psk值，如果TLS使用的是PSK模式，需要设置。
* 
* @param id 配置ID
* @param psk PSK
* @param psk_len PSK长度 
* @param psk_identity PSK ID
* @param psk_identity_len PSK ID 长度
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_sslconf_set_psk(uint32_t id, const uint8_t *psk, int psk_len, const uint8_t *psk_identity, int psk_identity_len);

/**
* @brief 获取SSL的psk值，如果TLS使用的是PSK模式，需要设置。
* 
* @param id 配置ID
* @param[out] psk PSK
* @param[out] psk_len PSK长度 
* @param[out] psk_identity PSK ID
* @param[out] psk_identity_len PSK ID 长度
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_sslconf_get_psk(uint32_t id, const uint8_t **psk, int *psk_len, const uint8_t **psk_identity, int *psk_identity_len);

/**
* @brief 是否设置了忽略过期证书
* 
* @param id 配置ID
* @return true 忽略
* @return false 不忽略
 */
bool ffw_sslconf_get_ignore_time(uint32_t id);

/**
* @brief 设置忽略过期证书
* 
* @param id 配置ID
* @param ignore true: 忽略 false: 不忽略
* @return int 
 */
int ffw_sslconf_set_ignore_time(uint32_t id, bool ignore);

/**
* @brief 是否设置了忽略SSL证书CN字段的检测
* 
* @param id 配置ID
* @return true 忽略
* @return false 不忽略
 */
bool ffw_sslconf_get_ignore_cn(uint32_t id);

/**
* @brief 设置忽略SSL证书CN字段的检测
* 
* @param id 配置ID
* @param ignore true: 忽略 false: 不忽略
* @return true 忽略
* @return false 不忽略
 */
bool ffw_sslconf_set_ignore_cn(uint32_t id, bool ignore);

/**
* @brief 设置SOCKET的OPTTION
* 
* @param id 配置ID
* @param retry_num 重试次数 
* @param min_TO MIN TO
* @param max_TO MAX TO
* @param max_close_delay max close delay
* @param is_nack_ind_req is nack ind req
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_sockopt_set(uint32_t id, int retry_num, int min_TO, int max_TO, int max_close_delay, int is_nack_ind_req);

/**
* @brief 获取SOCKET的OPTTION
* 
* @param id 配置ID
* @param[out] retry_num 重试次数 
* @param[out] min_TO MIN TO
* @param[out] max_TO MAX TO
* @param[out] max_close_delay max close delay
* @param[out] is_nack_ind_req is nack ind req
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_sockopt_get(uint32_t id, int *retry_num, int *min_TO, int *max_TO, int *max_close_delay, int *is_nack_ind_req);

/**
* @brief 
* 
* @param id 
* @param send_size 
* @param send_timeout 
* @param send_te_size 
* @param send_te_timetout 
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_ds_conf_set(uint32_t id, int32_t send_size, int32_t send_timeout, int32_t send_te_size, int32_t send_te_timetout);

/**
* @brief 
* 
* @param id 
* @param send_size 
* @param send_timeout 
* @param send_te_size 
* @param send_te_timeout 
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_ds_conf_get(uint32_t id, int32_t *send_size, int32_t *send_timeout, int32_t *send_te_size, int32_t *send_te_timeout);

/**
* @brief 设置SOCKET的KEEP配置
* 
* @param mode mode
* @param keepidle idle
* @param keepintvl intvl
* @param keepcnt cnt
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_sockkeep_set(int mode, int keepidle, int keepintvl, int keepcnt);

/**
* @brief 获取SOCKET的KEEP配置
* 
* @param[out] mode mode
* @param[out] keepidle idle
* @param[out] keepintvl intvl
* @param[out] keepcnt cnt
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_sockkeep_get(int *mode, int *keepidle, int *keepintvl, int *keepcnt);

/**
* @brief 设置SSL的SNI
* 
* @param id 配置ID
* @param sni sni
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_sslconf_set_sni(uint32_t id, const char *sni);

/**
* @brief 获取设置的SSL SNI
* 
* @param id 配置ID
* @param[out] sni sni
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_sslconf_get_sni(uint32_t id, const char **sni);

/**
* @brief 客户配置DNS服务器，@c id 为当前连接ID，如果id的值为0，则表示配置全局DNS服务器配置
* 
* @param id 链接ID
* @param primary_server 主服务器 
* @param second_server 备服务器
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_conf_set_dnsserver(uint32_t id, const char *primary_server, const char *second_server);

/**
* @brief 获取客户配置的DNS服务器，@c id 为当前连接的ID，如果id的值为0，则表示获取全局DNS服务器配置
* 
* @param id 链接ID
* @param[out] primary_server 主服务器 
* @param[out] second_server 备服务器
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_conf_get_dnsserver(uint32_t id, const char **primary_server, const char **second_server);

/**
* @brief 客户配置DNS V6服务器，@c id 为当前连接ID，如果id的值为0，则表示配置全局DNS服务器配置
* 
* @param id 链接ID
* @param primary_server 主服务器 
* @param second_server 备服务器
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_conf_set_dnsv6server(uint32_t id, const char *primary_server, const char *second_server);

/**
* @brief 获取客户配置的DNS服务器，@c id 为当前连接的ID，如果id的值为0，则表示获取全局DNS服务器配置
* 
* @param id 链接ID
* @param[out] primary_server 主服务器 
* @param[out] second_server 备服务器
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_conf_get_dnsv6server(uint32_t id, const char **primary_server, const char **second_server);

/**
* @brief 设置是否开启SSL的SNI功能
* 
* @param id 配置ID
* @param sw 是否开启
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_sslconf_sni_enable(uint32_t id, bool sw);

/**
* @brief 是否开启SSL的SNI功能
* 
* @param id 配置ID
* @param[out] sw 是否开启SNI
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_sslconf_get_sni_enable(uint32_t id, bool *sw);

/**
* @brief 批量设置SSL算法 
* 
* @param id 配置ID
* @param cipher_id_list 算法列表 
* @param use 是否启用
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_sslconf_set_cipherlist_batch(uint32_t id, const char *cipher_id_list, bool use);

#endif /* LIB_INCLUDE_FFW_SSL_CONF */
