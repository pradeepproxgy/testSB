/* Copyright (C) 2022 FIBOCOM Technologies Limited and/or its affiliates("FIBOCOM").
 * All rights reserved.
 *
 * This software is supplied "AS IS" without any warranties.
 * FIBOCOM assumes no responsibility or liability for the use of the software,
 * conveys no license or title under any patent, copyright, or mask work
 * right to the product. FIBOCOM reserves the right to make changes in the
 * software without notification.  FIBOCOM also make no representation or
 * warranty that such application will be suitable for the specified use
 * without further testing or modification.
 */


#define OSI_LOG_TAG OSI_MAKE_LOG_TAG('M', 'Y', 'A', 'P')

#include "fibo_opencpu_comm.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "osi_api.h"
#include "fibo_opencpu.h"

#define MAX_CID_SUPPORT 7
#define MAX_IP_LENGTH 60
#define PDP_ACTIVE 1
#define PDP_INACTIVE 0
#define PDP_ACTIVING 2
#define PDP_DEACTIVING 3

typedef struct{

    UINT8 pdp_status;
    UINT8 active_cid;
    UINT8 ip[MAX_IP_LENGTH];
    fibo_pdp_profile_t pdp_profile;
}pdp_cus_t;

typedef struct{
    UINT8 reg_status;
    pdp_cus_t  pdp_cus[MAX_CID_SUPPORT];
}net_info_cus_t;


static net_info_cus_t net_info;
char g_send_test[1024] = {0};

static void prvInvokeGlobalCtors(void)
{
    extern void (*__init_array_start[])();
    extern void (*__init_array_end[])();

    size_t count = __init_array_end - __init_array_start;
    for (size_t i = 0; i < count; ++i)
        __init_array_start[i]();
}

unsigned short htons(unsigned short n)
{
    return ((n & 0xff) << 8) | ((n & 0xff00) >> 8);
}

static void pdp_thread_entry(void *param)
{
    fibo_textTrace("[%s-%d]", __FUNCTION__, __LINE__);
    reg_info_t reg_stat = { 0 };
    int count = 0;
    memset(&net_info,0,sizeof(net_info));

    while(1)
    {
        if(fibo_reg_info_get(&reg_stat, 0)<0)
        {
            fibo_taskSleep(1000);
            fibo_textTrace("[%s-%d]", __FUNCTION__, __LINE__);
            continue;
        }

        fibo_textTrace("[%s-%d] nStatus = %d, rat =%d", __FUNCTION__, __LINE__, reg_stat.nStatus, reg_stat.curr_rat);
        if(reg_stat.nStatus)
        {
            count=0;
            fibo_taskSleep(1000);
            continue;
        }
        fibo_taskSleep(1000);
        count++;
        /*
         option operation, after 1 hour,turn on/off radio
        */
        if(count == 1*60*60)
        {
            fibo_cfun_set(0,0);
            fibo_taskSleep(500);
            fibo_cfun_set(1,0);
        }
        /*
         option operation, after 12 hours, reset the device
        */
        if(count == 12*60*60)
        {
            fibo_soft_reset();
        }
    }
}

static void data_thread_entry(void *param)
{
    fibo_textTrace("[%s-%d]", __FUNCTION__, __LINE__);
    struct timeval tv;
    fd_set readfds;
    fd_set errorfds;
    fd_set writeset;
    GAPP_TCPIP_ADDR_T addr;
    INT8 socketid;
    int ret = -1;
    char rcvbuf[1024];

    while(1)
    {
        bool pdp_active = 0;
        memset(&addr,0,sizeof(addr));
        memset(rcvbuf,0,sizeof(rcvbuf));
        if(net_info.reg_status)
        {
            for(int i=0;i<MAX_CID_SUPPORT;i++)
            {
                if(net_info.pdp_cus[i].pdp_status == PDP_ACTIVE)
                {
                    pdp_active = 1;
                    break;
                }
            }
        }
        else
        {
            goto error_exit;
        }
        if(!pdp_active)
            goto error_exit;
        socketid = fibo_sock_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if(socketid <= 0)
            goto error_exit;

        struct sockaddr_in addr4 = {0};
        addr4.sin_len = sizeof(struct sockaddr_in);
        addr4.sin_family = AF_INET;
        addr4.sin_port = htons(3120);
        fibo_sock_ip4addr_aton("47.110.234.36", (ip4_addr_t*)&addr4.sin_addr);
        //addr4.sin_port = htons(3564);
        //fibo_sock_ip4addr_aton("111.231.250.105", (ip4_addr_t*)&addr4.sin_addr);
        fibo_textTrace("sin_port=%x, sin_addr=%x", addr4.sin_port, addr4.sin_addr.s_addr);
        ret = fibo_sock_connect_v2(socketid, (struct sockaddr *)&addr4, sizeof(addr4));
        fibo_textTrace("connet socketId[%d] ret=%d", socketid, ret);
        if (ret < 0)
        {
            fibo_sock_close_v2(socketid);
            goto error_exit;
        }

        begin_send:
        memcpy(g_send_test, "123456abcd", sizeof("123456abcd"));
        ret = fibo_sock_send_v2(socketid, (UINT8 *)g_send_test, sizeof("123456abcd"),0);
        if(ret < 0){
            fibo_textTrace("[%s-%d]send error, errno = %d", __FUNCTION__, __LINE__, errno);
            goto error_exit;
        }
        fibo_textTrace("[%s-%d]send ok,len = %d",__FUNCTION__, __LINE__, ret);
        while(1)
        {
            FD_ZERO(&readfds);
            FD_ZERO(&errorfds);
            FD_ZERO(&writeset);
            FD_SET(socketid, &writeset);
            FD_SET(socketid, &readfds);
            FD_SET(socketid, &errorfds);
            tv.tv_sec  = 5; //5 seconds
            tv.tv_usec = 0;
            ret = fibo_sock_select(socketid+1, &readfds, NULL, &errorfds, &tv);
            if(ret <= 0){
            if (ret < 0) 
            {
                fibo_textTrace("[%s-%d]select ret < 0, error", __FUNCTION__, __LINE__);
            }
            if(ret == 0)
            {
                fibo_textTrace("[%s-%d]: select timeout.", __FUNCTION__, __LINE__);
                goto begin_send;
            }
            continue;
        }

        if(FD_ISSET(socketid, &errorfds))
        {
            int error = 0;
            uint32_t len = 4;
            fibo_textTrace("[%s-%d]socketid = %d, errno = %d\n", __FUNCTION__, __LINE__, socketid,errno);

            ret = fibo_sock_setsockopt(socketid, SOL_SOCKET, SO_ERROR,&error, len);
            fibo_textTrace("[%s-%d]fibo_sock_getOpt, error = %d\n", __FUNCTION__, __LINE__, error);
            fibo_taskSleep(1000);
            ret = fibo_sock_close(socketid);
            break;
        }
        else if(FD_ISSET(socketid, &readfds)) 
        {
            memset(rcvbuf,0,sizeof(rcvbuf));
            ret = fibo_sock_recv_v2(socketid, (UINT8 *)rcvbuf, 1024,0);
            if(ret <= 0){
                fibo_textTrace("[%s-%d]recv error, errno = %d\n", __FUNCTION__, __LINE__, errno);
                goto error_exit;
            }
            fibo_textTrace("[%s-%d]recv is OK, len = %d,tmp = %s\n", __FUNCTION__, __LINE__,ret, rcvbuf);
            fibo_textTrace("[%s-%d]recv is OK, len = %d,tmp = %c,%c\n", __FUNCTION__, __LINE__,ret, rcvbuf[0],rcvbuf[1]);
        }
            fibo_taskSleep(1000); 
        }
        error_exit:
        fibo_taskSleep(5000);
        fibo_textTrace("[%s-%d] ", __FUNCTION__, __LINE__);
    }

}

static uint8_t get_mcc_mnc_from_plmn(uint8_t *plmn, uint8_t *mcc, uint8_t *mnc)
{
    if (plmn == NULL || mcc == NULL || mnc == NULL)
    {
        return 0;
    }
    mcc[0] = plmn[0] & 0x0F;
    mcc[1] = plmn[0] >> 4;
    mcc[2] = plmn[1] & 0x0F;

    mnc[0] = plmn[2] & 0x0F;
    mnc[1] = plmn[2] >> 4;
    mnc[2] = plmn[1] >> 4;

    if (mnc[2] > 9)
    {
        mnc[2] = 0;
        return 2;
    }
    else
    {
        return 3;
    }
}

static void sig_res_callback(GAPP_SIGNAL_ID_T sig, va_list arg)
{
    switch (sig)
    {
        case GAPP_SIG_PDP_ACTIVE_IND:
        {
            UINT8 cid = (UINT8)va_arg(arg, int);
            fibo_textTrace("sig_res_callback  cid = %d", cid);
            va_end(arg);
            for(int i=0;i<MAX_CID_SUPPORT;i++)
            {
                if(net_info.pdp_cus[i].active_cid == cid)
                {
                    net_info.pdp_cus[i].active_cid = cid;
                    net_info.pdp_cus[i].pdp_status = PDP_ACTIVE;
                    break;
                }
            }
        }
        break;

        case GAPP_SIG_PDP_RELEASE_IND:
        {
            UINT8 cid = (UINT8)va_arg(arg, int);
            fibo_textTrace("sig_res_callback  cid = %d", cid);
            va_end(arg);
            for(int i=0;i<MAX_CID_SUPPORT;i++)
            {
                if(net_info.pdp_cus[i].active_cid==cid)
                {
                    net_info.pdp_cus[i].active_cid = 0;
                    net_info.pdp_cus[i].pdp_status = PDP_INACTIVE;
                    break;
                }
            }
        }
        break;
        case GAPP_SIG_PDP_ACTIVE_OR_DEACTIVE_FAIL_IND:
        {
            UINT8 cid = (UINT8)va_arg(arg, int);
            UINT8 state = (UINT8)va_arg(arg, int);
            fibo_textTrace("[%s-%d]cid = %d,state = %d", __FUNCTION__, __LINE__,cid,state);
            va_end(arg);
            if(net_info.reg_status == 1)
            {
                fibo_taskSleep(500);
                for(int i=0;i<MAX_CID_SUPPORT;i++)
                {
                    if(net_info.pdp_cus[i].active_cid == cid && net_info.pdp_cus[i].pdp_status == PDP_ACTIVING)
                    {
                        if(fibo_pdp_active(1,&net_info.pdp_cus->pdp_profile,0)<0)
                        {
                            fibo_textTrace("[%s-%d]", __FUNCTION__, __LINE__);
                            net_info.pdp_cus[i].active_cid = 0;
                            net_info.pdp_cus[i].pdp_status = PDP_INACTIVE;
                            break;
                        }
                        net_info.pdp_cus[i].pdp_status = PDP_ACTIVING;
                        break;
                    }
                }
            }
        }
        break;
        case GAPP_SIG_PDP_DEACTIVE_ABNORMALLY_IND:
        {
            UINT8 cid = (UINT8)va_arg(arg, int);
            fibo_textTrace("[%s-%d]cid = %d", __FUNCTION__, __LINE__,cid);
            va_end(arg);
            for(int i=0;i<MAX_CID_SUPPORT;i++)
            {
                if(net_info.pdp_cus[i].active_cid == cid)
                {
                    net_info.pdp_cus[i].active_cid = 0;
                    net_info.pdp_cus[i].pdp_status = PDP_INACTIVE;
                    if(net_info.reg_status == 1)
                    {
                        fibo_taskSleep(500);
                        if(fibo_pdp_active(1,&net_info.pdp_cus[i].pdp_profile,0))
                        {
                            fibo_textTrace("[%s-%d]", __FUNCTION__, __LINE__);
                            net_info.pdp_cus[i].active_cid = 0;
                            net_info.pdp_cus[i].pdp_status = PDP_INACTIVE;
                        }
                        net_info.pdp_cus[i].active_cid = cid;
                        net_info.pdp_cus[i].pdp_status = PDP_ACTIVING;
                    }
                    break;
                }
            }
        }
        break;
        case GAPP_SIG_RRC_STATUS_REPORT:
        {
            UINT8 conn = (UINT8)va_arg(arg, int);
            UINT8 state = (UINT8)va_arg(arg, int);
            UINT8 access = (UINT8)va_arg(arg, int);
            UINT8 sim = (UINT8)va_arg(arg, int);
            fibo_textTrace("[%s-%d]conn = %d,state=%d,access=%d,sim=%d", __FUNCTION__, __LINE__,conn,state,access,sim);
            va_end(arg);
        }
        break;
        case GAPP_SIG_REG_STATUS_IND:
        {
            bool need_active_pdp = 1;
            UINT8 status = (UINT8)va_arg(arg, int);
            UINT32 cellid = (UINT32)va_arg(arg, int);
            UINT16 tac = (UINT16)va_arg(arg, int);
            UINT8 access = (UINT8)va_arg(arg, int);
            fibo_textTrace("[%s-%d]status = %d,cellid=%x,tac=%x,access=%d", __FUNCTION__, __LINE__,status,cellid,tac,access);
            va_end(arg);
            net_info.reg_status = status;
            /*
             just only,if find just one pdp in actived or in activing state ,not need active any more,except mutilate PDN connection
            */
            for(int i=0;i<MAX_CID_SUPPORT;i++)
            {
                if(net_info.pdp_cus[i].pdp_status == PDP_ACTIVE || net_info.pdp_cus[i].pdp_status == PDP_ACTIVING)
                {
                    need_active_pdp = 0;
                    break;
                }
            }
            if(need_active_pdp)
            {
                fibo_taskSleep(500);
                if(net_info.reg_status)
                {
                    if(fibo_pdp_active(1, NULL, 0)<0)
                    {
                        fibo_textTrace("[%s-%d]", __FUNCTION__, __LINE__);
                        break;
                    }
                    for(int i=0;i<MAX_CID_SUPPORT;i++)
                    {
                        if(net_info.pdp_cus[i].active_cid == 0 || net_info.pdp_cus[i].active_cid == 1)
                        {
                            net_info.pdp_cus[i].pdp_status = PDP_ACTIVING;
                            net_info.pdp_cus[i].pdp_profile.cid=1;
                            net_info.pdp_cus[i].active_cid=1;
                            break;
                        }
                    }
                }

#if 0
                // maybe need active new APN on other CIDs according customer device
                if(net_info.reg_status){
                    fibo_pdp_profile_t pdp_profile;
                    memset(&pdp_profile,0,sizeof(pdp_profile));
                    pdp_profile.cid=2;
                    memcpy(pdp_profile.apn,"3gnet",sizeof("3gnet"));
                    memcpy(pdp_profile.nPdpType,"IPV4V6",sizeof("IPV4V6"));
                    fibo_pdp_active(1, &pdp_profile, 0); //active CID2 
                    memcpy(&net_info.pdp_cus->pdp_profile,&pdp_profile,sizeof(pdp_profile));
                    for(int i=0;i<MAX_CID_SUPPORT;i++)
                    {
                        if(net_info.pdp_cus[i].active_cid == 0 || net_info.pdp_cus[i].active_cid ==net_info.pdp_cus->pdp_profile.cid )
                        {
                             net_info.pdp_cus[i].active_cid = pdp_profile.cid;
                             net_info.pdp_cus[i].pdp_status = PDP_ACTIVING;
                             break;
                        }
                    }
                }
#endif
            }
        }
        break;
        case GAPP_SIG_REG_REJECT_IND:
        {
            UINT8 nSim = (UINT8)va_arg(arg, int);
            UINT8 cause_type = (UINT8)va_arg(arg, int);    //0:EMM cause 3GPP TS 24.301 
            UINT8 reject_cause = (UINT8)va_arg(arg, int);
            UINT8 plmn[3] = {0};
            plmn[0] = (UINT8)va_arg(arg, int);
            plmn[1] = (UINT8)va_arg(arg, int);
            plmn[2] = (UINT8)va_arg(arg, int);
            UINT16 tac = (UINT16)va_arg(arg, int);
            UINT8 cellid[4] = {0};
            cellid[0] = (UINT8)va_arg(arg, int);
            cellid[1] = (UINT8)va_arg(arg, int);
            cellid[2] = (UINT8)va_arg(arg, int);
            cellid[3] = (UINT8)va_arg(arg, int);
            fibo_textTrace("[%s-%d]sim_id = %d,cause_type = %d,reject_cause = %d", __FUNCTION__, __LINE__,nSim,cause_type,reject_cause);

            uint8_t mcc[3] = {0};
            uint8_t mnc[3] = {0};
            get_mcc_mnc_from_plmn(&plmn[0], &mcc[0], &mnc[0]);

            fibo_textTrace("MCC= %d%d%d,MNC= %d%d,tac = %X,cellid = %X%X%X%X",
            mcc[0],mcc[1],mcc[2],mnc[0],mnc[1],
            tac,
            cellid[0],cellid[1],cellid[2],cellid[3]);
            va_end(arg);
        }
        break;
        default:
            break;
    }
}

static void at_res_callback(UINT8 *buf, UINT16 len)
{
    fibo_textTrace("FIBO <--%s", buf);
}

FIBO_CALLBACK_T user_callback = {
    .fibo_signal = sig_res_callback,
    .at_resp = at_res_callback};


void *appimg_enter(void *param)
{
    fibo_textTrace("[%s-%d] param 0x%x",__FUNCTION__, __LINE__, param);
    prvInvokeGlobalCtors();
    fibo_thread_create(pdp_thread_entry, "pdp_custhread", 1024*8*2, NULL, OSI_PRIORITY_NORMAL);
    fibo_thread_create(data_thread_entry, "data_custhread", 1024*8*2, NULL, OSI_PRIORITY_NORMAL);
    return (void *)&user_callback;
}

void appimg_exit(void)
{
    fibo_textTrace("application image exit");
}
