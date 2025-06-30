#ifndef F5A2BF5C_2C83_4F71_B62E_453E0B786C4D
#define F5A2BF5C_2C83_4F71_B62E_453E0B786C4D

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define FFW_TEL_NUMBER_MAX_LEN (21 + 1)

/// phone number type define
typedef enum ffw_phone_number_type_s
{
    /// unknown type
    FFW_TELNUMBER_TYPE_UNKNOWN = 129,

    /// international
    FFW_TELNUMBER_TYPE_INTERNATIONAL = 145,

    /// natiional
    FFW_TELNUMBER_TYPE_NATIONAL = 161,
} ffw_phone_number_type_t;

/// phone number define
typedef struct ffw_phone_number_s
{
    /// bcd number
    uint8_t number[FFW_TEL_NUMBER_MAX_LEN];

    /// size of number
    uint32_t number_size;

    /// type
    ffw_phone_number_type_t type;

    /// reserverd
    uint8_t clir;

    /// reserverd
    uint8_t num_type;
} ffw_phone_number_t;

/// 短信状态定义
typedef enum ffw_sms_stat_s
{
    /// 未读
    FFW_SMS_STATUS_UNREAD = 0x01,

    /// 已读
    FFW_SMS_STATUS_READ = 0x02,

    /// 未发送
    FFW_SMS_STATUS_UNSENT = 0x04,

    /// 已发送
    FFW_SMS_STATUS_SENT = 0x08,

    /// all
    FFW_SMS_STATUS_ALL = 0x80
} ffw_sms_stat_t;

/// 短信时间戳定义
typedef struct ffw_sms_timestamp_s
{
    /// year
    uint16_t year;

    /// mon
    uint8_t month;

    /// day
    uint8_t day;

    /// hour
    uint8_t hour;

    /// min
    uint8_t min;

    /// second
    uint8_t second;

    /// zone
    int8_t zone;
} ffw_sms_timestamp_t;

/// 收到短信内容定义
typedef struct ffw_sms_recv_s
{
    /// 发送方短信号码
    uint8_t oa[FFW_TEL_NUMBER_MAX_LEN];

    /// 发送方号码大小
    uint8_t oa_size;

    /// 短信中心号码
    uint8_t sca[FFW_TEL_NUMBER_MAX_LEN];

    /// 短信中心号码大小
    uint8_t sca_size;

    /// 接收方短信号码
    uint8_t da[FFW_TEL_NUMBER_MAX_LEN];

    /// 接收方短信号码大小
    uint8_t da_size;

    /// 时间戳
    ffw_sms_timestamp_t scts;

    /// 发送方 @c  oa 短信号码的类型
    uint8_t tooa;

    /// 短信中心号码的类型
    uint8_t tosca;

    /// fo
    // uint8_t fo;

    /// 数据编码 (Data Coding Scheme)
    uint8_t dcs;

    /// 短信消息数据长度，也就是 @c data 的长度
    uint16_t length;

    /// 短信消息数据，这个指针指向的内存必须使用动态分配的内存，由调用者自负责释放
    uint8_t *data;
} ffw_sms_recv_t;

/// 发送短信内容定义
typedef struct ffw_sms_send_s
{
    /// 接收方短信号码
    uint8_t da[FFW_TEL_NUMBER_MAX_LEN];

    /// 接收方短信号码大小
    uint8_t da_size;

    /// 短信中心号码
    uint8_t sca[FFW_TEL_NUMBER_MAX_LEN];

    /// 短信中心号码大小
    uint8_t sca_size;

    /// 接收方 @c  da 短信号码的类型
    uint8_t toda;

    /// 短信中心号码的类型
    uint8_t tosca;

    /// fo 参考CSMP命令
    uint8_t fo;

    /// 协议ID (Protocol Identifier)
    uint8_t pid;

    /// 数据编码 (Data Coding Scheme)
    uint8_t dcs;

    /// validity period 
    uint8_t vp;

    /// 短信消息数据长度，也就是 @c data 的长度
    uint16_t length;

    /// 短信消息数据，这个指针指向的内存必须使用动态分配的内存，由调用者负责释放
    uint8_t *data;
} ffw_sms_send_t;

typedef struct ffw_sms_pdu_s
{
    /// 短信消息数据长度，也就是 @c data 的长度
    uint16_t length;

    /// 短信消息数据，这个指针指向的内存必须使用动态分配的内存，由调用者负责释放
    uint8_t *data;
} ffw_sms_pdu_t;

/// 短信元数据定义
typedef struct ffw_sms_node_s
{
    /// 存储类型ID
    uint16_t storage;

    /// 短信状态
    ffw_sms_stat_t status;

    /// prev
    uint16_t concat_prev_index;

    /// next
    uint16_t concat_next_index;

    /// current
    uint16_t concat_current_index;

    /// 消息类型
    uint8_t type;

    /// padding
    uint8_t padding;
} ffw_sms_node_t;

/// 短信内容定义
typedef union 
{
    /// 当type为1时
    ffw_sms_recv_t recv;

    /// 当type为3时
    ffw_sms_send_t send;

    /// 当type为7时
    ffw_sms_pdu_t pdu;
} ffw_sms_info_t;

/// 收到短信定义
typedef struct ffw_sms_data_s
{
    ffw_sms_node_t node;
    ffw_sms_info_t info;
} ffw_sms_data_t;

/**
 * @brief get number of sca for sms
 * 
 * @param simid 
 * @param number 
 * @return int 
 */
int ffw_sms_get_sca_number(uint8_t simid, const uint8_t **number);

/**
 * @brief 发送text sms消息
 * 
 * @param simid 
 * @param number 
 * @param data 
 * @param len 
 * @param cb 
 * @param arg 
 * @return int 
 */
int ffw_sms_send_text(uint8_t simid, ffw_phone_number_t *number, uint8_t *data, uint32_t len, void (*cb)(int result, void *arg), void *arg);

/**
 * @brief 发送pdu sms消息
 * 
 * @param simid 
 * @param data 
 * @param len 
 * @param cb 
 * @param arg 
 * @return int 
 */
int ffw_sms_send_pdu(uint8_t simid, uint8_t *data, uint32_t len, void (*cb)(int result, void *arg), void *arg);

/**
 * @brief 读取短信
 * 
 * @param simid SIM ID
 * @param sms_type 短信类型, 默认为1
 * @param idx 短信id
 * @param cb callback
 * @param arg 回調上下文
 * @return int 
 */
int ffw_sms_read(uint8_t simid, int sms_type, uint32_t idx, void (*cb)(int result, ffw_sms_data_t *sms_data, void *arg), void *arg);

#endif /* F5A2BF5C_2C83_4F71_B62E_453E0B786C4D */
