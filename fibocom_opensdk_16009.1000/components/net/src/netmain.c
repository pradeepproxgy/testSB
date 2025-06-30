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
#include "osi_api.h"
#include "osi_log.h"
#include "cfw_dispatch.h"
#include "cfw.h"
#include "cfw_event.h"
#include "tcpip.h"
#include "lwipopts.h"
#include "netmain.h"
#include "ppp_interface.h"
#include "netdev_interface.h"
#include "netdev_interface_nat_lan.h"
#include "netdev_interface_imp.h"
#include "sockets.h"
#if IP_NAT
#include "lwip/ip4_nat.h"
#endif
#include "nonip.h"

#ifdef CONFIG_NET_IDS_SUPPORT
#include "ids.h"
#endif
#include "osi_trace.h"
#include "srv_trace.h"
#ifdef CONFIG_CTHTTP_API_SUPPORT
#include "cthttp_api.h"
#endif
#include "drv_config.h"
#ifdef CONFIG_PAM_LTE_GNSS_WIFISCAN_SUPPORT
#include "pam_delay.h"
#endif
#if defined(CONFIG_FIBOCOM_BASE)
#include "ffw_broker.h"
#include "ffw_event.h"
#include "ffw_timer.h"
#endif

osiThread_t *netThreadID = NULL;

extern void tcpip_thread(void *arg);
#if defined(CONFIG_FIBOCOM_BASE)
extern void ffw_at_thread_callback(void (*cb)(void *arg), void *arg, bool sync);
extern void ffw_rat_netif_ip_cb(void *arg);
typedef unsigned int UINT32;
typedef int INT32;
char auto_ecm_switch[3] = {0};
UINT32 g_rndis_timer_start = 0;
static ffw_timerp_t ecm_timer = NULL;
extern bool is_rndis_call_on;
extern int8_t rndis_cid;
extern void autoActRndis();
extern int fibo_GetRndisCid();
extern int32_t fibo_get_gtset_ecmauto_flag();
static uint8_t autoecm_Simid = 0;
extern bool fibo_get_flag_autodial();
static void fibo_set_gtset_ecmauto_simid(uint8_t simid)
{
    OSI_LOGI(0, "fibo_set_gtset_ecmauto_simid is %d", simid);
    autoecm_Simid = simid;
}
uint8_t fibo_get_gtset_ecmauto_simid()
{
    return autoecm_Simid;
}

static void restart_auto_rndis(void *arg)
{
    int fd = vfs_open(AT_CFGFN_AUTOCONNECT, O_RDONLY);
    if(fd < 0)
    {
        /*if can not open it,autodial is turned on by default*/
        OSI_PRINTFE("atCmdHandleGTAUTOCONNECT file no exist");
    }
    else
    {
        vfs_read(fd, auto_ecm_switch, sizeof(auto_ecm_switch));
        vfs_close(fd);
        OSI_PRINTFI("[%s:%d] autoecmswitch is: %c", __FUNCTION__, __LINE__, auto_ecm_switch[0]);
        if(auto_ecm_switch[0] == '0')
        {
            OSI_PRINTFI("[%s:%d] auto act Rndis is not permit!", __FUNCTION__, __LINE__);
            goto FINISH;
        }
    }
    //if CFUN=0, not allowed do act or deact
    uint8_t nSimId = fibo_get_gtset_ecmauto_simid();
    CFW_COMM_MODE nFM = CFW_DISABLE_COMM;
    if (0 != CFW_GetComm(&nFM, nSimId))
    {
        OSI_PRINTFI("[%s:%d] CFW_GetComm fail", __FUNCTION__, __LINE__);
        goto FINISH;
    }
    if (nFM == CFW_DISABLE_COMM)
    {
        OSI_PRINTFI("[%s:%d] in flight mode,not allowed do act or deact", __FUNCTION__, __LINE__);
        goto FINISH;
    }

    // get gprs att state
    uint8_t att_state = 0;
    uint32_t ret = CFW_GetGprsAttState(&att_state, nSimId);
    if ((0 != ret) || (CFW_GPRS_ATTACHED != att_state))
    {
        OSI_PRINTFI("[%s:%d] GetGprsAtt fail", __FUNCTION__, __LINE__);
        return;
    }

    int nCid = 1;
    UINT32 nRndis_timer_run = 0;
    OSI_LOGI(0, "auto rndis start......");
    autoActRndis();
    if(nCid == fibo_GetRndisCid())
    {
        OSI_LOGI(0, "auto rndis success......");
        goto FINISH;
    }
    nRndis_timer_run = osiEpochSecond();
    if(nRndis_timer_run - g_rndis_timer_start > 300)
    {
        OSI_LOGI(0, "auto rndis timeout......");
        goto FINISH;
    }
    return;
FINISH:
    if(NULL != ecm_timer)
    {
        ffw_timer_stop(ecm_timer);
    }
    return;
}
#endif

#if LWIP_RFACK
extern void NetifRfAck(uint8_t nCid, uint8_t nSimId, uint32_t frAckSeqno, uint16_t ackOk);
#endif

#ifdef CONFIG_NET_SELF_REGISTE_SUPPORT
extern int vnet4gSelfRegister(uint8_t nCid, uint8_t nSimId);
#endif
#ifdef CONFIG_NET_SELFREG_COAP_SUPPORT
extern int vnetNBSelfRegister(uint8_t nCid, uint8_t nSimId);
#endif
#if defined(CONFIG_NET_DM_CISSDK_SUPPORT) || defined(CONFIG_NET_DM_LWM2M_SUPPORT) || defined(CONFIG_NET_DM_CUS_SUPPORT)
extern void dm_register(uint8_t nSimId);
#endif
extern void dm_lwm2m_register();
#ifdef CONFIG_NET_LIBDMP_SDK_SUPPORT
extern void uni_mqtt_selfRegister(uint8_t nSimId, uint8_t nCid);
extern void unidmp_netif_cleancheck(uint8_t nSim, uint8_t nCid);
#endif
#ifdef CONFIG_NET_CTWING_SELFREG_SUPPORT
extern void dm_aep_register();
#endif
#ifdef CONFIG_NET_CTWING_MQTT_SUPPORT
extern void ctwing_mqtt_init();
extern void ctwing_mqtt_autoRegister();
#endif
#ifdef CONFIG_AT_TCPCTWING_SUPPORT
extern void ctwing_tcp_autoRegister();
#endif
#ifdef CONFIG_NET_CTIOT_AEP_SUPPORT
extern void ctiot_engine_entry(uint8_t autoLoginFlag);
#endif
#if defined(CONFIG_SOC_8910) || defined(CONFIG_SOC_8850)
#define NET_STACK_SIZE (8192 * 4)
#else
#define NET_STACK_SIZE 8192
#endif
#if IP_NAT
extern struct netif *TCPIP_nat_lan_lwip_netif_create(uint8_t nCid, uint8_t nSimId);
extern struct lan_netif *newLan(uint8_t nSimId, uint8_t nCid, uint8_t nIdx);
extern struct wan_netif *newWan(uint8_t nSimId, uint8_t nCid);
extern void destroyWan(uint8_t nSimId, uint8_t nCid);
extern struct netif *getGprsNetIf(uint8_t nSim, uint8_t nCid);
#endif
extern uint8_t pamGetAgnssActPdp(void);

#if IP_NAT
extern netdevIntf_t gNetIntf;
void default_netif_switchcheck(int nMod, uint8_t nSimId, uint8_t nCid)
{
    struct netif *netif = NULL;
    uint8_t cid = 0;
    //uint8_t sim = 0;

    //if nat mix mode
    if (get_nat_enabled(0, 0x0f) != 0 && get_nat_cfg() != 0xff)
    {
        if (nMod == EV_TCPIP_CFW_GPRS_ACT)
        {
            //get default if has LAN
            netif = getGprsNetIf(nSimId, nCid);
            if (netif != netif_default)
            {
                cid = (netif_default->sim_cid & 0x0f);
                OSI_LOGI(0x1000a108, "default_netif_switchcheck 1 cid = %d", cid);
                if (cid == NAT_CID)
                {
                    //if lan lwip
                    struct lan_netif *plwip_lan = getLan(NAT_SIM, NAT_CID, NAT_LAN_IDX_LWIP);
                    if (plwip_lan != NULL && plwip_lan->netif != NULL)
                    {

                        struct lan_netif *plan = (struct lan_netif *)(netif_default->pstlan);

                        if (plan != NULL)
                        {
                            plan->isDefault = 0;
#ifdef CONFIG_USB_ETHER_SUPPORT
                            netdevIntf_t *nc = &gNetIntf;
                            drvEtherNetDown(nc->ether);
#endif
                            netif_set_default(netif); //change to current activated pdp
                            netif_reset_dns(netif);
#ifdef CONFIG_USB_ETHER_SUPPORT
                            drvEtherNetUp(nc->ether);
#endif
                        }
                    }
                }
            }
        }
        else if (nMod == EV_TCPIP_CFW_GPRS_DEACT)
        {
#ifdef CONFIG_USB_ETHER_SUPPORT
            //if has rndis lan
            if (netdevIsConnected() == true && netif_default == NULL)
            {
                struct lan_netif *plwip_lan = getLan(NAT_SIM, NAT_CID, NAT_LAN_IDX_LWIP);
                if (plwip_lan != NULL && plwip_lan->netif != NULL)
                {
                    netif_set_default(plwip_lan->netif);
                    netif_reset_dns(plwip_lan->netif);
                }
            }
#endif
        }
    }
    return;
}
#endif
void TCPIP_netif_create_SS(uint8_t nCid, uint8_t nSimId)
{
#if IP_NAT
    if (get_nat_enabled(nSimId, nCid))
    {
        newWan(nSimId, nCid);
    }
    else
    {
#endif
        TCPIP_netif_create(nCid, nSimId);
#if IP_NAT
    }
#endif
    return;
}
void TCPIP_netif_destory_SS(uint8_t nCid, uint8_t nSimId)
{
#if IP_NAT
    if (get_nat_enabled(nSimId, nCid))
    {
        destroyWan(nSimId, nCid);
    }
    else
    {
#endif
        TCPIP_netif_destory(nCid, nSimId);
#if IP_NAT
    }
#endif
    return;
}
extern osiThread_t *atEngineGetThreadId(void);
static void net_thread(void *arg)
{
    netThreadID = osiThreadCurrent();

    for (;;)
    {
        osiEvent_t event = {};
        osiEventWait(osiThreadCurrent(), &event);
        if (event.id == 0)
            continue;
        OSI_LOGI(0x1000752b, "Netthread get a event: 0x%08x/0x%08x/0x%08x/0x%08x", (unsigned int)event.id, (unsigned int)event.param1, (unsigned int)event.param2, (unsigned int)event.param3);

        OSI_LOGI(0x1000752c, "Netthread switch");
        if ((!cfwIsCfwIndicate(event.id)) && (cfwInvokeUtiCallback(&event)))
        {
            ; // handled by UTI
        }
        else
        {
            CFW_EVENT *cfw_event = (CFW_EVENT *)&event;
            switch (event.id)
            {
            case EV_INTER_APS_TCPIP_REQ:
            {
                struct tcpip_msg *msg;
                msg = (struct tcpip_msg *)event.param1;
                tcpip_thread(msg);
                break;
            }
            case EV_TCPIP_CFW_GPRS_ACT:
            {
                uint8_t nCid, nSimId;
                nCid = event.param1;
                nSimId = cfw_event->nFlag;

#ifdef CONFIG_NET_NONIP_SUPPORT
                CFW_GPRS_PDPCONT_INFO_V2 pdp_context;
                CFW_GprsGetPdpCxtV2(nCid, &pdp_context, nSimId);
                OSI_LOGI(0x10007ad9, "EV_TCPIP_CFW_GPRS_ACT pdp_context.nPdpType %d pdp_context.pdnType %d\n", pdp_context.nPdpType, pdp_context.PdnType);
                extern void atNonIPDataArriveCB(void *ctx, NonIPSession_t *nonIPSession);
                if (pdp_context.PdnType == CFW_GPRS_PDP_TYPE_NONIP)
                {
                    NonIPSessionCreate(nSimId, nCid, NULL, NULL);
                    osiEvent_t nonip_act_event = {.id = EV_CFW_GPRS_NONIP_ACT_IND, .param1 = nCid << 8 | nSimId};
                    osiEventSend(atEngineGetThreadId(), &nonip_act_event);
                    break;
                }
#endif

#if IP_NAT
                if (get_nat_enabled(nSimId, nCid))
                {
                    newWan(nSimId, nCid);
#ifdef CONFIG_USB_ETHER_SUPPORT
                    if (netdevIsConnected())
                    {
                        netdevLanNetUp(nSimId, nCid);
                    }
#endif
                }
                else
                {
#endif
                    TCPIP_netif_create(nCid, nSimId);
#ifdef CONFIG_USB_ETHER_SUPPORT
#if defined(CONFIG_SOC_8910) || defined(CONFIG_SOC_8850)
#ifdef CONFIG_PAM_LTE_GNSS_WIFISCAN_SUPPORT
                    if (netdevIsConnected() && (nCid != pamGetAgnssActPdp())
#else
                    if (netdevIsConnected())
#endif
                    {
                        netdevNetUp();
                    }
#endif
#endif
#if IP_NAT
                }
                default_netif_switchcheck(EV_TCPIP_CFW_GPRS_ACT, nSimId, nCid);
#endif

#if defined(CONFIG_FIBOCOM_BASE)
#ifndef CONFIG_NET_NONIP_SUPPORT
                CFW_GPRS_PDPCONT_INFO_V2 pdp_context;
                CFW_GprsGetPdpCxtV2(nCid, &pdp_context, nSimId);
#endif
                ffw_cp_evt_pdp_t ev = {0/*FFW_R_SUCCESS*/, 0, 1};
                ev.cid = nCid;
                if (pdp_context.PdnType == CFW_GPRS_PDP_TYPE_IP)
                {
                    OSI_LOGI(0, "[fibocom] -- net -- get ipv4");
                    ffw_broker_send_msg_at_task(FFW_EVT_CP_PDP_ACTIVED, &ev, sizeof(ev));
                }
#endif

#ifdef CONFIG_NET_SELF_REGISTE_SUPPORT
                vnet4gSelfRegister(nCid, nSimId); //dianxin4G×Ô×¢²á
#endif
#ifdef CONFIG_NET_SELFREG_COAP_SUPPORT
                vnetNBSelfRegister(nCid, nSimId); //dianxin4G×Ô×¢²á
#endif
#if defined(CONFIG_NET_DM_CISSDK_SUPPORT) || defined(CONFIG_NET_DM_LWM2M_SUPPORT) || defined(CONFIG_NET_DM_CUS_SUPPORT)
                dm_register(nSimId);
#endif
                dm_lwm2m_register();
#ifdef CONFIG_NET_CTWING_SELFREG_SUPPORT
                dm_aep_register();
#endif
#ifdef CONFIG_NET_LIBDMP_SDK_SUPPORT
                uni_mqtt_selfRegister(nSimId, nCid);
#endif
#ifdef CONFIG_AT_TCPCTWING_SUPPORT
                ctwing_tcp_autoRegister();
#endif
#ifdef CONFIG_NET_CTWING_MQTT_SUPPORT
                ctwing_mqtt_autoRegister();
#endif
#ifdef CONFIG_CTHTTP_API_SUPPORT
                ctwing_http_autoRegister();
#endif

#ifdef CONFIG_NET_CTIOT_AEP_SUPPORT
                ctiot_engine_entry(false);
#endif
                osiEvent_t tcpip_event = *(osiEvent_t *)cfw_event;
                tcpip_event.id = EV_TCPIP_CFW_GPRS_ACTED;
                osiEventSend(atEngineGetThreadId(), (const osiEvent_t *)&tcpip_event);
                break;
            }
            case EV_TCPIP_CFW_GPRS_DEACT:
            {
                uint8_t nCid, nSimId;
                nCid = event.param1;
                nSimId = cfw_event->nFlag;
#ifdef CONFIG_NET_NONIP_SUPPORT
                NonIPSession_t *nonIPSession = NULL;
                nonIPSession = getNonIPSession(nSimId, nCid);
                CFW_GPRS_PDPCONT_INFO_V2 pdp_context;
                CFW_GprsGetPdpCxtV2(nCid, &pdp_context, nSimId);
                if (pdp_context.PdnType == CFW_GPRS_PDP_TYPE_NONIP && nonIPSession != NULL)
                {
                    NonIPSessionDestory(nSimId, nCid);
                    break;
                }
#endif
#if IP_NAT
                if (get_nat_enabled(nSimId, nCid))
                {
                    destroyWan(nSimId, nCid);
                }
                else
                {
#endif
#ifdef CONFIG_USB_ETHER_SUPPORT
#if defined(CONFIG_SOC_8910) || defined(CONFIG_SOC_8850)
                    if (netdevIsConnected())
                    {
                        netdevNetDown(nSimId, nCid);
                    }
#endif
#endif

#ifdef CONFIG_NET_LIBDMP_SDK_SUPPORT
                    unidmp_netif_cleancheck(nSimId, nCid);
#endif
                    TCPIP_netif_destory(nCid, nSimId);

#if IP_NAT
                }
                default_netif_switchcheck(EV_TCPIP_CFW_GPRS_DEACT, nSimId, nCid);
#endif
                osiEvent_t tcpip_event = *(osiEvent_t *)cfw_event;
                tcpip_event.id = EV_TCPIP_CFW_GPRS_DEACTED;
                osiEventSend(atEngineGetThreadId(), (const osiEvent_t *)&tcpip_event);

#if defined(CONFIG_FIBOCOM_BASE)
                OSI_PRINTFE("EV_TCPIP_CFW_GPRS_DEACT fibo_get_flag_autodial = %d", fibo_get_flag_autodial());
                if(fibo_get_gtset_ecmauto_flag() && fibo_get_flag_autodial())
                {
                    OSI_LOGI(0, "fibo_get_gtset_ecmauto_flag is %d, begin re-autoecm", fibo_get_gtset_ecmauto_flag());
                    fibo_set_gtset_ecmauto_simid(nSimId);
                    g_rndis_timer_start = osiEpochSecond();
                    if(NULL == ecm_timer)
                    {
                        OSI_LOGI(0, "start auto rndis timer......");
                        ffw_timer_create(restart_auto_rndis, NULL, &ecm_timer);
                    }
                    ffw_timer_start(ecm_timer, 5 * 1000, 1);
                }
#endif
                break;
            }
            case EV_TCPIP_CFW_GPRS_AUTO_DEACT:
            {
                uint8_t nCid, nSimId;
                nCid = event.param1;
                nSimId = cfw_event->nFlag;
                OSI_PRINTFI("[fibocom] TCPIP_netif_destory EV_TCPIP_CFW_GPRS_AUTO_DEACT nCid : %d, nSimId : %d", nCid, nSimId);

                if (netdevIsConnected())
                {
                    netdevNetDown(nSimId, nCid);
                }
                //TCPIP_netif_destory(nCid, nSimId);
                break;
            }
#if defined(CONFIG_SOC_8910) || defined(CONFIG_SOC_8850)
#ifdef CONFIG_USB_ETHER_SUPPORT
            case EV_TCPIP_ETHCARD_CONNECT:
            {
                netdevConnect();
                break;
            }
            case EV_TCPIP_ETHCARD_DISCONNECT:
            {
                OSI_LOGI(0x10007ada, "EV_TCPIP_ETHCARD_DISCONNECT");
                netdevDisconnect();
                break;
            }
#endif
#endif
#if LWIP_RFACK
            case EV_TCPIP_CFW_GPRS_RFACK:
            {
                uint8_t nCid, nSimId;
                uint32_t app_sn = cfw_event->nParam1;
                uint16_t send_Ok = (cfw_event->nParam2 & 0xFFFF);
                nCid = (cfw_event->nParam2 >> 16) & 0xf;
                nSimId = cfw_event->nFlag;
                NetifRfAck(nCid, nSimId, app_sn, send_Ok);
                break;
            }
#endif //LWIP_RFACK
#ifdef CONFIG_FS_UNET_LOG_ENABLE
            case EV_TCPIP_UNETLOG_IND:
            {
                OSI_LOGI(0x1000a99b, "EV_TCPIP_UNETLOG_IND");
                void *param = (void *)cfw_event->nParam1;
                srvFstrace_UnetLog_SentCPExceptionLogFile(param);
                break;
            }
#endif //LWIP_RFACK
#if defined(CONFIG_NET_CTWING_MQTT_SUPPORT)
            case EV_CFW_GPRS_STATUS_IND:
            {

#ifdef CONFIG_NET_CTWING_MQTT_SUPPORT
                ctwing_mqtt_init();
#endif
                break;
            }
#endif

            default:
                break;
            }
        }
    }
}

static void prvLwipPsmSave(void *ctx, osiShutdownMode_t mode)
{
#if defined(CONFIG_NET_LTE_SUPPORT) || defined(LTE_SUPPORT)
    extern void NetifPm3Presave(void);
    if (mode == OSI_SHUTDOWN_PSM_SLEEP)
        NetifPm3Presave();
#endif
}

osiThread_t *netGetTaskID(void)
{
    return netThreadID;
}

void net_init()
{

#ifdef CONFIG_8850CM_TARGET
    netThreadID = osiThreadCreate("net", net_thread, NULL, OSI_PRIORITY_NORMAL_PLUS, NET_STACK_SIZE, 64);
#else
    netThreadID = osiThreadCreate("net", net_thread, NULL, OSI_PRIORITY_NORMAL, NET_STACK_SIZE, 64);
#endif
#ifdef CONFIG_KERNEL_MONITOR_ENABLE
    uintptr_t netStackBase = osiThreadStackBase(netThreadID);
    osiMemMonitorStart(netStackBase, netStackBase + 128);
#endif
    tcpip_init(NULL, NULL);

#if IP_NAT
    ip4_nat_init();
#endif
#ifdef CONFIG_USB_ETHER_SUPPORT
#if defined(CONFIG_SOC_8910) || defined(CONFIG_SOC_8850)
    netdevCheckEthConEvt();
#endif
#endif
#ifdef CONFIG_NET_IDS_SUPPORT
    IDS_init();
#endif
    osiRegisterShutdownCallback(prvLwipPsmSave, NULL);
}
