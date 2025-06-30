#ifndef F494367F_007A_44E2_B86D_53BE23481B1D
#define F494367F_007A_44E2_B86D_53BE23481B1D

/**
* @file ffw_mqttc.h
* @author sundaqing (sundaqing@fibocom.com)
* @brief MQTT 客户端实现
* @version 0.1
* @date 2021-05-18
* 
* Copyright (c) 2021 Fibocom. All rights reserved
* 
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "MQTTPacket.h"
#include "ffw_transport_interface.h"
#include "ffw_mqttc_interface.h"
#include "ffw_list.h"
#include "ffw_timer.h"
#include "ffw_buffer.h"

/// The max length of mqtt packet
#define MQTT_MAX_INCOMING_DATA_LEN (8192)

/// The max mqtt packet id
#define MAX_PACKET_ID (UINT16_MAX)

/// type of ffw_mqttc_s
typedef struct ffw_mqttc_s ffw_mqttc_t;

/// type of ffw_mqttc_s MQTTHeader
typedef MQTTHeader ffw_mqtt_header_t;

/// type of ffw_mqttc_s mqtt_packet_info_s
typedef struct mqtt_packet_info_s mqtt_packet_info_t;

/**
* @brief MQTT 消息解析状态机状态
* 
 */
enum mqtt_packet_status_s
{
    ///解析消息头部
    MQTT_PACKET_STATUS_HEADER = 0,

    ///解析消息数据的长度
    MQTT_PACKET_STATUS_LENGTH,

    ///解析消息数据
    MQTT_PACKET_STATUS_DATA,

    ///由于数据太大，忽略当前消息
    MQTT_PACKET_STATUS_IGNORE_DATA
};

/// type of mqtt_packet_status_s
typedef enum mqtt_packet_status_s mqtt_packet_status_t;

/**
* @brief MQTT 消息解析过程信息
* 
 */
struct mqtt_packet_info_s
{
    ///MQTT消息头部
    ffw_mqtt_header_t header;

    ///数据大小
    uint32_t data_size;

    ///计算数据的乘数
    uint32_t multiplier;

    ///长度占的字节数
    uint32_t remain_bytes;

    ///数据
    ffw_buffer_t data;

    ///当前状态
    mqtt_packet_status_t st;
};

/**
* @brief MQTT客户端定义
* 
 */
struct ffw_mqttc_s
{
    ///是否已经初始化
    bool init;

    ///是否使用PSK认证模式
    bool psk_mode;

    ///MQTT是否处于连接状态
    ffw_mqttc_status_t status;

    ///是否在等待ping的回应消息
    bool waiting_pingres;

    ///事件回调函数
    ffw_mqttc_interface_cb_t event_cb;

    ///收到消息回调函数
    ffw_mqttc_interface_on_data_t on_data;

    //收到消息回调函数，包含msg_id
    ffw_mqttc_interface_on_data_msgid_t on_data_msgid;

    ///回调上下文
    void *arg;

    ///MQTT 连接时的数据定义，包括用户名 密码 qos keepalive等等
    MQTTPacket_connectData conn_options;

    ///下一个消息ID
    uint32_t next_packetid;

    ///发送消息的b缓冲区
    uint8_t send_buff[MQTT_MAX_INCOMING_DATA_LEN];

    ///解析MQTT消息时的中间状态
    mqtt_packet_info_t pkt_info;

    ///TCP OR TLS 连接句柄
    ffw_transport_t t;

    /// TLS config id
    int config_id;

    /// 使用的网卡
    ffw_handle_t netif;

    /// send heartbeat timer
    ffw_timerp_t send_timer;

    /// recv timer
    ffw_timerp_t recv_timer;

    /// reconnect timer
    ffw_timerp_t reconnect_timer;

    /// 是否TLS
    bool tls;

    /// hostname
    char *hostname;

    /// port
    uint16_t port;

    /// 连接超时时间，单位毫秒
    int32_t conn_timeout;

    /// 是否重连
    bool reconnect;

    /// 重连机制启动
    bool start_reconnect;

    /// connect ack status
    uint8_t connect_ack;
};

/**
* @brief 初始化MQTT实例
* 
* @param client 实例
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_mqttc_init(ffw_mqttc_t *client);

/**
* @brief 设置MQTT事件和数据上报函数
* 
* @param client 实例
* @param evt_cb 事件上报回调函数
* @param data_cb 数据上报回调函数
* @param arg 回调上下文
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_mqttc_set_callback(ffw_mqttc_t *client, ffw_mqttc_interface_cb_t evt_cb, ffw_mqttc_interface_on_data_t data_cb, void *arg);

/**
* @brief 设置MQTT事件和数据上报函数
* 
* @param client 实例
* @param evt_cb 事件上报回调函数
* @param data_cb 数据上报回调函数，包含msgID
* @param arg 回调上下文
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_mqttc_set_callback1(ffw_mqttc_t *client, ffw_mqttc_interface_cb_t evt_cb, ffw_mqttc_interface_on_data_msgid_t data_cb, void *arg);

    /**
 * @brief 释放MQTT实例
 * 
 * @param client 实例句柄
 * @return int 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 */
int ffw_mqttc_destory(ffw_mqttc_t *client);

/**
 * @brief 设置MQTT用户名密码
 * 
 * @param client 实例
 * @param username 用户名
 * @param password 密码
 * @return int 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 */
int ffw_mqttc_set_user_pwd(ffw_mqttc_t *client, const char *username, const char *password);

/**
 * @brief 设置MQTT用户名密码
 * 
 * @param client 实例
 * @param username 用户名
 * @param namelen 用户名长度
 * @param password 密码
 * @param passwdlen 密码长度
 * @return int 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 */
int ffw_mqttc_set_user_pwd1(ffw_mqttc_t *client, const uint8_t *username, uint32_t namelen, const uint8_t *password, uint32_t passwdlen);

/**
 * @brief 获取用户名密码
 * 
 * @param client 实例
 * @param[out] username 用户名 
 * @param[out] password 密码
 * @return int
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败 
 */
int ffw_mqttc_get_user_pwd(ffw_mqttc_t *client, const char **username, const char **password);

/**
 * @brief 设置遗嘱
 * 
 * @param client 实例
 * @param topic 主题
 * @param qos 稳定传输级别
 * @param retain 是否保留信息 
 * @param payload 消息内容
 * @param payload_len 消息长度
 * @return int
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 */
int ffw_mqttc_set_will(ffw_mqttc_t *client, const char *topic, int qos, bool retain, const uint8_t *payload, uint16_t payload_len);

/**
 * @brief 获取遗嘱
 * 
 * @param client 实例
 * @param[out] topic 主题
 * @param[out] qos 稳定传输级别
 * @param[out] retain 是否保留信息 
 * @param[out] payload 消息内容
 * @param[out] payload_len 消息长度
 * @return int
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 */
int ffw_mqttc_get_will(ffw_mqttc_t *client, const char **topic, int *qos, bool *retain, const uint8_t **payload, uint16_t *payload_len);

/**
* @brief 设置MQTT的client id
* 
* @param c 实例
* @param client_id MQTT client id
* @return int
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_mqttc_set_clientid(ffw_mqttc_t *c, const char *client_id);

/**
* @brief 获取MQTT的client id
* 
* @param c 实例
* @param[out] client_id MQTT client id
* @return int
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_mqttc_get_clientid(ffw_mqttc_t *c, const char **client_id);

/**
* @brief 设置为PSK认证模式，只有在TLS模式下才起作用
* 
* @param c 实例
* @param psk_mode true: 启用psk认证模式 false: 不使用psk认证模式
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_mqttc_set_psk_mode(ffw_mqttc_t *c, bool psk_mode);

/**
* @brief 连接MQTT，支持是否使用TLS和指定超时时间 
* 
* @param c 实例
* @param client_id mqtt client id
* @param host 远端地址
* @param port mqtt服务端口
* @param clearsession 是否清除回话
* @param keepalive 心跳时间，单位为秒
* @param tls 是否使用TLS连接 
* @param timeout 超时时间，单位为毫秒，如果小于等于0，则超时时间取决于网络情况
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_mqttc_connect(ffw_mqttc_t *c, char *client_id, const char *host, uint16_t port, bool clearsession, uint32_t keepalive, bool tls, int32_t timeout);


/**
* @brief 连接MQTT，支持是否使用TLS和指定超时时间,断开之后会重连 
* 
* @param c 实例
* @param client_id mqtt client id
* @param host 远端地址
* @param port mqtt服务端口
* @param clearsession 是否清除回话
* @param keepalive 心跳时间，单位为秒
* @param tls 是否使用TLS连接 
* @param timeout 超时时间，单位为毫秒，如果小于等于0，则超时时间取决于网络情况
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_mqttc_connect1(ffw_mqttc_t *c, char *client_id, const char *host, uint16_t port, bool clearsession, uint32_t keepalive, bool tls, int32_t timeout);

/**
* @brief 连接MQTT，支持是否使用TLS和指定超时时间,断开之后会重连 
* 
* @param c 实例
* @param client_id mqtt client id
* @param host 远端地址
* @param port mqtt服务端口
* @param clearsession 是否清除回话
* @param keepalive 心跳时间，单位为秒
* @param tls 是否使用TLS连接 
* @param timeout 超时时间，单位为毫秒，如果小于等于0，则超时时间取决于网络情况
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_mqttc_connect2(ffw_mqttc_t *c, char *client_id, const char *host, uint16_t port, bool clearsession, uint32_t keepalive, bool tls, int32_t timeout);

/**
* @brief 订阅主题
* 
* @param c 实例
* @param topic 主题
* @param qos 服务质量，取值为 ::FFW_MQTT_QOS0 or ::FFW_MQTT_QOS1 or ::FFW_MQTT_QOS2
* @param pacid 消息ID
* @return int
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败  
 */
int ffw_mqttc_sub(ffw_mqttc_t *c, char *topic, int qos, uint16_t *pacid);

/**
* @brief 取消订阅的主题
* 
* @param client 实例
* @param topic 主题
* @param pacid 消息ID
* @return int
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败  
 */
int ffw_mqttc_unsub(ffw_mqttc_t *client, char *topic, uint16_t *pacid);

/**
* @brief 发布消息
* 
* @param client  实例
* @param topic 主题
* @param qos 服务质量 取值为 ::FFW_MQTT_QOS0 ::FFW_MQTT_QOS1 ::FFW_MQTT_QOS2
* @param retain 是否保存消息
* @param dup 是否是重复消息
* @param payload 消息数据
* @param payload_len 消息数据长度
* @param pacid 消息ID
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_mqttc_pub(ffw_mqttc_t *client, char *topic, int qos, bool retain, bool dup, uint8_t *payload, uint16_t payload_len, uint16_t *pacid);

/**
* @brief 发布消息。此接口需要和 ::ffw_mqttc_pub_data 配合使用，先使用这个接口发送MQTT消息头部和可变头部，然后再使用 ::ffw_mqttc_pub_data 发送数据，
*        如果数据比较大，可以调用多次，发送的数据的长度必须和参数 @c payload_len 一致
* 
* @param c 实例
* @param topic 主题
* @param qos 服务质量,取值为 ::FFW_MQTT_QOS0 ::FFW_MQTT_QOS1 ::FFW_MQTT_QOS2
* @param retain 是否保存消息
* @param dup 是否是重复消息
* @param payload_len 消息数据长度
* @param pacid 消息ID
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_mqttc_pub_header(ffw_mqttc_t *c, char *topic, int qos, bool retain, bool dup, uint16_t payload_len, uint16_t *pacid);

/**
* @brief 发送消息的数据部分，参考 ::ffw_mqttc_pub_header 说明
* 
* @param c 实例
* @param data 数据
* @param data_len 数据长度
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_mqttc_pub_data(ffw_mqttc_t *c, uint8_t *data, uint16_t data_len);

/**
 * @brief 断开MQTT连接
 * 
 * @param client 实例
 * @return int 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败 
 */
int ffw_mqttc_disconnect(ffw_mqttc_t *client);

/**
* @brief 判断mqtt实例是否已经连接
* 
* @param c 实例
* @return bool 
* @retval true: 连接状态
* @retval false: 非连接状态 
 */
bool ffw_mqttc_is_connected(ffw_mqttc_t *c);

/**
* @brief 实现了mqtt 接口
* 
* @param c 实例
* @param opt 选项
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_mqttc_init_interface(ffw_mqttc_interface_t *c, ffw_mqttc_interface_opt_t *opt);

/**
* @brief 设置MQTT连接使用的cid
* 
* @param client 实例
* @param netif 使用的网卡
 */
int ffw_mqttc_set_netif(ffw_mqttc_t *client, ffw_handle_t netif);

#endif /* F494367F_007A_44E2_B86D_53BE23481B1D */
