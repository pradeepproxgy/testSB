#ifndef LIB_INCLUDE_FFW_RBUFFER
#define LIB_INCLUDE_FFW_RBUFFER

/**
* @file ffw_rbuffer.h
* @author sundaqing (sundaqing@fibocom.com)
* @brief 环形缓冲区实现
* @version 0.1
* @date 2021-05-28
* 
* Copyright (c) 2021 Fibocom. All rights reserved
* 
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "ffw_os.h"

/// type of ffw_rbuffer_s
typedef struct ffw_rbuffer_s ffw_rbuffer_t;

/**
* @brief 环形缓冲区定义
* 
 */
struct ffw_rbuffer_s
{
    /// 是否初始化
    bool init;

    /// 缓冲区大小
    int size;

    /// 数据大小
    int data_size;

    /// 读缓冲区游标
    int rd_idx;

    /// 写缓冲区游标
    int wr_idx;

    /// 缓冲区
    uint8_t *buf;

    /// 互斥锁
    void *lock;

    /// 压入数据信号量
    void *sem_push;

    /// 弹出数据信号量
    void *sem_pop;
};

/**
 * @brief 初始化一个环形队列
 * 
 * @param rbuf 句柄
 * @param buff_size 队列大小，单位为字节 
 * @return int
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 */
int ffw_rbuffer_init(ffw_rbuffer_t *rbuf, int buff_size);

/**
 * @brief 重置缓冲区的数据，连续连续存放
 * 
 * @param rbuf 
 * @return int 
 */
int ffw_rbuffer_reset(ffw_rbuffer_t *rbuf);

/**
 * @brief 释放环形队列
 * 
 * @param rbuf 句柄
 * @return int 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 */
int ffw_rbuffer_free(ffw_rbuffer_t *rbuf);

/**
* @brief 队列中的数据大小
* 
* @param rbuf 句柄
* @return int 数据大小
 */
int ffw_rbuffer_data_size(ffw_rbuffer_t *rbuf);

/**
* @brief 队列中的空闲空间大小
* 
* @param rbuf 句柄
* @return int 空间大小
 */
int ffw_rbuffer_space(ffw_rbuffer_t *rbuf);

/**
* @brief 队列的总空间大小
* 
* @param rbuf 句柄
* @return int 总空间大小
 */
int ffw_rbuffer_buffer_size(ffw_rbuffer_t *rbuf);

/**
 * @brief 清空队列
 * 
 * @param rbuf 句柄
 * @return int  
 * @retval FFW_R_SUCCESS: 成功
 */
int ffw_rbuffer_clear(ffw_rbuffer_t *rbuf);

/**
 * @brief 队列是否已满
 * 
 * @param rbuf 句柄
 * @return int
 * @retval 0 未满
 * @retval 1 已满
 */
int ffw_rbuffer_full(ffw_rbuffer_t *rbuf);

/**
 * @brief 队列是否为空
 * 
 * @param rbuf 句柄
 * @return int 
 * @retval 0 不为空
 * @retval 1 为空
 */
int ffw_rbuffer_empty(ffw_rbuffer_t *rbuf);

/**
 * @brief 写入数据，不保证所有数据都写入
 * 
 * @param rbuf 句柄
 * @param wr_data 数据
 * @param size 数据大小
 * @param[out] write_size 写入数据大小
 * @return int 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 */
int ffw_rbuffer_write(ffw_rbuffer_t *rbuf, const void *wr_data, int size, int *write_size);

/**
 * @brief 写入所有数据，可以指定超时时间
 * 
 * @param rbuf 句柄
 * @param wr_data 数据
 * @param size 数据大小
 * @param timeout 超时时间，单位为毫秒，如果为0，则表不超时，直到所有数据写入完成才返回
 * @return int 
 * @retval FFW_R_SUCCESS: 成功，此时 @c size == *write_size
 * @retval FFW_R_FAILED: 失败
 */
int ffw_rbuffer_write_all(ffw_rbuffer_t *rbuf, const void *wr_data, int size, int timeout);

/**
 * @brief 异步读取数据
 * 
 * @param rbuf 句柄
 * @param size 需要读取的最大数据，如果队列数据大小大于 @c size,那么最多读取 @c size 大小的数据
 * @param read_cb 回调函数
 * @param param 回调上下文
 * @return int 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 */
int ffw_rbuffer_read(ffw_rbuffer_t *rbuf, int size, int (*read_cb)(void *data, int size, void *param), void *param);

/**
 * @brief 异步读取数据，如果没有数据，等待超时
 * 
 * @param rbuf 句柄
 * @param size 需要读取的最大数据，如果队列数据大小大于 @c size,那么最多读取 @c size 大小的数据
 * @param timeout 超时时间，单位毫秒，如果为0，表示不超时，如果小于0，表示永远等待
 * @param read_cb 回调函数
 * @param param 回调上下文
 * @return int 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 * @retval FFW_R_TIMEOUT: 超时
 */
int ffw_rbuffer_read_timeout(ffw_rbuffer_t *rbuf, int size, int timeout, int (*read_cb)(void *data, int size, void *param), void *param);

/**
* @brief 同步读取数据,如果没有数据，直接返回错误
* 
* @param rbuf 句柄
* @param buf 存储数据缓冲区
* @param size @c buf 的大小
* @param[out] len 读取数据的大小
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_rbuffer_read1(ffw_rbuffer_t *rbuf, uint8_t *buf, int size, uint32_t *len);

/**
* @brief 同步读取指定长度的数据，直到超时
* 
* @param rbuf 句柄
* @param buf 数据缓冲区
* @param size 读取数据的大小
* @param timeout 超时时间，单位毫秒，如果小于等于0，表示永远等待
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
* @retval FFW_R_TIMEOUT: 超时
 */
int ffw_rbuffer_read_fixed_size(ffw_rbuffer_t *rbuf, uint8_t *buf, int size, int timeout);

/**
 * @brief 同步读取数据，如果没有数据，等待超时超时
 * 
 * @param rbuf 句柄
 * @param buf 数据缓冲区
 * @param size 数据缓冲区大小 
 * @param len 读取数据的大小
 * @param timeout 超时时间，单位毫秒，如果为0，表示不超时，如果小于0，表示永远等待
 * @return int 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 * @retval FFW_R_TIMEOUT: 超时
 */
int ffw_rbuffer_read1_timeout(ffw_rbuffer_t *rbuf, uint8_t *buf, int size, uint32_t *len, int timeout);

/**
* @brief 异步读取所有数据，回调函数返回的是处理数据的长度，如果返回值为0，则不会消费数据，并停止上报数据 
* 
* @param rbuf 句柄
* @param read_cb 回调函数
* @param param 上下文
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_rbuffer_read_all(ffw_rbuffer_t *rbuf, int (*read_cb)(void *data, int size, void *param), void *param);

/**
 * @brief 预先获取指定大小的空间，需要和 @c ::ffw_rbuffer_occupy_space 配合使用，当数据已经拷贝完毕，使用这个函数占有空间
 * 
 * @param rbuf 句柄
 * @param space 空间起始地址
 * @param size 空间大小
 * @return int 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 */
int ffw_rbuffer_get_space(ffw_rbuffer_t *rbuf, void **space, int *size);

/**
 * @brief 占有空间
 * 
 * @param rbuf 句柄
 * @param size 占有空间的大小
 * @return int 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 */
int ffw_rbuffer_occupy_space(ffw_rbuffer_t *rbuf, int size);

#endif /* LIB_INCLUDE_FFW_RBUFFER */
