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
#include "at_cfw.h"
#include "lwipports.h"
#include "lwip/prot/ip.h"
#if LWIP_IPV6
#include "lwip/prot/ip6.h"
#endif
#ifdef CONFIG_NET_LTE_PSPACKET_SUPPORT
#include "ppp_interface.h"
#include "netmain.h"
#include "netutils.h"
#endif
#include "drv_config.h"

#define SIM_CID(sim, cid) ((((sim)&0xf) << 4) | ((cid)&0xf))

extern osiThread_t *netGetTaskID(void);
extern struct netif *getGprsNetIf(uint8_t nSim, uint8_t nCid);
extern uint32_t *getDNSServer(uint8_t nCid, uint8_t nSimID);
extern uint32_t do_send_stored_packet(uint8_t nCID, CFW_SIM_ID nSimID);
extern void dns_clean_entries(void);
extern err_t ppp_netif_output_ip4(struct netif *netif, struct pbuf *pb, const ip4_addr_t *ipaddr);
#if LWIP_IPV6
err_t ppp_netif_output_ip6(struct netif *netif, struct pbuf *pb, const ip6_addr_t *ipaddr);
#endif

#define MAX_SIM_ID 2
#if defined(CONFIG_SOC_8811) || defined(CONFIG_SOC_8821)
#define MAX_CID 7
#else
#define MAX_CID 7
#endif

#ifdef CONFIG_NET_LTE_PSPACKET_SUPPORT
#include "drv_md_ipc.h"
#define PS_UL_HDR_LEN IPC_PS_UL_HDR_LEN
#endif

struct netif gprs_netif[MAX_SIM_ID][MAX_CID] = {0};
struct netif *getGprsGobleNetIf(uint8_t nSim, uint8_t nCid)
{
    OSI_LOGI(0x10007537, "gprs_netif[%d][%d] = %p", nSim, nCid, &gprs_netif[nSim][nCid]);
    if (nSim >= MAX_SIM_ID || nCid >= MAX_CID)
        return NULL;
    return &gprs_netif[nSim][nCid];
}
#ifndef CONFIG_NET_LTE_PSPACKET_SUPPORT
static void gprs_data_ipc_to_lwip(void *ctx)
{
    struct netif *inp_netif = (struct netif *)ctx;
    if (inp_netif == NULL)
        return;
    struct pbuf *p, *q;
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
        len = readLen;
        OSI_LOGI(0x1000753a, "drvPsIntfRead out %d", len);
        if (len > 0)
        {
            sys_arch_dump(pData, len);
        }
        if (inp_netif != NULL && len > 0)
        {
#if defined(CONFIG_SOC_8811) || defined(CONFIG_SOC_8821)
            p = (struct pbuf *)pbuf_alloc(PBUF_RAW, len, PBUF_RAM);
#else
            p = (struct pbuf *)pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
#endif
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
                inp_netif->input(p, inp_netif);
                inp_netif->u32LwipDLSize += readLen;
            }
        }
    } while (readLen > 0);
    free(pData);
}

void lwip_pspathDataInput(void *ctx, drvPsIntf_t *p)
{
    OSI_LOGI(0x1000753b, "lwip_pspathDataInput osiThreadCallback in ");
    osiThreadCallback(netGetTaskID(), gprs_data_ipc_to_lwip, (void *)ctx);
    OSI_LOGI(0x1000753c, "lwip_pspathDataInput osiThreadCallback out");
}

err_t data_output(struct netif *netif, struct pbuf *p,
                  ip_addr_t *ipaddr)
{
    uint16_t offset = 0;
    struct pbuf *q = NULL;
    if (netif->link_mode == NETIF_LINK_MODE_NETCARD)
    {
        #if defined(CONFIG_FIBOCOM_BASE)
        OSI_LOGI(0, "[fibocom] error: data_output netif->link_mode = %d\n", netif->link_mode);
        #else
        OSI_LOGI(0x10009511, "data_output netif->link_mode = %d\n", netif->link_mode);
        #endif
        return !ERR_OK;
    }
#if 1
    OSI_LOGI(0x1000753d, "data_output ---------tot_len=%d, flags=0x%x---------", p->tot_len, p->flags);

    uint8_t *pData = malloc(p->tot_len);
    if (pData == NULL)
    {
        return !ERR_OK;
    }

    uint8_t flags = 0;
    uint8_t app_sn = 0;
    for (q = p; q != NULL; q = q->next)
    {
        memcpy(&pData[offset], q->payload, q->len);
        offset += q->len;
#if LWIP_RFACK
        if (q->app_sn != 0)
        {
            app_sn = q->app_sn;
        }
#endif
        flags |= q->flags >> 5;
    }

    OSI_LOGI(0x10009829, "data_output flags %d app_sn %d", flags, app_sn);
    sys_arch_dump(pData, p->tot_len);

    OSI_LOGI(0x1000753e, "drvPsIntfWrite--in---");
    extern bool NetifGetDPSDFlag(CFW_SIM_ID nSim);
#define GET_SIM(sim_cid) (((sim_cid) >> 4) & 0xf)

    if (!NetifGetDPSDFlag(GET_SIM(netif->sim_cid)))
        drvPsIntfWrite((drvPsIntf_t *)netif->pspathIntf, pData, p->tot_len, flags, app_sn);
    OSI_LOGI(0x1000753f, "drvPsIntfWrite--out---");
    netif->u32LwipULSize += p->tot_len;
    free(pData);
#endif

    OSI_LOGI(0x10007540, "data_output--return in netif_gprs---");

    return ERR_OK;
}
#endif
#ifdef CONFIG_NET_LTE_PSPACKET_SUPPORT
struct Gprs_Notity_t
{
    osiNotify_t *dl_read_notify;
    ps_buf_head_t buf_list;
    int16_t buf_list_count;
};
#define MAX_GPRS_BUF_COUNT 256

static struct Gprs_Notity_t gGPRSNotify = {0};

void psHeaderFreeAll(struct ps_header *psData)
{
    if (psData == NULL)
        return;
    struct ps_header *pNext = NULL;
    while (psData != NULL)
    {
        pNext = (struct ps_header *)psData->next;
        free(psData);
        psData = pNext;
    }
}
extern void gprs_ps_data_to_netcard(void *ctx);
extern void gprs_ps_data_to_mbim(void *ctx);
extern void gprs_ps_data_to_wan(void *ctx);
extern struct netif *getGprsWanNetIf(uint8_t nSim, uint8_t nCid);
extern struct netif *netif_get_by_cid(u8_t sim_cid);
extern bool get_nat_enabled(uint8_t nSimId, uint8_t nCid);
void _gprs_ps_data_to_lwip(void *ctx)
{
    //netdevIntf_t *nc = &gNetIntf;
    OSI_LOGI(0x1000a974, "_gprs_ps_data_to_lwip");
    uint8_t *pData = NULL;
    ps_buf_t *ps_buf = NULL;

    struct pbuf *p, *q;
    int len = 0;
    int readLen = 0;
    int offset = 0;

    struct ps_header *psData = NULL;
    while ((ps_buf = TAILQ_FIRST(&(gGPRSNotify.buf_list))) != NULL)
    {
        psData = ps_buf->psData;
#if LWIP_TCPIP_CORE_LOCKING
        LOCK_TCPIP_CORE();
#endif
        TAILQ_REMOVE(&(gGPRSNotify.buf_list), ps_buf, iter);
        if (gGPRSNotify.buf_list_count > 0)
            gGPRSNotify.buf_list_count--;
#if LWIP_TCPIP_CORE_LOCKING
        UNLOCK_TCPIP_CORE();
#endif
        //free iter
        free(ps_buf);

        struct netif *inp_netif = NULL;
#if IP_NAT
        if (get_nat_enabled(psData->simid, psData->cid))
        {
            inp_netif = getGprsWanNetIf(psData->simid, psData->cid);
            if (inp_netif == NULL)
            {
                free(psData);
                continue;
            }
            gprs_ps_data_to_wan(psData);
        }
        else
#endif
        {
            inp_netif = getGprsNetIf(psData->simid, psData->cid);
            if (inp_netif == NULL)
            {
                free(psData);
                continue;
            }
            readLen = psData->len;
            len = readLen;
            pData = ((uint8_t *)psData) + psData->data_off;

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

                inp_netif->input(p, inp_netif);
                inp_netif->u32LwipDLSize += readLen;
            }
            free(psData);
        }
    }
}

void gprs_to_lwip_datapull(struct ps_header *psData, struct netif *inp)
{
    ps_buf_t *ps_buf = (ps_buf_t *)malloc(sizeof(ps_buf_t));
    if (ps_buf == NULL)
    {
        psHeaderFreeAll(psData);
        return;
    }

    ps_buf->psData = psData;

    if (gGPRSNotify.dl_read_notify != NULL && gGPRSNotify.buf_list_count < MAX_GPRS_BUF_COUNT)
    {
#if LWIP_TCPIP_CORE_LOCKING
        LOCK_TCPIP_CORE();
#endif
        TAILQ_INSERT_TAIL(&(gGPRSNotify.buf_list), ps_buf, iter);
        gGPRSNotify.buf_list_count++;
        OSI_LOGI(0x1000a975, "gprs_to_lwip_datapull osiNotifyTrigger bufcount %d", gGPRSNotify.buf_list_count);
#if LWIP_TCPIP_CORE_LOCKING
        UNLOCK_TCPIP_CORE();
#endif
        osiNotifyTrigger(gGPRSNotify.dl_read_notify);
    }
    else
    {
        OSI_LOGI(0x1000a976, "gprs_to_lwip_datapull drop ps_bufer %x", psData);
        free(psData);
        free(ps_buf);
    }
}

static void gprs_ps_data_to_lwip(void *ctx)
{
    struct ps_header *psData = (struct ps_header *)ctx;
    if (psData == NULL)
        return;

    struct netif *inp_netif = NULL;
#if IP_NAT
    if (get_nat_enabled(psData->simid, psData->cid))
    {
        inp_netif = getGprsWanNetIf(psData->simid, psData->cid);
    }
    else
#endif
    {
        inp_netif = getGprsNetIf(psData->simid, psData->cid);
    }
    if (inp_netif == NULL)
    {
        psHeaderFreeAll(psData);
        return;
    }

    //struct pbuf *p, *q;
    OSI_LOGI(0x1000a977, "gprs_ps_data_to_lwip inp_netif = %p", inp_netif);
    uint8_t *pData = NULL;
    int len = 0;
    int readLen = 0;
    //int offset = 0;

    do
    {
        readLen = psData->len;
        len = readLen;
        pData = ((uint8_t *)psData) + psData->data_off;
        OSI_LOGI(0x1000a978, "psData buffer len %d", len);
        if (len > 0)
        {
            sys_arch_dump(pData, len);
        }
        if (inp_netif != NULL && len > 0)
        {
// modify by fibocom 20240522 for compile issue
#if defined(CONFIG_FIBOCOM_BASE) && defined(CONFIG_NET_LWIP_PPP_ON)
            if (inp_netif->link_mode == NETIF_LINK_MODE_PPP)
            {
                struct ps_header *psDataNext = (struct ps_header *)(psData->next);
                psData->next = 0;
                // insert to TAILQ
                gprs_to_ppp_datapull(psData, inp_netif);
                psData = psDataNext;
            }
            else
#endif
            {
                if (gGPRSNotify.dl_read_notify == NULL)
                {
                    gGPRSNotify.dl_read_notify = osiNotifyCreate(netGetTaskID(), _gprs_ps_data_to_lwip, NULL);
                    TAILQ_INIT(&(gGPRSNotify.buf_list));
                    gGPRSNotify.buf_list_count = 0;
                }
                struct ps_header *psDataNext = (struct ps_header *)(psData->next);
                psData->next = 0;
                // insert to TAILQ
                gprs_to_lwip_datapull(psData, inp_netif);
                psData = psDataNext;
            }
        }
    } while (psData != NULL);
}
void API_DLDataIpcHandle(struct ps_header *psData)
{
    OSI_LOGI(0x1000a979, "API_DLDataIpcHandle psData: %p", psData);
    if (psData == NULL)
        return;
    struct netif *inp_netif = NULL;
    uint8_t nSim = psData->simid;
    uint8_t nCid = psData->cid;

#if IP_NAT
    if (get_nat_enabled(nSim, nCid))
    {
        inp_netif = getGprsWanNetIf(nSim, nCid);
    }
    else
    {
        inp_netif = netif_get_by_cid(nSim << 4 | nCid);
    }
#else
    inp_netif = netif_get_by_cid(nSim << 4 | nCid);
#endif

    if (inp_netif == NULL)
    {
        psHeaderFreeAll(psData);
        return;
    }

    switch (inp_netif->link_mode)
    {
    case NETIF_LINK_MODE_LWIP:
    case NETIF_LINK_MODE_PPP:
        gprs_ps_data_to_lwip((void *)psData);
        break;
#ifdef CONFIG_USB_ETHER_SUPPORT
    case NETIF_LINK_MODE_NETCARD:
        gprs_ps_data_to_netcard((void *)psData);
        break;
#endif
#if IP_NAT
    case NETIF_LINK_MODE_NAT_WAN:
        gprs_ps_data_to_lwip((void *)psData);
        break;
#endif
    default:
        psHeaderFreeAll(psData);
        break;
    }
    //OSI_LOGI(0x1000a97a, "API_DLDataIpcHandle out");
}

extern void API_ULDataIpcHandle(struct ps_header **p_psIpcBufferList, uint16_t v_availLen);
extern void *osiMemalign(size_t alignment, size_t size);

int32_t GPRS_Daps_Upload_Data(uint8_t *data, uint32_t len, uint8_t nCid, uint8_t nSimId)
{
    if (data == NULL)
        return -1;

    struct netif *inp_netif = NULL;
    inp_netif = getGprsNetIf(nSimId, nCid);
    if (inp_netif == NULL)
        return -1;

    uint32_t ps_len = PS_UL_HDR_LEN + len + 28;
    uint32_t ps_len_align = OSI_ALIGN_UP(ps_len, 32); //32 align for cache
    struct ps_header *psHeader = (struct ps_header *)osiMemalign(32, ps_len_align);

    if (psHeader == NULL)
    {
        return -1;
    }

    memset(psHeader, 0, ps_len_align);
    psHeader->cid = nCid;
    psHeader->simid = nSimId;
    psHeader->len = len;
    psHeader->buf_size = ps_len;
    psHeader->data_off = PS_UL_HDR_LEN;
    uint8_t *p_ipData = (uint8_t *)psHeader + PS_UL_HDR_LEN;

    memcpy(p_ipData, data, len);
    gprs_ps_data_to_lwip((void *)psHeader);
    return len;
}

err_t data_output_ps_header(struct netif *netif, struct pbuf *p,
                            ip_addr_t *ipaddr)
{
    uint16_t offset = 0;
    struct pbuf *q = NULL;
    if (netif->link_mode == NETIF_LINK_MODE_NETCARD)
    {
        OSI_LOGI(0x1000a97b, "data_output_ps_header netif->link_mode = %d\n", netif->link_mode);
        return !ERR_OK;
    }

    OSI_LOGI(0x1000a97c, "data_output_ps_header ---------tot_len=%d, flags=0x%x---------", p->tot_len, p->flags);
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
    if (NETIF_LINK_MODE_LWIP == netif->link_mode)
    {
        netif->u32LwipULSize += p->tot_len;
    }
    if (NETIF_LINK_MODE_PPP == netif->link_mode)
    {
        netif->u32PPPULSize += p->tot_len;
    }

    OSI_LOGI(0x1000a97f, "data_output_ps_header--return in netif_gprs---");

    return ERR_OK;
}
#endif

static err_t netif_gprs_init(struct netif *netif)
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
        netif_set_default(netif);

    int nCid = netif->sim_cid & 0x0f;
    int nSimId = (netif->sim_cid & 0xf0) >> 4;

    CFW_GPRS_PDPCONT_INFO_V2 pdp_context = {0};
    ip_addr_t ip4_dns1 = {0};
    ip_addr_t ip4_dns2 = {0};
#if LWIP_IPV6
    ip_addr_t ip6_dns1 = {0};
    ip_addr_t ip6_dns2 = {0};
#endif
    CFW_GprsGetPdpCxtV2(nCid, &pdp_context, nSimId);

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
    dns_setserver(nSimId, nCid, 0, &ip4_dns1);
    dns_setserver(nSimId, nCid, 1, &ip4_dns2);
#if LWIP_IPV6
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
#endif

#if LWIP_IPV6
    if (pdp_context.PdnType == CFW_GPRS_PDP_TYPE_IPV6)
    {
        dns_setserver(nSimId, nCid, 0, &ip6_dns1);
        dns_setserver(nSimId, nCid, 1, &ip6_dns2);
    }
    if (pdp_context.PdnType == CFW_GPRS_PDP_TYPE_IPV4V6)
    {
        if (IP_IS_V6(&ip6_dns1))
            dns_setserver(nSimId, nCid, 1, &ip6_dns1);
#ifdef CONFIG_FIBOCOM_BASE
        OSI_PRINTFE("netif_gprs_init set IPV4 dns2 and IPV6 dns2 when IPV4V6");
        dns_setserver(nSimId, nCid, 2, &ip4_dns2);
        dns_setserver(nSimId, nCid, 3, &ip6_dns2);
#endif
    }
#endif

    return ERR_OK;
}

extern uint8_t CFW_GprsGetPdpIpv6Addr(uint8_t nCid, uint8_t *nLength, uint8_t *ipv6Addr, CFW_SIM_ID nSimID);
extern uint8_t CFW_GprsGetPdpIpv4Addr(uint8_t nCid, uint8_t *nLength, uint8_t *ipv4Addr, CFW_SIM_ID nSimID);
#if LWIP_IPV6
void setIp6address(struct netif *netif, CFW_GPRS_PDPCONT_INFO_V2 pdp_context)
{
    ip6_addr_t ip6 = {0};

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
        OSI_LOGXI(OSI_LOGPAR_S, 0x10007549, "TCPIP_netif_create IP6: %s", ip6addr_ntoa(&ip6));
    }
#endif
}
void TCPIP_netif_create(uint8_t nCid, uint8_t nSimId)
{
    struct netif *netif = NULL;
    uint8_t T_cid = nSimId << 4 | nCid;
    netif = getGprsNetIf(nSimId, nCid);
    if (netif != NULL)
    {
        OSI_LOGI(0x10007545, "TCPIP_netif_create netif already created cid %d simid %d\n", nCid, nSimId);
        return;
    }

    ip4_addr_t ip4 = {0};
    //uint8_t lenth;

    OSI_LOGI(0x10007546, "TCPIP_netif_create cid %d simid %d\n", nCid, nSimId);

    AT_Gprs_CidInfo *pCidInfo = &gAtCfwCtx.sim[nSimId].cid_info[nCid];

    if (pCidInfo->uCid != nCid)
    {
        pCidInfo->uCid = nCid;
    }

    CFW_GPRS_PDPCONT_INFO_V2 pdp_context = {0};
    CFW_GprsGetPdpCxtV2(nCid, &pdp_context, nSimId);
    OSI_LOGI(0x10007547, "TCPIP_netif_create pdp_context.PdnType %d\n", pdp_context.PdnType);
    if ((pdp_context.PdnType == CFW_GPRS_PDP_TYPE_IP) || (pdp_context.PdnType == CFW_GPRS_PDP_TYPE_IPV4V6) || (pdp_context.PdnType == CFW_GPRS_PDP_TYPE_X25) || (pdp_context.PdnType == CFW_GPRS_PDP_TYPE_PPP)) //add evade for nPdpType == 0, need delete later
    {
        IP4_ADDR(&ip4, pdp_context.pPdpAddr[0], pdp_context.pPdpAddr[1], pdp_context.pPdpAddr[2], pdp_context.pPdpAddr[3]);
#if defined(CONFIG_SOC_8811) || defined(CONFIG_SOC_8821)
        if (pdp_context.nPdpAddrSize == 12)
        {
            IP4_ADDR(&ip4, pdp_context.pPdpAddr[8], pdp_context.pPdpAddr[9], pdp_context.pPdpAddr[10], pdp_context.pPdpAddr[11]);
        }
#endif
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
    memset(netif, 0, sizeof(struct netif));
    netif->sim_cid = T_cid;
    netif->is_ppp_mode = 0;
    netif->link_mode = NETIF_LINK_MODE_LWIP;
    netif->pdnType = pdp_context.PdnType;
    netif->is_used = 1;
    netif->is_type = NETIF_LINK_TYPE_GPRS;
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
    netif_add(netif, &ip4, NULL, NULL, NULL, netif_gprs_init, tcpip_input);
#if LWIP_IPV6
    setIp6address(netif, pdp_context);
#endif
    netif_set_up(netif);
    netif_set_link_up(netif);

#if LWIP_TCPIP_CORE_LOCKING
    UNLOCK_TCPIP_CORE();
#endif
    OSI_LOGI(0x1000754a, "TCPIP_netif_create, netif->num: 0x%x\n", netif->num);
}

void TCPIP_netif_destory(uint8_t nCid, uint8_t nSimId)
{
    struct netif *netif = NULL;

    netif = getGprsNetIf(nSimId, nCid);
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
        OSI_LOGI(0x1000754b, "TCPIP_netif_destory netif");
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

void wifi_dns_setserver(u8_t numdns, const ip_addr_t *dnsserver) {}
void netif_reset_dns(struct netif *netif)
{
    OSI_LOGI(0x10007ae5, "netif_reset_dns/change dns_server");

    if (NULL == netif)
        return;

    int nCid = netif->sim_cid & 0x0f;
    int nSimId = (netif->sim_cid & 0xf0) >> 4;
    if (nSimId >= MAX_SIM_ID || nCid > MAX_CID)
        return;
    CFW_GPRS_PDPCONT_INFO_V2 pdp_context = {0};
    ip_addr_t ip4_dns1 = {0};
    ip_addr_t ip4_dns2 = {0};
#if LWIP_IPV6
    ip_addr_t ip6_dns1 = {0};
    ip_addr_t ip6_dns2 = {0};
#endif
    CFW_GprsGetPdpCxtV2(nCid, &pdp_context, nSimId);

    if ((pdp_context.PdnType == CFW_GPRS_PDP_TYPE_IP) || (pdp_context.PdnType == CFW_GPRS_PDP_TYPE_IPV4V6) || (pdp_context.PdnType == CFW_GPRS_PDP_TYPE_X25) ||
        (pdp_context.PdnType == CFW_GPRS_PDP_TYPE_PPP)) //add evade for nPdpType == 0, need delete later
    {
        if (pdp_context.nPdpDnsSize != 0)
        {
            IP_ADDR4(&ip4_dns1, pdp_context.pPdpDns[0], pdp_context.pPdpDns[1], pdp_context.pPdpDns[2], pdp_context.pPdpDns[3]);
            IP_ADDR4(&ip4_dns2, pdp_context.pPdpDns[21], pdp_context.pPdpDns[22], pdp_context.pPdpDns[23], pdp_context.pPdpDns[24]);
            OSI_LOGI(0x10007ae6, "netif_reset_dns/DNS size:%d  DNS1:%d.%d.%d.%d", pdp_context.nPdpDnsSize,
                     pdp_context.pPdpDns[0], pdp_context.pPdpDns[1], pdp_context.pPdpDns[2], pdp_context.pPdpDns[3]);
            OSI_LOGI(0x10007ae7, "netif_reset_dns/DNS2:%d.%d.%d.%d",
                     pdp_context.pPdpDns[21], pdp_context.pPdpDns[22], pdp_context.pPdpDns[23], pdp_context.pPdpDns[24]);
        }
    }
    dns_setserver(nSimId, nCid, 0, &ip4_dns1);
    dns_setserver(nSimId, nCid, 1, &ip4_dns2);
#if LWIP_IPV6
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
            OSI_LOGI(0x10007ae8, "netif_reset_dns/DNS size:%d  ipv6 DNS 12-15:%d.%d.%d.%d", pdp_context.nPdpDnsSize,
                     pdp_context.pPdpDns[12], pdp_context.pPdpDns[13], pdp_context.pPdpDns[14], pdp_context.pPdpDns[15]);
            OSI_LOGI(0x10007ae9, "netif_reset_dns/ipv6 DNS 33-36:%d.%d.%d.%d",
                     pdp_context.pPdpDns[33], pdp_context.pPdpDns[34], pdp_context.pPdpDns[35], pdp_context.pPdpDns[36]);
            IP_ADDR6(&ip6_dns1, addr0, addr1, addr2, addr3);
            IP_ADDR6(&ip6_dns2, addr4, addr5, addr6, addr7);
        }
    }

    if (pdp_context.PdnType == CFW_GPRS_PDP_TYPE_IPV6)
    {
        dns_setserver(nSimId, nCid, 0, &ip6_dns1);
        dns_setserver(nSimId, nCid, 1, &ip6_dns2);
    }
    if (pdp_context.PdnType == CFW_GPRS_PDP_TYPE_IPV4V6)
    {
        if (IP_IS_V6(&ip6_dns1))
            dns_setserver(nSimId, nCid, 1, &ip6_dns1);
#ifdef CONFIG_FIBOCOM_BASE
        OSI_PRINTFE("netif_reset_dns set IPV4 dns2 and IPV6 dns2 when IPV4V6");
        dns_setserver(nSimId, nCid, 2, &ip4_dns2);
        dns_setserver(nSimId, nCid, 3, &ip6_dns2);
#endif
    }
#endif
}

//
//Set "Discard PS Data" flag
//  TRUE: Discard ps data
//  FALSE: Do not Discard ps data
//
bool gNetifDPDFlag[CFW_SIM_COUNT] = {
    false,
};
void NetifSetDPSDFlag(bool bDPD, CFW_SIM_ID nSim)
{
    gNetifDPDFlag[nSim] = bDPD;
}
bool NetifGetDPSDFlag(CFW_SIM_ID nSim)
{
    return gNetifDPDFlag[nSim];
}

#if LWIP_RFACK
void NetifRfAck(uint8_t nCid, uint8_t nSimId, uint32_t frAckSeqno, uint16_t ackOk)
{
    struct netif *netif;
    uint8_t T_cid;
    netif = getGprsNetIf(nSimId, nCid);
    T_cid = nSimId << 4 | nCid;
    OSI_LOGI(0x1000982a, "NetifRfAck nCid=%d nSimId=%d frAckSeqno=%x  ackOk=%d netif=%x\n", nCid, nSimId, frAckSeqno, ackOk, netif);
    if (netif != NULL)
    {
        netif_gprs_data_rf_acked(netif, frAckSeqno, ackOk);
    }
    else
    {
        OSI_LOGI(0x1000982b, "Error ########### NetifRfAck can't find netif for CID=0x%x\n", T_cid);
    }
}
#endif

#ifdef CONFIG_LWIP_IMS_SUPPORT
#define IMS_CID 11
#define IMS_EMERG_CID 9
void IMS_GPRS_netif_destory(uint8_t nCid, uint8_t nSimId)
{
    struct netif *netif = NULL;

    netif = getGprsNetIf(nSimId, nCid);
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
        OSI_LOGI(0x1000a980, "IMS_GPRS_netif_destory netif");
#if LWIP_TCP
        tcp_debug_print_pcbs();
#endif
#if LWIP_TCPIP_CORE_LOCKING
        UNLOCK_TCPIP_CORE();
#endif
    }
    else
    {
        OSI_LOGE(0x1000a981, "Error ########### IMS_GPRS_netif_destory can't find netif for CID=0x%x\n", SIM_CID(nSimId, nCid));
    }
}

static err_t netif_ims_gprs_init(struct netif *netif)
{
    /* initialize the snmp variables and counters inside the struct netif
   * ifSpeed: no assumption can be made!
   */
    netif->name[0] = 'I';
    netif->name[1] = 'M';
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

    int nCid = netif->sim_cid & 0x0f;
    int nSimId = (netif->sim_cid & 0xf0) >> 4;

    if (nCid != IMS_CID && nCid != IMS_EMERG_CID)
    {
        if (NULL == netif_default)
            netif_set_default(netif);
    }

    CFW_GPRS_PDPCONT_INFO_V2 pdp_context = {0};
    ip_addr_t ip4_dns1 = {0};
    ip_addr_t ip4_dns2 = {0};
#if LWIP_IPV6
    ip_addr_t ip6_dns1 = {0};
    ip_addr_t ip6_dns2 = {0};
#endif
    CFW_GprsGetPdpCxtV2(nCid, &pdp_context, nSimId);

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
            dns_setserver(nSimId, nCid, 0, &ip4_dns1);
            dns_setserver(nSimId, nCid, 1, &ip4_dns2);
        }
    }

#if LWIP_IPV6
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
            if (pdp_context.PdnType == CFW_GPRS_PDP_TYPE_IPV6)
            {
                dns_setserver(nSimId, nCid, 0, &ip6_dns1);
                dns_setserver(nSimId, nCid, 1, &ip6_dns2);
            }
            if (pdp_context.PdnType == CFW_GPRS_PDP_TYPE_IPV4V6)
            {
                if (IP_IS_V6(&ip6_dns1))
                    dns_setserver(nSimId, nCid, 1, &ip6_dns1);
            }
        }
    }
#endif

    return ERR_OK;
}

void IMS_GPRS_netif_create(uint8_t nCid, uint8_t nSimId)
{
    struct netif *netif = NULL;
    uint8_t T_cid = nSimId << 4 | nCid;
    netif = getGprsNetIf(nSimId, nCid);
    if (netif != NULL)
    {
        OSI_LOGI(0x1000a982, "IMS_GPRS_netif_create netif already created cid %d simid %d\n", nCid, nSimId);
        return;
    }

    ip4_addr_t ip4 = {0};

    OSI_LOGI(0x1000a983, "IMS_GPRS_netif_create cid %d simid %d\n", nCid, nSimId);

    AT_Gprs_CidInfo *pCidInfo = &gAtCfwCtx.sim[nSimId].cid_info[nCid];

    if (pCidInfo->uCid != nCid)
    {
        pCidInfo->uCid = nCid;
    }

    CFW_GPRS_PDPCONT_INFO_V2 pdp_context = {0};
    CFW_GprsGetPdpCxtV2(nCid, &pdp_context, nSimId);
    OSI_LOGI(0x1000a984, "IMS_GPRS_netif_create pdp_context.PdnType %d\n", pdp_context.PdnType);
    if ((pdp_context.PdnType == CFW_GPRS_PDP_TYPE_IP) || (pdp_context.PdnType == CFW_GPRS_PDP_TYPE_IPV4V6) || (pdp_context.PdnType == CFW_GPRS_PDP_TYPE_X25) || (pdp_context.PdnType == CFW_GPRS_PDP_TYPE_PPP)) //add evade for nPdpType == 0, need delete later
    {
        IP4_ADDR(&ip4, pdp_context.pPdpAddr[0], pdp_context.pPdpAddr[1], pdp_context.pPdpAddr[2], pdp_context.pPdpAddr[3]);
        OSI_LOGXI(OSI_LOGPAR_S, 0x1000a985, "IMS_GPRS_netif_create IP4: %s", ip4addr_ntoa(&ip4));
    }
    if (nSimId < MAX_SIM_ID && nCid < MAX_CID)
    {
        netif = &gprs_netif[nSimId][nCid];
    }
    else
    {
        netif = malloc(sizeof(struct netif));
        if (netif == NULL)
        {
            OSI_LOGE(0x1000a986, "IMS_GPRS_netif_create outof memory ");
            return;
        }
    }
    memset(netif, 0, sizeof(struct netif));
    netif->sim_cid = T_cid;
    netif->is_ppp_mode = 0;
    netif->link_mode = NETIF_LINK_MODE_LWIP;
    netif->pdnType = pdp_context.PdnType;
    netif->is_used = 1;
    netif->is_type = NETIF_LINK_TYPE_IMS;
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
    netif_add(netif, &ip4, NULL, NULL, NULL, netif_ims_gprs_init, tcpip_input);

#if LWIP_IPV6
    setIp6address(netif, pdp_context);
#endif
    netif_set_up(netif);
    netif_set_link_up(netif);

#if LWIP_TCPIP_CORE_LOCKING
    UNLOCK_TCPIP_CORE();
#endif
    OSI_LOGI(0x1000a987, "IMS_GPRS_netif_create, netif->num: 0x%x\n", netif->num);

#ifdef CONFIG_NET_IMSHTTP_LIB_SUPPORT
    extern void HTTP_Init(void);
    HTTP_Init();
#endif
}
#else
void IMS_GPRS_netif_destory(uint8_t nCid, uint8_t nSimId)
{
}

void IMS_GPRS_netif_create(uint8_t nCid, uint8_t nSimId)
{
}
#endif
