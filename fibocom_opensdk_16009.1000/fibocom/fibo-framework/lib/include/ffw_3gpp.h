#ifndef A15CEFE1_DADB_4E17_8F4E_4F2D9EEE7B3B
#define A15CEFE1_DADB_4E17_8F4E_4F2D9EEE7B3B

#include "ffw_rat.h"
#include "ffw_network.h"
#include "stdint.h"

/**
* @brief PDP 状态定义
*
 */
typedef enum ffw_pdp_status_s
{
    ///去激活状态
    FFW_PDP_STATUS_DEACTIVE = 0,

    ///激活状态
    FFW_PDP_STATUS_ACTIVE,

    ///正在激活状态
    FFW_PDP_STATUS_ACTIVE_ING,

    ///deact ing
    FFW_PDP_STATUS_DEACTIVE_ING,

    ///deattach
    FFW_PDP_STATUS_DEATTACH_ING,

    ///deattach
    FFW_PDP_STATUS_ATTACH_ING,

    ///deattach for pdp active default
    FFW_PDP_STATUS_PDP_ACTIVE_DEATTACH_ING,

    ///attach for pdp active default
    FFW_PDP_STATUS_PDP_ACTIVE_ATTACH_ING,
    
    ///激活状态,但是没有拿到IPV6地址
    FFW_PDP_STATUS_ACTIVE_WITHOUT_IPV6,

    /// 去激活没有IPV6的情况下
    FFW_PDP_STATUS_DEACTIVE_ING_WITHOUT_IPV6,
} ffw_pdp_status_t;

/**
 * @brief The callback function type for pdp active
 *
 * @param cid The ID of pdp active 
 * @param status The result of pdp active, see @c ffw_pdp_status_t
 * @param ipv4 The ipv4 address if pdp acitve ipv4 protocol
 * @param ipv6 The ipv6 address if pdp acitve ipv6 protocol
 * @param arg The context of callback
 */
typedef void (*ffw_pdp_callback_t)(int cid, ffw_pdp_status_t status, const char *ipv4, const char *ipv6, void *arg);

/**
* @brief 
* 
* @param simid 
* @param profile 
* @param cb 
* @param arg 
* @return int 
 */
int ffw_rat_pdp_context_active(int8_t simid, ffw_pdp_profile_t *profile, ffw_pdp_callback_t cb, void *arg);


/**
* @brief 
* 
* @param simid 
* @param cid 
* @param cb 
* @param arg 
* @return int 
 */
int ffw_rat_pdp_context_deactive(uint8_t simid, uint8_t cid, ffw_pdp_callback_t cb, void *arg);

/**
* @brief 
* 
* @return int 
 */
int ffw_rat_pdp_active_num(void);

/**
* @brief 
* 
* @param simid 
* @param cb 
* @param arg 
* @return int 
 */
int ffw_rat_pdp_deactive_all(uint8_t simid, void (*cb)(int result, void *arg), void *arg);


/**
 * @brief 判断pdp是否激活
 * 
 * @param simid sim 卡
 * @param cid cid
 * @param with_default 如果当前cid没有激活，是否检查默认的cid
 * @return true 激活
 * @return false 非激活
 */
bool ffw_rat_pdp_is_active(int simid, int cid, bool with_default);

/**
* @brief
*
* @return int
 */
int ffw_rat_pdp_init(void);

/**
 * @brief 根据sim和cid获取网卡ID，判断当前cid的pdp是否激活，
 *        如果为激活状态，则 @c netif 设置为 @c cid，并返回 @c FFW_R_SUCCESS
 *        如果为非激活状态
 *          如果 @c with_default 为 false，直接返回 @c FFW_R_FAILED
 *          如果 @c with_default 为 true ，需要找到任意一个激活的cid，将 @c netif 设置为 该 @c cid，并返回 @c FFW_R_SUCCESS
 *          如果没有找到，则返回 @c FFW_R_FAILED
 * 
 * @param simid SIM卡ID
 * @param cid CID
 * @param[out] netif 网卡ID 
 * @param with_default 为true时，如果没要找到对应的netif，则返回默认的netif
 * @return int 
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 */
int ffw_get_netif(int simid, int cid, ffw_handle_t *netif, bool with_default);

/**
* @brief 获取网卡的地址，用于监听或者绑定网卡使用，如果 @c ifac 等于 ffw_loop_netif 返回值，则直接返回127.0.0.1的地址
* 
* @param iface 标记网卡的句柄， @c ffw_get_netif 的返回值，可以解释为cid
* @param family AF_INET: ipv4地址 AF_INET6: ipv6地址 
* @param port 端口，可以为0，表示使用随机端口
* @param laddr 本地地址
* @param laddrlen 本地地址大小
* @return int 
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败 
 */
int ffw_localaddr(ffw_handle_t iface, int family, uint16_t port, struct sockaddr *laddr, int *laddrlen);


/**
* @brief 获取网络分配的DNS服务器地址
* 
* @param ifac 网卡句柄
* @param first_server 主服务器地址
* @param second_server 备服务器地址
* @param ipv4 是否是IPV4
* @return int
* @retval FFW_R_SUCCESS: 成功
* @retval FFW_R_FAILED: 失败
 */
int ffw_get_dns_server(ffw_handle_t ifac, char *first_server, char *second_server, bool ipv4);


/**
 * @brief 获取本地回环地址的网卡 127.0.0.1，除非平台由对应的接口，否则直接返回 0xFF
 * 
 * @return ffw_handle_t 网卡句柄
 */
ffw_handle_t ffw_loop_netif();

/**
 * @brief Get the status of PDP info
 *
 * @param simid sim 卡id
 * @param cid cid
 * @return uint8_t 反回ffw_pdp_status_s的类型
 */
 uint8_t ffw_get_pdp_status(uint8_t simid, int cid);

 /**
  * @brief Get the pdp_type of PDP info
  *
  * @param simid sim 卡id
  * @param cid cid
  * @return uint8_t cid pdp_type
  */
  uint8_t ffw_get_pdp_type(uint8_t simid, int cid);

#endif /* A15CEFE1_DADB_4E17_8F4E_4F2D9EEE7B3B */
