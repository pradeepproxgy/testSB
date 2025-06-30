#ifndef CBB_INCLUDE_CBB_TCPIP
#define CBB_INCLUDE_CBB_TCPIP

/**
 * @file cbb_cmd_tcpip.h
 * @author sundaqing (sundaqing@fibocom.com)
 * @brief TCP/UDP AT命令逻辑实现，注意此文件的接口必须在 transport_loop 线程中执行，可以使用 ffw_transport_poll_exectask执行这些接口
 * @version 0.1
 * @date 2021-08-02
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <stdint.h>
#include <stdbool.h>

#include "ffw_buffer.h"
#include "ffw_ping.h"
#include "ffw_transport_interface.h"
#include "ffw_timer.h"

#define CBB_CONNECT_IS_UDP(x) (((x) == CBB_PROTOCOL_UDP_IPV4) || ((x) == CBB_PROTOCOL_UDP_IPV6))

/// 数据经过HEX编码
#define CBB_IPRFMT_HEX_MODE (0)

/// 上报数据不带前缀
#define CBB_IPRFMT_BIN_WITHOUT_PREFIX (1)

/// 上报数据带前缀
#define CBB_IPRFMT_BIN_WITH_PREFIX (2)

/// 缓存模式
#define CBB_IPRFMT_CACHE_MODE (5)

/// 非模式 数据有延迟
#define CBB_IPRFMT_REPORT_WITH_DELAY (6) ///

typedef enum cbb_protocol_s
{
    CBB_PROTOCOL_TCP_IPV4 = 0,
    CBB_PROTOCOL_UDP_IPV4 = 1,
    CBB_PROTOCOL_SSL_IPV4 = 2,
    CBB_PROTOCOL_TCP_IPV6 = 60,
    CBB_PROTOCOL_UDP_IPV6 = 61,
    CBB_PROTOCOL_SSL_IPV6 = 62,
} cbb_protocol_t;

/// 发送模式
typedef enum cbb_send_mode_s
{
    /// 尝试缓存，如果计时器超时时间为0或者出发发送的大小，则直接发送
    CBB_SEND_MODE_TRY_CACHE = 0,

    /// 直接发送
    CBB_SEND_MODE_DIRECTLY,

    /// 只是缓存
    CBB_SEND_MODE_JUST_CACHE
} cbb_send_mode_t;

/**
 * @brief 事件类型定义
 * 
 */
typedef enum cbb_event_s
{
    /// 流控打开事件
    CBB_EVT_XOFF = 0,

    /// 流控关闭事件
    CBB_EVT_XON,

    /// 数据上报事件
    CBB_EVT_DATA,

    /// 数据已缓存事件
    CBB_EVT_DATA_READ,

    /// 发送数据回应事件
    CBB_EVT_SEND_RSP,

    /// PUSH数据回应事件
    CBB_EVT_PUSH_RSP,

    /// 数据得到确认事件
    CBB_EVT_SEND_ACK,

    /// 停止事件
    CBB_EVT_ABORT,

    /// 服务器关闭事件
    CBB_EVT_ABORT_CLOSE,

    /// 连接已经打开事件
    CBB_EVT_OPEN,

    /// TLS打开事件
    CBB_EVT_TLS_OPEN,

    /// 关闭事件
    CBB_EVT_CLOSE,

    /// UDP关闭事件
    CBB_EVT_UDP_CLOSE,

    /// 服务接收一个tcp连接事件
    CBB_EVT_ACCEPT_OPEN,

    /// 连接失败事件
    CBB_EVT_SOCKET_CONNECT_FAIL,

    /// socket失败事件
    CBB_EVT_SOCKET_SOCK_FAIL,

    /// bind失败事件
    CBB_EVT_SOCKET_BIND_FAIL
} cbb_event_t;

typedef enum cbb_connection_status_s
{
    CBB_CONNECTION_IDLE = 0,
    CBB_CONNECTION_CONNECTING,
    CBB_CONNECTION_CONNECTTED,
    CBB_CONNECTION_FIN_RECV,
    CBB_CONNECTION_CLOSING,
    CBB_CONNECTION_REPORT_CLOSE_EVT,
    CBB_CONNECTION_ABORT,
    CBB_CONNECTION_SERVER
} cbb_connection_status_t;

/// tcp finish mode define
typedef enum cbb_connection_fin_mode_s
{
    /// invalid fin mode
    CBB_FIN_MODE_FIN_INVALID = -1,

    /// FIN ACK have recv
    CBB_FIN_MODE_FIN_ACK = 0,

    /// FIN have send
    CBB_FIN_MODE_FIN_SNED,

    /// FIN in 2msl
    CBB_FIN_MODE_FIN_2MSL,

    /// send rst
    CBB_FIN_MODE_FIN_RST,

} cbb_connection_fin_mode_t;

typedef void (*evt_callback_t)(int32_t id, cbb_event_t evt, uint32_t result, void *arg);

/// type of struct cbb_connection_s
typedef struct cbb_connection_s cbb_connection_t;

/**
 * @brief TCP/UDP 连接定义
 * 
 */
struct cbb_connection_s
{
    /// TCP/UDP 连接 id
    int32_t id;

    /// TCP/UDP 连接实例
    ffw_transport_t t;

    /// 接收缓冲区
    ffw_buffer_t rx_buf;

    /// 发送缓冲区
    ffw_buffer_t tx_buf;

    /// 发送数据到协议栈的数据大小阀值
    int32_t send_size_triggle;

    /// 发送数据到协议栈的超时时间，单位毫秒
    int32_t send_data_timeout;

    /// 上报数据的大小阀值
    int32_t notify_size;

    /// 上报数据的间隔时间，单位毫秒
    int32_t notify_interval;

    /// 连接的状态
    cbb_connection_status_t status;

    /// 流控是否打开
    bool xoff;

    /// 关闭模式
    cbb_connection_fin_mode_t close_mode;

    /// 数据来源地址
    char from_addr[FFW_INET6_ADDRSTRLEN];

    /// 数据来源端口
    uint16_t from_port;

    /// 发送数据对端的IP，适用于UDP协议
    const char *push_dstaddr;

    /// 发送数据对端的端口
    uint16_t push_port;

    /// cid
    int32_t cid;

    /// sim id
    int32_t sim;

    /// 源端口
    uint16_t src_port;

    /// 连接地址
    char *dst_addr;

    /// 连接端口
    uint16_t dst_port;

    /// 协议 see ::cbb_protocol_t
    int32_t protocol;

    /// 连接超时时间
    int32_t timeout;

    /// 发送数据到网络的计时器
    void *send_remote_timer;

     /// 上报CLOSE计时器
    ffw_timerp_t close_timer;

    /// 上报数据计时器
    ffw_timerp_t send_te_timer;

    /// @c send_te_timer 是否已经开启
    bool send_te_timer_start;

    /// TCP确认的数据
    uint32_t accumulate_ack;

    /// TCP发送的数据
    uint32_t accumulate_send;

    /// TCP接收数据字节数
    uint32_t totoal_recv;

    /// 缓存模式下累计读取的数据长度
    uint32_t totoal_read;

    /// accept connection
    bool accept_conn;

    /// 如果是accpet的连接，此处表示server id
    int server_id;

    /// engine mode
    int engine_mode;

    /// 是否支持暂时退出ODM，然后ATO重新进入
    bool psudo;

    /// AT上报数据格式
    int32_t iprfmt;

    /// 是否在缓存模式
    bool cache_mode;

    /// 发送数据的crc值， -1 表示没有crc校验
    int32_t crcnum;

    /// 如果连接断开，缓存的数据是否保留
    bool retain_data;

    /// 如果服务器关闭，客户端socket是否关闭
    bool close_client;

    /// 配置ID，ec200使用
    int32_t context_id;

    /// SSL配置ID，ec200使用
    int32_t ssl_contextid;

    /// for ec200
    int32_t access_mode;

    /// for ec200
    char service_type[32];

    /// 缓存中数据大小
    uint32_t cache_data_size;

    /// 是否上报recv urc , for ec200
    bool recv_urc_report;

    /// AT引擎
    void *at_engine;

    /// 是否在ODM模式
    bool odm_mode;

    /// 是否是server端
    bool is_server;

    /// 设置socket options的回调函数
    ffw_socket_callback_t sockfd_callback;

    /// for ec200
    bool ec200_cmd; /// for ec200
    int32_t period; /// 周期发送数据，单位秒
    uint8_t msg_auto[200]; /// 周期发送数据
    uint8_t recv_ignore[200]; /// 忽略的接收数据
    uint16_t msg_auto_len; /// @c msg_auto 的长度
    uint16_t recv_ignore_len;/// @c recv_ignore 的长度
};

/**
* @brief 
* 
* @param max_connections 
* @param rx_buffsize 
* @param tx_buffersize 
* @return int32_t 
 */
int32_t cbb_cmd_mipinit(int32_t max_connections, int32_t rx_buffsize, int32_t tx_buffersize);

/**
 * @brief 
 * 
 * @param id 
 * @param cid 
 * @param sim 
 * @param src_port 
 * @param dst_addr 
 * @param dst_port 
 * @param protocol 
 * @param timeout 
 * @param yy 你懂得 yiyuan
 * @return int32_t 
 */
int32_t cbb_cmd_mipopen(int32_t id, int32_t cid, int32_t sim, uint16_t src_port, const char *dst_addr, uint16_t dst_port, int32_t protocol, int32_t timeout, bool yy);

/**
 * @brief 
 * 
 * @param id 
 * @param mode 
 * @return int32_t 
 */
int32_t cbb_cmd_mipclose(int32_t id, cbb_connection_fin_mode_t mode);

/**
 * @brief 
 * 
 * @param id 
 * @param size 
 * @param timeout 
 * @return int32_t 
 */
int32_t cbb_cmd_mipsets(int32_t id, uint32_t size, uint32_t timeout);

/**
 * @brief 
 * 
 * @param id 
 * @param data 
 * @param size 
 * @param send_mode 
 * @param hex 
 * @return int32_t 
 */
int32_t cbb_cmd_mipsend(int32_t id, const void *data, uint32_t size, cbb_send_mode_t send_mode, bool hex);

/**
 * @brief 
 * 
 * @param id 
 * @param dst 
 * @param port 
 * @return int32_t 
 */
int32_t cbb_cmd_mippush(int32_t id, const char *dst, uint16_t port);

/**
 * @brief 
 * 
 * @param id 
 * @param size 
 * @param read_cb 
 * @param hex 
 * @return int32_t 
 */
int32_t cbb_cmd_mipread(int id, uint32_t size, int32_t (*read_cb)(int32_t id, uint8_t *data, int32_t len, int32_t proto, const char *src_addr, uint16_t src_port, void *arg), bool hex);

/**
 * @brief 
 * 
 * @param id 
 * @param read_cb 
 * @param hex 
 * @return int32_t 
 */
int32_t cbb_cmd_mipread1(int id, int32_t (*read_cb)(int32_t id, uint8_t *data, int32_t len, int32_t proto, const char *src_addr, uint16_t src_port, void *param), bool hex);

/**
* @brief 
* 
* @param id 
* @return int32_t 
 */
int32_t cbb_cmd_read_passive(int id);

/**
* @brief 
* 
* @param cmd_type 
* @param id 
* @param pri 
* @param sec 
* @param arg 
* @return int32_t 
 */
int32_t cbb_cmd_mddns(int32_t id, const char *pri, const char *sec, void *arg);

/**
* @brief 
* 
* @param cmd_type 
* @param hostname 
* @param type 
* @param arg 
* @return int32_t 
 */
int32_t cbb_cmd_mipdns(const char *hostname, uint32_t type, void *arg);

/**
 * @brief 
 * 
 * @param family 
 * @param dst 
 * @param count 
 * @param size 
 * @param ttl 
 * @param tos 
 * @param timeout 
 * @param cb 
 * @param end_cb 
 * @param arg 
 * @return int32_t 
 */
int32_t cbb_cmd_mping(int32_t family, const char *dst, int32_t count, int32_t size, int32_t ttl, int32_t tos, int32_t timeout, ffw_ping_callback_t cb, ffw_ping_end_callback_t end_cb, void *arg);

/**
* @brief 
* 
* @param cmd_type 
* @param id 
* @param arg 
* @return int32_t 
 */
int32_t cbb_cmd_mipflush(int32_t id);

/**
* @brief 
* 
* @param id 
* @param send_size 
* @return int32_t 
 */
int32_t cbb_cmd_get_txfreesize(int32_t id, int32_t *send_size);

/**
* @brief 
* 
* @param id 
* @param data_size 
* @return int32_t 
 */
int32_t cbb_cmd_get_txdatasize(int32_t id, int32_t *data_size);

/**
* @brief 
* 
* @param id 
* @param data_size 
* @return int32_t 
 */
int32_t cbb_cmd_get_rxdatasize(int32_t id, int32_t *data_size);

/**
* @brief 
* 
* @param id 
* @param size 
* @param timeout 
* @return int32_t 
 */
int32_t cbb_cmd_set_timeout_autopush(int32_t id, int32_t size, int32_t timeout);

/**
* @brief 
* 
* @param id 
* @param size 
* @param timeout 
* @return int32_t 
 */
int32_t cbb_cmd_get_timeout_autopush(int32_t id, int32_t *size, int32_t *timeout);

/**
* @brief 
* 
* @param id 
* @param size 
* @param timeout 
* @return int32_t 
 */
int32_t cbb_cmd_set_timeout_te(int32_t id, int32_t size, int32_t timeout);

/**
* @brief 
* 
* @param id 
* @param size 
* @param timeout 
* @return int32_t 
 */
int32_t cbb_cmd_get_timeout_te(int32_t id, int32_t *size, int32_t *timeout);

/**
* @brief 
* 
* @param send_size 
* @param recv_size 
* @return int32_t 
 */
int32_t cbb_cmd_get_statistic(uint64_t *send_size, uint64_t *recv_size);

/**
* @brief 
* 
* @return int32_t 
 */
int32_t cbb_cmd_reset_statistic();

/**
* @brief 
* 
* @return int32_t 
 */
int32_t cbb_cmd_save_statistic();

/**
* @brief 
* 
* @return int32_t 
 */
int32_t cbb_cmd_ping_abort();

/**
* @brief 
* 
* @param num 
* @param rx_size 
* @param tx_size 
* @return int32_t 
 */
int32_t cbb_cmd_get_info(int32_t *num, int32_t *rx_size, int32_t *tx_size);

/**
* @brief 
* 
* @param id 
* @param send_size 
* @param ack_size 
* @return int32_t 
 */
int32_t cbb_cmd_get_accumulate_ack_size(int32_t id, uint32_t *send_size, uint32_t *ack_size);

/**
 * @brief 
 * 
 * @param evt 
 * @return const char* 
 */
const char *cbb_cmd_evtname(cbb_event_t evt);

/**
 * @brief 
 * 
 * @param id 
 * @return true 
 * @return false 
 */
bool cbb_cmd_is_conn_status(int32_t id);

/**
* @brief 
* 
* @param cb 
* @param arg 
* @return int32_t 
 */
int32_t cbb_cmd_get_idle_conn(void (*cb)(int32_t id, void *arg), void *arg);

/**
* @brief 
* 
* @param cb 
* @param arg 
* @return int32_t 
 */
int32_t cbb_cmd_get_busy_conn(void (*cb)(int32_t id, void *arg), void *arg);

/**
* @brief 
* 
* @param evt_cb 
* @param data_cb 
* @return int32_t 
 */
int32_t cbb_cmd_set_callbacks(evt_callback_t evt_cb);

/**
 * @brief The connection of indicate by id is active?
 * 
 * @param id The id of the connection
 * @return true active
 * @return false deactive
 */
bool cbb_cmd_conn_is_active(int32_t id);

/**
 * @brief 根据id获取连接
 * 
 * @param id id
 * @return cbb_connection_t* tcp连接
 */
int32_t cbb_cmd_get_conn(int32_t id, cbb_connection_t **conn);

/**
* @brief 
* 
* @param id 
* @param period 
* @param data 
* @param len 
* @param hex 
* @return int32_t 
 */
int32_t cbb_cmd_ec200_set_period_data(int32_t id, int32_t period, const uint8_t *data, int len, bool hex);

/**
* @brief 
* 
* @param id 
* @param data 
* @param len 
* @param hex 
* @return int32_t 
 */
int32_t cbb_cmd_ec200_set_ignore_data(int32_t id, const uint8_t *data, int len, bool hex);

#endif /* CBB_INCLUDE_CBB_TCPIP */
