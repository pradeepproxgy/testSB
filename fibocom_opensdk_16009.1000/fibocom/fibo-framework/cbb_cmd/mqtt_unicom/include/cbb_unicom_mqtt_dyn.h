#ifndef CBB_UNICOM_MQTT_DYN_H
#define CBB_UNICOM_MQTT_DYN_H

typedef enum
{
    UNICOM_MQTT_DYN_EVENT_CONNECT_SUCCESS,
    UNICOM_MQTT_DYN_EVENT_CONNECT_FAILED,
    UNICOM_MQTT_DYN_EVENT_DISCONNECTED,
    UNICOM_MQTT_DYN_EVENT_DISCONNECT,
    UNICOM_MQTT_DYN_EVENT_PARAM_SUCCESS,
    UNICOM_MQTT_DYN_EVENT_PARAM_FAILED
}unicom_mqtt_dyn_event;

/**
 * @brief 动态注册回调
 * 
 */
typedef void (*unicom_mqtt_dynamic_event_callback)(int event, void *param);

/**
 * @brief 联通平台动态注册处理，一模一密、一型一密、一型一密免予注册
 * 
 * @param mqtt_connect 联通雁飞mqtt数据结构
 * @param id 连接id，目前支持AT和自注册
 * @param cid 指定CID配置
 * @param cb 事件响应回调
 * @param arg 事件响应参数
 * @return int32_t 
 */
int32_t unicom_mqtt_connect_dynamic(unicom_mqtt_connect_t *mqtt_connect, int id, int cid, unicom_mqtt_dynamic_event_callback cb, void *arg);
#endif
