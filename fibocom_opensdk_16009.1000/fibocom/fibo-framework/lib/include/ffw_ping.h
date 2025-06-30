#ifndef LIB_INCLUDE_FFW_PING
#define LIB_INCLUDE_FFW_PING

/**
* @file ffw_ping.h
* @author sundaqing (sundaqing@fibocom.com)
* @brief PING implement by socket
* @version 0.1
* @date 2021-05-20
* 
* Copyright (c) 2021 Fibocom. All rights reserved
* 
 */

#include <stdio.h>
#include <stdint.h>

#include "ffw_transport_interface.h"

/// 类型定义
typedef struct ffw_ping_info_s ffw_ping_info_t;

/// ping信息定义
struct ffw_ping_info_s
{
    bool timeout;       /// 是否超时
    char *dest;         /// 对端地址
    int32_t send_count; /// 发送的个数
    int32_t recv_count; /// 收到的个数
    int32_t lost_count; /// 丢失的个数
    int32_t min_rtt;    /// 回应最小时间, 单位毫秒
    int32_t max_rtt;    /// 回应最大时间, 单位毫秒
    int32_t total_rtt;  /// 总的RTT
    int32_t rtt_avg;    /// 平均RTT

    int32_t ttl;
    int32_t rtt;
    int32_t icmp_type; /// 类型
    int32_t icmp_code; /// 错误码
    int32_t seq;       /// 序列号

    int32_t ping_size;  /// ping包的大小
    int32_t ping_count; /// ping的总次数

    bool abort; /// 是否被打断
};

/**
 * @brief The callback function type for ping response
 *
 * @param ping_info ping信息
 * @param arg The context of callback
 */
typedef void (*ffw_ping_callback_t)(ffw_ping_info_t *ping_info, void *arg);

/**
 * @brief The callback function type for ping finish
 *
 * @param ping_info ping信息
 * @param arg The context of callback
 */
typedef void (*ffw_ping_end_callback_t)(ffw_ping_info_t *ping_info, void *arg);

/**
* @brief 初始化PING, 如果需要开始一次PING过程，需要先调用 @c ffw_ping_init, 然后调用 @c ffw_ping_send 发送PING包，再调用 @c ffw_ping_recv 接收PING包。
*        结束之后，调用 @c ffw_ping_deinit() 释放资源。
* 
* @param addr PING的地址
* @param family AF_INET for ipv4, AF_INET6 for ipv6
* @param ttl TTL
* @param tos TOS
* @param netif 网卡句柄
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_ping_init(const char *addr, int family, int ttl, int tos, ffw_handle_t netif);

/**
* @brief 发送ping包
* 
* @param pkt_no 序列号
* @param pkt_size 大小
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_ping_send(int pkt_no, int pkt_size);

/**
* @brief 接收PING包
* 
* @param[out] dest 对端地址
* @param dest_size @c dest缓冲区的大小
* @param seq 序列号
* @param ttl TTL
* @param rtt RTT
* @param pkt_size 包大小 
* @param icmp_type 类型
* @param icmp_code 错误码
* @param timeout 超时时间，单位毫秒
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_ping_recv(char *dest, int32_t dest_size, int16_t *seq, int32_t *ttl, int32_t *rtt, int32_t *pkt_size, uint8_t *icmp_type, int8_t *icmp_code, int32_t timeout);

/**
* @brief 释放PING资源
* 
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_ping_deinit();

/**
* @brief 初始化一个用于PING的transport，这样就可以使用 @c ffw_transport_poll_add 接口监控数据了。
* 
* @param t transport 句柄
* @param addr 对端地址
* @param family AF_INET for ipv4, AF_INET6 for ipv6
* @param ttl ttl 
* @param tos tos
* @param netif 网卡句柄
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_net_icmp_transport_init(ffw_transport_t *t, const char *addr, int family, int ttl, int tos, ffw_handle_t netif);

/**
* @brief 傻瓜式的开始一次PING过程，如果需要中间停止，可以使用 @c ffw_ping_stop，此函数为非线程安全
* 
* @param family AF_INET for ipv4, AF_INET6 for ipv6
* @param hostname 地址或者域名
* @param count 发送次数
* @param size 发送大小
* @param ttl ttl
* @param tos tos
* @param timeout 回应超时时间，单位为毫秒 
* @param netif 网卡句柄
* @param cb 回应回调函数
* @param end_cb PING结束时回到函数
* @param arg 回调上下文
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_ping_start(int family, const char *hostname, int count, int size, int ttl, int tos, int timeout, ffw_handle_t netif, ffw_ping_callback_t cb, ffw_ping_end_callback_t end_cb, void *arg);

/**
* @brief 停止一次PING过程, 次函数为异步操作 @c ffw_ping_start 的参数 @c end_cb 调用之后表示完全停止，此函数为非线程安全
* 
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_ping_stop();

#endif /* LIB_INCLUDE_FFW_PING */
