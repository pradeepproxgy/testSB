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

#define OSI_LOCAL_LOG_TAG OSI_MAKE_LOG_TAG('N', 'D', 'E', 'V')
#define OSI_LOCAL_LOG_LEVEL OSI_LOG_LEVEL_INFO

#include "cfw.h"
#include "osi_api.h"
#include "osi_log.h"
#include "lwip/netif.h"
#include "lwip/dns.h"
#include "lwip/tcpip.h"
#include "lwip/ip.h"

#include "drv_ps_path.h"
#include "netdev_interface_imp.h"
#include "netdev_interface_nat_lan.h"
#include "drv_ether.h"
#include "hal_chip.h"
#include "lwip/prot/ip.h"
#include "lwip/prot/icmp6.h"
#include "osi_api.h"
#include "hal_chip.h"
#if IP_NAT
#include "lwip/ip4_nat.h"
#include "netutils.h"
#if defined CONFIG_FIBOCOM_DEV_MODEM_SOCKET
#include "lwip/prot/tcp.h"
#include "lwip/udp.h"
#include "lwip/inet.h"
#include "lwip/mem.h"
#include "lwip/sys.h"
#include "ffw_nv.h"
#include "sockets.h"
#endif
#ifdef CONFIG_FIBOCOM_BASE
#include "lwip/opt.h"
#include "lwip/icmp.h"
#include "lwip/inet_chksum.h"
#include "ffw_channel.h"
#ifdef CONFIG_FIBOCOM_XLAT_SUPPORT
#include "ffw_plat_xlat.h"
#include "ffw_result.h"
#endif
#endif
#if defined CONFIG_FIBOCUS_HAIKANG_FEATURE
#include "lwip/ip4_addr.h"
#endif

extern osiThread_t *netGetTaskID(void);
extern bool isDhcpPackage(drvEthPacket_t *pkt, uint32_t size);
extern void dhcps_reply(drvEther_t *ether, drvEthPacket_t *pkt, uint32_t size, struct netif *netif);
extern bool isARPPackage(drvEthPacket_t *pkt, uint32_t size);
extern void ARP_reply(drvEther_t *ether, drvEthPacket_t *pkt, uint32_t size, struct netif *netif);
extern bool isRouterSolicitationPackage(drvEthPacket_t *pkt, uint32_t size);
extern void routerAdvertisement_reply(drvEther_t *ether, drvEthPacket_t *pkt, uint32_t size, struct netif *netif);
extern bool isRouterAdvertisementPackage(drvEthPacket_t *pkt, uint32_t size);
extern bool isNeighborSolicitationPackage(drvEthPacket_t *pkt, uint32_t size);
extern void neighborAdvertisement_reply(drvEther_t *ether, drvEthPacket_t *pkt, uint32_t size, struct netif *netif);
extern u8_t ip4_wan_forward(struct pbuf *p, struct netif *inp);

#ifdef CONFIG_NET_DNS_SERVER_SUPPORT
extern bool handleOutputDNSPackage(drvEther_t *ether, drvEthPacket_t *pkt, uint32_t size);
//extern bool handleInputDNSPackage(drvEthPacket_t *pkt, uint32_t size);
#endif

extern bool isDhcp6infoReqPackage(struct pbuf *pb);
extern struct pbuf *Dhcp6_Info_req_reply(struct netif *netif, struct pbuf *pb);
extern bool isRAPackage(struct pbuf *pb);
extern void RA_reply(struct pbuf *pb);

static bool prvNdevLanDataToPs(netSession_t *session, const void *data, size_t size);
static netSession_t *prvNdevLanSessionCreate(uint8_t sim, uint8_t cid);
netSession_t *prvNdevLanOnly_SessionCreate(uint8_t sim, uint8_t cid);
extern err_t nat_lan_lwip_tcpip_input(struct pbuf *p, struct netif *inp);
extern err_t netif_gprs_nat_lan_lwip_init(struct netif *netif);
static bool prvNdevLanSessionDelete(netSession_t *session);

extern netdevIntf_t gNetIntf;
extern u8_t netif_num;
#ifdef CONFIG_WDT_ENABLE
extern void drvFeedWdt(void);
#endif
#ifdef CONFIG_FIBOCOM_BASE
extern int8_t ffw_get_default_netif_cid();
extern int8_t rndis_cid;
static void* ip4_nat_check_header(struct pbuf *p, u16_t min_size);
static bool IsdevPingGW(netSession_t *session, const void *data, size_t size);
#endif

#ifdef CONFIG_FIBOCOM_AT_RNDIS_SUPPORT
extern uint32_t g_lan_data_received;
#endif
#if defined CONFIG_FIBOCOM_DEV_MODEM_SOCKET
#define DNS_PORT 53
uint8_t lan_dev_create(void *lannetif, uint8_t sim, uint8_t cid, netif_init_fn init, netif_input_fn input);
struct netif *TCPIP_lan_dev_netif_create(uint8_t nCid, uint8_t nSimId, uint8_t nIdx, netif_init_fn init, netif_input_fn input);
err_t netif_lan_dev_lwip_init(struct netif *netif);
static bool Isdevlanlwip(netSession_t *session, const void *data, size_t size);
#endif
#if defined CONFIG_FIBOCUS_HAIKANG_FEATURE
extern uint8_t fibo_get_current_lan_ipaddr_haikang();
#endif

static void prvEthLanUploadDataCB(drvEthPacket_t *pkt, uint32_t usize, void *ctx)
{
    netdevIntf_t *nc = (netdevIntf_t *)ctx;
    netSession_t *session = nc->session;
    int32_t size = (int32_t)usize;

    if (session == NULL)
    {
        OSI_LOGE(0x1000756c, "upload data no session");
        return;
    }
#ifdef CONFIG_WDT_ENABLE
    drvFeedWdt();
#endif
    halSysWdtStart();
    if (size > ETH_HLEN)
    {
#ifdef CONFIG_FIBOCOM_BASE
        OSI_LOGI(0, "prvEthLanUploadDataCB data size: %d", size - ETH_HLEN);
#else
        OSI_LOGE(0x1000756d, "prvEthUploadDataCB data size: %d", size - ETH_HLEN);
#endif
        uint8_t *ipData = pkt->data;
        sys_arch_dump(ipData, size - ETH_HLEN);
#ifdef CONFIG_NET_TRACE_IP_PACKET
        uint8_t *ipdata = ipData;
        uint16_t identify = (ipdata[4] << 8) + ipdata[5];
    #ifdef CONFIG_FIBOCOM_BASE
        OSI_LOGI(0, "prvEthLanUploadDataCB data size: %d identify %04x", size - ETH_HLEN, identify);
    #else
        OSI_LOGI(0x10007abc, "prvEthUploadDataCB data size: %d identify %04x", size - ETH_HLEN, identify);
    #endif
#endif
    }
    else
    {
        return;
    }

    #ifdef CONFIG_FIBOCOM_AT_RNDIS_SUPPORT
    if(!g_lan_data_received)
    {
       g_lan_data_received = 1;
    }
    #endif

    if (session->dev_netif != NULL && isARPPackage(pkt, size)) //drop ARP package
    {
        ARP_reply(nc->ether, pkt, size, session->dev_netif);
        return;
    }

    if (session->dev_netif != NULL && isNeighborSolicitationPackage(pkt, size))
    {
        neighborAdvertisement_reply(nc->ether, pkt, size, session->dev_netif);
        return;
    }

    bool isDHCPData = isDhcpPackage(pkt, size);
    if (session->dev_netif != NULL && isDHCPData)
    {
#ifdef CONFIG_FIBOCOM_BASE
        //int8_t actived_cid = ffw_get_default_netif_cid();//MTC0738-321
        OSI_LOGI(0, "isDhcpPackage rndis_cid is %d", rndis_cid);
        int nSim = ffw_get_data_sim();
        if(getGprsWanNetIf(nSim, rndis_cid)!=NULL)
        {
            OSI_LOGI(0, "getGprsWanNetIf is not NULL");
            dhcps_reply(nc->ether, pkt, size, session->dev_netif);
        }
        else
        {
            OSI_LOGI(0, "test getGprsWanNetIf is NULL");
        }
#else
        dhcps_reply(nc->ether, pkt, size, session->dev_netif);
#endif
    }
#ifdef CONFIG_NET_DNS_SERVER_SUPPORT
    else if (handleOutputDNSPackage(nc->ether, pkt, size))
    {
        OSI_LOGI(0x10009834, "handleDNSPackage Package");
    }
#endif
#ifdef CONFIG_FIBOCOM_BASE
    else if(IsdevPingGW(session, pkt->data, size - ETH_HLEN))
    {
        OSI_PRINTFE("IsdevPingGW IP Package");
    }
#endif
#ifdef CONFIG_FIBOCOM_DEV_MODEM_SOCKET
    else if(Isdevlanlwip(session, pkt->data, size - ETH_HLEN))
    {
        OSI_PRINTFE("Isdevlanlwip IP Package");
    }
#endif
    else
    {
        prvNdevLanDataToPs(session, pkt->data, size - ETH_HLEN);
    }
}

static err_t
wan_to_netdev_lan_datainput(struct pbuf *p, struct netif *nif)
{
    uint16_t offset = 0;
    struct pbuf *q = NULL;
    netdevIntf_t *nc = &gNetIntf;
    drvEthReq_t *tx_req = NULL;
    uint32_t size = 0;

    if (IP_HDR_GET_VERSION(p->payload) == 6)
    {
        OSI_LOGI(0x10007abd, "net device recieve a ipv6 package, rsize = %d", p->tot_len);
        if (isRAPackage(p))
        {
            RA_reply(p);
        }
    }

    if (nc->mutex != NULL)
    {
        osiMutexLock(nc->mutex);
    }
#if defined CONFIG_FIBOCUS_HAIKANG_FEATURE
    uint8_t reCnt = 0;//SPCSS01101537
#endif
    while ((nc->netdevIsSuspend == false) && ((tx_req = drvEtherTxReqAlloc(nc->ether)) == NULL))//SPCSS01101537
    {
        osiThreadSleep(1);
#if defined CONFIG_FIBOCUS_HAIKANG_FEATURE
        reCnt++; //SPCSS01101537
        if (nc->ether == NULL || reCnt > 10) //SPCSS01101537
        {
            OSI_LOGI(0, "wan_to_netdev_lan_datainput, Tx Req Alloc fail reCnt = %d", reCnt);
            if (nc->mutex != NULL)
            {
                osiMutexUnlock(nc->mutex);
            }
            pbuf_free(p);//SPCSS01101537
            return -1;
        }

#else
        if (nc->ether == NULL)
        {
            if (nc->mutex != NULL)
            {
                osiMutexUnlock(nc->mutex);
            }
#ifdef CONFIG_FIBOCOM_BASE
            OSI_LOGI(0, "wan_to_netdev_lan_datainput, nc->ether == NULL");
            pbuf_free(p);//MTC0733-135
#endif
            return -1;
        }
#endif
    }

    if (nc->netdevIsSuspend == true)
    {
        if (nc->mutex != NULL)
        {
            osiMutexUnlock(nc->mutex);
        }
        if (tx_req != NULL)
            drvEtherTxReqFree(nc->ether, tx_req);
#ifdef CONFIG_FIBOCOM_BASE
        OSI_LOGI(0, "wan_to_netdev_lan_datainput, nc->netdevIsSuspend == true");
        pbuf_free(p);//MTC0733-135,When entering sleep mode, the memory is not released
#endif
        return -1;
    }
    if (nc->mutex != NULL)
    {
        osiMutexUnlock(nc->mutex);
    }

    uint8_t *pData = tx_req->payload->data;
    size = p->tot_len;

    for (q = p; q != NULL; q = q->next)
    {
        memcpy(&pData[offset], q->payload, q->len);
        offset += q->len;
    }

    //handleInputDNSPackage(tx_req->payload, p->tot_len);
    sys_arch_dump(pData, p->tot_len);

    nif->u32RndisDLSize += p->tot_len;
#ifdef CONFIG_NET_TRACE_IP_PACKET
    uint8_t *ipdata = pData;
    uint16_t identify = (ipdata[4] << 8) + ipdata[5];
    OSI_LOGI(0x10007abe, "Wan to NC identify %04x", identify);
#endif
    OSI_LOGE(0x10007abf, "Wan to NC %d", p->tot_len);
    pbuf_free(p);
    if (nc->mutex != NULL)
    {
        osiMutexLock(nc->mutex);
    }
    if (nc->netdevIsSuspend == false)
    {
        if (!drvEtherTxReqSubmit(nc->ether, tx_req, size))
        {
            OSI_LOGW(0x10007ac0, "Wan to NC error %d", size);
            if (nc->mutex != NULL)
            {
                osiMutexUnlock(nc->mutex);
            }
            return -1;
        }
    }
    else
    {
        drvEtherTxReqFree(nc->ether, tx_req);
    }
    if (nc->mutex != NULL)
    {
        osiMutexUnlock(nc->mutex);
    }
    return 0;
}

static bool prvNdevLanDataToPs(netSession_t *session, const void *data, size_t size)
{
    struct netif *inp_netif = session->dev_netif;
    if (inp_netif == NULL)
        return false;

    if (size <= 0)
        return false;

    struct pbuf *p, *q, *dhcpv6_reply = NULL;
    int offset = 0;
    uint8_t *pData = (uint8_t *)data;
    p = (struct pbuf *)pbuf_alloc(PBUF_RAW, size, PBUF_POOL);
    if (p != NULL)
    {
        if (size > p->len)
        {
            for (q = p; size > q->len; q = q->next)
            {
                memcpy(q->payload, &(pData[offset]), q->len);
                size -= q->len;
                offset += q->len;
            }
            if (size != 0)
            {
                memcpy(q->payload, &(pData[offset]), size);
            }
        }
        else
        {
            memcpy(p->payload, &(pData[offset]), size);
        }
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
                OSI_LOGI(0x10007ac1, "prvNdevLanDataToPs get IPV6 ICMP6");
                if (icmp6hdr->type == ICMP6_TYPE_RS)
                {
                    //save IPV6 local addr to lan netif
                    ip6_addr_t src_ip6 = {0};
                    ip6_addr_copy_from_packed(src_ip6, ip6hdr->src);
                    netif_ip6_addr_set(inp_netif, 0, &src_ip6);
                    netif_ip6_addr_set_state(inp_netif, 0, IP6_ADDR_VALID);
                    OSI_LOGI(0x10007ac2, "prvNdevLanDataToPs get IPV6 local addr");
                }
            }
            if (IP_HDR_GET_VERSION(p->payload) == 6)
            {
                if (isDhcp6infoReqPackage(p) == true)
                {
                    //send Dhcp6_info_reply
                    dhcpv6_reply = Dhcp6_Info_req_reply(inp_netif, p);
                    if (dhcpv6_reply != NULL)
                    {
                        OSI_LOGI(0x10007ac3, "inp_netif->output_ip6 ===== ");
                        inp_netif->input(dhcpv6_reply, inp_netif);
                    }
                }
            }
            //find Wan netif with same SimCid to send IPV6 packeage out
            struct netif *Wannetif = netif_get_by_cid_type(inp_netif->sim_cid, NETIF_LINK_MODE_NAT_WAN);
            if (Wannetif)
            {
                OSI_LOGI(0x10007ac4, "prvNdevLanDataToPs IPV6 to Wan netif");
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

            u8_t taken = ip4_wan_forward(p, inp_netif);
            if (taken == 0)
                taken = ip4_nat_out(p);
#ifdef CONFIG_FIBOCOM_XLAT_SUPPORT // XLAT
            if (taken == 0)
            {
                //2024-06-24, sundaqing, Begin. Lwip support XLAT .
                taken = ffw_xlat_clat_output(p, p->tot_len, ((inp_netif->sim_cid) >> 4) & 0xF, (inp_netif->sim_cid) & 0xF) == FFW_R_SUCCESS;
                //2024-06-24, sundaqing, End. Lwip support XLAT .
            }
#endif
#if LWIP_TCPIP_CORE_LOCKING
            UNLOCK_TCPIP_CORE();
#endif
            OSI_LOGI(0x10007ac5, "prvNdevLanDataToPs %d", taken);
        }
        inp_netif->u32RndisULSize += p->tot_len;
        pbuf_free(p);
    }
    return true;
}

static void prvProcessNdevLanConnect(void *par)
{
    netdevIntf_t *nc = (netdevIntf_t *)par;
    OSI_LOGI(0x10007ac6, "prvProcessNdevLanConnect (ether %p, session %p)", nc->ether, nc->session);
    if (nc->ether == NULL)
        return;

    nc->netdevIsSuspend = false;
    if (nc->session == NULL && netif_default != NULL)
    {
        uint8_t nCid = netif_default->sim_cid & 0x0f;
        uint8_t nSimId = netif_default->sim_cid >> 4;
        nc->session = prvNdevLanSessionCreate(nSimId, nCid);
    }
    else if (nc->session == NULL && netif_default == NULL)
    {
        nc->session = prvNdevLanOnly_SessionCreate(NAT_SIM, NAT_CID);
    }

    if (nc->session == NULL)
        return;

    drvEtherSetULDataCB(nc->ether, prvEthLanUploadDataCB, nc);
    if (!drvEtherNetUp(nc->ether))
    {
        OSI_LOGI(0x10007575, "NC connect ether open fail");
        prvNdevLanSessionDelete(nc->session);
        nc->session = NULL;
        return;
    }
#ifdef CONFIG_TB_API_SUPPORT
    osiEvent_t tcpip_event = {0}; //*(osiEvent_t *)cfw_event;
    tcpip_event.id = EV_TCPIP_ETHCARD_CONNECT;
    osiEventSend(atEngineGetThreadId(), (const osiEvent_t *)&tcpip_event);
#endif
}

void netdevLanConnect(void)
{
    OSI_LOGI(0x10007ac7, "netdevLanConnect timer start.");
    netdevIntf_t *nc = &gNetIntf;
    if (nc->connect_timer != NULL)
        osiTimerDelete(nc->connect_timer);
    nc->connect_timer = osiTimerCreate(netGetTaskID(), prvProcessNdevLanConnect, nc);
#ifdef CONFIG_FIBOCOM_BASE
    //MTC0647-77 Reduce network card startup time, 500ms is suitable for early 8910 products, not required for 8850.
    //If 8910 needs to be modified, it needs to be confirmed with the platform in advance
    osiTimerStart(nc->connect_timer, 5);
#else
    osiTimerStart(nc->connect_timer, 500);
#endif
}
void netdevLanDisconnect(void)
{
    OSI_LOGI(0x10007ac8, "netdevDisconnect 4444");
    netdevIntf_t *nc = &gNetIntf;
    drvEtherNetDown(nc->ether);
    prvNdevLanSessionDelete(nc->session);
    nc->session = NULL;
    if (nc->connect_timer != NULL)
    {
        osiTimerDelete(nc->connect_timer);
        nc->connect_timer = NULL;
    }
#ifdef CONFIG_TB_API_SUPPORT
    osiEvent_t tcpip_event = {0}; //*(osiEvent_t *)cfw_event;
    tcpip_event.id = EV_TCPIP_ETHCARD_DISCONNECT;
    osiEventSend(atEngineGetThreadId(), (const osiEvent_t *)&tcpip_event);
#endif
}
void _netdevLanNetUp(uint8_t nSimID, uint8_t nCID)
{
    OSI_LOGI(0x10007577, "netdevNetUp");
    netdevIntf_t *nc = &gNetIntf;
    if (nc->ether == NULL)
        return;

    if (nc->session != NULL)
    {
        OSI_LOGI(0x10007ac9, "netdevLanNetUp Net session create already created");
        return;
    }

    uint8_t T_cid = nSimID << 4 | nCID;
    struct netif *wan_netif = netif_get_by_cid_type(T_cid, NETIF_LINK_MODE_NAT_WAN);
    if (wan_netif == NULL)
        nc->session = prvNdevLanOnly_SessionCreate(nSimID, nCID);
    else
        nc->session = prvNdevLanSessionCreate(nSimID, nCID);

    if (nc->session == NULL)
        return;

    drvEtherSetULDataCB(nc->ether, prvEthLanUploadDataCB, nc);
    if (!drvEtherNetUp(nc->ether))
    {
        OSI_LOGI(0x10007578, "NC net up ether open fail.");
        prvNdevLanSessionDelete(nc->session);
        nc->session = NULL;
        return;
    }
}
void netdevLanNetUp(uint8_t nSimID, uint8_t nCID)
{
    if (netif_default == NULL)
    {
        _netdevLanNetUp(NAT_SIM, NAT_CID);
        return;
    }
    if (netif_default != NULL && get_nat_enabled((netif_default->sim_cid >> 4), (netif_default->sim_cid & 0x0f)))
    {
        uint8_t nCid = netif_default->sim_cid & 0x0f;
        uint8_t nSimId = netif_default->sim_cid >> 4;
        _netdevLanNetUp(nSimId, nCid);
    }
}
void _netdevLanNetDown(uint8_t nSimID, uint8_t nCID)
{
    netdevIntf_t *nc = &gNetIntf;
    OSI_LOGI(0x10007579, "pre netdevNetDown");
    if ((nc->netdevIsConnected == true) && (nc->session != NULL) && (nc->session->is_created == true))
    {
        OSI_LOGI(0x10007aca, "netdevNetDown ssssssss 1111");
        struct netif *inp_netif = nc->session->dev_netif;
        if ((inp_netif != NULL) && ((nSimID << 4 | nCID) == inp_netif->sim_cid))
        {
            OSI_LOGI(0x1000757a, "netdevNetDown");
            drvEtherNetDown(nc->ether);
            prvNdevLanSessionDelete(nc->session);
            nc->session = NULL;
        }
    }
}
static void prvNdevLanRestart(void *ctx)
{
    if (netif_default != NULL)
    {
        netdevLanNetUp(NAT_SIM, NAT_CID);
    }
}
void netdevLanNetDown(uint8_t nSimID, uint8_t nCID)
{
    if (get_nat_enabled(nSimID, nCID))
    {
        _netdevLanNetDown(nSimID, nCID);
    }
    if (netif_default != NULL) //muilty cid actived; restart Netdev on other cid
    {
        osiThreadCallback(netGetTaskID(), prvNdevLanRestart, NULL);
    }
}
static err_t nat_lan_netdev_data_output(struct netif *netif, struct pbuf *p,
                                        ip_addr_t *ipaddr)
{
    return 0;
}

static err_t netif_gprs_nat_lan_ndev_init(struct netif *netif)
{
    /* initialize the snmp variables and counters inside the struct netif
   * ifSpeed: no assumption can be made!
   */
    netif->name[0] = 'N';
    netif->name[1] = 'C';
    netif->mtu = GPRS_MTU;
#if LWIP_IPV4
    netif->output = (netif_output_fn)nat_lan_netdev_data_output;
#endif
    return ERR_OK;
}

netSession_t *prvNdevLanSessionCreate(uint8_t sim, uint8_t cid)
{
    static netSession_t s_net_nat_session;
    OSI_LOGI(0x1000757d, "Net session create");

    uint32_t critical = osiEnterCritical();
    netSession_t *session = &s_net_nat_session;
    if (session != NULL && session->is_created == true)
    {
        OSI_LOGI(0x1000757f, "Net session create already created");
        goto end;
    }

    struct netif *netif = NULL;
    struct wan_netif *pwan = getWan(sim, cid);
    struct lan_netif *plan = newLan(sim, cid, NAT_LAN_IDX_RNDIS);
    if (plan == NULL)
        goto end;

    osiExitCritical(critical);
    if (lan_create(plan, sim, cid, netif_gprs_nat_lan_ndev_init, wan_to_netdev_lan_datainput) != true)
        return NULL;
#ifdef CONFIG_FIBOCOM_DEV_MODEM_SOCKET
    /*create 192.168.x.1 of LAN*/
    struct lan_netif *plan_dev = newLan(NAT_SIM, NAT_CID, NAT_LAN_IDX_LWIP);
    if (plan_dev == NULL)
    {
        OSI_PRINTFE("plan_dev new fail!");
        goto end;
    }
    if (lan_dev_create(plan_dev, NAT_SIM, NAT_CID, netif_lan_dev_lwip_init, nat_lan_lwip_tcpip_input) != true)
    {
        OSI_PRINTFE("lan_dev_create fail!");
        goto end;
    }
#endif
    critical = osiEnterCritical();
    lan_add2wan(plan, pwan);
    netif = plan->netif;

    memset(session, 0, sizeof(netSession_t));
    session->dev_netif = netif;
    OSI_LOGI(0x10007580, "Net end session %x\n", session);
    session->is_created = true;

end:
    osiExitCritical(critical);
    return session;
}

bool prvNdevLanSessionDelete(netSession_t *session)
{
    OSI_LOGI(0x10007acc, "netdevDisconnect 5555");
    uint32_t critical = osiEnterCritical();
    if (session == NULL)
    {
        osiExitCritical(critical);
        return false;
    }
    OSI_LOGI(0x10007acd, "netdevDisconnect 6666 %p session->dev_netif->sim_cid = %d", session->dev_netif, session->dev_netif->sim_cid);

    if (session->dev_netif != NULL)
    {
        struct lan_netif *plan = (struct lan_netif *)session->dev_netif->pstlan;
        if (plan != NULL)
        {
            lan_remove2wan(plan, getWan(plan->sim_cid >> 4, plan->sim_cid & 0x0f));
            lan_destroy(plan);
        }
        if (session->dev_netif->sim_cid == (NAT_SIM << 4 | NAT_CID))
        {
            OSI_LOGI(0x10007ace, "netdevDisconnect 6666 netif_set_default");
            netif_set_default(NULL);
        }
        session->dev_netif = NULL;
    }
#if defined CONFIG_FIBOCOM_DEV_MODEM_SOCKET
    struct lan_netif *plwip_lan = getLan(NAT_SIM, NAT_CID, NAT_LAN_IDX_LWIP);
    if (plwip_lan != NULL)
    {
        OSI_PRINTFE("remove lan_dev_netif!");
        #if 0
        lan_remove2wan(plwip_lan, NULL);
        #endif
        lan_destroy(plwip_lan);
    }
#endif
    session->is_created = false;
    osiExitCritical(critical);
    OSI_LOGI(0x10007acf, "netdevDisconnect 7777");
    return true;
}
netSession_t *prvNdevLanOnly_SessionCreate(uint8_t sim, uint8_t cid)
{
    static netSession_t s_net_nat_session;
    OSI_LOGI(0x1000757d, "Net session create");
    uint8_t T_cid = sim << 4 | cid;

    //No Wan
    struct netif *wan_netif = netif_get_by_cid_type(T_cid, NETIF_LINK_MODE_NAT_WAN);
    if (wan_netif != NULL)
    {
        return NULL;
    }

    //create lan
    struct lan_netif *plan = newLan(NAT_SIM, NAT_CID, NAT_LAN_IDX_RNDIS);
    if (plan == NULL)
        return NULL;
    if (lan_create(plan, sim, cid, netif_gprs_nat_lan_ndev_init, wan_to_netdev_lan_datainput) != true)
        return NULL;
    struct netif *netif = plan->netif;

#ifdef CONFIG_FIBOCOM_DEV_MODEM_SOCKET
    /*create 192.168.x.1 of LAN*/
    struct lan_netif *plan_dev = newLan(NAT_SIM, NAT_CID, NAT_LAN_IDX_LWIP);
    if (plan_dev == NULL)
    {
        OSI_PRINTFE("plan_dev new fail!");
        return NULL;
    }
    if (lan_dev_create(plan_dev, NAT_SIM, NAT_CID, netif_lan_dev_lwip_init, nat_lan_lwip_tcpip_input) != true)
    {
        OSI_PRINTFE("lan_dev_create fail!");
        return NULL;
    }


#endif



    //set default netif for lwip
    struct lan_netif *plwip_lan = getLan(NAT_SIM, NAT_CID, NAT_LAN_IDX_LWIP);
    if (plwip_lan->isUsing == 1)
        lan_setDefaultNetif(plwip_lan);

    //Session setting
    uint32_t critical = osiEnterCritical();
    netSession_t *session = &s_net_nat_session;
    if (session->is_created == true)
    {
        osiExitCritical(critical);
        OSI_LOGI(0x1000757f, "Net session create already created");
        return session;
    }
    memset(session, 0, sizeof(netSession_t));
    session->dev_netif = netif;
    OSI_LOGI(0x10007580, "Net end session %x\n", session);
    session->is_created = true;
    osiExitCritical(critical);
    return session;
}

#if defined CONFIG_FIBOCOM_DEV_MODEM_SOCKET

static err_t
ip4_lan_dev_forward_output(struct netif *netif, struct pbuf *p,
                  ip_addr_t *ipaddr)
{
    uint16_t offset = 0;
    struct pbuf *q = NULL;
    netdevIntf_t *nc = &gNetIntf;
    drvEthReq_t *tx_req = NULL;
    uint32_t size = 0;

    if (nc->mutex != NULL)
    {
        osiMutexLock(nc->mutex);
    }
#if defined CONFIG_FIBOCUS_HAIKANG_FEATURE
    uint8_t reCnt = 0;//SPCSS01101537
#endif
    while ((nc->netdevIsSuspend == false) && ((tx_req = drvEtherTxReqAlloc(nc->ether)) == NULL))//SPCSS01101537
    {
        osiThreadSleep(1);
#if defined CONFIG_FIBOCUS_HAIKANG_FEATURE
        reCnt++; //SPCSS01101537
        if (nc->ether == NULL || reCnt > 10) //SPCSS01101537
        {
            OSI_PRINTFE("ip4_lan_dev_forward_output, Tx Req Alloc fail reCnt = %d", reCnt);
            if (nc->mutex != NULL)
            {
                osiMutexUnlock(nc->mutex);
            }
            pbuf_free(p);//SPCSS01101537
            return -1;
        }

#else
        if (nc->ether == NULL)
        {
            if (nc->mutex != NULL)
            {
                osiMutexUnlock(nc->mutex);
            }
            return -1;
        }
#endif
    }

    if (nc->netdevIsSuspend == true)
    {
        if (nc->mutex != NULL)
        {
            osiMutexUnlock(nc->mutex);
        }
        if (tx_req != NULL)
            drvEtherTxReqFree(nc->ether, tx_req);
        return -1;
    }
    if (nc->mutex != NULL)
    {
        osiMutexUnlock(nc->mutex);
    }

    uint8_t *pData = tx_req->payload->data;
    size = p->tot_len;

    for (q = p; q != NULL; q = q->next)
    {
        memcpy(&pData[offset], q->payload, q->len);
        offset += q->len;
    }

    //handleInputDNSPackage(tx_req->payload, p->tot_len);
    sys_arch_dump(pData, p->tot_len);

    netif->u32LwipULSize += p->tot_len;
#ifdef CONFIG_NET_TRACE_IP_PACKET
    uint8_t *ipdata = pData;
    uint16_t identify = (ipdata[4] << 8) + ipdata[5];
    OSI_PRINTFE("ip4_lan_dev_forward_output Wan to NC identify %04x", identify);
#endif
    OSI_PRINTFE("ip4_lan_dev_forward_output Wan to NC %d", p->tot_len);
    //pbuf_free(p);
    if (nc->mutex != NULL)
    {
        osiMutexLock(nc->mutex);
    }
    if (nc->netdevIsSuspend == false)
    {
        if (!drvEtherTxReqSubmit(nc->ether, tx_req, size))
        {
            OSI_PRINTFE("ip4_lan_dev_forward_output Wan to NC error %d", size);
            if (nc->mutex != NULL)
            {
                osiMutexUnlock(nc->mutex);
            }
            return -1;
        }
    }
    else
    {
        drvEtherTxReqFree(nc->ether, tx_req);
    }
    if (nc->mutex != NULL)
    {
        osiMutexUnlock(nc->mutex);
    }
    return 0;
}
err_t netif_lan_dev_lwip_init(struct netif *netif)
{
    /* initialize the snmp variables and counters inside the struct netif
   * ifSpeed: no assumption can be made!
   */
    netif->name[0] = 'G';
    netif->name[1] = 'P';

#if LWIP_IPV4
    netif->output = (netif_output_fn)ip4_lan_dev_forward_output;
#endif

    netif->mtu = GPRS_MTU;

    if (NULL == netif_default)
        netif_set_default(netif);

    return ERR_OK;
}

bool ip4_dev_lan_forward(struct pbuf *p, struct netif *inp)
{
    bool flag = false;
    if (IP_HDR_GET_VERSION(p->payload) == 4)
    {
        //find lan netif with same CID IPV4 addr to send
        struct netif *netif;
        NETIF_FOREACH(netif)
        {
            if (NETIF_LINK_MODE_NAT_LWIP_DEV_LAN == netif->link_mode)
            {
                struct ip_hdr *iphdr = p->payload;
                struct udp_hdr       *udphdr;
                if (IP_PROTO_UDP == IPH_PROTO(iphdr))
                {
                    OSI_PRINTFE("ip4_dev_lan_forward is IP_PROTO_UDP");
                    udphdr = (struct udp_hdr *)ip4_nat_check_header(p, sizeof(struct udp_hdr));
                    if (DNS_PORT == ntohs(udphdr->dest))
                    {
                        OSI_PRINTFE("ip4_dev_lan_forward is IP_PROTO_UDP destport is %u", ntohs(udphdr->dest));
                        continue;
                    }
                }
                ip4_addr_t current_iphdr_dest;
                ip4_addr_t *current_netif_addr;
                ip4_addr_t local_cycle_addr;
                IP4_ADDR(&local_cycle_addr, 127,0,0,1);
                ip4_addr_copy(current_iphdr_dest, iphdr->dest);
                current_netif_addr = (ip4_addr_t *)netif_ip4_addr(netif);
                OSI_PRINTFE("ip4_dev_lan_forward netif is %p", netif);
                OSI_PRINTFE("ip4_dev_lan_forward current_iphdr_dest is %s", ip4addr_ntoa(&current_iphdr_dest));
                OSI_PRINTFE("ip4_dev_lan_forward current_netif_addr is %s", ip4addr_ntoa(current_netif_addr));
                if (current_iphdr_dest.addr == current_netif_addr->addr && (current_iphdr_dest.addr != local_cycle_addr.addr))
                {
                    OSI_PRINTFE("ip4_dev_lan_forward IPV4 to Lan netif");
                    pbuf_ref(p);
                    netif->input(p, netif);
                    flag = true;
                    break;
                }
            }
        }
    }
    return flag;
}

static bool Isdevlanlwip(netSession_t *session, const void *data, size_t size)
{
    bool result = false;
    struct netif *inp_netif = session->dev_netif;
    if (inp_netif == NULL)
        return false;

    if (size <= 0)
        return false;

    struct pbuf *p, *q;
    int offset = 0;
    uint8_t *pData = (uint8_t *)data;
    p = (struct pbuf *)pbuf_alloc(PBUF_RAW, size, PBUF_POOL);
    if (NULL == p)
        return false;
    if (size > p->len)
    {
        for (q = p; size > q->len; q = q->next)
        {
            memcpy(q->payload, &(pData[offset]), q->len);
            size -= q->len;
            offset += q->len;
        }
        if (size != 0)
        {
            memcpy(q->payload, &(pData[offset]), size);
        }
    }
    else
    {
        memcpy(p->payload, &(pData[offset]), size);
    }
#if LWIP_IPV6
    if (IP_HDR_GET_VERSION(p->payload) == 6)
    {
        OSI_PRINTFE("Isdevlanlwip IP PACKET VERSION is V6");
    }
    else
#endif
    {
#if LWIP_TCPIP_CORE_LOCKING
        LOCK_TCPIP_CORE();
#endif
        result = ip4_dev_lan_forward(p, inp_netif);
#if LWIP_TCPIP_CORE_LOCKING
        UNLOCK_TCPIP_CORE();
#endif
        OSI_PRINTFE("Isdevlanlwip %d", result);
    }
    pbuf_free(p);
    return result;
}

#if defined CONFIG_FIBOCUS_HAIKANG_FEATURE
void saveDevLanSocketIP()
{
    char *ip_gw[4] = {"192.168.0.1", "192.168.225.1", "192.168.226.1", "192.168.224.1"};
    int cnt = 2;
    uint32_t ip = 0;
    int32_t temp_ip = 0;

    cnt = fibo_get_current_lan_ipaddr_haikang();
    if (cnt < 1 || cnt > 4)
    {
        cnt = 2;
    }

    cnt -= 1;
    OSI_PRINTFE("[fibocom] getDevLanSocketIP save %s to DEV_LAN_SOCKET_IP in nv", ip_gw[cnt]);
    ip = inet_addr((const char *)ip_gw[cnt]);
    temp_ip = 4294967296 - ntohl(ip);
    ffw_nv_set("DEV_LAN_SOCKET_IP", &temp_ip, 1);

    return;
}
#endif

struct netif *TCPIP_lan_dev_netif_create(uint8_t nCid, uint8_t nSimId, uint8_t nIdx, netif_init_fn init, netif_input_fn input)
{
    struct netif *netif = NULL;
    uint8_t T_cid = nSimId << 4 | nCid;
    ip4_addr_t ip4;
    ip4_addr_t ip4_gw;
    ip4_addr_t ip4_netMast;

    ip4_addr_t ip_addr;
    char *ip_split[20];
    char *outer_ptr = NULL;
    char lan_ip_addr[32] = {0};
    int   i = 0;
    struct sockaddr_in stLocalAddr = {0};

#if defined CONFIG_FIBOCUS_HAIKANG_FEATURE
    saveDevLanSocketIP();
#endif
    int32_t val[1]={0};
    ffw_nv_get("DEV_LAN_SOCKET_IP",val,1);
    OSI_PRINTFE("[fibocom] TCPIP_lan_dev_netif_create get IP inet_addr is %d", val[0]);
    stLocalAddr.sin_addr.s_addr = htonl(4294967296 - (unsigned)val[0]);

    const char *ip_addr_t1 = inet_ntoa(stLocalAddr.sin_addr);
    OSI_PRINTFE("[fibocom] TCPIP_lan_dev_netif_create get IP addr is %s", ip_addr_t1);
    ip4addr_aton(ip_addr_t1, &ip_addr);
    ip4_addr_copy(ip4, ip_addr);

    memcpy(lan_ip_addr, ip_addr_t1, strlen(ip_addr_t1));
    char *lan_ip_t = lan_ip_addr;
    while((ip_split[i]=strtok_r(lan_ip_t,".",&outer_ptr))!=NULL)
    {
      if(atoi(ip_split[i]) < 0 || atoi(ip_split[i]) > 255)
      {
          OSI_LOGI(0, "TCPIP_lan_dev_netif_create set LAN ip is NULL!!! ");
          return NULL;
      }
      i++;
      lan_ip_t = NULL;
    }
    IP4_ADDR(&ip4_gw, atoi(ip_split[0]), atoi(ip_split[1]), atoi(ip_split[2]), 254);

    IP4_ADDR(&ip4_netMast, 255, 255, 255, 0);
    netif = malloc(sizeof(struct netif));

    if (netif != NULL)
        memset(netif, 0, sizeof(struct netif));
    else
        return NULL;


    OSI_PRINTFE("TCPIP_lan_dev_netif_create cid %d simid %d\n", nCid, nSimId);
    netif->sim_cid = T_cid;
    netif->is_ppp_mode = 0;
    netif->link_mode = NETIF_LINK_MODE_NAT_LWIP_DEV_LAN;
    netif->pdnType = CFW_GPRS_PDP_TYPE_PPP + 1;
    netif->is_used = 1;


#if LWIP_TCPIP_CORE_LOCKING
    LOCK_TCPIP_CORE();
#endif
    netif_add(netif, &ip4, NULL, NULL, NULL, init, input);
    netif_set_up(netif);
    netif_set_link_up(netif);

#if LWIP_TCPIP_CORE_LOCKING
    UNLOCK_TCPIP_CORE();
#endif

    OSI_PRINTFE("TCPIP_lan_dev_netif_create, netif = %p", netif);
    return netif;
}
uint8_t lan_dev_create(void *lannetif, uint8_t sim, uint8_t cid, netif_init_fn init, netif_input_fn input)
{
    uint8_t iRet = false;
    struct lan_netif *plan = (struct lan_netif *)lannetif;
    if (plan == NULL)
        goto end;
    if ((plan->netif = TCPIP_lan_dev_netif_create(cid, sim, plan->iIdx, init, input)) == NULL)
        goto end;
    plan->netif->pstlan = plan;
#if 0
    if (sim == NAT_SIM && cid == NAT_CID)
    {
        //add to list if without netwok
        lan_add2wan(lannetif, NULL);
    }
#endif
    iRet = true;
end:
    OSI_PRINTFE("lan_dev_create iRet = %d", iRet);
    return iRet;
}


#endif

#ifdef CONFIG_FIBOCOM_BASE
static void*
ip4_nat_check_header(struct pbuf *p, u16_t min_size)
{
  void *ret = NULL;
  struct ip_hdr  *iphdr = (struct ip_hdr*)p->payload;
  s16_t iphdr_len = IPH_HL(iphdr) * 4;
  LWIP_DEBUGF(NAT_DEBUG, (0x10007c48, "ip4_nat_check_header: iphdr_len %d\n", iphdr_len));
  if(p->len > iphdr_len)
  {
    if(!pbuf_header(p, -iphdr_len)) {
      if(p->tot_len >= min_size) {
        ret = p->payload;
      }
      /* Restore pbuf payload pointer from previous header check. */
      pbuf_header(p, iphdr_len);
    }
  }else
  {
    uint16_t offset = 0;
    struct pbuf *q1 = NULL;
    for (q1 = p; q1!= NULL; q1 = q1->next)
    {
        if(offset + q1->len > iphdr_len)
        {
            if(q1->len > min_size)
            {
                ret = q1->payload + offset - iphdr_len;
            }else
            {
                break;
            }
        }
        offset += q1->len;

    }
  }
  return ret;
}
bool res_dev_ping_gw(struct pbuf *p, struct netif *inp)
{
    bool flag = false;
    if (IP_HDR_GET_VERSION(p->payload) == 4)
    {
        //find lan netif with same CID IPV4 addr to send
        struct netif *netif;
        NETIF_FOREACH(netif)
        {
            if ((NETIF_LINK_MODE_NAT_LWIP_LAN == netif->link_mode || NETIF_LINK_MODE_NAT_PPP_LAN == netif->link_mode || NETIF_LINK_MODE_NAT_NETDEV_LAN == netif->link_mode))
            {
                struct ip_hdr *iphdr = (struct ip_hdr*)p->payload;
                struct icmp_echo_hdr *icmphdr;
                if (IP_PROTO_ICMP == IPH_PROTO(iphdr))
                {
                    OSI_PRINTFE("res_dev_ping_gw is IP_PROTO_ICMP");
                    icmphdr = (struct icmp_echo_hdr *)ip4_nat_check_header(p, sizeof(struct icmp_echo_hdr));

                    ip4_addr_t current_iphdr_dest;
                    ip4_addr_t *current_netif_addr;

                    ip4_addr_t *gw_ipaddr = ip_2_ip4(&(netif->gw));
                    ip4_addr_copy(current_iphdr_dest, iphdr->dest);
                    current_netif_addr = (ip4_addr_t *)netif_ip4_addr(netif);
                    OSI_PRINTFE("res_dev_ping_gw netif is %p", netif);
                    OSI_PRINTFE("res_dev_ping_gw current_iphdr_dest is %s", ip4addr_ntoa(&current_iphdr_dest));
                    OSI_PRINTFE("res_dev_ping_gw current_netif_addr is %s", ip4addr_ntoa(current_netif_addr));
                    OSI_PRINTFE("res_dev_ping_gw current_netif_gw_addr is %s", ip4addr_ntoa(gw_ipaddr));
                    if (current_iphdr_dest.addr == gw_ipaddr->addr)
                    {
                        OSI_PRINTFE("res_dev_ping_gw IPV4 to netif");

                        /*response icmp*/
                        /*IP ADDR*/
                        ip4_addr_copy(iphdr->dest, iphdr->src);
                        ip4_addr_copy(iphdr->src, *gw_ipaddr);
                        /*ICMP TYPE*/
                        ICMPH_TYPE_SET(icmphdr, ICMP_ER);

                        /* adjust the checksum */
                        if (icmphdr->chksum > PP_HTONS(0xffffU - (ICMP_ECHO << 8)))
                        {
                            icmphdr->chksum = (u16_t)(icmphdr->chksum + PP_HTONS((u16_t)(ICMP_ECHO << 8)) + 1);
                        }
                        else
                        {
                            icmphdr->chksum = (u16_t)(icmphdr->chksum + PP_HTONS(ICMP_ECHO << 8));
                        }

                        IPH_TTL_SET(iphdr, ICMP_TTL);
                        IPH_CHKSUM_SET(iphdr, 0);
                    #if CHECKSUM_GEN_IP
                        IF__NETIF_CHECKSUM_ENABLED(inp, NETIF_CHECKSUM_GEN_IP) {
                          IPH_CHKSUM_SET(iphdr, inet_chksum(iphdr, IP_HLEN));
                        }
                    #endif /* CHECKSUM_GEN_IP */

                        pbuf_ref(p);
                        wan_to_netdev_lan_datainput(p, netif);
                        flag = true;
                        break;
                    }
                }
            }
        }
    }
    return flag;
}


static bool IsdevPingGW(netSession_t *session, const void *data, size_t size)
{
    bool result = false;
    struct netif *inp_netif = session->dev_netif;
    if (inp_netif == NULL)
        return false;

    if (size <= 0)
        return false;

    struct pbuf *p, *q;
    int offset = 0;
    uint8_t *pData = (uint8_t *)data;
    p = (struct pbuf *)pbuf_alloc(PBUF_RAW, size, PBUF_POOL);
    if (NULL == p)
        return false;
    if (size > p->len)
    {
        for (q = p; size > q->len; q = q->next)
        {
            memcpy(q->payload, &(pData[offset]), q->len);
            size -= q->len;
            offset += q->len;
        }
        if (size != 0)
        {
            memcpy(q->payload, &(pData[offset]), size);
        }
    }
    else
    {
        memcpy(p->payload, &(pData[offset]), size);
    }
#if LWIP_IPV6
    if (IP_HDR_GET_VERSION(p->payload) == 6)
    {
        OSI_PRINTFE("Isdevlanlwip IP PACKET VERSION is V6");
    }
    else
#endif
    {
#if LWIP_TCPIP_CORE_LOCKING
        LOCK_TCPIP_CORE();
#endif
        result = res_dev_ping_gw(p, inp_netif);
#if LWIP_TCPIP_CORE_LOCKING
        UNLOCK_TCPIP_CORE();
#endif
        OSI_PRINTFE("IsdevPingGW %d", result);
    }
    pbuf_free(p);
    return result;
}
#endif

#endif
