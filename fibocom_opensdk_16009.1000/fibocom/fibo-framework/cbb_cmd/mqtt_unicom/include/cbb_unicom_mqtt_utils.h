#ifndef CBB_UNICOM_MQTT_UTILS_H
#define CBB_UNICOM_MQTT_UTILS_H

#include "at_cfw.h"
#include "ffw_log.h"
#include "ffw_utils.h"

#define PLATFORM_UNISOC //debug

#define UNICOM_ICCID_LEN 24
#define UNICOM_IMSI_LEN 24

typedef struct
{
    void *value;
    int type;
    int min;
    int max;
    const char *name;
    const char *hint;
} sysnvItem_t;
#define ITEM_INT_VAL(pval) (*((int *)pval))
#define ITEM_BOOL_VAL(pval) (*((bool *)pval))

extern const sysnvItem_t *prvNameGetItem_ex(const char *name);
extern bool osiSysnvSave(void);
extern uint8_t CFW_nvGetPsmEnable(uint8_t nSimID);
extern uint8_t Mapping_Creg_From_PsType(uint8_t pstype);
extern int nvmReadSN(int index, void *nvmSn, int size);

typedef enum
{
    UNICOM_MQTT_CHINA_MOBILE_1 = 46000,
    UNICOM_MQTT_CHINA_MOBILE_2 = 46002,
    UNICOM_MQTT_CHINA_MOBILE_3 = 46004,
    UNICOM_MQTT_CHINA_MOBILE_4 = 46007,
    UNICOM_MQTT_CHINA_MOBILE_5 = 46008,
    UNICOM_MQTT_CHINA_MOBILE_6 = 46013,
    UNICOM_MQTT_CHINA_UNICOM_1 = 46001,
    UNICOM_MQTT_CHINA_UNICOM_2 = 46006,
    UNICOM_MQTT_CHINA_UNICOM_3 = 46009,
    UNICOM_MQTT_CHINA_TELECOM_1 = 46003,
    UNICOM_MQTT_CHINA_TELECOM_2 = 46005,
    UNICOM_MQTT_CHINA_TELECOM_3 = 46011,
    UNICOM_MQTT_CHINA_TELECOM_4 = 46012,
    UNICOM_MQTT_CHINA_GUANGDIAN = 46015
}unicom_mqtt_operator;

/**
 * @brief 设置netif
 * 
 * @param client 
 * @param nCid 
 * @return true 
 * @return false 
 */
bool unicom_set_netif(ffw_mqttc_t *client, uint8_t nCid);

/**
 * @brief 获取operator
 * 
 */
int unicom_get_operator();

/**
 * @brief 获取当前cellid
 * 
 * @param nSimID 
 * @return char* 
 */
char* unicom_get_current_cellid(CFW_SIM_ID nSimID);

/**
 * @brief 获取IMEI
 * 
 * @param pImei 
 * @return int32_t 
 */
int32_t unicom_mqtt_get_imei(char **pImei);
#endif