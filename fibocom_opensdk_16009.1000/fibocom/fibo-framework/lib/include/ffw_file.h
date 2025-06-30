#ifndef LIB_INCLUDE_FFW_FILE
#define LIB_INCLUDE_FFW_FILE

#include <stdint.h>

/**
* @file ffw_file.h
* @author sundaqing (sundaqing@fibocom.com)
* @brief 文件操作的二次封装，包括读取文件，追加文件，递归遍历目录，递归删除目录等等
* @version 0.1
* @date 2021-05-24
* 
* Copyright (c) 2021 Fibocom. All rights reserved
* 
 */

#include <stdio.h>
#include "stdbool.h"

/**
* @brief 遍历目录的回调函数
* 
* @param dirname 遍历的目录
* @param name 文件的相对路径
* @param abspath 文件的绝对路径
* @param is_file true: 文件 false: 目录
* @param arg: 回调上下文
* @return int 这个返回值会作为调用者判断成功或者失败的标志
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
typedef int (*listdir_callback_t)(const char *dirname, const char *name, const char *abspath, bool is_file, void *arg);

/**
* @brief 以覆盖的方式写入文件
* 
* @param path 文件路径 
* @param data 数据
* @param size 数据大小
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_file_write(const char *path, const void *data, size_t size);

/**
* @brief 以追加的方式写入文件
* 
* @param path 文件路径 
* @param data 数据
* @param size 数据大小
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_file_append(const char *path, const void *data, size_t size);

/**
* @brief 读取文件特定长度的数据
* 
* @param path 文件路径
* @param dst 存储数据缓冲区
* @param size @c dst 缓冲区的大小
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_file_read(const char *path, void *dst, size_t size);

/**
 * @brief 读取文件特定长度的数据
 * 
* @param path 文件路径
* @param dst 存储数据缓冲区
* @param size @c dst 缓冲区的大小
* @param lenout 读取数据的长度
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
* @return int 
*/
int ffw_file_read1(const char *path, void *dst, size_t size, uint32_t *lenout);

/**
* @brief 读取整个文件数据
* 
* @param path 文件路径
* @param[out] buf 存储数据缓冲区, 使用完之后，需要用 @c ffw_put_memory 释放
* @param[out] size 数据大小
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_file_read_all(const char *path, void **buf, int *size);

/**
* @brief 截断文件到指定长度
* 
* @param path 文件路径
* @param length 长度
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_truncate(const char *path, long length);

/**
* @brief 文件拷贝
* 
* @param src 源文件
* @param dst 目的文件
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_file_copy(const char *src, const char *dst);

/** 
* @brief 使用回调函数的方式读取文件
* 
* @param path 文件路径
* @param buff_size 读取数据大小
* @param cb 回调函数
* @param arg 回调上下文
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_file_read_async(const char *path, int32_t buff_size, int (*cb)(void *buf, int32_t size, void *arg), void *arg);

/**
* @brief 使用回调函数的方式读取整个文件
* 
* @param path 文件路径
* @param cb 回调函数
* @param arg 回调上下文
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_file_read_all_async(const char *path, int (*cb)(void *buf, int32_t size, void *arg), void *arg);

/**
* @brief 遍历目录
* 
* @param name 目录名称
* @param recursive 是否递归
* @param cb 回调函数
* @param arg 回调上下文
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_file_listdir(const char *name, bool recursive, listdir_callback_t cb, void *arg);

/**
* @brief 删除目录
* 
* @param name 目录名
* @param recursive 是否递归删除
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_file_rmdir(const char *name, bool recursive);

/**
* @brief 拷贝文件或者目录
* 
* @param src 源文件或者目录
* @param dst 目的文件或者目录
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_file_cpdir(const char *src, const char *dst);


/**
 * @brief 判断目录是否存在
 * 
 * @param name 目录名
 * @return true 存在
 * @return false 不存在
 */
bool ffw_file_dir_exist(const char *name);


/**
* @brief 判断文件是否存在
* 
* @param name 文件名
 * @return true 存在
 * @return false 不存在
 */
bool ffw_file_exist(const char *name);


/**
 * @brief 递归创建目录
 * 
 * @param name 目录名
 * @param mode 权限
 * @return int 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 */
int ffw_file_mkdir_p(const char *name, int mode);

#endif /* LIB_INCLUDE_FFW_FILE */
