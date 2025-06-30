#ifndef FFW_DNS_FFW_DNS
#define FFW_DNS_FFW_DNS

/**
* @file ffw_dns.h
* @author sundaqing (sundaqing@fibocom.com)
* @brief 实现域名解析功能，将域名转换为可用的地址，类似getaddrinfo的功能
* @version 0.1
* @date 2021-05-06
* 
* Copyright (c) 2021 Fibocom. All rights reserved
* 
 */

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "ffw_list.h"
#include "ffw_buffer.h"
#include "ffw_utils.h"
#include "ffw_transport_interface.h"

/// The max count of secion
#define DNS_SECTION_MAX (4)

/// udp mode
#define DNS_UDP_MODE (0)

/// tcp mode
#define DNS_TCP_MODE (1)

/// dns服务器的个数
#define DNS_SERVER_MAX_COUNT (4)

/// type of ffw_dns_message_s
typedef struct ffw_dns_message_s ffw_dns_message_t;

/// type of ffw_dns_handle_s
typedef struct ffw_dns_handle_s ffw_dns_handle_t;

/**
* @brief DNS报文结构体
* 
 */
struct ffw_dns_message_s
{
    ///请求ID
    uint16_t id;

    ///消息FLAG
    uint16_t flags;

    ///返回码
    uint16_t rcode;

    ///操作码
    uint16_t opcode;

    ///每个section的内容的个数
    uint16_t counts[DNS_SECTION_MAX];

    ///section内容链表
    struct list_head records[DNS_SECTION_MAX];
};

/**
* @brief 异步DNS请求句柄
* 
 */
struct ffw_dns_handle_s
{
    /// 异步回调函数
    void (*cb)(int result, struct ffw_addrinfo *addrinfo, void *arg);

    /// 回调上下文
    void *arg;

    /// 超时时间，单位毫秒
    int timeout;

    /// 剩余的超时时间，单位毫秒
    int timeout_left;

    /// time stamp
    int64_t now;

    /// port
    int port;

    /// 网络连接句柄
    ffw_transport_t t;

    /// 解析类型 AAAA or A
    int qtype[2];

    /// 当前解析类型下标
    int qtype_idx;

    /// 解析类型个数
    int qtype_cnt;

    /// DNS 请求消息 A 和 AAAA
    uint8_t dnsmsg[2][256];

    /// 请求消息的长度
    int32_t dnsmsglen[2];

    /// 重试次数
    int server_trycnt;

    /// 当前服务器下表
    int server_idx;

    /// 服务器个数
    int server_cnt;

    /// dns服务器
    char dnsserver[DNS_SERVER_MAX_COUNT][FFW_INET6_ADDRSTRLEN];

    /// TCP 回应数据长度字节
    uint8_t tcplen[2];

    /// 已接收的数据长度的字节数
    uint8_t tcp_recvlen;

    /// TCP 数据
    uint8_t tcpdata[4096];

    /// TCP 数据长度
    uint32_t tcpdatalen;

    /// dns请求使用的网卡句柄
    ffw_handle_t netif;

    /// 如果答案的缓存里，则可以设置结果在此
    struct ffw_addrinfo *addrinfo;

    /// 此次请求的ID
    uint16_t queryid;

    bool use_ipv4;
};

/// set qr
#define ffw_dns_message_set_qr(msg, qr) ((msg)->flags |= ((!!(qr)) << 15))

/// set rd
#define ffw_dns_message_set_rd(msg, rd) ((msg)->flags |= (((uint16_t)(!!(rd))) << 8))

/// set ad
#define ffw_dns_message_set_ad(msg, ad) ((msg)->flags |= (((uint16_t)(!!(ad))) << 5))

/**
* @brief 初始化域名解析功能，在使用域名解析功能最开始调用
* 
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_dns_init();

/**
 * @brief 初始化一个DNS消息报文实例
 * 
 * @param msg 
 * @return int 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 */
int ffw_dns_message_init(ffw_dns_message_t *msg);

/**
 * @brief 设置需要查询的域名和类型
 * 
 * @param name 域名
 * @param type 类型1: A 28: AAAA
 * @param msg 消息报文实例
 * @return int 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 */
int ffw_dns_message_set_query(const char *name, int type, ffw_dns_message_t *msg);

/**
* @brief 从 @c buf 里解析一个DNS报文
* 
* @param buf DNS消息数据
* @param msg DNS消息实例
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_dns_message_decode(ffw_buffer_t *buf, ffw_dns_message_t *msg);

/**
* @brief 编码一个DNS消息实例
* 
* @param msg DNS消息实例
* @param buf DNS消息数据
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_dns_message_encode(const ffw_dns_message_t *msg, ffw_buffer_t *buf);

/**
* @brief 销毁DNS消息实例
* 
* @param msg DNS消息实例
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_dns_message_destory(ffw_dns_message_t *msg);

/**
* @brief 打印DNS消息实例，用于调试使用
* 
* @param msg DNS消息实例
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_dns_message_print(ffw_dns_message_t *msg);

/**
* @brief 通过TCP的方式解析域名
* 
* @param dns_server 域名服务器
* @param hostname 需要解析的域名
* @param port 域名服务器端口
* @param qtype 解析类型，1: A 28: AAAA
* @param timeout 超时时间，单位为毫秒
* @param[out] addrinfo 解析到的地址
* @param netif 使用的网卡
* @return int
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_dns_resolv_tcp(const char *dns_server, const char *hostname, uint16_t port, int qtype, int timeout, struct ffw_addrinfo **addrinfo, ffw_handle_t netif);

/**
* @brief 通过UDP的方式解析域名
* 
* @param dns_server 域名服务器
* @param hostname 需要解析的域名
* @param port 域名服务器端口
* @param qtype 解析类型，1: A 28: AAAA
* @param timeout 超时时间，单位为毫秒
* @param[out] addrinfo 解析到的地址
* @param netif 使用的网卡
* @return int
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_dns_resolv_udp(const char *dns_server, const char *hostname, uint16_t port, int qtype, int timeout, struct ffw_addrinfo **addrinfo, ffw_handle_t netif);

/**
* @brief 异步UDP域名解析
* 
* @param h 解析句柄
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_dns_resolv_udp_async(ffw_dns_handle_t *h);

/**
* @brief 域名TCP域名解析
* 
* @param h 解析句柄
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_dns_resolv_tcp_async(ffw_dns_handle_t *h);

/**
* @brief 清除dns解析的缓存数据
* 
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_dns_clear_cache();

/**
* @brief 销毁解析成功的地址
* 
* @param addrinfo 地址
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_freeaddrinfo(struct ffw_addrinfo *addrinfo);

/**
* @brief 转换域名或者IP为地址
* 
* @param hostname 域名或者IP
* @param type AF_INET 或者 AF_INET6
* @param port 端口
* @param timeout 超时时间，单位为毫秒
* @param[out] addrinfo 地址
* @param netif 使用的网卡
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_getaddrinfo(const char *hostname, int type, uint16_t port, int timeout, struct ffw_addrinfo **addrinfo, ffw_handle_t netif);

/**
* @brief 转换域名或者IP为地址，异步接口，可以指定域名服务器，最多两个
* 
* @param h 异步请求handle
* @param dnsserver 域名服务器地址
* @param servercnt 域名服务器地址的个数
* @param hostname 域名或者IP
* @param type AF_INET 或者 AF_INET6
* @param port 端口
* @param timeout 超时时间，单位为毫秒
* @param cb 异步回调函数
* @param arg 异步回调上下文
* @param netif 使用的网卡
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_getaddrinfo1_a(ffw_dns_handle_t *h, const char **dnsserver, int servercnt, const char *hostname, int type, uint16_t port, int timeout, void (*cb)(int result, struct ffw_addrinfo *addrinfo, void *arg),
                          void *arg, ffw_handle_t netif);

/**
* @brief 转换域名或者IP为地址，异步接口
* 
* @param h 异步请求handle
* @param hostname 域名或者IP
* @param type AF_INET 或者 AF_INET6
* @param port 端口
* @param timeout 超时时间，单位为毫秒
* @param cb 异步回调函数
* @param arg 异步回调上下文
* @param netif 使用的网卡
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_getaddrinfo_a(ffw_dns_handle_t *h, const char *hostname, int type, uint16_t port, int timeout, void (*cb)(int result, struct ffw_addrinfo *addrinfo, void *arg), void *arg, ffw_handle_t netif);

/**
* @brief 转换域名或者IP为地址，可以指定多个域名服务器地址
* 
* @param hostname 域名或者IP
* @param type AF_INET 或者 AF_INET6
* @param port 端口
* @param timeout 超时时间，单位为毫秒
* @param[out] addrinfo 地址
* @param dnsserver 域名服务器地址
* @param servercnt 域名服务器地址的个数
* @param netif 使用的网卡
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_getaddrinfo1(const char *hostname, int type, uint16_t port, int timeout, struct ffw_addrinfo **addrinfo, const char **dnsserver, int servercnt, ffw_handle_t netif);

/**
* @brief 设置DNS查询模式，tcp模式或者udp模式，默认是udp模式
* 
* @param mode ::DNS_UDP_MODE udp OR ::DNS_TCP_MODE tcp
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_dns_mode(int mode);


int ffw_getaddrinfo2_a(ffw_dns_handle_t *h, const char **dnsserver, int servercnt, const char *hostname, int type, uint16_t port, int timeout,
                          void (*cb)(int result, struct ffw_addrinfo *addrinfo, void *arg), void *arg, ffw_handle_t netif, bool use_ipv4_server);


#endif /* FFW_DNS_FFW_DNS */
