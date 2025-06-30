#ifndef E8C7D6BC_7912_4170_8E0B_CC237ACD85F7
#define E8C7D6BC_7912_4170_8E0B_CC237ACD85F7

#include "fibo_opencpu.h"

// Enum for PDP IP types: IPv4, IPv6, or both.
typedef enum
{
    APP_PDP_IPTYPE_IPV4 = 1,
    APP_PDP_IPTYPE_IPV6,
    APP_PDP_IPTYPE_IPV4V6
}app_pdp_iptype_t;

// Enum for PDP authentication protocols: None, PAP, or CHAP.
typedef enum
{
    APP_PDP_AUTHPROTO_NONE = 0,
    APP_PDP_AUTHPROTO_PAP,
    APP_PDP_AUTHPROTO_CHAP
}app_pdp_authproto_t;

/**
* Initializes the PDP context.
* 
* @return int Status code.
 */
int app_pdp_init();

/**
* Activates a PDP context.
* 
* @param simid SIM card identifier.
* @param cid Context identifier.
* @param iptype IP type (IPv4, IPv6, or both).
* @param apn Access Point Name.
* @param apnuser APN username.
* @param apnpasswd APN password.
* @param auth_proto Authentication protocol (None, PAP, CHAP).
* @return int Status code.
 */
int app_pdp_active(int simid, int cid, app_pdp_iptype_t iptype, char *apn, char *apnuser, char *apnpasswd, app_pdp_authproto_t auth_proto);

/**
* Deactivates a PDP context.
* 
* @param cid Context identifier.
* @return int Status code.
 */
int app_pdp_deactive(int cid);

/**
* Retrieves the IP address associated with a PDP context.
* 
* @param cid Context identifier.
* @param ipv4 Pointer to store the IPv4 address.
* @param ipv6 Pointer to store the IPv6 address.
* @return int Status code.
 */
int app_pdp_get_ipaddress(int cid, char **ipv4, char **ipv6);

/**
* Gets the RRC (Radio Resource Control) status: 0 for not connected, 1 for connected.
* 
* @return int RRC status.
 */
int app_pdp_get_rrc_status();

/**
* Waits for the RRC to become idle.
* 
* @return int Status code.
 */
int app_pdp_wait_rrc_idle();

/**
* Checks if the PDP context has been released.
* 
* @param cid Context identifier.
* @return int Status code.
 */
int app_pdp_check_pdp_release(int cid);

#endif /* E8C7D6BC_7912_4170_8E0B_CC237ACD85F7 */
