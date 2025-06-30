/* Copyright (C) 2016 RDA Technologies Limited and/or its affiliates("RDA").
* All rights reserved.
*
* This software is supplied "AS IS" without any warranties.
* RDA assumes no responsibility or liability for the use of the software,
* conveys no license or title under any patent, copyright, or mask work
* right to the product. RDA reserves the right to make changes in the
* software without notification.  RDA also make no representation or
* warranty that such application will be suitable for the specified use
* without further testing or modification.
*/

#include "cfw.h"
#include "osi_api.h"
#include "osi_log.h"
#include "sockets.h"
#include "lwip/netif.h"
#include "lwip/dns.h"
#include "lwip/tcpip.h"
#include "drv_ps_path.h"
#include "netif_ppp.h"
#include "netmain.h"

#if defined(LTE_NBIOT_SUPPORT) || defined(LTE_SUPPORT)
#define MAX_SIM_ID 2
#if defined(CONFIG_SOC_8811) || defined(CONFIG_SOC_8821)
#define MAX_CID 7
#else
#define MAX_CID 7
#endif

typedef struct T_Netif_Gprs_Backup_Info_Tag
{
    uint8_t cid;
    uint8_t pdnType;
    uint8_t simId;
    ip_addr_t ip6_addr[LWIP_IPV6_NUM_ADDRESSES];
    u8_t ip6_addr_state[LWIP_IPV6_NUM_ADDRESSES];
#if LWIP_IPV6_ADDRESS_LIFETIMES
    u32_t ip6_addr_valid_life[LWIP_IPV6_NUM_ADDRESSES];
    u32_t ip6_addr_pref_life[LWIP_IPV6_NUM_ADDRESSES];
#endif /* LWIP_IPV6_ADDRESS_LIFETIMES */
#if LWIP_IPV6_AUTOCONFIG
    u8_t ip6_autoconfig_enabled;
#endif /* LWIP_IPV6_AUTOCONFIG */
#if LWIP_IPV6_SEND_ROUTER_SOLICIT
    u8_t rs_count;
#endif /* LWIP_IPV6_SEND_ROUTER_SOLICIT */
} T_Netif_Gprs_Backup_Info;

typedef struct T_Netif_Backup_Info_Tag
{
    uint32_t if_info;
    T_Netif_Gprs_Backup_Info gprsNetif[MAX_SIM_ID][MAX_CID];
    ip_addr_t dns_servers[DNS_MAX_SIM][DNS_MAX_CID][DNS_MAX_SERVERS];
} T_Netif_Backup_Info;

#define SIM_CID(sim, cid) ((((sim)&0xf) << 4) | ((cid)&0xf))

extern osiThread_t *netGetTaskID();
extern struct netif *getGprsNetIf(uint8_t nSim, uint8_t nCid);
extern uint32_t *getDNSServer(uint8_t nCid, uint8_t nSimID);
extern void dns_clean_entries(void);

extern err_t data_output_ps_header(struct netif *netif, struct pbuf *p, ip_addr_t *ipaddr);
extern void lwip_pspathDataInput(void *ctx, drvPsIntf_t *p);
extern err_t data_output(struct netif *netif, struct pbuf *p, ip_addr_t *ipaddr);
#ifdef CONFIG_NET_CTWING_SELFREG_SUPPORT
extern void dm_aep_register();
#endif
extern void dm_lwm2m_register();
#ifdef CONFIG_NET_CTIOT_AEP_SUPPORT
extern void ctiot_engine_entry(uint8_t autoLoginFlag);
#endif
#ifdef CONFIG_FIBOCOM_BASE
#if IP_NAT
extern struct wan_netif *newWan(uint8_t nSimId, uint8_t nCid);
bool isPsmNatRestore = false;
extern int8_t rndis_cid;
extern bool is_rndis_call_on;
#endif
#endif

static err_t netif_gprs_psm_init(struct netif *netif)
{
    /* initialize the snmp variables and counters inside the struct netif
   * ifSpeed: no assumption can be made!
   */
    netif->name[0] = 'G';
    netif->name[1] = 'P';
#ifdef CONFIG_NET_LTE_PSPACKET_SUPPORT
#if LWIP_IPV4
    netif->output = (netif_output_fn)data_output_ps_header;
#endif
#if LWIP_IPV6
    netif->output_ip6 = (netif_output_ip6_fn)data_output_ps_header;
#endif
#else
#if LWIP_IPV4
    netif->output = (netif_output_fn)data_output;
#endif
#if LWIP_IPV6
    netif->output_ip6 = (netif_output_ip6_fn)data_output;
#endif
#endif
    netif->mtu = GPRS_MTU;
    if (NULL == netif_default)
    {
        netif_set_default(netif);
    }

    return ERR_OK;
}

extern uint8_t CFW_GprsGetPdpIpv6Addr(uint8_t nCid, uint8_t *nLength, uint8_t *ipv6Addr, CFW_SIM_ID nSimID);
extern uint8_t CFW_GprsGetPdpIpv4Addr(uint8_t nCid, uint8_t *nLength, uint8_t *ipv4Addr, CFW_SIM_ID nSimID);

static void NetifGprsPsmSaveBackupInfo(T_Netif_Gprs_Backup_Info *backupInfo, struct netif *currentNetif)
{
    uint8_t state = 0;
    backupInfo->cid = currentNetif->sim_cid & 0x0F;
    backupInfo->simId = (currentNetif->sim_cid >> 4) & 0x0F;
    state = netif_ip6_addr_state(currentNetif, 0);
    if (IP6_ADDR_VALID & state)
    {
        backupInfo->pdnType |= 0x02;
    }
    if (!ip_addr_isany(&currentNetif->ip_addr))
        backupInfo->pdnType |= 0x01;

    for (int i = 0; i < LWIP_IPV6_NUM_ADDRESSES; i++)
    {
        ip_addr_copy(backupInfo->ip6_addr[i], currentNetif->ip6_addr[i]);
        backupInfo->ip6_addr_state[i] = netif_ip6_addr_state(currentNetif, i);
#if LWIP_IPV6_ADDRESS_LIFETIMES
        backupInfo->ip6_addr_valid_life[i] = netif_ip6_addr_valid_life(currentNetif, i);
        backupInfo->ip6_addr_pref_life[i] = netif_ip6_addr_pref_life(currentNetif, i);
#endif
#if LWIP_IPV6_AUTOCONFIG
        backupInfo->ip6_autoconfig_enabled = currentNetif->ip6_autoconfig_enabled;
#endif
#if LWIP_IPV6_SEND_ROUTER_SOLICIT
        backupInfo->rs_count = currentNetif->rs_count;
#endif
    }
}

static void NetifPsmSaveBackupInfo(T_Netif_Backup_Info *netBackupInfo)
{
    OSI_LOGI(0x1000a968, "NetifPsmSaveBackupInfo enter\n");
    struct netif *tmp = NULL;

    tmp = netif_list;
    while (tmp)
    {
        if ((tmp->name[0] == 'G') && (tmp->name[1] == 'P'))
        {
            uint8_t cid = tmp->sim_cid & 0x0F;
            uint8_t simId = (tmp->sim_cid >> 4) & 0x0F;
            if (cid < MAX_CID && simId < MAX_SIM_ID)
            {
                T_Netif_Gprs_Backup_Info *backupInfo = &netBackupInfo->gprsNetif[simId][cid];
                NetifGprsPsmSaveBackupInfo(backupInfo, tmp);
            }
        }
        tmp = tmp->next;
    }

    for (int sid = 0; sid < DNS_MAX_SIM; sid++)
        for (int cid = 1; cid <= DNS_MAX_CID; cid++)
            for (int i = 0; i < DNS_MAX_SERVERS; i++)
                memcpy(&netBackupInfo->dns_servers[sid][cid - 1][i], dns_getserver(sid, cid, i), sizeof(ip_addr_t));
}
#ifdef CONFIG_FIBOCOM_BASE
#if IP_NAT
void netif_reset_Ip6addr(struct netif *netif, T_Netif_Gprs_Backup_Info *backupInfo)
{
    OSI_LOGI(0, "netif_reset_Ip6addr");
    int nCid = netif->sim_cid & 0x0f;
    int nSimId = (netif->sim_cid & 0xf0) >> 4;
    CFW_GPRS_PDPCONT_INFO_V2 pdp_context = {0};
    CFW_GprsGetPdpCxtV2(nCid, &pdp_context, nSimId);
    OSI_LOGI(0, "netif_reset_Ip6addr pdp_context.PdnType %d\n", pdp_context.PdnType);
#if LWIP_IPV6
    if (((pdp_context.PdnType == CFW_GPRS_PDP_TYPE_IPV6) || (pdp_context.PdnType == CFW_GPRS_PDP_TYPE_IPV4V6)))
    {
        for (int i = 0; i < LWIP_IPV6_NUM_ADDRESSES; i++)
        {
            ip_addr_copy(netif->ip6_addr[i], backupInfo->ip6_addr[i]);
#if LWIP_IPV6_ADDRESS_LIFETIMES
            netif_ip6_addr_set_valid_life(netif, i, backupInfo->ip6_addr_valid_life[i]);
            netif_ip6_addr_set_pref_life(netif, i, backupInfo->ip6_addr_pref_life[i]);
#endif
            netif_ip6_addr_set_state(netif, i, backupInfo->ip6_addr_state[i]);
#if LWIP_IPV6_AUTOCONFIG
            netif->ip6_autoconfig_enabled = backupInfo->ip6_autoconfig_enabled;
#endif
            OSI_LOGXI(OSI_LOGPAR_S, 0, "netif_reset_Ip6addr IP6: %s", ipaddr_ntoa(&(netif->ip6_addr[i])));
        }
    }
#endif
#if LWIP_IPV6_SEND_ROUTER_SOLICIT
    netif->rs_count = backupInfo->rs_count;
#endif
}
void TCPIP_nat_netif_create_psm(T_Netif_Gprs_Backup_Info *backupInfo)
{
    OSI_LOGI(0, "TCPIP_nat_netif_create_psm begin\n");
    uint8_t nCid = backupInfo->cid;
    uint8_t nSimId = backupInfo->simId;
    struct wan_netif *wan = NULL;
    struct netif *wan_netif = NULL;
    wan = newWan(nSimId, nCid);
    if (wan != NULL)
    {
        wan_netif = wan->netif;
        netif_reset_Ip6addr(wan_netif, backupInfo);
    }
    isPsmNatRestore = false;
}
#endif
#endif

/**/
void TCPIP_netif_create_psm(T_Netif_Gprs_Backup_Info *backupInfo)
{
    uint8_t nCid = backupInfo->cid;
    uint8_t nSimId = backupInfo->simId;
    struct netif *netif = NULL;
    uint8_t T_cid = nSimId << 4 | nCid;
    netif = getGprsNetIf(nSimId, nCid);
    if (netif != NULL)
    {
        OSI_LOGI(0x1000982c, "TCPIP_netif_create_psm netif already created cid %d simid %d\n", nCid, nSimId);
        return;
    }

    ip4_addr_t ip4 = {0};

    OSI_LOGI(0x1000982d, "TCPIP_netif_create_psm cid %d simid %d\n", nCid, nSimId);

    CFW_GPRS_PDPCONT_INFO_V2 pdp_context = {0};
    CFW_GprsGetPdpCxtV2(nCid, &pdp_context, nSimId);
    OSI_LOGI(0x1000982e, "TCPIP_netif_create_psm pdp_context.PdnType %d\n", pdp_context.PdnType);
    if ((pdp_context.PdnType == CFW_GPRS_PDP_TYPE_IP) || (pdp_context.PdnType == CFW_GPRS_PDP_TYPE_IPV4V6) || (pdp_context.PdnType == CFW_GPRS_PDP_TYPE_X25) || (pdp_context.PdnType == CFW_GPRS_PDP_TYPE_PPP)) //add evade for nPdpType == 0, need delete later
    {
        IP4_ADDR(&ip4, pdp_context.pPdpAddr[0], pdp_context.pPdpAddr[1], pdp_context.pPdpAddr[2], pdp_context.pPdpAddr[3]);
#if defined(CONFIG_SOC_8811) || defined(CONFIG_SOC_8821)
        if (pdp_context.nPdpAddrSize == 12)
        {
            IP4_ADDR(&ip4, pdp_context.pPdpAddr[8], pdp_context.pPdpAddr[9], pdp_context.pPdpAddr[10], pdp_context.pPdpAddr[11]);
        }
#endif
        OSI_LOGXI(OSI_LOGPAR_S, 0x1000982f, "TCPIP_netif_create_psm IP4: %s", ip4addr_ntoa(&ip4));
    }
    if (nSimId < MAX_SIM_ID && nCid < MAX_CID)
    {
        netif = getGprsGobleNetIf(nSimId, nCid);
    }
    else
    {
        netif = malloc(sizeof(struct netif));
    }
    if (netif == NULL)
    {
        OSI_LOGI(0x10009830, "TCPIP_netif_create_psm memerror netif is NULL\n");
        return;
    }
    memset(netif, 0, sizeof(struct netif));
    netif->sim_cid = T_cid;
    netif->is_ppp_mode = 0;
    netif->link_mode = NETIF_LINK_MODE_LWIP;
    netif->pdnType = pdp_context.PdnType;
    netif->is_used = 1;
#ifndef CONFIG_NET_LTE_PSPACKET_SUPPORT
    drvPsIntf_t *dataPsPath = drvPsIntfOpen(nSimId, nCid, lwip_pspathDataInput, (void *)netif);
    if (dataPsPath != NULL)
    {
        netif->pspathIntf = dataPsPath;
        /**if dataPsPath is NULL, it has been write in netif->pspathIntf.
		   netif->pspathIntf need not update any more.**/
    }
#endif
#if LWIP_TCPIP_CORE_LOCKING
    LOCK_TCPIP_CORE();
#endif
    netif_add(netif, &ip4, NULL, NULL, NULL, netif_gprs_psm_init, tcpip_input);
#if LWIP_IPV6
    if (((pdp_context.PdnType == CFW_GPRS_PDP_TYPE_IPV6) || (pdp_context.PdnType == CFW_GPRS_PDP_TYPE_IPV4V6)))
    {
        for (int i = 0; i < LWIP_IPV6_NUM_ADDRESSES; i++)
        {
            ip_addr_copy(netif->ip6_addr[i], backupInfo->ip6_addr[i]);
#if LWIP_IPV6_ADDRESS_LIFETIMES
            netif_ip6_addr_set_valid_life(netif, i, backupInfo->ip6_addr_valid_life[i]);
            netif_ip6_addr_set_pref_life(netif, i, backupInfo->ip6_addr_pref_life[i]);
#endif
            netif_ip6_addr_set_state(netif, i, backupInfo->ip6_addr_state[i]);
#if LWIP_IPV6_AUTOCONFIG
            netif->ip6_autoconfig_enabled = backupInfo->ip6_autoconfig_enabled;
#endif
            OSI_LOGXI(OSI_LOGPAR_S, 0x10009831, "TCPIP_netif_create_psm IP6: %s", ipaddr_ntoa(&(netif->ip6_addr[i])));
        }
    }
#endif
    netif_set_up(netif);
    netif_set_link_up(netif);

#if LWIP_IPV6_SEND_ROUTER_SOLICIT
    netif->rs_count = backupInfo->rs_count;
#endif

#if LWIP_TCPIP_CORE_LOCKING
    UNLOCK_TCPIP_CORE();
#endif
    OSI_LOGI(0x10009832, "TCPIP_netif_create_psm, netif->num: 0x%x\n", netif->num);
}

static void NetifGprsPsmRestoreBackupInfo(T_Netif_Gprs_Backup_Info *backupInfo)
{
#ifdef CONFIG_FIBOCOM_BASE
#if IP_NAT
    uint8_t nCid = backupInfo->cid;
    uint8_t nSimId = backupInfo->simId;
    if (get_nat_enabled(nSimId, nCid))
    {
        isPsmNatRestore = true;
        TCPIP_nat_netif_create_psm(backupInfo);
        OSI_LOGI(0, "NetifGprsPsmRestoreBackupInfo:isPsmNatRestore %d", isPsmNatRestore);
    }
    else
#endif
#endif
        TCPIP_netif_create_psm(backupInfo);
}

static void NetifPsmRestoreBackupInfo(T_Netif_Backup_Info *netBackupInfo, uint8_t cId, uint8_t simId)
{
    OSI_LOGI(0x1000a969, "NetifPsmRestoreBackupInfo enter\n");
    // 8811
    if (cId == 0xff)
    {
        T_Netif_Gprs_Backup_Info *backupInfo;
        for (int cid = 0; cid < MAX_CID; cid++)
        {
            backupInfo = &netBackupInfo->gprsNetif[simId][cid];
            if (backupInfo->simId == simId && backupInfo->cid == cid && backupInfo->pdnType != 0)
            {
                NetifGprsPsmRestoreBackupInfo(backupInfo);
            }
        }
    }
    else
    {
        if (simId >= MAX_SIM_ID || cId >= MAX_CID)
        {
            OSI_LOGI(0x10009833, "NetifPsmRestoreBackupInfo ivalid cid %d simid %d\n", cId, simId);
            return;
        }
        T_Netif_Gprs_Backup_Info *backupInfo = &netBackupInfo->gprsNetif[simId][cId];
        if (backupInfo->cid == cId && backupInfo->simId == simId)
        {
            NetifGprsPsmRestoreBackupInfo(backupInfo);
        }
    }
    for (int sid = 0; sid < DNS_MAX_SIM; sid++)
        for (int cid = 1; cid <= DNS_MAX_CID; cid++)
            for (int i = 0; i < DNS_MAX_SERVERS; i++)
            {
                dns_setserver(sid, cid, i, &netBackupInfo->dns_servers[sid][cid - 1][i]);
            }
}

void NetifPm3Presave(void)
{
    unsigned size = sizeof(T_Netif_Backup_Info);
    T_Netif_Backup_Info *netif_backupInfo = (T_Netif_Backup_Info *)calloc(1, size);
    if (netif_backupInfo != NULL)
    {
        OSI_LOGI(0x1000a96a, "NetifPm3Presave begin\n");
        NetifPsmSaveBackupInfo(netif_backupInfo);
        osiPsmDataSave(OSI_PSMDATA_OWNER_LWIP, netif_backupInfo, size);
        free(netif_backupInfo);
    }
}

void NetifPm3Restore(uint8_t cId, uint8_t simId)
{
    unsigned size = sizeof(T_Netif_Backup_Info);
    T_Netif_Backup_Info *netif_backupInfo = (T_Netif_Backup_Info *)calloc(1, size);
    if (netif_backupInfo != NULL)
    {
        OSI_LOGI(0x1000a96b, "NetifPm3Restore begin\n");
        osiPsmDataRestore(OSI_PSMDATA_OWNER_LWIP, netif_backupInfo, size);
        NetifPsmRestoreBackupInfo(netif_backupInfo, cId, simId);
        free(netif_backupInfo);
#ifdef CONFIG_NET_CTWING_SELFREG_SUPPORT
        dm_aep_register();
#endif
#ifdef CONFIG_NET_CTIOT_AEP_SUPPORT
        ctiot_engine_entry(false);
#endif
        // There are macros inside this function.
        dm_lwm2m_register();
    }
}

#endif
