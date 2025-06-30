#ifndef EFFE4854_9134_6471_CBE1_54A18D753F24
#define EFFE4854_9134_6471_CBE1_54A18D753F24

#include <stdint.h>
#include <ffw_utils.h>
#include <ffw_os.h>

#include <cbb_ate_resp_plat.h>

/**
 * @brief 
 * 
 * @param e 
 * @param fmt 
 * @param ... 
 * @return int32_t 
 */
#ifdef __LINUX__
int32_t cbb_ate_send_urc_fmt(ffw_handle_t e, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
#else
int32_t cbb_ate_send_urc_fmt(ffw_handle_t e, const char *fmt, ...);
#endif

/**
 * @brief 
 * 
 * @param e 
 * @param fmt 
 * @param ... 
 * @return int32_t 
 */
int32_t cbb_ate_send_urc_fmt1(ffw_handle_t engine, const char *fmt, ...);

/**
 * @brief 格式化的方式发送数据，并在后面追加AT配置s3,s4（一般为\r\n）
 * 
 * @param e AT引擎的句柄
 * @param fmt 格式化字符串
 * @param ... 可变参数
 * @return int32_t
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 */

#ifdef __LINUX__
int32_t cbb_ate_info_text_fmt(ffw_handle_t e, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
#else
int32_t cbb_ate_info_text_fmt(ffw_handle_t e, const char *fmt, ...);
#endif

/**
 * @brief 处理中间自定义信息上报，无回车换行
 * 
 * @param engine 
 * @param fmt 
 * @param ... 
 * @return int32_t 
 */
int32_t cbb_ate_info_data_fmt(ffw_handle_t engine, const char *fmt, ...);

/**
 * @brief 格式化的方式发送信息，无回车换行
 * 
 * @param engine AT引擎的句柄
 * @param fmt 格式化字符串
 * @param ... 可变参数
 * @return int32_t 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 */
int32_t cbb_ate_result_data_fmt(ffw_handle_t engine, const char *fmt, ...);

/**
 * @brief 格式化的方式发送错误信息
 * 
 * @param engine AT引擎的句柄
 * @param fmt 格式化字符串
 * @param ... 可变参数
 * @return int32_t 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 */
#ifdef __LINUX__
int32_t cbb_ate_result_text_fmt(ffw_handle_t engine, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
#else
int32_t cbb_ate_result_text_fmt(ffw_handle_t engine, const char *fmt, ...);
#endif

/**
* @brief 解析字符串参数,如果 @c idx 大于 @c count 也返回成功，默认值由调用者设置
* 
* @param p AT命令参数类型
* @param idx 参数索引
* @param count 参数总数
* @param[out] p_out 解析到的字符串 
* @param len_out 解析到的字符串长度
* @return int32_t 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int32_t cbb_ate_try_parse_str(ffw_handle_t p, int32_t idx, int32_t count, const char **p_out);

/**
* @brief 解析字符串参数,如果 @c idx 大于 @c count 也返回成功，默认值由调用者设置，字符串可以不带引号
* 
* @param p AT命令参数类型
* @param idx 参数索引
* @param count 参数总数
* @param[out] p_out 解析到的字符串 
* @param len_out 解析到的字符串长度
* @return int32_t 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int32_t cbb_ate_try_parse_raw_str(ffw_handle_t p, int32_t idx, int32_t count, const char **p_out);

/**
* @brief cbb_ate_try_parse_raw_str_range
* 
* @param p AT命令参数类型
* @param idx 参数索引
* @param count 参数总数
* @param[out] p_out 解析到的字符串 
* @param len_out 解析到的字符串长度
* @return int32_t 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int32_t cbb_ate_try_parse_raw_str_range(ffw_handle_t p, int32_t idx, int32_t count, const char **p_out, int32_t minlen, int32_t maxlen);

/**
* @brief 解析整型参数，如果 @c idx 大于 @c count 也返回成功，默认值由调用者设置
* 
* @param p AT命令参数类型
* @param idx  参数索引
* @param count 参数总数
* @param[out] out 解析到的数字 
* @return int32_t 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int32_t cbb_ate_try_parse_int(ffw_handle_t p, int32_t idx, int32_t count, int32_t *out);

/**
* @brief 解析整型参数，如果 @c idx 大于 @c count 也返回成功，默认值由调用者设置
* 
* @param p AT命令参数类型
* @param idx  参数索引
* @param count 参数总数
* @param[out] out 解析到的数字 
* @return int32_t 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int32_t cbb_ate_try_parse_uint(ffw_handle_t p, int32_t idx, int32_t count, uint32_t *out);

/**
* @brief 解析字符串参数，判断字符串的大小长度和最大长度
* 
* @param p AT命令参数类型
* @param idx  参数索引
* @param count 参数总数
* @param[out] out 解析到的数字 
* @param minlen 最小长度
* @param maxlen 最大长度
* @return int32_t 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int32_t cbb_ate_parse_str_range(ffw_handle_t p, int32_t idx, int32_t count, const char **p_out, int32_t minlen, int32_t maxlen);

/**
* @brief 解析字符串参数，判断字符串的大小长度和最大长度，字符串可以不带引号
* 
* @param p AT命令参数类型
* @param idx  参数索引
* @param count 参数总数
* @param[out] out 解析到的数字 
* @param minlen 最小长度
* @param maxlen 最大长度
* @return int32_t 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int32_t cbb_ate_parse_raw_str_range(ffw_handle_t p, int32_t idx, int32_t count, const char **p_out, int32_t minlen, int32_t maxlen);

/**
* @brief 解析整型参数，判断最大值和最小值
* 
* @param p AT命令参数类型
* @param idx  参数索引
* @param count 参数总数
* @param[out] out 解析到的数字 
* @param minlen 最小长度
* @param maxlen 最大长度
* @return int32_t 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int32_t cbb_ate_parse_int_range(ffw_handle_t p, int32_t idx, int32_t count, int32_t *out, int32_t minlen, int32_t maxlen);

/**
* @brief 解析float参数，判断最大值和最小值
* 
* @param p AT命令参数类型
* @param idx  参数索引
* @param count 参数总数
* @param[out] out 解析到的数字 
* @param minlen 最小长度
* @param maxlen 最大长度
* @return int32_t 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int32_t cbb_ate_parse_float_range(ffw_handle_t p, int32_t idx, int32_t count, double *out, double minlen, double maxlen);

/**
* @brief 解析整型参数，判断最大值和最小值
* 
* @param p AT命令参数类型
* @param idx  参数索引
* @param count 参数总数
* @param[out] out 解析到的数字 
* @param minlen 最小长度
* @param maxlen 最大长度
* @return int32_t 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int32_t cbb_ate_parse_uint_range(ffw_handle_t p, int32_t idx, int32_t count, uint32_t *out, uint32_t minlen, uint32_t maxlen);

/**
* @brief 解析字符串参数，判断字符串的大小长度和最大长度, 如果 @c idx 大于 @c count 也返回成功，默认值由调用者设置
* 
* @param p AT命令参数类型
* @param idx  参数索引
* @param count 参数总数
* @param[out] out 解析到的数字 
* @param minlen 最小长度
* @param maxlen 最大长度
* @return int32_t 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int32_t cbb_ate_try_parse_str_range(ffw_handle_t p, int32_t idx, int32_t count, const char **p_out, int32_t minlen, int32_t maxlen);

/**
* @brief 解析整型参数，判断最大值和最小值, 如果 @c idx 大于 @c count 也返回成功，默认值由调用者设置
* 
* @param p AT命令参数类型
* @param idx  参数索引
* @param count 参数总数
* @param[out] out 解析到的数字 
* @param minlen 最小长度
* @param maxlen 最大长度
* @return int32_t 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int32_t cbb_ate_try_parse_int_range(ffw_handle_t p, int32_t idx, int32_t count, int32_t *out, int32_t minlen, int32_t maxlen);


/**
* @brief 解析无符号整型参数，判断最大值和最小值, 如果 @c idx 大于 @c count 也返回成功，默认值由调用者设置
* 
* @param p AT命令参数类型
* @param idx  参数索引
* @param count 参数总数
* @param[out] out 解析到的数字 
* @param minlen 最小长度
* @param maxlen 最大长度
* @return int32_t 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int32_t cbb_ate_try_parse_uint_range(ffw_handle_t p, int32_t idx, int32_t count, uint32_t *out, uint32_t minlen, uint32_t maxlen);

/**
 * @brief AT引擎进入透传模式，Ctrl+z退出
 * 
 * @param e AT引擎的句柄
 * @param total_len 进入透传模式需要输入的数据的长度
 * @param timeout 没有数据输入时，退出透传模式的超时时间，单位ms
 * @param data_cb 数据回调函数
 * @param fin 退出透传模式回调函数
 * @param arg 回调上下文
 * @param prompt 进入透传模式的提示符，如果为NULL,表示没有提示符
 * @param ctrlz 是否支持ctrlz结束
 * @param esc 是否支持esc取消
 * @param echo 是否回显，需要和ATE一起判断，两者都为true才回显
 * @return int32_t 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 */
int32_t cbb_ate_enter_bypass(ffw_handle_t e, int32_t total_len, int32_t timeout, void (*data_cb)(void *e, void *param, const void *data, int32_t size),
                             void (*fin)(void *e, int32_t cause, void *arg), void *arg, const char *prompt, bool ctrlz, bool esc, bool echo);

/**
 * @brief 退出透传模式
 * 
 * @param AT引擎的句柄 
 * @return int32_t 
 */
int32_t cbb_ate_exit_bypass(ffw_handle_t e);

/**
 * @brief 错误码转换为字符串
 * 
 * @param code 错误码
 * @return const char* 
 */
const char *cbb_ate_get_code_str(ffw_handle_t e, int code);

/**
 * @brief AT命令处理完毕，发送CMEE错误码
 * 
 * @param e AT引擎
 * @param code 错误码
 * @return int32_t 
 */
int32_t cbb_ate_cme_code(ffw_handle_t e, int code);

#endif /* EFFE4854_9134_6471_CBE1_54A18D753F24 */
