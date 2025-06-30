#ifndef LIB_INCLUDE_FFW_BROKER
#define LIB_INCLUDE_FFW_BROKER

/**
 * @file ffw_broker.h
 * @author sundaqing (sundaqing@fibocom.com)
 * @brief 简易消息代理
 * @version 0.1
 * @date 2021-05-30
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "ffw_list.h"

/**
 * @brief 消息处理回调函数类型定义
 * 
 * @param msg_id 消息ID
 * @param data 消息数据
 */
typedef void (*observer_on_recv_t)(uint32_t msg_id, void *data);

/**
* @brief 初始化消息代理
* 
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_broker_init();

/**
* @brief 发送消息到消息代理
* 
* @param msg_id 消息ID
* @param data 消息数据
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_broker_send_msg(uint32_t msg_id, void *data);

/**
 * @brief 发送消息到消息代理, 在AT任务里执行消息代理函数
 * 
 * @param msg_id msg_id 消息ID
 * @param data 消息数据
 * @param size @c data 的大小
 * @return int 
 */
int ffw_broker_send_msg_at_task(uint32_t msg_id, const void *data, uint32_t size);

/**
 * @brief 添加消息观察者到消息代理
 * 
 * @param name 观察者名称，这个名字必须是唯一的，不能重复
 * @param msg_id 观察消息id
 * @param on_recv 回调函数
 * @return int 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 */
int ffw_broker_add_observer(const char *name, uint32_t msg_id, observer_on_recv_t on_recv);

/**
* @brief 删除消息观察者
* 
* @param name 观察者名字
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_broker_del_observer(const char *name);

#endif /* LIB_INCLUDE_FFW_BROKER */
