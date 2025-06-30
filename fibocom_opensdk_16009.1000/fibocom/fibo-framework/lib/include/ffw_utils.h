#ifndef LIB_INCLUDE_FFW_UTILS
#define LIB_INCLUDE_FFW_UTILS

/**
* @file ffw_utils.h
* @author sundaqing (sundaqing@fibocom.com)
* @brief 一些工具函数
* @version 0.1
* @date 2021-05-06
* 
* Copyright (c) 2021 Fibocom. All rights reserved
* 
 */

#include <stdint.h>
#include <stdbool.h>

#include "ffw_result.h"
#include "ffw_types.h"
#include "ffw_plat_header.h"

/// compate with c++
#define EXPORT_C extern "C"

/// The size of a array
#define FFW_ARRAY_SIZE(arr) (sizeof(arr) / (sizeof((arr)[0])))

/// 大小4字节对齐
#define FFW_ALIGN(d, a) (((d) + ((a)-1)) & ~((a)-1))

/// IPV4地址长度
#define FFW_INET_ADDRSTRLEN (16)

/// IPV6地址长度
#define FFW_INET6_ADDRSTRLEN (46)

/**
 * @brief 取两者之中最小值
 * 
 */
#define FFW_MIN(a, b)       \
    ({                         \
        typeof(a) __a = (a);   \
        typeof(b) __b = (b);   \
        __a > __b ? __b : __a; \
    })

/**
 * @brief 去两者之中最大值
 * 
 */
#define FFW_MAX(a, b)       \
    ({                         \
        typeof(a) __a = (a);   \
        typeof(b) __b = (b);   \
        __a > __b ? __a : __b; \
    })

/// break if
#define FFW_BREAK_IF(x) \
    if (x)                 \
    {                      \
        break;             \
    }

/// break if then print log
#define FFW_BREAK_IF_LOG(x, format, ...)     \
    if (x)                                      \
    {                                           \
        FFW_LOG_INFO(format, ##__VA_ARGS__); \
        break;                                  \
    }

/// break if, assign, then print log
#define FFW_BREAK_AS_LOG(x, t, format, ...)  \
    if (x)                                      \
    {                                           \
        (t);                                    \
        FFW_LOG_INFO(format, ##__VA_ARGS__); \
        break;                                  \
    }

/// continue if, then print log
#define FFW_CONTINUE_IF_LOG(x, format, ...)  \
    if (x)                                      \
    {                                           \
        FFW_LOG_INFO(format, ##__VA_ARGS__); \
        continue;                               \
    }

/// continue if
#define FFW_CONTINUE_IF(x) \
    if (x)                    \
    {                         \
        continue;             \
    }

/**
 * @brief 分配指定大小的内存，会对内存进行重置
 * 
 */
#define ffw_get_memory(size)                               \
    ({                                                        \
        void *p = ffw_malloc(size);                        \
        if (ffw_should_print_memory_use_info())            \
        {                                                     \
            FFW_LOG_INFO("malloc :%p %d", p, (int)(size)); \
        }                                                     \
        else                                                  \
        {                                                     \
            FFW_LOG_DEBUG("malloc :%p", p)                 \
        };                                                    \
        p;                                                    \
    })

/**
 * @brief 模拟申请内存，记录系统函数申请的内存
 * 
 */
#define ffw_get_memory_mock(p)                   \
    ({                                              \
        ffw_inc_malloc();                        \
        if (ffw_should_print_memory_use_info())  \
        {                                           \
            FFW_LOG_INFO("malloc :%p mock", p);  \
        }                                           \
        else                                        \
        {                                           \
            FFW_LOG_DEBUG("malloc :%p mock", p); \
        };                                          \
    })

/**
 * @brief 模拟释放内存，记录系统函数释放的内存
 * 
 */
#define ffw_put_memory_mock(p)                  \
    ({                                             \
        ffw_inc_malloc();                       \
        if (ffw_should_print_memory_use_info()) \
        {                                          \
            FFW_LOG_INFO("free :%p mock", p);   \
        }                                          \
        else                                       \
        {                                          \
            FFW_LOG_DEBUG("free :%p mock", p);  \
        };                                         \
    })

/**
 * @brief 分配指定大小的内存，如果分配失败，会goto到错误处理
 * 
 */
#define ffw_get_memory1(size)                              \
    ({                                                        \
        void *p = ffw_malloc(size);                        \
        CHECK(p == NULL, FFW_R_FAILED, "get memory fail"); \
        if (ffw_should_print_memory_use_info())            \
        {                                                     \
            FFW_LOG_INFO("malloc :%p %d", p, (int)(size)); \
        }                                                     \
        else                                                  \
        {                                                     \
            FFW_LOG_DEBUG("malloc :%p", p);                \
        };                                                    \
        p;                                                    \
    })

/**
 * @brief 复制一份字符串，需要通过 @c ::ffw_put_memory 释放内存
 * 
 */
#define ffw_strdup(str)                       \
    ({                                           \
        void *p = NULL;                          \
        if ((str) != NULL)                       \
        {                                        \
            int __size = strlen(str);            \
            p = ffw_get_memory(__size + 1);   \
            if (p != NULL)                       \
            {                                    \
                ffw_memcpy(p, (str), __size); \
            }                                    \
        }                                        \
        p;                                       \
    })

/**
 * @brief 复制一份字符串，最大长度为 n,需要通过 @c ::ffw_put_memory 释放内存s
 * 
 */
#define ffw_strndup(str, n)                     \
    ({                                             \
        void *p = NULL;                            \
        if ((str) != NULL)                         \
        {                                          \
            int __size = ffw_strnlen((str), n); \
            p = ffw_get_memory(__size + 1);     \
            if (p != NULL)                         \
            {                                      \
                ffw_memcpy(p, (str), __size);   \
            }                                      \
        }                                          \
        p;                                         \
    })

/**
 * @brief 释放内存
 * 
 */
#define ffw_put_memory(p)                           \
    {                                                  \
        if (p != NULL)                                 \
        {                                              \
            if (ffw_should_print_memory_use_info()) \
            {                                          \
                FFW_LOG_INFO("free:%p", p)          \
            }                                          \
            else                                       \
            {                                          \
                FFW_LOG_DEBUG("free:%p", p)         \
            }                                          \
        }                                              \
        ffw_free(p);                                \
        p = NULL;                                      \
    }

/**
 * @brief 内存拷贝，安全模式
 * 
 */
#define ffw_memcpy_s(dst, dst_len, src, src_len)                             \
    {                                                                           \
        ffw_memcpy(dst, src, (dst_len) > (src_len) ? (src_len) : (dst_len)); \
    }

/// check x is digit
#define ISDIGIT(x) (x >= '0' && x <= '9')

/// call function with 1 parameter
#define FFW_CALL_FUNCTION0(func) \
    do                              \
    {                               \
        if ((func) != NULL)         \
        {                           \
            (func)();               \
        }                           \
    } while (0);

/// call function with 1 parameter
#define FFW_CALL_FUNCTION1(func, arg1) \
    do                                    \
    {                                     \
        if ((func) != NULL)               \
        {                                 \
            (func)(arg1);                 \
        }                                 \
    } while (0);

/// call function with 2 parameter
#define FFW_CALL_FUNCTION2(func, arg1, arg2) \
    do                                          \
    {                                           \
        if ((func) != NULL)                     \
        {                                       \
            (func)((arg1), (arg2));             \
        }                                       \
    } while (0);

/// call function with 3 parameter
#define FFW_CALL_FUNCTION3(func, arg1, arg2, arg3) \
    do                                                \
    {                                                 \
        if ((func) != NULL)                           \
        {                                             \
            (func)((arg1), (arg2), (arg3));           \
        }                                             \
    } while (0);

/// call function with 4 parameter
#define FFW_CALL_FUNCTION4(func, arg1, arg2, arg3, arg4) \
    do                                                      \
    {                                                       \
        if ((func) != NULL)                                 \
        {                                                   \
            (func)((arg1), (arg2), (arg3), (arg4));         \
        }                                                   \
    } while (0);

/// call function with 5 parameter
#define FFW_CALL_FUNCTION5(func, arg1, arg2, arg3, arg4, arg5) \
    do                                                            \
    {                                                             \
        if ((func) != NULL)                                       \
        {                                                         \
            (func)((arg1), (arg2), (arg3), (arg4), (arg5));       \
        }                                                         \
    } while (0);

/// call function with 6 parameter
#define FFW_CALL_FUNCTION6(func, arg1, arg2, arg3, arg4, arg5, arg6) \
    do                                                                  \
    {                                                                   \
        if ((func) != NULL)                                             \
        {                                                               \
            (func)((arg1), (arg2), (arg3), (arg4), (arg5), (arg6));     \
        }                                                               \
    } while (0);

/// call function with 7 parameter
#define FFW_CALL_FUNCTION7(func, arg1, arg2, arg3, arg4, arg5, arg6, arg7) \
    do                                                                        \
    {                                                                         \
        if ((func) != NULL)                                                   \
        {                                                                     \
            (func)((arg1), (arg2), (arg3), (arg4), (arg5), (arg6), (arg7));   \
        }                                                                     \
    } while (0);

/// call function with 8 parameter
#define FFW_CALL_FUNCTION8(func, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) \
    do                                                                              \
    {                                                                               \
        if ((func) != NULL)                                                         \
        {                                                                           \
            (func)((arg1), (arg2), (arg3), (arg4), (arg5), (arg6), (arg7), (arg8)); \
        }                                                                           \
    } while (0);

/// call function with 9 parameter
#define FFW_CALL_FUNCTION9(func, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9)   \
    do                                                                                      \
    {                                                                                       \
        if ((func) != NULL)                                                                 \
        {                                                                                   \
            (func)((arg1), (arg2), (arg3), (arg4), (arg5), (arg6), (arg7), (arg8), (arg9)); \
        }                                                                                   \
    } while (0);

/// wait a condtion in timeout
#define FFW_WAIT_TIMEOUT(lock, sem, status, timeout)                                                 \
    do                                                                                                  \
    {                                                                                                   \
        int64_t now = 0;                                                                                \
        int32_t _timeout = timeout;                                                                     \
        while (true)                                                                                    \
        {                                                                                               \
            ffw_mutex_lock(lock);                                                                    \
            bool _status = (status);                                                                    \
            ffw_mutex_unlock(lock);                                                                  \
            if (_status)                                                                                \
            {                                                                                           \
                break;                                                                                  \
            }                                                                                           \
            else                                                                                        \
            {                                                                                           \
                CHECK(_timeout <= 0 && timeout > 0, FFW_R_TIMEOUT, "wait timeout %d", (int)timeout); \
                now = ffw_time_ms();                                                                 \
                CHECK_OK_ONLY(ffw_sem_wait(sem, _timeout));                                          \
                _timeout -= (int32_t)(ffw_time_ms() - now);                                          \
            }                                                                                           \
        }                                                                                               \
    } while (0)

/// wait condition in timeout in ms
#define FFW_WAIT_COND_TIMEOUT(lock, sem, cond, exec, timeout)                                    \
    do                                                                                              \
    {                                                                                               \
        int64_t now = 0;                                                                            \
        int32_t _timeout = timeout;                                                                 \
        while (true)                                                                                \
        {                                                                                           \
            now = ffw_time_ms();                                                                 \
            CHECK_OK_ONLY(ffw_sem_wait(sem, _timeout))                                           \
            ffw_mutex_lock(lock);                                                                \
            if (cond)                                                                               \
            {                                                                                       \
                (exec);                                                                             \
                ffw_mutex_unlock(lock);                                                          \
                break;                                                                              \
            }                                                                                       \
            else                                                                                    \
            {                                                                                       \
                ffw_mutex_unlock(lock);                                                          \
            }                                                                                       \
            _timeout -= (int32_t)(ffw_time_ms() - now);                                          \
            CHECK(_timeout <= 0 && timeout > 0, FFW_R_TIMEOUT, "wait timeout %d", (int)timeout); \
        }                                                                                           \
    } while (0);

/**
 * @brief 分配内存
 * 
 * @param size 分配内存的大小
 * @return void* NULL failed otherwise success 
 */
ffw_handle_t ffw_malloc(size_t size);

/**
 * @brief 分配内存
 * 
 * @param count 个数
 * @param size 大小
 * @return void* NULL failed otherwise success 
 */
ffw_handle_t ffw_calloc(size_t count, size_t size);

/**
 * @brief 释放内存
 * 
 * @param p 内存地址
 */
void ffw_free(ffw_handle_t p);

/**
* @brief 增加分配内存计数器
* 
 */
void ffw_inc_malloc();

/**
* @brief 增加释放内存计数器
* 
 */
void ffw_inc_free();

/**
 * @brief 获取内存操作计数器
 * 
 * @param malloc_num 分配内存次数
 * @param free_num 释放内存次数
 */
void ffw_get_memory_num(uint32_t *malloc_num, uint32_t *free_num);

/**
 * @brief 重置内存分配计数器
 * 
 */
void ffw_reset_malloc_num();

/**
* @brief 打印内存分配计数器
* 
 */
void ffw_print_memory_num();

/**
 * @brief Convert binary data to hex string
 *
 * @param buf The source binary data
 * @param len The buffer length of @c buf
 * @param out_buf The out data buffer
 * @param out_buf_len The length of @c out_buf
 * @param upper upper case or lower case
 * @return int int 0 success otherwise failed
 */
int ffw_bin2hex1(const uint8_t *buf, uint32_t len, uint8_t *out_buf, uint32_t out_buf_len, bool upper);

/**
 * @brief Convert binary data to hex string
 *
 * @param buf The source binary data
 * @param len The buffer length of @c buf
 * @param out_buf The out data buffer
 * @param out_buf_len The length of @c out_buf
 * @return int 0 success otherwise failed
 */
int ffw_bin2hex(const uint8_t *buf, uint32_t len, uint8_t *out_buf, uint32_t out_buf_len);

/**
 * @brief Convert hex string data to binary data
 *
 * @param buf The source binary data
 * @param len The buffer length of @c buf
 * @param out_buf The out data buffer
 * @param out_buf_len The length of @c out_buf
 * @return int 0 success otherwise failed
 */
int ffw_hex2bin(const char *buf, uint32_t len, uint8_t *out_buf, uint32_t out_buf_len);

/**
 *@brief Remove specified char at the beginning of the string
 *
 *@param str The string will been trimed
 *@param ch The removing char
 *@return const char* trimed string
 */
char *ffw_str_trim_left(char *str, char ch);

/**
 *@brief Remove specified char at the end of the string
 *
 *@param str The string will been trimed
 *@param ch The removing char
 *@return const char* trimed string
 */
char *ffw_str_trim_right(char *str, char ch);

/**
 *@brief Remove specified char at the beginning and at the end of the string
 *
 *@param str The string will been trimed
 *@param ch The removing char
 *@return const char* trimed string
 */
char *ffw_str_trim(char *str, char ch);

/**
* @brief 将字符串转换为数字
* 
* @param str 字符串形式的数字
* @param[out] result 转换后的数字 
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_strtoint32(const char *str, int32_t *result);

/**
* @brief 将字符串转换为无符号数字
* 
* @param str 字符串形式的数字
* @param[out] result 转换后的数字 
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_strtouint32(const char *str, uint32_t *result);

/**
 * @brief 将字符串转换为数字
 * 
 * @param str 字符串形式的数字
 * @param min 最小值
 * @param max 最大值
 * @param result 转换后的数字 
 * @return int 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 */
int ffw_strtoint32_range(const char *str, int32_t min, int32_t max, int32_t *result);

/// write one byte to buffer
#define ffw_write_uint8(d, val) \
    do                             \
    {                              \
        uint8_t *t = (uint8_t *)d; \
        t = val;                   \
        d += 1;                    \
    } while (0)

/// write uint16 byte to buffer int network order
#define ffw_write_uint16(d, val) \
    do                              \
    {                               \
        uint8_t *t = (uint8_t *)d;  \
        t[0] = (uint8_t)(val >> 8); \
        t[1] = (uint8_t)val;        \
        d += 2;                     \
    } while (0)

/// write uint24 byte to buffer int network order
#define ffw_write_uint24(d, val)  \
    do                               \
    {                                \
        uint8_t *t = (uint8_t *)d;   \
        t[0] = (uint8_t)(val >> 16); \
        t[1] = (uint8_t)(val >> 8);  \
        t[2] = (uint8_t)val;         \
        d += 3;                      \
    } while (0)

/// write uint32 byte to buffer int network order
#define ffw_write_uint32(d, val)  \
    do                               \
    {                                \
        uint8_t *t = (uint8_t *)d;   \
        t[0] = (uint8_t)(val >> 24); \
        t[1] = (uint8_t)(val >> 16); \
        t[2] = (uint8_t)(val >> 8);  \
        t[3] = (uint8_t)val;         \
        d += 4;                      \
    } while (0)

/// read uint8 from buffer as network order
#define ffw_read_uint8(d)    \
    ({                          \
        uint8_t iret = 0;       \
        iret = (uint8_t)(d[0]); \
        d += 1;                 \
        iret;                   \
    })

/// read uint16 from buffer as network order
#define ffw_read_uint16(d)         \
    ({                                \
        uint16_t iret = 0;            \
        iret = ((uint16_t)d[0]) << 8; \
        iret |= d[1];                 \
        d += 2;                       \
        iret;                         \
    })

/// read uint24 from buffer as network order
#define ffw_read_uint24(d)          \
    ({                                 \
        uint32_t iret = 0;             \
        iret = ((uint32_t)d[0]) << 16; \
        iret |= ((uint32_t)d[1]) << 8; \
        iret |= (uint32_t)(d[2];       \
        d += 3;                        \
        iret;                          \
    })

/// read uint32 from buffer as network order
#define ffw_read_uint32(d)             \
    ({                                    \
        uint32_t iret = 0;                \
        iret = ((uint32_t)(d[0])) << 24;  \
        iret |= ((uint32_t)(d[1])) << 16; \
        iret |= ((uint32_t)(d[2])) << 8;  \
        iret |= (uint32_t)(d[3]);         \
        d += 4;                           \
        iret;                             \
    })

/**
* @brief 获取字符串形式的时间
* 
* @param buf 缓冲区
* @param len 缓冲区长度
 */
void ffw_get_date_string(char *buf, uint32_t len);

/**
* @brief base64编码
* 
* @param dst 存储结果的缓冲区
* @param dlen @c dst 缓冲区的大小
* @param[out] olen 编码过后数据的大小
* @param src 源数据
* @param slen 源数据大小
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_base64_encode(unsigned char *dst, size_t dlen, size_t *olen, const unsigned char *src, size_t slen);

/**
* @brief base64解码
* 
* @param dst 存储结果的缓冲区
* @param dlen @c dst 缓冲区的大小
* @param olen 解码过后数据的大小
* @param src 源数据
* @param slen 源数据大小
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_base64_decode(unsigned char *dst, size_t dlen, size_t *olen, const unsigned char *src, size_t slen);

/**
* @brief 是否打印内存分配的日志
* 
* @param on true or false
* @return int
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_print_memory_use_info(bool on);

/**
* @brief 获取是否打印内存分配的日志开关
* 
* @return int true or false
 */
int ffw_should_print_memory_use_info();

/**
 * @brief 判断两块内存数据是否相等
 * 
 * @param data1 数据1
 * @param data1len 数据1长度
 * @param data2 数据2
 * @param data2len 数据2长度
 * @return true 相等
 * @return false 不相等
 */
bool ffw_mem_equal(const void *data1, int data1len, const void *data2, int data2len);

/**
 * @brief 判断两个字符串是否相等，需要传递字符串的长度
 * 
 * @param str1 字符串1
 * @param strlen1 字符串1的长度
 * @param str2 字符串2
 * @param strlen2 字符串2长度
 * @param case_sensetive 是否大小写敏感
 * @return true 相等
 * @return false 不相等
 */
bool ffw_nstr_equal(const char *str1, int strlen1, const char *str2, int strlen2, bool case_sensetive);

/**
 * @brief 判断两个字符串是否相等
 * 
 * @param str1 字符串1
 * @param str2 字符串2
 * @param case_sensetive 是否大小写敏感
 * @return true 相等
 * @return false 不相等
 */
bool ffw_str_equal(const char *str1, const char *str2, bool case_sensetive);

/**
 * @brief 计算bcc校验码
 * 
 * @param data 数据
 * @param len 数据长度
 * @return int 校验码
 */
int ffw_bcc(const uint8_t *data, int len);

/**
 * @brief 计算crc16校验码
 * 
 * @param data 数据
 * @param len 数据长度
 * @return int 校验码
 */
int ffw_crc16(const uint8_t *data, int len);

/**
 * @brief 转换为大写
 * 
 * @param str 字符串
 * @return char* @c str
 */
char *ffw_str_uppper(char *str);

/**
 * @brief 转换为小写
 * 
 * @param str 字符串
 * @return char* @c str
 */
char *ffw_str_lower(char *str);

/**
 * @brief 根据时间戳格式获取日期
 * 
 * @param fmt 格式, 如"yyyy-MM-dd HH:mm:ss"
 * @param ts 毫秒单位时间戳
 * @param[out] dateout 输出日期
 * @return int 成功返回 @c dateout, 失败返回NULL
 */
const char *ffw_date_fmt(const char *fmt, int64_t ts, char *dateout);

/**
 * @brief 
 * 
 * @param ts 
 * @param tm 
 * @return int 
 */
int ffw_localtime(int64_t ts, struct tm *tm);

/**
* @brief 
* 
* @param year0 
* @param mon0 
* @param day 
* @param hour 
* @param min 
* @param sec 
* @param ts 
* @return int 
 */
int ffw_mktime(const unsigned int year0, const unsigned int mon0, const unsigned int day, const unsigned int hour, const unsigned int min, const unsigned int sec, int64_t *ts);

/**
 * @brief convert bcd to ascii
 * 
 * @param bcd 
 * @param len 
 * @param ascii 
 * @param ascii_len 
 * @param outlen 
 * @return int 
 */
int ffw_bcd_to_ascii(const uint8_t *bcd, uint32_t len, uint8_t *ascii, uint32_t ascii_len, uint32_t *outlen);

/**
 * @brief convert ascii to bcd
 * 
 * @param ascii 
 * @param len 
 * @param bcd 
 * @param outlen 
 * @return int 
 */
int ffw_ascii_to_bcd(const uint8_t *ascii, uint32_t len, uint8_t *bcd, uint32_t bcd_len, uint32_t *outlen);

/**
 * @brief 校验电话号码
 * 
 * @param number 
 * @param size 
 * @param is_international 
 * @return int 
 */
int ffw_verify_phone_number(const char *number, uint32_t size, bool *is_international);

/**
 * @brief 7 bit 解码
 * 
 * @param bit7 
 * @param len 
 * @param dst 
 * @param len_out 
 * @return int 
 */
int ffw_7bit_decode(const void *bit7, uint32_t len, void *dst, uint32_t dstlen, uint32_t *len_out);

/**
 * @brief 7 bit 编码
 * 
 * @param src 
 * @param len 
 * @param dst 
 * @param len_out 
 * @return int 
 */
int ffw_7bit_encode(const void *src, uint32_t len, void *dst, uint32_t dstlen, uint32_t *len_out);

/**
 * @brief 克隆一段缓冲区数据
 * 
 * @param src 
 * @param len 
 * @param dst 
 * @return int 
 */
int ffw_dup_memory(const void *src, uint32_t len, void **dst);

/**
 * @brief 计算字符串的长度，有最大长度限制
 * 
 * @param src 
 * @param maxlen 
 * @return int 
 */
int ffw_strnlen(const char *str, int maxlen);

/**
* @brief 准备好OTA升级
* 
* @param way 0 本地升级 1 远程升级
* @return int
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_fota_ready(uint8_t way);

/**
* @brief 检查OTA升级结果
* 
* @param[out] way 0 本地升级 1 远程升级
* @return int 
 */
int ffw_check_ota_result(uint8_t *way);

/**
* @brief 分割字符串
* 
* @param str 
* @param ch 
* @param cb 
* @param arg 
* @return int 
 */
int ffw_str_split(const char *str, char ch, void (*cb)(char *ele, int32_t len, void *arg), void *arg);

/**
* @brief 
* 
* @param ts 
* @param gmtdate 
* @return const char* 
 */
const char *ffw_date_gmt(int64_t ts, char *gmtdate);

#endif /* LIB_INCLUDE_FFW_UTILS */
