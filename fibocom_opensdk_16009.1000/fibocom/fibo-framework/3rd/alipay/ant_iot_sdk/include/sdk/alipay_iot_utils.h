#ifndef __H_ALIPAY_IOT_UTILS
#define __H_ALIPAY_IOT_UTILS
#include "alipay_iot_kal.h"

/*
 *gzip压缩初始化
 *gzip_handler  : 初始化成功后输出句柄
 *total_size    : 需要压缩的数据总长度
 *return        : 0为初始化成功，其他值为失败
*/
int alipay_iot_gzip_init(void **gzip_handler, int total_size);

/*
 *gzip流式压缩，在不超过alipay_iot_gzip_init指定的total_size时，可以进行连续的压缩
 *gzip_handler  : alipay_iot_gzip_init输出句柄
 *data_origin   : 本次需要压缩的输入数据，需调用方在本接口返回后自行释放
 *in_len        : 本次压缩的输入数据长度
 *return        : 0为本次压缩成功，其他值为失败
*/
int alipay_iot_gzip_compress(void *gzip_handler, void *data_origin, int in_len);

/*
 *gzip流式压缩，通过本接口获取压缩后的数据，多次compress后，通过一次调用本接口获取总体的压缩结果
 只要init返回成功，则无论compress是否成功，都应该调用end。在调用end后请勿再操作gzip_handler
 *gzip_handler  : alipay_iot_gzip_init输出句柄
 *data_out      : 流式压缩的结果总输出数据，需调用方在使用后进行释放
 *out_len       : 流式压缩后结果的长度
 *return        : 0为本次压缩成功，其他值为失败
*/
int alipay_iot_gzip_end(void *gzip_handler, void **data_out, int *out_len);
#endif
