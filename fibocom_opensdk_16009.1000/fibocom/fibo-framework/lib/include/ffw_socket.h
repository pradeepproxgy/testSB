#ifndef BAADC737_8549_4323_B675_8116DF56316E
#define BAADC737_8549_4323_B675_8116DF56316E

/**
* @file ffw_socket.h
* @author sundaqing (sundaqing@fibocom.com)
* @brief SOCKET api 抽象层，根据平台不同实现
* @version 0.1
* @date 2021-06-01
* 
* Copyright (c) 2021 Fibocom. All rights reserved
* 
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "ffw_plat_header.h"
#include "ffw_types.h"

/// FIN have send
#define FFW_SOCKET_EVT_FIN_SEND (0)

/// FIN ACK have recv
#define FFW_SOCKET_EVT_FIN_ACK (1)

/// FIN in 2msl
#define FFW_SOCKET_EVT_FIN_2MSL (2)

/// send rst
#define FFW_SOCKET_EVT_FIN_RST (3)

/// recv send ack
#define FFW_SOCKET_EVT_SEND_ACK (4)

/// recv fin packet
#define FFW_SOCKET_EVT_FIN (5)

/// fd set zero
#define FFW_FD_ZERO(s) ffw_fds_zero(s)

/// set fd in fd set
#define FFW_FD_SET(fd, s) ffw_fds_set(fd, s)

/// check fd is set in fd
#define FFW_FD_ISSET(fd, s) ffw_fds_isset(fd, s)

#define FFW_AF_INET 2
#define FFW_AF_INET6 10
#define FFW_AF_UNSPEC 0

/* Structure to contain information about address of a service provider.  */
struct ffw_addrinfo
{
    int ai_flags;             /* Input flags.  */
    int ai_family;            /* Protocol family for socket.  */
    int ai_socktype;          /* Socket type.  */
    int ai_protocol;          /* Protocol for socket.  */
    int ai_addrlen;     /* Length of socket address.  */
    int ai_ttl; /// ttl
    int ai_cnt; /// 答案的个数
    struct sockaddr *ai_addr; /* Socket address for socket.  */
    char *ai_canonname;       /* Canonical name for service location.  */
    struct ffw_addrinfo *ai_next; /* Pointer to next in list.  */
};


/// socket api netif 参数是创建socket指定的网卡，有些平台不支持，可以忽略
int ffw_socket(int domain, int type, int proto, void *netif);

/// connect api
int ffw_connect(int fd, const struct sockaddr *pname, int namelen);

/// close api
int ffw_close(int fd);

/// shutdown api
int ffw_shutdown(int fd, int how);

/// send api
int ffw_send(int fd, const void *data, uint32_t size, int flags);

/// sendto api
int ffw_sendto(int fd, const void *data, uint32_t size, int flags, const struct sockaddr *to, int tolen);

/// recv api
int ffw_recv(int fd, void *data, uint32_t size, int flags);

/// recvfrom api
int ffw_recvfrom(int fd, void *data, uint32_t size, int flags, struct sockaddr *src_addr, int *addrlen);

/// accept api
int ffw_accept(int fd, struct sockaddr *addr, int *addrlen);

/// bind api
int ffw_bind(int fd, const struct sockaddr *pname, int namelen);

/// listen api
int ffw_listen(int fd, int backlog);

/// getsockopt
int ffw_getsockopt(int sockfd, int level, int optname, void *optval, int *optlen);

/// setsockopt
int ffw_setsockopt(int sockfd, int level, int optname, const void *optval, int optlen);

/// select
int ffw_select(int nfds, ffw_fd_set *readfds, ffw_fd_set *writefds, ffw_fd_set *exceptfds, struct timeval *timeout);

/// fds set in select fd set
void ffw_fds_set(int fd, ffw_fd_set *s);

/// fds isset in select fd set
bool ffw_fds_isset(int fd, ffw_fd_set *s);

/// clear fds in select fd set
void ffw_fds_zero(ffw_fd_set *s);

/**
* @brief 设置errno
* 
* @param erno errno
* @return int success forever
 */
int ffw_set_errno(int erno);

/**
* @brief 按照指定的方式关闭socket，@c type 可以为 
*        0. 等待FIND ACK 回应
*        1. 不等待FIN ACK 回应
*        2. 等待2MSL,也就是socket资源完全释放
*        4. 直接发送RST
* 
* @param fd 描述符
* @param type 关闭模式
* @return int32_t FFW_R_SUCCESS forever
 */
int32_t ffw_socket_close_type(int32_t fd, int32_t type);

/**
* @brief 用于上报send ack、fin ack等事件
* 
* @param fd 
* @param cb event参数可以为 FFW_SOCKET_EVT_FIN_SEND FFW_SOCKET_EVT_FIN_ACK FFW_SOCKET_EVT_FIN_2MSL FFW_SOCKET_EVT_FIN_RST FFW_SOCKET_EVT_SEND_ACK
* @param arg 
* @return int32_t 
 */
int32_t ffw_socket_evt_callback(int32_t fd, void (*cb)(int sock, int event, uint16_t size, void *arg), void *arg);

/**
* @brief 设置socket为阻塞或非阻塞模式
* 
* @param sockfd fd
* @param nonblock true: 非阻塞 false: 阻塞
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_socket_nonblock(int sockfd, bool nonblock);

/**
* @brief 判断socket是否为阻塞模式
* 
* @param sockfd fd
* @return true 非阻塞模式
* @return false 阻塞模式
 */
bool ffw_socket_is_nonblock(int sockfd);

/**
 * @brief 设置socket的偏移量,此接口的意义是本库初始化时需要占用两个socket，为了不影响用户的socket从1开始，决定在初始化，先讲socket开始值设置到最后两个。
 * 
 * @return 永远成功
 */
int ffw_socket_set_offset();

/**
 * @brief 恢复socket的偏移量，对应 @c ffw_socket_set_offset, 为了恢复socket从1开始
 * 
 * @return 永远成功
 */
int ffw_socket_reset_offset();

/**
 * @brief 获取当前socket没有确认的数据的大小
 * 
 * @param fd socket 描述符
 * @param[out] unacked 没有确认的数据的大小
 * @param[out] acked 确认的数据的大小
 * @return int 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败 
 */
int ffw_socket_get_unacked(uint32_t fd, uint32_t *unacked, uint32_t *acked);

/**
 * @brief 获取当前TCP连接的MSS
 * 
 * @param sockfd 描述符
 * @return int32_t mss
 */
int32_t ffw_socket_get_mss(int sockfd);

/**
 * @brief 获取本端的连接地址
 * 
 * @param sockfd 描述符
 * @return int32_t 
 */
int32_t ffw_socket_getsockname(int sockfd, struct sockaddr *laddr);

/**
 * @brief 获取远端的连接地址
 * 
 * @param sockfd 描述符
 * @return int32_t 
 */
int32_t ffw_socket_getpeername(int sockfd, struct sockaddr *laddr);

/**
 * @brief 获取socket的错误码
 * 
 * @param fd socket描述符
 * @return int 错误码
 */
int ffw_sock_errno(int fd);


/**
 * @brief 获取socket发送缓冲区剩余大小
 * 
 * @param fd socket描述符
 * @return int 错误码
 */
int ffw_sock_txrestsize(int fd, int32_t *size);

/**
 * @brief for net traffic info query
 * 
 * @param pNetInfo, uint32_t buff array point.
 * @param DataSize, pNetInfo array len.
 * @param simId, data simid
 * @return int, result succ or not.
 */
int ffw_get_NetInfo_platf(uint32_t *pNetInfo, uint8_t DataSize, uint8_t simId);

#endif /* BAADC737_8549_4323_B675_8116DF56316E */
