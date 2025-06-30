#ifndef LIB_INCLUDE_FFW_EASYTCP
#define LIB_INCLUDE_FFW_EASYTCP

/**
* @file ffw_easytcp.h
* @author sundaqing (sundaiqng@fibocom.com)
* @brief 实现了简单快捷的TCP(s)/UDP/DTLSl连接API
* @version 0.1
* @date 2020-11-26
* 
* @copyright Copyright (c) 2020
* 
 */

#include <stdint.h>
#include <stdbool.h>

/**
* @brief TCP(s)/UDP/DTLS事件类型定义
* 
 */
typedef enum ffw_easytcp_evt_s
{
    ///表示连接成功
    FFW_EASYTCP_EVT_CONNECT_OK = 0,

    ///表示连接失败
    FFW_EASYTCP_EVT_CONNECT_FAIL,

    ///有数据需要接受，调用 @c ffw_easytcp_recv 接受数据
    FFW_EASYTCP_EVT_IN_DATA,

    ///可以发送数据了
    FFW_EASYTCP_EVT_OUT_DATA,

    ///TICK时间，如果在调用 @c ffw_easytcp_open时tick_interval参数大于0，则会周期性产生此事件
    FFW_EASYTCP_EVT_TICK
} ffw_easytcp_evt_t;

/**
* @brief 打开UDP或者TCP连接时的事件回到函数
* 
* @param sock 表示连接的句柄
* @param evt  上报的事件，参考类型 @c ffw_easytcp_evt_t
* @param arg  回调数据
* @return
 */
typedef void (*ffw_easytcp_callback_t)(void *sock, ffw_easytcp_evt_t evt, void *arg);

/**
* @brief 创建一个TCP/UDP连接
* 
* @param host 域名或者IP地址
* @param port TCP/UDP服务端口
* @param tcp  true TCP连接 false UDP连接
* @param tls  是否使用TLS加密
* @param timeout 连接超时时间，单位为毫秒，只有tcp为true有效
* @param tick_interval 对于长连接来说，这是一个TICK发生器，在周期性（单位为毫秒）通过 @c ffw_easytcp_callback_t 产生一个 @c FFW_EASYTCP_EVT_TICK 事件
* @param[out] sock 输出参数，创建成功时，表示连接的句柄
* @param cb 事假回调函数，
* @param arg 回调用户数据
* @return int 
*        - FFW_R_SUCCESS 成功，
*        - FFW_R_FAIL 表示失败
 */
int ffw_easytcp_open(const char *host, uint16_t port, bool tcp, bool tls, int32_t timeout, int32_t tick_interval, void **sock, ffw_easytcp_callback_t cb, void *arg);

/**
* @brief 创建一个TCP/UDP连接
* 
* @param url 连接的url，如tcp[s]://host:port udp://host:port dtls://host:port
* @param timeout 连接超时时间，单位为毫秒，只有tcp为true有效
* @param tick_interval 对于长连接来说，这是一个TICK发生器，在周期性（单位为毫秒）通过 @c ffw_easytcp_callback_t 产生一个 @c FFW_EASYTCP_EVT_TICK 事件
* @param[out] sock 输出参数，创建成功时，表示连接的句柄
* @param cb 事假回调函数，
* @param arg 回调用户数据
* @return int 
*        - FFW_R_SUCCESS 成功，
*        - FFW_R_FAIL 表示失败
 */
int ffw_easytcp_open1(const char *url, int32_t timeout, int32_t tick_interval, void **sock, ffw_easytcp_callback_t cb, void *arg);

/**
* @brief 
* 
* @param sock 
* @param data 
* @param len 
* @param send_len 
* @return int 
 */
int ffw_easytcp_send(void *sock, const void *data, uint32_t len, uint32_t *send_len);

/**
 * @brief 
 * 
 * @param sock 
 * @param data 
 * @param len 
 * @return int 
 */
int ffw_easytcp_send1(void *sock, const void *data, uint32_t len);

/**
* @brief 
* 
* @param sock 
* @param data 
* @param len 
* @param len_out 
* @return int 
 */
int ffw_easytcp_recv(void *sock, void *data, uint32_t len, uint32_t *len_out);

/**
* @brief 
* 
* @param sock 
* @return int 
 */
int ffw_easytcp_close(void *sock);

/**
* @brief 
* 
* @param host 
* @param port 
* @param tcp 
* @param tls 
* @param timeout 
* @param data 
* @param len 
* @param recv_buf 
* @param recv_buflen 
* @param recv_len 
* @return int 
 */
int ffw_easytcp_read_data(const char *host, uint16_t port, bool tcp, bool tls, int32_t timeout, const void *data, uint32_t len, void *recv_buf, uint32_t recv_buflen, uint32_t *recv_len);

/**
* @brief 
* 
* @param url 
* @param timeout 
* @param data 
* @param len 
* @param recv_buf 
* @param recv_buflen 
* @param recv_len 
* @return int 
 */
int ffw_easytcp_read_data1(const char *url, int32_t timeout, const void *data, uint32_t len, void *recv_buf, uint32_t recv_buflen, uint32_t *recv_len);

#endif /* LIB_INCLUDE_FFW_EASYTCP */
