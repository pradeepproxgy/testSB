#ifndef B0D73B55_19E9_4C40_B792_AB8029579855
#define B0D73B55_19E9_4C40_B792_AB8029579855

/**
* @file ffw_ntp.h
* @author sundaqing (sundaqing@fibocom.com)
* @brief 使用NTP对时
* @version 0.1
* @date 2021-05-06
* 
* Copyright (c) 2021 Fibocom. All rights reserved
* 
 */

#include <stdint.h>

#include "ffw_os.h"
#include "ffw_utils.h"

/**
* @brief update local time by ntp protocol
* 
* @param ntp_host NTP server hostname, it will use ntp.ntsc.ac.cn domian if @c ntp_host is NULL
* @param port NTP server port, it will use 123 if @c port is 0
* @param timeout 超时时间，单位毫秒，如果为0，表示没有超时时间，永远等待下去
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_ntp_update(const char *ntp_host, uint16_t port, int timeout);

/**
 * @brief update local time by ntp protocol async mode
 * 
 * @param ntp_host NTP server hostname, it will use ntp.ntsc.ac.cn domian if @c ntp_host is NULL
 * @param port NTP server port, it will use 123 if @c port is 0
 * @param family AF_INET OR AF_INET6
 * @param timeout 超时时间，单位毫秒，如果为0，表示没有超时时间，永远等待下去
 * @param cb 回调函数
 * @param arg 回调上下文
 * @return int 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 */
int ffw_ntp_update_async(const char *ntp_host, uint16_t port, int family, int timeout, void (*cb)(int result, struct ffw_timeval *tval, void *arg), void *arg);

/**
 * @brief update local time by ntp protocol async mode, can specified netif
 * 
 * @param ntp_host NTP server hostname, it will use ntp.ntsc.ac.cn domian if @c ntp_host is NULL
 * @param port NTP server port, it will use 123 if @c port is 0
 * @param family AF_INET OR AF_INET6
 * @param timeout 超时时间，单位毫秒，如果为0，表示没有超时时间，永远等待下去
 * @param settime 是否设置本地时间
 * @param netif 网卡句柄
 * @param cb 回调函数
 * @param arg 回调上下文
 * @return int 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 */
int ffw_ntp_update_async_netif(const char *ntp_host, uint16_t port, int family, int timeout, bool settime, ffw_handle_t netif, void (*cb)(int result, struct ffw_timeval *tval, void *arg), void *arg);

#endif /* B0D73B55_19E9_4C40_B792_AB8029579855 */
