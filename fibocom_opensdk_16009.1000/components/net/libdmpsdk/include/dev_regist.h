/*******************************************************************************
* Company		  :  unicom-IOT
* Author		  :  lwd
* Version		  :  V1.0
* Date			  :  2021-05-27
* Description	  :  dev_regist.h
********************************************************************************/

/********************************* Include File ********************************/
#ifndef _DEV_REGIST_H_
#define _DEV_REGIST_H_

#include "util_macro.h"
#include "util_common.h"

/********************************* Macro Definition ****************************/



/********************************* Type Definition *****************************/


/********************************* Variables ***********************************/

/********************************* Function ************************************/



int _construct_first_reg_password(const char *device_id, global_devinfo_s *meta_info, uni_auth_mqtt_s *passwdout);
int32_t UNI_MQTT_AuthInfoSign( global_devinfo_s *meta, uni_auth_mqtt_s *authinfo);
int _uni_noreg_clientid_construct(char *clientid_string, const char *device_id, const char *productkey,int bindmode);

#endif
