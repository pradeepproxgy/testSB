#ifndef OPENCPU_API_OC_MQTT_CLOUD
#define OPENCPU_API_OC_MQTT_CLOUD

typedef enum  fibo_tuya_action {
    FIBO_MQTT_TUYA_STOP = 0,
    FIBO_MQTT_TUYA_START = 1
} fibo_tuya_action_t;

typedef enum fibo_mqtt_tuya_event
{
    FIBO_TUYA_RESET = 1,
    FIBO_TUYA_BIND_START,
    FIBO_TUYA_BIND_TOKEN_ON,
    FIBO_TUYA_ACTIVATE_SUCCESSED,
    FIBO_TUYA_MQTT_CONNECTED,
    FIBO_TUYA_MQTT_DISCONNECT,
    FIBO_TUYA_BIND_STOP,
    FIBO_TUYA_DP_RECEIVE,
    FIBO_TUYA_DP_RECEIVE_CJSON,
    FIBO_TUYA_UPGRADE_NOTIFY,
    FIBO_TUYA_RESET_COMPLETE,
    FIBO_TUYA_TIMESTAMP_SYNC,
}fibo_mqtt_tuya_event_t;


typedef enum{
    FIBO_TUYA_OTA_EVENT_START,
    FIBO_TUYA_OTA_EVENT_ON_DATA,
    FIBO_TUYA_OTA_EVENT_FINISH,
} fibo_tuya_ota_event_id_t;

typedef struct {
    fibo_tuya_ota_event_id_t id;
    void* data;
    size_t data_len;
    size_t offset;
    size_t file_size;
    void* user_data;
} fibo_tuya_ota_event_t;

typedef void (*fibo_tuya_ota_event_cb_t)(fibo_tuya_ota_event_t* event);

typedef struct {
    fibo_tuya_ota_event_cb_t event_cb;
    size_t range_size;
    uint32_t timeout_ms;
    void* user_data;
} fibo_tuya_ota_config_t;

/**
 * @brief Struct to hold Tuya IoT configuration parameters.
 * 
 * This struct is used to store the configuration parameters required for connecting to the Tuya IoT platform.
 *
 * @param software_ver The software version of the device.
 * @param modules The list of supported modules for the device.
 * @param prepublish A boolean flag indicating whether the device is in pre-evn mode.
 */
typedef struct {
    const char* software_ver;
    const char* modules;
    bool prepublish;
} fibo_tuya_iot_config_t;

/**
 * @brief Config aliyun info, get mqtt connect prarm.
 * 
 * @param product_key ProductKey get from aliyun
 * @param device_name DeviceName get from aliyun
 * @param device_secret DeviceSecret get from aliyun
 * @param tls_enable enable tls or not
 * @return INT32 
 */
INT32 fibo_mqtt_aliyun_config(const uint8_t* product_key, const uint8_t* device_name, const uint8_t* device_secret, bool tls_enable);
INT32 fibo_mqtt_tecent_config(const uint8_t* product_id, const uint8_t* device_name, const uint8_t* device_secret);
INT32 fibo_mqtt_huawei_config(const uint8_t* device_name, const uint8_t* device_secret);
INT32 fibo_mqtt_onenet_config(const uint8_t *productid, const uint8_t *devicename, const uint8_t *accessKey);
INT32 fibo_mqtt_baidu_config(const uint8_t *iotcoreid, const uint8_t *devicekey,const uint8_t *devicesecret);
INT32 fibo_tuya_init(const fibo_tuya_iot_config_t *config);
INT32 fibo_mqtt_tuya_config(int32_t actiontype, const uint8_t *productkey, const uint8_t *uuid, const uint8_t *authkey);
INT32 fibo_mqtt_tuya_bind_info_del(void);
INT32 fibo_mqtt_aliyun_dynamic(const uint8_t* product_key, const uint8_t* product_secret, const uint8_t* device_name, const uint8_t* host, int32_t port);
INT32 fibo_mqtt_tecent_dynamic(const uint8_t* productId, const uint8_t* deviceName, const uint8_t* productSecret, const uint8_t*url);
INT32 fibo_mqtt_aep_config(const uint8_t *productid, const uint8_t *devicenumber,const uint8_t *token,const uint8_t *aepusername);
INT32 fibo_tuya_extension_modules_version_update(const char *modules);
INT32 fibo_tuya_version_update_sync(void);
INT32 fibo_tuya_mqtt_report_json(const char *dps);
INT32 fibo_tuya_mqtt_report_json_with_time(const char* dps, const char* time);
INT32 fibo_tuya_reset();
INT32 fibo_tuya_activated_data_remove();
INT32 fibo_tuya_ota_init(const fibo_tuya_ota_config_t* config);
INT32 fibo_tuya_ota_begin(void* upgrade);
INT32 fibo_tuya_ota_upgrade_status_report(int status);
INT32 fibo_tuya_ota_upgrade_progress_report(int percent);
INT32 fibo_tuya_ota_channel_set(uint8_t channel);

#endif