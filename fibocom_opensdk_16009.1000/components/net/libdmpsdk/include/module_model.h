/*******************************************************************************
* Company		  :  unicom-IOT
* Author		  :  lwd
* Version		  :  V1.0
* Date			  :  2021-05-27
* Description	  :  module_model.h
********************************************************************************/

/********************************* Include File ********************************/
#ifndef __DEV_MODULE_API_H__
#define __DEV_MODULE_API_H__

/********************************* Macro Definition ****************************/



/********************************* Type Definition *****************************/


/********************************* Variables ***********************************/

extern int rsrp;
extern float rsrq;

extern uint8_t imei[UNI_IMEI_INFO_LEN];
extern uint8_t imsi[UNI_IMSI_INFO_LEN];

extern uint8_t firmware_version[UNI_FIREWARE_VER_LEN];
extern uint8_t hardware_version[UNI_HARDWARE_VER_LEN];
extern uint8_t manufacture_info[UNI_MANUFACTURE_INFO_LEN];
extern uint8_t model_info[UNI_MODULEINFO_LEN];
extern char name[UNI_APN_INFO_LEN];
//uint8_t rssi_copy[UNI_RSSI_INFO_LEN] = {0};
extern uint8_t rssi;
extern uint8_t ber;

extern uint8_t cellid[UNI_CELLID_LEN];
extern uint8_t iccid_cut[UNI_ICCID_LEN];

/********************************* Function ************************************/
void UNI_Get_ModuleSysInfo();
int UNI_Sub_module_MODEL(void *handle);
int UNI_Pub_module_MODEL(void *handle);

#endif

