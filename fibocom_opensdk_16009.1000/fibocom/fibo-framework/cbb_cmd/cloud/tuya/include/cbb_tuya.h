#ifndef __CBB_TUYA_H__
#define __CBB_TUYA_H__

#include <stdint.h>
#include <stdbool.h>

#include "ffw_mqttc.h"

typedef enum  cbb_tuya_action {
    CBB_MQTT_TUYA_STOP = 0,
    CBB_MQTT_TUYA_START = 1
} cbb_tuya_action_t;

typedef enum cbb_mqtt_tuya_event
{
    CBB_TUYA_RESET = 1,
    CBB_TUYA_BIND_START,
    CBB_TUYA_BIND_TOKEN_ON,
    CBB_TUYA_ACTIVATE_SUCCESSED,
    CBB_TUYA_MQTT_CONNECTED,
    CBB_TUYA_MQTT_DISCONNECT,
    CBB_TUYA_BIND_STOP,
    CBB_TUYA_DP_RECEIVE,
    CBB_TUYA_DP_RECEIVE_CJSON,
    CBB_TUYA_UPGRADE_NOTIFY,
    CBB_TUYA_RESET_COMPLETE,
    CBB_TUYA_TIMESTAMP_SYNC,
}cbb_mqtt_tuya_event_t;

typedef enum{
    CBB_TUYA_OTA_EVENT_START,
    CBB_TUYA_OTA_EVENT_ON_DATA,
    CBB_TUYA_OTA_EVENT_FINISH,
} cbb_tuya_ota_event_id_t;

typedef struct {
    cbb_tuya_ota_event_id_t id;
    void* data;
    size_t data_len;
    size_t offset;
    size_t file_size;
    void* user_data;
} cbb_tuya_ota_event_t;

typedef void (*cbb_tuya_ota_event_cb_t)(cbb_tuya_ota_event_t* event);

typedef struct {
    cbb_tuya_ota_event_cb_t event_cb;
    size_t range_size;
    uint32_t timeout_ms;
    void* user_data;
} cbb_tuya_ota_config_t;

typedef struct {
    const char* software_ver;
    const char* modules;
    bool prepublish;
} cbb_tuya_iot_config_t;

/**
 * @brief Delete tuya current bind info. Please use with caution.
 * 
 * @return int32_t FFW_R_SUCCESS is success, other is fail.
 */
int32_t cbb_tuya_mqtt_bind_info_delete(void);

int32_t cbb_tuya_iot_init(const cbb_tuya_iot_config_t *config);

int32_t cbb_cmd_mqttauth_tuya_set(
    int32_t actiontype,
    const uint8_t *productkey,
    const uint8_t *uuid,
    const uint8_t *authkey,
    void *event_handler,
    void *engine);

int32_t cbb_cmd_mqttauth_tuya_read(void *engine);

int cbb_tuya_mqtt_message_parse(
    const uint8_t* payload, size_t payload_len, char **data, uint16_t *data_len);

int cbb_tuya_mqtt_message_create(const char* data, int data_len, char **payload, int *payload_len);

int cbb_tuya_mqtt_enable();

int cbb_tuya_modules_version_update(const char *modules);

int cbb_tuya_version_update_sync(void);

const char* cbb_tuya_devid_get(void);

const char* cbb_tuya_localkey_get(void);

const char* cbb_tuya_seckey_get(void);

const char* cbb_tuya_timezone_get(void);

int cbb_tuya_mqtt_report_json(const char *dps);

int cbb_tuya_mqtt_menual_enable(ffw_mqttc_t *mqtt, const char *hostname, uint16_t port, const char *clientid);

int cbb_tuya_mqtt_report_json_with_time(const char* dps, const char* time);

int cbb_tuya_reset();

int cbb_tuya_activated_data_remove();

int cbb_tuya_ota_init(const cbb_tuya_ota_config_t* config);

int cbb_tuya_ota_begin(void* upgrade);

int cbb_tuya_ota_upgrade_status_report(int status);

int cbb_tuya_ota_upgrade_progress_report(int percent);

int cbb_tuya_ota_channel_set(uint8_t channel);

#endif
