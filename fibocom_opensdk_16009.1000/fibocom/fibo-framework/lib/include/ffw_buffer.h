#ifndef LIB_INCLUDE_FFW_BUFFER
#define LIB_INCLUDE_FFW_BUFFER

/**
* @file ffw_buffer.h
* @author sundaqing (sundaqing@fibocom.com)
* @brief 实现了缓冲区组件，可以自动判断内存大小，并自动扩展大小
* @version 0.1
* @date 2021-04-01
* 
* @copyright Copyright (c) 2021
* 
 */

#include <stdint.h>
#include <stdbool.h>

#define FFW_BUFFER_CREATE(b) \
    ffw_buffer_t _;          \
    ffw_buffer_t *b = &_;    \
    ffw_buffer_create(b, 0);

/// type of ffw_buffer_s
typedef struct ffw_buffer_s ffw_buffer_t;

/**
* @brief 缓冲区定义
* 
 */
struct ffw_buffer_s
{
    /// 魔术数字
    uint32_t magic;

    /// 缓冲区内存
    void *base;

    /// 缓冲区内存大小
    uint32_t length;

    /// 已使用的内存大小
    uint32_t used;

    /// 数据开始地址
    uint32_t current;

    /// 调用 @c ::ffw_buffer_destory 时，是否需要释放 @c base
    bool need_free;
};

/// 获取使用的缓冲区的起始地址
#define ffw_buffer_base(b) ((void *)(b)->base)

/// 获取当前数据的起始地址
#define ffw_buffer_current(b) ((void *)((uint8_t *)(b)->base + (b)->current))

/// 获取空闲起始地址
#define ffw_buffer_used(b) ((void *)((uint8_t *)((b)->base) + (b)->used))

/// 获取缓冲区的总长度
#define ffw_buffer_length(b) ((b)->length)

///消耗的buffer空间
#define ffw_buffer_usedlength(b) ((b)->used)

///消耗的数据字节数
#define ffw_buffer_consumedlength(b) ((b)->current)

///可用的数据空间
#define ffw_buffer_remaininglength(b) ((b)->used - (b)->current)

///可用的数据空间
#define ffw_buffer_datalength(b) ffw_buffer_remaininglength(b)

///可用的buff空间
#define ffw_buffer_availablelength(b) ((b)->length - (b)->used)

/// 写入一个字节
#define ffw_buffer_getuint8(b)                                                                 \
    ({                                                                                            \
        unsigned char *cp;                                                                        \
        uint8_t result = 0;                                                                       \
        CHECK((b)->used < (sizeof(uint8_t) + (b)->current), FFW_R_FAILED, "buffer too short"); \
        cp = (unsigned char *)ffw_buffer_current(b);                                           \
        (b)->current += sizeof(uint8_t);                                                          \
        result |= ((uint8_t)(cp[0]));                                                             \
        (result);                                                                                 \
    })

/// 以网络序冲缓冲区获取uint16
#define ffw_buffer_getuint16(b)                                                                 \
    ({                                                                                             \
        unsigned char *cp;                                                                         \
        uint16_t result = 0;                                                                       \
        CHECK((b)->used < (sizeof(uint16_t) + (b)->current), FFW_R_FAILED, "buffer too short"); \
        cp = (unsigned char *)ffw_buffer_current(b);                                            \
        (b)->current += sizeof(uint16_t);                                                          \
        result = ((uint16_t)(cp[0])) << 8;                                                         \
        result |= ((uint16_t)(cp[1]));                                                             \
        (result);                                                                                  \
    })

/// 以网络序冲缓冲区获取uint32
#define ffw_buffer_getuint32(b)                                                                 \
    ({                                                                                             \
        unsigned char *cp;                                                                         \
        uint32_t result = 0;                                                                       \
        CHECK((b)->used < (sizeof(uint32_t) + (b)->current), FFW_R_FAILED, "buffer too short"); \
        cp = (unsigned char *)ffw_buffer_current(b);                                            \
        (b)->current += sizeof(uint32_t);                                                          \
        result = ((uint32_t)(cp[0])) << 24;                                                        \
        result |= ((uint32_t)(cp[1])) << 16;                                                       \
        result |= ((uint32_t)(cp[2])) << 8;                                                        \
        result |= ((uint32_t)(cp[3]));                                                             \
        (result);                                                                                  \
    })

/// 以网络序冲缓冲区获取uint64
#define ffw_buffer_getuint64(b)                                                                 \
    ({                                                                                             \
        unsigned char *cp;                                                                         \
        uint64_t result = 0;                                                                       \
        CHECK((b)->used < (sizeof(uint64_t) + (b)->current), FFW_R_FAILED, "buffer too short"); \
        cp = (unsigned char *)ffw_buffer_current(b);                                            \
        (b)->current += sizeof(uint64_t);                                                          \
        result = ((uint64_t)(cp[0])) << 56;                                                        \
        result |= ((uint64_t)(cp[1])) << 48;                                                       \
        result |= ((uint64_t)(cp[2])) << 40;                                                       \
        result |= ((uint64_t)(cp[3])) << 32;                                                       \
        result |= ((uint64_t)(cp[4])) << 24;                                                       \
        result |= ((uint64_t)(cp[5])) << 16;                                                       \
        result |= ((uint64_t)(cp[6])) << 8;                                                        \
        result |= ((uint64_t)(cp[7]));                                                             \
        (result);                                                                                  \
    })

/**
* @brief 初始化一个buffer实例
* 
* @param b 
* @param base 用于使用的缓冲区
* @param length 用于使用的的缓冲区的长度
* @return int 
 */
int ffw_buffer_init(ffw_buffer_t *b, void *base, uint32_t length);

/**
* @brief 
* 
* @param b 
* @return int 
 */
int ffw_buffer_reset(ffw_buffer_t *b);

/**
* @brief 
* 
* @param b 
* @param size 
* @return int 
 */
int ffw_buffer_resize(ffw_buffer_t *b, uint32_t size);

/**
* @brief 
* 
* @param b 
* @param size 
* @return int 
 */
int ffw_buffer_create(ffw_buffer_t *b, uint32_t size);

/**
* @brief 
* 
* @param b 
* @return int 
 */
int ffw_buffer_destory(ffw_buffer_t *b);

/**
* @brief 占用一定字节的空间
* 
* @param b 
* @param n 
* @return int 
 */
int ffw_buffer_add(ffw_buffer_t *b, uint32_t n);

/**
* @brief 交还一定地址的空间
* 
* @param b 
* @param n 
* @return int 
 */
int ffw_buffer_sub(ffw_buffer_t *b, unsigned int n);

/**
* @brief 消费一定字节的数据
* 
* @param b 
* @param n 
* @return int 
 */
int ffw_buffer_forward(ffw_buffer_t *b, uint32_t n);

/**
* @brief 交还一定字节的数据
* 
* @param b 
* @param n 
* @return int 
 */
int ffw_buffer_back(ffw_buffer_t *b, uint32_t n);

/**
* @brief 将所有数据移动的buffer的开头
* 
* @param b 
* @return int 
 */
int ffw_buffer_compact(ffw_buffer_t *b);

/**
* @brief 拷贝数据到 @c b, 如果缓冲区的空间不够，则自动扩大缓冲区的空间
* 
* @param b 
* @param data 
* @param len 
* @return int 
 */
int ffw_buffer_append(ffw_buffer_t *b, const void *data, uint32_t len);

/**
* @brief 尝试给 @c b 拷贝 @c len 长度的数据，如果缓冲区的空间不够，则只拷贝部分数据，并且返回错误
* 
* @param b 
* @param data 
* @param len 
* @return int 
 */
int ffw_buffer_try_append(ffw_buffer_t *b, const void *data, uint32_t len);

/**
* @brief 用格式化的方式给 @c b填充数据，类似于snprintf
* 
* @param b 缓冲区句柄
* @param append 是否需要追加到buffer的后面
* @param fmt 
* @param ... 
* @return int 
 */
int ffw_buffer_csprintf(ffw_buffer_t *b, bool append, const char *fmt, ...) __attribute__((format(printf, 3, 4)));

/**
* @brief 移动缓冲区内容
* 
* @param src 
* @param dst 
* @return int 
 */
int ffw_buffer_move(ffw_buffer_t *src, ffw_buffer_t *dst);

/**
 * @brief 以网络序吸入一个uint8到缓冲区
 * 
 * @param b 
 * @param c 
 * @return int 
 */
int ffw_buffer_putuint8(ffw_buffer_t *b, uint8_t c);

/**
 * @brief 以网络序吸入一个uint16到缓冲区
 * 
 * @param b 
 * @param c 
 * @return int 
 */
int ffw_buffer_putuint16(ffw_buffer_t *b, uint16_t c);

/**
 * @brief 以网络序吸入一个uint32到缓冲区
 * 
 * @param b 
 * @param c 
 * @return int 
 */
int ffw_buffer_putuint32(ffw_buffer_t *b, uint32_t c);

/**
* @brief 以网络序吸入一个uint64到缓冲区
* 
* @param b 
* @param c 
* @return int 
 */
int ffw_buffer_putuint64(ffw_buffer_t *b, uint64_t c);

/**
* @brief * @brief 对数据进行HEX编码，以缓冲区的形式返回结果，需要使用 @c ::ffw_buffer_destory 释放资源
* 
* @param b 
* @param data 
* @param data_size 
* @return int 
 */
int ffw_buffer_hex_encode(ffw_buffer_t *b, const void *data, uint32_t data_size);

/**
* @brief 对数据进行HEX解码，以缓冲区的形式返回结果，需要使用 @c ::ffw_buffer_destory 释放资源
* 
* @param b 
* @param data 
* @param data_size 
* @return int 
 */
int ffw_buffer_hex_decode(ffw_buffer_t *b, const void *data, uint32_t data_size);

/**
* @brief 对数据进行base64编码，以缓冲区的形式返回结果，需要使用 @c ::ffw_buffer_destory 释放资源
* 
* @param b 
* @param data 
* @param data_size 
* @return int 
 */
int ffw_buffer_base64_encode(ffw_buffer_t *b, const void *data, uint32_t data_size);

/**
* @brief 对数据进行base64解码，以缓冲区的形式返回结果，需要使用 @c ::ffw_buffer_destory 释放资源
* 
* @param b 
* @param data 
* @param data_size 
* @return int 
 */
int ffw_buffer_base64_decode(ffw_buffer_t *b, const void *data, uint32_t data_size);

/// printf will overwrite buffer
#define ffw_buffer_sprintf(b, format, ...) ffw_buffer_csprintf(b, false, format, ##__VA_ARGS__)

/// printf will append to buffer
#define ffw_buffer_asprintf(b, format, ...) ffw_buffer_csprintf(b, true, format, ##__VA_ARGS__)

#endif /* LIB_INCLUDE_FFW_BUFFER */
