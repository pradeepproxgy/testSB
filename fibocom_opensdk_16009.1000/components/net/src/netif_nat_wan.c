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
#ifdef CONFIG_FIBOCOM_BASE
#include "ffw_nv.h"
#ifdef CONFIG_FIBOCOM_XLAT_SUPPORT
#include "ffw_plat_xlat.h"
#endif
#endif

#if IP_NAT
#define SIM_CID(sim, cid) ((((sim)&0xf) << 4) | ((cid)&0xf))

extern osiThread_t *netGetTaskID(void);
extern uint32_t *getDNSServer(uint8_t nCid, uint8_t nSimID);
extern uint32_t do_send_stored_packet(uint8_t nCID, CFW_SIM_ID nSimID);
extern void dns_clean_entries(void);
#ifdef CONFIG_FIBOCOM_BASE
#if IP_NAT
extern bool isPsmNatRestore;
#endif
#endif

#define MAX_SIM_ID 2
#define MAX_CID 8 //as cid is from 1~7, the array declare 8 element for make index same as cid value
extern u8_t netif_num;
extern void setIp6address(struct netif *netif, CFW_GPRS_PDPCONT_INFO_V2 pdp_context);
struct wan_netif *wan[MAX_SIM_ID][MAX_CID] = {0};

#ifdef CONFIG_NET_LTE_PSPACKET_SUPPORT
#include "drv_md_ipc.h"
#define PS_UL_HDR_LEN IPC_PS_UL_HDR_LEN
#endif

#if defined CONFIG_FIBOCUS_HAIKANG_FEATURE
extern uint8_t fibo_get_current_lan_ipaddr_haikang();
#endif
extern char* fibo_get_ecm_host_ip(void);
#ifdef CONFIG_FIBOCOM_ECM_IP_MASK_GW_API
static uint8_t ecm_tcid = 0;
void fibo_setcid(uint8_t cid)
{
    ecm_tcid = cid;
}
uint8_t fibo_getcid(void)
{
    return ecm_tcid;
}
#endif

uint8_t wan_isUnique(uint8_t sim, uint8_t cid)
{
    uint8_t iRet = true;
    uint8_t i, j;
    //struct wan_netif *pwanlist[2]= &wan[0];

    //if (pwanlist == NULL)
    //    goto end;

    for (i = 0; i < MAX_SIM_ID; i++)
    {
        for (j = 0; j < MAX_CID; j++)
        {
            if (wan[i][j] != NULL)
            {
                OSI_LOGI(0x10007b4a, "wan[%d][%d]", i, j);
                if ((wan[i][j]->act == 1) && (i != sim || j != cid))
                {
                    iRet = false;
                    break;
                }
            }
        }
    }
    //end:
    OSI_LOGI(0x10007b4b, "wan_isUnique iRet = %d", iRet);
    return iRet;
}
extern struct netif gprs_netif[MAX_SIM_ID][MAX_CID];

static void gprs_data_ipc_to_lwip_nat_wan(void *ctx)
{
    struct netif *inp_netif = (struct netif *)ctx;
    if (inp_netif == NULL)
        return;
    //struct pbuf *p, *q;
    struct pbuf *p, *q, *ptmp;
    OSI_LOGI(0x10007538, "gprs_data_ipc_to_lwip");
    uint8_t *pData = malloc(1600);
    if (pData == NULL)
        return;
    int len = 0;
    int readLen = 0;
    int offset = 0;
    do
    {
        OSI_LOGI(0x10007539, "drvPsIntfRead in");
        readLen = drvPsIntfRead(inp_netif->pspathIntf, pData, 1600);

#ifdef CONFIG_FIBOCOM_XLAT_SUPPORT
        ffw_xlat_plat_input(pData, readLen, &readLen);
#endif


#ifdef CONFIG_NET_TRACE_IP_PACKET
        uint8_t *ipdata = pData;
        uint16_t identify = (ipdata[4] << 8) + ipdata[5];
        OSI_LOGI(0x10007b26, "Wan DL read from IPC thread identify %04x", identify);
#endif
        len = readLen;
        OSI_LOGI(0x1000753a, "drvPsIntfRead out %d", len);
        if (len > 0)
        {
            sys_arch_dump(pData, len);
        }
        if (inp_netif != NULL && len > 0)
        {
            p = (struct pbuf *)pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
            offset = 0;
            if (p != NULL)
            {
                if (len > p->len)
                {
                    for (q = p; len > q->len; q = q->next)
                    {
                        memcpy(q->payload, &(pData[offset]), q->len);
                        len -= q->len;
                        offset += q->len;
                    }
                    if (len != 0)
                    {
                        memcpy(q->payload, &(pData[offset]), len);
                    }
                }
                else
                {
                    memcpy(p->payload, &(pData[offset]), len);
                }
#if  LWIP_IPV6
#if defined CONFIG_FIBOCUS_HAIKANG_FEATURE
                if (IP_HDR_GET_VERSION(p->payload) == 6 && inp_netif->pdnType == CFW_GPRS_PDP_TYPE_IP)
                {
                    OSI_LOGI(0, "drop ipv6 data in CFW_GPRS_PDP_TYPE_IP ");
                    pbuf_free(p);
                    continue;
                 }

                 OSI_LOGI(0, "gprs_data_ipc_to_lwip_nat_wan IP_VERSION %d ",IP_HDR_GET_VERSION(p->payload));
#endif
                if (IP_HDR_GET_VERSION(p->payload) == 6)
                { //find lan netif with same SimCid and same IPV6 addr to send
                    struct netif *netif;
                    u8_t sim_cid = inp_netif->sim_cid;
                    u8_t taken = 0;
                    NETIF_FOREACH(netif)
                    {
                        if (sim_cid == netif->sim_cid && (NETIF_LINK_MODE_NAT_LWIP_LAN == netif->link_mode || NETIF_LINK_MODE_NAT_PPP_LAN == netif->link_mode || NETIF_LINK_MODE_NAT_NETDEV_LAN == netif->link_mode))
                        {
                            struct ip6_hdr *ip6hdr = p->payload;
                            ip6_addr_t current_iphdr_dest;
                            ip6_addr_t *current_netif_addr;
                            ip6_addr_copy_from_packed(current_iphdr_dest, ip6hdr->dest);
                            current_netif_addr = (ip6_addr_t *)netif_ip6_addr(netif, 0);
                            if (current_netif_addr->addr[2] == current_iphdr_dest.addr[2] && current_netif_addr->addr[3] == current_iphdr_dest.addr[3])
                            {
                                OSI_LOGI(0x10007b27, "gprs_data_ipc_to_lwip_nat_wan IPV6 to Lan netif");
                                netif->input(p, netif);
                                taken = 1;
                                break;
                            }
                        }
                    }
                    if (taken == 0)
                    {
#if LWIP_TCPIP_CORE_LOCKING
                        LOCK_TCPIP_CORE();
#endif
                        NETIF_FOREACH(netif)
                        {
                            if (sim_cid == netif->sim_cid && (NETIF_LINK_MODE_NAT_LWIP_LAN == netif->link_mode || NETIF_LINK_MODE_NAT_PPP_LAN == netif->link_mode || NETIF_LINK_MODE_NAT_NETDEV_LAN == netif->link_mode))
                            {
                                OSI_LOGI(0x10007b28, "gprs_data_ipc_to_lwip_nat_wan brodcast IPV6 to Lan netif");
                                //pbuf_ref(p);
                                // netif->input(p, netif);
                                ptmp = (struct pbuf *)pbuf_alloc(PBUF_RAW, readLen, PBUF_POOL);
                               if (ptmp != NULL)
                                {

                                pbuf_copy(ptmp, p);
                                netif->input(ptmp, netif);
                                }

                            }
                        }
#if LWIP_TCPIP_CORE_LOCKING
                        UNLOCK_TCPIP_CORE();
#endif
                        inp_netif->input(p, inp_netif);
                    }
                }
                else
#endif
                {
                    u8_t taken = 0;
#if LWIP_TCPIP_CORE_LOCKING
                    LOCK_TCPIP_CORE();
#endif
                    taken = ip4_nat_input(p);
#if LWIP_TCPIP_CORE_LOCKING
                    UNLOCK_TCPIP_CORE();
#endif
                    if (taken == 0)
                        inp_netif->input(p, inp_netif);
                }
                inp_netif->u32LwipDLSize += readLen;
            }
        }
    } while (readLen > 0);
    free(pData);
}

void lwip_nat_wan_pspathDataInput(void *ctx, drvPsIntf_t *p)
{
    OSI_LOGI(0x10007b29, "lwip_nat_wan_pspathDataInput osiThreadCallback in ");
    osiThreadCallback(netGetTaskID(), gprs_data_ipc_to_lwip_nat_wan, (void *)ctx);
    OSI_LOGI(0x10007b2a, "lwip_nat_wan_pspathDataInput osiThreadCallback out");
}
#ifdef CONFIG_NET_LTE_PSPACKET_SUPPORT
void gprs_ps_data_to_wan(void *ctx)
{
    struct ps_header *psData = (struct ps_header *)ctx;
    if (psData == NULL)
        return;

    struct netif *inp_netif = NULL;
    inp_netif = getGprsWanNetIf(psData->simid, psData->cid);
    if (inp_netif == NULL)
    {
        free(psData);
        return;
    }
    struct pbuf *p, *q, *ptmp;
    OSI_LOGI(0x1000a988, "gprs_ps_data_to_lwip");
    uint8_t *pData = NULL;
    int len = 0;
    int readLen = 0;
    int offset = 0;

    readLen = psData->len;
    len = readLen;
    pData = ((uint8_t *)psData) + psData->data_off;
    OSI_LOGI(0x1000a978, "psData buffer len %d", len);
#ifdef CONFIG_FIBOCOM_XLAT_SUPPORT
    ffw_xlat_plat_input(pData, readLen, &readLen);
    len = readLen;
#endif
    if (len > 0)
    {
        sys_arch_dump(pData, len);
    }
    if (inp_netif != NULL && len > 0)
    {
        p = (struct pbuf *)pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
        offset = 0;
        if (p != NULL)
        {
            if (len > p->len)
            {
                for (q = p; len > q->len; q = q->next)
                {
                    memcpy(q->payload, &(pData[offset]), q->len);
                    len -= q->len;
                    offset += q->len;
                }
                if (len != 0)
                {
                    memcpy(q->payload, &(pData[offset]), len);
                }
            }
            else
            {
                memcpy(p->payload, &(pData[offset]), len);
            }
#if LWIP_IPV6
            if (IP_HDR_GET_VERSION(p->payload) == 6)
            { //find lan netif with same SimCid and same IPV6 addr to send
                struct netif *netif;
                u8_t sim_cid = inp_netif->sim_cid;
                u8_t taken = 0;
                NETIF_FOREACH(netif)
                {
                    if (sim_cid == netif->sim_cid && (NETIF_LINK_MODE_NAT_LWIP_LAN == netif->link_mode || NETIF_LINK_MODE_NAT_PPP_LAN == netif->link_mode || NETIF_LINK_MODE_NAT_NETDEV_LAN == netif->link_mode))
                    {
                        struct ip6_hdr *ip6hdr = p->payload;
                        ip6_addr_t current_iphdr_dest;
                        ip6_addr_t *current_netif_addr;
                        ip6_addr_copy_from_packed(current_iphdr_dest, ip6hdr->dest);
                        current_netif_addr = (ip6_addr_t *)netif_ip6_addr(netif, 0);
                        if (current_netif_addr->addr[2] == current_iphdr_dest.addr[2] && current_netif_addr->addr[3] == current_iphdr_dest.addr[3])
                        {
                            OSI_LOGI(0x10007b27, "gprs_data_ipc_to_lwip_nat_wan IPV6 to Lan netif");
                            netif->input(p, netif);
                            taken = 1;
                            break;
                        }
                    }
                }
                if (taken == 0)
                {
                    NETIF_FOREACH(netif)
                    {
                        if (sim_cid == netif->sim_cid && (NETIF_LINK_MODE_NAT_LWIP_LAN == netif->link_mode || NETIF_LINK_MODE_NAT_PPP_LAN == netif->link_mode || NETIF_LINK_MODE_NAT_NETDEV_LAN == netif->link_mode))
                        {
                            OSI_LOGI(0x10007b28, "gprs_data_ipc_to_lwip_nat_wan brodcast IPV6 to Lan netif");
                            ptmp = (struct pbuf *)pbuf_alloc(PBUF_RAW, readLen, PBUF_POOL);
                            pbuf_copy(ptmp, p);
                            netif->input(ptmp, netif);
                        }
                    }
                    inp_netif->input(p, inp_netif);
                }
            }
            else
#endif
            {
                u8_t taken = 0;
#if LWIP_TCPIP_CORE_LOCKING
                LOCK_TCPIP_CORE();
#endif
                taken = ip4_nat_input(p);
#if LWIP_TCPIP_CORE_LOCKING
                UNLOCK_TCPIP_CORE();
#endif
                if (taken == 0)
                    inp_netif->input(p, inp_netif);
            }
            inp_netif->u32LwipDLSize += readLen;
        }
    }
    else
    {
        OSI_LOGI(0x1000a989, "gprs_ps_data_to_wan pbuffer malloc err, out of memery");
    }
    free(psData);
}

extern void API_ULDataIpcHandle(struct ps_header **p_psIpcBufferList, uint16_t v_availLen);
extern void *osiMemalign(size_t alignment, size_t size);
err_t nat_wan_data_output_ps_header(struct netif *netif, struct pbuf *p,
                                    ip_addr_t *ipaddr)
{
    uint16_t offset = 0;
    struct pbuf *q = NULL;

    OSI_LOGI(0x1000a98a, "nat_wan_data_output_ps_header ---------tot_len=%d, flags=0x%x---------", p->tot_len, p->flags);
    int nCid = netif->sim_cid & 0x0f;
    int nSimId = (netif->sim_cid & 0xf0) >> 4;
    uint32_t ps_len = PS_UL_HDR_LEN + p->tot_len + 28;
    uint32_t ps_len_align = OSI_ALIGN_UP(ps_len, 32); //32 align for cache
    struct ps_header *psHeader = (struct ps_header *)osiMemalign(32, ps_len_align);
    if (psHeader == NULL)
    {
        return !ERR_OK;
    }

    memset(psHeader, 0, ps_len_align);
    psHeader->cid = nCid;
    psHeader->simid = nSimId;
    psHeader->len = p->tot_len;
    psHeader->buf_size = ps_len;
    psHeader->data_off = PS_UL_HDR_LEN;
    uint8_t *p_ipData = (uint8_t *)psHeader + PS_UL_HDR_LEN;
    for (q = p; q != NULL; q = q->next)
    {
        memcpy(&p_ipData[offset], q->payload, q->len);
        offset += q->len;
    }

    sys_arch_dump(p_ipData, p->tot_len);

    OSI_LOGI(0x1000a97d, "sendPsDataToLte--in---");

    extern bool NetifGetDPSDFlag(CFW_SIM_ID nSim);
#define GET_SIM(sim_cid) (((sim_cid) >> 4) & 0xf)

    if (!NetifGetDPSDFlag(GET_SIM(netif->sim_cid)))
    {
        struct ps_header *p_psIpcBufferList[1] = {0};
        p_psIpcBufferList[0] = psHeader;
        API_ULDataIpcHandle(p_psIpcBufferList, 1);
    }
    else
    {
        free(psHeader);
    }
    OSI_LOGI(0x1000a97e, "sendPsDataToLte--out---");
    netif->u32LwipULSize += p->tot_len;

    OSI_LOGI(0x1000a98b, "nat_wan_data_output_ps_header--return in netif_gprs---");

    return ERR_OK;
}
#else

static err_t nat_wan_data_output(struct netif *netif, struct pbuf *p,
                                 ip_addr_t *ipaddr)
{
    uint16_t offset = 0;
    struct pbuf *q = NULL;

#if 1
    OSI_LOGI(0x1000753d, "data_output ---------tot_len=%d, flags=0x%x---------", p->tot_len, p->flags);

    uint8_t *pData = malloc(p->tot_len);
    if (pData == NULL)
    {
        return !ERR_OK;
    }

    uint8_t flags = 0;
    uint32_t rfAckSeqno = 0;
    for (q = p; q != NULL; q = q->next)
    {
        memcpy(&pData[offset], q->payload, q->len);
        offset += q->len;
#if LWIP_RFACK
        if (q->data_rf_ack_seqno != 0)
        {
            rfAckSeqno = q->data_rf_ack_seqno;
        }
#endif
        flags |= q->flags >> 5;
    }
    OSI_LOGI(0x10009816, "nat_wan_data_output flags %d rfAckSeqno %ld", flags, rfAckSeqno);
    sys_arch_dump(pData, p->tot_len);

    OSI_LOGI(0x1000753e, "drvPsIntfWrite--in---");
    drvPsIntfWrite((drvPsIntf_t *)netif->pspathIntf, pData, p->tot_len, flags, rfAckSeqno);
    OSI_LOGI(0x1000753f, "drvPsIntfWrite--out---");
    netif->u32LwipULSize += p->tot_len;
    free(pData);
#endif

    OSI_LOGI(0x10007540, "data_output--return in netif_gprs---");

    return ERR_OK;
}
#endif
void setIp4dns(int simid, int cid, CFW_GPRS_PDPCONT_INFO_V2 pdp_context)
{
    ip_addr_t ip4_dns1 = {0};
    ip_addr_t ip4_dns2 = {0};

    if ((pdp_context.PdnType == CFW_GPRS_PDP_TYPE_IP) || (pdp_context.PdnType == CFW_GPRS_PDP_TYPE_IPV4V6) || (pdp_context.PdnType == CFW_GPRS_PDP_TYPE_X25) ||
        (pdp_context.PdnType == CFW_GPRS_PDP_TYPE_PPP)) //add evade for nPdpType == 0, need delete later
    {
        if (pdp_context.nPdpDnsSize != 0)
        {
            IP_ADDR4(&ip4_dns1, pdp_context.pPdpDns[0], pdp_context.pPdpDns[1], pdp_context.pPdpDns[2], pdp_context.pPdpDns[3]);
            IP_ADDR4(&ip4_dns2, pdp_context.pPdpDns[21], pdp_context.pPdpDns[22], pdp_context.pPdpDns[23], pdp_context.pPdpDns[24]);
            OSI_LOGI(0x10007541, "DNS size:%d  DNS1:%d.%d.%d.%d", pdp_context.nPdpDnsSize,
                     pdp_context.pPdpDns[0], pdp_context.pPdpDns[1], pdp_context.pPdpDns[2], pdp_context.pPdpDns[3]);
            OSI_LOGI(0x10007542, "DNS2:%d.%d.%d.%d",
                     pdp_context.pPdpDns[21], pdp_context.pPdpDns[22], pdp_context.pPdpDns[23], pdp_context.pPdpDns[24]);
        }
    }
    dns_setserver(simid, cid, 0, &ip4_dns1);
    dns_setserver(simid, cid, 1, &ip4_dns2);
#ifdef CONFIG_FIBOCOM_BASE
    if (pdp_context.PdnType == CFW_GPRS_PDP_TYPE_IPV4V6)
    {
        OSI_PRINTFE("setIp4dns set IPV4 dns2 when IPV4V6");
        dns_setserver(simid, cid, 2, &ip4_dns2);
    }
#endif
}
#if LWIP_IPV6
void setIp6dns(int simid, int cid, CFW_GPRS_PDPCONT_INFO_V2 pdp_context)
{
    ip_addr_t ip6_dns1 = {0};
    ip_addr_t ip6_dns2 = {0};

    if (((pdp_context.PdnType == CFW_GPRS_PDP_TYPE_IPV6) || (pdp_context.PdnType == CFW_GPRS_PDP_TYPE_IPV4V6)))
    {
        if (pdp_context.nPdpDnsSize != 0)
        {
            uint32_t addr0 = PP_HTONL(LWIP_MAKEU32(pdp_context.pPdpDns[4], pdp_context.pPdpDns[5], pdp_context.pPdpDns[6], pdp_context.pPdpDns[7]));
            uint32_t addr1 = PP_HTONL(LWIP_MAKEU32(pdp_context.pPdpDns[8], pdp_context.pPdpDns[9], pdp_context.pPdpDns[10], pdp_context.pPdpDns[11]));
            uint32_t addr2 = PP_HTONL(LWIP_MAKEU32(pdp_context.pPdpDns[12], pdp_context.pPdpDns[13], pdp_context.pPdpDns[14], pdp_context.pPdpDns[15]));
            uint32_t addr3 = PP_HTONL(LWIP_MAKEU32(pdp_context.pPdpDns[16], pdp_context.pPdpDns[17], pdp_context.pPdpDns[18], pdp_context.pPdpDns[19]));
            uint32_t addr4 = PP_HTONL(LWIP_MAKEU32(pdp_context.pPdpDns[25], pdp_context.pPdpDns[26], pdp_context.pPdpDns[27], pdp_context.pPdpDns[28]));
            uint32_t addr5 = PP_HTONL(LWIP_MAKEU32(pdp_context.pPdpDns[29], pdp_context.pPdpDns[30], pdp_context.pPdpDns[31], pdp_context.pPdpDns[32]));
            uint32_t addr6 = PP_HTONL(LWIP_MAKEU32(pdp_context.pPdpDns[33], pdp_context.pPdpDns[34], pdp_context.pPdpDns[35], pdp_context.pPdpDns[36]));
            uint32_t addr7 = PP_HTONL(LWIP_MAKEU32(pdp_context.pPdpDns[37], pdp_context.pPdpDns[38], pdp_context.pPdpDns[39], pdp_context.pPdpDns[40]));
            OSI_LOGI(0x10007543, "DNS size:%d  ipv6 DNS 12-15:%d.%d.%d.%d", pdp_context.nPdpDnsSize,
                     pdp_context.pPdpDns[12], pdp_context.pPdpDns[13], pdp_context.pPdpDns[14], pdp_context.pPdpDns[15]);
            OSI_LOGI(0x10007544, " ipv6 DNS 33-36:%d.%d.%d.%d",
                     pdp_context.pPdpDns[33], pdp_context.pPdpDns[34], pdp_context.pPdpDns[35], pdp_context.pPdpDns[36]);
            IP_ADDR6(&ip6_dns1, addr0, addr1, addr2, addr3);
            IP_ADDR6(&ip6_dns2, addr4, addr5, addr6, addr7);
        }
    }
    if (pdp_context.PdnType == CFW_GPRS_PDP_TYPE_IPV6)
    {
        dns_setserver(simid, cid, 0, &ip6_dns1);
        dns_setserver(simid, cid, 1, &ip6_dns2);
    }
    if (pdp_context.PdnType == CFW_GPRS_PDP_TYPE_IPV4V6)
    {
        if (IP_IS_V6(&ip6_dns1))
            dns_setserver(simid, cid, 1, &ip6_dns1);
#ifdef CONFIG_FIBOCOM_BASE
        OSI_PRINTFE("setIp6dns set IPV6 dns2 when IPV4V6");
        dns_setserver(simid, cid, 3, &ip6_dns2);
#endif
    }
}
#endif

static err_t netif_gprs_nat_wan_init(struct netif *netif)
{
    /* initialize the snmp variables and counters inside the struct netif
   * ifSpeed: no assumption can be made!
   */
    netif->name[0] = 'G';
    netif->name[1] = 'P';

#ifdef CONFIG_NET_LTE_PSPACKET_SUPPORT
#if LWIP_IPV4
    netif->output = (netif_output_fn)nat_wan_data_output_ps_header;
#endif
#if LWIP_IPV6
    netif->output_ip6 = (netif_output_ip6_fn)nat_wan_data_output_ps_header;
#endif
#else
#if LWIP_IPV4
    netif->output = (netif_output_fn)nat_wan_data_output;
#endif
#if LWIP_IPV6
    netif->output_ip6 = (netif_output_ip6_fn)nat_wan_data_output;
#endif
#endif

    netif->mtu = GPRS_MTU;
    uint8_t nCid = netif->sim_cid & 0x0f;
    uint8_t nSimId = netif->sim_cid >> 4;

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

extern uint8_t CFW_GprsGetPdpIpv6Addr(uint8_t nCid, uint8_t *nLength, uint8_t *ipv6Addr, CFW_SIM_ID nSimID);
extern uint8_t CFW_GprsGetPdpIpv4Addr(uint8_t nCid, uint8_t *nLength, uint8_t *ipv4Addr, CFW_SIM_ID nSimID);
extern err_t ND6_Lan_Send_RS(struct netif *lan_netif, struct netif *netif);
struct netif *TCPIP_nat_wan_netif_create(uint8_t nCid, uint8_t nSimId)
{
    struct netif *netif = NULL;
    uint8_t T_cid = nSimId << 4 | nCid;
    netif = getGprsWanNetIf(nSimId, nCid);
    if (netif != NULL)
    {
        OSI_LOGI(0x10007b2b, "TCPIP_nat_wan_netif_create netif already created cid %d simid %d\n", nCid, nSimId);
        goto end;
    }

    ip4_addr_t ip4 = {0};
    //uint8_t lenth;

    OSI_LOGI(0x10007b2c, "TCPIP_nat_wan_netif_create cid %d simid %d\n", nCid, nSimId);

    AT_Gprs_CidInfo *pCidInfo = &gAtCfwCtx.sim[nSimId].cid_info[nCid];

    if (pCidInfo->uCid != nCid)
    {
        pCidInfo->uCid = nCid;
    }

    CFW_GPRS_PDPCONT_INFO_V2 pdp_context;
    if (CFW_GprsGetPdpCxtV2(nCid, &pdp_context, nSimId) != ERR_SUCCESS)
        goto end;
    OSI_LOGI(0x10007b2d, "TCPIP_nat_wan_netif_create pdp_context.PdnType %d\n", pdp_context.PdnType);
    if ((pdp_context.PdnType == CFW_GPRS_PDP_TYPE_IP) || (pdp_context.PdnType == CFW_GPRS_PDP_TYPE_IPV6) || (pdp_context.PdnType == CFW_GPRS_PDP_TYPE_IPV4V6) || (pdp_context.PdnType == CFW_GPRS_PDP_TYPE_X25) || (pdp_context.PdnType == CFW_GPRS_PDP_TYPE_PPP)) //add evade for nPdpType == 0, need delete later
    {
        IP4_ADDR(&ip4, pdp_context.pPdpAddr[0], pdp_context.pPdpAddr[1], pdp_context.pPdpAddr[2], pdp_context.pPdpAddr[3]);
#if defined(CONFIG_SOC_8811) || defined(CONFIG_SOC_8821)
        if (pdp_context.nPdpAddrSize == 12)
        {
            IP4_ADDR(&ip4, pdp_context.pPdpAddr[8], pdp_context.pPdpAddr[9], pdp_context.pPdpAddr[10], pdp_context.pPdpAddr[11]);
        }
#endif
        OSI_LOGXI(OSI_LOGPAR_S, 0x10007b2e, "TCPIP_nat_wan_netif_create IP4: %s", ip4addr_ntoa(&ip4));
    }
    else
        goto end;

    if (nSimId < MAX_SIM_ID && nCid < MAX_CID)
    {
        netif = &gprs_netif[nSimId][nCid];
    }
    else
    {
        netif = malloc(sizeof(struct netif));
    }
    memset(netif, 0, sizeof(struct netif));
    netif->sim_cid = T_cid;
    netif->is_ppp_mode = 0;
    netif->link_mode = NETIF_LINK_MODE_NAT_WAN;
    netif->pdnType = pdp_context.PdnType;
    netif->is_used = 1;

#ifndef CONFIG_NET_LTE_PSPACKET_SUPPORT
    drvPsIntf_t *dataPsPath = drvPsIntfOpen(nSimId, nCid, lwip_nat_wan_pspathDataInput, (void *)netif);
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
    netif_add(netif, &ip4, NULL, NULL, NULL, netif_gprs_nat_wan_init, tcpip_input);
#if LWIP_IPV6
#ifdef CONFIG_FIBOCOM_BASE
    #if IP_NAT
        if (isPsmNatRestore)
        {
            OSI_LOGI(0, "wan_netif_create, IP6 addr use backinfo restore");
        }
        else
    #endif
#endif
            setIp6address(netif, pdp_context);
#endif

#if LWIP_TCPIP_CORE_LOCKING
    UNLOCK_TCPIP_CORE();
#endif
end:
    OSI_LOGI(0x10007b2f, "wan_netif_create, netif = %p", netif);
    return netif;
}
void TCPIP_nat_wan_netif_destory(uint8_t nCid, uint8_t nSimId)
{
    struct netif *netif = NULL;

    netif = getGprsWanNetIf(nSimId, nCid);
    if (netif != NULL)
    {
#if LWIP_TCPIP_CORE_LOCKING
        LOCK_TCPIP_CORE();
#endif
#ifndef CONFIG_NET_LTE_PSPACKET_SUPPORT
        drvPsIntf_t *dataPsPath = netif->pspathIntf;
        drvPsIntfClose(dataPsPath);
#endif
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
        OSI_LOGI(0x10007b30, "TCPIP_nat_wan_netif_destory netif");
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

u8_t ip4_wan_forward(struct pbuf *p, struct netif *inp)
{
    u8_t taken = 0;
    if (IP_HDR_GET_VERSION(p->payload) == 4)
    {
        //find lan netif with same CID IPV4 addr to send
        struct netif *netif;
        NETIF_FOREACH(netif)
        {
            if ((NETIF_LINK_MODE_NAT_LWIP_LAN == netif->link_mode || NETIF_LINK_MODE_NAT_PPP_LAN == netif->link_mode || NETIF_LINK_MODE_NAT_NETDEV_LAN == netif->link_mode))
            {
                struct ip_hdr *iphdr = p->payload;
                ip4_addr_t current_iphdr_dest;
                ip4_addr_t *current_netif_addr;
                ip4_addr_copy(current_iphdr_dest, iphdr->dest);
                current_netif_addr = (ip4_addr_t *)netif_ip4_addr(netif);
                if (current_iphdr_dest.addr == current_netif_addr->addr)
                {
                    OSI_LOGI(0x10007b31, "ip4_wan_forward IPV4 to Lan netif");
                    pbuf_ref(p);
                    netif->input(p, netif);
                    taken = 1;
                    break;
                }
            }
        }
    }
    return taken;
}

extern err_t netif_gprs_nat_lan_lwip_init(struct netif *netif);
extern err_t nat_lan_lwip_tcpip_input(struct pbuf *p, struct netif *inp);
struct lan_netif *gLan_Netiflist_Nopdp = NULL;
uint8_t lantype[NAT_MAX_LANNUM] = {NETIF_LINK_MODE_NAT_LWIP_LAN, NETIF_LINK_MODE_NAT_NETDEV_LAN, NETIF_LINK_MODE_NAT_PPP_LAN};
uint8_t lanip[NAT_MAX_LANNUM] = {NAT_IPADDR_LWIP, NAT_IPADDR_RNDIS, NAT_IPADDR_PPP};
uint8_t wan_priority[MAX_SIM_ID][MAX_CID] = {{1, 1, 1, 1, 1, 1, 1}, {1, 1, 1, 1, 1, 1, 1}}; //{0};
struct netif *nat_lan_netif_create(uint8_t nCid, uint8_t nSimId, uint8_t nIdx, netif_init_fn init, netif_input_fn input)
{
    uint8_t T_cid = nSimId << 4 | nCid;
    struct netif *netif = NULL;
    ip4_addr_t ip4;
    ip4_addr_t ip4_gw;
    OSI_LOGI(0x10007b32, "nat_lan_netif_create ipaddress %d\n", lanip[nIdx]);

#ifdef CONFIG_FIBOCOM_ECM_IP_MASK_GW_API
    fibo_setcid(T_cid);
#endif
    if ((nIdx == NAT_LAN_IDX_RNDIS) || (nIdx == NAT_LAN_IDX_LWIP && nSimId == NAT_SIM && nCid == NAT_CID))
    {
#if defined CONFIG_FIBOCUS_HAIKANG_FEATURE
        uint8_t current_lan_ip = fibo_get_current_lan_ipaddr_haikang();
        if(current_lan_ip == 1)
        {
            IP4_ADDR(&ip4, 192, 168, 0, 100);
            IP4_ADDR(&ip4_gw, 192, 168, 0, 1);
        }
        else if(current_lan_ip == 2)
        {
            IP4_ADDR(&ip4, 192, 168, 225, 100);
            IP4_ADDR(&ip4_gw, 192, 168, 225, 1);
        }
        else if(current_lan_ip == 3)
        {
            IP4_ADDR(&ip4, 192, 168, 226, 100);
            IP4_ADDR(&ip4_gw, 192, 168, 226, 1);
        }
        else if(current_lan_ip == 4)
        {
            IP4_ADDR(&ip4, 192, 168, 224, 100);
            IP4_ADDR(&ip4_gw, 192, 168, 224, 1);
        }
#elif defined CONFIG_MC665_CN_19_90_WANJIAAN
        /*WANJIAAN requires a fixed IP address, but no specific address was specified */
        IP4_ADDR(&ip4, 192, 168, 225, 100);
        IP4_ADDR(&ip4_gw, 192, 168, 225, 1);
#elif defined CONFIG_MC665_CN_19_70_HZHC
        /*HZHC requires a fixed IP address, but no specific address was specified */
        IP4_ADDR(&ip4, 192, 168, 225, 108);
        IP4_ADDR(&ip4_gw, 192, 168, 225, 1);
#elif defined CONFIG_MC665_EU_19_90_HZHC
        /*HZHC requires a fixed IP address, but no specific address was specified */
        IP4_ADDR(&ip4, 192, 168, 225, 108);
        IP4_ADDR(&ip4_gw, 192, 168, 225, 1);
#else
    #ifdef CONFIG_FIBOCOM_BASE
        ip4_addr_t ip_addr;
        const char *ip_addr_t1;
        char *ip_split[20];
        char *outer_ptr = NULL;
        char lan_ip_addr[32] = {0};
        int   i = 0;

        if(NULL != fibo_get_ecm_host_ip())
        {
            ip_addr_t1 = fibo_get_ecm_host_ip();
            OSI_PRINTFE("[fibocom] fibo_get_ecm_host_ip get IP addr is %s", ip_addr_t1);
        }
        else
        {
            int32_t val[1]={0};
            ffw_nv_get("ECM_HOSTLANIP",val,1);
            OSI_PRINTFE("[fibocom] ECM_HOSTLANIP get IP inet_addr is %d", val[0]);

            struct sockaddr_in stLocalAddr = {0};
            stLocalAddr.sin_addr.s_addr = htonl(4294967296 - (unsigned)val[0]);
            ip_addr_t1 = inet_ntoa(stLocalAddr.sin_addr);
            OSI_PRINTFE("[fibocom] ECM_HOSTLANIP get IP addr is %s", ip_addr_t1);
        }
        ip4addr_aton(ip_addr_t1, &ip_addr);
        ip4_addr_copy(ip4, ip_addr);

        memcpy(lan_ip_addr, ip_addr_t1, strlen(ip_addr_t1));
        char *lan_ip_t = lan_ip_addr;
        while((ip_split[i]=strtok_r(lan_ip_t,".",&outer_ptr))!=NULL)
        {
            if(atoi(ip_split[i]) < 0 || atoi(ip_split[i]) > 255)
            {
                OSI_LOGI(0, "nat_lan_netif_create set LAN ip is NULL!!! ");
                return NULL;
            }
            i++;
            lan_ip_t = NULL;
        }
        IP4_ADDR(&ip4_gw, atoi(ip_split[0]), atoi(ip_split[1]), atoi(ip_split[2]), 1);
    #else
        IP4_ADDR(&ip4, 192, 168, 225, lanip[nIdx]); //);2 + netif_num);
        IP4_ADDR(&ip4_gw, 192, 168, 225, 1);
    #endif
#endif
    }
    else
    {
        IP4_ADDR(&ip4, 192, 168, nCid, 2 + netif_num);
        IP4_ADDR(&ip4_gw, 192, 168, nCid, 1);
    }

    ip4_addr_t ip4_netMast;
    IP4_ADDR(&ip4_netMast, 255, 255, 255, 0);
#if LWIP_IPV6
    ip6_addr_t ip6 = {0};
#endif
    //uint8_t lenth;

    OSI_LOGI(0x10007aed, "TCPIP_nat_lan_lwip_netif_create cid %d simid %d\n", nCid, nSimId);

    CFW_GPRS_PDPCONT_INFO_V2 pdp_context = {0};

#ifdef CONFIG_FIBOCOM_XLAT_SUPPORT
    int32_t xlat_switch = 0;
    ffw_nv_get1("XLAT", &xlat_switch);
    OSI_LOGI(0, "nat_lan_netif_create xlat_switch %d", (int)xlat_switch);
#endif


    if ((nCid != 0x0f && nSimId != 0x0f) && CFW_GprsGetPdpCxtV2(nCid, &pdp_context, nSimId) == ERR_SUCCESS)
    {
        OSI_LOGI(0x10007aee, "TCPIP_nat_lan_lwip_netif_create pdp_context.PdnType %d\n", pdp_context.PdnType);
        if ((pdp_context.PdnType == CFW_GPRS_PDP_TYPE_IP) || (pdp_context.PdnType == CFW_GPRS_PDP_TYPE_IPV4V6) || (pdp_context.PdnType == CFW_GPRS_PDP_TYPE_X25) || (pdp_context.PdnType == CFW_GPRS_PDP_TYPE_PPP)) //add evade for nPdpType == 0, need delete later
        {
            OSI_LOGXI(OSI_LOGPAR_S, 0x10007548, "TCPIP_netif_create IP4: %s", ip4addr_ntoa(&ip4));
        }
        else
        {
#ifdef CONFIG_FIBOCOM_XLAT_SUPPORT
            if(!xlat_switch)
            {
#endif
            ip4.addr = 0;
#ifdef CONFIG_FIBOCOM_XLAT_SUPPORT
            }
#endif
            OSI_LOGXI(OSI_LOGPAR_S, 0x10007548, "TCPIP_netif_create IP4: %s", ip4addr_ntoa(&ip4));
        }
    }
    else
        pdp_context.PdnType = CFW_GPRS_PDP_TYPE_PPP + 1;

    OSI_LOGI(0x10007b33, "1111 pdp_context.PdnType = %d", pdp_context.PdnType);

    //if (nSimId < MAX_SIM_ID && nCid < MAX_CID)
    //{
    //    netif = &gprs_netif[nSimId][nCid];
    //}
    //else
    //{
    netif = malloc(sizeof(struct netif));

    if (netif != NULL)
        memset(netif, 0, sizeof(struct netif));
    else
        return NULL;

    netif->sim_cid = T_cid;
    if (lantype[nIdx] == NETIF_LINK_MODE_NAT_PPP_LAN)
        netif->is_ppp_mode = 1;
    else
        netif->is_ppp_mode = 0;
    netif->link_mode = lantype[nIdx];
    netif->pdnType = pdp_context.PdnType;
    netif->is_used = 1;
    OSI_LOGI(0x10007b34, "2222 pdp_context.PdnType = %d", pdp_context.PdnType);
#if LWIP_TCPIP_CORE_LOCKING
    LOCK_TCPIP_CORE();
#endif
    OSI_LOGI(0x10007b35, "4444 pdp_context.PdnType = %d", pdp_context.PdnType);
    netif_add(netif, &ip4, &ip4_netMast, &ip4_gw, NULL, init, input);
    OSI_LOGI(0x10007b36, "5555 pdp_context.PdnType = %d", pdp_context.PdnType);
#if LWIP_IPV6
    if (((pdp_context.PdnType == CFW_GPRS_PDP_TYPE_IPV6) || (pdp_context.PdnType == CFW_GPRS_PDP_TYPE_IPV4V6)))
    {
        uint32_t addr0 = PP_HTONL(LWIP_MAKEU32(pdp_context.pPdpAddr[4], pdp_context.pPdpAddr[5], pdp_context.pPdpAddr[6], pdp_context.pPdpAddr[7]));
        uint32_t addr1 = PP_HTONL(LWIP_MAKEU32(pdp_context.pPdpAddr[8], pdp_context.pPdpAddr[9], pdp_context.pPdpAddr[10], pdp_context.pPdpAddr[11]));
        uint32_t addr2 = PP_HTONL(LWIP_MAKEU32(pdp_context.pPdpAddr[12], pdp_context.pPdpAddr[13], pdp_context.pPdpAddr[14], pdp_context.pPdpAddr[15]));
        uint32_t addr3 = PP_HTONL(LWIP_MAKEU32(pdp_context.pPdpAddr[16], pdp_context.pPdpAddr[17], pdp_context.pPdpAddr[18], pdp_context.pPdpAddr[19]));

#if defined(CONFIG_SOC_8811) || defined(CONFIG_SOC_8821)
        addr0 = PP_HTONL(LWIP_MAKEU32(0, 0, 0, 0));
        addr1 = PP_HTONL(LWIP_MAKEU32(0, 0, 0, 0));
        addr2 = PP_HTONL(LWIP_MAKEU32(pdp_context.pPdpAddr[0], pdp_context.pPdpAddr[1], pdp_context.pPdpAddr[2], pdp_context.pPdpAddr[3]));
        addr3 = PP_HTONL(LWIP_MAKEU32(pdp_context.pPdpAddr[4], pdp_context.pPdpAddr[5], pdp_context.pPdpAddr[6], pdp_context.pPdpAddr[7]));
#endif
        IP6_ADDR(&ip6, addr0, addr1, addr2, addr3);
        if (ip6.addr[0] == 0x0)
        {
            ip6.addr[0] = PP_HTONL(0xfe800000ul);
        }
        netif_ip6_addr_set(netif, 0, &ip6);
        netif_ip6_addr_set_state(netif, 0, IP6_ADDR_VALID);
        OSI_LOGXI(OSI_LOGPAR_S, 0x10007aef, "TCPIP_nat_lan_lwip_netif_create IP6: %s", ip6addr_ntoa(&ip6));
    }
#endif
    OSI_LOGI(0x10007b37, "6666 pdp_context.PdnType = %d", pdp_context.PdnType);
#if LWIP_TCPIP_CORE_LOCKING
    UNLOCK_TCPIP_CORE();
#endif
    netif_set_up(netif);
    netif_set_link_up(netif);
    OSI_LOGI(0x10007b38, "3333 netif = %p", netif);
    return netif;
}
void lan_setDefaultNetif(void *this)
{
    struct lan_netif *plan = (struct lan_netif *)this;

    if (plan == NULL)
        goto end;

    if ((plan->isUsing == 1) && (plan->netif != NULL))
    {
        netif_set_default(plan->netif);
        netif_reset_dns(plan->netif);
        plan->isDefault = 1;
        plan->netif->pstlan = plan;
        OSI_LOGI(0x10007b39, "lan_setDefaultNetif");
    }
end:
    return;
}
void lan_setSimCid(void *lannetif, int8_t sim, uint8_t cid)
{
    struct lan_netif *plan = (struct lan_netif *)lannetif;

    if (plan == NULL)
        goto end;

    if ((plan->isUsing == 1) && (plan->netif != NULL))
    {
        plan->sim_cid = sim << 4 | cid;
        plan->netif->sim_cid = sim << 4 | cid;
        if (sim == NAT_SIM && cid == NAT_CID)
            plan->netif->pdnType = CFW_GPRS_PDP_TYPE_PPP + 1;
        OSI_LOGI(0x10007b3a, "lan_setSimCid = %d pdnType = %d", plan->netif->sim_cid, plan->netif->pdnType);
    }
end:
    return;
}
uint8_t lan_create(void *lannetif, uint8_t sim, uint8_t cid, netif_init_fn init, netif_input_fn input)
{
    uint8_t iRet = false;
    struct lan_netif *plan = (struct lan_netif *)lannetif;
    if (plan == NULL)
        goto end;
    if ((plan->netif = nat_lan_netif_create(cid, sim, plan->iIdx, init, input)) == NULL)
        goto end;
    plan->netif->pstlan = plan;

    if (sim == NAT_SIM && cid == NAT_CID)
    {
        //add to list if without netwok
        lan_add2wan(lannetif, NULL);
    }
    iRet = true;
end:
    OSI_LOGI(0x10007b3b, "lan_create iRet = %d", iRet);
    return iRet;
}
void lan_destroy(void *lannetif)
{
    struct lan_netif *plan = (struct lan_netif *)lannetif;

#if LWIP_TCPIP_CORE_LOCKING
    LOCK_TCPIP_CORE();
#endif
    if (plan == NULL)
        goto end;
    struct netif *netif = plan->netif;
    if (netif == NULL)
        goto end;

    if ((netif = plan->netif) != NULL)
    {
        //remove wan table
        lan_removeNATEntry(plan);

        //remove netif
        netif_set_link_down(netif);
        netif_remove(netif);
        dns_clean_entries();
        //if (sim < MAX_SIM_ID && cid < MAX_CID)
        //{
        //    netif->is_used = 0;
        //}
        //else
        //{
        free(netif);
        //}
        OSI_LOGI(0x10007af1, "TCPIP_nat_lan_lwip_netif_destory netif");
        tcp_debug_print_pcbs();

        OSI_LOGI(0x10009817, "lan_destroy plan =%p", plan);
        //free lan
        free(plan);
    }
end:
#if LWIP_TCPIP_CORE_LOCKING
    UNLOCK_TCPIP_CORE();
#endif
    return;
}

uint8_t lan_addNATEntry(void *lannetif, void *wannetif)
{
    uint8_t iRet = false;
    struct wan_netif *pWan = (struct wan_netif *)wannetif;
    struct lan_netif *plan = (struct lan_netif *)lannetif;
    OSI_LOGI(0x10007b3c, "lan_addNATEntry wannetif = %p", wannetif);
    if (pWan == NULL || plan == NULL)
        goto end;

    struct netif *lan_netif = plan->netif;
    struct netif *wan_netif = pWan->netif;

    if (lan_netif == NULL || wan_netif == NULL)
        goto end;

    ip4_addr_t *wan_ip4_addr = (ip4_addr_t *)netif_ip4_addr(wan_netif);
    ip4_addr_t *netif_ip4 = ip_2_ip4(&(lan_netif->ip_addr));

    //add nat entry for lan netif
    ip4_nat_entry_t *new_nat_entry = (ip4_nat_entry_t *)calloc(1, sizeof(ip4_nat_entry_t));
    if (new_nat_entry == NULL)
        goto end;

    new_nat_entry->in_if = plan->netif;
    new_nat_entry->out_if = pWan->netif;
    ip4_addr_copy(new_nat_entry->dest_net, *wan_ip4_addr);
    IP4_ADDR(&(new_nat_entry->dest_netmask), 255, 255, 255, 255);

    netif_ip4 = ip_2_ip4(&(lan_netif->ip_addr));
    ip4_addr_copy(new_nat_entry->source_net, *netif_ip4);
    IP4_ADDR(&(new_nat_entry->source_netmask), 255, 255, 255, 255);
    ip4_nat_add(new_nat_entry);
    lan_netif->nat_entry = new_nat_entry;

    /* add this netif to the wan lanlist */
    plan->next = pWan->lan_netiflist;
    pWan->lan_netiflist = plan;
    OSI_LOGI(0x10009818, "lan_addNATEntry pWan->lan_netiflist = %p plan->next = %p", pWan->lan_netiflist, plan->next);
    pWan->lan_num++;
    iRet = true;
end:
    OSI_LOGI(0x10007b3d, "lan_addNATEntry iRet = %d", iRet);
    return iRet;
}
uint8_t lan_removeNATEntry(void *lannetif)
{
    uint8_t iRet = false;
    struct lan_netif *plan = (struct lan_netif *)lannetif;
    if (plan == NULL)
        goto end;
    struct netif *netif = plan->netif;
    if (netif == NULL)
        goto end;
#if LWIP_TCPIP_CORE_LOCKING
    LOCK_TCPIP_CORE();
#endif
    ip4_nat_entry_t *new_nat_entry = netif->nat_entry;
    if (new_nat_entry != NULL)
    {
        netif->nat_entry = NULL;
        ip4_nat_remove(new_nat_entry);
        free(new_nat_entry);
        iRet = true;
    }
#if LWIP_TCPIP_CORE_LOCKING
    UNLOCK_TCPIP_CORE();
#endif
end:
    OSI_LOGI(0x10007b3e, "lan_removeNATEntry iRet = %d", iRet);
    return iRet;
}

uint8_t lan_add2wan(void *lannetif, void *wannetif)
{
    uint8_t iRet = false;
    struct wan_netif *pwan = (struct wan_netif *)wannetif;
    struct lan_netif *plan = (struct lan_netif *)lannetif;
    OSI_LOGI(0x10007b3f, "lan_add2wan wannetif = %p", wannetif);

    if (plan == NULL)
        goto end;

    /* if no active pdp */
    if (pwan == NULL)
    {
        plan->next = gLan_Netiflist_Nopdp;
        gLan_Netiflist_Nopdp = plan;
        OSI_LOGI(0x10009819, "lan_add2wan gLan_Netiflist_Nopdp = %p plan->next = %p", gLan_Netiflist_Nopdp, plan->next);
    }
    else
    {
        //add new wan table
        OSI_LOGI(0x1000981a, "lan_add2wan add lan %p to wan %p", plan, pwan);
        lan_addNATEntry(plan, pwan);
    }
    iRet = true;
end:
    OSI_LOGI(0x10007b3e, "lan_removeNATEntry iRet = %d", iRet);
    return iRet;
}

uint8_t lan_remove2wan(void *lannetif, void *wannetif)
{
    uint8_t iRet = false;
    struct wan_netif *pwan = (struct wan_netif *)wannetif;
    struct lan_netif *plan = (struct lan_netif *)lannetif;
    struct lan_netif *plist = NULL;
    OSI_LOGI(0x10007b41, "lan_remove2wan wannetif = %p", wannetif);

    if (plan == NULL)
        goto end;

    if (pwan == NULL)
    {
        plist = gLan_Netiflist_Nopdp;
        OSI_LOGI(0x1000981b, "lan_remove2wan return plist %p, plist->next = %p", plist, plist->next);
    }
    else
    {
        plist = pwan->lan_netiflist;
    }

    lan_removeNATEntry(plan);
    //pwan->lan_netif[iIdx] = NULL;

    if (plist == NULL)
    {
        goto end; /* netif is not on the list */
    }

    /*  is it the first netif? */
    if (plist == plan)
    {
        if (pwan == NULL)
        {
            gLan_Netiflist_Nopdp = plan->next;
            OSI_LOGI(0x1000981c, "lan_remove2wan first gLan_Netiflist_Nopdp = %p", gLan_Netiflist_Nopdp);
        }
        else
        {
            pwan->lan_netiflist = plan->next;
            OSI_LOGI(0x1000981d, "lan_remove2wan first plist = %p", plist);
        }
    }
    else
    {
        /*  look for netif further down the list */
        struct lan_netif *tmplan_netif;
        for (tmplan_netif = plist; tmplan_netif != NULL; tmplan_netif = tmplan_netif->next)
        {
            OSI_LOGI(0x1000981e, "lan_remove2wan tmplan_netif->next = %p plan = %p", tmplan_netif, plan);
            if (tmplan_netif->next == plan)
            {
                tmplan_netif->next = plan->next;
                OSI_LOGI(0x1000981f, "lan_remove2wan tmplan_netif = %p", tmplan_netif);
                break;
            }
        }
    }
    iRet = true;
end:
    OSI_LOGI(0x10007b42, "lan_remove2wan iRet = %d", iRet);
    return iRet;
}

uint8_t lan_GetLinkModeIdx(void *lannetif)
{
    uint8_t iIdx = NAT_MAX_LANNUM;
    struct lan_netif *plan = (struct lan_netif *)lannetif;
    if (plan == NULL)
        goto end;

    iIdx = plan->iIdx;
end:
    OSI_LOGI(0x10007b43, "LinkModeSwitch2 Idx %d\n", iIdx);
    return iIdx;
}
static struct lan_netif *getLanFromLanList(uint8_t nSimId, uint8_t nCid, uint8_t nType, struct lan_netif *planlist)
{
    struct lan_netif *plan = NULL;

    for (plan = planlist; plan != NULL; plan = plan->next)
    {
        if (plan->sim_cid == (nSimId << 4 | nCid) && plan->iIdx == nType)
            break;
    }

    return plan;
}

/* for TBOX project return first LAN pointer of this Link Mode
*/
struct lan_netif *getLan(uint8_t nSimId, uint8_t nCid, uint8_t nIdx)
{
    struct lan_netif *plan = NULL;
    OSI_LOGI(0x10007b44, "getLan SimId = %d nCid = %d nIdx = %d", nSimId, nCid, nIdx);
    //check sim cid
    if (nSimId >= MAX_SIM_ID || nCid >= MAX_CID || nIdx >= NAT_MAX_LANNUM)
    {
        if ((nSimId == NAT_SIM && nCid == NAT_CID) && nIdx < NAT_MAX_LANNUM)
        {
            /* find in gLan_Netiflist_Nonet*/
            plan = getLanFromLanList(nSimId, nCid, nIdx, gLan_Netiflist_Nopdp);
        }
    }
    else
    {
        /* find from lanlist of wan*/
        struct wan_netif *pwan = NULL;
        if ((pwan = getWan(nSimId, nCid)) != NULL)
            plan = getLanFromLanList(nSimId, nCid, nIdx, pwan->lan_netiflist);
    }
    OSI_LOGI(0x10007b45, "getLan plan = %p", plan);
    return plan;
}
static void Lan_init(struct lan_netif *plan, uint8_t nSimId, uint8_t nCid, uint8_t nLinkMode)
{
    memset(plan, 0, sizeof(struct lan_netif));
    plan->sim_cid = nSimId << 4 | nCid;
    plan->iIdx = nLinkMode;
    plan->isUsing = 1;

    OSI_LOGI(0x10007b46, "initLan");
}
struct lan_netif *newLan(uint8_t nSimId, uint8_t nCid, uint8_t nLinkMode)
{
    struct lan_netif *plan = NULL;
    OSI_LOGI(0x10007b47, "newLan %d %d %d", nSimId, nCid, nLinkMode);
    if (nLinkMode > NAT_MAX_LANNUM)
        goto end;

    plan = malloc(sizeof(struct lan_netif));
    if (plan == NULL)
        goto end;
    Lan_init(plan, nSimId, nCid, nLinkMode);

end:
    OSI_LOGI(0x10007b48, "newLan plan = %p", plan);
    return plan;
}
bool wan_setPriority(uint8_t nSimId, uint8_t nCid, uint8_t nPriority)
{
    int8_t iRet = false;
    struct wan_netif *pwan = NULL;

    if (nSimId >= MAX_SIM_ID || nCid >= MAX_CID || nPriority >= MAX_CID)
        goto leave;

    if ((pwan = wan[nSimId][nCid]) != NULL)
    {
        //if (pwan->act == 1)
        {
            pwan->priority = nPriority;
            OSI_LOGI(0x10009820, "new Priority wan[%d][%d] = %p", pwan, nSimId, nCid, nPriority);
        }
    }
    wan_priority[nSimId][nCid] = nPriority;
    iRet = true;
leave:
    return iRet;
}

uint8_t wan_getPriority(uint8_t nSimId, uint8_t nCid)
{
    int8_t iRet = 0;
    if (nSimId >= MAX_SIM_ID || nCid >= MAX_CID)
        goto leave;
    iRet = wan_priority[nSimId][nCid];
leave:
    return iRet;
}

struct wan_netif *wan_getHighestPriorityActivateWan(void)
{
    struct wan_netif *pwan = NULL;
    uint8_t i, j;
    uint8_t iPriority = 0;

    for (i = 0; i < MAX_SIM_ID; i++)
    {
        for (j = 0; j < MAX_CID; j++)
        {
            if ((pwan = wan[i][j]) != NULL)
            {
                if (pwan->act == 1)
                {
                    if (pwan->priority > iPriority)
                    {
                        iPriority = pwan->priority;
                    }
                }
            }
        }
    }
    //end:
    OSI_LOGI(0x10009821, "wan_getHighestPriorityActivateWan pwan = %p", pwan);
    return pwan;
}
uint8_t wan_create(void *wannetif, uint8_t sim, uint8_t cid)
{
    uint8_t iRet = false;
    struct wan_netif *pwan = (struct wan_netif *)wannetif;

    if ((pwan->netif = TCPIP_nat_wan_netif_create(cid, sim)) == NULL)
        goto end;

    //set default is NULL when first wan is created
    if (netif_default != NULL && wan_isUnique(sim, cid) == true)
        netif_set_default(NULL);

    pwan->priority = wan_priority[sim][cid];
    //add lwip lan
    struct lan_netif *plan = pwan->lan_netiflist;
    if (plan == NULL)
    {
        plan = newLan(sim, cid, NAT_LAN_IDX_LWIP);
        if (plan != NULL)
        {
            if (lan_create(plan, sim, cid, netif_gprs_nat_lan_lwip_init, nat_lan_lwip_tcpip_input) != true)
                goto end;
            lan_add2wan(plan, pwan);
            if (netif_default == NULL)
                lan_setDefaultNetif(plan->netif);
        }
    }

    wan_bindLan(pwan);

    iRet = true;
end:
    OSI_LOGI(0x10007b4c, "wan_create iRet = %d", iRet);
    return iRet;
}
void wan_destroy(void *wannetif, uint8_t sim, uint8_t cid)
{
    struct wan_netif *pwan = (struct wan_netif *)wannetif;
    struct netif *netif = NULL;
    if (pwan == NULL)
        goto end;
    netif = pwan->netif;
    if (netif != NULL)
    {
#if LWIP_TCPIP_CORE_LOCKING
        LOCK_TCPIP_CORE();
#endif
#ifndef CONFIG_NET_LTE_PSPACKET_SUPPORT
        drvPsIntf_t *dataPsPath = netif->pspathIntf;
        drvPsIntfClose(dataPsPath);
#endif
        netif_set_link_down(netif);
        netif_remove(netif);
        dns_clean_entries();
        if (sim < MAX_SIM_ID && cid < MAX_CID)
        {
            netif->is_used = 0;
        }
        else
        {
            free(netif);
        }
        OSI_LOGI(0x10007b30, "TCPIP_nat_wan_netif_destory netif");
        tcp_debug_print_pcbs();


        pwan->netif = NULL;

#if LWIP_TCPIP_CORE_LOCKING
        UNLOCK_TCPIP_CORE();
#endif
    }
    else
    {
        OSI_LOGI(0x1000754c, "Error ########### CFW_GPRS_DEACTIVED can't find netif for CID=0x%x\n", SIM_CID(sim, cid));
    }
end:
    return;
}
struct wan_netif *wan_getOneActivateWan(void)
{
    struct wan_netif *pwan = NULL;
    uint8_t i, j;

    for (i = 0; i < MAX_SIM_ID; i++)
    {
        for (j = 0; j < MAX_CID; j++)
        {
            if (wan[i][j] != NULL)
            {
                if (wan[i][j]->act == 1)
                {
                    pwan = wan[i][j];
                    break;
                }
            }
        }
    }
    //end:
    OSI_LOGI(0x10007b4d, "getOneActivateWan pwan = %p", pwan);
    return pwan;
}
void wan_unbindLan(void *wannetif, uint8_t nSimId, uint8_t nCid)
{
    uint8_t iUnique = 0;
    struct lan_netif *act_plan = NULL;
    struct lan_netif *plan = NULL;
    struct wan_netif *pwan = (struct wan_netif *)wannetif;
    if (pwan == NULL)
        goto end;
    OSI_LOGI(0x10007b4e, "wan_unbindLan nSimId = %d, nCid = %d", nSimId, nCid);
    iUnique = wan_isUnique(nSimId, nCid);

    //delete lan
    OSI_LOGI(0x10009822, "wan_unbindLan return pwan->lan_netiflist = %p", pwan->lan_netiflist);
    while (pwan->lan_netiflist != NULL)
    {
        plan = pwan->lan_netiflist;
        uint8_t nType = 0;
        OSI_LOGI(0x10009823, "wan_unbindLan return plan %p, plan->next = %p", plan, plan->next);
        if (plan != NULL)
        {
            nType = plan->iIdx;
            OSI_LOGI(0x10009824, "wan_unbindLan pwan->iIdx = %d", plan->iIdx);
            lan_remove2wan(plan, pwan);
            struct lan_netif *pplan = plan;

            /* process this pplan */
            if (nType == NAT_LAN_IDX_LWIP || nType == NAT_LAN_IDX_PPP)
            {
                if (netif_default == pplan->netif)
                    netif_set_default(NULL);
                OSI_LOGI(0x10009825, "wan_unbindLan destroyWan remove lan %d", pplan->iIdx);
                lan_destroy(pplan);
            }
            pwan->lan_num--;

            if (iUnique == true)
            {
                if (nType == NAT_LAN_IDX_RNDIS)
                {
                    lan_setSimCid(pplan, NAT_SIM, NAT_CID);
                    /* add to lanlist without pdp */
                    lan_add2wan(pplan, NULL);
                }
            }
            else
            {
                //add lan to other activate wan
                struct wan_netif *act_wan = wan_getOneActivateWan(); //wan_getHighestPriorityActivateWan();
                if (act_wan != NULL && act_wan->netif != NULL)
                {
                    if (act_wan->priority != 0)
                    {
                        if (nType != NAT_LAN_IDX_LWIP)
                            lan_add2wan(pplan, act_wan);
                    }
                    else
                    {
                        if (netif_default == NULL)
                        {
                            lan_setDefaultNetif(act_plan);
                        }
                    }
                }
            }
        }
    }
    OSI_LOGI(0x10007b51, "destroyWan pwan->lan_num %d", pwan->lan_num);
end:
    return;
}
void wan_bindLan(void *wannetif)
{
    struct wan_netif *pwan = (struct wan_netif *)wannetif;
    struct lan_netif *plan = NULL;
    struct netif *wan_netif = pwan->netif;
    uint8_t nSimId, nCid;
#if LWIP_IPV6
    //ip6_addr_t ip6 = {0};
#endif

    if (wan_netif == NULL)
        goto end;
    else
    {
        nSimId = pwan->netif->sim_cid >> 4;
        nCid = pwan->netif->sim_cid & 0x0f;
    }
    uint8_t T_cid = nSimId << 4 | nCid;

    ip4_addr_t *wan_ip4_addr = (ip4_addr_t *)netif_ip4_addr(wan_netif);
    OSI_LOGI(0x10007aec, "TCPIP_nat_lan_lwip_netif_create wan_ip4_addr 0x%x\n", wan_ip4_addr->addr);
    ip4_addr_t ip4_netMast;
    IP4_ADDR(&ip4_netMast, 255, 255, 255, 0);

    CFW_GPRS_PDPCONT_INFO_V2 pdp_context = {0};
    CFW_GprsGetPdpCxtV2(nCid, &pdp_context, nSimId);
    OSI_LOGI(0x10007aee, "TCPIP_nat_lan_lwip_netif_create pdp_context.PdnType %d\n", pdp_context.PdnType);

    //find lan
    struct netif *plan_netif = NULL;
    plan = gLan_Netiflist_Nopdp;
    while (plan != NULL)
    {
        OSI_LOGI(0x10009826, "wan_bindLan return gLan_Netiflist_Nopdp plan %p, plan->next = %p", plan, plan->next);
        plan_netif = plan->netif;
        if (plan_netif == NULL)
        {
            plan = plan->next;
            continue;
        }

        struct lan_netif *pplan = NULL;
        pplan = plan;

        plan = plan->next;

        if ((plan_netif->pdnType == CFW_GPRS_PDP_TYPE_PPP + 1) && (plan_netif->link_mode != NETIF_LINK_MODE_NAT_LWIP_LAN))
        {
            OSI_LOGI(0x10007b53, "return lan_netif %p, T_cid = %d", plan_netif, T_cid);

            //cid problem
            plan_netif->sim_cid = T_cid;
            pplan->sim_cid = T_cid;
            plan_netif->pdnType = pdp_context.PdnType;

            //remove from list without pdp
            lan_remove2wan(pplan, NULL);

            if (lan_add2wan(pplan, pwan) == true)
            {
                if (pplan->iIdx == NAT_LAN_IDX_RNDIS)
                    ND6_Lan_Send_RS(plan_netif, wan_netif);
            }
            OSI_LOGI(0x10007b54, "pwan->lan_num %d", pwan->lan_num);
        }
    }
end:
    return;
}
struct wan_netif *getWan(uint8_t nSimId, uint8_t nCid)
{
    struct wan_netif *pwan = NULL;
    //check sim cid
    if (nSimId >= MAX_SIM_ID || nCid >= MAX_CID)
        goto end;

    pwan = wan[nSimId][nCid];
end:
    OSI_LOGI(0x10007b55, "getWan pwan = %p", pwan);
    return pwan;
}
static void Wan_init(struct wan_netif *pwan)
{
    memset(pwan, 0, sizeof(struct wan_netif));

    pwan->act = 1;

    OSI_LOGI(0x10007b56, "initWan");
}
struct wan_netif *newWan(uint8_t nSimId, uint8_t nCid)
{
    struct wan_netif *pwan = NULL;

    //check sim cid
    if (nSimId >= MAX_SIM_ID || nCid >= MAX_CID)
        goto end;

    pwan = wan[nSimId][nCid];
    if (pwan == NULL)
    {
        pwan = malloc(sizeof(struct wan_netif));
        if (pwan == NULL)
            goto end;
        wan[nSimId][nCid] = pwan;
    }
    else
    {
        if (wan[nSimId][nCid]->act == 1)
            OSI_LOGI(0x10007b57, "newWan pwan = %p", pwan);
        else
            pwan = wan[nSimId][nCid];
    }

    Wan_init(pwan);
    wan_create(pwan, nSimId, nCid);

end:
    OSI_LOGI(0x10007b57, "newWan pwan = %p", pwan);
    return pwan;
}

void destroyWan(uint8_t nSimId, uint8_t nCid)
{
    //check sim cid
    if (nSimId >= MAX_SIM_ID || nCid >= MAX_CID)
        goto end;

    struct wan_netif *pwan = wan[nSimId][nCid];
    if (pwan == NULL)
        goto end;

    //set act status as deactivate
    pwan->act = 0;
    int iUniqe = wan_isUnique(nSimId, nCid);

    wan_unbindLan(pwan, nSimId, nCid);
    wan_destroy(pwan, nSimId, nCid);

    //set default netif
    struct lan_netif *plan = NULL;
    struct lan_netif *plan_lwip = NULL;
    if (iUniqe == true)
    {
        plan = getLan(NAT_SIM, NAT_CID, NAT_LAN_IDX_RNDIS);     //getAvailableRndisLan(nSimId, nCid);
        plan_lwip = getLan(NAT_SIM, NAT_CID, NAT_LAN_IDX_LWIP); //lan_internal[NAT_LAN_IDX_LWIP];
        if (plan != NULL && plan_lwip->netif != NULL)
            lan_setDefaultNetif(plan_lwip);
        else
            netif_set_default(NULL);
    }

end:
    OSI_LOGI(0x10007b58, "destroyWan");
    return;
}
#endif
