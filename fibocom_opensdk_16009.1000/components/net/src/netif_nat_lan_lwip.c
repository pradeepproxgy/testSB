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
#include "lwip/prot/ip.h"
#include "lwip/prot/ip6.h"
#include "lwip/netif.h"
#include "lwip/dns.h"
#include "lwip/tcpip.h"
#if IP_NAT
#include "lwip/ip4_nat.h"
#endif

#include "drv_ps_path.h"
#include "netif_ppp.h"
#include "netutils.h"
#include "at_cfw.h"

#ifdef CONFIG_FIBOCOM_XLAT_SUPPORT
#include "ffw_plat_xlat.h"
#include "ffw_result.h"
#endif

#if IP_NAT
#define SIM_CID(sim, cid) ((((sim)&0xf) << 4) | ((cid)&0xf))

extern osiThread_t *netGetTaskID(void);
extern uint32_t *getDNSServer(uint8_t nCid, uint8_t nSimID);
extern uint32_t do_send_stored_packet(uint8_t nCID, CFW_SIM_ID nSimID);
extern void dns_clean_entries(void);
extern u8_t ip4_wan_forward(struct pbuf *p, struct netif *inp);
extern u8_t netif_num;
#if LWIP_IPV6
extern void setIp6address(struct netif *netif, CFW_GPRS_PDPCONT_INFO_V2 pdp_context);
#endif

extern void setIp4dns(int simid, int cid, CFW_GPRS_PDPCONT_INFO_V2 pdp_context);
#if LWIP_IPV6
extern void setIp6dns(int simid, int cid, CFW_GPRS_PDPCONT_INFO_V2 pdp_context);
#endif

#define MAX_SIM_ID 2
#define MAX_CID 7
static struct netif gprs_netif[MAX_SIM_ID][MAX_CID] = {0};

err_t nat_lan_lwip_tcpip_input(struct pbuf *p, struct netif *inp)
{
    if (p == NULL || inp == NULL)
        return 0;
    inp->u32LwipDLSize += p->tot_len;
    return tcpip_input(p, inp);
}

static err_t nat_lan_lwip_data_output(struct netif *netif, struct pbuf *p,
                                      ip_addr_t *ipaddr)
{
    if (p != NULL)
    {
#if LWIP_IPV6
        if (IP_HDR_GET_VERSION(p->payload) == 6)
        {
            struct ip6_hdr *ip6hdr;
            ip6hdr = (struct ip6_hdr *)p->payload;
            u8_t nexth;
            pbuf_realloc(p, IP6_HLEN + IP6H_PLEN(ip6hdr));
            nexth = IP6H_NEXTH(ip6hdr);
            if (nexth == IP6_NEXTH_ICMP6)
            {
                struct icmp6_hdr *icmp6hdr;
                icmp6hdr = (struct icmp6_hdr *)p->payload;
                if (icmp6hdr->type == ICMP6_TYPE_RS)
                {
                    //save IPV6 local addr to lan netif
                }
            }
            //find Wan netif with same SimCid to send IPV6 packeage out
            struct netif *Wannetif = netif_get_by_cid_type(netif->sim_cid, NETIF_LINK_MODE_NAT_WAN);
            if (Wannetif)
            {
                OSI_LOGI(0x10007aea, "nat_lan_lwip_data_output IPV6 to Wan netif");
#if LWIP_TCPIP_CORE_LOCKING
                LOCK_TCPIP_CORE();
#endif
                Wannetif->output_ip6(Wannetif, p, NULL);
#if LWIP_TCPIP_CORE_LOCKING
                UNLOCK_TCPIP_CORE();
#endif
            }
        }
        else
#endif
        {
#if LWIP_TCPIP_CORE_LOCKING
            LOCK_TCPIP_CORE();
#endif
            u8_t taken = ip4_wan_forward(p, netif);
            if (taken == 0)
                taken = ip4_nat_out(p);

#ifdef CONFIG_FIBOCOM_XLAT_SUPPORT // XLAT
            if (taken == 0)
            {
                //2024-06-24, sundaqing, Begin. Lwip support XLAT 
                taken = ffw_xlat_clat_output(p, p->tot_len, ((netif->sim_cid) >> 4) & 0xF, (netif->sim_cid) & 0xF) == FFW_R_SUCCESS;
                //2024-06-24, sundaqing, End. Lwip support XLAT 
            }
#endif
#if LWIP_TCPIP_CORE_LOCKING
            UNLOCK_TCPIP_CORE();
#endif
            OSI_LOGI(0x10007aeb, "nat_lan_lwip_data_output %d", taken);
        }
        netif->u32LwipULSize += p->tot_len;
    }

    return ERR_OK;
}

err_t netif_gprs_nat_lan_lwip_init(struct netif *netif)
{
    /* initialize the snmp variables and counters inside the struct netif
   * ifSpeed: no assumption can be made!
   */
    netif->name[0] = 'G';
    netif->name[1] = 'P';

#if LWIP_IPV4
    netif->output = (netif_output_fn)nat_lan_lwip_data_output;
#endif
#if LWIP_IPV6
    netif->output_ip6 = (netif_output_ip6_fn)nat_lan_lwip_data_output;
#endif
    netif->mtu = GPRS_MTU;

    if (netif->pdnType > CFW_GPRS_PDP_TYPE_PPP)
        return ERR_OK;

    if (NULL == netif_default)
        netif_set_default(netif);

    int nCid = netif->sim_cid & 0x0f;
    int nSimId = (netif->sim_cid & 0xf0) >> 4;
    CFW_GPRS_PDPCONT_INFO_V2 pdp_context;

    if (CFW_GprsGetPdpCxtV2(nCid, &pdp_context, nSimId) == ERR_SUCCESS)
    {
        setIp4dns(nSimId, nCid, pdp_context);
#if LWIP_IPV6
        setIp6dns(nSimId, nCid, pdp_context);
#endif
    }
    return ERR_OK;
}

struct netif *TCPIP_nat_lan_lwip_netif_create(uint8_t nCid, uint8_t nSimId)
{
    struct netif *netif = NULL;
    uint8_t T_cid = nSimId << 4 | nCid;
    struct netif *wan_netif = netif_get_by_cid_type(T_cid, NETIF_LINK_MODE_NAT_WAN);
    if (wan_netif == NULL)
    {
        return NULL;
    }
    ip4_addr_t *wan_ip4_addr = (ip4_addr_t *)netif_ip4_addr(wan_netif);
    ip4_addr_t ip4;
    ip4_addr_t ip4_gw;
    OSI_LOGI(0x10007aec, "TCPIP_nat_lan_lwip_netif_create wan_ip4_addr 0x%x\n", wan_ip4_addr->addr);
    if ((wan_ip4_addr->addr & 0xff) != 0xc0) //192.xxx.xxx.xxx
    {
        IP4_ADDR(&ip4, 192, 168, nCid, 2 + netif_num);
        IP4_ADDR(&ip4_gw, 192, 168, nCid, 1);
    }
    else
    {
        IP4_ADDR(&ip4, 10, 0, nCid, 2 + netif_num);
        IP4_ADDR(&ip4_gw, 10, 0, nCid, 1);
    }
    ip4_addr_t ip4_netMast;
    IP4_ADDR(&ip4_netMast, 255, 255, 255, 0);
    //uint8_t lenth;

    OSI_LOGI(0x10007aed, "TCPIP_nat_lan_lwip_netif_create cid %d simid %d\n", nCid, nSimId);

    CFW_GPRS_PDPCONT_INFO_V2 pdp_context;
    CFW_GprsGetPdpCxtV2(nCid, &pdp_context, nSimId);
    OSI_LOGI(0x10007aee, "TCPIP_nat_lan_lwip_netif_create pdp_context.PdnType %d\n", pdp_context.PdnType);
    if ((pdp_context.PdnType == CFW_GPRS_PDP_TYPE_IP) || (pdp_context.PdnType == CFW_GPRS_PDP_TYPE_IPV4V6) || (pdp_context.PdnType == CFW_GPRS_PDP_TYPE_X25) || (pdp_context.PdnType == CFW_GPRS_PDP_TYPE_PPP)) //add evade for nPdpType == 0, need delete later
    {
        OSI_LOGXI(OSI_LOGPAR_S, 0x10007548, "TCPIP_netif_create IP4: %s", ip4addr_ntoa(&ip4));
    }
    else
    {
        ip4.addr = 0;
        OSI_LOGXI(OSI_LOGPAR_S, 0x10007548, "TCPIP_netif_create IP4: %s", ip4addr_ntoa(&ip4));
    }
    if (nSimId < MAX_SIM_ID && nCid < MAX_CID)
    {
        netif = &gprs_netif[nSimId][nCid];
    }
    else
    {
        netif = malloc(sizeof(struct netif));
    }

    if (netif == NULL)
    {
        OSI_LOGI(0x10009813, "TCPIP_nat_lan_lwip_netif_create memerror netif is NULL\n");
        return NULL;
    }
    memset(netif, 0, sizeof(struct netif));
    netif->sim_cid = T_cid;
    netif->is_ppp_mode = 0;
    netif->link_mode = NETIF_LINK_MODE_NAT_LWIP_LAN;
    netif->pdnType = pdp_context.PdnType;
    netif->is_used = 1;

#if LWIP_TCPIP_CORE_LOCKING
    LOCK_TCPIP_CORE();
#endif
    netif_add(netif, &ip4, &ip4_netMast, &ip4_gw, NULL, netif_gprs_nat_lan_lwip_init, nat_lan_lwip_tcpip_input);
#if LWIP_IPV6
    setIp6address(netif, pdp_context);
#endif

    netif_set_up(netif);
    netif_set_link_up(netif);

    ip4_nat_entry_t *new_nat_entry = (ip4_nat_entry_t *)calloc(1, sizeof(ip4_nat_entry_t));
    if (new_nat_entry == NULL)
    {
        if (nSimId < MAX_SIM_ID && nCid < MAX_CID)
        {
            netif->is_used = 0;
        }
        else
        {
            free(netif);
        }
        return NULL;
    }
    new_nat_entry->in_if = netif;
    new_nat_entry->out_if = wan_netif;
    IP4_ADDR(&(new_nat_entry->dest_netmask), 255, 255, 255, 255);
    ip4_addr_copy(new_nat_entry->source_net, ip4);
    IP4_ADDR(&(new_nat_entry->source_netmask), 255, 255, 255, 255);
    ip4_nat_add(new_nat_entry);
    netif->nat_entry = new_nat_entry;
#if LWIP_TCPIP_CORE_LOCKING
    UNLOCK_TCPIP_CORE();
#endif
    OSI_LOGI(0x10007af0, "TCPIP_nat_lan_lwip_netif_create, netif->num: 0x%x\n", netif->num);
    return netif;
}

void TCPIP_nat_lan_lwip_netif_destory(uint8_t nCid, uint8_t nSimId)
{
    struct netif *netif = NULL;

    netif = netif_get_by_cid_type(nSimId << 4 | nCid, NETIF_LINK_MODE_NAT_LWIP_LAN);
    if (netif != NULL)
    {
#if LWIP_TCPIP_CORE_LOCKING
        LOCK_TCPIP_CORE();
#endif
        ip4_nat_entry_t *new_nat_entry = netif->nat_entry;
        if (new_nat_entry != NULL)
        {
            netif->nat_entry = NULL;
            ip4_nat_remove(new_nat_entry);
            free(new_nat_entry);
        }
        netif_set_link_down(netif);
        netif_remove(netif);
        dns_clean_entries();
        if (nSimId < MAX_SIM_ID && nCid < MAX_CID)
        {
            netif->is_used = 0;
        }
        else
        {
            free(netif);
        }
        OSI_LOGI(0x10007af1, "TCPIP_nat_lan_lwip_netif_destory netif");
#if LWIP_TCP
        tcp_debug_print_pcbs();
#endif
#if LWIP_TCPIP_CORE_LOCKING
        UNLOCK_TCPIP_CORE();
#endif
    }
    else
    {
        OSI_LOGI(0x1000754c, "Error ########### CFW_GPRS_DEACTIVED can't find netif for CID=0x%x\n", SIM_CID(nSimId, nCid));
    }
}
void netif_setup_lwip_lanOnly()
{

    OSI_LOGI(0x10007af4, "net_nat\n");
    if (get_nat_enabled(0, 0x0f))
    {
        //TCPIP_nat_lanOnly_lwip_netif_create(0x0f, 0x0f);
        struct lan_netif *plan = newLan(NAT_SIM, NAT_CID, NAT_LAN_IDX_LWIP);
        if (plan == NULL)
            return;
        lan_create(plan, NAT_SIM, NAT_CID, netif_gprs_nat_lan_lwip_init, nat_lan_lwip_tcpip_input);
    }

    return;
}
#endif //IP_NAT
