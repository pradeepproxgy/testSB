/*******************************************************************************
* Author		  :  author
* Version		  :  V1.0
* Date			  :  2021-08-17
* Description	  :  dev_model_api.h
********************************************************************************/
	
#ifndef __DEV_MODEL_API_H__
#define __DEV_MODEL_API_H__
	
/********************************* Include File ********************************/
#include <stdbool.h>
#include "cJSON.h"


/********************************* Macro Definition ****************************/


/********************************* Type Definition *****************************/
typedef enum
{
    Json_NULL,
	Json_STRING,
    Json_INT,
    Json_FLOAT,
    Json_BOOL,
}JsValuetype;


/********************************* Function Prototype Definition ***************/
//单属性上报, json数据创建
cJSON *UNI_OneProperty_CreateStr(char *proKey, char *proValue, char *ts);
cJSON *UNI_OneProperty_CreateInt(char *proKey, int proValue, char *ts);
cJSON *UNI_OneProperty_CreateFloat(char *proKey, float proValue, int stepLen, char *ts);
cJSON *UNI_OneProperty_CreateBool(char *proKey, bool proValue, char *ts);

//批量属性上报, json数据创建, 单个属性的增加或替换
cJSON *UNI_Propertys_Create(void);
int UNI_Propertys_AddStrPro(cJSON *root, char *proKey, char *proValue, char *ts);
int UNI_Propertys_AddIntPro(cJSON *root, char *proKey, int proValue, char *ts);
int UNI_Propertys_AddFloatPro(cJSON *root, char *proKey, float proValue, int stepLen, char *ts);
int UNI_Propertys_AddBoolPro(cJSON *root, char *proKey, bool proValue, char *ts);

//单事件上报, json数据创建, info数据增加或替换
cJSON *UNI_OneEvent_Create(char *eventKey, char *ts);
int UNI_OneEvent_AddStrInfo(cJSON *root, char *infoKey, char *infoValue);
int UNI_OneEvent_AddIntInfo(cJSON *root, char *infoKey, int infoValue);
int UNI_OneEvent_AddFloatInfo(cJSON *root, char *infoKey, float infoValue, int stepLen);
int UNI_OneEvent_AddBoolInfo(cJSON *root, char *infoKey, bool infoValue);

//批量事件上报, json数据创建，事件及信息的增加或替换
cJSON *UNI_Events_Create(void);
int UNI_Events_AddStrInfoToEvent(cJSON *root, char *eventKey, char *ts, char *infoKey, char *infoValue);
int UNI_Events_AddIntInfoToEvent(cJSON *root, char *eventKey, char *ts, char *infoKey, int infoValue);
int UNI_Events_AddFloatInfoToEvent(cJSON *root, char *eventKey, char *ts, char *infoKey, float infoValue, int stepLen);
int UNI_Events_AddBoolInfoToEvent(cJSON *root, char *eventKey, char *ts, char *infoKey, bool infoValue);

//服务调用的解析，获取key对应的value值
int UNI_Services_GetStrValue(cJSON *root, char *serKey, char *infoKey, char *infoValue, int infoValueLen);
int UNI_Services_GetIntValue(cJSON *root, char *serKey, char *infoKey, int *infoValue);
int UNI_Services_GetFloatValue(cJSON *root, char *serKey, char *infoKey, float *infoValue);
int UNI_Services_GetBoolValue(cJSON *root, char *serKey, char *infoKey, bool *infoValue);

//服务调用的响应，json数据创建，响应的增加或替换
cJSON *UNI_Services_CreateRsp(char *code, char *codeMsg, char *msgID);
int UNI_Services_AddStrInfo(cJSON *root, char *rspKey, char *rspValue);
int UNI_Services_AddIntInfo(cJSON *root, char *rspKey, int rspValue);
int UNI_Services_AddFloatInfo(cJSON *root, char *rspKey, float rspValue, int stepLen);
int UNI_Services_AddBoolInfo(cJSON *root, char *rspKey, bool rspValue);

//属性设置调用的解析，获取key对应的value值
int UNI_PropertySet_GetStrValue(cJSON *root, char *proKey, char *proValue, int proValueLen);
int UNI_PropertySet_GetIntValue(cJSON *root, char *proKey, int *proValue);
int UNI_PropertySet_GetFloatValue(cJSON *root, char *proKey, float *proValue);
int UNI_PropertySet_GetBoolValue(cJSON *root, char *proKey, bool *proValue);
cJSON *UNI_PropertySet_CreateRsp(char *code, char *codeMsg, char *msgID);


#endif

