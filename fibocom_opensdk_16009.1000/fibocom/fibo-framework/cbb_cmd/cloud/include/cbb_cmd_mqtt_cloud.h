#ifndef __CBB_CMD_MQTT_CLOUD_H__
#define __CBB_CMD_MQTT_CLOUD_H__

#include "tuya_iot.h"
#include "cbb_tuya.h"

#define MQTT_CLOUD_INFO_NULL ""
#define MQTT_CLOUD_WAIT_CONNECT_TIMEOUT 10*1000
#define MQTT_CLOUD_WAIT_RESULT_TIMEOUT 10*1000

enum cbb_mqtt_cloud_type
{
    CBB_MQTT_TYPE_ALI_CLOUD = 1,
    CBB_MQTT_TYPE_TENCENT_CLOUD,
    CBB_MQTT_TYPE_HUAWEI_CLOUD,
    CBB_MQTT_TYPE_TUYA_CLOUD,
    CBB_MQTT_TYPE_ONENET_CLOUD,
    CBB_MQTT_TYPE_BAIDU_CLOUD,
    CBB_MQTT_TYPE_AMAZON_CLOUD,
    CBB_MQTT_TYPE_AEP_CLOUD,
    CBB_MQTT_TYPE_MAX
};

enum cbb_mqtt_connect_status
{
    CBB_MQTT_CLOUD_CONNECT_SUCCESS = 1,
    CBB_MQTT_CLOUD_CONNECT_FAILED
};

typedef struct {
    uint8_t type;
} cbb_mqtt_cloud_type_t;

typedef struct {
    uint8_t type;
    uint8_t *host;
    uint16_t port;
    uint8_t *username;
    uint8_t *password;
    uint8_t *clientid;
} cbb_mqtt_auth_res_t;

typedef struct {
    uint8_t type;
    uint8_t *product_key; 
    uint8_t *device_name;
    uint8_t *device_secret;
    void    *engine;
} cbb_mqtt_dynamic_ali_t;

typedef struct {
    uint8_t type;
    int32_t decrypt_type;
    uint8_t *psk;
    void *engine;
} cbb_mqtt_dynamic_tecent_t;

typedef void (*cbb_cmd_mqtt_cloud_auth_callback)(cbb_mqtt_auth_res_t *result);
typedef void (*cbb_cmd_mqtt_cloud_auth_dyn_callback)(cbb_mqtt_cloud_type_t *result);
typedef void (*cbb_cmd_tuya_event_callback)(cbb_mqtt_tuya_event_t result, void *arg);
typedef void (*cbb_cmd_mqtt_cloud_event_callback)(void *engine, uint8_t type, uint8_t status);

void cbb_cmd_mqtt_auth_cb_register(cbb_cmd_mqtt_cloud_auth_callback callback);
cbb_cmd_mqtt_cloud_auth_callback get_cbb_cmd_mqtt_auth_cb();
void cbb_cmd_mqtt_auth_dyn_cb_register(cbb_cmd_mqtt_cloud_auth_dyn_callback callback);
cbb_cmd_mqtt_cloud_auth_dyn_callback get_cbb_cmd_mqtt_auth_dyn_cb();

int aliyun_calculate_MQTT_param(const char* product_key, const char* device_name, const char* device_secret, bool tls_enable, void *engine);
int32_t baidu_calculate_MQTT_param(const uint8_t *iotcoreid, const uint8_t *devicekey,const uint8_t *devicesecret, void *engine);
int32_t baidu_MQTT_param_read(uint8_t **iotcoreid, uint8_t **devicekey, uint8_t **devicesecret);
int32_t huawei_calculate_MQTT_param(const char* device_name, const char* device_secret, char ** clientid, char **username, char **password);
int32_t onenet_calculate_MQTT_param(const uint8_t *productid, const uint8_t *devicename,const uint8_t *accessKey,void* engine);
int32_t onenet_MQTT_param_read(uint8_t **productid, uint8_t **devicename,uint8_t **accessKey);
int tecent_calculate_MQTT_param(const char* product_id, const char* device_name, const char* device_secret, void *engine);
int MQTT_Dynamic_Register(const char* product_key, const char* product_secret, const char* device_name, const char* host, int port, void* engine , void *connect_cb);
int Dynamic_Register(const char* productId, const char* deviceName, const char* productSecret, const char* url, void* engine);

#endif
