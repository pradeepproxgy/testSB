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

#include "fibo_opencpu.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

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
    fibo_textTrace("sig_res_callback  sig = %d", sig);
    switch (sig)
    {
        case GAPP_SIG_NW_SIGNAL_QUALITY_IND:
        {
            UINT8 nSim = (UINT8)va_arg(arg, int);
            INT16 rsrp = (INT16)va_arg(arg, int);
            INT16 rsrq = (INT16)va_arg(arg, int);
            UINT16 sinr = (INT16)va_arg(arg, int);
            UINT8 plmn[3] = {0};
            plmn[0] = (UINT8)va_arg(arg, int);
            plmn[1] = (UINT8)va_arg(arg, int);
            plmn[2] = (UINT8)va_arg(arg, int);
            UINT16 tac = (UINT16)va_arg(arg, int);
            UINT32 cellid = (UINT32)va_arg(arg, uint32_t);

            fibo_textTrace("[%s-%d]sim_id = %d,rsrp = %d,rsrq = %d,sinr = %d", __FUNCTION__, __LINE__,nSim,rsrp,rsrq,sinr);

            uint8_t mcc[3] = {0};
            uint8_t mnc[3] = {0};
            get_mcc_mnc_from_plmn(&plmn[0], &mcc[0], &mnc[0]);

            fibo_textTrace("MCC= %d%d%d,MNC= %d%d,tac = %X,cellid = %X",
            mcc[0],mcc[1],mcc[2],mnc[0],mnc[1],
            tac,
            cellid);
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

static FIBO_CALLBACK_T user_callback = {
    .fibo_signal = sig_res_callback,
    .at_resp = at_res_callback};

extern void test_printf(void);

static void prvInvokeGlobalCtors(void)
{
    extern void (*__init_array_start[])();
    extern void (*__init_array_end[])();

    size_t count = __init_array_end - __init_array_start;
    for (size_t i = 0; i < count; ++i)
        __init_array_start[i]();
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

key_callback get_cellinfo_cb(void *param)
{

    if(param == NULL)
    {
        fibo_textTrace("[%s-%d]GET cellinfo FAIL", __FUNCTION__, __LINE__);
        return NULL;
    }
    uint8_t sMCC[3] = {0};
    uint8_t sMNC[3] = {0};
    uint8_t sMNC_Number = 0;
    opencpu_cellinfo_t *cellinfo = (opencpu_cellinfo_t *)param;
    if(cellinfo->curr_rat >=4)
    {
        int cellid_s = cellinfo->lte_cellinfo.lteScell.enodeBID << 8 | cellinfo->lte_cellinfo.lteScell.cellId;
        sMNC_Number = get_mcc_mnc_from_plmn(&cellinfo->lte_cellinfo.lteScell.plmn[0], &sMCC[0], &sMNC[0]);
        fibo_textTrace("sMNC_Number = %d",sMNC_Number);
        fibo_textTrace("[%s-%d]LTE service MCC=%d%d%d,MNC=%d%d,CELL_ID = %lX,TAC=%X", __FUNCTION__, __LINE__,sMCC[0],sMCC[1],sMCC[2],sMNC[0],sMNC[1],cellid_s,cellinfo->lte_cellinfo.lteScell.tac);
        if(cellinfo->lte_cellinfo.lteNcellNum > 0)
        {
            for(int i =0; i<cellinfo->lte_cellinfo.lteNcellNum;i++)
            {
                memset(sMCC,0,3);
                memset(sMNC,0,3);
                int cellid_nb = cellinfo->lte_cellinfo.lteNcell[i].cellAccRelInfo.enodeBID << 8 | cellinfo->lte_cellinfo.lteNcell[i].cellAccRelInfo.cellId;
                sMNC_Number = get_mcc_mnc_from_plmn(&cellinfo->lte_cellinfo.lteNcell[i].cellAccRelInfo.plmn[0], &sMCC[0], &sMNC[0]);
                int tac = cellinfo->lte_cellinfo.lteNcell[i].cellAccRelInfo.tac;
                fibo_textTrace("[%s-%d]LTE Ncell[%d]MCC=%d%d%d,MNC=%d%d, CELL ID = %lX,TAC=%X", __FUNCTION__, __LINE__,i,sMCC[0],sMCC[1],sMCC[2],sMNC[0],sMNC[1],cellid_nb,tac);
            }
        }
    }
    return NULL;
   
}


static void oc_cell_demo_thread(void *param)
{
    fibo_textTrace("application thread enter, param 0x%x", param);

    for (int n = 0; n < 10; n++)
    {
        fibo_textTrace("hello world %d", n);
        fibo_taskSleep(1000);
    }

    fibo_nw_config_signal_quality_urc_t config_signal_quality_urc = {0};
    config_signal_quality_urc.config = FIBO_SIGNAL_QUALITY_URC_DEFAULT;
    fibo_nw_set_config(0, FIBO_NW_LTE_SIGNAL_QUALITY_URC, &config_signal_quality_urc, NULL);
    while(1)
    {
        fibo_textTrace("[%s-%d]cellinfo_ex begin", __FUNCTION__, __LINE__);
        fibo_getCellInfo_ex(get_cellinfo_cb, 0);
        fibo_taskSleep(20*1000);
    }

    test_printf();
    fibo_thread_delete();
}

static void oc_get_operator_info_demo(void)
{
    operator_info_t operator_info;
    fibo_get_operator_info(&operator_info,0);

    fibo_textTrace("plmn: %x %x %x %x %x %x",operator_info.operator_id[0],operator_info.operator_id[1],
        operator_info.operator_id[2],operator_info.operator_id[3],
        operator_info.operator_id[4],operator_info.operator_id[5]);
    fibo_textTrace("operator_name: %s",operator_info.operator_name);
    fibo_textTrace("operator_name: %s",operator_info.oper_short_name);
}

static void oc_get_operator_info_demo2(void)
{
    operator_info_t operator_info;
    fibo_send_cmd(5,NULL,0,&operator_info,NULL);

    fibo_textTrace("plmn: %x %x %x %x %x %x",operator_info.operator_id[0],operator_info.operator_id[1],
        operator_info.operator_id[2],operator_info.operator_id[3],
        operator_info.operator_id[4],operator_info.operator_id[5]);
    fibo_textTrace("operator_name: %s",operator_info.operator_name);
    fibo_textTrace("operator_name: %s",operator_info.oper_short_name);
}

static void oc_plmn_lock_demo(void)
{
    uint8_t simid = 0;
    uint8_t nMode = 0;    //white list
    uint8_t ltePlmnNum = 2;
    char ltePlmnList[2][6] = {"46012","46013"};    //when ltePlmnNum = 2

    uint8_t ret = 0;

    ret = fibo_plmn_lock_set(nMode,ltePlmnNum,(uint8_t *)ltePlmnList,simid);
    if(ret == false)
        fibo_textTrace("fibo_plmn_lock_set fail");
}

void * appimg_enter(void *param)
{
    fibo_textTrace("application image enter, param 0x%x", param);

    prvInvokeGlobalCtors();

    fibo_thread_create(oc_cell_demo_thread, "cell_demo_thread", 1024*4*2, NULL, OSI_PRIORITY_NORMAL);
    return (void *)&user_callback;
}

void appimg_exit(void)
{
    fibo_textTrace("application image exit");
}

