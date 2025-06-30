#ifndef FF4B714E_9134_6470_EC1F_3FDB090E75F1
#define FF4B714E_9134_6470_EC1F_3FDB090E75F1

/**
 * @file cbb_ate_resp_plat.h
 * @author sundaqing (sundaqing@fibocom.com)
 * @brief 此文件中所有的接口都依赖平台的AT引擎，主要的接口包括以下：
 *        1. cbb_ate_info_text 发送infomation response text(字符串) (受S3 S4 atv影响)
 *        1.0 cbb_ate_info_data 发送infomation response text(任意数据) (不受S3 S4 atv影响)
 *        1.1 cbb_ate_info_code 发送中间处理结果码
 *        1.2 cbb_ate_result_code 发送最终处理结果码
 *        1.3 cbb_ate_result_text 发送最终处理自定义信息
 *        1.4 cbb_ate_cme_code 发送cme错误码
 *        2. cbb_ate_urc_text 发送URC到TE或者对端，也是以S3 S4结尾
 *        3. cbb_ate_urc_code 发送URC结果码到TE，错误码根据各个平台自己定义
 *        4. cbb_ate_enter_odm 进入odm模式，可以通过+++退出odm模式，如果psudo为false，可以通过ATO重新进入
 *        5. cbb_ate_odm_send_data 在ODM模式下发送数据
 *        6. cbb_ate_exit_odm 主动退出ODM模式
 *        7. cbb_ate_get_baud 如果是串口，则可以获取串口的波特率
 *        
 *        处理AT命令，AT命令应该遵循如下规则：
 *        
 *        1. AT需要在发送最终结果码之前发送infomation response, 则调用 cbb_ate_info_text(受S3 S4 atv影响) 或者 cbb_ate_info_data(不受S3 S4 atv影响)
 *        2. AT需要在发送最终结果码之前发送中间处理结果码(如RING, CONNECT), 则调用 cbb_ate_info_code
 *        3. AT处理完毕，则发送最终结果码或者信息
 *           3.1 如果发送标准的结果码（如OK ERROR, 受atv影响），调用 cbb_ate_result_code
 *           3.2 如果发送自定义的最终结果字符串，调用 cbb_ate_result_text
 *           3.3 如果发送cme错误码 （受AT+CMEE影响），调用 cbb_ate_cme_code
 *           Note： 以上三个api不能同时调用
 *        4. 3描述的三个API调用完毕之后，如果需要发送回应，在只能调用
 *           4.1 cbb_ate_urc_text 发送URC信息（字符串，不受atv s3 s4影响）
 *           4.2 cbb_ate_urc_code（受atv，s3 s4影响） 发送URC 结果码
 *           4.3 cbb_ate_urc_data 发送任意数据
 * @version 0.1
 * @date 2021-08-10
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <stdint.h>
#include <ffw_utils.h>
#include <ffw_os.h>
#include <cbb_ate_plat.h>

#define CBB_AT_RC_OK 0         ///< "OK"
#define CBB_AT_RC_CONNECT 1    ///< "CONNECT"
#define CBB_AT_RC_RING 2       ///< "RING/CRING"
#define CBB_AT_RC_NOCARRIER 3  ///< "NO CARRIER"
#define CBB_AT_RC_ERROR 4      ///< "ERROR"
#define CBB_AT_RC_NOTSUPPORT 5 ///< "NOT SUPPORT"
#define CBB_AT_RC_NODIALTONE 6 ///< "NO DIALTONE"
#define CBB_AT_RC_BUSY 7       ///< "BUSY"
#define CBB_AT_RC_NOANSWER 8   ///< "NO ANSWER"
#define CBB_AT_RC_INVCMDLINE 9 ///< "INVALID COMMAND LINE"
#define CBB_AT_RC_CR 10        ///< "\r\n"
#define CBB_AT_RC_SIMDROP 11   ///< "Sim drop"

#define CBB_BYPASS_FIN_ALL_DATA (0)
#define CBB_BYPASS_FIN_CTRL_Z (1)
#define CBB_BYPASS_FIN_TIMEOUT (2)
#define CBB_BYPASS_FIN_ESC (3)

#define CBB_ODM_EVT_ENTER (0)
#define CBB_ODM_EVT_EXIT_TMP (1)
#define CBB_ODM_EVT_REENTER (2)
#define CBB_ODM_EVT_EXIT (3)

typedef struct
{
    const void *data;
    int32_t len;
} cbb_data_attr_t;

typedef void (*ffw_odm_handler_t)(ffw_handle_t engine, int32_t evt, void *arg);
typedef int32_t (*ffw_bypass_data_t)(ffw_handle_t param, const void *data, int32_t size);

/**
* @brief 发送INFO TEXT，在没有处理完毕之前的信息
* 
* @param e AT引擎的句柄
* @param text 数据
* @param len 数据长度
* @return int32_t
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int32_t cbb_ate_info_text(ffw_handle_t e, const void *text, int32_t len);

/**
* @brief 发送INFO DATA, 原始数据发送，不受s3 s4和atv影响
* 
* @param e AT引擎的句柄
* @param code 错误码
* @return int32_t
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int32_t cbb_ate_info_data(ffw_handle_t e, const void *data, int32_t len);

/**
* @brief 发送中间处理结果码
* 
* @param e AT引擎的句柄
* @param code 错误码
* @return int32_t
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int32_t cbb_ate_info_code(ffw_handle_t e, int code);

/**
 * @brief AT命令处理完毕，发送自定义的错误信息
 * 
 * @param e  AT引擎
 * @param code 
 * @return int32_t 
 */
int32_t cbb_ate_result_text(ffw_handle_t e, const char *text);

int32_t cbb_resp_finish_1(ffw_handle_t engine);


/**
 * @brief AT命令处理完毕，发送结果码
 * 
 * @param e AT引擎
 * @param code 
 * @return int32_t 
 */
int32_t cbb_ate_result_code(ffw_handle_t e, int code);

/**
 * @brief AT命令处理完毕，发送CMEE错误码
 * 
 * @param e AT引擎
 * @param code 错误码
 * @return int32_t 
 */
int32_t cbb_ate_cme_code(ffw_handle_t e, int code);

/**
 * @brief 获取+CMEE的配置值
 * 
 * @param e AT引擎
 * @param code 错误码
 * @return int32_t 
 */
int32_t cbb_ate_get_cmee(ffw_handle_t e);

/**
* @brief 以URC的发送原始数据，不会添加任务回车换行的字符
* 
* @param e AT引擎的句柄, 注意 @c e 可以为NULL, 表示向所有AT口发送URC消息
* @param text 数据
* @param len 数据长度
* @return int32_t 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int32_t cbb_ate_urc_data(ffw_handle_t e, const void *data, int32_t len);

/**
 * @brief 发送URC字符串
 * 
 * @param e AT引擎的句柄，注意 @c e 可以为NULL, 表示向所有AT口发送URC消息
 * @param text 
 * @param len 
 * @return int32_t 
 */
int32_t cbb_ate_urc_text(ffw_handle_t e, const void *text, int32_t len);

/**
* @brief 以URC的方式发送AT结果码, 此结果会根据atv的配置决定是发送数字还是字符串
* 
* @param e AT引擎的句柄，注意 @c e 可以为NULL, 表示向所有AT口发送URC消息
* @param code 错误码
* @return int32_t
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int32_t cbb_ate_urc_code(ffw_handle_t e, int code);

/**
* @brief AT引擎进入ODM模式
* 
* @param e AT引擎的句柄
* @param psudo 是否支持使用ATO重新进入ODM模式，true为不支持，false为支持
* @param ctrl_cb 事件回调函数
* @param data_cb 数据回调函数
* @param arg 回调上下文
* @return int32_t
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int32_t cbb_ate_enter_odm(ffw_handle_t e, bool psudo, ffw_odm_handler_t ctrl_cb, ffw_bypass_data_t data_cb, void *arg);

/**
* @brief AT引擎进入ODM模式,针对电力的接口
* 
* @param e AT引擎的句柄
* @param psudo 是否支持使用ATO重新进入ODM模式，true为不支持，false为支持
* @param ctrl_cb 事件回调函数
* @param data_cb 数据回调函数
* @param arg 回调上下文
* @return int32_t
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int32_t cbb_ate_enter_odm_ele(ffw_handle_t e, bool psudo, ffw_odm_handler_t ctrl_cb, ffw_bypass_data_t data_cb, void *arg);

/**
* @brief 在ODM模式下，发送数据到AT引擎
* 
* @param e AT引擎的句柄
* @param text 数据
* @param len 数据长度
* @return int32_t 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int32_t cbb_ate_odm_send_data(ffw_handle_t e, const void *text, int32_t len);

/**
* @brief 退出ODM模式
* 
* @param e AT引擎的句柄
* @return int32_t 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int32_t cbb_ate_exit_odm(ffw_handle_t e);

/**
 * @brief 平台进入透传模式
 * 
 * @param e 
 * @param cb 
 * @param arg 
 * @param prompt 
 * @return int32_t 
 */
int32_t cbb_ate_enter_bypass_plat(ffw_handle_t e, ffw_bypass_data_t cb, void *arg, const char *prompt);

/**
* @brief 退出透传模式
* 
* @param e AT引擎的句柄
* @return int32_t 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int32_t cbb_ate_exit_bypass_plat(ffw_handle_t e);

/**
* @brief 解析字符串参数
* 
* @param p AT命令参数类型
* @param idx 参数索引
* @param[out] p_out 解析到的字符串 
* @return int32_t 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int32_t cbb_ate_parse_str(ffw_handle_t p, int32_t idx, const char **p_out);

/**
 * @brief 区别于 @c cbb_ate_parse_str 函数，如果字符串参数没有用双引号括起来，用这个函数解析
 * 
 * @param p AT命令参数类型
 * @param idx 参数索引
 * @param p_out 解析到的字符串
 * @return int32_t 
 */
int32_t cbb_ate_parse_str_raw(void *p, int32_t idx, const char **p_out);

/**
* @brief 解析整型串参数
* 
* @param p AT命令参数类型
* @param idx  参数索引
* @return int32_t 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int32_t cbb_ate_parse_int(ffw_handle_t p, int32_t idx, int32_t *out);

/**
* @brief 解析hex数据
* 
* @param p AT命令参数类型
* @param idx  参数索引
* @return int32_t 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int32_t cbb_ate_parse_hex_raw(void *p, int32_t idx, uint32_t len, const char **p_out);

/**
* @brief 解析float数据
* 
* @param p AT命令参数类型
* @param idx  参数索引
* @return int32_t 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int32_t cbb_ate_parse_float(void *p, int32_t idx, double *p_out);

/**
* @brief 解析int64参数
* 
* @param p AT命令参数类型
* @param idx  参数索引
* @return int64_t 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int32_t cbb_ate_parse_int64(void *p, int32_t idx, int64_t *out);

/**
* @brief 解析无符号整型串参数
* 
* @param p AT命令参数类型
* @param idx  参数索引
* @return int32_t 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int32_t cbb_ate_parse_uint(ffw_handle_t p, int32_t idx, uint32_t *out);

/**
 * @brief 获取当前串口的波特率
 * 
 * @param e AT引擎
 * @return int32_t 波特率
 */
int32_t cbb_ate_get_baud(ffw_handle_t e);

/**
 * @brief 获取ATE命令设置的值
 * 
 * @param e AT引擎
 * @return int ATE的值
 */
bool cbb_ate_get_echo(ffw_handle_t e);

/**
 * @brief 获取ATV命令设置的值
 * 
 * @param e AT引擎
 * @return int ATE的值
 */
bool cbb_ate_get_atv(ffw_handle_t e);


#endif /* FF4B714E_9134_6470_EC1F_3FDB090E75F1 */
