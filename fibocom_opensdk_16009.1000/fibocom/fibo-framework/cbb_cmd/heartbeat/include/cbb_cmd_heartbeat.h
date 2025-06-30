#ifndef CBB_INCLUDE_CBB_CMD_HEARTBEAT
#define CBB_INCLUDE_CBB_CMD_HEARTBEAT

#include "ffw_socket.h"
#include "ffw_network.h"
#include "ffw_log.h"
#include "ffw_result.h"
#include "ffw_utils.h"
#include "ffw_timer.h"
#include "ffw_os.h"
#include "ffw_transport_poll.h"
#include "ffw_file.h"
#include "ffw_utils.h"
#include "ffw_broker.h"
#include "ffw_event.h"

#define CBB_HEARTBEAT_SERVER_MIN_LEN 1
#define CBB_HEARTBEAT_SERVER_LEN 64
#define CBB_HEARTBEAT_DATA_HEX_MIN_LEN 2
#define CBB_HEARTBEAT_DATA_HEX_LEN 200
#define CBB_HEARTBEAT_DATA_LEN 100
#define CBB_HEARTBEAT_PORT_MIN 1
#define CBB_HEARTBEAT_PORT_MAX 65535
#define CBB_HEARTBEAT_PORT_DEFAULT 1
#define CBB_HEARTBEAT_TIME_MIN 1
#define CBB_HEARTBEAT_TIME_MAX 65535
#define CBB_HEARTBEAT_TIME_DEFAULT 10

#define CBB_HEARTBEAT_RECONNECT_MAX 100

#define CBB_HEARTBEAT_TIME_NUM 6
#define CBB_HEARTBEAT_DISABLE  0
#define CBB_HEARTBEAT_ENABLE   1
#define CBB_HEARTBEAT_TCP      0
#define CBB_HEARTBEAT_UDP      1
/* heartbeat time 1-6 , min*/
#define CBB_HEARTBEAT_BASE_TIME   2

#define CBB_HEARTBEAT_TIMEOUT  30*1000
#define CBB_HEARTBEAT_BUFFER_MAX 128

#define CBB_HEARTBEAT_TIME_PATH FFW_BASE_DIRNAME FFW_FS_SEP "hearbeattime"
#define CBB_HEARTBEAT_PARAM_PATH FFW_BASE_DIRNAME FFW_FS_SEP "hearbeat_param"
#define CBB_HEARTBEAT_DEFAULT_SOCKET 1
#define CBB_HEARTBEAT_MIN_SOCKET 1
#define CBB_HEARTBEAT_MAX_SOCKET 6

#define CBB_HEARTBEAT_DATA_HEX 1
#define CBB_HEARTBEAT_DATA_STR 0

typedef int (*cbb_hb_event_callback_t)(int32_t event, void *arg);
typedef int (*cbb_hb_recv_callback_t)(uint8_t *data, int len, void *arg);


typedef enum cbb_hb_connection_status_s
{
    CBB_HB_CONNECTION_IDLE = 0,
    CBB_HB_CONNECTION_CONNECTING,
    CBB_HB_CONNECTION_CONNECTTED,
    CBB_HB_CONNECTION_CLOSING,
    CBB_HB_CONNECTION_SERVER,
    CBB_HB_CONNECTION_REPEAT
} cbb_hb_connection_status_t;

typedef enum cbb_hb_event_s
{
    CBB_HB_CONNECT_SUCCESS = 0,
    CBB_HB_RECEIVE_MESSAGE,
    CBB_HB_RECONNECT_START,
    CBB_HB_RECONNECT_SUCCESS,
    CBB_HB_DISCONNECT
} cbb_hb_event_t;

typedef enum cbb_heartbeat_client_s
{
    CBB_HEARTBEAT_CLIENT_1 = 0,
    CBB_HEARTBEAT_CLIENT_2,
    CBB_HEARTBEAT_CLIENT_3,
    CBB_HEARTBEAT_CLIENT_MAX
} cbb_heartbeat_client_t;

typedef enum cbb_heartbeat_ctrl_s
{
    CBB_HEARTBEAT_OPEN = 0,
    CBB_HEARTBEAT_CLOSE
} cbb_heartbeat_ctrl_t;

typedef enum cbb_heartbeat_param_ctrl_s
{
    CBB_HEARTBEAT_SAVE_PARAM = 0,
    CBB_HEARTBEAT_NO_SAVE_PARAM
} cbb_heartbeat_param_ctrl_t;

/**
 * @brief heartbeat time
 *
 */
typedef struct
{
    int32_t index;
    uint32_t time[CBB_HEARTBEAT_TIME_NUM];
}cbb_heartbeat_time_t;

typedef struct
{
    int32_t mode;
    int32_t protocol;
    char server_addr[CBB_HEARTBEAT_SERVER_LEN + 1];
    uint16_t port;
    uint32_t interval;
    char data[CBB_HEARTBEAT_DATA_HEX_LEN + 1];
    int32_t socketid;
}cbb_hearbeat_param_t;


/**
 * @brief
 *
 *
 *
 */
typedef struct
{
    /// TCP/UDP 连接 id
    int32_t id;

    /// TCP/UDP 连接实例
    ffw_transport_t t;

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

    /// 心跳数据
    ffw_buffer_t data;

    /// 心跳数据字符串形式
    char *data_str;

    /// 心跳定时器时间
    cbb_heartbeat_time_t heartbeat_time;

    /// 心跳间隔
    uint32_t interval;

    /// 心跳定时器
    void *heartbeat_timer;

    /// 数据类型
    uint8_t data_type;

    /// 连接的状态
    cbb_hb_connection_status_t status;

    /// 连续未回复ACK的次数
    uint32_t continuous_noack;

    int32_t mode;

    cbb_hb_event_callback_t event;
    cbb_hb_recv_callback_t recv_cb;

    void *event_arg;

    uint8_t switch_ctrl;

    /// 重连次数
    uint8_t reconn_time;

    /// 重发次数
    uint8_t resend_time;

    /// 心跳ID
    uint8_t heartbeat_id;

    /// 参数保存标记
    uint8_t save_param_ctrl;

    /// AT引擎
    void *at_engine;
}cbb_heartbeat_t;

typedef void (*cbb_cmd_gthben_read_cb)(void *engine, int32_t mode, int32_t protocol);
typedef void (*cbb_cmd_gthbtime_read_cb)(void *engine, uint32_t time1, uint32_t time2, uint32_t time3, uint32_t time4, uint32_t time5, uint32_t time6);

int32_t cbb_cmd_gthben_get(int32_t *mode, int32_t *protocol);
int32_t cbb_cmd_gthbeat_get(char **dst_addr, uint16_t *dst_port, uint32_t *time, char **data, int32_t *id);
int32_t cbb_cmd_gthbtime_get(
    uint32_t *time1,
    uint32_t *time2,
    uint32_t *time3,
    uint32_t *time4,
    uint32_t *time5,
    uint32_t *time6);

int32_t cbb_cmd_gthben_read(void *engine, cbb_cmd_gthben_read_cb cb);
int32_t cbb_cmd_gthbtime_read(void *engine, cbb_cmd_gthbtime_read_cb cb);
int32_t cbb_cmd_gthben_set(int32_t mode, int32_t protocol);
int32_t cbb_cmd_gthben_set_ext(uint8_t cid,  uint8_t nsim, int32_t mode, int32_t protocol);
int32_t cbb_cmd_gthbeat_set(const char *dst_addr, uint16_t dst_port, uint32_t time, const char *data, int32_t id);
int32_t cbb_cmd_gthbtime_set(uint32_t time1, uint32_t time2, uint32_t time3, uint32_t time4, uint32_t time5, uint32_t time6);
int32_t cbb_cmd_gthbtime_reset(void);
int32_t cbb_cmd_heartbeat_pdp_cb_register(void);

int32_t cbb_cmd_heartbeat_data_type(uint8_t id, int32_t type);
int32_t cbb_cmd_heartbeat_data_type_get(uint8_t id, int32_t *type);
int32_t cbb_cmd_heartbeat_param_set(uint8_t id, const char *dst_addr, uint16_t dst_port, uint32_t time,
    const char *data, cbb_hb_event_callback_t cb, cbb_hb_recv_callback_t recv, void *arg);
int32_t cbb_cmd_heartbeat_param_get(uint8_t id, char **dst_addr, uint16_t *dst_port, uint32_t *time, char **data);
int32_t cbb_cmd_heartbeat_send(uint8_t id, uint8_t *msg, int32_t len);
int32_t cbb_cmd_heartbeat_control(uint8_t id, uint8_t ctrl);
int32_t cbb_cmd_heartbeat_reconnect_time(uint8_t id, uint8_t time);
int32_t cbb_cmd_heartbeat_create(uint8_t id, int32_t mode, int32_t protocol);
int32_t cbb_cmd_heartbeat_save_param(uint8_t id, uint8_t ctrl);
int32_t cbb_cmd_heartbeat_mode_get(uint8_t id, int32_t *mode, int32_t *protocol);
int32_t cbb_cmd_heartbeat_resend_time(uint8_t id, uint8_t time);

#endif
