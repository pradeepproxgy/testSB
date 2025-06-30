#ifndef IOT_HAL_H
#define IOT_HAL_H

#include "alipay_iot_type.h"

#if defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif

/**
 * 打开一个与SE的通信通道。
 * 参数：无
 * 返回值：打开通道的状态码：0 - 成功；其它值 - 失败
 **/
extern int32_t sechn_open_se_channel();

/**
 * 关闭当前与SE的通信通道。
 * 参数：无
 * 返回值：无
 **/
extern void sechn_close_se_channel();

/**
 * 重置当前SE的通信通道。
 * 参数：无
 * 返回值：重置通道的状态码：0 - 成功；其它值 - 失败
 **/ 
extern int32_t sechn_reset_se_channel();

/**
 * 将apduCmd中的数据发送到SE中，在apduResp中接收返回数据。本接口是同步调用，需要等待APDU命令执行完成之后返回数据。
 * 参数：
 * apduCmd - 发送到SE当中的APDU命令
 * apduLen - 发送数据的长度
 * apduResp - SE执行完成APDU命令之后返回的响应数据
 * 返回值：apduResp中响应数据的长度，执行失败时返回负值。
 **/
extern int32_t sechn_transmit_apdu(const uint8_t apduCmd[], uint32_t apduLen, uint8_t apduResp[]);


#if defined(__cplusplus) || defined(c_plusplus)
}
#endif


#endif //IOT_HAL_H
