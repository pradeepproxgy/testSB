#ifndef LIB_INCLUDE_FFW_MQTTC_INTERFACE
#define LIB_INCLUDE_FFW_MQTTC_INTERFACE

/**
 * @file ffw_mqttc_interface.h
 * @author sundaqing (sundaqing@fibocom.com)
 * @brief MQTT MQTT-SN接口定义
 * @version 0.1
 * @date 2021-05-30
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <stdint.h>
#include <stdbool.h>

/// set user password
#define ffw_mqttc_i_set_user_pwd(c, username, password) ((c)->set_user_pwd)(c, username, password)

/// get user password
#define ffw_mqttc_i_get_user_pwd(c, username, password) ((c)->get_user_pwd)(c, username, password)

/// set will
#define ffw_mqttc_i_set_will(c, topic, qos, retain, payload, payload_len) ((c)->set_will)(c, topic, qos, retain, payload, payload_len)

/// get will
#define ffw_mqttc_i_get_will(c, topic, qos, retain, payload, payload_len) ((c)->get_will)(c, topic, qos, retain, payload, payload_len)

/// set client id
#define ffw_mqttc_i_set_clientid(c, clientid) ((c)->set_clientid)(c, (clientid))

/// get client id
#define ffw_mqttc_i_get_clientid(c, clientid) ((c)->get_clientid)(c, (clientid))

/// enter psk mode when use tls
#define ffw_mqttc_i_set_pskmode(c, psk_mode) ((c)->set_pskmode)(c, (psk_mode))

/// connect mqtt in async mode
#define ffw_mqttc_i_connect(c, host, port, clearsession, keepalive, tls, timeout) ((c)->connect)(c, host, port, clearsession, keepalive, tls, timeout)


/// connect mqtt in async mode， 支持断开重连
#define ffw_mqttc_i_connect1(c, host, port, clearsession, keepalive, tls, timeout) ((c)->connect1)(c, host, port, clearsession, keepalive, tls, timeout)

/// sub
#define ffw_mqttc_i_sub(c, topic, topic_len, qos) ((c)->sub)(c, topic, topic_len, qos)

/// unsub
#define ffw_mqttc_i_unsub(c, topic, topic_len) ((c)->unsub)(c, topic, topic_len)

/// pub
#define ffw_mqttc_i_pub(c, topic, topic_len, qos, retain, dup, payload, payload_len) ((c)->pub)(c, topic, topic_len, qos, retain, dup, payload, payload_len)

/// reg topic
#define ffw_mqttc_i_reg_topic(c, topic) ((c)->topic_reg)(c, topic)

/// update will topic
#define ffw_mqttc_i_topicud(c, topic, qos, retain) ((c)->willtopicud)(c, topic, qos, retain)

/// update will message
#define ffw_mqttc_i_messageud(c, message) ((c)->willmsgud)(c, message)

/// disconnect
#define ffw_mqttc_i_disconnect(c) ((c)->disconnect)(c)

/// connect
#define ffw_mqttc_i_is_connect(c) ((c)->is_connect)(c)

/// destory
#define ffw_mqttc_i_destroy(c, t) ((c)->destroy)(c)


/// set netif
#define ffw_mqttc_i_setneif(c, netif) ((c)->set_netif)(c, netif)

/// mqtt socket id
#define MQTT_SOCKET_ID (0)

/// type of ffw_mqttc_interface_s
typedef struct ffw_mqttc_interface_s ffw_mqttc_interface_t;

/// type of ffw_mqttc_interface_opt_s
typedef struct ffw_mqttc_interface_opt_s ffw_mqttc_interface_opt_t;

/**
* @brief MQTT客户端上报事件定义
* 
 */
enum ffw_mqtt_event_s
{
    /// CONN ACK
    FFW_MQTT_CONN_ACK = 0,

    /// PUB ACK
    FFW_MQTT_PUB_ACK,
    /// PUB DATA
    FFW_MQTT_PUB_DATA,

    /// SUB ACK
    FFW_MQTT_SUB_ACK,
    /// UNSUB ACK
    FFW_MQTT_UNSUB_ACK,

    /// DISCOON ACK
    FFW_MQTT_DISCONN_ACK, //5

    /// DISCONNECT
    FFW_MQTT_DISCONN,

    /// reCONNECT
    FFW_MQTT_RECONN_ACK,

    /// REG
    FFW_MQTT_REG,

    /// REG ACK
    FFW_MQTT_REG_ACK,

    /// WILL TOPCI ACK
    FFW_MQTT_WILLTOPICUD_ACK, //10

    /// WILL MESSAGE ACK
    FFW_MQTT_WILLMESGUD_ACK,

    /// TOTAL COUNT
    FFW_MQTT_EVT_COUNT,
};

/**
* @brief MQTT qos 定义
* 
 */
enum ffw_mqtt_qos_s
{
    /// QOS0: 消息最多传递一次，如果当时客户端不可用，则会丢失该消息。
    FFW_MQTT_QOS0 = 0,

    /// QOS1: 消息传递至少 1 次。可能会重复
    FFW_MQTT_QOS1,

    /// QOS2：消息仅传送一次。
    FFW_MQTT_QOS2,

    /// QOS3: MQTTSN publish messages without first creating a connection by using QOS of -1 or 3.
    FFW_MQTT_QOS3,

    /// QOS-1: MQTTSN publish messages without first creating a connection by using QOS of -1 or 3.
    FFW_MQTT_SUBFAIL = -1
};

/**
* @brief MQTT 客户端状态定义
* 
 */
enum ffw_mqttc_status_s
{
    /// 未连接
    FFW_MQTTC_DISCONNECTED,

    /// 连接中
    FFW_MQTTC_CONNECTING,

    /// 已连接
    FFW_MQTTC_CONNECTTED,

    /// 断开中
    FFW_MQTTC_DISCONNECTTING
};

/**
* @brief MQTT 客户端链接状态定义
* 
 */
enum ffw_mqttc_connect_ack_s
{
    /// 连接已接受
    FFW_MQTTC_CONNECTION_ACCEPTED = 0,

    /// 服务端不支持客户端请求的MQTT协议级别
    FFW_MQTTC_UNNACCEPTABLE_PROTOCOL,

    /// 客户端标识符是UTF-8编码，但服务端不允许使用
    FFW_MQTTC_CLIENTID_REJECTED,

    /// 网络连接已建立，但MQTT服务不可用
    FFW_MQTTC_SERVER_UNAVAILABLE,

    /// 用户名或密码的数据格式无效
    FFW_MQTTC_BAD_USERNAME_OR_PASSWORD,

    /// 客户端未被授权连接到此服务器
    FFW_MQTTC_NOT_AUTHORIZED,

    /// 其他错误，暂不细分
    FFW_MQTTC_OTHER_ERROR
};

/**
* @brief MQTT客户单端运行中状态定义
* 
 */
enum ffw_mqtt_state_s
{
    /// IDLE
    FFW_MQTT_STATE_IDLE = 0,

    /// OPEN ING
    FFW_MQTT_STATE_OPEN,

    /// SUB ING
    FFW_MQTT_STATE_SUB,

    /// UNSUB ING
    FFW_MQTT_STATE_UNSUB,

    /// PUB ING
    FFW_MQTT_STATE_PUB,

    /// CLOSE ING
    FFW_MQTT_STATE_CLOSE,

    /// REG ING
    FFW_MQTT_STATE_REG,

    /// WILL TOPIC UPDATE ING
    FFW_MQTT_STATE_WILLTOPICUD,

    /// WILL MESSAGE UPDATE ING
    FFW_MQTT_STATE_WILLMESGUD,
};

/// type of ffw_mqtt_state_s
typedef enum ffw_mqtt_state_s ffw_mqtt_state_t;

/// type of ffw_mqtt_event_s
typedef enum ffw_mqtt_event_s ffw_mqtt_event_t;

/// type of ffw_mqttc_status_s
typedef enum ffw_mqttc_status_s ffw_mqttc_status_t;

/// type of ffw_mqtt_qos_s
typedef enum ffw_mqtt_qos_s ffw_mqtt_qos_t;

/**
* @brief 事件回调函数定义
* 
* @param handle MQTT/MQTTSN 实例
* @param evt 事件值
* @param pacid The id of packet
* @param result 结果 0：失败 1：成功
* @param arg 回调上下文
 */
typedef void (*ffw_mqttc_interface_cb_t)(void *handle, ffw_mqtt_event_t evt, uint16_t pacid, int result, void *arg);

/**
* @brief PUBLISH消息回调函数定义
* 
* @param handle MQTT/MQTTSN 实例
* @param topic 主题
* @param topic_len 主题长度
* @param qos qos
* @param payload 数据
* @param len 数据长度
* @param arg 回调上下文
 */
typedef void (*ffw_mqttc_interface_on_data_t)(void *handle, char *topic, int topic_len, uint8_t qos, uint8_t *payload, uint32_t len, void *arg);

/**
* @brief PUBLISH消息回调函数定义
* 
* @param handle MQTT/MQTTSN 实例
* @param topic 主题
* @param topic_len 主题长度
* @param qos qos
* @param payload 数据
* @param len 数据长度
* @param arg 回调上下文
 */
typedef void (*ffw_mqttc_interface_on_data_msgid_t)(void *handle, char *topic, int topic_len, uint8_t qos, uint8_t *payload, uint32_t len, uint32_t msg_id, void *arg);

/**
* @brief 注册主题回调函数定义
* 
* @param handle MQTT/MQTTSN 实例
* @param topic_id 主题ID
* @param topic 主题
* @param topic_len 主题长度
* @param result 结果 0：失败 1：成功
* @param arg 回调上下文
 */
typedef void (*ffw_mqttc_interface_on_regack_t)(void *handle, uint16_t topic_id, int result, void *arg);

/**
* @brief 服务器主动通报注册主题回调函数定义
* 
* @param handle MQTT/MQTTSN 实例
* @param topic_id 主题ID
* @param topic 主题
* @param topic_len 主题长度
* @param result 结果 0：失败 1：成功
* @param arg 回调上下文
 */
typedef void (*ffw_mqttc_interface_on_reg_t)(void *handle, uint16_t topic_id, const char *topic, int topic_len, int result, void *arg);

/// type of ffw_mqtt_message_s
typedef struct ffw_mqtt_message_s ffw_mqtt_message_t;

/// option 定义
struct ffw_mqttc_interface_opt_s
{
    /// 事件回调函数
    ffw_mqttc_interface_cb_t evt_cb;

    /// 注册主题回调函数
    ffw_mqttc_interface_on_regack_t regack;

    /// 服务器主动通报注册主题回调函数
    ffw_mqttc_interface_on_reg_t on_reg;

    /// 消息回调函数
    ffw_mqttc_interface_on_data_t data_cb;

    /// 回调上下文
    void *arg;
};

/**
* @brief MQTT PUBLISH 消息定义
* 
 */
struct ffw_mqtt_message_s
{
    /// qos
    int qos;

    /// 是否保存
    uint8_t retained;

    /// 是否重复
    uint8_t dup;

    /// 消息ID
    uint16_t id;

    /// 数据
    void *payload;

    /// 数据长度
    uint32_t payloadlen;

    /// 主题
    char *topic_name;

    /// 主题长度
    int topic_len;

    /// 预留
    void *arg;
};

/**
* @brief MQTT接口定义
* 
 */
struct ffw_mqttc_interface_s
{
    /// 实例名字
    const char *name;

    /// 实例句柄
    void *client_handle;

    /// 设置用户名密码函数
    int (*set_user_pwd)(ffw_mqttc_interface_t *c, const char *username, const char *password);

    /// 获取用户名密码函数
    int (*get_user_pwd)(ffw_mqttc_interface_t *client, const char **username, const char **password);

    /// 设置遗嘱函数
    int (*set_will)(ffw_mqttc_interface_t *c, const char *topic, int qos, bool retain, const uint8_t *payload, uint16_t payload_len);

    /// 获取遗嘱函数
    int (*get_will)(ffw_mqttc_interface_t *client, const char **topic, int *qos, bool *retain, const uint8_t **payload, uint16_t *payload_len);

    /// 设置CLIENT ID函数
    int (*set_clientid)(ffw_mqttc_interface_t *c, const char *client_id);

    /// 获取CLIENT ID函数
    int (*get_clientid)(ffw_mqttc_interface_t *c, const char **client_id);

    /// 设置psk模式
    int (*set_pskmode)(ffw_mqttc_interface_t *c, bool psk_mode);

    /// 销毁实例
    int (*destroy)(ffw_mqttc_interface_t *c);

    /// 异步连接函数
    int (*connect)(ffw_mqttc_interface_t *c, const char *host, uint16_t port, bool clearsession, uint32_t keepalive, bool tls, int32_t timeout);

    /// 异步连接函数,支持重连
    int (*connect1)(ffw_mqttc_interface_t *c, const char *host, uint16_t port, bool clearsession, uint32_t keepalive, bool tls, int32_t timeout);

    /// 设置网卡
    int (*set_netif)(ffw_mqttc_interface_t *c, void *netif);

    /// 订阅
    int (*sub)(ffw_mqttc_interface_t *c, char *topic, int topic_len, int qos);

    /// 去订阅
    int (*unsub)(ffw_mqttc_interface_t *c, char *topic, int topic_len);

    /// 发布
    int (*pub)(ffw_mqttc_interface_t *c, char *topic, int topic_len, int qos, bool retain, bool dup, uint8_t *payload, uint16_t payload_len);

    /// 更新遗嘱topic
    int (*willtopicud)(ffw_mqttc_interface_t *c, char *topic, int qos, bool retain);

    /// 更新遗嘱消息
    int (*willmsgud)(ffw_mqttc_interface_t *c, char *message);

    /// 注册主题
    int (*topic_reg)(ffw_mqttc_interface_t *c, char *topic);

    /// 断开连接
    int (*disconnect)(ffw_mqttc_interface_t *c);

    /// 是否已经连接
    int (*is_connect)(ffw_mqttc_interface_t *c);
};

#endif /* LIB_INCLUDE_FFW_MQTTC_INTERFACE */
