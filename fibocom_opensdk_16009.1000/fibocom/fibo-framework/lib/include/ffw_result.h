#ifndef LIB_INCLUDE_ERR_CODE_H
#define LIB_INCLUDE_ERR_CODE_H

/**
* @file ffw_result.h
* @author sundaqing (sundaqing@fibocom.com)
* @brief 错误码定义
* @version 0.1
* @date 2021-05-31
* 
* Copyright (c) 2021 Fibocom. All rights reserved
* 
 */

#include <stdio.h>
#include <time.h>
#include <string.h>

#include "ffw_log.h"

/**
* @brief 错误码定义
* 
 */
enum ffw_result_s
{
    /// 通用失败错误码
    FFW_R_FAILED = -1,

    /// 成功
    FFW_R_SUCCESS = 0,

    /// 超出范围
    FFW_R_OUT_OF_RANGE = 100,

    /// 无效的字符串
    FFW_R_INVALID_INT_STR,

    /// 内存不足
    FFW_R_MEMORY_NOT_ENOUGTH,

    /// send again
    FFW_R_SEND_AGAIN,

    /// 读取socket失败
    FFW_R_SOCKET_READ_FAIL,

    /// SOCKET上的数据读取完毕，等待下一次网路数据
    FFW_R_SOCKET_READ_AGAIN,

    /// 发送缓冲区已满
    FFW_R_SOCKET_SEND_AGAIN,

    /// TCP已经完毕
    FFW_R_SOCKET_CLOSE,

    /// EINTR
    FFW_R_SOCKET_EINTR,

    /// TCP连接断开
    FFW_R_SOCKET_CONNECTION_LOST_UNEXPECTED,

    /// 创建socket失败
    FFW_R_SOCKET_SOCK_FAIL,

    /// bind地址失败
    FFW_R_SOCKET_BIND_FAIL,

    /// 连接被拒绝
    FFW_R_SOCKET_CONNECT_REFUSED,

    /// 连接地址失败
    FFW_R_SOCKET_CONNECT_FAIL,

    /// 监听地址失败
    FFW_R_SOCKET_LISTEN_FAIL,

    /// 接收连接失败
    FFW_R_SOCKET_ACCEPT_FAIL,

    /// SELECT 失败
    FFW_R_SOCKET_SELECT_FAIL,

    /// 发送数据失败
    FFW_R_SOCKET_SEND_FAIL,

    /// 接收数据失败
    FFW_R_SOCKET_RECV_FAIL,

    /// 关闭socket失败
    FFW_R_SOCKET_CLOSE_FAIL,

    /// 无效的socket描述符
    FFW_R_BAD_SOCKET,

    /// 超时
    FFW_R_TIMEOUT,

    /// dns解析超时
    FFW_R_DNS_QUERY_TIMEOUT,

    /// 信号量已满
    FFW_R_SEM_OVERFLOW,

    /// FTP命令处理失败
    FFW_R_FTP_CMD_FAILED,

    /// MBEDTLS 接口出错
    FFW_R_MBEDTLS_FAILED,

    /// TCP 流控打开
    FFW_R_CMD_TCPIP_XOFF,

    /// TCP 无数据需要发送
    FFW_R_CMD_TCPIP_NODATA,

    /// TCP 有数据已经发送
    FFW_R_CMD_TCPIP_PUSH_DATA,

    /// 命令无回应
    FFW_R_CMD_NO_RSP,

    /// websocket连接关闭
    FFW_R_WS_REMOTE_CLOSE,

    /// 队列为空
    FFW_R_QUEUE_EMPTY,

    /// base64 错误码开始
    FFW_R_BASE64_ERR_START = 200,

    /// 缓冲区太短
    FFW_R_BASE64_BUFFER_TOO_SMALL,

    /// 无效的字符
    FFW_R_BASE64_INVALID_CHARACTER,

    /// 域名不存在
    FFW_R_DNS_NAME_NOT_EXIST,

    /// 域名地址不合法
    FFW_R_DNS_NAME_INVALID_ADDR,

    /// TLS连接
    FFW_R_TLS_CONNECT_FAIL,

    /// TLS握手失败
    FFW_R_TLS_HANDSHAKE_FAIL,

    /// PDP已经激活
    FFW_R_PDP_ACTIVED,

    /// PDP未激活
    FFW_R_PDP_DEACTIVED,

    /// FTP客户端已存在
    FFW_R_FTP_CMD_CLIENT_EXIST,

    /// TLS ERROR CODE

    /// 证书过期
    FFW_R_TLS_X509_BADCERT_EXPIRED = 3000,

    /// 证书吊销
    FFW_R_TLS_X509_BADCERT_REVOKED,

    /// 证书CN不匹配
    FFW_R_TLS_X509_BADCERT_CN_MISMATCH,

    /// CA证书验证失败(CA证书不对)
    FFW_R_TLS_X509_BADCERT_NOT_TRUSTED,

    /// 吊销列表签名不正确
    FFW_R_TLS_X509_BADCRL_NOT_TRUSTED,

    /// 吊销列表过期
    FFW_R_TLS_X509_BADCRL_EXPIRED,

    /// 缺少公钥证书
    FFW_R_TLS_X509_BADCERT_MISSING,

    /// 公约证书验证被忽略
    FFW_R_TLS_X509_BADCERT_SKIP_VERIFY,

    /// 其他错误
    FFW_R_TLS_X509_BADCERT_OTHER,

    /// 证书日期在将来
    FFW_R_TLS_X509_BADCERT_FUTURE,

    /// 吊销列表日期在将来
    FFW_R_TLS_X509_BADCRL_FUTURE,

    /// KEY的使用不符合要求
    FFW_R_TLS_X509_BADCERT_KEY_USAGE,

    /// KEY的使用不符合扩展KEY的使用要求
    FFW_R_TLS_X509_BADCERT_EXT_KEY_USAGE,

    /// nsCertType extension 使用不正确
    FFW_R_TLS_X509_BADCERT_NS_CERT_TYPE,

    /// 公钥的签名哈希算法不符合要求
    FFW_R_TLS_X509_BADCERT_BAD_MD,

    /// 公钥的签名算法不符合要求
    FFW_R_TLS_X509_BADCERT_BAD_PK,

    /// 公钥签名算法不对，比如RSA太短
    FFW_R_TLS_X509_BADCERT_BAD_KEY,

    /// CRL的签名哈希算法不符合要求
    FFW_R_TLS_X509_BADCRL_BAD_MD,

    /// CRL的签名算法不符合要求
    FFW_R_TLS_X509_BADCRL_BAD_PK,

    /// CRL签名算法不对，比如RSA太短
    FFW_R_TLS_X509_BADCRL_BAD_KEY,

    /// 无OTA操作
    FFW_R_FOTA_NOT_READY,

    /// OTA升级失败
    FFW_R_FOTA_FAIL,

    /// OTA升级成功
    FFW_R_FOTA_OK,

    //WIFISCAN AP数量为0
    FFW_R_NO_AP,

    //定位失败
    FFW_R_LOCATION_FAIL,

    //socket id used
    FFW_R_SOCKET_ID_NOT_IDLE,

    //GTGIS no key
    FFW_R_NO_KEY,
};

/// type of ffw_result_s
typedef enum ffw_result_s ffw_result_t;

/**
* @brief 电力错误码定义
* 
 */
enum ffw_electric_result_s
{
    FFW_ELECTRIC_NETWORK_REFUSED = 900,
    FFW_ELECTRIC_PDP_DEACTIVED = 901,
    FFW_ELECTRIC_PDP_ACTIVED = 902,
    FFW_ELECTRIC_TCPIP_PEER_REFUSED = 910,
    FFW_ELECTRIC_TCPIP_CONNECT_TIMEOUT = 911, //IP OR PORT IS NOT CORRECT
    FFW_ELECTRIC_TCPIP_ALREADY_CONNECTED = 912,
    FFW_ELECTRIC_TCPIP_NOT_CONNECTED = 913,
    FFW_ELECTRIC_BUFFER_FULL = 914,
    FFW_ELECTRIC_SEND_TIMEOUT = 915,
    FFW_ELECTRIC_DNS_FAILED = 916,
    FFW_ELECTRIC_DNS_TIMEOUT = 917,
    FFW_ELECTRIC_DNS_UNKNOW_ERROR = 918,
    FFW_ELECTRIC_PARAM_ERROR = 980,
    FFW_ELECTRIC_UNKNOWN_ERROR = 981,
};

/**
* @brief EC200错误码定义
* 
 */
enum ffw_ec200_result_s
{
    FFW_EC200_R_DNS_BUSY = 564,
    FFW_EC200_R_DNS_PARSE_FAILED = 565,
    FFW_EC200_R_TIMEOUT = 569,
};

/**
* @brief Hellobike错误码定义
* 
*/
enum ffw_hellobike_result_e
{
    FFW_HELLOBIKE_SUCCESS       = 0,
    FFW_HELLOBIKE_FAILED,

    FFW_HELLOBIKE_SIM_REPORT    = 0200,

    FFW_HELLOBIKE_SIM_DROP      = 1000,
    FFW_HELLOBIKE_SIM_UNIDENFY,
    FFW_HELLOBIKE_SIM_URC,

    FFW_HELLOBIKE_NETWORK_FAILURE   = 2000,
    FFW_HELLOBIKE_NETWORK_URC,

    FFW_HELLOBIKE_SMS_INVALID       = 3000,
    FFW_HELLOBIKE_SMS_REJECT_BY_NETWORK,
    FFW_HELLOBIKE_SMS_INVALID_MEMORY_INDEX,
    FFW_HELLOBIKE_SMS_MEMORY_FULL,
    FFW_HELLOBIKE_SMS_MESSAGE_LENGTH_EXCEEDS,
    FFW_HELLOBIKE_SMS_URC,
    FFW_HELLOBIKE_SMS_CRC_FAILED,

    FFW_HELLOBIKE_FS_INVALID_FILE_NAME  = 4001,
    FFW_HELLOBIKE_FILE_NOT_FOUND,
    FFW_HELLOBIKE_FILE_SPACE_FULL,
    FFW_HELLOBIKE_FILE_LARGE,
    FFW_HELLOBIKE_FILE_TIMEOUT,
    FFW_HELLOBIKE_FILE_READ_ERROR,

    // ...

    FFW_HELLOBIKE_GNSS_CHECKSUM_ERROR   = 1200,
    FFW_HELLOBIKE_GNSS_NMEA_LOST,
};


/// 检查条件并打印错误日志
#define CHECK(condition, err_code, format, ...)      \
    do                                               \
    {                                                \
        if (condition)                               \
        {                                            \
            FFW_LOG_ERROR(format, ##__VA_ARGS__); \
            ret = err_code;                          \
            goto error;                              \
        }                                            \
    } while (0);

/// 检查是否为空
#define CHECK_NULL(condition)                           \
    do                                                  \
    {                                                   \
        if ((condition) == NULL)                        \
        {                                               \
            FFW_LOG_ERROR("%s is null", #condition); \
            ret = FFW_R_FAILED;                      \
            goto error;                                 \
        }                                               \
    } while (0);

/// 检查条件并打印INFO日志
#define CHECK_COMM(condition, err_code, format, ...) \
    do                                               \
    {                                                \
        if (condition)                               \
        {                                            \
            FFW_LOG_INFO(format, ##__VA_ARGS__);  \
            ret = err_code;                          \
            goto error;                              \
        }                                            \
    } while (0);

/// 检查条件不打印日志
#define CHECK_ONLY(condition, err_code) \
    do                                  \
    {                                   \
        if (condition)                  \
        {                               \
            ret = err_code;             \
            goto error;                 \
        }                               \
    } while (0);

/// 检查条件是否为FFW_R_SUCCESS. 如果不是，则打印错误日志
#define CHECK_OK(iret, format, ...)                  \
    do                                               \
    {                                                \
        ret = (iret);                                \
        if ((ret) != FFW_R_SUCCESS)               \
        {                                            \
            FFW_LOG_ERROR(format, ##__VA_ARGS__); \
            goto error;                              \
        }                                            \
    } while (0);

/// 检查条件是否为FFW_R_SUCCESS
#define CHECK_OK_ONLY(iret)                                       \
    do                                                            \
    {                                                             \
        ret = (iret);                                             \
        FFW_LOG_TRACE("call %s", #iret);                       \
        if ((ret) != FFW_R_SUCCESS)                            \
        {                                                         \
            if (FFW_R_SOCKET_READ_AGAIN != ret)                \
            {                                                     \
                FFW_LOG_ERROR("call %s fail(%d)", #iret, ret); \
            }                                                     \
            goto error;                                           \
        }                                                         \
    } while (0);

#endif /* LIB_INCLUDE_ERR_CODE_H */
