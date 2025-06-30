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
#include "lwip/prot/ip.h"
#include "netdev_interface_imp.h"
#include "netdev_interface.h"
#include "netmain.h"
#include "hal_chip.h"
#if IP_NAT
#include "netdev_interface_nat_lan.h"
#include "lwip/ip4_nat.h"
#endif

#ifdef CONFIG_NET_LTE_PSPACKET_SUPPORT
#include "drv_md_ipc.h"
#define PS_UL_HDR_LEN IPC_PS_UL_HDR_LEN
#define MAX_NETCARD_BUF_COUNT 300
#endif
#include "fibocom.h"

extern void lwip_pspathDataInput(void *ctx, drvPsIntf_t *p);
extern bool isDhcpPackage(drvEthPacket_t *pkt, uint32_t size);
extern void dhcps_reply(drvEther_t *ether, drvEthPacket_t *pkt, uint32_t size, struct netif *netif);
extern bool isARPPackage(drvEthPacket_t *pkt, uint32_t size);
extern void ARP_reply(drvEther_t *ether, drvEthPacket_t *pkt, uint32_t size, struct netif *netif);
extern bool isRouterSolicitationPackage(drvEthPacket_t *pkt, uint32_t size);
extern void routerAdvertisement_reply(drvEther_t *ether, drvEthPacket_t *pkt, uint32_t size, struct netif *netif);
extern bool isRouterAdvertisementPackage(drvEthPacket_t *pkt, uint32_t size);
extern bool isNeighborSolicitationPackage(drvEthPacket_t *pkt, uint32_t size);
extern void neighborAdvertisement_reply(drvEther_t *ether, drvEthPacket_t *pkt, uint32_t size, struct netif *netif);

extern void ND6_Dhcp6_Info_req_reply(drvEther_t *ether, drvEthPacket_t *pkt, uint32_t size, struct netif *netif);
extern bool ND6_isRAPackage(drvEthPacket_t *pkt);
extern void ND6_RA_reply(uint8_t *pkt, uint32_t size);
extern bool ND6_isDhcp6infoReqPackage(drvEthPacket_t *pkt);
#ifdef CONFIG_FIBOCOM_AT_RNDIS_SUPPORT
extern int fibo_GetRndisCid();
#endif

static bool prvNdevDataToPs(netSession_t *session, const void *data, size_t size);
#ifndef CONFIG_NET_LTE_PSPACKET_SUPPORT
static void prvPsDataToNdev(void *ctx, drvPsIntf_t *p);
#endif
static netSession_t *prvNdevSessionCreate();
static bool prvNdevSessionDelete(netSession_t *session);

#ifdef CONFIG_WDT_ENABLE
extern void drvFeedWdt(void);
#endif
netdevIntf_t gNetIntf = {.clk_registry = {.tag = HAL_NAME_NETCARD}, .mutex = NULL};
static bool g_bNeedSendEthConEvt = false;
#if defined(CONFIG_FIBOCOM_BASE)
extern int fibo_get_issleep();
extern int32_t fibo_get_gtset_ecmauto_flag();
#endif
#ifdef CONFIG_FIBOCOM_AT_RNDIS_SUPPORT
extern uint32_t g_lan_data_received;
#endif
void netdevCheckEthConEvt(void)
{
    OSI_LOGI(0x10009510, "netdevCheckEthConEvt g_bNeedSendEthConEvt = %d", g_bNeedSendEthConEvt);
    if (g_bNeedSendEthConEvt == true)
    {
        osiEvent_t tcpip_event = {0};
        tcpip_event.id = EV_TCPIP_ETHCARD_CONNECT;
        osiEventSend(netGetTaskID(), (const osiEvent_t *)&tcpip_event);
    }
}
static void prvEthEventCB(uint32_t evt_id, void *ctx)
{
    netdevIntf_t *nc = (netdevIntf_t *)ctx;
    OSI_LOGI(0x10007569, "Ethernet event cb %d (e: %p, s: %p)", evt_id, nc->ether, nc->session);
    if (nc->mutex != NULL)
    {
        osiMutexLock(nc->mutex);
    }
    if (DRV_ETHER_EVENT_CONNECT == evt_id)
    {
        osiEvent_t tcpip_event = {0};
        tcpip_event.id = EV_TCPIP_ETHCARD_CONNECT;
        if (netGetTaskID() == NULL)
            g_bNeedSendEthConEvt = true;
        else
            osiEventSend(netGetTaskID(), (const osiEvent_t *)&tcpip_event);
        nc->netdevIsConnected = true;
        nc->netdevIsSuspend = false;
        osiRequestSysClkActive(&nc->clk_registry);
        OSI_LOGI(0x1000756a, "net card EV_TCPIP_ETHCARD_CONNECT");
    }

/*
MTC0693-35
Do not close NET after waking up from sleep(do not execute The EV_TCPIP_ETHCARD_DISCONNECT event)
*/
#if defined(CONFIG_FIBOCOM_BASE)
#if defined(CONFIG_FIBOCOM_ECM_EXTREME_PERFORMANCE_TIME)
    if(0 == fibo_get_issleep())
    {
        OSI_PRINTFI("wake---");
        if (DRV_ETHER_EVENT_DISCONNECT == evt_id)
        {
            osiEvent_t tcpip_event = {0};
            tcpip_event.id = EV_TCPIP_ETHCARD_DISCONNECT;
            osiEventSend(netGetTaskID(), (const osiEvent_t *)&tcpip_event);
            nc->netdevIsConnected = false;
            nc->netdevIsSuspend = false;
            osiReleaseClk(&nc->clk_registry);
            OSI_LOGI(0x1000756b, "net card EV_TCPIP_ETHCARD_DISCONNECT");
        }
    }
#else
    if (DRV_ETHER_EVENT_DISCONNECT == evt_id)
    {
        osiReleaseClk(&nc->clk_registry);
        OSI_PRINTFE("prvEthEventCB auto act Rndis gtset is %d", fibo_get_gtset_ecmauto_flag());
        if(fibo_get_gtset_ecmauto_flag() != 0)
        {
           if(0 == fibo_get_issleep())
           {
                OSI_PRINTFI("wake---");
                osiEvent_t tcpip_event = {0};
                tcpip_event.id = EV_TCPIP_ETHCARD_DISCONNECT;
                osiEventSend(netGetTaskID(), (const osiEvent_t *)&tcpip_event);
                nc->netdevIsConnected = false;
                nc->netdevIsSuspend = false;
                osiReleaseClk(&nc->clk_registry);
                OSI_LOGI(0x1000756b, "net card EV_TCPIP_ETHCARD_DISCONNECT");
           }
       }
       else
       {

            osiEvent_t tcpip_event = {0};
            tcpip_event.id = EV_TCPIP_ETHCARD_DISCONNECT;
            osiEventSend(netGetTaskID(), (const osiEvent_t *)&tcpip_event);
            nc->netdevIsConnected = false;
            nc->netdevIsSuspend = false;
            osiReleaseClk(&nc->clk_registry);
            OSI_LOGI(0x1000756b, "net card EV_TCPIP_ETHCARD_DISCONNECT");
        }
    }
#endif
#else
    if (DRV_ETHER_EVENT_DISCONNECT == evt_id)
    {
        osiEvent_t tcpip_event = {0};
        tcpip_event.id = EV_TCPIP_ETHCARD_DISCONNECT;
        osiEventSend(netGetTaskID(), (const osiEvent_t *)&tcpip_event);
        nc->netdevIsConnected = false;
        nc->netdevIsSuspend = false;
        osiReleaseClk(&nc->clk_registry);
        OSI_LOGI(0x1000756b, "net card EV_TCPIP_ETHCARD_DISCONNECT");
    }
#endif
    if (DRV_ETHER_EVENT_SUSPEND_DISCONNECT == evt_id)
    {
        nc->netdevIsSuspend = true;
        osiReleaseClk(&nc->clk_registry);
        OSI_LOGI(0x1000a105, "net card DRV_ETHER_EVENT_SUSPEND_DISCONNECT");
    }

    if (DRV_ETHER_EVENT_RESUME_CONNECT == evt_id)
    {
        nc->netdevIsSuspend = false;
#ifndef CONFIG_NET_LTE_PSPACKET_SUPPORT
        netSession_t *session = nc->session;
        if (((session) != NULL) && (session->dev_netif != NULL) && (session->dev_netif->pspathIntf != NULL))
            drvPsIntfSetDataArriveCB(session->dev_netif->pspathIntf, prvPsDataToNdev);
#endif
        osiRequestSysClkActive(&nc->clk_registry);
        OSI_LOGI(0x1000a106, "net card DRV_ETHER_EVENT_RESUME_CONNECT");
    }

    if (nc->mutex != NULL)
    {
        osiMutexUnlock(nc->mutex);
    }
}

static void prvEthUploadDataCB(drvEthPacket_t *pkt, uint32_t usize, void *ctx)
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
    //OSI_LOGI(0x1000756d, "prvEthUploadDataCB data size: %d", size);

    if (size > ETH_HLEN)
    {
        uint8_t *ipData = pkt->data;
#ifdef CONFIG_NET_TRACE_IP_PACKET
        uint8_t *ipdata = ipData;
        uint16_t identify = (ipdata[4] << 8) + ipdata[5];
        OSI_LOGI(0x10007adb, "prvEthUploadDataCB identify %04x", identify);
#endif
        //sys_arch_dump(ipData, size - ETH_HLEN);
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
        dhcps_reply(nc->ether, pkt, size, session->dev_netif);
    }
    else if (session->dev_netif != NULL && (__ntohs(pkt->ethhd.h_proto) == ETH_P_IPV6) && (ND6_isDhcp6infoReqPackage(pkt) == true))
    {
        //send Dhcp6_info_reply
        ND6_Dhcp6_Info_req_reply(nc->ether, pkt, size, session->dev_netif);
    }
    else
    {
        prvNdevDataToPs(session, pkt->data, size - ETH_HLEN);
    }
}

#ifdef CONFIG_NET_LTE_PSPACKET_SUPPORT
void SetNetdevThreadPri(uint32_t priority)
{
    netdevIntf_t *nc = &gNetIntf;
    if (nc->ether != NULL && nc->ether->thread != NULL)
    {
        osiThreadSetPriority((osiThread_t *)nc->ether->thread, priority);
    }
}

extern void psHeaderFreeAll(struct ps_header *psData);
#define UL_MAX_CACHE_NUM 10
#define UL_CACHE_TIMEOUT_MS 6
#define UL_CACHE_MIN_INTERVAL_US (5000)

void gprs_ps_data_to_netcard(void *ctx)
{
    struct ps_header *psData = (struct ps_header *)ctx;
    if (psData == NULL)
        return;

    netdevIntf_t *nc = &gNetIntf;
    if (nc == NULL || nc->session == NULL)
    {
        psHeaderFreeAll(psData);
        return;
    }

    if (nc->netdevIsSuspend == true)
    {
        psHeaderFreeAll(psData);
        return;
    }

    if (nc->mutex != NULL)
    {
        osiMutexLock(nc->mutex);
    }

    //OSI_LOGI(0x1000a98d, "gprs_ps_data_to_netcard nc->session = %p", nc->session);
    //uint8_t *pData = NULL;
    int len = 0;
    int readLen = 0;
#if LWIP_TCPIP_CORE_LOCKING
    LOCK_TCPIP_CORE();
#endif
    do
    {
        readLen = psData->len;
        len = readLen;
        //pData = ((uint8_t *)psData) + psData->data_off;
        //OSI_LOGI(0x1000a98e, "psData buffer pData %plen %d", pData, len);
        if (len > 0)
        {
            //sys_arch_dump(pData, len);
        }
        if (nc->session != NULL && len > 0)
        {
            if (nc->session->dev_netif->link_mode == NETIF_LINK_MODE_NETCARD)
            {
                struct ps_header *psDataNext = (struct ps_header *)(psData->next);
                if (nc->session->buf_list_count < MAX_NETCARD_BUF_COUNT)
                {
                    // insert to TAILQ
                    ps_buf_t *ps_buf = (ps_buf_t *)malloc(sizeof(ps_buf_t));
                    ps_buf->psData = psData;
                    ps_buf->psData->next = 0;
                    TAILQ_INSERT_TAIL(&(nc->session->buf_list), ps_buf, iter);
                    nc->session->buf_list_count++;
                    //OSI_LOGI(0x1000a98f, "gprs_ps_data_to_netcard bufcount %d", nc->session->buf_list_count);
                }
                else
                {
                    OSI_LOGI(0x1000a990, "gprs_ps_data_to_netcard MAX_NETCARD_BUF_COUNT drop psData buffer ! %x", psData);
                    free(psData);
                }
                psData = psDataNext;
            }
            else
            {
                psHeaderFreeAll(psData);
                psData = NULL;
            }
        }
        else
        {
            psHeaderFreeAll(psData);
            psData = NULL;
        }
    } while (psData != NULL);
#if LWIP_TCPIP_CORE_LOCKING
    UNLOCK_TCPIP_CORE();
#endif
    if (nc->session->dl_read_notify != NULL)
        osiNotifyTrigger(nc->session->dl_read_notify);
    if (nc->mutex != NULL)
    {
        osiMutexUnlock(nc->mutex);
    }
}
void _gprs_ps_data_to_netcard(void *ctx)
{
    netdevIntf_t *nc = &gNetIntf;
    //OSI_LOGI(0x1000a991, "_gprs_ps_data_to_netcard");
    uint8_t *pData = NULL;
    ps_buf_t *ps_buf = NULL;

    if (nc == NULL || nc->session == NULL || nc->session->dev_netif == NULL || nc->netdevIsSuspend == true)
    {
        //psHeaderFreeAll(psData);
        //clean buf list
#ifdef CONFIG_NET_LTE_PSPACKET_SUPPORT
        ps_buf_t *ps_buf = NULL;
        if (nc->session != NULL)
        {
            while ((ps_buf = TAILQ_FIRST(&(nc->session->buf_list))) != NULL)
            {
                struct ps_header *psData = ps_buf->psData;
                TAILQ_REMOVE(&(nc->session->buf_list), ps_buf, iter);
                if (NULL != psData)
                    free(psData);
                free(ps_buf);
            }
            nc->session->buf_list_count = 0;
        }
#endif
        return;
    }

    struct ps_header *psData = NULL;
    while ((ps_buf = TAILQ_FIRST(&(nc->session->buf_list))) != NULL)
    {
        psData = ps_buf->psData;
#if LWIP_TCPIP_CORE_LOCKING
        LOCK_TCPIP_CORE();
#endif
        TAILQ_REMOVE(&(nc->session->buf_list), ps_buf, iter);
        if (nc->session->buf_list_count > 0)
            nc->session->buf_list_count--;
#if LWIP_TCPIP_CORE_LOCKING
        UNLOCK_TCPIP_CORE();
#endif
        //free iter
        free(ps_buf);

        drvEthReq_t *tx_req = NULL;
        if (nc->mutex != NULL)
        {
            osiMutexLock(nc->mutex);
        }
        if ((nc->netdevIsSuspend == false) && ((tx_req = drvEtherTxReqAlloc(nc->ether)) == NULL))
        {
            OSI_LOGI(0x1000a992, "No USB TX Memory !!!");
            //free ps Data
            free(psData);
            psData = NULL;

            if (nc->mutex != NULL)
            {
                osiMutexUnlock(nc->mutex);
            }
            return;
        }

        if (nc->netdevIsSuspend == true)
        {
            ps_buf_t *ps_buf = NULL;
            while ((ps_buf = TAILQ_FIRST(&(nc->session->buf_list))) != NULL)
            {
                struct ps_header *psData = ps_buf->psData;
                TAILQ_REMOVE(&(nc->session->buf_list), ps_buf, iter);
                if (NULL != psData)
                {
                    free(psData);
                    psData = NULL;
                }
                free(ps_buf);
            }
            nc->session->buf_list_count = 0;
            if (nc->mutex != NULL)
            {
                osiMutexUnlock(nc->mutex);
            }
            if (tx_req != NULL)
                drvEtherTxReqFree(nc->ether, tx_req);
            return;
        }
        if (nc->mutex != NULL)
        {
            osiMutexUnlock(nc->mutex);
        }

        uint8_t *rdata = tx_req->payload->data;
        pData = ((uint8_t *)psData) + psData->data_off;
        int rsize = psData->len;
        uint8_t *ipdata = pData;
        if ((IP_HDR_GET_VERSION(ipdata) != 6) && (IP_HDR_GET_VERSION(ipdata) != 4))
        {
            OSI_LOGE(0x1000a993, "psHeader data is NOT IPV4 or IPV6, drop it!");
            rsize = 0;
        }
        if (rsize <= 0)
        {
            drvEtherTxReqFree(nc->ether, tx_req);
        }
        if (rsize > 0)
        {
            //OSI_LOGI(0x1000a994, "psHeader->buf_size %d", rsize);

#ifdef CONFIG_NET_TRACE_IP_PACKET
            uint16_t identify = (ipdata[4] << 8) + ipdata[5];
            OSI_LOGI(0x1000a995, "gprs_ps_data_to_netcard identify %04x", identify);
#endif
            //sys_arch_dump(ipdata, rsize);
            memcpy(rdata, pData, rsize);
            nc->session->dev_netif->u32RndisDLSize += rsize;
            if (IP_HDR_GET_VERSION(rdata) == 6)
            {
                OSI_LOGI(0x10007abd, "net device recieve a ipv6 package, rsize = %d", rsize);
                if (ND6_isRAPackage(tx_req->payload))
                {
                    ND6_RA_reply(rdata, rsize);
                }
            }
            //OSI_LOGI(0x1000a107, "nc %p nc->ether %p", nc, nc->ether);

            if (nc->mutex != NULL)
            {
                osiMutexLock(nc->mutex);
            }
            if (nc->netdevIsSuspend == false)
            {
                if (!drvEtherTxReqSubmit(nc->ether, tx_req, rsize))
                {
                    OSI_LOGW(0x10007570, "PS to NC error %d", rsize);
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
        }
        //free ps Data
        free(psData);
        psData = NULL;
    }
}

extern void API_ULDataIpcHandle(struct ps_header **p_psIpcBufferList, u16_t v_availLen);
extern void *osiMemalign(size_t alignment, size_t size);

static void prvULCacheTimeout(void *param)
{
    netdevIntf_t *nc = &gNetIntf;
    if (nc->ether == NULL)
        return;

    netSession_t *session = (netSession_t *)param;
    OSI_LOGI(0x1000a996, "prvULCacheTimeout data_send_ps_header - ul_cache_num %d", session->ul_cache_num);

    if (nc->mutex != NULL)
    {
        osiMutexLock(nc->mutex);
    }
    struct ps_header *p_psIpcBufferList[1] = {0};
    p_psIpcBufferList[0] = (struct ps_header *)(session->ul_buf_head);
    session->ul_cache_num = 0;
    session->ul_buf_head = NULL;
    osiElapsedTimerStart(&session->ul_elapsed);

    API_ULDataIpcHandle(p_psIpcBufferList, 1);
    if (nc->mutex != NULL)
    {
        osiMutexUnlock(nc->mutex);
    }
}

static uint16_t data_send_ps_header(netSession_t *session, uint8_t nSimId, uint8_t nCid, const void *pData, uint16_t size)
{
    //OSI_LOGI(0x1000a997, "data_send_ps_header ----nSimId =%d nCid =%d pData = 0x%x size =%d -----------", nSimId, nCid, pData, size);
    if (size == 0)
        return 0;
    uint32_t ps_len = PS_UL_HDR_LEN + size + 28;
    uint32_t ps_len_align = OSI_ALIGN_UP(ps_len, 32); //32 align for cache
    struct ps_header *psHeader = (struct ps_header *)osiMemalign(32, ps_len_align);

    if (psHeader == NULL)
    {
        return 0;
    }

    memset(psHeader, 0, ps_len_align);
    psHeader->cid = nCid;
    psHeader->simid = nSimId;
    psHeader->len = size;
    psHeader->buf_size = ps_len;
    psHeader->data_off = PS_UL_HDR_LEN;
    uint8_t *p_ipData = (uint8_t *)psHeader + PS_UL_HDR_LEN;

    memcpy(p_ipData, pData, size);

    netdevIntf_t *nc = &gNetIntf;
    if (nc->mutex != NULL)
    {
        osiMutexLock(nc->mutex);
    }
    if (session->ul_cache_num == 0)
    {
        session->ul_buf_head = (void *)psHeader;
    }
    else
    {
        struct ps_header *psBuffer = (struct ps_header *)(session->ul_buf_head);
        uint16_t index = 0;
        while (psBuffer->next != 0 && index <= UL_MAX_CACHE_NUM)
        {
            psBuffer = (struct ps_header *)(psBuffer->next);
            index++;
        }
        uint32_t psBuffer_next = psBuffer->next;
        psBuffer->next = (uint32_t)psHeader;
        psHeader->next = psBuffer_next;
    }
    session->ul_cache_num++;

    if (session->ul_cache_num >= UL_MAX_CACHE_NUM ||
        osiElapsedTimeUS(&session->ul_elapsed) > UL_CACHE_MIN_INTERVAL_US)
    {
        //OSI_LOGI(0x1000a998, "data_send_ps_header - ul_cache_num %d", session->ul_cache_num);
        struct ps_header *p_psIpcBufferList[1] = {0};
        p_psIpcBufferList[0] = (struct ps_header *)(session->ul_buf_head);
        session->ul_buf_head = NULL;
        session->ul_cache_num = 0;
        osiElapsedTimerStart(&session->ul_elapsed);
        osiTimerStop(session->ul_cache_timer);

        API_ULDataIpcHandle(p_psIpcBufferList, 1);
    }
    else if (session->ul_cache_num == 1)
    {
        //OSI_LOGI(0x10007d81, "ul cache timer start");
        osiTimerStart(session->ul_cache_timer, UL_CACHE_TIMEOUT_MS);
    }
    if (nc->mutex != NULL)
    {
        osiMutexUnlock(nc->mutex);
    }
    return size;
}

static bool prvNdevDataToPs(netSession_t *session, const void *data, size_t size)
{
    struct netif *inp_netif = session->dev_netif;
    if (inp_netif == NULL)
        return false;

    if (size <= 0)
        return false;

    int written = 0;
    int nCid = inp_netif->sim_cid & 0x0f;
    int nSimId = (inp_netif->sim_cid & 0xf0) >> 4;

    extern bool NetifGetDPSDFlag(CFW_SIM_ID nSim);
    if (!NetifGetDPSDFlag(nSimId))
        written = data_send_ps_header(session, nSimId, nCid, data, size);
    else
        written = size;
    inp_netif->u32RndisULSize += written;
    return true;
}

#else
static void prvNdevProcessPsData(void *ctx)
{
    netdevIntf_t *nc = &gNetIntf;
    struct netif *nif = (struct netif *)ctx;
    if (nif == NULL)
    {
        OSI_LOGE(0x1000756e, "NC PsIntf get netif failed");
        osiPanic();
    }

    if (nc->netdevIsSuspend == true)
        return;

    int rsize = 0;
    do
    {
        drvEthReq_t *tx_req = NULL;
        if (nc->mutex != NULL)
        {
            osiMutexLock(nc->mutex);
        }
        while ((nc->netdevIsSuspend == false) && ((tx_req = drvEtherTxReqAlloc(nc->ether)) == NULL))
        {
            osiThreadSleep(1);
            if (nc->ether == NULL)
            {
                if (nc->mutex != NULL)
                {
                    osiMutexUnlock(nc->mutex);
                }
                return;
            }
        }

        if (nc->netdevIsSuspend == true)
        {
            if (nc->mutex != NULL)
            {
                osiMutexUnlock(nc->mutex);
            }
            if (tx_req != NULL)
                drvEtherTxReqFree(nc->ether, tx_req);
            return;
        }
        if (nc->mutex != NULL)
        {
            osiMutexUnlock(nc->mutex);
        }
        uint8_t *rdata = tx_req->payload->data;
        rsize = drvPsIntfRead(nif->pspathIntf, rdata, 1600);
        if (rsize <= 0)
        {
            drvEtherTxReqFree(nc->ether, tx_req);
        }
        if (rsize > 0)
        {
            nif->u32RndisDLSize += rsize;
#ifdef CONFIG_NET_TRACE_IP_PACKET
            uint8_t *ipdata = rdata;
            uint16_t identify = (ipdata[4] << 8) + ipdata[5];
            OSI_LOGI(0x10007adc, "prvNdevProcessPsData identify %04x", identify);
#endif
            OSI_LOGE(0x1000756f, "PsIntfRead %d", rsize);
            sys_arch_dump(ipdata, rsize);

            if (IP_HDR_GET_VERSION(rdata) == 6)
            {
                OSI_LOGI(0x10007abd, "net device recieve a ipv6 package, rsize = %d", rsize);
                if (ND6_isRAPackage(tx_req->payload))
                {
                    ND6_RA_reply(rdata, rsize);
                }
            }
            OSI_LOGI(0x1000a107, "nc %p nc->ether %p", nc, nc->ether);

            if (nc->mutex != NULL)
            {
                osiMutexLock(nc->mutex);
            }
            if (nc->netdevIsSuspend == false)
            {
                if (!drvEtherTxReqSubmit(nc->ether, tx_req, rsize))
                {
                    OSI_LOGW(0x10007570, "PS to NC error %d", rsize);
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
        }
    } while (rsize > 0);
}

static void prvPsDataToNdev(void *ctx, drvPsIntf_t *p)
{
    osiThreadCallback(netGetTaskID(), prvNdevProcessPsData, (void *)ctx);
}

static bool prvNdevDataToPs(netSession_t *session, const void *data, size_t size)
{
    struct netif *inp_netif = session->dev_netif;
    if (inp_netif == NULL)
        return false;

    if (size <= 0)
        return false;

    int written = 0x00;

    extern bool NetifGetDPSDFlag(CFW_SIM_ID nSim);
#define GET_SIM(sim_cid) (((sim_cid) >> 4) & 0xf)

    if (!NetifGetDPSDFlag(GET_SIM(inp_netif->sim_cid)))
        written = drvPsIntfWrite(inp_netif->pspathIntf, data, size, 0, 0);
    else
        written = size;
    if (written < 0)
    {
        OSI_LOGI(0x10007571, "written: %d will osiPanic", written);
        //osiThreadSleep(2);
        //osiPanic();
        return false;
    }
    else
    {
        if (written == size)
        {
            OSI_LOGE(0x10007572, "PsIntfWrite %d", size);
            inp_netif->u32RndisULSize += written;
        }
        else
            OSI_LOGW(0x10007573, "PsIntfWrite error %d %d", size, written);
    }
    return true;
}
#endif

static void prvProcessNdevConnect(void *par)
{
    netdevIntf_t *nc = (netdevIntf_t *)par;
    OSI_LOGI(0x10007574, "netdevConnect (ether %p, session %p)", nc->ether, nc->session);
    if (nc->ether == NULL)
        return;

    nc->netdevIsSuspend = false;
    if (nc->session == NULL)
        nc->session = prvNdevSessionCreate();

    if (nc->session == NULL)
        return;

    drvEtherSetULDataCB(nc->ether, prvEthUploadDataCB, nc);
    if (!drvEtherNetUp(nc->ether))
    {
        OSI_LOGI(0x10007575, "NC connect ether open fail");
        prvNdevSessionDelete(nc->session);
        nc->session = NULL;
    }
}

void netdevConnect(void)
{
#if IP_NAT
    //no wan
    if (netif_default == NULL && get_nat_enabled(0, 0x0f) != 0)
    {
        OSI_LOGI(0x10007add, "netdevConnect");
        netdevLanConnect();
        return;
    }

    if (netif_default != NULL && get_nat_enabled((netif_default->sim_cid >> 4), (netif_default->sim_cid & 0x0f)))
    {
        OSI_LOGI(0x10007adf, "have wan netdevConnect");
        netdevLanConnect();
    }
    else
    {
#endif
        OSI_LOGI(0x10007576, "netdevConnect timer start.");
        netdevIntf_t *nc = &gNetIntf;
        if (nc->connect_timer != NULL)
            osiTimerDelete(nc->connect_timer);
        nc->connect_timer = osiTimerCreate(netGetTaskID(), prvProcessNdevConnect, nc);
#ifdef CONFIG_FIBOCOM_BASE
        //MTC0647-77 Reduce network card startup time, 500ms is suitable for early 8910 products, not required for 8850.
        //If 8910 needs to be modified, it needs to be confirmed with the platform in advance
        osiTimerStart(nc->connect_timer, 5);
#else
        osiTimerStart(nc->connect_timer, 500);
#endif
#if IP_NAT
    }
#endif
}

void netdevDisconnect(void)
{
#if IP_NAT
    netdevIntf_t *nc = &gNetIntf;
    struct netif *inp_netif = NULL;
    if ((nc->session != NULL) && (nc->session->is_created == true))
    {
        OSI_LOGI(0x10007ae0, "netdevDisconnect 1111 %p %d", nc->session, nc->session->is_created);
        inp_netif = nc->session->dev_netif;
    }
    OSI_LOGI(0x10007ae1, "netdevDisconnect 2222");
    if (inp_netif != NULL && get_nat_enabled((inp_netif->sim_cid >> 4), (inp_netif->sim_cid & 0x0f)))
    {
        OSI_LOGI(0x10007ae2, "netdevDisconnect 3333");
        netdevLanDisconnect();
    }
    else
    {
#endif
        netdevIntf_t *nc = &gNetIntf;
        drvEtherNetDown(nc->ether);
        prvNdevSessionDelete(nc->session);
        nc->session = NULL;
        if (nc->connect_timer != NULL)
        {
            osiTimerDelete(nc->connect_timer);
            nc->connect_timer = NULL;
        }
#if IP_NAT
    }
#endif
}

void netdevNetUp(void)
{
    OSI_LOGI(0x10007577, "netdevNetUp");
    netdevIntf_t *nc = &gNetIntf;
    if (nc->ether == NULL)
        return;

    if (nc->session != NULL)
    {
        OSI_LOGI(0x10007ae3, "netdevNetUp Net session create already created");
        return;
    }

    nc->session = prvNdevSessionCreate();

    if (nc->session == NULL)
        return;

    drvEtherSetULDataCB(nc->ether, prvEthUploadDataCB, nc);
    if (!drvEtherNetUp(nc->ether))
    {
        OSI_LOGI(0x10007578, "NC net up ether open fail.");
        prvNdevSessionDelete(nc->session);
        nc->session = NULL;
        return;
    }
}

static void prvNdevRestart(void *ctx)
{
    if (netif_default != NULL)
    {
        netdevNetUp();
    }
}

void netdevNetDown(uint8_t nSimID, uint8_t nCID)
{

    netdevIntf_t *nc = &gNetIntf;
    OSI_LOGI(0x10007579, "pre netdevNetDown");
    if ((nc->netdevIsConnected == true) && (nc->session != NULL) && (nc->session->is_created == true))
    {
        struct netif *inp_netif = nc->session->dev_netif;
        if ((inp_netif != NULL) && ((nSimID << 4 | nCID) == inp_netif->sim_cid))
        {
            OSI_LOGI(0x1000757a, "netdevNetDown");
            drvEtherNetDown(nc->ether);
            prvNdevSessionDelete(nc->session);
            nc->session = NULL;
        }
    }

    if (netif_default != NULL) //muilty cid actived; restart Netdev on other cid
    {
        osiThreadCallback(netGetTaskID(), prvNdevRestart, NULL);
    }
}

bool netdevIsConnected(void)
{
    return gNetIntf.netdevIsConnected;
}

void netdevExit(void)
{
    OSI_LOGI(0x1000757b, "NC exit");
    uint32_t critical = osiEnterCritical();
    netdevIntf_t *nc = &gNetIntf;
    drvEtherSetEventCB(nc->ether, NULL, NULL);
    if (gNetIntf.mutex != NULL)
    {
        osiMutexDelete(gNetIntf.mutex);
        gNetIntf.mutex = NULL;
    }
    nc->ether = NULL;
    nc->netdevIsConnected = false;
    osiExitCritical(critical);
}

void netdevInit(drvEther_t *ether)
{
    OSI_LOGI(0x1000757c, "netdevInit");
    uint32_t critical = osiEnterCritical();
    gNetIntf.ether = ether;
    OSI_LOGE(0x1000a999, "netdevInit, gNetIntf.mutex %x", gNetIntf.mutex);
    if (gNetIntf.mutex == NULL)
    {
        gNetIntf.mutex = osiMutexCreate();
        if (gNetIntf.mutex == NULL)
        {
            OSI_LOGI(0x1000a99a, "netdevInit, mutex create failed");
        }
    }
    drvEtherSetEventCB(ether, prvEthEventCB, &gNetIntf);
    osiExitCritical(critical);
}

netSession_t *prvNdevSessionCreate()
{
#ifdef CONFIG_FIBOCOM_AT_RNDIS_SUPPORT
    OSI_PRINTFI("fibo_GetRndisCid:%d",fibo_GetRndisCid());
    if(fibo_GetRndisCid() == -1)
    {
        OSI_LOGI(0, "not ecm cid,do not ecm dial");
        return NULL;
    }
#endif

    static netSession_t s_net_session;
    OSI_LOGI(0x1000757d, "Net session create");
    uint32_t critical = osiEnterCritical();
#ifndef CONFIG_NET_LTE_PSPACKET_SUPPORT
    if (netif_default == NULL || netif_default->pspathIntf == NULL || netif_default->link_mode != NETIF_LINK_MODE_LWIP)
#else
    if (netif_default == NULL || netif_default->link_mode != NETIF_LINK_MODE_LWIP)
#endif
    {
        osiExitCritical(critical);
        OSI_LOGE(0x1000757e, "Net session create no netif_default %p", netif_default);
        return NULL;
    }

    netSession_t *session = &s_net_session;
    if (session->is_created == true)
    {
        osiExitCritical(critical);
        OSI_LOGI(0x1000757f, "Net session create already created");
        return session;
    }
    memset(session, 0, sizeof(netSession_t));
#ifdef CONFIG_FIBOCOM_AT_RNDIS_SUPPORT
        session->dev_netif = netif_get_by_cid(fibo_GetRndisCid());
        OSI_PRINTFI("[fibocom] net session creat, dev_netif=%p, netif_default=%p", session->dev_netif, netif_default);
        //solve for cgact=1 crash,20201013
        if(session->dev_netif == NULL)
        {
            session->dev_netif = netif_default;
        }
#else
    session->dev_netif = netif_default;
#endif
#ifndef CONFIG_NET_LTE_PSPACKET_SUPPORT

    session->old_psintf_cb = drvPsIntfSetDataArriveCB(session->dev_netif->pspathIntf, prvPsDataToNdev);
#endif
    OSI_LOGI(0x10007580, "Net end session %x\n", session);
    session->dev_netif->link_mode = NETIF_LINK_MODE_NETCARD;
    session->is_created = true;
    osiExitCritical(critical);
    netif_default = NULL;
#ifdef CONFIG_NET_LTE_PSPACKET_SUPPORT
    session->dl_read_notify = osiNotifyCreate(netGetTaskID(), _gprs_ps_data_to_netcard, NULL);
    session->ul_cache_timer = osiTimerCreate(netGetTaskID(), prvULCacheTimeout, session);

    TAILQ_INIT(&(session->buf_list));
    session->buf_list_count = 0;
#endif
#if !LWIP_SINGLE_NETIF
    if (netif_list != NULL)
    {
        struct netif *next_netif = netif_list;
        while (next_netif != NULL)
        {
            OSI_LOGI(0x10007ae4, "reset default_netif next_netif id %d", next_netif->num);
            if ((next_netif->name[0] != 'l') && (next_netif->name[1] != 'o') && (next_netif->is_type != NETIF_LINK_TYPE_IMS))
            {
#if IP_NAT
                if ((next_netif->link_mode == NETIF_LINK_MODE_LWIP) || (next_netif->link_mode == NETIF_LINK_MODE_NAT_LWIP_LAN))
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
    return session;
}

bool prvNdevSessionDelete(netSession_t *session)
{
    uint32_t critical = osiEnterCritical();
    if (session == NULL)
    {
        osiExitCritical(critical);
        return false;
    }

    netdevIntf_t *nc = &gNetIntf;
    if (nc->mutex != NULL)
    {
        osiMutexLock(nc->mutex);
    }
    if (session->dev_netif != NULL)
    {
#ifndef CONFIG_NET_LTE_PSPACKET_SUPPORT
        if (session->dev_netif->pspathIntf != NULL)
            drvPsIntfSetDataArriveCB(session->dev_netif->pspathIntf, lwip_pspathDataInput);
#endif
        session->dev_netif->link_mode = NETIF_LINK_MODE_LWIP;
    }
    session->is_created = false;
#ifdef CONFIG_NET_LTE_PSPACKET_SUPPORT
    ps_buf_t *ps_buf = NULL;
    while ((ps_buf = TAILQ_FIRST(&(session->buf_list))) != NULL)
    {
        struct ps_header *psData = ps_buf->psData;
        TAILQ_REMOVE(&(session->buf_list), ps_buf, iter);
        if (NULL != psData)
            free(psData);
        free(ps_buf);
    }
    session->buf_list_count = 0;
    osiNotifyDelete(session->dl_read_notify);
    session->dl_read_notify = NULL;
    osiTimerDelete(session->ul_cache_timer);
    session->ul_cache_timer = NULL;
    psHeaderFreeAll((struct ps_header *)session->ul_buf_head);
    session->ul_buf_head = NULL;
    session->ul_cache_num = 0;
#endif

    if (nc->mutex != NULL)
    {
        osiMutexUnlock(nc->mutex);
    }
    osiExitCritical(critical);
#if !LWIP_SINGLE_NETIF
    if (netif_default == NULL && netif_list != NULL)
    {
        struct netif *next_netif = netif_list;
        netif_default = NULL;
        while (next_netif != NULL)
        {
            OSI_LOGI(0x10007ae4, "reset default_netif next_netif id %d", next_netif->num);
            if ((next_netif->name[0] != 'l') && (next_netif->name[1] != 'o') && (next_netif->is_type != NETIF_LINK_TYPE_IMS))
            {
#if IP_NAT
                if ((next_netif->link_mode == NETIF_LINK_MODE_LWIP) || (next_netif->link_mode == NETIF_LINK_MODE_NAT_LWIP_LAN))
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
    return true;
}
