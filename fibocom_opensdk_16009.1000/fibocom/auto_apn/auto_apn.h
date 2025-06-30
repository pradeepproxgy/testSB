/*****************************************************************
*   Copyright (C), 2018, Xian Fibocom Wireless Inc
*              All rights reserved.
*   FileName    :auto_apn.h
*   Author      :dongfu
*   Created     :2020-03-31
*   Description :auto_apn.h
*****************************************************************/

#pragma once
#ifndef _AUTO_APN_H_
#define _AUTO_APN_H_
#ifdef __cplusplus

extern "C"
{
#endif

#define  ICCID_LEN 24
#define  IMSI_LEN 16


typedef struct
{
    char   iccid[24];
    char   imsi[20];
}last_sim_info_t;

typedef struct
{
    char *plmn;         ///< string value
    char *iccid;        ///< string value
    char *short_name;   ///< string value
    int   mark;
} operator_plmn_name_map_t;


void fibo_init_last_sim_info(CFW_SIM_ID nSimID);
int CFW_GprsGetDefaultPdnInfo_fibo(CFW_GPRS_PDPCONT_DFTPDN_INFO *PdnInfo, CFW_SIM_ID nSimID);
int fibo_remove_pdp(CFW_SIM_ID nSim);
int fibo_addr6_dispay_format(uint8_t format, char *addr6_in, char *addr6_out);
void fibo_set_initial_pdp_context(uint8_t pdptype, char *apn, uint8_t nAuthProt, char *username, char *pwd, CFW_SIM_ID nSim);
char *fibo_get_ICCID(CFW_SIM_ID nSimID);
int find_mark_by_iccid(char *iccid);

#ifdef __cplusplus
}
#endif
#endif  // _AUTO_APN_H_
