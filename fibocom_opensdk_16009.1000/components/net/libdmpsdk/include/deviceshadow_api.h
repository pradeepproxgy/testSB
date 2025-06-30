/*******************************************************************************
* Author		  :  author
* Version		  :  V1.0
* Date			  :  2021-08-17
* Description	  :  deviceshadow_api.h
********************************************************************************/

#ifndef _DEVICESHADOW_H_
#define _DEVICESHADOW_H_

#if defined(__cplusplus)
extern "C" {
#endif
/********************************* Include File ********************************/
//#include "util_types.h"
//#include "util_defs.h"
#include "cJSON.h"

#include "mqtt_client.h" //ws add

/********************************* Macro Definition ****************************/

/********************************* Type Definition *****************************/
typedef enum
{
    SHADOW_NULL,
    SHADOW_STRING,
    SHADOW_INT,
    SHADOW_FLOAT,
    SHADOW_BOOL,
} ShadowDatatype;

typedef struct
{
    char *key;               // property key name
    ShadowDatatype dataType; // property data type
    int dataStepLen;         // property data step len, only for float, -1：默认步长; 0和正数：需要的步长
    void *reportData;        // property report data
    int reportDataLen;       // property report data len, only for string
    void *desireData;        // property desir data
    int desireDataLen;       // property desir data len, only for string
    uint64_t reportTs;       // property report timestamp, unit:ms
    uint64_t desireTs;       // property desir timestamp, unit:ms
    uint32_t version;
    int getResult; // result of get property from json, 负数代表错误，正数，右第一位代表desired有值，右第二位代表reported有值
} ShadowProperty;

/********************************* Variables ***********************************/

/********************************* Function Prototype Definition ***************/
void PrintShadowPro(ShadowProperty *pProperty);
void ShadowSubCallback(void *topicData);

//初始化属性
void *UNI_Shadow_InitProperty(ShadowDatatype Datatype, char *key);

//解析平台反馈的设备影子json文档
int UNI_Shadow_GetPropertyValue(cJSON *root, ShadowProperty *property);

//订阅设备影子的相关主题
int UNI_Shadow_SubCommand(void *handle, uni_mqtt_qos_s qos);     //ws add qos
int UNI_Shadow_SubUpdateReply(void *handle, uni_mqtt_qos_s qos); //ws add qos
int UNI_Shadow_SubGetReply(void *handle, uni_mqtt_qos_s qos);    //ws add qos
//ws add
int UNI_Shadow_UnSubCommand(void *handle);
int UNI_Shadow_UnSubUpdateReply(void *handle);
int UNI_Shadow_UnSubGetReply(void *handle);

//主动获取设备影子
int UNI_Shadow_PubGet(void *handle, char *propertyName, uni_mqtt_qos_s qos);

//响应平台下发
int UNI_Shadow_PubCommandReply(void *handle, char *code, char *codeMsg, char *msgID);

//更新设备影子
int UNI_Shadow_UpdateAddPro(cJSON *root, ShadowProperty *property);
int UNI_Shadow_UpdatePub(void *handle, cJSON *payload, uni_mqtt_qos_s qos);

//清理设备影子的期望值
int UNI_Shadow_CleanDesiredAddPro(cJSON *root, ShadowProperty *property);
int UNI_Shadow_CleanDesiredPub(void *handle, cJSON *payload);

#if defined(__cplusplus)
}
#endif
#endif
