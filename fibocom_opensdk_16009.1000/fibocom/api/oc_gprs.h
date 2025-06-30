#ifndef _OC_GPRS_H_
#define _OC_GPRS_H_

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "fibo_opencpu_comm.h"
#include "lwip/netdb.h"
#include "lwip/ip_addr.h"
#include "lwip/sockets.h"

#define FIBO_PDPTYPE_MAX_LEN 32
#define FIBO_APN_MAX_LEN 99
#define FIBO_APN_USER_MAX_LEN 64
#define FIBO_APN_PWD_MAX_LEN 64
#define FIBO_PDP_DNS_NUM 2
#define FIBO_PDP_ADDR_MAX_LEN 21

typedef struct pdp_profile_s
{
    INT8 cid;
    UINT8 nPdpType[FIBO_PDPTYPE_MAX_LEN];
    UINT8 apn[FIBO_APN_MAX_LEN];
    UINT8 apnUserSize;
    UINT8 apnPwdSize;
    UINT8 apnUser[FIBO_APN_USER_MAX_LEN];
    UINT8 apnPwd[FIBO_APN_PWD_MAX_LEN];
    UINT8 nAuthProt;
} fibo_pdp_profile_t;


//for old API
typedef struct pdp_profile
{
    INT8 active_state;
    INT8 cid;
    UINT8 nPdpType[FIBO_APN_MAX_LEN];
    UINT8 apn[FIBO_APN_MAX_LEN];
    INT8 d_comp;
    INT8 h_comp;
    INT8 pdpDns[FIBO_APN_MAX_LEN * FIBO_PDP_DNS_NUM];
    UINT8 pdpAddrSize;
    UINT8 apnUserSize;
    UINT8 apnPwdSize;
    UINT8 apnUser[FIBO_APN_USER_MAX_LEN];
    UINT8 apnPwd[FIBO_APN_PWD_MAX_LEN];
    UINT8 pdpAddr[FIBO_PDP_ADDR_MAX_LEN];
    UINT8 nAuthProt;
} pdp_profile_t;

typedef struct
{
    INT8 value; ///< integer value
    char *str;  ///< string value
} opencpuValueStrMap_t;

typedef struct
{
    uint32_t tac;
    uint32_t cell_id;
} lte_scell_info_t;

typedef struct
{
    uint32_t lac;
    uint32_t cell_id;
} gsm_scell_info_t;

typedef struct
{
    uint8_t curr_rat;
    uint8_t nStatus;
    lte_scell_info_t lte_scell_info;
    gsm_scell_info_t gsm_scell_info;
} reg_info_t;

typedef struct
{
    uint8_t operator_id[6];
    uint8_t *operator_name;
    uint8_t *oper_short_name;
} operator_info_t;

typedef enum
{
    FIBO_E_DEFAULT = 0,
    FIBO_E_CONNECT_ID1,
    FIBO_E_CONNECT_ID2,
    FIBO_E_CONNECT_ID3,
    FIBO_E_CONNECT_ID4,
    FIBO_E_CONNECT_ID5 = 5,
    FIBO_E_CONNECT_ID6,
    FIBO_E_FTP,
    FIBO_E_HTTP,
    FIBO_E_MQTT1,
    FIBO_E_MQTT2 = 10,
    FIBO_E_LBS,
    FIBO_E_NTP,
    FIBO_E_DNS,
    FIBO_E_PING,
    FIBO_E_PRO_MAX
} FIBO_PROTOCOL_E_ID;

#define FIBO_SUPPORT_PROTOCOL_MAX_COUNT 16

INT32 fibo_pdp_active(UINT8 active, fibo_pdp_profile_t *pdp_profile, uint8_t nSimID);
INT32 fibo_pdp_release(UINT8 deactive, INT32 cid, CFW_SIM_ID nSimID);
INT32 fibo_pdp_status_get(INT8 cid, UINT8 *ip, UINT8 *cid_status, uint8_t nSimID);
INT32 fibo_initial_pdp_set(INT8 set_delete, uint8_t pdptype, char *apn, uint8_t nAuthProt,char *username,char *pwd, uint8_t nSim);
INT32 fibo_reg_info_get(reg_info_t *reg_info, uint8_t nSimID);
#ifdef CONFIG_FIBOCOM_XIAONIU_FEATURE
INT32 fibo_getRegInfo(reg_info_t *reg_info, uint8_t nSimID);
#endif


INT32 fibo_get_ip_info_by_cid(uint8_t cid, uint8_t *ip_type, struct sockaddr_in *ip_addr, struct sockaddr_in6 *ip6_addr);
INT32 fibo_multi_pdn_cfg(FIBO_PROTOCOL_E_ID protocol_id, uint8_t cid, uint8_t simid);

//old API
INT32 fibo_PDPActive(UINT8 active, UINT8 *apn, UINT8 *username, UINT8 *password, UINT8 nAuthProt, CFW_SIM_ID nSimID, UINT8 *ip);
INT32 fibo_asyn_PDPActive(UINT8 active, pdp_profile_t *pdp_profile, CFW_SIM_ID nSimID);
INT32 fibo_PDPRelease(UINT8 deactive, CFW_SIM_ID nSimID);
INT32 fibo_asyn_PDPRelease(UINT8 deactive, INT32 cid, CFW_SIM_ID nSimID);
INT32 fibo_PDPStatus(INT8 cid, UINT8 *ip, UINT8 *cid_status,CFW_SIM_ID nSimID);
INT32 fibo_set_initial_pdp(INT8 set_delete, uint8_t pdptype, char *apn, uint8_t nAuthProt,char *username,char *pwd,CFW_SIM_ID nSim);
INT32 fibo_initial_pdp_set_reattach(INT8 set_delete, uint8_t pdptype, char *apn, uint8_t nAuthProt,char *username,char *pwd,CFW_SIM_ID nSim);
INT32 fibo_pdp_remove(uint8_t simid, int cid);
INT32 fibo_get_operator_info(operator_info_t *operator_info,uint8_t simid);
#endif

