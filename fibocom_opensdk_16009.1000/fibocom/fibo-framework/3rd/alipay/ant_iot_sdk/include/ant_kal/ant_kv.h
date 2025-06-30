/**
 * ant_kv.h
 *
 * KV存储访问接口
 */
#ifndef __ANT_KV_H__
#define __ANT_KV_H__

#include "ant_typedef.h"
#ifdef __cplusplus
extern "C" {
#endif
    
typedef void* kv_handle;

/**
 * kv存储初始化
 */
// void      ant_kv_init(void);

/**
 * 打开KV存储
 *
 * @return 非0值： 打开成功，返回kv访问句柄
 *           -1： 打开失败
 */
// kv_handle ant_kv_open(void);

/**
 * 获取kv中的字符串值
 * @param h kv访问句柄
 * @param key 键值
 * @param value 字符串缓存，返回键值对应的字符串
 * @param len 字符串缓存长度
 * 
 * @return  0： 获取成功
 *         -1： 获取失败
 */
// ant_s32   ant_kv_get_string(void *h, const char *key, char *value, ant_u32 *len);

/**
 * 设置kv中的字符串值
 * @param h kv访问句柄
 * @param key 键值
 * @param value 字符串缓存，键值对应的字符串值
  * 
 * @return  0： 设置成功
 *         -1： 设置失败
 */
// ant_s32   ant_kv_set_string(void *h, const char *key, char *value);

/**
 * 获取kv中的整型值
 * @param h kv访问句柄
 * @param key 键值
 * @param value 整型缓存，返回键值对应的整型数据
  * 
 * @return  0： 获取成功
 *         -1： 获取失败
 */
// ant_s32   ant_kv_get_u32(void *h, const char *key, ant_u32 *value);

/**
 * 设置kv中的整型值
 * @param h kv访问句柄
 * @param key 键值
 * @param value 整型缓存，键值对应的整型值
  * 
 * @return  0： 设置成功
 *         -1： 设置失败
 */
// ant_s32   ant_kv_set_u32(void *h, const char *key, ant_u32 value);

/**
 * 删除kv中的键值对
 * 
 * @param h kv访问句柄 
 * @param key 键值
 * 
 * @return  0： 删除成功
 *         -1： 删除失败 
 */
// ant_s32   ant_kv_erase(void *h, const char *key);

/**
 * 关闭kv句柄
 * 
 * @param h kv访问句柄 
 */
// void      ant_kv_close(kv_handle h);


/*
 * @brief: 打开block，对于任何block，无论存在与否，都应该打开成功。
    block是个抽象概念，表示一类key的集合。操作kv之前，会首先执行block打开操作。
 * @input: block_name block名称
 * @return: NULL 打开失败
 *          非NULL  打开成功,返回指向block的句柄
 * @note:  保证线程安全
 */
void *ant_iot_kv_open(const char *block_name);

/*
 * @brief: 获取某个block中指定key的value值
 * @input: h block句柄，该block必须是已经打开的。
 * @input: key key字符串，长度小于32
 * @output: 指向 value 值的指针的地址，需要将本函数内部动态分配的value内存，赋值给*value。
 * @output: len 返回的实际的value的长度
 * @return: <0 失败
 *           0 未找到key
 *          =1  成功
 * @note: 使用完毕后需要调用方释放 （void*)value 指针，否则会内存泄漏。
 * @note: 如果value是字符串，不能保证以'\0'结尾，除非调用alipay_iot_kv_set的时候，将'\0'存入了
 * @note: 保证线程安全
 */
int ant_iot_kv_get(const void *h, const char *key, void **value, size_t *len);

/*
 * @brief: 设置某个block中指定key的值为value
 * @input: h block句柄，该block必须是已经打开的。
 * @input: key key字符串，长度小于32
 * @input: value 值的地址，注意value是void*数据，非字符串，长度小于4K
 * @input: len 要设置的value的长度
 * @return: <0 失败
 *          0 没有足够的空间存储
 *          =1  成功
 * @note:  保证线程安全
 */
int ant_iot_kv_set(const void *h, const char *key, const void *value, size_t len);

/*
 * @brief: 根据key删除某个block中指定的kv
    注意kv或block删除后，后续的set仍然应该可以成功进行。
 * @input: h block句柄，该block必须是已经打开的。
 * @input: key key字符串; 若key为NULL，则删除block对应的所有kv
 * @return: <0 失败
 *           0 未找到key
 *          =1  成功
 * @note:  保证线程安全
 */
int ant_iot_kv_delete(const void *h, const char *key);

/*
 * @brief: 关闭block，关闭后的block，除了打开操作外，无法进行其他操作。
 * @input: h block句柄
 * @note:  保证线程安全
 */
void ant_iot_kv_close(const void *handle);

// ant_s32 ant_kv_erase_iot_data(void);

// ant_s32 ant_kv_erase_ant_data(void);
    
#ifdef __cplusplus
} // extern "C"
#endif

#endif

