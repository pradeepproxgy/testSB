#ifndef LIB_INCLUDE_FFW_NV
#define LIB_INCLUDE_FFW_NV

/**
 * @file ffw_nv.h
 * @author sundaqing (sundaqing@fibocom.com)
 * @brief 持久化的一些配置设置，用JSON文件配置维护，编译时用 ./tools/gen.py转换为二进制格式，此时可以使用 @c ::ffw_conf_parse 解析
 * @version 0.1
 * @date 2021-05-29
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <stdint.h>
#include <ffw_buffer.h>

/**
* @brief 解析配置数据
* 
* @param data 配置模板数据
* @param len 数据长度
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_conf_parse(uint8_t *data, int32_t len);

/**
* @brief 加载保存的配置数据
* 
* @param filename 配置文件
* @param data 配置模板数据
* @param len 模板数据长度
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_conf_merge_from_file(const char *filename, uint8_t *data, int32_t len);

/**
 * @brief 从文件加载保存的数据，并和当前数据合并，文件的路径默认已经定义
 * 
 * @param data 配置模板数据
 * @param len 模板数据长度
 * @return int 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 */
int ffw_conf_load(uint8_t *data, int32_t len);

/**
* @brief 将配置数据dump到文件
* 
* @param conf 配置数据
* @param filename 文件路径
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_conf_dump_tofile(const void *conf, const char *filename);

/**
* @brief 根据名字获取配置数据
* 
* @param name 配置项名称
* @param[out] data 数据
* @param count 数据个数
* @return int32_t 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int32_t ffw_nv_get(const char *name, int32_t *data, int count);

/**
* @brief 根据名字获取配置数据
* 
* @param name 
* @param data 
* @param count 
* @param count_out 
* @return int32_t 
 */
int32_t ffw_nv_get2(const char *name, int32_t *data, int count, int *count_out);

/**
* @brief 根据名字获取配置数据，针对只有一个数据的配置项
* 
* @param name 配置项名称
* @param[out] data 数据
* @return int32_t 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int32_t ffw_nv_get1(const char *name, int32_t *data);

/**
* @brief 设置配置项
* 
* @param name 配置项名称
* @param data 数据
* @param count 数据个数
* @return int32_t 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int32_t ffw_nv_set(const char *name, int32_t *data, int count);

/**
* @brief 设置配置项，可以控制是否掉电保存
* 
* @param name 配置项名称
* @param data 数据
* @param count 数据个数
* @param save 是否需要掉电保存，如果这个参数为false， 即使配置的gtset为掉电保存，也不会保存
* @return int32_t 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int32_t ffw_nv_set1(const char *name, int32_t *data, int count, bool save);

/**
* @brief 设置配置项，可以控制是否掉电保存
* 
* @param name 配置项名称
* @param data 数据
* @param count 数据个数
* @param save 是否需要掉电保存，如果这个参数为false， 即使配置的gtset为掉电保存，也不会保存
* @param class_type 1:fibocom gtset 2:ec200 qicfg 3:ec200 qisslcfg
* @return int32_t 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int32_t ffw_nv_set2(const char *name, int32_t *data, int count, bool save, int class_type);


/**
 * @brief 设置配置项完成时通知函数
 * 
 * @param name 设置项
 * @param notify_callback callback
 * @return int32_t 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 */
int32_t ffw_nv_set_notify(const char *name, void (*notify_callback)(const char *name, const int32_t *data, int count));

/**
* @brief 设置配置数据保存的文件路径
* 
* @param filename 文件路径
* @return int32_t 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int32_t ffw_nv_filename(const char *filename);

/**
* @brief 将配置数据转换为字符串，人可以看懂的格式
* 
* @param b 缓冲区
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_nv_to_str(ffw_buffer_t *b);

/**
* @brief 将配置数据转换为字符串，人可以看懂的格式
* 
* @param b 缓冲区
* @param class 1 fibocom 2 yy
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_nv_to_str1(ffw_buffer_t *b, int class);

/**
* @brief 将配置数据转换为字符串，人可以看懂的格式, EC200使用
* 
* @param b 缓冲区
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_nv_to_str_ec200(ffw_buffer_t *b);
#endif /* LIB_INCLUDE_FFW_NV */
