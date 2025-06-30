#ifndef LIB_INCLUDE_FFW_TIMER
#define LIB_INCLUDE_FFW_TIMER

/**
* @file ffw_timer.h
* @author sundaqing (sundaqing@fibocom.com)
* @brief 计时器实现
* @version 0.1
* @date 2021-05-29
* 
* Copyright (c) 2021 Fibocom. All rights reserved
* 
 */

#include <stdbool.h>
#include <stdint.h>

/**
* @brief 计时器回调函数类型
* 
* @param arg 回调函数上下文 
 */
typedef void (*ffw_timer_callback_t)(void *arg);

/// 计时器类型定义
typedef void *ffw_timerp_t;

/**
 * @brief 初始化计时器系统
 * 
 * @return int 
 */
int ffw_timer_init();

/**
 * @brief 创建一个计时器,callback会在AT任务里运行
 * 
 * @param cb 计时器超时回调函数
 * @param arg 回调上下文
 * @param[out] timer_out 计时器实例
 * @return int 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 */
int ffw_timer_create(ffw_timer_callback_t cb, void *arg, ffw_timerp_t *timer_out);


/**
 * @brief 创建一个计时器,callback在中断里运行
 * 
 * @param cb 计时器超时回调函数
 * @param arg 回调上下文
 * @param[out] timer_out 计时器实例
 * @return int 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 */
int ffw_timer_isr_create(ffw_timer_callback_t cb, void *arg, ffw_timerp_t *timer_out);

/**
* @brief 启动计时器
* 
* @param timer 计时器实例
* @param timeout 超时时间，单位ms
* @param periodic 是否周期计时器
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_timer_start(ffw_timerp_t timer, int timeout, bool periodic);

/**
* @brief 启动计时器
* 
* @param timer 计时器实例
* @param timeout 超时时间，单位微妙
* @param periodic 是否周期计时器
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_timer_microsecond_start(ffw_timerp_t timer, int timeout, bool periodic);

/**
 * @brief 停止计时器
 * 
 * @param timer 计时器实例
 * @return int 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 */
int ffw_timer_stop(ffw_timerp_t timer);

/**
 * @brief 销毁计时器
 * 
 * @param timer 计时器实例
 * @return int 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 */
int ffw_timer_delete(ffw_timerp_t timer);


int ffw_timer_is_running(ffw_timerp_t timer);


int32_t ffw_timer_remaining(ffw_timerp_t timer);

#endif /* LIB_INCLUDE_FFW_TIMER */
