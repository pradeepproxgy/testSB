#ifndef LIB_INCLUDE_FFW_TRANSPORT_POLL
#define LIB_INCLUDE_FFW_TRANSPORT_POLL

/**
* @file ffw_transport_poll.h
* @author sundaqing (sundaqing@fibocom.com)
* @brief 这是一个transport的调度系统，用户可以将创建好的 transport 使用 @c ::ffw_transport_poll_add
*        加入这个调度系统，可以用来监控 ::TRANSPORT_EV_R (读事件)  ::TRANSPORT_EV_W (写事件)  ::TRANSPORT_EV_TMOUT (超时事件), 达到 select
*        epoll的功能
* @version 0.1
* @date 2021-05-29
* 
* Copyright (c) 2021 Fibocom. All rights reserved
* 
 */

#include "ffw_list.h"
#include "ffw_os.h"
#include "ffw_transport_interface.h"

#ifndef FFW_POOL_MAX_NUMBER
/// 调度系统可以添加的最大的transport个数
#define FFW_POOL_MAX_NUMBER (50)
#endif

/// type of ffw_transport_poll_s
typedef struct ffw_transport_poll_s ffw_transport_poll_t;

/**
* @brief 监控事件类型定义
* 
 */
typedef enum tranport_ev_type_s
{
    /// 读事件
    TRANSPORT_EV_R = 0x1,

    /// 写事件
    TRANSPORT_EV_W = 0x2,
    // TRANSPORT_EV_C = 0x4,
    // TRANSPORT_EV_E = 0x8,

    /// 超时事件
    TRANSPORT_EV_TMOUT = 0x10,
} transport_ev_type_t;

/**
* @brief 事件上报函数类型定义
*
* @param t 产生事件的transport
* @param ev_mask: 事件类型，是各个事件的或(|)起来的值
* @param arg 回调上下文 
 */
typedef void (*tranport_poll_callback_t)(ffw_transport_t *t, int ev_mask, void *arg);

/**
* @brief 调度系统定义
* 
 */
struct ffw_transport_poll_s
{
    /// transport 数组
    ffw_transport_t *transport_list[FFW_POOL_MAX_NUMBER];
    int evt_mask[FFW_POOL_MAX_NUMBER];


    /// transport 的个数
    int num;

    /*! @cond PRIVATE */
    bool init;
    bool finish;
    void *lock;
    void *sem;
    bool pending;

    ffw_transport_t cmd_trans_client;
    ffw_transport_t cmd_trans_server;

    void *select_tid;
    ffw_fd_set readfds;
    ffw_fd_set writefds;
    ffw_handle_t timer;
    struct list_head tasklist;
    /********私有数据，可不用关注*****************************/
    /*! @endcond */
};

/**
 * @brief 初始化调度系统
 * 
 * @param tp 调度系统实例，如果为NULL, 表示使用全局调度系统实例
 * @return int 
 */
int ffw_transport_poll_init(ffw_transport_poll_t *tp);

/**
* @brief 刷新一次调度系统，此时调度任务会空跑一次
* 
* @param tp 调度系统实例，如果为NULL, 表示使用全局调度系统实例
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_transport_poll_refresh(ffw_transport_poll_t *tp);

/**
* @brief 将一个transport实例加入到调度系，可以指定超时时间
* 
 * @param tp 调度系统实例，如果为NULL, 表示使用全局调度系统实例
 * @param t transport实例
 * @param ev_type 需要监控的事件，是各个事件或起来的值
 * @param timeout 如果要监控超时事件，这里指定超时的时间长度，单位为毫秒
 * @param cb 事件回调函数
 * @param arg 回调上下文
 * @return int 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 */
int ffw_transport_poll_add(ffw_transport_poll_t *tp, ffw_transport_t *t, int ev_type, int32_t timeout, tranport_poll_callback_t cb, void *arg);

/**
* @brief 更新一个transport需要监控的事件
* 
* @param tp 调度系统实例，如果为NULL, 表示使用全局调度系统实例
* @param t transport实例
* @param ev_type 需要监控的事件，是各个事件或起来的值
* @param timeout 如果要监控超时事件，这里指定超时的时间长度，单位为毫秒
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_transport_poll_update(ffw_transport_poll_t *tp, ffw_transport_t *t, int ev_type, int32_t timeout);

/**
* @brief 在AT任务执行一个函数
* 
* @param tp 调度系统实例，如果为NULL, 表示使用全局调度系统实例
* @param cb 函数
* @param arg 上下文
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_transport_poll_run_attask(ffw_transport_poll_t *tp, void (*cb)(void *arg), void *arg);

/**
* @brief 自动上报一次指定的事件
* 
* @param tp 
* @param t 
* @param ev_type 
* @return int 
 */
int ffw_transport_poll_notify_evt(ffw_transport_poll_t *tp, ffw_transport_t *t, int ev_type);

/**
* @brief 更新一个transport需要监控的事件,分为增加事件和删除事件监控
* 
* @param tp 调度系统实例，如果为NULL, 表示使用全局调度系统实例
* @param t transport实例
* @param add_event 增加的监控的事件
* @param del_event 增加的监控的事件
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_transport_poll_update_event(ffw_transport_poll_t *tp, ffw_transport_t *t, int add_event, int del_event);

/**
* @brief 
* 
* @param tp 
* @param t 
* @param add_event 
* @param del_event 
* @param timeout 
* @return int 
 */
int ffw_transport_poll_update_event1(ffw_transport_poll_t *tp, ffw_transport_t *t, int add_event, int del_event, int32_t timeout);

/**
* @brief 从调度系统中删除一个transport
* 
* @param tp 调度系统实例，如果为NULL, 表示使用全局调度系统实例
* @param t transport实例
* @return int
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_transport_poll_del(ffw_transport_poll_t *tp, ffw_transport_t *t);

/**
 * @brief 销毁一个调度系统
 * 
 * @param tp 调度系统实例，如果为NULL, 表示使用全局调度系统实例
 * @return int 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败 
 */
int ffw_transport_poll_destroy(ffw_transport_poll_t *tp);

#endif /* LIB_INCLUDE_FFW_TRANSPORT_POLL */
