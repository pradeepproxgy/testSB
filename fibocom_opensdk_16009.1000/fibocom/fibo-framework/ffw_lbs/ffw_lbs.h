#ifndef FFW_LBS_FFW_LBS
#define FFW_LBS_FFW_LBS
#include <stdint.h>  // for int32_t
struct ffw_lbs_info_s;

/**
* @file ffw_lbs.h
* @author sundaqing (sundaqing@fibocom.com)
* @brief 实现基站定位功能(LBS)，利用高德amap的api实现，国外可能无法使用
* @version 0.1
* @date 2021-05-06
* 
* Copyright (c) 2021 Fibocom. All rights reserved
* 
 */

/// type of ffw_lbs_info_s
typedef struct ffw_lbs_info_s ffw_lbs_info_t;

/**
* @brief 基站定位结果
* 
 */
struct ffw_lbs_info_s
{
    /// 原始数据
    char *raw_data;

    /// 城市
    char *city;

    /// 省份
    char *province;

    /// poi
    char *poi;

    /// 邮政编码
    char *adcode;

    /// 街道
    char *street;

    /// desc
    char *desc;

    /// 国家
    char *country;

    /// 经纬度
    char *location;

    /// road
    char *road;

    /// radius
    char *radius;

    /// city code
    char *citycode;
};

typedef enum
{
    /// gprs
    FFW_LBS_TYPE_GPRS,

    /// wifi
    FFW_LBS_TYPE_WIFI,
}ffw_lbs_type_t;

/**
* @brief 高德基站定位功能(LBS)
* 
* @param key This amap api key, if key is NULL, will use fibocom api key of amap
* @param access_type reserved， must be 0
* @param imei the imei of device
* @param bts 小区信息
* @param nearbts 邻区信息
* @param[out] lbs_info 获取到的位置信息
* @param timeout 超时时间，单位为毫秒
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
* @retval FFW_R_INVALID_INT_STR: 失败
 */
int ffw_lbs_amap(const char *key, int access_type, const char *imei, const char *bts, const char *nearbts, ffw_lbs_info_t *lbs_info, int32_t timeout);

/**
* @brief 高德wifi定位功能(LBS)
* 
* @param key This amap api key, if key is NULL, will use fibocom api key of amap
* @param access_type reserved， must be 0
* @param imei the imei of device
* @param macs WIFI地址
* @param[out] lbs_info 获取到的位置信息
* @param timeout 超时时间，单位为毫秒
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
* @retval FFW_R_INVALID_INT_STR: 失败
 */
int ffw_wifi_lbs_amap(const char *key, int access_type, const char *imei, const char *macs, ffw_lbs_info_t *lbs_info, int32_t timeout);

/**
* @brief 更高一层基站定位功能，自动获取小区和邻区信息。
* 
* @param key API的鉴权
* @param simid 卡ID
* @param  wifi_gprs  类型
* @param[out] lbs_info 获取到的位置信息
* @param timeout 超时时间，单位为毫秒
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
* @retval FFW_R_INVALID_INT_STR: 失败
 */
int ffw_lbs_easy_amap(const char *key, int simid, ffw_lbs_type_t lbstype, ffw_lbs_info_t *lbs_info, int32_t timeout);

/**
* @brief 异步接口
* 
* @param key 
* @param simid 
* @param  wifi_gprs  1 GPRS 0 WIFI
* @param timeout 
* @param cb 
* @param arg 
* @return int 
 */
int ffw_lbs_easy_amap_async(const char *key, int simid, ffw_lbs_type_t lbstype, int32_t timeout, void (*cb)(int result, ffw_lbs_info_t *lbsinfo, void *arg), void *arg);

#endif /* FFW_LBS_FFW_LBS */
