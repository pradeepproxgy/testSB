#ifndef LIB_INCLUDE_FFW_TRANSPORT_INTERFACE
#define LIB_INCLUDE_FFW_TRANSPORT_INTERFACE

/**
* @file ffw_transport_interface.h
* @author sundaqing (sundaqing@fibocom.com)
* @brief 网络通信的接口定义
* @version 0.1
* @date 2021-05-08
* 
* Copyright (c) 2021 Fibocom. All rights reserved
* 
 */

#include <stdint.h>
#include <stdbool.h>

#include "ffw_socket.h"
#include "ffw_socket.h"
#include "ffw_os.h"
#include "ffw_list.h"
#include "ffw_utils.h"

/// socket创建之后的回调函数，可在回调函数里设置sockopt
typedef int (*ffw_socket_callback_t)(int fd, void *arg);

/// 统计tcp性能的间隔时间
#define TRANSPORT_PERFORMANCE_TIME 5 //min

/**
* @brief transport 事件定义
* 
 */
typedef enum
{
    /// 连接成功
    FFW_TRANSPORT_EVT_CONNECT_OK = 0,

    /// 连接超时
    FFW_TRANSPORT_EVT_CONNECT_TIMEOUT,

    /// 连接失败
    FFW_TRANSPORT_EVT_CONNECT_FAIL,

    /// 网络有数据
    FFW_TRANSPORT_EVT_DATA,

    /// 连接关闭
    FFW_TRANSPORT_EVT_REMOMTE_CLOSE,

    /// 读取数据错误
    FFW_TRANSPORT_EVT_ERROR
} ffw_transport_event_t;

/**
* @brief transport 类型定义
* 
 */
typedef enum
{
    /// TCP
    FFW_TRANSPORT_TCP = 0,

    /// UDP
    FFW_TRANSPORT_UDP,

    /// TCP over tls
    FFW_TRANSPORT_SSL,

    /// UDP over tls
    FFW_TRANSPORT_DTLS
} ffw_transport_type_t;

/**
* @brief transport 状态定义
* 
 */
typedef enum ffw_transport_status_s
{
    /// 非连接
    FFW_TRANSPORT_STATUS_DISCONNECTTED = 0,

    /// 连接中
    FFW_TRANSPORT_STATUS_CONNECTTING = 1,

    /// 正在握手，针对7层协议来说，如MQTT，TLS等等
    FFW_TRANSPORT_STATUS_HANDSHAKE = 2,

    /// 已经连接
    FFW_TRANSPORT_STATUS_CONNECTTED = 3,
} ffw_transport_status_t;

/**
* @brief transport 事件上报回调函数类型
* 
* @param t transport 句柄
* @param evt 事件ID
* @param buf 数据缓冲区
* @param len @c buf 缓冲区的长度
* @param arg 回调上下文
 */
typedef void (*on_event_t)(void *t, ffw_transport_event_t evt, void *buf, uint32_t len, void *arg);

/// reset performace
#define transport_reset_performace(t, min)                   \
    do                                                       \
    {                                                        \
        if ((ffw_timestamp() - t->start_time) / 60 > min) \
        {                                                    \
            t->recv_bytes = 0;                               \
            t->send_bytes = 0;                               \
            t->start_time = ffw_timestamp();              \
        }                                                    \
    } while (0);

/// connect with family
#define ffw_transport_connnect_family(t, lport, host, port, family)                              \
    ({                                                                                              \
        int ret = FFW_R_FAILED;                                                                  \
        if ((t)->init && (t)->connect)                                                              \
        {                                                                                           \
            ret = (((t)->connect)((t), (lport), (host), (port), ((t)->connect_timeout), (family))); \
        }                                                                                           \
        ret;                                                                                        \
    })

/// connect
#define ffw_transport_connnect(t, lport, host, port)                                              \
    ({                                                                                               \
        int ret = FFW_R_FAILED;                                                                   \
        if ((t)->init && (t)->connect)                                                               \
        {                                                                                            \
            ret = (((t)->connect)((t), (lport), (host), (port), ((t)->connect_timeout), AF_UNSPEC)); \
        }                                                                                            \
        ret;                                                                                         \
    })

/// connect async (dns query is async)
#define ffw_transport_connnect_async(t, lport, host, port, family, cb, arg)                                   \
    ({                                                                                                           \
        int ret = FFW_R_FAILED;                                                                               \
        if ((t)->init && (t)->connect_async)                                                                     \
        {                                                                                                        \
            ret = (((t)->connect_async)((t), (lport), (host), (port), ((t)->connect_timeout), family, cb, arg)); \
        }                                                                                                        \
        ret;                                                                                                     \
    })

/// disconnect
#define ffw_transport_disconnnect(t) \
    ({                                  \
        int ret = FFW_R_FAILED;      \
        if ((t)->init && (t)->disconn)  \
        {                               \
            ret = ((t)->disconn)(t);    \
        }                               \
        ret;                            \
    })

/// send (will send all data in @c buf)
#define ffw_transport_send(t, buf, len)         \
    ({                                             \
        int ret = FFW_R_FAILED;                 \
        if ((t)->init && (t)->send)                \
        {                                          \
            ret = ((t)->send)((t), (buf), (len));  \
        }                                          \
        else                                       \
        {                                          \
            FFW_LOG_ERROR("transport not init") \
        }                                          \
        ret;                                       \
    })

/// send1,
#define ffw_transport_send1(t, buf, len, send_len)          \
    ({                                                         \
        int ret = FFW_R_FAILED;                             \
        if ((t)->init && (t)->send1)                           \
        {                                                      \
            ret = ((t)->send1)((t), (buf), (len), (send_len)); \
        }                                                      \
        else                                                   \
        {                                                      \
            FFW_LOG_ERROR("transport not init")             \
        }                                                      \
        ret;                                                   \
    })

/// sendto
#define ffw_transport_sendto(t, buf, len, dst, port)        \
    ({                                                         \
        int ret = FFW_R_FAILED;                             \
        if ((t)->init && (t)->sendto)                          \
        {                                                      \
            ret = ((t)->sendto)((t), (buf), (len), dst, port); \
        }                                                      \
        else                                                   \
        {                                                      \
            FFW_LOG_ERROR("transport not init")             \
        }                                                      \
        ret;                                                   \
    })

/// sendto1
#define ffw_transport_sendto1(t, buf, len, dst, destlen)        \
    ({                                                             \
        int ret = FFW_R_FAILED;                                 \
        if ((t)->init && (t)->sendto1)                             \
        {                                                          \
            ret = ((t)->sendto1)((t), (buf), (len), dst, destlen); \
        }                                                          \
        ret;                                                       \
    })

/// recv
#define ffw_transport_recv(t, buf, len, read_len)          \
    ({                                                        \
        int ret = FFW_R_FAILED;                            \
        if ((t)->init && (t)->recv)                           \
        {                                                     \
            ret = ((t)->recv)((t), (buf), (len), (read_len)); \
        }                                                     \
        ret;                                                  \
    })

/// recv from
#define ffw_transport_recvfrom(t, buf, len, read_len, from, from_len, fromport)                \
    ({                                                                                            \
        int ret = FFW_R_FAILED;                                                                \
        if ((t)->init && (t)->recvfrom)                                                           \
        {                                                                                         \
            ret = ((t)->recvfrom)((t), (buf), (len), (read_len), (from), (from_len), (fromport)); \
        }                                                                                         \
        ret;                                                                                      \
    })

/// recvfrom 1
#define ffw_transport_recvfrom1(t, buf, len, read_len, from, from_len)              \
    ({                                                                                 \
        int ret = FFW_R_FAILED;                                                     \
        if ((t)->init && (t)->recvfrom1)                                               \
        {                                                                              \
            ret = ((t)->recvfrom1)((t), (buf), (len), (read_len), (from), (from_len)); \
        }                                                                              \
        ret;                                                                           \
    })

/// recv timeout
#define ffw_transport_recv_timeout(t, buf, len, read_len, timeout)            \
    ({                                                                           \
        int ret = FFW_R_FAILED;                                               \
        if ((t)->init && (t)->recv_timeout)                                      \
        {                                                                        \
            ret = ((t)->recv_timeout)((t), (buf), (len), (read_len), (timeout)); \
        }                                                                        \
        ret;                                                                     \
    })

/// socket io ctl
#define ffw_transport_ctl(t, request, ...)                        \
    ({                                                               \
        int ret = FFW_R_FAILED;                                   \
        if ((t)->init && (t)->get_fd)                                \
        {                                                            \
            ret = ioctl(((t)->get_fd)(t), (request), ##__VA_ARGS__); \
        }                                                            \
        ret;                                                         \
    })

/// set to nonblock
#define ffw_transport_set_nonblock(t, nonblock)                    \
    ({                                                                \
        int ret = FFW_R_FAILED;                                    \
        if ((t)->init && (t)->get_fd)                                 \
        {                                                             \
            ret = ffw_socket_nonblock(((t)->get_fd)(t), nonblock); \
        }                                                             \
        ret;                                                          \
    })

/// get socket descriptor
#define ffw_transport_get_fd(t)    \
    ({                                \
        int ret = FFW_R_FAILED;    \
        if ((t)->init && (t)->get_fd) \
        {                             \
            ret = ((t)->get_fd)(t);   \
        }                             \
        ret;                          \
    })

/// set socket option
#define ffw_transport_setopt(t, level, optname, optval, optlen)                     \
    ({                                                                                 \
        int ret = FFW_R_FAILED;                                                     \
        if ((t)->init && (t)->get_fd)                                                  \
        {                                                                              \
            ret = ffw_setsockopt(((t)->get_fd)(t), level, optname, optval, optlen); \
        }                                                                              \
        ret;                                                                           \
    })

/// destroy
#define ffw_transport_destory(t)              \
    ({                                           \
        int ret = FFW_R_FAILED;               \
        if ((t)->init && (t)->destory)           \
        {                                        \
            ret = (t)->destory(t);               \
            ffw_put_memory((t)->recv_buffer); \
        }                                        \
        ret;                                     \
    })

/// accept
#define ffw_transport_accept(t, addr, addr_len, h_out)     \
    ({                                                        \
        int ret = FFW_R_FAILED;                            \
        if ((t)->init && (t)->accept_conn)                    \
        {                                                     \
            ret = (t)->accept_conn(t, addr, addr_len, h_out); \
        }                                                     \
        ret;                                                  \
    })

/// wait data
#define ffw_transport_wait(t, timeout)     \
    ({                                        \
        int ret = FFW_R_FAILED;            \
        if ((t)->init && (t)->wait_data)      \
        {                                     \
            ret = (t)->wait_data(t, timeout); \
        }                                     \
        ret;                                  \
    })

/// wait to send data
#define ffw_transport_wait_send(t, timeout) \
    ({                                         \
        int ret = FFW_R_FAILED;             \
        if ((t)->init && (t)->wait_send)       \
        {                                      \
            ret = (t)->wait_send(t, timeout);  \
        }                                      \
        ret;                                   \
    })

/// get dest address
#define ffw_transport_get_dstaddr(t, addr, addrlen) \
    ({                                                 \
        int xret = FFW_R_FAILED;                    \
        if ((t)->init && (t)->get_dstaddr)             \
        {                                              \
            xret = (t)->get_dstaddr(t, addr, addrlen); \
        }                                              \
        xret;                                          \
    })

/// set timeout of connect
#define ffw_tranport_set_conn_timeout(t, timeout) \
    do                                               \
    {                                                \
        if ((t)->init)                               \
        {                                            \
            (t)->connect_timeout = timeout;          \
        }                                            \
    } while (0)

/// get perf data
#define ffw_tranport_get_perf(t, recv, send)                                     \
    do                                                                              \
    {                                                                               \
        (recv) = ((float)(t)->recv_bytes) / (ffw_timestamp() - (t)->start_time); \
        (send) = ((float)(t)->send_bytes) / (ffw_timestamp() - (t)->start_time); \
    } while (0);

/// set netif
#define ffw_transport_set_netif(t, ifac) \
    ({                                      \
        int ret = FFW_R_FAILED;          \
        if ((t)->init && (t)->set_netif)    \
        {                                   \
            ret = (t)->set_netif(t, ifac);  \
        }                                   \
        ret;                                \
    })

/// get sockname
#define ffw_transport_get_sockename(t, ip, port) \
    ({                                              \
        int ret = FFW_R_FAILED;                  \
        if ((t)->init && (t)->get_sockname)         \
        {                                           \
            ret = (t)->get_sockname(t, ip, port);   \
        }                                           \
        ret;                                        \
    })

/// get sockname
#define ffw_transport_get_dstname(t, ip, port) \
    ({                                            \
        int ret = FFW_R_FAILED;                \
        if ((t)->init && (t)->get_dstname)        \
        {                                         \
            ret = (t)->get_dstname(t, ip, port);  \
        }                                         \
        ret;                                      \
    })

/// get sockname
#define ffw_transport_set_sockfd_callback(t, cb, arg) \
    ({                                                   \
        int ret = FFW_R_FAILED;                       \
        if ((t)->init && (t)->set_sockfd_callback)       \
        {                                                \
            ret = (t)->set_sockfd_callback(t, cb, arg);  \
        }                                                \
        ret;                                             \
    })

/*! Creates a type name for ffw_transport_s */
typedef struct ffw_transport_s ffw_transport_t;

/**
* @brief 异步连接时，上报连接结果的回调函数类型
* 
* @param result 0 表示成功 非0表示失败，具体看错误码， 
* @param arg 回调上下文
 */
typedef void (*transport_on_connect_t)(int result, void *arg);

/**
* @brief 同步连接函数指针类型，本地地址为字符串类型
*
* @param h transport 句柄
* @param lport 需要绑定的本地端口
* @param host 远端地址
* @param port 远端端口
* @param timeout 超时时间，单位毫秒
* @param family AF_INET 表示连接IPV4地址 AF_INET6 表示连接IPV6地址，AF_UNSPEC 表示任意一个
* @return int
* @retval FFW_R_SUCCESS 成功
* @retval FFW_R_FAILED 失败
* @retval FFW_R_TIMEOUT 超时
 */
typedef int (*ffw_transport_connect_t)(ffw_transport_t *h, uint16_t lport, const char *host, uint16_t port, int timeout, int family);

/**
* @brief 异步函数指针类型，本地地址为结构体类型
* 
* @param h transport 句柄
* @param lport 绑定本地端口
* @param host 远端地址
* @param port 远端端口
* @param timeout 超时时间，单位毫秒
* @param family AF_INET 表示连接IPV4地址 AF_INET6 表示连接IPV6地址，AF_UNSPEC 表示任意一个
* @param conn 异步回调函数
* @param arg 回调上下文
* @return int
* @retval FFW_R_SUCCESS 成功
* @retval FFW_R_FAILED 失败
 */
typedef int (*ffw_transport_connect_async_t)(ffw_transport_t *h, uint16_t lport, const char *host, uint16_t port, int timeout, int family, transport_on_connect_t conn, void *arg);

/**
* @brief 接收一个客户端的连接函数指针
* 
* @param h transport 句柄
* @param addr 客户端地址
* @param addr_len @c addr 的大小，注意此参数传入时表示 @c addr 结构体的大小，传出时表示远端地址的大小
* @param h_out 远端地址 连接的transport句柄
* @return int
* @retval FFW_R_SUCCESS 成功
* @retval FFW_R_FAILED 失败
 */
typedef int (*ffw_transport_accept_t)(ffw_transport_t *h, void *addr, int *addr_len, ffw_transport_t *h_out);

/**
* @brief 发送数据函数指针，保证所有数据全部发送
* 
* @param h transport 句柄
* @param buf 数据缓冲区
* @param len @c buf 的大小
* @return int
* @retval FFW_R_SUCCESS 成功
* @retval FFW_R_FAILED 失败
 */
typedef int (*ffw_transport_send_t)(ffw_transport_t *h, const void *buf, uint32_t len);

/**
* @brief 发送数据函数指针, 返回值表示发送的实际数据的长度，不一定和发送的数据长度相等
* 
* @param h transport 句柄
* @param buf 数据缓冲区
* @param len @c buf 的大小
* @param send_len 发送的数据长度
* @return int
* @retval FFW_R_SUCCESS 成功
* @retval FFW_R_SOCKET_SEND_AGAIN 发送缓冲区满了，等待下一次数据
* @retval FFW_R_FAILED 失败
 */
typedef int (*ffw_transport_send1_t)(ffw_transport_t *h, const void *buf, uint32_t len, uint32_t *send_len);

/**
* @brief UDP发送数据函数指针，需要指定远端的IP地址或者域名和端口
* 
* @param h transport 句柄
* @param buf 数据缓冲区
* @param len @c buf 的大小
* @param dest 远端地址
* @param port 远端端口
* @return int
* @retval FFW_R_SUCCESS 成功
* @retval FFW_R_SOCKET_SEND_AGAIN 发送缓冲区满了，等待下一次数据, 需要调用 @c ffw_transport_waitsend_t 函数等待
* @retval FFW_R_FAILED 失败
 */
typedef int (*ffw_transport_sendto_t)(ffw_transport_t *h, const void *buf, uint32_t len, const char *dest, uint16_t port);

/**
* @brief UDP发送数据函数指针，需要指定远端地址，地址类型为 @c struct sockaddr *
* 
* @param h transport 句柄
* @param buf 数据缓冲区
* @param len @c buf 的大小
* @param dest 远端地址
* @param destlen 远端地址大小
* @return int
* @retval FFW_R_SUCCESS 成功
* @retval FFW_R_FAILED 失败
 */
typedef int (*ffw_transport_sendto1_t)(ffw_transport_t *h, const void *buf, uint32_t len, struct sockaddr *dest, int destlen);

/**
* @brief TCP接收数据函数指针
* 
* @param h transport 句柄
* @param buf 数据缓冲区
* @param len @c buf 的大小
* @param[out] read_len 接收数据的大小
* @return int
* @retval FFW_R_SUCCESS 成功
* @retval FFW_R_SOCKET_READ_AGAIN 数据读取完毕，等待下一次数据, 需要调用 @c ffw_transport_waitdata_t 函数等待
* @retval FFW_R_FAILED 失败
 */
typedef int (*ffw_transport_recv_t)(ffw_transport_t *h, void *buf, uint32_t len, uint32_t *read_len);

/**
* @brief UDP接收数据函数指针,会返回远端的IP地址和端口
* 
* @param h transport 句柄
* @param buf 数据缓冲区
* @param len @c buf 的大小
* @param[out] read_len 接收数据的大小
* @param[out] from 远端的IP地址，可以为NULL
* @param from_len @c from 的大小
* @param[out] from_port @c 远端的端口，可以为NULL
* @return int
* @retval FFW_R_SUCCESS 成功
* @retval FFW_R_SOCKET_READ_AGAIN 数据读取完毕，等待下一次数据, 需要调用 @c ffw_transport_waitdata_t 函数等待
* @retval FFW_R_FAILED 失败
 */
typedef int (*ffw_transport_recvfrom_t)(ffw_transport_t *h, void *buf, uint32_t len, uint32_t *read_len, char *from, int from_len, uint16_t *from_port);

/**
* @brief UDP接收数据函数指针,会返回远端的IP地址和端口
* 
* @param h transport 句柄
* @param buf 数据缓冲区
* @param len @c buf 的大小
* @param[out] read_len 接收数据的大小
* @param[out] from 远端地址
* @param fromlen @c from 的大小
* @return int
* @retval FFW_R_SUCCESS 成功
* @retval FFW_R_SOCKET_READ_AGAIN 数据读取完毕，等待下一次数据
* @retval FFW_R_FAILED 失败
 */
typedef int (*ffw_transport_recvfrom1_t)(ffw_transport_t *h, void *buf, uint32_t len, uint32_t *read_len, struct sockaddr *from, int *fromlen);

/**
* @brief 接收数据，可以指定超时时间
* 
* @param h transport 句柄
* @param buf 数据缓冲区
* @param len @c buf 的大小
* @param[out] read_len 接收数据的大小
* @param timeout 超时时间，单位毫秒
* @return int
* @retval FFW_R_SUCCESS 成功
* @retval FFW_R_TIMEOUT 超时时间
* @retval FFW_R_FAILED 失败
 */
typedef int (*ffw_transport_recv_timeout_t)(ffw_transport_t *h, void *buf, uint32_t len, uint32_t *read_len, int timeout);

/**
* @brief 断开连接函数指针
* 
* @param h transport 句柄
* @return int
* @retval FFW_R_SUCCESS 成功
* @retval FFW_R_FAILED 失败
 */
typedef int (*ffw_transport_disconn_t)(ffw_transport_t *h);

/**
* @brief 获取连接的描述符
* 
* @param h transport 句柄
* @return int
* @retval > 0 描述符
* @retval FFW_R_FAILED 失败
 */
typedef int (*ffw_transport_get_fd_t)(ffw_transport_t *h);

/**
* @brief 销毁transport
* 
* @param h transport 句柄
* @return int
* @retval FFW_R_SUCCESS 成功
* @retval FFW_R_FAILED 失败
 */
typedef int (*ffw_transport_destory_t)(ffw_transport_t *h);

/**
* @brief 等待transport数据到来
* 
* @param h transport 句柄
* @param timeout 超时时间，单位毫秒
* @return int
* @retval FFW_R_SUCCESS 成功
* @retval FFW_R_FAILED 失败
 */
typedef int (*ffw_transport_waitdata_t)(ffw_transport_t *h, int32_t timeout);

/**
* @brief 等待transport发送缓冲区可用
* 
* @param h transport 句柄
* @param timeout 超时时间，单位毫秒
* @return int
* @retval FFW_R_SUCCESS 成功
* @retval FFW_R_FAILED 失败
 */
typedef int (*ffw_transport_waitsend_t)(ffw_transport_t *h, int32_t timeout);

/**
* @brief 获取transport连接的远端地址
* 
* @param h transport 句柄
* @param[out] addr 存储远端地址的缓冲区
* @param[out] addrlen @c addr 的大小
* @return int
* @retval FFW_R_SUCCESS 成功
* @retval FFW_R_FAILED 失败
 */
typedef int (*ffw_transport_get_dstaddr_t)(ffw_transport_t *t, struct sockaddr *addr, int *addrlen);

/**
* @brief 设置transport使用的netif
* 
* @param h transport 句柄
* @param[out] netif 网卡句柄
* @return int
* @retval FFW_R_SUCCESS 成功
* @retval FFW_R_FAILED 失败
 */
typedef int (*ffw_transport_set_netif_t)(ffw_transport_t *t, ffw_handle_t netif);

/**
* @brief 获取tcp连接后本地的地址和端口
* 
* @param h transport 句柄
* @param[out] ip 本地地址
* @param[out] port 本地端口
* @return int
* @retval FFW_R_SUCCESS 成功
* @retval FFW_R_FAILED 失败
 */
typedef int (*ffw_transport_get_sockname_t)(ffw_transport_t *t, const char **ip, uint16_t *port);

/**
* @brief 获取tcp连接后本地的地址和端口
* 
* @param h transport 句柄
* @param callback socket 创建成功回调函数
* @return int
* @retval FFW_R_SUCCESS 成功
* @retval FFW_R_FAILED 失败
 */
typedef int (*ffw_transport_set_sockfd_callback_t)(ffw_transport_t *t, ffw_socket_callback_t callback, void *arg);

/**
 * ! \struct ffw_transport_s
 * 网络连接接口，可以实现为TCP UDP TLS等等
 * 
 */
struct ffw_transport_s
{
    ///连接名字
    const char *name;

    ///网络连接句柄
    void *client_handle;

    ///连接函数
    ffw_transport_connect_t connect;

    ///异步连接函数
    ffw_transport_connect_async_t connect_async;

    ///ACCEPT一个连接
    ffw_transport_accept_t accept_conn;

    ///TCP发送所有数据
    ffw_transport_send_t send;

    ///TCP发送数据，返回发送数据的长度
    ffw_transport_send1_t send1;

    ///UDP发送数据
    ffw_transport_sendto_t sendto;

    ///UDP发送数据
    ffw_transport_sendto1_t sendto1;

    ///TCP接收数据
    ffw_transport_recv_t recv;

    ///UDP接收数据
    ffw_transport_recvfrom_t recvfrom;

    ///UDP接收数据，远端地址用结构体表示
    ffw_transport_recvfrom1_t recvfrom1;

    ///TCP以超时的方式接收数据
    ffw_transport_recv_timeout_t recv_timeout;

    ///断开连接
    ffw_transport_disconn_t disconn;

    ///获取连接的socket描述符
    ffw_transport_get_fd_t get_fd;

    ///销毁实例
    ffw_transport_destory_t destory;

    ///等待网络数据
    ffw_transport_waitdata_t wait_data;

    ///等待可以发送数据
    ffw_transport_waitsend_t wait_send;

    ///获取对端数据
    ffw_transport_get_dstaddr_t get_dstaddr;

    /// 设置使用的网卡
    ffw_transport_set_netif_t set_netif;

    /// 获取本地地址和端口
    ffw_transport_get_sockname_t get_sockname;

    /// 获取目的地址和端口
    ffw_transport_get_sockname_t get_dstname;

    /// socket创建之后的回调函数，可在回调函数里设置sockopt
    ffw_transport_set_sockfd_callback_t set_sockfd_callback;

    /*! @cond PRIVATE */
    /********私有数据，可不用关注*****************************/
    ///连接超时时间
    int connect_timeout;
    transport_on_connect_t conn_cb;
    void *conn_arg;

    bool init;

    //for transport fact init
    on_event_t user_cb;
    void *user_arg;
    void *recv_buffer;
    int32_t recv_buffer_size;

    int32_t timeout;

    int cid;

    //for poll
    int ev;
    void *arg;
    void *cb;
    int32_t poll_timeout; //miliisecond
    int32_t timeout_left;
    int64_t now;

    int64_t recv_bytes;
    int64_t send_bytes;
    int64_t start_time;
    /********私有数据，可不用关注*****************************/
    /*! @endcond */
};

/**
* @brief transport 工厂函数，自动连接
* 
* @param t 实例
* @param type ::ffw_transport_type_t
* @param hostname 连接地址
* @param port 端口
* @param timeout 超时时间，单位毫秒
* @param recv_buf_size 接收缓冲区大小
* @param cb 数据回调函数
* @param arg 回调上下文
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_transport_create(ffw_transport_t *t, ffw_transport_type_t type, const char *hostname, uint16_t port, int32_t timeout, int32_t recv_buf_size, on_event_t cb, void *arg);

/**
* @brief 释放一个transport
* 
* @param t 实例
* @return int
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_transport_close(ffw_transport_t *t);

/**
* @brief 一次读取TCP连接的数据，然后关闭
* 
* @param type ::ffw_transport_type_t
* @param hostname  连接地址
* @param port 端口
* @param timeout 超时时间，单位毫秒
* @param send_data 发送的数据
* @param send_datasize 发送的数据大小
* @param buf 接收数据缓冲区
* @param buf_size 接收数据缓冲区大小
* @param data_size 接收数据的大小
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_transport_read(ffw_transport_type_t type, const char *hostname, uint16_t port, int32_t timeout, void *send_data, int32_t send_datasize, void *buf, int32_t buf_size, int32_t *data_size);

/**
* @brief transport 工厂函数
* 
* @param t 实例
* @param type ::ffw_transport_type_t
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_transport_fact_init(ffw_transport_t *t, ffw_transport_type_t type);

/**
 * @brief 读取固定长度的数据
 * 
 * @param t 实例
 * @param data 缓冲区
 * @param len 需要读取的长度
 * @param timeout 超时事件，单位毫秒
 * @return int 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败 
 */
int ffw_transport_read_fixed_length(ffw_transport_t *t, uint8_t *data, uint32_t len, int32_t timeout);
#endif /* LIB_INCLUDE_FFW_TRANSPORT_INTERFACE */
