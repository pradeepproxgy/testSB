/* Copyright (C) 2018 RDA Technologies Limited and/or its affiliates("RDA").
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

// #define OSI_LOCAL_LOG_LEVEL OSI_LOG_LEVEL_DEBUG
#define OSI_LOCAL_LOG_TAG OSI_MAKE_LOG_TAG('P', 'P', 'P', 'S')

#include "osi_log.h"
#include "osi_api.h"
#include "hal_chip.h"

#include "ppp_interface.h"
#include "lwipopts.h"
#ifdef CONFIG_NET_LWIP_PPP_ON
#include "sockets.h"
#include "netif/ppp/pppos.h"
#include "drv_ps_path.h"
#include "netif.h"
#include "lwip/prot/ip.h"
#include "lwip/prot/ip6.h"
#if PPP_AUTHGPRS_SUPPORT
#include "netif/ppp/chap-new.h"
#include "netif/ppp/upap.h"
#include "netif/ppp/ppp_impl.h"
#endif
#include "at_cfw.h"
#include "netmain.h"
#ifdef CONFIG_FIBOCOM_BASE
#include "ffw_nv.h"
#endif
#if IP_NAT
#include "lwip/ip4_nat.h"
#include "netutils.h"
#endif
#ifdef CONFIG_NET_LTE_PSPACKET_SUPPORT
#include "drv_md_ipc.h"
#define MAX_PPP_BUF_COUNT 256
#endif
#define DUMP_MAX_SIZE (256)
#define DUMP_SIZE(size) ((size) > DUMP_MAX_SIZE ? DUMP_MAX_SIZE : (size))
extern bool AT_GetOperatorDefaultApn(uint8_t pOperatorId[6], CFW_APNS_UNAME_UPWD *apn);
extern void lwip_pspathDataInput(void *ctx, drvPsIntf_t *p);
extern struct netif *getGprsNetIf(uint8_t nSim, uint8_t cid);
extern err_t ppp_netif_output_ip4(struct netif *netif, struct pbuf *pb, const ip4_addr_t *ipaddr);
#ifdef CONFIG_FIBOCOM_BASE
extern bool fibo_set_dcd_status(uint8_t linkName, bool isActive);
atCmdEngine_t *ppp_engine = NULL;
static osiTimer_t *ppp_deact_timer = NULL;
void fibocom_exit_ppp();
void fibocom_exit_ppp_without_pdp_deact();
extern void fibo_lpg_ppp_set(bool ppp_open);
#endif
#if LWIP_IPV6
err_t ppp_netif_output_ip6(struct netif *netif, struct pbuf *pb, const ip6_addr_t *ipaddr);
#endif
extern void start_network_phase(ppp_pcb *pcb);
#if IP_NAT
extern u8_t ip4_wan_forward(struct pbuf *p, struct netif *inp);
extern u8_t netif_num;
typedef TAILQ_ENTRY(ppp_buf) ppp_buf_iter_t;
typedef TAILQ_HEAD(ppp_buf_head, ppp_buf) ppp_buf_head_t;
#define MAX_PPP_DL_LIT_PACK_NUM 256
#define MAX_PPP_DL_BIG_PACK_NUM 128
static int sPPPDLitPackNum = 0;
static int sPPPDBigPackNum = 0;
typedef struct ppp_buf
{
    ppp_buf_iter_t iter;
    void *buf;
} ppp_buf_t;
#endif
#ifndef CONFIG_NET_LTE_PSPACKET_SUPPORT
static void _psIntfRead(void *ctx);
#endif
extern bool isRAPackage(struct pbuf *pb);
extern void RA_reply(struct pbuf *pb);
extern void pppDhcp6_Info_req_reply(struct netif *netif, struct pbuf *pb);
extern bool isDhcp6infoReqPackage(struct pbuf *pb);

#ifdef CONFIG_WDT_ENABLE
extern void drvFeedWdt(void);
#endif

struct pppSession
{
    ppp_pcb *pcb;
    netif_output_fn old_output;
#if LWIP_IPV6
    netif_output_ip6_fn old_outputt_ip6;
#endif
    drvPsIntfDataArriveCB_t old_psintf_cb;
    pppOutputCallback_t output_cb;
    pppEndCallback_t end_cb;
    pppFlowControlCallback_t flowctrl_cb;
    void *end_cb_ctx;
    void *output_cb_ctx;
    void *flowctrl_cb_ctx;
    struct netif *nif;
    u8_t cid;
    u8_t sim;
    osiNotify_t *dl_read_notify;
    osiThread_t *dl_thread;
    int is_deleted;
    int cgact_activated;
    int uti_attact;
    int retrycnt_attact;
#if IP_NAT
    ppp_buf_head_t buffer_list;
    ip4_nat_entry_t ppp_nat_entry;
    osiTimer_t *dl_read_notify_timer;
#endif
#ifdef CONFIG_NET_LTE_PSPACKET_SUPPORT
    ps_buf_head_t buf_list;
    int16_t buf_list_count;
#endif
};

#if IP_NAT
static void _ppp_read_notify_timeout(void *ctx)
{
    struct netif *nif = (struct netif *)ctx;
    if (nif->state == NULL)
    {
        OSI_LOGI(0x10007b01, "_ppp_lan_data_pull nif->state");
        return;
    }

    ppp_pcb *pcb = (ppp_pcb *)nif->state;
    if (pcb->ppp_session == NULL)
    {
        OSI_LOGI(0x10007b02, "_ppp_lan_data_pull pcb->ppp_session");
        return;
    }

    pppSession_t *pppSession = (pppSession_t *)pcb->ppp_session;
    osiNotifyTrigger(pppSession->dl_read_notify);
}

static void _ppp_lan_data_pull(void *ctx)
{
    OSI_LOGI(0x10007b03, "_ppp_lan_data_pull");
    struct netif *nif = (struct netif *)ctx;
    if (nif->state == NULL)
        return;

    ppp_pcb *pcb = (ppp_pcb *)nif->state;
    if (pcb->ppp_session == NULL)
        return;

    pppSession_t *pppSession = (pppSession_t *)pcb->ppp_session;
    if (pppSession->flowctrl_cb != NULL)
    {
        int flowcntrol = pppSession->flowctrl_cb(pppSession->flowctrl_cb_ctx);
        if (flowcntrol > 1)
        {
            OSI_LOGI(0x10007b04, "_ppp_lan_data_pull PPP dl read later by hard flow control");
            //osiNotifyTrigger(pppSession->dl_read_notify);
            osiTimerStart(pppSession->dl_read_notify_timer, 10);
            return;
        }
    }

    ppp_buf_t *ppp_buf;
    while ((ppp_buf = TAILQ_FIRST(&(pppSession->buffer_list))) != NULL)
    {
        struct pbuf *p = ppp_buf->buf;
#if LWIP_TCPIP_CORE_LOCKING
        LOCK_TCPIP_CORE();
#endif
        TAILQ_REMOVE(&(pppSession->buffer_list), ppp_buf, iter);
        if (p->tot_len < 160)
        {
            sPPPDLitPackNum--;
            if (sPPPDLitPackNum < 0)
                sPPPDLitPackNum = 0;
        }
        else
        {
            sPPPDBigPackNum--;
            if (sPPPDBigPackNum < 0)
                sPPPDBigPackNum = 0;
        }
#if LWIP_TCPIP_CORE_LOCKING
        UNLOCK_TCPIP_CORE();
#endif
        free(ppp_buf);

#if LWIP_IPV6
        if (IP_HDR_GET_VERSION(p->payload) == 6)
            ppp_netif_output_ip6(nif, p, NULL);
        else
#endif
            ppp_netif_output_ip4(nif, p, NULL);
        sys_arch_dump(p->payload, p->tot_len);
        nif->u32PPPDLSize += p->tot_len;
        pbuf_free(p);
        if (pppSession->flowctrl_cb != NULL)
        {
            int flowcntrol = pppSession->flowctrl_cb(pppSession->flowctrl_cb_ctx);
            if (flowcntrol > 0)
            {
                OSI_LOGI(0x1000562c, "PPP dl read later by flow control");
                //osiNotifyTrigger(pppSession->dl_read_notify);
                osiTimerStart(pppSession->dl_read_notify_timer, 10);
                break;
            }
        }
    }
}

err_t wan_to_ppp_lan_datainput(struct pbuf *p, struct netif *inp)
{
    OSI_LOGI(0x10007b05, "wan_to_ppp_lan_datainput Lit %d Big %d", sPPPDLitPackNum, sPPPDBigPackNum);
    ppp_pcb *pcb = (ppp_pcb *)inp->state;
    if (pcb->ppp_session == NULL)
    {
        pbuf_free(p);
        return -1;
    }

#if LWIP_TCPIP_CORE_LOCKING
    LOCK_TCPIP_CORE();
#endif
    pppSession_t *pppSession = (pppSession_t *)pcb->ppp_session;
    if (p->tot_len < 160 && sPPPDLitPackNum > MAX_PPP_DL_LIT_PACK_NUM)
    {
        OSI_LOGI(0x10007b06, "wan_to_ppp_lan_datainput Lit buffer full, drop it");
        pbuf_free(p);
        osiNotifyTrigger(pppSession->dl_read_notify);
#if LWIP_TCPIP_CORE_LOCKING
        UNLOCK_TCPIP_CORE();
#endif
        return -1;
    }

    if (p->tot_len > 159 && sPPPDBigPackNum > MAX_PPP_DL_BIG_PACK_NUM)
    {
        OSI_LOGI(0x10007b07, "wan_to_ppp_lan_datainput Big buffer full, drop it");
        pbuf_free(p);
        osiNotifyTrigger(pppSession->dl_read_notify);
#if LWIP_TCPIP_CORE_LOCKING
        UNLOCK_TCPIP_CORE();
#endif
        return -1;
    }

    ppp_buf_t *ppp_buf = (ppp_buf_t *)malloc(sizeof(ppp_buf_t));
    ppp_buf->buf = p;
    if (p->tot_len < 160)
    {
        sPPPDLitPackNum++;
    }
    else
    {
        sPPPDBigPackNum++;
    }
    TAILQ_INSERT_TAIL(&(pppSession->buffer_list), ppp_buf, iter);
#if LWIP_TCPIP_CORE_LOCKING
    UNLOCK_TCPIP_CORE();
#endif
    OSI_LOGI(0x10007b08, "wan_to_ppp_lan_datainput osiNotifyTrigger");
    osiNotifyTrigger(pppSession->dl_read_notify);
    return 0;
}

static err_t nat_lan_ppp_data_output(struct netif *netif, struct pbuf *p,
                                     ip_addr_t *ipaddr)
{
    if (p != NULL)
    {
        sys_arch_dump(p->payload, p->tot_len);
#if LWIP_IPV6
        if (IP_HDR_GET_VERSION(p->payload) == 6)
        {
            struct ip6_hdr *ip6hdr;
            ip6hdr = (struct ip6_hdr *)p->payload;
            u8_t nexth = IP6H_NEXTH(ip6hdr);
            if (nexth == IP6_NEXTH_ICMP6)
            {
                struct icmp6_hdr *icmp6hdr;
                uint8_t *ip6Data = p->payload;
                icmp6hdr = (struct icmp6_hdr *)(ip6Data + 40);
                OSI_LOGI(0x10007b09, "nat_lan_ppp_data_output get IPV6 ICMP6");
                if (icmp6hdr->type == ICMP6_TYPE_RS)
                {
                    //save IPV6 local addr to lan netif
                    ip6_addr_t src_ip6 = {0};
                    ip6_addr_copy_from_packed(src_ip6, ip6hdr->src);
                    netif_ip6_addr_set(netif, 0, &src_ip6);
                    OSI_LOGI(0x10007b0a, "nat_lan_ppp_data_output get IPV6 local addr");
                }
            }
            //find Wan netif with same SimCid to send IPV6 packeage out
            struct netif *Wannetif = netif_get_by_cid_type(netif->sim_cid, NETIF_LINK_MODE_NAT_WAN);
            if (Wannetif)
            {
                OSI_LOGI(0x10007b0b, "nat_lan_ppp_data_output IPV6 to Wan netif");
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
#if LWIP_TCPIP_CORE_LOCKING
            UNLOCK_TCPIP_CORE();
#endif
            OSI_LOGI(0x10007b0c, "nat_lan_ppp_data_output %d", taken);
        }
        netif->u32PPPULSize += p->tot_len;
    }

    return ERR_OK;
}

static err_t netif_gprs_nat_lan_ppp_init(struct netif *netif)
{
    /* initialize the snmp variables and counters inside the struct netif
   * ifSpeed: no assumption can be made!
   */
    netif->name[0] = 'P';
    netif->name[1] = 'P';

#if LWIP_IPV4
    netif->output = (netif_output_fn)nat_lan_ppp_data_output;
#endif
#if LWIP_IPV6
    netif->output_ip6 = (netif_output_ip6_fn)nat_lan_ppp_data_output;
#endif
    netif->mtu = GPRS_MTU;

    return ERR_OK;
}
#endif

void ppp_ip_output(struct pbuf *p, void *ctx)
{
    pppSession_t *ppp = (pppSession_t *)ctx;
    //OSI_LOGI(0x1000562a, "PPP ul ip output");

    struct netif *nif = ppp->nif;
    if (p != NULL && nif != NULL)
    {
#if LWIP_IPV6
        if (IP_HDR_GET_VERSION(p->payload) == 6)
        {
            if (isDhcp6infoReqPackage(p) == true)
            {
                //send Dhcp6_info_reply
                pppDhcp6_Info_req_reply(nif, p);
            }
            else
            {
                OSI_LOGI(0x10007b0d, "PPP 6 up load");
                nif->output_ip6(nif, p, NULL);
            }
        }
        else
#endif
            nif->output(nif, p, NULL);
    }
    if (p)
        pbuf_free(p);
}
#ifndef CONFIG_NET_LTE_PSPACKET_SUPPORT
static void _psIntfRead(void *ctx)
{
    struct netif *nif = (struct netif *)ctx;
    if (nif == NULL)
    {
        OSI_LOGE(0x1000562b, "PPP PsIntf get netif failed");
        osiPanic();
    }

    if (nif->state == NULL)
        return;

    ppp_pcb *pcb = (ppp_pcb *)nif->state;
    if (pcb->ppp_session == NULL)
        return;

#ifdef CONFIG_WDT_ENABLE
    drvFeedWdt();
#endif
    halSysWdtStart();
    pppSession_t *pppSession = (pppSession_t *)pcb->ppp_session;
    if (pppSession->flowctrl_cb != NULL)
    {
        int flowcntrol = pppSession->flowctrl_cb(pppSession->flowctrl_cb_ctx);
        if (flowcntrol > 1)
        {
            OSI_LOGI(0x100075c1, "PPP dl read later by hard flow control");
            osiNotifyTrigger(pppSession->dl_read_notify);
            return;
        }
    }
    // Grab a buffer, and will be reused for all DL buffers
    // It is required that PBUF_POOL_BUFSIZE can hold the maximum DL buffer
    struct pbuf *p = pbuf_alloc(PBUF_RAW, 0, PBUF_POOL);
    if (p == NULL)
    {
        OSI_LOGE(0x1000562f, "PPP dl alloc pbuf failed, packet dropped");
        return;
    }

    for (;;)
    {
        int rsize = drvPsIntfRead(nif->pspathIntf, p->payload, PBUF_POOL_BUFSIZE);
        if (rsize <= 0)
            break;

        p->tot_len = p->len = rsize;

        nif->u32PPPDLSize += rsize;
        sys_arch_dump(p->payload, rsize);
#ifdef CONFIG_NET_TRACE_IP_PACKET
        uint8_t *ipdata = p->payload;
        uint16_t identify = (ipdata[4] << 8) + ipdata[5];
        OSI_LOGI(0x10005661, "PPP DL read from IPC thread identify %04x", identify);
#endif

#if LWIP_IPV6
#if defined(CONFIG_SOC_8910) || defined(CONFIG_SOC_8850)
        if (IP_HDR_GET_VERSION(p->payload) == 6)
        {
            OSI_LOGI(0x10007abd, "net device recieve a ipv6 package, rsize = %d", rsize);
            if (isRAPackage(p))
            {
                RA_reply(p);
            }
            ppp_netif_output_ip6(nif, p, NULL);
        }
        else
#endif
#endif
            ppp_netif_output_ip4(nif, p, NULL);

        if (pppSession->flowctrl_cb != NULL)
        {
            int flowcntrol = pppSession->flowctrl_cb(pppSession->flowctrl_cb_ctx);
            if (flowcntrol > 0)
            {
                OSI_LOGI(0x1000562c, "PPP dl read later by flow control");
                osiNotifyTrigger(pppSession->dl_read_notify);
                break;
            }
        }
    }

    pbuf_free(p);
}
static err_t _psIntfWrite(struct netif *netif, struct pbuf *p,
                          const ip4_addr_t *ipaddr)
{
    for (struct pbuf *q = p; q != NULL; q = q->next)
    {
        if (q->len == 0)
            continue;

        // When uplink buffer unavailable, it will be dropped inside
        int wsize = 0x00;

        extern bool NetifGetDPSDFlag(CFW_SIM_ID nSim);
#define GET_SIM(sim_cid) (((sim_cid) >> 4) & 0xf)
#if defined(CONFIG_SOC_8910) || defined(CONFIG_SOC_8850)
        sys_arch_dump(p->payload, p->tot_len);
        if (IP_HDR_GET_VERSION(p->payload) == 6)
        {
            if (isDhcp6infoReqPackage(p) == true)
            {
                //send Dhcp6_info_reply
                pppDhcp6_Info_req_reply(netif, p);
            }
        }
#endif
        if (!NetifGetDPSDFlag(GET_SIM(netif->sim_cid)))
            wsize = drvPsIntfWrite(netif->pspathIntf, q->payload, q->len, 0, 0);
        else
            wsize = q->len;

#ifdef CONFIG_NET_TRACE_IP_PACKET
        uint8_t *ipdata = q->payload;
        uint16_t identify = (ipdata[4] << 8) + ipdata[5];
        OSI_LOGI(0x10005662, "PPP UL write to IPC thread identify %04x", identify);
#endif

        if (wsize != q->len)
            OSI_LOGE(0x10005630, "PPP ul drop, size/%d written/%d", q->len, wsize);
        else
            netif->u32PPPULSize += wsize;
    }
    return ERR_OK;
}

static void _psIntfCB(void *ctx, drvPsIntf_t *p)
{
    struct netif *nif;
    ppp_pcb *pcb;
    pppSession_t *pppSession;
    if (ctx == NULL)
        return;

    nif = (struct netif *)ctx;
    if (nif->state == NULL)
        return;

    pcb = (ppp_pcb *)nif->state;
    if (pcb->ppp_session == NULL)
        return;

    pppSession = (pppSession_t *)pcb->ppp_session;
    osiNotifyTrigger(pppSession->dl_read_notify);
}
#else
err_t gprs_to_ppp_datapull(struct ps_header *psData, struct netif *inp)
{
    OSI_LOGI(0x1000a9a0, "gprs_to_ppp_datapull");
#if LWIP_TCPIP_CORE_LOCKING
    LOCK_TCPIP_CORE();
#endif
    if (NULL == psData || NULL == inp)
    {
        if (NULL != psData)
            free(psData);
        goto LEAVE;
    }

    ppp_pcb *pcb = (ppp_pcb *)inp->state;

    if (NULL == pcb)
    {
        free(psData);
        goto LEAVE;
    }

    pppSession_t *pppSession = (pppSession_t *)pcb->ppp_session;
    if (NULL == pppSession)
    {
        free(psData);
        goto LEAVE;
    }

    if (pppSession->buf_list_count < MAX_PPP_BUF_COUNT)
    {
        ps_buf_t *ps_buf = (ps_buf_t *)malloc(sizeof(ps_buf_t));
        ps_buf->psData = psData;
        TAILQ_INSERT_TAIL(&(pppSession->buf_list), ps_buf, iter);
        pppSession->buf_list_count++;
        OSI_LOGI(0x1000a9a1, "gprs_to_ppp_datapull osiNotifyTrigger bufCount %d", pppSession->buf_list_count);
        osiNotifyTrigger(pppSession->dl_read_notify);
    }
    else
    {
        free(psData);
        OSI_LOGI(0x1000a9a2, "gprs_to_ppp_datapull MAX_PPP_BUF_COUNT drop psData buffer ! %x", psData);
    }
LEAVE:
#if LWIP_TCPIP_CORE_LOCKING
    UNLOCK_TCPIP_CORE();
#endif
    return 0;
}
static void _gprs_to_ppp_datainput(void *ctx)
{
    OSI_LOGI(0x1000a9a3, "_gprs_to_ppp_datainput");
    struct ps_header *psData = NULL;

    struct netif *nif = (struct netif *)ctx;
    if (NULL == nif || NULL == nif->state)
        goto LEAVE;

    ppp_pcb *pcb = (ppp_pcb *)nif->state;
    if (NULL == pcb || NULL == pcb->ppp_session)
        goto LEAVE;

    pppSession_t *pppSession = (pppSession_t *)pcb->ppp_session;
    if (NULL == pppSession)
        goto LEAVE;

    if (NULL != pppSession->flowctrl_cb)
    {
        int flowcntrol = pppSession->flowctrl_cb(pppSession->flowctrl_cb_ctx);
        if (flowcntrol > 1)
        {
            OSI_LOGI(0x1000a9a4, "_gprs_to_ppp_datainput PPP dl read later by hard flow control");
            osiNotifyTrigger(pppSession->dl_read_notify);
            goto LEAVE;
        }
    }
    struct pbuf *p = NULL;
    struct pbuf *q = NULL;
    uint8_t *pData = NULL;
    int len = 0;
    int offset = 0;
    int readLen = 0;
    ps_buf_t *ps_buf = NULL;

    while ((ps_buf = TAILQ_FIRST(&(pppSession->buf_list))) != NULL)
    {
        psData = ps_buf->psData;
#if LWIP_TCPIP_CORE_LOCKING
        LOCK_TCPIP_CORE();
#endif
        TAILQ_REMOVE(&(pppSession->buf_list), ps_buf, iter);
        if (pppSession->buf_list_count > 0)
            pppSession->buf_list_count--;
#if LWIP_TCPIP_CORE_LOCKING
        UNLOCK_TCPIP_CORE();
#endif
        //free iter
        free(ps_buf);

        readLen = psData->len;
        len = readLen;
        pData = ((uint8_t *)psData) + psData->data_off;

        if (len > 0)
        {
            sys_arch_dump(pData, len);
        }

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
        }
        else
            goto LEAVE;

        //free ps Data
        free(psData);
        psData = NULL;

#ifdef CONFIG_NET_TRACE_IP_PACKET
        uint8_t *ipdata = p->payload;
        uint16_t identify = (ipdata[4] << 8) + ipdata[5];
        OSI_LOGI(0x10005661, "PPP DL read from IPC thread identify %04x", identify);
#endif

#if LWIP_IPV6
        if (IP_HDR_GET_VERSION(p->payload) == 6)
        {
            OSI_LOGI(0x10007abd, "net device recieve a ipv6 package, rsize = %d", readLen);
            if (isRAPackage(p))
            {
                RA_reply(p);
            }
            ppp_netif_output_ip6(nif, p, NULL);
        }
        else
#endif
            ppp_netif_output_ip4(nif, p, NULL);

        sys_arch_dump(p->payload, p->tot_len);
        nif->u32PPPDLSize += p->tot_len;
        //free pbuf
        pbuf_free(p);
        if (pppSession->flowctrl_cb != NULL)
        {
            int flowcntrol = pppSession->flowctrl_cb(pppSession->flowctrl_cb_ctx);
            if (flowcntrol > 0)
            {
                OSI_LOGI(0x1000562c, "PPP dl read later by flow control");
                osiNotifyTrigger(pppSession->dl_read_notify);
                break;
            }
        }
    }
LEAVE:
#ifdef CONFIG_WDT_ENABLE
    drvFeedWdt();
#endif
    halSysWdtStart(); // feed wdt manually
    if (NULL != psData)
    {
        //free ps Data
        free(psData);
    }
    return;
}
#endif

static void pppos_linkstate_cb(ppp_pcb *pcb, int err_code, void *ctx)
{
    pppSession_t *ppp = (pppSession_t *)ctx;

    OSI_LOGI(0x10005631, "PPP linkstate err_code = %d", err_code);
    if (err_code == PPPERR_CONNECT || err_code == PPPERR_USER || err_code == PPPERR_PEERDEAD || err_code == PPPERR_AUTHFAIL)
    {
        if (ppp->end_cb != NULL)
        {
            ppp->end_cb(ppp->output_cb_ctx, err_code);
            ppp->end_cb = NULL;
        }
    }
}

static u32_t pppos_output_cb(ppp_pcb *pcb, const void *data, u32_t len, void *ctx)
{
    pppSession_t *ppp = (pppSession_t *)ctx;

    //OSI_LOGI(0x10005632, "PPP dl output len/%d lcp_fsm/%d lpcp_fsm/%d",
    //         len, pcb->lcp_fsm.state, pcb->ipcp_fsm.state);

    if (ppp->output_cb != NULL)
        ppp->output_cb(ppp->output_cb_ctx, (uint8_t *)data, len);
    return len;
}

void ppp_StartGprs(u8_t *usrname, u8_t *usrpwd, void *ctx)
{
    OSI_LOGXI(OSI_LOGPAR_SS, 0x10005633, "PPP set usrname/'%s' usrpwd/'%s'", usrname, usrpwd);
}

static void _setPppAddress(pppSession_t *ppp)
{
    struct netif *nif = ppp->nif;
    if (nif == NULL)
    {
        OSI_LOGE(0x10005634, "PPP set address without netif");
        return;
    }

#if IP_NAT
    if (get_nat_enabled(ppp->sim, ppp->cid))
    {
        struct netif *wan_netif = netif_get_by_cid_type(nif->sim_cid, NETIF_LINK_MODE_NAT_WAN);
        ip4_addr_t *addr = (ip4_addr_t *)netif_ip4_addr(wan_netif);
        ppp_set_ipcp_ouraddr(ppp->pcb, addr);
    }
    else
    {
#endif
        /* Set our address */
#ifdef CONFIG_FIBOCOM_BASE
        ip4_addr_t ip_addr;
        int32_t val[1]={0};
        ffw_nv_get("PPP_SETIP",val,1);
        OSI_PRINTFE("[fibocom] PPP_SETIP get IP inet_addr is %d", val[0]);

        struct sockaddr_in stLocalAddr = {0};
        stLocalAddr.sin_addr.s_addr = htonl(4294967296 - (unsigned)val[0]);
        const char *ip_addr_t1 = inet_ntoa(stLocalAddr.sin_addr);
        OSI_PRINTFE("[fibocom] PPP_SETIP get IP addr is %s", ip_addr_t1);

        ip4addr_aton(ip_addr_t1, &ip_addr);
        ppp_set_ipcp_ouraddr(ppp->pcb, &ip_addr);
#else

        ip4_addr_t addr;
        IP4_ADDR(&addr, 192, 168, 0, 1);
        ppp_set_ipcp_ouraddr(ppp->pcb, &addr);
#endif
#if IP_NAT
    }
#endif

    ppp->pcb->ipcp_wantoptions.accept_local = 1;

    /* Set peer(his) address */
    ppp_set_ipcp_hisaddr(ppp->pcb, netif_ip4_addr(ppp->nif));

    /* Set primary DNS server */
    ip_addr_t dns_server0;
    CFW_GPRS_PDPCONT_INFO_V2 pdp_context;
    CFW_GprsGetPdpCxtV2(ppp->cid, &pdp_context, ppp->sim);
    IP_ADDR4(&dns_server0, pdp_context.pPdpDns[0], pdp_context.pPdpDns[1],
             pdp_context.pPdpDns[2], pdp_context.pPdpDns[3]);
    ppp_set_ipcp_dnsaddr(ppp->pcb, 0, ip_2_ip4(&dns_server0));

    /* Set secondary DNS server */
    IP_ADDR4(&dns_server0, pdp_context.pPdpDns[21], pdp_context.pPdpDns[22],
             pdp_context.pPdpDns[23], pdp_context.pPdpDns[24]);
    ppp_set_ipcp_dnsaddr(ppp->pcb, 1, ip_2_ip4(&dns_server0));

    OSI_LOGI(0x10007b0e, "PPP address local/%08x his/%08x dns0/%08x dns1/%08x",
             ppp->pcb->ipcp_wantoptions.ouraddr,
             ppp->pcb->ipcp_wantoptions.hisaddr,
             ppp->pcb->ipcp_allowoptions.dnsaddr[0],
             ppp->pcb->ipcp_allowoptions.dnsaddr[1]);
}

static void _pppAuthActRsp(pppSession_t *ppp, const osiEvent_t *event);
static void _pppAuthActDone(pppSession_t *ppp, int _iActiavedbyPPP)
{
    uint8_t simId = ppp->sim;
    uint8_t cId = ppp->cid;
    uint8_t iCnt = 0;
    ppp->cgact_activated = _iActiavedbyPPP;
#if IP_NAT
    if (get_nat_enabled(simId, cId))
    {
        struct netif *wan_netif = getGprsWanNetIf(simId, cId);
        struct netif *lan_netif = getGprsNetIf(simId, cId);
        while ((wan_netif == NULL || lan_netif == NULL) && iCnt < 20)
        {
            osiThreadSleep(5);
            wan_netif = getGprsWanNetIf(simId, cId);
            lan_netif = getGprsNetIf(simId, cId);
            iCnt++;
        }

        if (wan_netif == NULL)
        {
            OSI_LOGE(0x10005639, "PPP netif is NULL");
            return;
        }

        //get wan
        struct wan_netif *pwan = getWan(simId, cId);
        if (pwan == NULL)
            return;

        //create lan
        struct lan_netif *plan = newLan(simId, cId, NAT_LAN_IDX_PPP);
        if (plan == NULL)
            return;
        if (lan_create(plan, simId, cId, netif_gprs_nat_lan_ppp_init, wan_to_ppp_lan_datainput) != true)
            return;
        struct netif *netif = plan->netif;
        ppp->dl_read_notify = osiNotifyCreate(ppp->dl_thread, _ppp_lan_data_pull, netif);
        ppp->dl_read_notify_timer = osiTimerCreate(ppp->dl_thread, _ppp_read_notify_timeout, netif);
        TAILQ_INIT(&(ppp->buffer_list));

        if (lan_add2wan(plan, pwan) == true)
        {
            //pwan->lan_netif[NAT_LAN_IDX_PPP] = plan;
            //pwan->lan_num++;
        }
        ppp->nif = netif;
        ppp->nif->state = ppp->pcb;
    }
    else
    {
#endif
        struct netif *netif = getGprsNetIf(simId, cId);
        while (netif == NULL && iCnt < 20)
        {
            osiThreadSleep(5);
            netif = getGprsNetIf(simId, cId);
            iCnt++;
        }
        ppp->nif = netif;
        if (ppp->nif == NULL)
        {
            OSI_LOGE(0x10005639, "PPP netif is NULL");
            return;
        }
#ifndef CONFIG_NET_LTE_PSPACKET_SUPPORT
        ppp->dl_read_notify = osiNotifyCreate(atEngineGetThreadId(), _psIntfRead, netif);
#else
    ppp->dl_read_notify = osiNotifyCreate(atEngineGetThreadId(), _gprs_to_ppp_datainput, netif);
    TAILQ_INIT(&(ppp->buf_list));
    ppp->buf_list_count = 0;
#endif
#ifndef CONFIG_NET_LTE_PSPACKET_SUPPORT
        if (ppp->nif->pspathIntf == NULL)
        {
            OSI_LOGE(0x1000563a, "PPP pspath interface is NULL");
            return;
        }
#endif
        if (ppp->nif->link_mode != NETIF_LINK_MODE_LWIP)
        {
            OSI_LOGE(0x10007b10, "_pppStart wrong mode %d", ppp->nif->link_mode);
            return;
        }
#ifndef CONFIG_NET_LTE_PSPACKET_SUPPORT
        ppp->old_psintf_cb = drvPsIntfSetDataArriveCB(ppp->nif->pspathIntf, _psIntfCB);

        ppp->old_output = ppp->nif->output;
#if LWIP_IPV6
        ppp->old_outputt_ip6 = ppp->nif->output_ip6;
#endif
        ppp->nif->output = _psIntfWrite;
        ppp->nif->output_ip6 = (netif_output_ip6_fn)_psIntfWrite;
#endif
        ppp->nif->state = ppp->pcb;
        ppp->nif->is_ppp_mode = 1;
        ppp->nif->link_mode = NETIF_LINK_MODE_PPP;
#if IP_NAT
    }
#endif
    OSI_LOGI(0x10007b11, "set mtu");
    ppp_pcb *pcb = ppp->pcb;
    lcp_options *wo = &pcb->lcp_wantoptions;
    lcp_options *ho = &pcb->lcp_hisoptions;
    lcp_options *go = &pcb->lcp_gotoptions;
    lcp_options *ao = &pcb->lcp_allowoptions;
    int mtu, mru;

    if (!go->neg_magicnumber)
        go->magicnumber = 0;
    if (!ho->neg_magicnumber)
        ho->magicnumber = 0;

    /*
     * Set our MTU to the smaller of the MTU we wanted and
     * the MRU our peer wanted.  If we negotiated an MRU,
     * set our MRU to the larger of value we wanted and
     * the value we got in the negotiation.
     * Note on the MTU: the link MTU can be the MRU the peer wanted,
     * the interface MTU is set to the lowest of that, the
     * MTU we want to use, and our link MRU.
     */
    mtu = ho->neg_mru ? ho->mru : PPP_MRU;
    mru = go->neg_mru ? LWIP_MAX(wo->mru, go->mru) : PPP_MRU;

    pcb->netif = ppp->nif;
    pcb->netif->mtu = LWIP_MIN(LWIP_MIN(mtu, mru), ao->mru);

#if !LWIP_SINGLE_NETIF
    if (netif_default == ppp->nif && netif_list != NULL)
    {
        struct netif *next_netif = netif_list;
        netif_default = NULL;
        while (next_netif != NULL)
        {
            if ((next_netif->name[0] != 'l') && (next_netif->name[1] != 'o') && next_netif->is_type != NETIF_LINK_TYPE_IMS)
            {
#if IP_NAT
                if (((next_netif->link_mode == NETIF_LINK_MODE_LWIP) || (next_netif->link_mode == NETIF_LINK_MODE_NAT_LWIP_LAN)) && next_netif->sim_cid != (NAT_SIM << 4 | NAT_SIM))
#else
                if (next_netif->link_mode == NETIF_LINK_MODE_LWIP)
#endif
                {
                    netif_set_default(next_netif);
                    netif_reset_dns(next_netif);
                    break;
                }
            }
            next_netif = next_netif->next;
        }
    }
#endif
    _setPppAddress(ppp);
#if PPP_AUTH_SUPPORT
#if PPP_AUTHGPRS_SUPPORT
    if (ppp->pcb->auth_type == 1) //pap
    {
        upap_sAuthRsp(ppp->pcb, CFW_GPRS_ACTIVED);
    }
    else if (ppp->pcb->auth_type == 2) //chap
    {
        chap_SendAuthResponse(ppp->pcb, CFW_GPRS_ACTIVED);
    }
#endif
#endif
    start_network_phase(ppp->pcb);
}

static void _pppAuthActReq(pppSession_t *ppp)
{

    uint8_t act_state = 0;
    uint8_t sim = ppp->sim;
    uint8_t cid = ppp->cid;
    uint16_t uti = 0;
    uint32_t ret;

    CFW_GetGprsActState(cid, &act_state, sim);
    if (act_state != CFW_GPRS_ACTIVED)
    {
        uint8_t oper_id[6];
        uint8_t mode;
        CFW_GPRS_PDPCONT_INFO_V2 sPdpCont;
        CFW_GPRS_PDPCONT_INFO_V2 sPdpContSet;
        memset(&sPdpContSet, 0x00, sizeof(CFW_GPRS_PDPCONT_INFO_V2));
        memset(&sPdpCont, 0x00, sizeof(CFW_GPRS_PDPCONT_INFO_V2));
        ret = CFW_GprsGetPdpCxtV2(cid, &sPdpCont, sim);
        if (ret != 0)
        {
            if (CFW_GprsSetPdpCxtV2(cid, &sPdpContSet, sim) != 0)
            {
                return;
            }
        }
        CFW_GprsGetPdpCxtV2(cid, &sPdpCont, sim);
        if (sPdpCont.nApnSize == 0)
        {
            if (CFW_NwGetCurrentOperator(oper_id, &mode, sim) != 0)
            {
                return;
            }

            CFW_APNS_UNAME_UPWD apn = {0};
            if (AT_GetOperatorDefaultApn(oper_id, &apn))
            {
                sPdpCont.nApnSize = strlen(apn.apn);
                OSI_LOGXI(OSI_LOGPAR_S, 0x10007b12, "sPdpCont.defaultApnInfo %s", apn.apn);
                OSI_LOGI(0x10007b13, "sPdpCont.nApnSize: %d", sPdpCont.nApnSize);
                if (sPdpCont.nApnSize >= THE_APN_MAX_LEN)
                {
                    return;
                }
                memcpy(sPdpCont.pApn, apn.apn, sPdpCont.nApnSize);
            }
        }
        if (sPdpCont.nPdpType == 0)
        {
            sPdpCont.nPdpType = CFW_GPRS_PDP_TYPE_IPV4V6; /* Default IPV4 */
        }
        sPdpCont.nDComp = 0;
        sPdpCont.nHComp = 0;
        sPdpCont.nNSLPI = 0;
        CFW_GPRS_QOS Qos = {3, 4, 3, 4, 16};
        CFW_GprsSetReqQos(cid, &Qos, sim);
#if PPP_AUTH_SUPPORT
#if PPP_AUTHGPRS_SUPPORT
        OSI_LOGI(0x10007b14, "ppp->pcb->auth_type: %d", ppp->pcb->auth_type);
        if (ppp->pcb->auth_type == 1)
        {
            memcpy(sPdpCont.pApnUser, ppp->pcb->peer_username, ppp->pcb->len_username);
            OSI_LOGXI(OSI_LOGPAR_S, 0x10007b15, "ppp->pcb->peer_username: %s", ppp->pcb->peer_username);
            sPdpCont.nApnUserSize = ppp->pcb->len_username;
            memcpy(sPdpCont.pApnPwd, ppp->pcb->peer_response, ppp->pcb->len_response);
            OSI_LOGXI(OSI_LOGPAR_S, 0x10007b16, "ppp->pcb->peer_response: %s", ppp->pcb->peer_response);
            sPdpCont.nApnPwdSize = ppp->pcb->len_response;
            sPdpCont.nAuthProt = 1;
            OSI_LOGI(0x10007b17, "sPdpCont.nApnUserSize: %d, sPdpCont.nApnPwdSize: %d sPdpCont.nAuthProt %d", sPdpCont.nApnUserSize, sPdpCont.nApnPwdSize, sPdpCont.nAuthProt);
        }
        else if (ppp->pcb->auth_type == 2)
        {
            memcpy(sPdpCont.pApnUser, ppp->pcb->peer_username, ppp->pcb->len_username);
            OSI_LOGXI(OSI_LOGPAR_S, 0x10007b15, "ppp->pcb->peer_username: %s", ppp->pcb->peer_username);
            sPdpCont.nApnUserSize = ppp->pcb->len_username;

            memcpy(sPdpCont.pApnPwd, ppp->pcb->peer_response, ppp->pcb->len_response);
            int dumplen = ppp->pcb->len_response;
            OSI_LOGI(0x10007b18, "ppp->pcb->len_response: %d", dumplen);
            OSI_LOGXI(OSI_LOGPAR_M, 0x10007b19, "ppp->pcb->peer_response : %*s", dumplen, ppp->pcb->peer_response);
            dumplen = ppp->pcb->chap_challenge_len;
            memcpy(sPdpCont.pApnPwd + ppp->pcb->len_response, ppp->pcb->chap_challenge, ppp->pcb->chap_challenge_len);
            OSI_LOGXI(OSI_LOGPAR_M, 0x10007b1a, "ppp->pcb->chap_challenge : %*s", dumplen, ppp->pcb->chap_challenge);

            sPdpCont.nApnPwdSize = ppp->pcb->len_response + ppp->pcb->chap_challenge_len;
            sPdpCont.nAuthProt = 3;
            OSI_LOGI(0x10007b17, "sPdpCont.nApnUserSize: %d, sPdpCont.nApnPwdSize: %d sPdpCont.nAuthProt %d", sPdpCont.nApnUserSize, sPdpCont.nApnPwdSize, sPdpCont.nAuthProt);
            dumplen = ppp->pcb->chap_challenge_len + ppp->pcb->len_response;
            OSI_LOGXI(OSI_LOGPAR_M, 0x10007b1b, "sPdpCont.pApnPwd : %*s", dumplen, sPdpCont.pApnPwd);
        }
#endif
#endif
        if (CFW_GprsSetPdpCxtV2(cid, &sPdpCont, sim) != 0)
        {
            return;
        }
        OSI_LOGI(0x10007b1c, "_pppAuthActReq current Task ID %lu\n", (uint32_t)osiThreadCurrent());
        uti = cfwRequestUTI((osiEventCallback_t)_pppAuthActRsp, ppp);
        if ((ret = CFW_GprsAct(CFW_GPRS_ACTIVED, cid, uti, sim)) != 0)
        {
            cfwReleaseUTI(uti);
            return;
        }
        ppp->uti_attact = uti;
        ppp->pcb->acted = 1;
        return;
    }
    ppp->pcb->acted = 1;
    _pppAuthActDone(ppp, 0);
}

static void _pppAuthActRsp(pppSession_t *ppp, const osiEvent_t *event)
{
    const CFW_EVENT *cfw_event = (const CFW_EVENT *)event;
    OSI_LOGI(0x10007b1d, "_pppAuthActRsp EV_CFW_GPRS_ACT_RSP");

    ppp->uti_attact = 0;

    if (((cfw_event->nType == 0xf1) || (cfw_event->nParam2 == 0x6f) || (cfw_event->nParam2 == 0x27)) && (ppp->retrycnt_attact != 0))
    {
        ppp->retrycnt_attact--;
#if PPP_AUTHGPRS_SUPPORT
        if (ppp->retrycnt_attact == 0)
        {
            //restore nAuthProt
            CFW_GPRS_PDPCONT_INFO_V2 sPdpCont;
            memset(&sPdpCont, 0x00, sizeof(CFW_GPRS_PDPCONT_INFO_V2));
            CFW_GprsGetPdpCxtV2(ppp->cid, &sPdpCont, ppp->sim);
            //sPdpCont.nAuthProt = ppp->pcb->nAuthProt_bak; ////modify by fibocom for compile fail for unusufell
            if (CFW_GprsSetPdpCxtV2(ppp->cid, &sPdpCont, ppp->sim) != 0)
            {
                OSI_LOGI(0x1000a109, "CFW_GprsSetPdpCxtV2 nAuthProt fail!");
                return;
            }
            return;
        }
#endif
        _pppAuthActReq(ppp);
        return;
    }

    if (cfw_event->nType != CFW_GPRS_ACTIVED)
    {
#if PPP_AUTH_SUPPORT
#if PPP_AUTHGPRS_SUPPORT
        if (ppp->pcb->auth_type == 1) //pap
        {
            upap_sAuthRsp(ppp->pcb, 0);
        }
        else if (ppp->pcb->auth_type == 2) //chap
        {
            chap_SendAuthResponse(ppp->pcb, 0);
        }
#endif
#endif
        return;
    }

    //send event to netthread and wait for netif created
    OSI_LOGI(0x10005667, "We got CFW_GPRS_ACTIVED");
    osiEvent_t tcpip_event = *event;
    tcpip_event.id = EV_TCPIP_CFW_GPRS_ACT;
    osiEventSend(netGetTaskID(), (const osiEvent_t *)&tcpip_event);

#if defined (CONFIG_FIBOCOM_BASE)
    fibo_lpg_ppp_set(true);
#endif
    _pppAuthActDone(ppp, 1);
}

static void _pppAuthAttRsp(pppSession_t *ppp, const osiEvent_t *event)
{
    // EV_CFW_GPRS_ATT_RSP
    const CFW_EVENT *cfw_event = (const CFW_EVENT *)event;
    OSI_LOGI(0x10007b1e, "_pppAuthAttRsp att_state %p", ppp);
    if (cfw_event->nType != CFW_GPRS_ATTACHED)
    {
        return;
    }
    OSI_LOGI(0x10007b1e, "_pppAuthAttRsp att_state %p", ppp);
    ppp->uti_attact = 0;

    ppp->retrycnt_attact = 3;
    _pppAuthActReq(ppp);
}

void pppAuthDoActive(void *_ppp)
{
    pppSession_t *ppp = (pppSession_t *)_ppp;
    uint8_t cid = ppp->cid;
    OSI_LOGI(0x10007b1f, "current Task ID %lu\n", (uint32_t)osiThreadCurrent());
    OSI_LOGXI(OSI_LOGPAR_IS, 0x10007b20, "enter PPP mode cid/%d", cid);
    uint8_t sim = ppp->sim;
    uint32_t ret;
    uint16_t uti = 0;

    uint8_t att_state = 0;
    CFW_GetGprsAttState(&att_state, sim);
    OSI_LOGI(0x10007b21, "pppAuthDoActive att_state %d", att_state);
    if (att_state != CFW_GPRS_ATTACHED)
    {
        uti = cfwRequestUTI((osiEventCallback_t)_pppAuthAttRsp, ppp);
        if ((ret = CFW_GprsAtt(CFW_GPRS_ATTACHED, uti, sim)) != 0)
        {
            cfwReleaseUTI(uti);
        }
        ppp->uti_attact = uti;
        return;
    }
    ppp->retrycnt_attact = 3;
    _pppAuthActReq(ppp);
}


static void ppp_notify_phase_cb(ppp_pcb *pcb, u8_t phase, void *ctx)
{
    pppSession_t *ppp = (pppSession_t *)ctx;

#if PPP_IPV6_SUPPORT
    OSI_LOGI(0x10007b22, "PPP status phase:%d lcp_fsm:%d, ipcp_fsm:%d, ipv6cp_fsm:%d",
             phase, pcb->lcp_fsm.state, pcb->ipcp_fsm.state, pcb->ipv6cp_fsm.state);
#else
    OSI_LOGI(0x10007b23, "PPP status phase:%d lcp_fsm:%d, ipcp_fsm:%d",
             phase, pcb->lcp_fsm.state, pcb->ipcp_fsm.state);
#endif

    switch (phase)
    {
    case PPP_PHASE_NETWORK:
        if (pcb->lcp_fsm.state == PPP_FSM_STOPPING)
        {
            //Terminating
            //CFW_GprsAct(CFW_GPRS_DEACTIVED, ppp->cid, Act_UTI, ppp->sim);
            //cfwRespSetUtiCB(gAtCfwCtx.resp_man, Act_UTI, cfwActRsp_CB, ppp);
        }
        else if (pcb->lcp_fsm.state == PPP_FSM_OPENED)
        {
            if (pcb->ipcp_fsm.state == PPP_FSM_CLOSED
#if PPP_IPV6_SUPPORT
                || pcb->ipv6cp_fsm.state == PPP_FSM_CLOSED
#endif
            )
            {
                if (ppp->nif != NULL)
                    _setPppAddress(ppp);
            }
        }
        break;
    /* Session is down (either permanently or briefly) */
    case PPP_PHASE_DEAD:
        break;

    /* We are between two sessions */
    case PPP_PHASE_HOLDOFF:
        //led_set(PPP_LED, LED_SLOW_BLINK);
        break;

    /* Session just started */
    case PPP_PHASE_INITIALIZE:
        //led_set(PPP_LED, LED_FAST_BLINK);
        break;

    /* Session is running */
    case PPP_PHASE_RUNNING:
        //led_set(PPP_LED, LED_ON);
        break;
    /* Session is on PPP_PHASE_AUTHENTICATE */
    case PPP_PHASE_CALLBACK:
        sys_arch_printf("ppp_notify_phase_cb PPP_PHASE_CALLBACK\n");
        pppAuthDoActive(ppp);
        break;
    default:
        break;
    }
}

static int _pppStart(pppSession_t *ppp)
{
    OSI_LOGI(0x10005638, "PPP start ...");

    ppp->pcb = pppos_create(ppp->nif, pppos_output_cb, pppos_linkstate_cb, (void *)ppp);
    if (ppp->pcb == NULL)
    {
        ppp->nif = NULL;
        OSI_LOGE(0x1000563b, "PPP pppos_create failed");
        return -1;
    }
    ppp->pcb->ppp_session = ppp;
    ppp->pcb->cid_id = ppp->cid;
    ppp->pcb->sim_id = ppp->sim;

    ppp_set_notify_phase_callback(ppp->pcb, ppp_notify_phase_cb);

#if PAP_FOR_SIM_AUTH && PPP_AUTH_SUPPORT
    /* Auth configuration, this is pretty self-explanatory */
    ppp_set_auth(ppp->pcb, PPPAUTHTYPE_ANY, "login", "password");

    /* Require peer to authenticate */
    ppp_set_auth_required(ppp->pcb, 1);
#endif

    /*
     * Only for PPPoS, the PPP session should be up and waiting for input.
     *
     * Note: for PPPoS, ppp_connect() and ppp_listen() are actually the same thing.
     * The listen call is meant for future support of PPPoE and PPPoL2TP server
     * mode, where we will need to negotiate the incoming PPPoE session or L2TP
     * session before initiating PPP itself. We need this call because there is
     * two passive modes for PPPoS, ppp_set_passive and ppp_set_silent.
     */
    ppp_set_silent(ppp->pcb, 1);

    /*
     * Initiate PPP listener (i.e. wait for an incoming connection), can only
     * be called if PPP session is in the dead state (i.e. disconnected).
     */
    ppp_listen(ppp->pcb);

#ifdef CONFIG_FIBOCOM_BASE
    fibo_set_dcd_status(1, true);
#endif
    OSI_LOGI(0x1000563c, "PPP start done");
    return 0;
}

static int _pppStop(pppSession_t *ppp)
{
    if (ppp == NULL)
        return -1;
#ifdef CONFIG_FIBOCOM_BASE
        fibo_lpg_ppp_set(false);
#endif

    OSI_LOGI(0x1000563d, "PPP stop %p pcb/%p nif/%p", ppp, ppp->pcb, ppp->nif);
    if (ppp->pcb != NULL)
    {
        //ppp_close(ppp->pcb, 0);
        if (ppp->nif != NULL)
        {
            ppp->nif->state = NULL;
        }
        ppp_free(ppp->pcb);
        //osiThreadCallback(netGetTaskID(), _freeppppcb, (void *)ppp->pcb);
        ppp->pcb = NULL;
    }
#if IP_NAT
    if (get_nat_enabled(ppp->sim, ppp->cid) == false)
    {
#endif
        if (ppp->nif != NULL && ppp->nif->is_used == 1)
        {
            if (ppp->nif->link_mode == NETIF_LINK_MODE_PPP)
            {
#ifndef CONFIG_NET_LTE_PSPACKET_SUPPORT
                drvPsIntfSetDataArriveCB(ppp->nif->pspathIntf, lwip_pspathDataInput);
#endif
                ppp->nif->link_mode = NETIF_LINK_MODE_LWIP;
            }
            if (ppp->old_output != NULL)
            {
                ppp->nif->output = ppp->old_output;
            }

#if LWIP_IPV6
            if (ppp->old_outputt_ip6 != NULL)
            {
                ppp->nif->output_ip6 = ppp->old_outputt_ip6;
            }
#endif
            uint8_t addr[THE_PDP_ADDR_MAX_LEN];
            uint8_t length;
            CFW_GprsGetPdpAddr(ppp->cid, &length, addr, ppp->sim);
            uint32_t ip_addr = (addr[3] << 24) | (addr[2] << 16) | (addr[1] << 8) | addr[0];
            OSI_LOGI(0x1000563e, "PPP restore netif ip address 0x%08x", ip_addr);

            ip4_addr_t ip = {0};
            ip.addr = ip_addr;
            netif_set_addr(ppp->nif, &ip, IP4_ADDR_ANY4, IP4_ADDR_ANY4);
            netif_set_link_up(ppp->nif);
            ppp->nif->is_ppp_mode = 0;
#if !LWIP_SINGLE_NETIF
            if (netif_default == NULL && netif_list != NULL)
            {
                struct netif *next_netif = netif_list;
                netif_default = NULL;
#if IP_NAT
                struct netif *lan_netif = NULL;
#endif
                while (next_netif != NULL)
                {
                    if ((next_netif->name[0] != 'l') && (next_netif->name[1] != 'o') && (next_netif->link_mode == NETIF_LINK_MODE_LWIP) && (next_netif->is_type != NETIF_LINK_TYPE_IMS))
                    {
#if IP_NAT
                        if (next_netif->sim_cid == (NAT_SIM << 4 | NAT_SIM))
                        {
                            lan_netif = next_netif;
                            next_netif = next_netif->next;
                            continue;
                        }
#endif
                        netif_set_default(next_netif);
                        netif_reset_dns(next_netif);
                        break;
                    }
                    next_netif = next_netif->next;
                }
#if IP_NAT
                if (netif_default == NULL)
                {
                    netif_set_default(lan_netif);
                    netif_reset_dns(lan_netif);
                }
#endif
            }
        }
#endif
#if IP_NAT
    }
#endif
#ifdef CONFIG_FIBOCOM_BASE
    fibo_set_dcd_status(1, false);
#endif
    ppp->output_cb = NULL;
    return 0;
}

int pppSessionSuspend(pppSession_t *ppp)
{
    return 0;
}

int pppSessionResume(pppSession_t *ppp)
{
    return 0;
}

int pppSessionInput(pppSession_t *ppp, const void *data, size_t size)
{
    int iRet = -1;
#if LWIP_TCPIP_CORE_LOCKING
    LOCK_TCPIP_CORE();
#endif
    if (ppp == NULL || ppp->pcb == NULL)
    {
        goto LEAVE;
    }

#ifdef CONFIG_NET_TRACE_IP_PACKET
    if (size > 30)
    {
        uint8_t *ipdata = (uint8_t *)data + 2;
        uint16_t identify = (ipdata[4] << 8) + ipdata[5];
        OSI_LOGI(0x10005663, "PPP UL AT input identify %04x", identify);
    }
#endif

    pppos_input(ppp->pcb, (uint8_t *)data, (int)size);
    iRet = 0;
LEAVE:
#if LWIP_TCPIP_CORE_LOCKING
    UNLOCK_TCPIP_CORE();
#endif
    return iRet;
}
pppSession_t *pppSessionCreate(uint8_t cid, uint8_t sim, osiThread_t *dl_thread,
                               pppOutputCallback_t output_cb, void *output_cb_ctx,
                               pppEndCallback_t end_cb, void *end_cb_ctx,
                               pppFlowControlCallback_t flowctrl_cb, void *flowctrl_cb_ctx,
                               int _iActiavedbyPPP)
{
    OSI_LOGI(0x10005640, "PPP session create");
#ifdef CONFIG_FIBOCOM_BASE
    fibo_lpg_ppp_set(true);
#endif
    pppSession_t *ppp = NULL;
#if LWIP_TCPIP_CORE_LOCKING
    LOCK_TCPIP_CORE();
#endif
    struct netif *netif = NULL;
#if IP_NAT
    if (get_nat_enabled(sim, cid))
    {
        netif = getPPPNetIf(sim, cid);
        if (netif != NULL)
        {
            OSI_LOGE(0x10005641, "PPP create already in ppp mode");
            goto LEAVE;
        }
    }
    else
    {
#endif
        netif = getGprsNetIf(sim, cid);
        if (netif != NULL && netif->is_ppp_mode)
        {
            OSI_LOGE(0x10005641, "PPP create already in ppp mode");
            goto LEAVE;
        }
#if IP_NAT
    }
#endif
    ppp = (pppSession_t *)calloc(1, sizeof(pppSession_t));
    if (ppp == NULL)
        goto LEAVE;
    memset(ppp, 0, sizeof(pppSession_t));

    ppp->cid = cid;
    ppp->sim = sim;
    ppp->output_cb = output_cb;
    ppp->output_cb_ctx = output_cb_ctx;
    ppp->end_cb = end_cb;
    ppp->end_cb_ctx = end_cb_ctx;
    ppp->is_deleted = 1;
    ppp->cgact_activated = _iActiavedbyPPP;
    ppp->uti_attact = 0xff;
    OSI_LOGE(0x10007b24, "ppp->cgact_activated = %d", _iActiavedbyPPP);
    ppp->dl_thread = dl_thread;
    ppp->flowctrl_cb = flowctrl_cb;
    ppp->flowctrl_cb_ctx = flowctrl_cb_ctx;
    ppp->nif = netif;

    int ret = _pppStart(ppp);
    if (ret != 0)
    {
        free(ppp);
        ppp = NULL;
        goto LEAVE;
    }
    OSI_LOGI(0x10005642, "PPP created %p", ppp);
LEAVE:
#if LWIP_TCPIP_CORE_LOCKING
    UNLOCK_TCPIP_CORE();
#endif
    return ppp;
}

bool pppSessionClose(pppSession_t *ppp, uint8_t termflag)
{
    bool bRet = false;
#ifdef CONFIG_FIBOCOM_BASE
    fibo_lpg_ppp_set(false);
#endif
#if LWIP_TCPIP_CORE_LOCKING
    LOCK_TCPIP_CORE();
#endif
    OSI_LOGI(0x10005643, "PPP close %p, termflag/%d", ppp, termflag);

    if (ppp == NULL)
    {
        goto LEAVE;
    }
    if (termflag != 1 && termflag != 0)
        termflag = 1; //default set to disconnect rigth now
    ppp_close(ppp->pcb, termflag);
#if IP_NAT
    //for active clsoe
    pppSessionLanDestroy(ppp);
#endif
    bRet = true;
LEAVE:
#if LWIP_TCPIP_CORE_LOCKING
    UNLOCK_TCPIP_CORE();
#endif
    return bRet;
}
#if IP_NAT
void pppSessionLanDestroy(pppSession_t *ppp)
{
    if (ppp == NULL)
        return;
    if (get_nat_enabled(ppp->sim, ppp->cid))
    {
        struct netif *netif = ppp->nif;
        struct lan_netif *plan = NULL;

        if (netif != NULL && (plan = ppp->nif->pstlan) != NULL)
        {
            //for ppp close, need sync destroy lan first
            //to aviod DL data use wild nat entry
            if (plan != NULL)
            {
                OSI_LOGI(0x1000a10a, "pppSessionLanDestroy lan = %p", plan);
                lan_remove2wan(plan, getWan(ppp->sim, ppp->cid));
                lan_destroy(plan);
                ppp->nif = NULL;
            }
        }
    }
}
#endif
static void _pppDeactiveRsp(pppSession_t *ppp, const osiEvent_t *event)
{
    // EV_CFW_GPRS_ACT_RSP
    const CFW_EVENT *cfw_event = (const CFW_EVENT *)event;

    OSI_LOGI(0x10005644, "PPP deactive response, nType/%d", cfw_event->nType);
#ifdef CONFIG_FIBOCOM_BASE
    if(ppp_deact_timer != NULL)
    {
        osiTimerStop(ppp_deact_timer);
        osiTimerDelete(ppp_deact_timer);
        ppp_deact_timer = NULL;
        OSI_PRINTFI("Stop ppp_deact_timer");
    }
#endif
    if (cfw_event->nType == CFW_GPRS_DEACTIVED)
    {
        OSI_LOGI(0x100075c3, "PPP delete by cid = %d sim = %d", event->param1, (uint8_t)event->param3);
        pppSessionDeleteByNetifDestoryed(event->param3, event->param1);
        #ifdef CONFIG_FIBOCOM_BASE
         fibocom_exit_ppp();
         fibo_lpg_ppp_set(false);
        #endif

        osiEvent_t tcpip_event = *event;
        tcpip_event.id = EV_TCPIP_CFW_GPRS_DEACT;
        CFW_EVENT *tcpip_cfw_event = (CFW_EVENT *)&tcpip_event;
        tcpip_cfw_event->nUTI = 0;
        osiEventSend(netGetTaskID(), (const osiEvent_t *)&tcpip_event);
    }
}

bool pppSessionDelete(pppSession_t *ppp)
{
    OSI_LOGI(0x10005645, "PPP %p delete ...", ppp);

    if (ppp == NULL)
        return false;
#ifdef CONFIG_FIBOCOM_BASE
        fibo_lpg_ppp_set(false);
#endif

    if (ppp->is_deleted == 0)
    {
        OSI_LOGE(0x10005646, "PPP double delete");
        return true;
    }

    //释放没有回复的uti
    if (ppp->uti_attact != 0xff)
    {
        cfwReleaseUTI(ppp->uti_attact);
    }

    ppp->is_deleted = 0;
    _pppStop(ppp);
#if LWIP_TCPIP_CORE_LOCKING
    LOCK_TCPIP_CORE();
#endif

    //deactivate net if activated by PPP session
    OSI_LOGI(0x10005647, "PPP cgact_activated/%d cid/%d", ppp->cgact_activated, ppp->cid);
    if ((ppp->cgact_activated & 0x01) == 1 && ppp->cid != 0xFF)
    {
        uint8_t act_state = 0;
        int ret = CFW_GetGprsActState(ppp->cid, &act_state, ppp->sim);
        if (act_state != CFW_GPRS_DEACTIVED && ret == 0)
        {
            uint16_t uti = cfwRequestUTI((osiEventCallback_t)_pppDeactiveRsp, ppp);
            uint32_t res = CFW_GprsAct(CFW_GPRS_DEACTIVED, ppp->cid, uti, ppp->sim);
            if (res != 0)
            {
                cfwReleaseUTI(uti);
                OSI_LOGE(0x10005648, "PPP deactive failed res/%d", res);
            }
        #ifdef CONFIG_FIBOCOM_BASE
            if(ppp_deact_timer != NULL)
            {
                osiTimerDelete(ppp_deact_timer);
                ppp_deact_timer = NULL;
            }
            ppp_deact_timer = osiTimerCreate(atEngineGetThreadId(), fibocom_exit_ppp_without_pdp_deact, NULL);
        #endif
        }
    #ifdef CONFIG_FIBOCOM_BASE
        else
        {
            fibocom_exit_ppp();
        }
    #endif
    }

#if IP_NAT
    if (get_nat_enabled(ppp->sim, ppp->cid))
    {
        struct netif *netif = ppp->nif;
        struct lan_netif *plan = NULL;

        if (netif != NULL && (plan = ppp->nif->pstlan) != NULL)
        {
            ppp_buf_t *ppp_buf;
            while ((ppp_buf = TAILQ_FIRST(&(ppp->buffer_list))) != NULL)
            {
                struct pbuf *p = ppp_buf->buf;
                TAILQ_REMOVE(&(ppp->buffer_list), ppp_buf, iter);
                pbuf_free(p);
                free(ppp_buf);
            }
            sPPPDLitPackNum = 0;
            sPPPDBigPackNum = 0;
            osiTimerDelete(ppp->dl_read_notify_timer);
        }
    #ifdef CONFIG_FIBOCOM_BASE
        osiNotifyDelete(ppp->dl_read_notify);
    #endif
    }
    else
    {
#endif
        osiNotifyDelete(ppp->dl_read_notify);
        ppp->dl_read_notify = NULL;
#ifdef CONFIG_NET_LTE_PSPACKET_SUPPORT
        ps_buf_t *ps_buf = NULL;
        while ((ps_buf = TAILQ_FIRST(&(ppp->buf_list))) != NULL)
        {
            struct ps_header *psData = ps_buf->psData;
            TAILQ_REMOVE(&(ppp->buf_list), ps_buf, iter);
            if (NULL != psData)
                free(psData);
            free(ps_buf);
        }
        ppp->buf_list_count = 0;
#endif
        struct netif *netif = ppp->nif;
        if (NULL != netif)
        {
            ppp_pcb *pcb = (ppp_pcb *)netif->state;
            if (NULL != pcb)
            {
                pcb->ppp_session = NULL;
            }
            pcb = NULL;
        }
#if IP_NAT
    }
#endif
    free(ppp);
#if LWIP_TCPIP_CORE_LOCKING
    UNLOCK_TCPIP_CORE();
#endif
    OSI_LOGI(0x10005649, "PPP delete done");
    return true;
}

void pppSessionDeleteByNetifDestoryed(uint8_t nSimId, uint8_t nCid)
{
    OSI_LOGI(0x1000564a, "PPP delete by sim/%d cid/%d", nSimId, nCid);
#if LWIP_TCPIP_CORE_LOCKING
    LOCK_TCPIP_CORE();
#endif
    struct netif *netif;
#if IP_NAT
    if (get_nat_enabled(nSimId, nCid))
    {
        netif = getPPPNetIf(nSimId, nCid);
    }
    else
    {
#endif
        netif = getGprsNetIf(nSimId, nCid);
#if IP_NAT
    }
#endif
    if (netif == NULL)
    {
        OSI_LOGE(0x1000564b, "PPP delete netif is NULL");
        goto LEAVE;
    }
    if (netif->is_ppp_mode == 0)
    {
        OSI_LOGE(0x1000564c, "PPP delete netif is not ppp mode");
        goto LEAVE;
    }
    if (netif->state == NULL)
    {
        OSI_LOGE(0x1000564d, "PPP delete netif state is NULL");
        goto LEAVE;
    }

    pppSession_t *pppSession = ((ppp_pcb *)netif->state)->ppp_session;
    if (pppSession == NULL)
    {
        OSI_LOGE(0x1000564e, "PPP delete session is NULL");
        goto LEAVE;
    }

    pppSessionClose(pppSession, 0);
LEAVE:
#if LWIP_TCPIP_CORE_LOCKING
    UNLOCK_TCPIP_CORE();
#endif
    return;
}

void pppSessionDeleteBySimCid(uint8_t nSimId, uint8_t nCid, uint8_t forceClose)
{
    OSI_LOGI(0x1000564a, "PPP delete by sim/%d cid/%d", nSimId, nCid);

    struct netif *netif;
    netif = getGprsNetIf(nSimId, nCid);
    if (netif == NULL)
    {
        OSI_LOGE(0x1000564b, "PPP delete netif is NULL");
        return;
    }
    if (netif->is_ppp_mode == 0)
    {
        OSI_LOGE(0x1000564c, "PPP delete netif is not ppp mode");
        return;
    }
    if (netif->state == NULL)
    {
        OSI_LOGE(0x1000564d, "PPP delete netif state is NULL");
        return;
    }

    pppSession_t *pppSession = ((ppp_pcb *)netif->state)->ppp_session;
    if (pppSession == NULL)
    {
        OSI_LOGE(0x1000564e, "PPP delete session is NULL");
        return;
    }

    pppSessionClose(pppSession, forceClose);
}

#ifdef CONFIG_FIBOCOM_BASE
void fibocom_exit_ppp()
{
    OSI_LOGI(0, "fibocom_exit_ppp enter");
    atCmdEngine_t *engine = NULL;

    if (NULL != ppp_engine)
    {
        engine = ppp_engine;
    }
    else
    {
        OSI_LOGI(0, "fibocom_exit_ppp error exit");
        return;
    }
    if (NULL != engine)
    {
        atCmdRespUrcCode(engine, CMD_RC_NOCARRIER);
    }
    ppp_engine = NULL;
    OSI_LOGI(0, "fibocom_exit_ppp exit");

}
void fibocom_exit_ppp_without_pdp_deact()
{
    OSI_LOGI(0, "fibocom_exit_ppp_without_pdp_deact enter");
    atCmdEngine_t *engine = NULL;

    if (NULL != ppp_engine)
    {
        engine = ppp_engine;
    }
    else
    {
        return;
    }
    if(ppp_deact_timer != NULL)
    {
        osiTimerStop(ppp_deact_timer);
        osiTimerDelete(ppp_deact_timer);
        ppp_deact_timer = NULL;
    }
    if (NULL != engine)
    {
        atCmdRespUrcCode(engine, CMD_RC_NOCARRIER);
    }
    ppp_engine = NULL;
    OSI_LOGI(0, "fibocom_exit_ppp_without_pdp_deact exit");
}

#endif

#if PPP_AUTHGPRS_SUPPORT
uint8_t getPppSessionActAttUTI(pppSession_t *ppp)
{
    uint8_t iUti = 0xff;
    if (ppp != NULL)
    {
        iUti = ppp->uti_attact;
    }
    OSI_LOGE(0x10007b25, "getPppSessionActAttUTI iUti = %d", iUti);
    return iUti;
}
#endif
#else
pppSession_t *pppSessionCreate(uint8_t cid, uint8_t sim, osiThread_t *dl_thread,
                               pppOutputCallback_t output_cb, void *output_cb_ctx,
                               pppEndCallback_t end_cb, void *end_cb_ctx,
                               pppFlowControlCallback_t flowctrl_cb, void *flowctrl_cb_ctx,
                               int _iActiavedbyPPP)
{
    return NULL;
}
bool pppSessionDelete(pppSession_t *ppp)
{
    return NULL;
}
bool pppSessionClose(pppSession_t *ppp, uint8_t termflag)
{
    return NULL;
}
int pppSessionSuspend(pppSession_t *ppp)
{
    return 0;
}
int pppSessionResume(pppSession_t *ppp)
{
    return 0;
}
int pppSessionInput(pppSession_t *ppp, const void *data, size_t size)
{
    return -1;
}
void pppSessionDeleteByNetifDestoryed(uint8_t nSimId, uint8_t nCid)
{
    return;
}
void pppSessionDeleteBySimCid(uint8_t nSimId, uint8_t nCid, uint8_t forceClose)
{
    return;
}
#endif
