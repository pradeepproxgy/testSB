#ifndef C07392CB_8831_470A_ABEC_80E747195EDC
#define C07392CB_8831_470A_ABEC_80E747195EDC

/**
* @file ffw_url.h
* @author sundaqing (sundaqing@fibocom.com)
* @brief URL解析
* @version 0.1
* @date 2021-05-29
* 
* Copyright (c) 2021 Fibocom. All rights reserved
* 
 */

#include <stdint.h>
#include <stdbool.h>

/// type of ffw_url_s
typedef struct ffw_url_s ffw_url_t;

/**
* @brief URL解析结构体
* 
 */
struct ffw_url_s
{
    ///URL的拷贝
    char *b;

    ///协议
    char *scheme;

    ///路径
    char *path;

    ///包括query的原始路径
    char *raw_path;

    ///域名或者IP
    char *host;

    ///query部分
    char *query;

    ///query各个部分的位置
    uint16_t *query_pos;

    ///query部分的个数
    int32_t query_count;

    ///端口
    uint16_t port;

    ///是否是ipv6
    bool host_ipv6;
};

/**
* @brief 解析URL字符串
* 
* @param url URL字符串
* @param[out] u 解析后的结果 
* @return int 
 */
int ffw_url_parse(const char *url, ffw_url_t *u);

/**
* @brief 获取某个query的值
* 
* @param u 使用 @c ffw_url_parse 解析后的url
* @param name query的名字
* @param[out] value query的值
* @return int
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_url_query_get(const ffw_url_t *u, const char *name, char **value);

/**
* @brief 释放解析的url，@c ffw_url_parse 函数的结果如果不使用，需要使用此函数释放资源
* 
* @param u 使用 @c ffw_url_parse 解析后的url
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_url_free(ffw_url_t *u);

#endif /* C07392CB_8831_470A_ABEC_80E747195EDC */
