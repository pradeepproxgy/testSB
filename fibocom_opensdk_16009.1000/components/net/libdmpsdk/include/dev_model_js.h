/*******************************************************************************
* Company		  :  unicom-IOT
* Author		  :  lwd
* Version		  :  V1.0
* Date			  :  2021-05-27
* Description	  :  dev_model_js.h
********************************************************************************/

/********************************* Include File ********************************/
#ifndef _DEV_MODEL_JS_H_
#define  _DEV_MODEL_JS_H_

#include "cJSON.h"
#include "util_macro.h"
//#include "deviceshadow_api.h"

/********************************* Macro Definition ****************************/



/********************************* Type Definition *****************************/


/********************************* Variables ***********************************/


/********************************* Function ************************************/

cJSON *uni_create_js_model(void);
cJSON *uni_create_js_psmshadow(char *mode);

cJSON *uni_create_js_shadow(void);
cJSON *uni_create_js_devicecard(char *iccid, char *imsi);
cJSON *uni_create_js_terminalreg(char *ts);
cJSON *uni_create_js_common(char *iccid);

void uni_addstr_js_model(cJSON *js_array,char *key,char *value,char *epoch_time);
void uni_addnum_js_model(cJSON *js_array,char *key,int value,char *epoch_time);
int uni_getmsgid_js_shadow(cJSON *root, char *msgID, int msgIDLen);
int32_t uni_js_get_value(const char *input, uint32_t input_len, const char *key, uint32_t key_len, char **value, uint32_t *value_len);
void uni_freejson(cJSON *root);



#endif



