/**
 * @file ffw_channel.h
 * @author sundaqing (you@domain.com)
 * @brief 配置每个协议使用的通道，也就是cid的配置
 * @version 0.1
 * @date 2021-08-19
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "stdint.h"

#ifndef C562F499_36F5_4C43_9A39_B97922849C6F
#define C562F499_36F5_4C43_9A39_B97922849C6F

#define FFW_PROTOCOL_DEFAULT_CID (0)

/**
 * @brief 每个协议对应的通道ID
 * 
 */
typedef enum
{
    /// 第1路TCP
    FFW_PROTOCOL_CONNECT_ID1 = 1,

    /// 第2路TCP
    FFW_PROTOCOL_CONNECT_ID2,

    /// 第3路TCP
    FFW_PROTOCOL_CONNECT_ID3,
    /// 第4路TCP
    FFW_PROTOCOL_CONNECT_ID4,

    /// 第5路TCP
    FFW_PROTOCOL_CONNECT_ID5,

    /// 第6路TCP
    FFW_PROTOCOL_CONNECT_ID6, //6

    /// FTP
    FFW_PROTOCOL_FTP, //7

    /// HTTP
    FFW_PROTOCOL_HTTP, //8

    /// 第1路MQTT
    FFW_PROTOCOL_MQTT1, //9

    /// 第2路MQTT
    FFW_PROTOCOL_MQTT2, //10

    /// LBS
    FFW_PROTOCOL_GIGIS, //11

    /// NTP
    FFW_PROTOCOL_NTP, //12

    /// dns
    FFW_PROTOCOL_DNS, //13

    /// PING
    FFW_PROTOCOL_PING, //14

    /// WEBSOCKET

    /// end
    FFW_PROTOCOL_MAX
} ffw_protocol_t;

/// 通道信息定义
typedef struct
{
    /// sim id
    int simid;

    /// cid
    int cid;
} channel_info_t;

/**
* @brief 获取当前使用的sim卡ID
* 
* @return uint8_t SIM ID
 */
uint8_t ffw_get_dual_sim();


/**
* @brief 获取当前数据卡ID
* 
* @return uint8_t SIM ID
 */
uint8_t ffw_get_data_sim();

/**
* @brief 设置当前使用的sim卡ID
* 
* @return uint8_t SIM ID
 */
int ffw_set_dual_sim(uint8_t simid);


/**
* @brief 设置当前数据卡ID, 此时会去激活当前卡所有激活的PDP, @c cb 回调函数表示所有pdp已经全部去激活
* 
* @return uint8_t SIM ID
 */
int ffw_set_data_sim(uint8_t simid, void (*cb)(int result, void *arg), void *arg);


/**
 * @brief 
 * 
 * @param channel 
 * @param simid 
 * @param cid 
 * @return int 
 */
int ffw_channel_set(ffw_protocol_t channel, int simid, int cid);

/**
 * @brief 
 * 
 * @param channel 
 * @param simid 
 * @param cid 
 * @return int 
 */
int ffw_channel_get(ffw_protocol_t channel, int *simid, int *cid);

/**
* @brief 获取当前数据卡ID
*
* @return uint8_t SIM ID
 */
int ffw_set_data_simid(uint8_t simid);


#endif /* C562F499_36F5_4C43_9A39_B97922849C6F */
