/**
 * @file ffw_sms.h
 * @author your name (you@domain.com)
 * @brief 短信相关接口
 * @version 0.1
 * @date 2021-10-09
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef B506E9FD_B7E8_45AE_9F92_397D2BCAB66C
#define B506E9FD_B7E8_45AE_9F92_397D2BCAB66C

#include <stdint.h>
#include <ffw_sms_plat.h>

/// sms format define
typedef enum ffw_sms_format_s
{
    FFW_SMS_F_PDU,
    FFW_SMS_F_TEXT
} ffw_sms_format_t;

/// 短信的编码方式定义
typedef enum ffw_sms_cs_s
{
    /// default cs (gsm)
    FFW_SMS_CS_GSM = 0,

    /// 8BIT
    FFW_SMS_CS_8BIT = 1,

    /// UCS2
    FFW_SMS_CS_UCS2 = 2,

} ffw_sms_cs_t;

/// 平台编码方式定义
typedef enum
{
    /// gsm
    FFW_CS_CS_GSM,

    /// gsm
    FFW_CS_CS_HEX,

    /// ucs2(UTF16)
    FFW_CS_CS_UCS2,

    /// gbk
    FFW_CS_CS_GBK, // equals to PCCP 936 char set

    /// ira
    FFW_CS_CS_IRA,
} ffw_sms_cscs_t;

/**
 * @brief 发送短信
 * 
 * @param simid 卡ID
 * @param to_number 目的号码
 * @param number_type 
 * @param format 格式
 * @param cscs 平台编码方式
 * @param cs 短信编码方式
 * @param data 内容
 * @param len 内容长度
 * @param cb 回调函数
 * @param arg 回调上下文
 * @return int 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败 
 */
int ffw_sms_send(uint8_t simid, const char *to_number, ffw_phone_number_type_t number_type, ffw_sms_format_t format, ffw_sms_cscs_t cscs, ffw_sms_cs_t cs, void *data, uint32_t len,
                    void (*cb)(int result, void *arg), void *arg);

/**
 * @brief 接收短信
 * 
 * @param simid 
 * @param cscs 
 * @param sms_type 
 * @param idx 
 * @param sms_data 
 * @return int 
 */
int ffw_sms_recv(uint8_t simid, ffw_sms_cscs_t cscs, int sms_type, uint32_t idx, void (*cb)(int result, ffw_sms_data_t *sms_data, void *arg), void *arg);

/**
 * @brief 打印短信內容
 * 
 * @param simid 
 * @param idx 
 * @return int 
 */
int ffw_sms_print(uint8_t simid, uint32_t idx);

#endif /* B506E9FD_B7E8_45AE_9F92_397D2BCAB66C */
