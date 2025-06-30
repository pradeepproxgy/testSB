#ifndef E8C7D6BC_7912_4170_8E0B_CC237ACD85F7
#define E8C7D6BC_7912_4170_8E0B_CC237ACD85F7

#include "app.h"

typedef enum
{
    APP_PDP_IPTYPE_IPV4 = 1,
    APP_PDP_IPTYPE_IPV6,
    APP_PDP_IPTYPE_IPV4V6
}app_pdp_iptype_t;

///0 ：None； 1 ：PAP； 2 ：CHAP
typedef enum
{
    APP_PDP_AUTHPROTO_NONE = 0,
    APP_PDP_AUTHPROTO_PAP,
    APP_PDP_AUTHPROTO_CHAP
}app_pdp_authproto_t;

/**
* @brief 
* 
* @return int 
 */
int app_pdp_init();

/**
* @brief 
* 
* @param simid 
* @param cid 
* @param iptype 
* @param apn 
* @param apnuser 
* @param apnpasswd 
* @param auth_proto 
* @return int 
 */
int app_pdp_active(int simid, int cid, app_pdp_iptype_t iptype, char *apn, char *apnuser, char *apnpasswd, app_pdp_authproto_t auth_proto);


/**
* @brief 
* 
* @return int 
 */
int app_pdp_deactive(int cid);

/**
* @brief 
* 
* @param cid 
* @param ipv4 
* @param ipv6 
* @return int 
 */
int app_pdp_get_ipaddress(int cid, char **ipv4, char **ipv6);

/**
* @brief 获取rrc的状态，0 未连接状态，1：连接状态
* 
 */
int app_pdp_get_rrc_status();

/**
* @brief 
* 
* @return int 
 */
int app_pdp_wait_rrc_idle();

/**
* @brief 
* 
* @param cid 
* @return int 
 */
int app_pdp_check_pdp_release(int cid);

#endif /* E8C7D6BC_7912_4170_8E0B_CC237ACD85F7 */
