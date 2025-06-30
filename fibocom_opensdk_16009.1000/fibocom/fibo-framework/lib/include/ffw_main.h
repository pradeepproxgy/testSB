/**
* @file ffw_main.h
* @author sundaqing (sundaqing@fibocom.com)
* @brief ffw lib的入口文件
* @version 0.1
* @date 2021-06-18
* 
* Copyright (c) 2021 Fibocom. All rights reserved
* 
 */

#ifndef LIB_INCLUDE_FFW_MAIN
#define LIB_INCLUDE_FFW_MAIN

/**
* @brief 初始化ffw库，ffw的入库函数
* 
 */
void ffw_init();

/**
 * @brief 异步初始化ffw库，有些平台需要异步初始化
 * 
 * @param cb 完成之后的callback
 */
void ffw_init_async(void (*cb)());

/**
 * @brief 测试各项依赖与平台的功能
 * 
 */
void ffw_test();

/**
 * @brief 初始化GTSET文件
 * 
 */
void ffw_init_gtset();

#endif /* LIB_INCLUDE_FFW_MAIN */
