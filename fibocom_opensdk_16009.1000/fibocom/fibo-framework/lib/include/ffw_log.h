#ifndef BF7A9477_5991_4A5C_9C75_F926B6A35AA5
#define BF7A9477_5991_4A5C_9C75_F926B6A35AA5
/**
 * @file ffw_log.h
 * @author sundaiqing (sundaqing@fibocom.com)
 * @brief 简易的日志组件
 * @version 0.1
 * @date 2021-05-29
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <stdbool.h>
#include <stdint.h>

#include "ffw_os.h"

/**
 * @brief 日志级别定义
 * 
 */
typedef enum ffw_log_level_s {
    /// FATAL
    FFW_LOG_LEVEL_FATAL = 0,

    /// ERROR
    FFW_LOG_LEVEL_ERROR,

    /// WARN
    FFW_LOG_LEVEL_WARN,

    /// INFO
    FFW_LOG_LEVEL_INFO,

    /// DEBUG
    FFW_LOG_LEVEL_DEBUG,

    /// TRACE
    FFW_LOG_LEVEL_TRACE,
} ffw_log_level_t;

#ifdef CONFIG_8850BM_C1
/// macro of base log
#define LOG_P(format, ...)                                                                                 \
    do                                                                                                     \
    {                                                                                                      \
        char datename[48];                                                                                 \
        ffw_get_date_string(datename, sizeof(datename));                                                \
        ffw_log("[][%s][%s:%d]-" format " \n", datename, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    } while (0);

/// redefine to ffw_log
#define FFW_LOG LOG_P

/// print log if condtion meet
#define FFW_LOG_INFO_IF(con, format, ...)                                                                                                                                                           \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        if (ffw_log_should_log(FFW_LOG_LEVEL_INFO) && (con))                                                                                                                                     \
        {                                                                                                                                                                                              \
           ffw_log("[][][fatal][][%s:%d]-" format " \n", __FUNCTION__, __LINE__, ##__VA_ARGS__);                                                   \
       }                                                                                                                                                                                              \
    } while (0);

/// log info
#define FFW_LOG_INFO(format, ...)                                                                                                                                                                   \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        if (ffw_log_should_log(FFW_LOG_LEVEL_INFO))                                                                                                                                              \
        {                                                                                                                                                                                              \
            ffw_log("[x][i][%s:%d]-" format " \n", __FUNCTION__, __LINE__, ##__VA_ARGS__);                                                    \
        }                                                                                                                                                                                              \
    } while (0);

/// log fatal
#define FFW_LOG_FATAL(format, ...)                                                                                                                                                                  \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        if (ffw_log_should_log(FFW_LOG_LEVEL_FATAL))                                                                                                                                             \
        {                                                                                                                                                                                              \
           ffw_log("[%s:%d]-" format " \n", __FUNCTION__, __LINE__, ##__VA_ARGS__);                                                   \
        }                                                                                                                                                                                              \
    } while (0);

/// log error
#define FFW_LOG_ERROR(format, ...)                                                                                                                                                                  \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        if (ffw_log_should_log(FFW_LOG_LEVEL_ERROR))                                                                                                                                             \
        {                                                                                                                                                                                              \
           ffw_log("[%s:%d]-" format " \n", __FUNCTION__, __LINE__, ##__VA_ARGS__);                                                   \
        }                                                                                                                                                                                              \
    } while (0);

/// log debug
#define FFW_LOG_DEBUG(format, ...)                                                                                                                                                                  \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        if (ffw_log_should_log(FFW_LOG_LEVEL_DEBUG))                                                                                                                                             \
        {                                                                                                                                                                                              \
           ffw_log("[%s:%d]-" format " \n", __FUNCTION__, __LINE__, ##__VA_ARGS__);                                                   \
        }                                                                                                                                                                                              \
    } while (0);

/// log trace
#define FFW_LOG_TRACE(format, ...)                                                                                                                                                                  \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        if (ffw_log_should_log(FFW_LOG_LEVEL_TRACE))                                                                                                                                             \
        {                                                                                                                                                                                              \
           ffw_log("[][%s:%d]-" format " \n", __FUNCTION__, __LINE__, ##__VA_ARGS__);                                                   \
        }                                                                                                                                                                                              \
    } while (0);

/// log warn
#define FFW_LOG_WARN(format, ...)                                                                                                                                                                   \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        if (ffw_log_should_log(FFW_LOG_LEVEL_WARN))                                                                                                                                              \
        {                                                                                                                                                                                              \
           ffw_log("[][fibocom][fatal][][%s:%d]-" format " \n", __FUNCTION__, __LINE__, ##__VA_ARGS__);                                                   \
      }                                                                                                                                                                                              \
    } while (0);
    
#else /// C1
/// macro of base log
#define LOG_P(format, ...)                                                                                 \
    do                                                                                                     \
    {                                                                                                      \
        char datename[48];                                                                                 \
        ffw_get_date_string(datename, sizeof(datename));                                                \
        ffw_log("[fibocom][%s][%s:%d]-" format " \n", datename, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    } while (0);

/// redefine to ffw_log
#define FFW_LOG LOG_P

/// print log if condtion meet
#define FFW_LOG_INFO_IF(con, format, ...)                                                                                                                                                           \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        if (ffw_log_should_log(FFW_LOG_LEVEL_INFO) && (con))                                                                                                                                     \
        {                                                                                                                                                                                              \
            char datename[48];                                                                                                                                                                         \
            ffw_get_date_string(datename, sizeof(datename));                                                                                                                                        \
            ffw_log("[%s][fibocom][info][%s][%s:%d]-" format " \n", ffw_self_thread_name(), datename, __FUNCTION__, __LINE__, ##__VA_ARGS__);                                                    \
        }                                                                                                                                                                                              \
    } while (0);

/// log info
#define FFW_LOG_INFO(format, ...)                                                                                                                                                                   \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        if (ffw_log_should_log(FFW_LOG_LEVEL_INFO))                                                                                                                                              \
        {                                                                                                                                                                                              \
            char datename[48];                                                                                                                                                                         \
            ffw_get_date_string(datename, sizeof(datename));                                                                                                                                        \
            ffw_log("[%s][fibocom][info][%s][%s:%d]-" format " \n", ffw_self_thread_name(), datename, __FUNCTION__, __LINE__, ##__VA_ARGS__);                                                    \
        }                                                                                                                                                                                              \
    } while (0);

/// log fatal
#define FFW_LOG_FATAL(format, ...)                                                                                                                                                                  \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        if (ffw_log_should_log(FFW_LOG_LEVEL_FATAL))                                                                                                                                             \
        {                                                                                                                                                                                              \
            char datename[48];                                                                                                                                                                         \
            ffw_get_date_string(datename, sizeof(datename));                                                                                                                                        \
            ffw_log("[%s][fibocom][fatal][%s][%s:%d]-" format " \n", ffw_self_thread_name(), datename, __FUNCTION__, __LINE__, ##__VA_ARGS__);                                                   \
        }                                                                                                                                                                                              \
    } while (0);

/// log error
#define FFW_LOG_ERROR(format, ...)                                                                                                                                                                  \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        if (ffw_log_should_log(FFW_LOG_LEVEL_ERROR))                                                                                                                                             \
        {                                                                                                                                                                                              \
            char datename[48];                                                                                                                                                                         \
            ffw_get_date_string(datename, sizeof(datename));                                                                                                                                        \
            ffw_log("[%s][fibocom][error][%s][%s:%d]-" format " \n", ffw_self_thread_name(), datename, __FUNCTION__, __LINE__, ##__VA_ARGS__);                                                   \
        }                                                                                                                                                                                              \
    } while (0);

/// log debug
#define FFW_LOG_DEBUG(format, ...)                                                                                                                                                                  \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        if (ffw_log_should_log(FFW_LOG_LEVEL_DEBUG))                                                                                                                                             \
        {                                                                                                                                                                                              \
            char datename[48];                                                                                                                                                                         \
            ffw_get_date_string(datename, sizeof(datename));                                                                                                                                        \
            ffw_log("[%s][fibocom][debug][%s][%s:%d]-" format " \n", ffw_self_thread_name(), datename, __FUNCTION__, __LINE__, ##__VA_ARGS__);                                                   \
        }                                                                                                                                                                                              \
    } while (0);

/// log trace
#define FFW_LOG_TRACE(format, ...)                                                                                                                                                                  \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        if (ffw_log_should_log(FFW_LOG_LEVEL_TRACE))                                                                                                                                             \
        {                                                                                                                                                                                              \
            char datename[48];                                                                                                                                                                         \
            ffw_get_date_string(datename, sizeof(datename));                                                                                                                                        \
            ffw_log("[%s][fibocom][trace][%s][%s:%d]-" format " \n", ffw_self_thread_name(), datename, __FUNCTION__, __LINE__, ##__VA_ARGS__);                                                   \
        }                                                                                                                                                                                              \
    } while (0);

/// log warn
#define FFW_LOG_WARN(format, ...)                                                                                                                                                                   \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        if (ffw_log_should_log(FFW_LOG_LEVEL_WARN))                                                                                                                                              \
        {                                                                                                                                                                                              \
            char datename[48];                                                                                                                                                                         \
            ffw_get_date_string(datename, sizeof(datename));                                                                                                                                        \
            ffw_log("[%s][fibocom][warn][%s][%s:%d]-" format " \n", ffw_self_thread_name(), datename, __FUNCTION__, __LINE__, ##__VA_ARGS__);                                                    \
        }                                                                                                                                                                                              \
    } while (0);
#endif
/**
 * @brief 初始化日志组件
 * 
 * @return int 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败 
 */
int ffw_log_init();

/**
 * @brief 设置日志级别
 * 
 * @param level 日志级别
 * @return int 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败 
 */
int ffw_log_set_level(ffw_log_level_t level);


/**
 * @brief 获取日志级别
 * 
 * @return int 
 */
int ffw_log_get_level();

/**
 * @brief 判断某个日志级别是否可以打印
 * 
 * @param level 当前日志级别
 * @return true 可以打印
 * @return false 不可以打印
 */
bool ffw_log_should_log(ffw_log_level_t level);

/**
 * @brief 以hex编码的方式打印二进制数据
 * 
 * @param prefix 前缀
 * @param data 二进制数据
 * @param len 二进制数据长度
 * @return int 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败 
 */
int ffw_log_dump(const char *prefix, const uint8_t *data, int len);


#endif /* BF7A9477_5991_4A5C_9C75_F926B6A35AA5 */
