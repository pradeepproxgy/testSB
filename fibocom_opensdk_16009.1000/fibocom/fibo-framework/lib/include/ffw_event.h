/**
 * @file ffw_event.h
 * @author sundaqing (sundaqing@fibocom.com)
 * @brief 全局事件定义
 * @version 0.1
 * @date 2021-10-08
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef C427EE94_5798_4144_AF3E_0D1ECB0ADF20
#define C427EE94_5798_4144_AF3E_0D1ECB0ADF20

#include <stdint.h>
#include <ffw_utils.h>

/**
 * @brief 全局事件定义
 * 
 */
typedef enum ffw_event_s
{
    /// pdp cp 激活事件, 内部移植时使用
    FFW_EVT_CP_PDP_ACTIVED = 0,

    /// pdp cp 去激活事件，内部移植时使用
    FFW_EVT_CP_PDP_DEACTIVED,

    /// pdp cp 收到sms，内置事件，其他应用模块无法使用
    FFW_EVT_CP_SMS_IN,

    /// 网络驻网变化, 如果系统网络有变化，需要发送这个事件，ffw内部使用，其他用户接口不得使用
    FFW_EVT_CP_GPRS_NETWORK_CHANGE,

    /// 驻网事件 ffw内部使用，其他用户接口不得使用
    FFW_EVT_CP_GPRS_ATTACHED,

    /// 去驻网事件  ffw内部使用，其他用户接口不得使用
    FFW_EVT_CP_GPRS_DEATTACHED,

    /// pdp 激活事件
    FFW_EVT_PDP_ACTIVED = 100,

    /// 第一个pdp激活事件
    FFW_EVT_PDP_FIRST_ACTIVED,

    /// pdp 去激活事件
    FFW_EVT_PDP_DEACTIVED,

    /// pdp 被动去激活事件
    FFW_EVT_PDP_ABORT,

    /// 最后一个pdp去激活
    FFW_EVT_PDP_LAST_DEACTIVED,

    /// pdp 掉网事件
    FFW_EVT_PDP_RELEASE,

    /// 收到短信事件
    FFW_EVT_SMS_IN,

    /// GTSET有改变
    FFW_EVT_GTSET_CHANGED,

    /// 网络驻网变化
    FFW_EVT_GPRS_NETWORK_CHANGE,

    /// TCP在使用事件
    FFW_EVT_TCP_CONNECTTED,

    /// TCP未使用事件
    FFW_EVT_TCP_DISCONNECTTED,

    //ATTACH事件
    FFW_EVT_REATTACHED,

    //PSM RESTORE ACT
    FFW_EVT_CP_PDP_PSM_ACTIVED,

    //RRC RELEASE
    FFW_EVT_RRC_RELEASE,

    //RRC CONNECTTED
    FFW_EVT_RRC_CONNECTTED,

    //Wakeupin event
    FFW_EVT_WAKEUP_MODEM,

    //sleep in event
    FFW_EVT_SLEEP_MODEM,

    /// MAX EVENT
    FFW_EVT_MAX
} ffw_event_t;

/// PDP激活/去激活相关(FFW_EVT_PDP_ACTIVED/FFW_EVT_PDP_DEACTIVED)事件数据
typedef struct ffw_evt_pdp_s
{
    /// sim id
    uint8_t simid;

    /// cid
    uint8_t cid;

    /// ip 类型
    uint8_t iptype; /// 1: ip 2: ipv6 3: ipv4v6

    /// ipv4 address
    char ipv4[FFW_INET_ADDRSTRLEN];

    /// ipv6 address
    char ipv6[FFW_INET6_ADDRSTRLEN];
} ffw_evt_pdp_t;

typedef struct ffw_cp_evt_pdp_s
{
    /// 结果
    int result;

    /// sim id
    uint8_t simid;

    /// cid
    uint8_t cid;
} ffw_cp_evt_pdp_t;

/// sms事件数据 FFW_EVT_SMS_IN
typedef struct ffw_evt_sms_s
{
    /// sim id
    uint8_t simid;

    /// 短信ID
    uint32_t index;
} ffw_evt_sms_t;

/// GTSET变更事件数据 FFW_EVT_GTSET_CHANGED
typedef struct ffw_evt_gtset_s
{
    /// name
    const char *name;

    /// 数据
    int32_t *data;

    /// 数据长度
    uint32_t len;
} ffw_evt_gtset_t;


/// GPRS/LTE 网络变化事件
typedef struct ffw_evt_gprs_s
{
    /// sim id
    uint8_t simid;
    
    /// 是否已经驻网
    bool attached;
} ffw_evt_gprs_t;

#endif /* C427EE94_5798_4144_AF3E_0D1ECB0ADF20 */
