#ifndef F83EFBFA_5DEE_4C56_9BCC_0EC4A1516F30
#define F83EFBFA_5DEE_4C56_9BCC_0EC4A1516F30

#include "app.h"
#include <stdint.h>

typedef enum
{
    APP_SOCKET_TYPE_TCP = 0,
    APP_SOCKET_TYPE_UDP
}app_socket_type_t;

typedef enum 
{
    APP_SOCKET_RESULT_SUCC = 0,
    APP_SOCKET_RESULT_FAILED = -1,

    /// 缓冲区数据收完，等待新的数据
    APP_SOCKET_RESULT_READ_AGAIN = -2,

    /// 发送缓冲区满了，等待发送缓冲区发送完毕
    APP_SOCKET_RESULT_SEND_AGAIN = -3,

    /// TCP断开
    APP_SOCKET_RESULT_DISCONNECTTED = -4,

    /// 超时
    APP_SOCKET_RESULT_TIMEOUT = -5
}app_socket_result_t;


int app_socket_init();

/**
* @brief 
* 
* @param cid 
* @param sock_type 
* @param hostname 
* @param port 
* @param timeout 
* @return int 
 */
int app_socket_connect(int cid, app_socket_type_t sock_type, char *hostname, uint16_t port, uint32_t timeout);

/**
* @brief 
* 
* @param sock 
* @param data 
* @param len 
* @return int 
 */
int app_socket_send(int sock, const uint8_t *data, uint32_t len);

/**
* @brief 
* 
* @param sock 
* @param data 
* @param size 
* @param len 
* @return int 
 */
int app_socket_recv(int sock, uint8_t *data, uint32_t size, uint32_t *len);

/**
* @brief 
* 
* @param sock 
* @param data 
* @param size 
* @param len 
* @param timeout 超时时间，单位：毫秒
* @return int 
 */
int app_socket_recv_fixed(int sock, uint8_t *data, uint32_t size, uint32_t *len, int32_t timeout);

/**
* @brief 
* 
* @param ntpserver 
* @param port 
* @param timestamp 
* @return int 
 */
int app_socket_ntp(int cid, char *ntpserver, uint16_t port, uint64_t *timestamp);

/**
* @brief 
* 
* @param sock_type 
* @param sv 
* @return int 
 */
int app_socket_socketpair(app_socket_type_t sock_type, int sv[2]);

/**
* @brief 
* 
 */
void app_socket_demo();

#endif /* F83EFBFA_5DEE_4C56_9BCC_0EC4A1516F30 */
