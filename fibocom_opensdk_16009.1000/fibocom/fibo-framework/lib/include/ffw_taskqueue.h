#ifndef LIB_INCLUDE_FFW_TASKQUEUE
#define LIB_INCLUDE_FFW_TASKQUEUE

/**
* @file ffw_taskqueue.h
* @author sundaqing (sundaqing@fibocom.com)
* @brief 任务队列实现
* @version 0.1
* @date 2021-05-29
* 
* Copyright (c) 2021 Fibocom. All rights reserved
* 
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include "ffw_list.h"

/**
* @brief 任务回调函数类型
* 
* @param arg 回调数据
 */
typedef void (*ffw_task_callback_t)(void *arg);

/// type of ffw_task_s
typedef struct ffw_task_s ffw_task_t;

/// type of ffw_taskqueue_s
typedef struct ffw_taskqueue_s ffw_taskqueue_t;

/**
* @brief 任务队列定义
* 
 */
struct ffw_taskqueue_s
{
    /// 队列
    void *q;

    /// 是否初始化
    bool init;
};

/**
* @brief 初始化一个全局的任务队列
* 
* @param thread_num 线程数量
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_taskqueue_init(int thread_num);

/**
* @brief 创建一个任务队列
* 
* @param[out] tq 任务队列 
* @param thread_num 线程数量
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_taskqueue_create(ffw_taskqueue_t **tq, int thread_num);

/**
* @brief 创建一个任务
* 
* @param cb 任务函数
* @param arg 任务数据
* @param task_out 创建任务
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_taskqueue_create_task(ffw_task_callback_t cb, void *arg, void **task_out);

/**
* @brief 加入任务到任务列表
* 
* @param tq 任务队列句柄，可以NULL, 如果为NULL, 表示使用全局的任务队列，之前需要调用 @c ::ffw_taskqueue_init 函数初始化
* @param task 任务
* @param priority 优先级，目前预留
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_taskqueue_add_task(ffw_taskqueue_t *tq, void *task, int priority);

/**
* @brief 增加一个任务
* 
* @param tq 任务队列句柄，可以NULL, 如果为NULL, 表示使用全局的任务队列，之前需要调用 @c ::ffw_taskqueue_init 函数初始化
* @param cb 任务函数
* @param arg 任务数据
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_taskqueue_add_task1(ffw_taskqueue_t *tq, ffw_task_callback_t cb, void *arg);

/**
* @brief 释放一个任务
* 
* @param task 任务句柄
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_taskqueue_delete_task(void *task);

#endif /* LIB_INCLUDE_FFW_TASKQUEUE */
