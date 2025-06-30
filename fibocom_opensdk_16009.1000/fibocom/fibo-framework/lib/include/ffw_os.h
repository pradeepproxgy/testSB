#ifndef LIB_INCLUDE_FFW_THREAD
#define LIB_INCLUDE_FFW_THREAD

/**
* @file ffw_os.h
* @author sundaqing (sundaqing@fibocom.com)
* @brief 操作系统相关接口，如线程、互斥锁、信号量和日志打印等接口
* @version 0.1
* @date 2021-05-21
* 
* Copyright (c) 2021 Fibocom. All rights reserved
* 
 */

#include <stdint.h>
#include <string.h>
#include <stdbool.h>

struct ffw_timeval
{
    uint64_t tv_sec;           /* seconds */
    unsigned long tv_usec; /* microseconds */
};

/// type of ffw_thread_attr_s
typedef struct ffw_thread_attr_s ffw_thread_attr_t;

/**
* @brief 任务函数类型
* 
 */
typedef void (*thread_callback_t)(void *data);

/// 句柄定义
typedef void *ffw_handlep_t;

/**
* @brief 线程属性定义
* 
 */
struct ffw_thread_attr_s
{
    /// 线程优先级
    int priority;

    /// 线程栈大小
    int stack_size;
};

/**
* @brief 线程睡眠
* 
* @param msec 单位：毫秒
 */
void ffw_thread_sleep(uint32_t msec);

/**
* @brief 创建线程
* 
* @param[out] th 线程句柄 
* @param name 线程名字
* @param attr 线程相关属性
* @param callback 线程函数
* @param data 线程数据
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_thread_create(ffw_handlep_t *th, const char *name, ffw_thread_attr_t *attr, thread_callback_t callback, void *data);


/**
* @brief 删除当前task
* 
 */
void ffw_thread_delete(void);

/**
* @brief 获取线程名字
* 
* @param th 线程句柄
* @param[out] name 存储名字的缓冲区 
* @param len @c name缓冲区的长度
* @return const char* NULL or @c name
 */
const char *ffw_thread_getname(ffw_handlep_t th, char *name, int len);

/**
* @brief 获取当前线程名字
* 
* @return const char* thread name
 */
const char *ffw_self_thread_name();

/**
* @brief detach a thread
* 
* @param th 线程句柄
* @return int
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_thread_detach(ffw_handlep_t th);

/**
* @brief 启动线程
* 
* @param th 线程句柄
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_thread_start(ffw_handlep_t th);

/**
* @brief 挂起线程
* 
* @param th 线程句柄
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_thread_suspend(ffw_handlep_t th);

/**
* @brief 创建互斥锁，一旦创建城建成功，可以使用 @c ffw_mutex_lock 或者 @c ffw_mutex_trylock 锁定当前锁，@c ffw_mutex_unlock 释放锁
* 
* @param[out] mutex 锁句柄 
* @param attr 锁属性
* @return int
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_mutex_create(ffw_handlep_t *mutex, void *attr);

/**
* @brief 锁定互斥锁，如果锁已经被锁定，则阻塞当前线程
* 
* @param mutex 锁句柄
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_mutex_lock(ffw_handlep_t mutex);

/**
* @brief 释放互斥锁
* 
* @param mutex 锁句柄
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_mutex_unlock(ffw_handlep_t mutex);

/**
* @brief 尝试锁定互斥锁
* 
* @param mutex 锁句柄
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败，表示锁已经被锁定
 */
int ffw_mutex_trylock(ffw_handlep_t mutex);


/**
* @brief 尝试锁定互斥锁
* 
* @param mutex 锁句柄
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败，表示锁已经被锁定
 */
int ffw_mutex_trylock1(ffw_handlep_t mutex, uint32_t timeout);

/**
* @brief 销毁互斥锁
* 
* @param mutex 锁句柄
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_mutex_delete(ffw_handlep_t mutex);

/**
* @brief 获取当前线程的句柄(ID)
* 
* @return void *
 */
void *ffw_thread_self();

/**
 * @brief 创建一个信号量，创建成功之后，可以使用 @c ffw_sem_wait 消费信号量，使用 @c ffw_sem_post 生产信号量
 * 
 * @param[out] sem 信号量句柄 
 * @param max_value 最大信号量个数
 * @param init_value 初始信号量个数
 * @return int 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 */
int ffw_sem_init(ffw_handlep_t *sem, uint32_t max_value, uint32_t init_value);

/**
 * @brief 生产一个信号量
 * 
 * @param sem 信号量句柄 
 * @return int
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 */
int ffw_sem_post(ffw_handlep_t sem);

/**
 * @brief 消耗一个信号量
 * 
 * @param sem 信号量句柄 
 * @param timeout 等待超时时间，单位毫秒，如果小于等于0，则表示永远等待
 * @return int 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 * @retval FFW_R_TIMEOUT: 超时
 */
int ffw_sem_wait(ffw_handlep_t sem, int timeout);

/**
 * @brief 销毁一个信号量
 * 
 * @param sem 信号量句柄 
 * @return int 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 */
int ffw_sem_deinit(ffw_handlep_t sem);

/**
* @brief 获取当前时间戳，单位为秒
* 
* @return int 时间戳
 */
int64_t ffw_timestamp();

/**
* @brief 设置系统时间
* 
* @param t 系统时间, 类型为struct timeval *
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_set_timestamp(const void *t);

/**
* @brief 获取当前时间戳，单位为毫秒
* 
* @return int64_t 时间戳
 */
int64_t ffw_time_ms();

/**
 * @brief 获取微妙级别的事件
 * 
 * @param tv 时间
 * @param tz 时区
 * @return int 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 */
int ffw_gettimeofday(struct ffw_timeval *tv, void *tz);

/**
* @brief 获取内存信息
*
* @param total 内存总的大小
* @param ava_size 可用内存的大小
* @param max_block_size 最大可用块大小
* @return int
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */

int ffw_get_memory_info1(uint32_t *total, uint32_t *ava_size, uint32_t *max_block_size);

/**
* @brief 获取内存信息
*
* @param total 内存总的大小
* @param ava_size 可用内存的大小
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_get_memory_info(uint32_t *total, uint32_t *ava_size);

/**
 * @brief 在AT线程执行某个函数
 * 
 * @param cb 函数
 * @param arg 数据
 * @param sync 是否同步
 */
void ffw_at_thread_callback(void (*cb)(void *arg), void *arg, bool sync);

bool ffw_at_thread_callback1(void (*cb)(void *arg), void *arg, bool sync);


/**
* @brief 生成随机数
* 
* @param[out] buf 存储随机串的缓冲区 
* @param size @c buf 的大小
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_rng_generate(void *buf, int size);

/**
 * @brief 分配内存
 * 
 * @param size 内存大小
 * @return void* 内存首地址
 */
void *ffw_malloc_plat(uint32_t size);

/**
 * @brief 分配内存
 * 
 * @param nmemb 块的个数
 * @param size 块的大小
 * @return void* 内存首地址
 */
void *ffw_calloc_plat(uint32_t nmemb, uint32_t size);

/**
 * @brief 释放内存
 * 
 * @param p 内存首地址
 */
void ffw_free_plat(void *p);

/**
 * @brief 
 * 
 * @param dest 
 * @param src 
 * @param n 
 */
void *ffw_memcpy(void *dest, const void *src, uint32_t n);

/**
 * @brief 
 * 
 * @param dest 
 * @param c 
 * @param n 
 */
void *ffw_memset(void *dest, int c, uint32_t n);

/**
 * @brief csdf格式设置
 * 
 * @return uint8_t
 * if 1 time:yy/MM/dd(默认值)
 * if 2 time:yyyy/MM/dd
 */
uint8_t ffw_get_csdf_auxmode();

/**
 * @brief 获取时区
 * 
 * @return int 时区 -96-96
 */
int ffw_get_timezone();
 /**
 * @brief 设置时区
 * 
 * @param timezone 时区
 */
int ffw_set_timezone(int timezone);


/**
* @brief 软重启
* 
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_sys_soft_reset(void);

/**
* @brief 关机
* 
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_sys_poweroff(void);

/**
* @brief 设置版本号
* 
* @return int 
 */
int ffw_set_soft_version(char *ver);

/**
 * @brief 判断fota升级结束是否要上报
 *
 * @return void
 */
int ffw_check_update_success();

/**
 * @brief 判断差分包是否匹配
 *
 * @return void
 */
bool ffw_check_fota_match(void);

int64_t ffw_get_sys_tick(void);
bool ffw_soft_reset(void);
bool ffw_soft_power_off(void);
void ffw_sys_panic();
bool ffw_sys_is_security_boot();
uint16_t ffw_get_poweron_cause(void);
int ffw_get_thread_stack_remaining_size(ffw_handlep_t th,uint32_t *size);
void ffw_feed_dog();
int ffw_sem_wait_v2(ffw_handlep_t sem, uint32_t timeout);


#endif /* LIB_INCLUDE_FFW_THREAD */
