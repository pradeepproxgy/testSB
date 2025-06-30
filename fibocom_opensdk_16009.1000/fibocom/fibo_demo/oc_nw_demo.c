#include "osi_api.h"
#include "osi_log.h"

//char *app_ver = "APP_0001";

#include "app_image.h"
#include "fibo_opencpu.h"

uint8_t g_sim_id = 0;
static void sig_res_callback(GAPP_SIGNAL_ID_T sig, va_list arg)
{
    fibo_textTrace("sig_res_callback  sig = %d", sig);
}

static void at_res_callback(UINT8 *buf, UINT16 len)
{
    fibo_textTrace("FIBO <--%s", buf);
}

static FIBO_CALLBACK_T user_callback = {
    .fibo_signal = sig_res_callback,
    .at_resp = at_res_callback};

/*get network roaming status*/
int fibo_app_get_nw_roaming_status(uint8_t simId, uint8_t *roaming_status)
{
    return fibo_nw_get_config(simId, FIBO_NW_ROAMING_STATUS, NULL, roaming_status);
}

/*get LTE timing advance*/
int fibo_app_get_nw_lte_ta(uint8_t simId, int16_t *lte_ta)
{
    return fibo_nw_get_config(simId, FIBO_NW_LTE_TIMING_ADVANCE, NULL, lte_ta);
}
#if 0
/*get GTRRCREL value*/
int fibo_app_get_nw_gtrrcrel(uint8_t simId, fibo_nw_config_rrcrel_t *gtrrcrel)
{
    return fibo_nw_get_config(simId, FIBO_NW_GTRRCREL, NULL, gtrrcrel);
}

/*set GTRRCREL value*/
int fibo_app_set_nw_gtrrcrel(uint8_t simId, fibo_nw_config_rrcrel_t *gtrrcrel)
{
    return fibo_nw_set_config(simId, FIBO_NW_GTRRCREL, gtrrcrel, NULL);
}
#endif
/*test demo: roaming status*/
void fibo_test_demo_roaming_status(void)
{
    uint8_t roaming_status = 0;
    int ret = 0;

    ret = fibo_app_get_nw_roaming_status(g_sim_id, &roaming_status);

    if (0 != ret)
    {
        fibo_textTrace("FIBO APP TEST: %s error %d", __FUNCTION__, ret);
        return;
    }

    if (1 == roaming_status)
    {
        fibo_textTrace("FIBO APP TEST: %s camping network: home network", __FUNCTION__); //home network
    }
    else if (5 == roaming_status)
    {
        fibo_textTrace("FIBO APP TEST: %s camping network: roaming network", __FUNCTION__); //roaming network
    }
    else
    {
        fibo_textTrace("FIBO APP TEST: %s camping network: no registered", __FUNCTION__); //no registered;
    }
}


/*test demo: LTE timing advance*/
void fibo_test_demo_lte_timing_advance(void)
{
    int16_t lte_timing_advance = 0;
    int ret = 0;
    
    ret = fibo_app_get_nw_lte_ta(g_sim_id, &lte_timing_advance);

    if (0 != ret)
    {
        fibo_textTrace("FIBO APP TEST: %s error %d", __FUNCTION__, ret);
        return;
    }

    fibo_textTrace("FIBO APP TEST: %s %d", __FUNCTION__, lte_timing_advance);
}
#if 0
/*test demo: gtrrcrel set value, get value
value range: 0 - 20
mode range: 0 - 1
frequency range: 0 - 30*/
void fibo_test_demo_gtrrcrel(void)
{
    int ret = 0;
    
    fibo_nw_config_rrcrel_t gtrrcrel;
    memset(&gtrrcrel, 0, sizeof(fibo_nw_config_rrcrel_t));

    ret = fibo_app_get_nw_gtrrcrel(g_sim_id, &gtrrcrel);

    if (0 != ret)
    {
        fibo_textTrace("FIBO APP TEST: %s return error: %d", __FUNCTION__, ret);
    }

    fibo_textTrace("FIBO APP TEST %s gtrrcrel.frequency:%d, gtrrcrel.mode:%d, gtrrcrel.value:%d",__FUNCTION__, gtrrcrel.frequency, gtrrcrel.mode, gtrrcrel.value);

    memset(&gtrrcrel, 0, sizeof(fibo_nw_config_rrcrel_t));
    gtrrcrel.frequency = 5;
    gtrrcrel.mode = 1;
    gtrrcrel.value = 3;

    ret = fibo_app_set_nw_gtrrcrel(g_sim_id, &gtrrcrel);

    if (0 != ret)
    {
        fibo_textTrace("FIBO APP TEST: %s return error: %d", __FUNCTION__, ret);
    }

    fibo_textTrace("FIBO APP TEST %s gtrrcrel.frequency:%d, gtrrcrel.mode:%d, gtrrcrel.value:%d",__FUNCTION__, gtrrcrel.frequency, gtrrcrel.mode, gtrrcrel.value);

    memset(&gtrrcrel, 0, sizeof(fibo_nw_config_rrcrel_t));

    ret = fibo_app_get_nw_gtrrcrel(g_sim_id, &gtrrcrel);
    
    if (0 != ret)
    {
        fibo_textTrace("FIBO APP TEST: %s return error: %d", __FUNCTION__, ret);
    }

    fibo_textTrace("FIBO APP TEST %s gtrrcrel.frequency:%d, gtrrcrel.mode:%d, gtrrcrel.value:%d",__FUNCTION__, gtrrcrel.frequency, gtrrcrel.mode, gtrrcrel.value);

    memset(&gtrrcrel, 0, sizeof(fibo_nw_config_rrcrel_t));
    gtrrcrel.mode = 2;

    ret = fibo_app_set_nw_gtrrcrel(g_sim_id, &gtrrcrel);

    if (0 != ret)
    {
        fibo_textTrace("FIBO APP TEST: %s return error: %d", __FUNCTION__, ret);
    }

    fibo_textTrace("FIBO APP TEST %s gtrrcrel.frequency:%d, gtrrcrel.mode:%d, gtrrcrel.value:%d",__FUNCTION__, gtrrcrel.frequency, gtrrcrel.mode, gtrrcrel.value);

    
    memset(&gtrrcrel, 0, sizeof(fibo_nw_config_rrcrel_t));
    gtrrcrel.value = 21;

    ret = fibo_app_set_nw_gtrrcrel(g_sim_id, &gtrrcrel);

    if (0 != ret)
    {
        fibo_textTrace("FIBO APP TEST: %s return error: %d", __FUNCTION__, ret);
    }

    fibo_textTrace("FIBO APP TEST %s gtrrcrel.frequency:%d, gtrrcrel.mode:%d, gtrrcrel.value:%d",__FUNCTION__, gtrrcrel.frequency, gtrrcrel.mode, gtrrcrel.value);

    memset(&gtrrcrel, 0, sizeof(fibo_nw_config_rrcrel_t));
    gtrrcrel.frequency = 31;

    ret = fibo_app_set_nw_gtrrcrel(g_sim_id, &gtrrcrel);

    if (0 != ret)
    {
        fibo_textTrace("FIBO APP TEST: %s return error: %d", __FUNCTION__, ret);
    }

    fibo_textTrace("FIBO APP TEST %s gtrrcrel.frequency:%d, gtrrcrel.mode:%d, gtrrcrel.value:%d",__FUNCTION__, gtrrcrel.frequency, gtrrcrel.mode, gtrrcrel.value);
    
}
#endif
static uint8_t fibo_app_get_mcc_mnc_from_plmn(uint8_t *plmn, uint8_t *mcc, uint8_t *mnc)
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

void fibo_test_demo_get_scellinfo(void)
{
    int ret = 0;
    opencpu_cellinfo_t opencpu_cellinfo;
    uint8_t sMNC_Number = 0;
    uint8_t sMCC[3] = {0};
	uint8_t sMNC[3] = {0};
    
    int lte_cellid = 0;
    int lte_tac = 0;
    int lte_enodeBID = 0;
    int16_t lte_timing_advance = 0;
#if 0
    int gsm_cellid = 0;
    int gsm_lac = 0;
    uint8_t gsm_timing_advance = 0;
#endif
    uint16_t netMode = 0; // 0- TDD, 1 -- FDD;
    uint16_t distance = 0;

    ret = fibo_get_scellinfo(&opencpu_cellinfo, g_sim_id);

    if (0 != ret)
    {
        fibo_textTrace("FIBO APP TEST: %s return error: %d", __FUNCTION__, ret);

        return;
    }

    if (7 == opencpu_cellinfo.curr_rat)
    {
        sMNC_Number = fibo_app_get_mcc_mnc_from_plmn(&opencpu_cellinfo.lte_cellinfo.lteScell.plmn[0], &sMCC[0], &sMNC[0]);
        lte_cellid = opencpu_cellinfo.lte_cellinfo.lteScell.enodeBID << 8 | opencpu_cellinfo.lte_cellinfo.lteScell.cellId;
        lte_tac = opencpu_cellinfo.lte_cellinfo.lteScell.tac;
        lte_enodeBID = opencpu_cellinfo.lte_cellinfo.lteScell.enodeBID;
        fibo_textTrace("FIBO APP TEST fibo_get_scellinfo. LTE info:sMNC_Number:%d, mcc:%d%d%d,mnc:%d%d%d,cell_id:%d,tac:%d, lte_enodeBID: %d",sMNC_Number, sMCC[0],sMCC[1],sMCC[2],sMNC[0],sMNC[1],sMNC[2],lte_cellid,lte_tac, lte_enodeBID);

        netMode = opencpu_cellinfo.lte_cellinfo.lteScell.netMode;
        fibo_textTrace("FIBO APP TEST fibo_get_scellinfo. LTE info:netMode %d", netMode);
        if (0 == netMode)
            fibo_textTrace("FIBO APP TEST fibo_get_scellinfo. LTE info:netMode TDD");
        else if (1 == netMode)
            fibo_textTrace("FIBO APP TEST fibo_get_scellinfo. LTE info:netMode FDD");
        else
            fibo_textTrace("FIBO APP TEST fibo_get_scellinfo. LTE info:netMode is %d not TDD/FDD", netMode);

        fibo_app_get_nw_lte_ta(g_sim_id, &lte_timing_advance);
        fibo_textTrace("FIBO APP TEST fibo_get_scellinfo. LTE info:lte_timing_advance %d", lte_timing_advance);

        /*Calculate the distance between UE and base station based on TA
          LTE range 0 ~1282*/
        if (lte_timing_advance >= 0 && lte_timing_advance < 1283)
        {
            if (0 == netMode)//TDD needs to consider the upstream and downstream conversion time, and 624Ts is the initial TA
            {
                distance = (lte_timing_advance - 624) * 489 / 100;
            }
            else
            {
                distance = lte_timing_advance * 489 / 100;
            }
        }

        fibo_textTrace("FIBO APP TEST fibo_get_scellinfo. LTE info:calculate %d", distance);
    }
    #if 0
    /*if not support GSM, this code need delete, 8850 need delete this code*/
    else
    {
        sMNC_Number = fibo_app_get_mcc_mnc_from_plmn(&opencpu_cellinfo.gsm_cellinfo.pCurrCellInfo.nTSM_LAI[0], &sMCC[0], &sMNC[0]);
        gsm_cellid = opencpu_cellinfo.gsm_cellinfo.pCurrCellInfo.nTSM_CellID[0] << 8 | opencpu_cellinfo.gsm_cellinfo.pCurrCellInfo.nTSM_CellID[1];
        gsm_lac = opencpu_cellinfo.gsm_cellinfo.pCurrCellInfo.nTSM_LAI[3] << 8 | opencpu_cellinfo.gsm_cellinfo.pCurrCellInfo.nTSM_LAI[4];
        fibo_textTrace("FIBO APP TEST fibo_get_scellinfo. GSM info:sMNC_Number:%d, mcc:%d%d%d,mnc:%d%d%d,cell_id:%d,lac:%d",sMNC_Number, sMCC[0],sMCC[1],sMCC[2],sMNC[0],sMNC[1],sMNC[2],gsm_cellid,gsm_lac);

        gsm_timing_advance = opencpu_cellinfo.gsm_cellinfo.pCurrCellInfo.nTSM_TimeADV;
        fibo_textTrace("FIBO APP TEST fibo_get_scellinfo. GSM info:gsm_timing_advance %d", gsm_timing_advance);

        /*Calculate the distance between UE and base station based on TA;
        GSM TA is 255, this is invalid value
        GSM TA range 0 ~ 63*/
        if (gsm_timing_advance >= 0 && gsm_timing_advance < 64)
        {
            distance = gsm_timing_advance * 550;
            fibo_textTrace("FIBO APP TEST fibo_get_scellinfo. GSM info:calculate %d", distance);
        }
    }
    #endif
}


static void hello_world_demo()
{
    fibo_textTrace("application thread enter");
    set_app_ver(app_ver);

    fibo_taskSleep(3*1000);
    reg_info_t reg_info;

    while(1)
    {
        fibo_reg_info_get(&reg_info, 0);
        if (reg_info.nStatus == 1)
            break;
        fibo_taskSleep(1*1000);
        fibo_textTrace("FIBO APP TEST fibo_reg_info_get reg_info.nStatus:%d",reg_info.nStatus);
    }

    fibo_test_demo_roaming_status();

    fibo_test_demo_lte_timing_advance();

    //fibo_test_demo_gtrrcrel();

    fibo_test_demo_get_scellinfo();

    while(1)
    {
        fibo_taskSleep(3*1000);
        fibo_test_demo_get_scellinfo();
    }

}


void *appimg_enter(void *param)
{
    fibo_textTrace("app image enter");

    fibo_thread_create(hello_world_demo, "mythread", 10*1024, NULL, OSI_PRIORITY_NORMAL);
    return (void *)&user_callback;
}

void appimg_exit(void)
{
    OSI_LOGI(0, "application image exit");
}

