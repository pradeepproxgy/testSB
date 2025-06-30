#ifndef LIB_INCLUDE_FFW_NETWORK
#define LIB_INCLUDE_FFW_NETWORK

/**
* @file ffw_network.h
* @author sundaqing (sundaqing@fibocom.com)
* @brief 基于socket的网络连接组件
* @version 0.1
* @date 2021-05-07
* 
* Copyright (c) 2021 Fibocom. All rights reserved
* 
 */

#include <stdio.h>
#include <stdbool.h>

#include <stdlib.h>
#include <string.h>

#include "ffw_plat_header.h"
#include "ffw_socket.h"
#include "ffw_os.h"
#include "ffw_transport_interface.h"
#include "ffw_dns.h"

/*! Creates a type name for ffw_net_handler_s */
typedef struct ffw_net_handler_s ffw_net_handler_t;

/*! \struct ffw_net_handler_s
* \brief tcp or udp connection
* 
 */
struct ffw_net_handler_s
{
    ///The id of current network connection
    uint32_t id;

    ///The socket descriptor of network connection
    int sock;

    /// Is async mode when connect
    bool async_mode;

    ///The status of connection, see @c ffw_transport_status_t
    ffw_transport_status_t status;

    ///AF_INET for ipv4, AF_INET6 for ipv6
    int family;

    ///The connection description
    const char *sock_type;

    ///The dest address of network connection
    struct sockaddr_storage dst_addr;

    /// The size of @c in bytes
    int dst_addrlen;

    /// will connect udp to address?
    bool connect_udp;

    /// 所有DNS解析的地址
    struct ffw_addrinfo *all_addrinfo;

    /// 当前连接的地址
    struct ffw_addrinfo *cur_addrinfo;

    /// timeout in milliseconds for connect
    int32_t timeout;

    ///current timestamps
    int64_t now;

    /// 连接协议 IPPROTO_TCP 或者 IPPROTO_UDP
    int proto;

    /// DNS解析句柄，用于终止DNS解析过程
    ffw_dns_handle_t dns_handle;

    /// 使用的网卡，如果为空，则会自动选择任意一个可用的网卡
    ffw_handle_t netif;

    /// 源端口
    uint16_t src_port;

    /// 源IP地址，连接成功之后会填写
    char src_ip[FFW_INET6_ADDRSTRLEN];

    /// 目的端口
    uint16_t dst_port;

    /// 目的IP地址，连接成功之后会填写
    char dst_ip[FFW_INET6_ADDRSTRLEN];

    /// 目的域名或者IP，连接成功之后会填写
    char *dst_hostname;

    /// socket创建之后的回调函数，可在回调函数里设置sockopt
    ffw_socket_callback_t sockfd_cb;

    /// sockfd_cb 的回调上下文
    void *sockfd_cb_arg;
};

/**
* @brief 初始化一个net句柄
* 
* @param h 句柄实例
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_net_init(ffw_net_handler_t *h);

/**
* @brief 当网络连接建立时，可以用改接口获取对端的地址
* 
* @param client 句柄
* @param[out] addr 地址
* @param[out] addrlen 地址大小
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_net_get_dstaddr(ffw_net_handler_t *client, struct sockaddr *addr, int *addrlen);

/**
 * @brief 同步的方式建立连接
 * 
 * @param client 句柄
 * @param host 域名或者ip
 * @param port 端口
 * @param timeout 超时时间
 * @return int 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 */
int ffw_net_connect(ffw_net_handler_t *client, const char *host, uint16_t port, int timeout);

/**
 * @brief 发送数据
 * `
 * @param h 句柄
 * @param buf 数据缓冲区
 * @param len 数据大小
 * @return int 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 */
int ffw_net_send(ffw_net_handler_t *h, const void *buf, uint32_t len);

/**
* @brief 检测连接是否可以发送数据
* 
* @param h 句柄
* @param timeout 超时时间，单位毫秒
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_net_wait_send_available(ffw_net_handler_t *h, int32_t timeout);

/**
* @brief 发送数据，会返回已经发送数据的长度
* 
* @param h 句柄
* @param buf 数据缓冲区
* @param len 数据大小
* @param[out] send_len 已经发送的数据大小
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_net_send1(ffw_net_handler_t *h, const void *buf, uint32_t len, uint32_t *send_len);

/**
* @brief 发送数据到特定地址，只对UDP有效
* 
* @param h 句柄
* @param buf 数据缓冲区
* @param len 数据大小
* @param dest 目的地址，域名或者IP
* @param port 端口
* @return int
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_net_sendto(ffw_net_handler_t *h, const void *buf, uint32_t len, const char *dest, uint16_t port);

/**
 * @brief 发送数据到特定地址，只对UDP有效，地址为网络类型的地址
 * 
 * @param h 句柄
 * @param buf 数据缓冲区
 * @param len 数据大小
 * @param dest  网络类型的地址
 * @param destlen 地址的大小
 * @return int 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败 
 */
int ffw_net_sendto1(ffw_net_handler_t *h, const void *buf, uint32_t len, struct sockaddr *dest, int destlen);

/**
 * @brief 接收数据
 * 
 * @param h 句柄
 * @param buf 接收数据缓冲区
 * @param len 缓冲区大小
 * @param[out] read_len 接收数据的大小
 * @return int 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败 
 */
int ffw_net_recv(ffw_net_handler_t *h, void *buf, uint32_t len, uint32_t *read_len);

/**
* @brief 接收数据，只对UDP有效
* 
* @param h 句柄
* @param buf 接收数据缓冲区
* @param len 缓冲区大小
* @param[out] read_len 接收数据的大小 
* @param from 对端地址
* @param from_len @c from 的大小
* @param from_port 对端端口
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_net_recvfrom(ffw_net_handler_t *h, void *buf, uint32_t len, uint32_t *read_len, char *from, int from_len, uint16_t *from_port);

/**
 * @brief 接收数据，只对UDP有效，返回的对端地址为网络类型的
 * 
 * @param h 句柄
 * @param buf 接收数据缓冲区
 * @param len 缓冲区大小
 * @param[out] read_len 接收数据的大小 
 * @param from 对端地址 
 * @param fromlen 对端地址大小 
 * @return int
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败 
 */
int ffw_net_recvfrom1(ffw_net_handler_t *h, void *buf, uint32_t len, uint32_t *read_len, struct sockaddr *from, int *fromlen);

/**
* @brief 接收数据，只对tcp连接有效，可以设置接收超时时间
* 
* @param h 句柄
* @param buf 接收数据缓冲区
* @param len 缓冲区大小
* @param[out] read_len 接收数据的大小 
* @param timeout 超时时间，单位毫秒
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
* @retval FFW_R_TIMEOUT: 接收超时
 */
int ffw_net_recv_timeout(ffw_net_handler_t *h, void *buf, uint32_t len, uint32_t *read_len, int timeout);

/**
 * @brief 断开连接
 * 
 * @param h 句柄
 * @return int 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 */
int ffw_net_disconnect(ffw_net_handler_t *h);

/**
* @brief 判断当前实例是否已经处于连接状态
* 
* @param h 句柄
* @return int
* @retval 1: 连接状态
* @retval 0: 非连接状态 
 */
int ffw_net_is_connected(ffw_net_handler_t *h);

/**
* @brief 设置连接处于阻塞或者非阻塞状态
* 
* @param h 句柄
* @param block true: 阻塞状态 false: 非阻塞状态
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_net_set_block(ffw_net_handler_t *h, bool block);

/**
* @brief 判断连接是否处于非阻塞状态
* 
* @param h 句柄
* @return true 非阻塞状态
* @return false 阻塞状态 
 */
bool ffw_net_is_non_blocking(ffw_net_handler_t *h);

/**
 * @brief 等待网络数据
 * 
 * @param h 句柄
 * @param timeout 超时时间，单位好毫秒 
 * @return int 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 * @retval FFW_R_TIMEOUT: 超时
 */
int ffw_net_wait_data(ffw_net_handler_t *h, int timeout);

/**
 * @brief 判断网卡是否支持IPV4
 * 
 * @param netif 网卡句柄
 * @return true 支持IPV4
 * @return false 不支持IPV4
 */
bool ffw_support_ipv4(ffw_handle_t netif);

/**
 * @brief 判断网卡是否支持IPV6
 * 
 * @param netif 
 * @return true 支持IPV4
 * @return false 不支持IPV4
 */
bool ffw_support_ipv6(ffw_handle_t netif);

/**
* @brief 创建一个TCP或者UDP服务
* 
* @param h 句柄
* @param netif 监听网卡
* @param family AF_INET or AF_INET6
* @param port 监听端口
* @param proto IPPROTO_TCP or IPPROTO_TCP
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_net_create_server(ffw_net_handler_t *h, ffw_handle_t netif, int family, uint16_t port, int proto);

/**
 * @brief 接收一个客户端连接，@c h 必须是 @c ffw_net_create_server 创建的
 * 
 * @param h 服务句柄
 * @param addr 对端地址
 * @param addrlen 对端地址大小
 * @param h_out 新的连接
 * @return int
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败 
 */
int ffw_net_accept(ffw_net_handler_t *h, struct sockaddr *addr, int *addrlen, ffw_net_handler_t *h_out);

/**
* @brief 实现了 ::ffw_transport_t 接口，用于建立TCP连接
* 
* @param t 连接实例
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_net_tcp_transport_init(ffw_transport_t *t);

/**
* @brief 实现了 ::ffw_transport_t 接口，用于建立TCP连接，需要设置连接的ID
* 
* @param t 连接实例
* @param id 连接ID, id必须大于0
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_net_tcp_transport_init1(ffw_transport_t *t, int id);

/**
 * @brief 实现了 ::ffw_transport_t 接口，用于建立UDP连接
 * 
 * @param t 连接实例
 * @return int 
 */
int ffw_net_udp_transport_init(ffw_transport_t *t);

/**
* @brief This will connect socket with remote addr
* 
* @param t 连接实例
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_net_udp_transport_init1(ffw_transport_t *t);

/**
* @brief 
* 
* @param t 连接实例
* @param id context id
* @return int 
 */
int ffw_net_udp_transport_init2(ffw_transport_t *t, int id);

/**
* @brief 设置网络时间回调函数，只对LWIP有用
* 
* @param t 连接实例
* @param cb 回调函数
* @param arg 回调上下文
* @return in
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_net_transport_set_param(ffw_transport_t *t, void (*cb)(void *arg), void *arg);

/**
 * @brief 实现了 ::ffw_transport_t 接口，用于建立TCP或者UDP服务
 * 
 * @param t 连接实例
 * @param netif 监听网卡
 * @param family AF_INET or AF_INET6
 * @param port 监听端口
 * @param proto IPPROTO_TCP or IPPROTO_TCP
 * @param id 
 * @return int 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 */
int ffw_net_server_transport_init(ffw_transport_t *t, ffw_handle_t netif, int family, uint16_t port, int proto, int id);

/**
* @brief 实现了 ::ffw_transport_t 接口，用于发送域名请求，此实例只能连接IP，不能连接域名
* 
* @param t 连接实例
* @return int
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_net_udp_transport_init_dns(ffw_transport_t *t);

/**
* @brief 实现了 ::ffw_transport_t 接口，用于发送域名请求，此实例只能连接IP，不能连接域名
* 
* @param t 连接实例
* @return int
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_net_tcp_transport_init_dns(ffw_transport_t *t);


/**
* @brief ip地址和端口转换为可以连接的地址
* 
* @param ipaddr ip地址
* @param port 端口
* @param[out] addr_out 转换结果 
* @param[out] addrlen 地址的长度
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_inet_pton(const char *ipaddr, uint16_t port, struct sockaddr *addr_out, int32_t *addrlen);



/**
* @brief 可以连接的地址转换为ip地址和端口
* 
* @param addr 地址
* @param[out] dst ip地址缓冲区
* @param len @c dst 缓冲区的长度
* @param[out] port 端口
* @return const char* 失败返回NULL, 成功返回 @c dst 
 */
const char *ffw_inet_ntop(struct sockaddr *addr, char *dst, int len, uint16_t *port);

/**
* @brief 判断IP地址是否合法
* 
* @param ipaddr 
* @return true 合法 IPV4或者IPV6
* @return false 非法地址
 */
bool ffw_inet_ip_valid(const char *ipaddr);

/**
* @brief 字符串形式的IP转为网络形式的IP
* 
* @param af AF_INET ipv4 AF_INET6 ipv6 AF_UNSPEC: ipv4 or ipv6
* @param src ip地址
* @param dst 转换结果
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
*/
int ffw_ip_pton(int af, const char *src, void *dst);

/**
* @brief 网络形式的IP转换为字符串形式的地址
* 
* @param af AF_INET ipv4 AF_INET6 ipv6
* @param src 网络形式的字符串
* @param dst 转换结果缓冲区
* @param size 缓冲区大小
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_ip_ntop(int af, const void *src, char *dst, int size);

#endif /* LIB_INCLUDE_FFW_NETWORK */
