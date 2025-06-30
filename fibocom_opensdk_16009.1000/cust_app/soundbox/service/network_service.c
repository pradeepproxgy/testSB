#include <stdio.h>
#include <string.h>
#include "network_service.h"
#include "fibo_opencpu.h"
#include "voice_play.h"
#include "led.h"

#define netWork_log OSI_PRINTFI

#define NET_SEARCH_CNT 30 // 模块查询次数 失败10次重新查询
#define NET_MAX_RSSI 31   // 信号值判断
#define NET_MIN_RSSI 0

#define NET_SEARCH_CNT 30 // 模块查询次数 失败10次重新查询
#define NET_MAX_RSSI 31   // 信号值判断
#define NET_MIN_RSSI 0

//extern UINT32 network_fail_indication;

/*unsigned long lwip_htonl(unsigned long n)
{
    return ((n & 0xff) << 24) | ((n & 0xff00) << 8) | ((n & 0xff0000) >> 8) | ((n & 0xff000000) >> 24);
}

unsigned short lwip_htons(unsigned short n)
{
    return ((n & 0xff) << 8) | ((n & 0xff00) >> 8);
}*/

typedef enum
{
    NET_STATUS_INIT = 1,
    NET_CHECK_SIM,
    NET_SET_APN,
    NET_SIM_READY,
    NET_STATUS_CSQ,
    NET_STATUS_REGISTER,
    NET_PDP_ACTIVE,
    NET_GET_ACTIVE_STATUS,
    NET_PDP_IS_ACTIVE,

} network_status;

typedef struct
{
    INT8 *mccmnc;
    INT8 *apn;
} plmn_t;

const plmn_t plmn_table[] =
    {
        {"50501", "testra.internet"},
        {"50502", "Internet"},
        {"50503", "live.vodafone.com"},
        {"46000", "cmnet"},
        {"46002", "cmnet"},
        {"46004", "cmnet"},
        {"46007", "cmnet"},
        {"46008", "cmnet"},
        {"46001", "3gnet"},
        {"46003", "3gnet"},
        {"46006", "3gnet"},
        {"46009", "3gnet"},
        {"46005", "ctnet"},
        {"46011", "ctnet"},
        {"44010", "biglobe.jp"},
        {"44020", "plus.4g"},
        {"53005", "wap.telecom.co.nz"},
        {"53001", "vodafone"},
        {"52003", "Internet"},
        {"52005", "Internet"},
        {"52000", "Internet"},
        {"25001", "internet.mtc.ru"},
        {"25002", "Internet"},
        {"25099", "internet.beeline.ru"},
        {"24491", "Internet"},
        {"24405", "Internet"},
        {"24412", "Internet"},
        {"24001", "online.telia.se"},
        {"24008", "internet.telenor.se"},
        {"24002", "bredband.tre.se"},
        {"24007", "4g.tele2.se"},
        {"24201", "telenor.mbb"},
        {"24202", "internet.netcom.no"},
        {"23801", "internet"},
        {"23802", "internet"},
        {"23820", "www.internet.mtelia.dk"},
        {"23806", "bredband.tre.dk"},
        {"26003", "Internet"},
        {"26002", "Internet"},
        {"26001", "Internet"},
        {"20416", "smartsites.t-mobile"},
        {"20408", "fastinternet"},
        {"20404", "office.vodafone.nl"},
        {"20601", "internet.proximus.be"},
        {"20620", "gprs.base.be"},
        {"20610", "mworld.be"},
        {"30261", "pda.bell.ca"},
        {"30272", "rogers-core-appl1.apn"},
        {"30222", "isp.telus.com"},
        {"33402", "internet.itelcel.com"},
        {"33403", "internet.movistar.mx"},
        {"33405", "web.iusacellgsm.mx"},
        {"72410", "zap.vivo.com.br"},
        {"72405", "java.claro.com.br"},
        {"72403", "tim.br"},
        {"65507", "internet"},
        {"65501", "lte.vodacom.za"},
        {"65510", "internet"},
        {"62150", "gloflat"},
        {"62120", "internet.ng.zain.com"},
        {"62160", "etisalat"},
        {"42402", "etisalat.ae"},
        {"42403", "du"},
        {"42003", "web2"},
        {"42001", "jawalnet.com.sa"},
        {"42004", "zain"},
        {NULL, NULL}};

int net_init_index = 0;
int net_connect_status = 0;
dev_net_info_t dev_net_info;
static int search_cnt = 0;

// return -1://get imsi fail   -2://get ccid fail   -3://get apn fail
int get_sim_info(UINT8 *imsi, UINT8 *ccid, UINT8 *apn)
{
    UINT8 apn_index = 0;
    int ret = 0;

    ret = fibo_get_imsi(imsi);
    netWork_log("imsi ret=%d", ret);
    if ((ret < 0) && (imsi == NULL))
    {
        return -1; // get imsi fail
    }

    ret = fibo_get_ccid(ccid,0);
    netWork_log("ccid ret=%d", ret);
    if ((ret < 0) && (ccid == NULL))
    {
        return -2; // get ccid fail
    }

    while (plmn_table[apn_index].mccmnc != NULL)
    {
        netWork_log("while true plmn_table apn_index");
        if (strstr((INT8 *)imsi, plmn_table[apn_index].mccmnc))
        {
            netWork_log("if imsi plmn_table");
            memcpy((INT8 *)apn, plmn_table[apn_index].apn, strlen(plmn_table[apn_index].apn));
            return 0;
        }
        apn_index++;
    }
    return -3; // get apn fail
}

int get_net_init_sta(void)
{
    return net_init_index;
}

void set_net_init_sta(int index)
{
    search_cnt = 0;
    net_init_index = index;
}

int get_net_connect_sta(void)
{
    return net_connect_status;
}

void set_net_connect_sta(int net_sta)
{
    net_connect_status = net_sta;
}

static int network_init(void)
{
    int ret = 0;
    int net_index = 0;
    static int frist_poweron_flag = 0;

    while (1)
    {
        search_cnt++;
        net_index = get_net_init_sta();
        netWork_log("network_init[%d]\r\n", net_index);

        switch (net_index)
        {
        case NET_CHECK_SIM:
            net_connect_status = 0;
            fibo_get_sim_status((uint8_t *)&dev_net_info.sim_status);
            netWork_log("network sim_status[%d]\r\n", dev_net_info.sim_status);
            if (dev_net_info.sim_status == 1)
            {
                set_net_init_sta(NET_SIM_READY);
            }
            else
            {
                if (search_cnt > NET_SEARCH_CNT)
                {
                    return NET_NO_SIM;
                }
            }
            break;

        case NET_SIM_READY: /// via msg_event

            if (frist_poweron_flag == 0)
            {
                frist_poweron_flag = 1;
            }
            ret = get_sim_info((UINT8 *)dev_net_info.imsi, (UINT8 *)dev_net_info.ccid, (UINT8 *)dev_net_info.apn);
            if ((ret == 0) || (ret == -3)) // 未查找到apn 采用网络下发apn
            {
                set_net_init_sta(NET_SET_APN);
            }
            else
            {
                if (search_cnt > NET_SEARCH_CNT)
                {
                    if (ret == -1)
                    {
                        return NET_NO_IMSI;
                    }
                    else if (ret == -2)
                    {
                        return NET_NO_CCID;
                    }
                }
            }
            break;

        case NET_SET_APN:
            fibo_asyn_PDPRelease(0, 1, 0);
            if (dev_net_info.apn[0] != 0) // APN为空 则使用网络下发的apn
            {
                ret = fibo_set_initial_pdp(1, 1, (char *)dev_net_info.apn, 0, NULL, NULL, 0);
            }
            set_net_init_sta(NET_STATUS_CSQ);
            break;

        case NET_STATUS_CSQ: // 小于等于31时数值越大信号越好，12以下弱信号（含）
            fibo_get_csq(&dev_net_info.rssi, &dev_net_info.ber,0);
            netWork_log("network csq rssi[%d]\r\n", dev_net_info.rssi);
            if (dev_net_info.rssi > NET_MIN_RSSI)
            {
                set_net_init_sta(NET_STATUS_REGISTER);
            }
            else
            {
                if (search_cnt > NET_SEARCH_CNT)
                {
                    return NET_NO_SIGNAL;
                }
            }
            break;

        case NET_STATUS_REGISTER:                                // cgreg
            fibo_reg_info_get(&dev_net_info.registered_status, 0); // 0/2/3/4 表示未注册，其他值表示注册
            netWork_log("network registered_status [%d]\r\n", dev_net_info.registered_status.nStatus);
            if ((dev_net_info.registered_status.nStatus == 1) || (dev_net_info.registered_status.nStatus == 5))
            {
                set_net_init_sta(NET_PDP_ACTIVE); /// via msg_event
            }
            else
            {
                if (search_cnt > NET_SEARCH_CNT)
                {
                    return NET_NO_REGISTER;
                }
            }
            break;

        case NET_PDP_ACTIVE: /// via msg_event
        {
            pdp_profile_t pdp_profile;

            ret = fibo_PDPStatus(1, (UINT8 *)dev_net_info.ip, (UINT8 *)&dev_net_info.cid_status, 0);
            if (dev_net_info.cid_status == 0)
            {
                ret = fibo_PDPActive(1, NULL, NULL, NULL, 0, 0, NULL);
                memset(&pdp_profile, 0, sizeof(pdp_profile));
                memcpy(pdp_profile.nPdpType, "IP", strlen("IP"));
                pdp_profile.cid = 1;
                if (fibo_asyn_PDPActive(1, &pdp_profile, 0) == 0)
                {
                    memcpy(dev_net_info.ip, pdp_profile.pdpAddr, pdp_profile.pdpAddrSize);
                }
                set_net_init_sta(NET_GET_ACTIVE_STATUS);
            }
            else
            {
                set_net_init_sta(NET_PDP_IS_ACTIVE);
            }
        }
        break;

        case NET_GET_ACTIVE_STATUS:
            ret = fibo_PDPStatus(1, (UINT8 *)dev_net_info.ip, (UINT8 *)&dev_net_info.cid_status, 0);
            netWork_log("fibo_PDPStatus ip[%s] cid_statu[%d]\r\n", dev_net_info.ip, dev_net_info.cid_status);
            if ((ret == 0) && (dev_net_info.ip != NULL) && (dev_net_info.cid_status == 1))
            {
                set_net_init_sta(NET_PDP_IS_ACTIVE);
            }
            else
            {
                if (search_cnt > NET_SEARCH_CNT)
                {
                    return NET_NO_ACTIVE;
                }
            }
            break;

        case NET_PDP_IS_ACTIVE:
            REDLED_OFF();
            return NET_CONNECT_SUCCESS;
            break;
        }
        fibo_taskSleep(100);
    }
}

static int network_loop()
{
    int ret = -1;

    set_net_connect_sta(NET_DISCONNECT);
    memset(&dev_net_info, 0, sizeof(dev_net_info));
    set_net_init_sta(NET_CHECK_SIM);

    ret = network_init();
    netWork_log("network_init return [%d]\n", ret);

    switch (ret)
    {
    case NET_CONNECT_SUCCESS:
        GREENLED_ON();
	push_index(36);
	fibo_taskSleep(1000);
        set_net_connect_sta(NET_CONNECT);
        while (1)
        {
            fibo_get_sim_status((uint8_t *)&dev_net_info.sim_status);
            if (dev_net_info.sim_status == 0)
            {
                return -1;
            }

            fibo_get_csq((INT32 *)&dev_net_info.rssi, (INT32 *)&dev_net_info.ber,0);
            if (dev_net_info.rssi < 0)
            {
                netWork_log("network timer rssi[%d]\r\n", dev_net_info.rssi);
                return -1;
            }

            fibo_PDPStatus(1, (UINT8 *)dev_net_info.ip, (UINT8 *)&dev_net_info.cid_status, 0);
            if (dev_net_info.cid_status == 0)
            {
                netWork_log("network timer cid_status[%d]\r\n", dev_net_info.cid_status);
                return -1;
            }
            fibo_taskSleep(100);
        }
        break;
    case NET_NO_SIM:
        REDLED_ON();
        push_index(34);
       // fibo_sem_signal(network_fail_indication);
        break;
    case NET_NO_IMSI:
    case NET_NO_CCID:
    case NET_NO_APN:
    case NET_NO_SIGNAL:
    case NET_NO_REGISTER:
    case NET_NO_ACTIVE:
        REDLED_ON();
        push_index(35);
       // fibo_sem_signal(network_fail_indication);
        break;
    }
    return 0;
}

static int network_pThreadId = 0;

static void network_task(void *arg)
{
    while (1)
    {
        network_loop();
        fibo_taskSleep(500);
    }
    network_pThreadId = 0;
    fibo_thread_delete();
}

int network_enter(void)
{
    int ret = -1;

    if (network_pThreadId == 0)
    {
        ret = fibo_thread_create_ex(network_task, "_network_task", 1024 * 4 * 10, NULL, OSI_PRIORITY_NORMAL, (UINT32 *)&network_pThreadId);
    }

    return ret;
}
