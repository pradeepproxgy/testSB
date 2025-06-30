#ifndef CA940D85_0E4A_4C7F_B160_12AE0E0E55B0
#define CA940D85_0E4A_4C7F_B160_12AE0E0E55B0

/**
 * @file ffw_lwip.h
 * @author your name (you@domain.com)
 * @brief 为了适配LWIP协议栈抽象的接口
 * @version 0.1
 * @date 2021-08-22
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <stdint.h>

/**
 * @brief dump功能
 * 
 * @param data 
 * @param len 
 * @return int 
 */
int ffw_lwip_data_dump(uint8_t *data, int len);

/**
 * @brief should discard ps data
 * 
 * @return true 
 * @return false 
 */
bool ffw_lwip_getdpsd_flag(uint8_t sim);

/**
 * @brief 
 * 
 * @param ipv4 
 * @param ipv6 
 * @param type 
 * @return int 
 */
int ffw_get_pdp_ipaddress(void *ipv4, void *ipv6, uint8_t *type);

/**
 * @brief 
 * 
 * @param sim 
 * @param cid 
 * @param handle 
 * @param arrive_cb 
 * @param ctx 
 * @return int 
 */
int ffw_drv_open(uint8_t sim, uint8_t cid, void **handle, void (*arrive_cb)(void *ctx, void *handle), void *ctx);

/**
 * @brief 
 * 
 * @param handle 
 * @param data 
 * @param len 
 * @return int 
 */
int ffw_drv_write(void *handle, uint8_t *data, int len);

/**
 * @brief 
 * 
 * @param handle 
 * @param data 
 * @param len 
 * @return int 
 */
int ffw_drv_read(void *handle, uint8_t *data, int len);

/**
 * @brief 
 * 
 * @param handle 
 * @return int 
 */
int ffw_drv_close(void *handle);

/**
 * @brief 是否是测试卡
 * 
 * @param sim 
 * @return true 
 * @return false 
 */
bool ffw_check_test_sim(int sim);

#endif /* CA940D85_0E4A_4C7F_B160_12AE0E0E55B0 */
