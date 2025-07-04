#include "iotx_dm_internal.h"
#include "cJSON.h"

#ifdef LOG_REPORT_TO_CLOUD
    #include "iotx_log_report.h"
#endif

static dm_client_uri_map_t g_dm_client_uri_map[] = {
#if !defined(DEVICE_MODEL_RAWDATA_SOLO)
    {DM_URI_THING_EVENT_POST_REPLY_WILDCARD,  DM_URI_SYS_PREFIX,         IOTX_DM_DEVICE_ALL, (void *)dm_client_thing_event_post_reply             },
#ifdef DEVICE_MODEL_SHADOW
    {DM_URI_THING_PROPERTY_DESIRED_DELETE_REPLY, DM_URI_SYS_PREFIX,      IOTX_DM_DEVICE_ALL, (void *)dm_client_thing_property_desired_delete_reply},
    {DM_URI_THING_PROPERTY_DESIRED_GET_REPLY, DM_URI_SYS_PREFIX,         IOTX_DM_DEVICE_ALL, (void *)dm_client_thing_property_desired_get_reply   },
    {DM_URI_THING_SERVICE_PROPERTY_GET,       DM_URI_SYS_PREFIX,         IOTX_DM_DEVICE_ALL, (void *)dm_client_thing_service_property_get         },
#endif

#ifdef DEVICE_HISTORY_POST
    {DM_URI_THING_EVENT_HISTORY_POST_REPLY,   DM_URI_SYS_PREFIX,         IOTX_DM_DEVICE_ALL, (void *)dm_client_thing_event_post_reply             },
#endif

    {DM_URI_THING_SERVICE_PROPERTY_SET,       DM_URI_SYS_PREFIX,         IOTX_DM_DEVICE_ALL, (void *)dm_client_thing_service_property_set         },
    {DM_URI_THING_SERVICE_REQUEST_WILDCARD,   DM_URI_SYS_PREFIX,         IOTX_DM_DEVICE_ALL, (void *)dm_client_thing_service_request              },
    {DM_URI_THING_DEVICEINFO_UPDATE_REPLY,    DM_URI_SYS_PREFIX,         IOTX_DM_DEVICE_ALL, (void *)dm_client_thing_deviceinfo_update_reply      },
    {DM_URI_THING_DEVICEINFO_DELETE_REPLY,    DM_URI_SYS_PREFIX,         IOTX_DM_DEVICE_ALL, (void *)dm_client_thing_deviceinfo_delete_reply      },
    {DM_URI_THING_DYNAMICTSL_GET_REPLY,       DM_URI_SYS_PREFIX,         IOTX_DM_DEVICE_ALL, (void *)dm_client_thing_dynamictsl_get_reply         },
    {DM_URI_RRPC_REQUEST_WILDCARD,            DM_URI_SYS_PREFIX,         IOTX_DM_DEVICE_ALL, (void *)dm_client_rrpc_request_wildcard              },
    {DM_URI_NTP_RESPONSE,                     DM_URI_EXT_NTP_PREFIX,     IOTX_DM_DEVICE_ALL, (void *)dm_client_ntp_response                       },
    {NULL,                                    DM_URI_EXT_ERROR_PREFIX,   IOTX_DM_DEVICE_ALL, (void *)dm_client_ext_error                          },
#endif
    {DM_URI_THING_MODEL_DOWN_RAW,             DM_URI_SYS_PREFIX,         IOTX_DM_DEVICE_ALL, (void *)dm_client_thing_model_down_raw               },
    {DM_URI_THING_MODEL_UP_RAW_REPLY,         DM_URI_SYS_PREFIX,         IOTX_DM_DEVICE_ALL, (void *)dm_client_thing_model_up_raw_reply           },
    {DM_URI_CUSTOM_USER_GET,                  DM_URI_CUSTOM_PREFIX,      IOTX_DM_DEVICE_ALL, (void *)dm_client_custom_user_get                      },
#ifdef DEVICE_MODEL_GATEWAY
    {DM_URI_THING_TOPO_ADD_NOTIFY,              DM_URI_SYS_PREFIX,         IOTX_DM_DEVICE_GATEWAY, (void *)dm_client_thing_topo_add_notify              },
    {DM_URI_THING_GATEWAY_PERMIT,               DM_URI_SYS_PREFIX,         IOTX_DM_DEVICE_GATEWAY, (void *)dm_client_thing_gateway_permit               },
    {DM_URI_THING_SUB_REGISTER_REPLY,           DM_URI_SYS_PREFIX,         IOTX_DM_DEVICE_GATEWAY, (void *)dm_client_thing_sub_register_reply           },
    {DM_URI_THING_PROXY_PRODUCT_REGISTER_REPLY, DM_URI_SYS_PREFIX,         IOTX_DM_DEVICE_GATEWAY, (void *)dm_client_thing_proxy_product_register_reply },
    {DM_URI_THING_SUB_UNREGISTER_REPLY,         DM_URI_SYS_PREFIX,         IOTX_DM_DEVICE_GATEWAY, (void *)dm_client_thing_sub_unregister_reply         },
    {DM_URI_THING_TOPO_ADD_REPLY,               DM_URI_SYS_PREFIX,         IOTX_DM_DEVICE_GATEWAY, (void *)dm_client_thing_topo_add_reply               },
    {DM_URI_THING_TOPO_DELETE_REPLY,            DM_URI_SYS_PREFIX,         IOTX_DM_DEVICE_GATEWAY, (void *)dm_client_thing_topo_delete_reply            },
    {DM_URI_THING_TOPO_GET_REPLY,               DM_URI_SYS_PREFIX,         IOTX_DM_DEVICE_GATEWAY, (void *)dm_client_thing_topo_get_reply               },
    {DM_URI_THING_LIST_FOUND_REPLY,             DM_URI_SYS_PREFIX,         IOTX_DM_DEVICE_GATEWAY, (void *)dm_client_thing_list_found_reply             },
    {DM_URI_COMBINE_LOGIN_REPLY,                DM_URI_EXT_SESSION_PREFIX, IOTX_DM_DEVICE_GATEWAY, (void *)dm_client_combine_login_reply                },
    {DM_URI_COMBINE_LOGOUT_REPLY,               DM_URI_EXT_SESSION_PREFIX, IOTX_DM_DEVICE_GATEWAY, (void *)dm_client_combine_logout_reply               },
    {DM_URI_THING_DISABLE,                      DM_URI_SYS_PREFIX,         IOTX_DM_DEVICE_GATEWAY, (void *)dm_client_thing_disable                      },
    {DM_URI_THING_ENABLE,                       DM_URI_SYS_PREFIX,         IOTX_DM_DEVICE_GATEWAY, (void *)dm_client_thing_enable                       },
    {DM_URI_THING_DELETE,                       DM_URI_SYS_PREFIX,         IOTX_DM_DEVICE_GATEWAY, (void *)dm_client_thing_delete                       },
#endif
};

#if !defined(DEVICE_MODEL_RAWDATA_SOLO)
/* property/event post reply filter */
static int _dm_client_subscribe_filter(char *uri, iotx_cm_data_handle_cb cb)
{
    int res = 0;
    int event_post_reply_opt = 0;
    int retry_cnt = IOTX_DM_CLIENT_SUB_RETRY_MAX_COUNTS;

    res = dm_opt_get(DM_OPT_DOWNSTREAM_EVENT_POST_REPLY, &event_post_reply_opt);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    if (event_post_reply_opt == 0) {
        res = dm_client_unsubscribe(uri);
        return res;
    } else {
        res = -1;
        while (res < SUCCESS_RETURN && retry_cnt--) {
            res = dm_client_subscribe(uri, cb, 0);
        }
        return res;
    }
}
#endif /* #if !defined(DEVICE_MODEL_RAWDATA_SOLO) */

int dm_client_subscribe_all(int devid, char product_key[IOTX_PRODUCT_KEY_LEN + 1],
                            char device_name[IOTX_DEVICE_NAME_LEN + 1],
                            int dev_type)
{
    int res = 0, index = 0;
    int number = sizeof(g_dm_client_uri_map) / sizeof(dm_client_uri_map_t);
    char *uri = NULL;
    uint8_t local_sub = 0;

#if !defined(DEVICE_MODEL_RAWDATA_SOLO)
    /* index 0 must be DM_URI_THING_EVENT_POST_REPLY_WILDCARD */
    res = dm_utils_service_name((char *)g_dm_client_uri_map[0].uri_prefix, (char *)g_dm_client_uri_map[0].uri_name,
                                product_key, device_name, &uri);
    if (res == SUCCESS_RETURN) {
        _dm_client_subscribe_filter(uri, (iotx_cm_data_handle_cb)g_dm_client_uri_map[0].callback);
        DM_free(uri);
    }
    index = 1;
#else
    index = 0;
#endif

#ifdef MQTT_AUTO_SUBSCRIBE
    if (devid != 0) {
        iotx_state_event(ITE_STATE_DEV_MODEL, STATE_DEV_MODEL_IN_AUTOSUB_MODE, "subdev subscribe bypass");
        return SUCCESS_RETURN;
    }
#else
    (void)devid;
#endif  /* #ifdef MQTT_AUTO_SUBSCRIBE */

    for (; index < number; index++) {
        if ((g_dm_client_uri_map[index].dev_type & dev_type) == 0) {
            continue;
        }

#ifdef MQTT_AUTO_SUBSCRIBE
        res = dm_utils_service_name((char *)g_dm_client_uri_map[index].uri_prefix, (char *)g_dm_client_uri_map[index].uri_name,
                                    "+", "+", &uri);    /* plus sign wildcards used */
        if (res < SUCCESS_RETURN) {
            continue;
        }

        local_sub = 1;
        res = dm_client_subscribe(uri, (iotx_cm_data_handle_cb)g_dm_client_uri_map[index].callback, &local_sub);
        DM_free(uri);
#else
        res = dm_utils_service_name((char *)g_dm_client_uri_map[index].uri_prefix, (char *)g_dm_client_uri_map[index].uri_name,
                                    product_key, device_name, &uri);
        if (res < SUCCESS_RETURN) {
            continue;
        }

        {
            int retry_cnt = IOTX_DM_CLIENT_SUB_RETRY_MAX_COUNTS;
            local_sub = 0;
            do {
                res = dm_client_subscribe(uri, (iotx_cm_data_handle_cb)g_dm_client_uri_map[index].callback, &local_sub);
            } while (res < SUCCESS_RETURN && --retry_cnt);
            DM_free(uri);
        }
#endif /*  MQTT_AUTO_SUBSCRIBE */
    }

    return SUCCESS_RETURN;
}

static void _dm_client_event_cloud_connected_handle(void)
{
    dm_msg_cloud_connected();
}

static void _dm_client_event_cloud_disconnect_handle(void)
{
    dm_msg_cloud_disconnect();
}

void dm_client_event_handle(int fd, iotx_cm_event_msg_t *event, void *context)
{
    switch (event->type) {
        case IOTX_CM_EVENT_CLOUD_CONNECTED: {
            _dm_client_event_cloud_connected_handle();
        }
        break;
        case IOTX_CM_EVENT_CLOUD_CONNECT_FAILED: {

        }
        break;
        case IOTX_CM_EVENT_CLOUD_DISCONNECT: {
            _dm_client_event_cloud_disconnect_handle();
        }
        break;
        default:
            break;
    }
}

void dm_client_thing_model_down_raw(int fd, const char *topic, const char *payload, unsigned int payload_len,
                                    void *context)
{
    dm_msg_source_t source;

    memset(&source, 0, sizeof(dm_msg_source_t));

    source.uri = topic;
    source.payload = (unsigned char *)payload;
    source.payload_len = payload_len;
    source.context = NULL;

    dm_msg_proc_thing_model_down_raw(&source);
}

void dm_client_thing_model_up_raw_reply(int fd, const char *topic, const char *payload, unsigned int payload_len,
                                        void *context)
{
    dm_msg_source_t source;

    memset(&source, 0, sizeof(dm_msg_source_t));

    source.uri = topic;
    source.payload = (unsigned char *)payload;
    source.payload_len = payload_len;
    source.context = NULL;

    dm_msg_proc_thing_model_up_raw_reply(&source);
}


extern int push_tts_play_list(char *play_text);
int dm_msg_proc_custom_user_get(_IN_ dm_msg_source_t *source)
{
    uint8_t rx_len=0; 
    #define TTS_MAX_DATA 100
    dm_log_info("source->uri :%s\r\n", source->uri);
    // dm_log_info("source->context :%s\r\n", source->context);
    dm_log_info("source->payload :%s\r\n", source->payload);
    dm_log_info("source->payload_len :%d\r\n", source->payload_len);

    cJSON *money_decimal_1 = NULL;
    cJSON *money_1 = NULL;
    cJSON *type_1= NULL;
    cJSON *params_1= NULL;

    char  momey_str[100]={0};
    char  momey_hex_str[100]={0};
    float  decimals_f=0;
    float  integer_f=0;
    float  momey_f=0;
    int   pay_type=0;
    char zfb_buf[]="e694afe4bb98e5ae9de588b0e8b4a6";//支付宝到帐
    char wx_buf[]="e5beaee4bfa1e588b0e8b4a6";//微信到帐
    char  RMB_BUF[]="e58583";//元
    char  tts_buff[TTS_MAX_DATA];


    cJSON *req = cJSON_Parse((const char *)source->payload);
    if (req == NULL || !cJSON_IsObject(req)) {
        return STATE_DEV_MODEL_WRONG_JSON_FORMAT;
    }
    params_1 = cJSON_GetObjectItem(req, "params");

    money_1 = cJSON_GetObjectItem(params_1, "money");
    if (money_1 != NULL && cJSON_IsNumber(money_1)) 
    {
        /* process property PowerSwitch_1 here */
        integer_f=money_1->valueint;
  
  
    }
    money_decimal_1 = cJSON_GetObjectItem(params_1, "money_decimal");
    if (money_decimal_1 != NULL && cJSON_IsNumber(money_decimal_1)) 
    {
        /* process property PowerSwitch_1 here */
    //dm_log_info("money_decimal: %d\r\n", money_decimal_1->valueint);
        decimals_f=money_decimal_1->valueint;
        decimals_f=decimals_f/100;
    }

    type_1 = cJSON_GetObjectItem(params_1, "type");
    if (type_1 != NULL && cJSON_IsNumber(type_1)) 
    {
        /* process property PowerSwitch_1 here */
        pay_type = type_1->valueint;
    }


    momey_f=integer_f+decimals_f;
    memset(momey_str,0,sizeof(momey_str));
    memset(momey_hex_str,0,sizeof(momey_hex_str));
    sprintf(momey_str,"%g",momey_f);
    int i=0;
    for(i=0;i<strlen(momey_str);i++)
    {
        sprintf(momey_hex_str+i*2,"%02x",momey_str[i]);
    }
    dm_log_info("source  momey_str[%s] momey_hex_str[%s]\r\n",momey_str,momey_hex_str);


    memset(tts_buff,0,sizeof(tts_buff));
    if(pay_type==0)//支付宝
    {
        rx_len=strlen(zfb_buf)+strlen(momey_hex_str)+strlen(RMB_BUF)+1;
        HAL_Snprintf(tts_buff,rx_len,"%s%s%s",zfb_buf,momey_hex_str,RMB_BUF);
    }
    else if(pay_type==1)//微信
    {
        rx_len=strlen(wx_buf)+strlen(momey_hex_str)+strlen(RMB_BUF)+1;
        HAL_Snprintf(tts_buff,rx_len,"%s%s%s",wx_buf,momey_hex_str,RMB_BUF);
    }
    else
    {
        rx_len=strlen(momey_hex_str)+strlen(RMB_BUF)+1;
        HAL_Snprintf(tts_buff,rx_len,"%s%s",momey_hex_str,RMB_BUF);
    }
      //将json结构占用的数据空间释放
    cJSON_Delete(req);

    if(rx_len<=TTS_MAX_DATA)
    {
        if(push_tts_play_list(tts_buff)<0)
        {
            dm_log_info("InQueue failed\r\n");
        }
    }
    else
    {
        dm_log_info("Receive data to long max:80 Byte rx_len:%d\r\n",rx_len);
    }  
    return 0;
}

void dm_client_custom_user_get(int fd, const char *topic, const char *payload, unsigned int payload_len,
                                      void *context)
{
    dm_msg_source_t source;

    memset(&source, 0, sizeof(dm_msg_source_t));

    source.uri = topic;
    source.payload = (unsigned char *)payload;
    source.payload_len = payload_len;
    source.context = NULL;
    dm_msg_proc_custom_user_get(&source);
}





#if !defined(DEVICE_MODEL_RAWDATA_SOLO)
void dm_client_thing_service_property_set(int fd, const char *topic, const char *payload, unsigned int payload_len,
        void *context)
{
    int res = 0;
    dm_msg_source_t source;
    dm_msg_dest_t dest;
    dm_msg_request_payload_t request;
    dm_msg_response_t response;
    int prop_set_reply_opt = 0;

    memset(&source, 0, sizeof(dm_msg_source_t));
    memset(&dest, 0, sizeof(dm_msg_dest_t));
    memset(&request, 0, sizeof(dm_msg_request_payload_t));
    memset(&response, 0, sizeof(dm_msg_response_t));

    source.uri = topic;
    source.payload = (unsigned char *)payload;
    source.payload_len = payload_len;
    source.context = NULL;

    dest.uri_name = DM_URI_THING_SERVICE_PROPERTY_SET_REPLY;

    res = dm_msg_proc_thing_service_property_set(&source, &dest, &request, &response);
    if (res < SUCCESS_RETURN) {
        return;
    }

    prop_set_reply_opt = 0;
    res = dm_opt_get(DM_OPT_UPSTREAM_PROPERTY_SET_REPLY, &prop_set_reply_opt);
    if (res == SUCCESS_RETURN) {
        if (prop_set_reply_opt) {
            dm_msg_response(DM_MSG_DEST_CLOUD, &request, &response, "{}", strlen("{}"), NULL);
#ifdef LOG_REPORT_TO_CLOUD
            if (SUCCESS_RETURN == check_target_msg(request.id.value, request.id.value_length)) {
                send_permance_info(request.id.value, request.id.value_length, "2", 1);
            }
#endif
        }
    }
}

#ifdef DEVICE_MODEL_SHADOW
void dm_client_thing_service_property_get(int fd, const char *topic, const char *payload, unsigned int payload_len,
        void *context)
{
    int res = 0;
    dm_msg_source_t source;
    dm_msg_dest_t dest;
    dm_msg_request_payload_t request;
    dm_msg_response_t response;
    unsigned char *data = NULL;
    int data_len = 0;

    memset(&source, 0, sizeof(dm_msg_source_t));
    memset(&dest, 0, sizeof(dm_msg_dest_t));
    memset(&request, 0, sizeof(dm_msg_request_payload_t));
    memset(&response, 0, sizeof(dm_msg_response_t));

    source.uri = topic;
    source.payload = (unsigned char *)payload;
    source.payload_len = payload_len;
    source.context = NULL;

    dest.uri_name = DM_URI_THING_SERVICE_PROPERTY_GET_REPLY;

    res = dm_msg_proc_thing_service_property_get(&source, &dest, &request, &response, &data, &data_len);
    if (res < SUCCESS_RETURN) {
        return;
    }
}
#endif

void dm_client_thing_service_request(int fd, const char *topic, const char *payload, unsigned int payload_len,
                                     void *context)
{
    dm_msg_source_t source;

    memset(&source, 0, sizeof(dm_msg_source_t));

    source.uri = topic;
    source.payload = (unsigned char *)payload;
    source.payload_len = payload_len;
    source.context = NULL;

    dm_msg_proc_thing_service_request(&source);
}

void dm_client_thing_event_post_reply(int fd, const char *topic, const char *payload, unsigned int payload_len,
                                      void *context)
{
    dm_msg_source_t source;

    memset(&source, 0, sizeof(dm_msg_source_t));

    source.uri = topic;
    source.payload = (unsigned char *)payload;
    source.payload_len = payload_len;
    source.context = NULL;

    dm_msg_proc_thing_event_post_reply(&source);
}
#ifdef DEVICE_MODEL_SHADOW
void dm_client_thing_property_desired_get_reply(int fd, const char *topic, const char *payload,
        unsigned int payload_len, void *context)
{
    dm_msg_source_t source;

    memset(&source, 0, sizeof(dm_msg_source_t));

    source.uri = topic;
    source.payload = (unsigned char *)payload;
    source.payload_len = payload_len;
    source.context = NULL;

    dm_msg_proc_thing_property_desired_get_reply(&source);
}

void dm_client_thing_property_desired_delete_reply(int fd, const char *topic, const char *payload,
        unsigned int payload_len, void *context)
{
    dm_msg_source_t source;

    memset(&source, 0, sizeof(dm_msg_source_t));

    source.uri = topic;
    source.payload = (unsigned char *)payload;
    source.payload_len = payload_len;
    source.context = NULL;

    dm_msg_proc_thing_property_desired_delete_reply(&source);
}
#endif

void dm_client_thing_deviceinfo_update_reply(int fd, const char *topic, const char *payload, unsigned int payload_len,
        void *context)
{
    dm_msg_source_t source;

    memset(&source, 0, sizeof(dm_msg_source_t));

    source.uri = topic;
    source.payload = (unsigned char *)payload;
    source.payload_len = payload_len;
    source.context = NULL;

    dm_msg_proc_thing_deviceinfo_update_reply(&source);
}

void dm_client_thing_deviceinfo_delete_reply(int fd, const char *topic, const char *payload, unsigned int payload_len,
        void *context)
{
    dm_msg_source_t source;

    memset(&source, 0, sizeof(dm_msg_source_t));

    source.uri = topic;
    source.payload = (unsigned char *)payload;
    source.payload_len = payload_len;
    source.context = NULL;

    dm_msg_proc_thing_deviceinfo_delete_reply(&source);
}

void dm_client_thing_dynamictsl_get_reply(int fd, const char *topic, const char *payload, unsigned int payload_len,
        void *context)
{
    dm_msg_source_t source;

    memset(&source, 0, sizeof(dm_msg_source_t));

    source.uri = topic;
    source.payload = (unsigned char *)payload;
    source.payload_len = payload_len;
    source.context = NULL;

    dm_msg_proc_thing_dynamictsl_get_reply(&source);
}

void dm_client_rrpc_request_wildcard(int fd, const char *topic, const char *payload, unsigned int payload_len,
                                     void *context)
{
    dm_msg_source_t source;

    memset(&source, 0, sizeof(dm_msg_source_t));

    source.uri = topic;
    source.payload = (unsigned char *)payload;
    source.payload_len = payload_len;
    source.context = NULL;

    dm_msg_proc_rrpc_request(&source);
}

void dm_client_ntp_response(int fd, const char *topic, const char *payload, unsigned int payload_len, void *context)
{
    dm_msg_source_t source;

    memset(&source, 0, sizeof(dm_msg_source_t));

    source.uri = topic;
    source.payload = (unsigned char *)payload;
    source.payload_len = payload_len;
    source.context = NULL;

    dm_disp_ntp_response(&source);
}

void dm_client_ext_error(int fd, const char *topic, const char *payload, unsigned int payload_len, void *context)
{
    dm_msg_source_t source;

    memset(&source, 0, sizeof(dm_msg_source_t));

    source.uri = topic;
    source.payload = (unsigned char *)payload;
    source.payload_len = payload_len;
    source.context = NULL;

    dm_disp_ext_error_response(&source);
}
#endif

#ifdef DEVICE_MODEL_GATEWAY
int dm_client_subdev_unsubscribe(char product_key[IOTX_PRODUCT_KEY_LEN + 1], char device_name[IOTX_DEVICE_NAME_LEN + 1])
{
    int res = 0, index = 0;
    int number = sizeof(g_dm_client_uri_map) / sizeof(dm_client_uri_map_t);
    char *uri = NULL;

    for (index = 0; index < number; index++) {
        if ((g_dm_client_uri_map[index].dev_type & IOTX_DM_DEVICE_SUBDEV) == 0) {
            continue;
        }

        res = dm_utils_service_name((char *)g_dm_client_uri_map[index].uri_prefix, (char *)g_dm_client_uri_map[index].uri_name,
                                    product_key, device_name, &uri);
        if (res < SUCCESS_RETURN) {
            index--;
            continue;
        }

        dm_client_unsubscribe(uri);
        DM_free(uri);
    }

    return SUCCESS_RETURN;
}

void dm_client_thing_topo_add_notify(int fd, const char *topic, const char *payload, unsigned int payload_len,
                                     void *context)
{
    int res = 0;
    dm_msg_source_t source;
    dm_msg_dest_t dest;
    dm_msg_request_payload_t request;
    dm_msg_response_t response;

    memset(&source, 0, sizeof(dm_msg_source_t));
    memset(&dest, 0, sizeof(dm_msg_dest_t));
    memset(&request, 0, sizeof(dm_msg_request_payload_t));
    memset(&response, 0, sizeof(dm_msg_response_t));

    source.uri = topic;
    source.payload = (unsigned char *)payload;
    source.payload_len = payload_len;
    source.context = NULL;

    dest.uri_name = DM_URI_THING_TOPO_ADD_NOTIFY_REPLY;

    res = dm_msg_proc_thing_topo_add_notify(&source, &dest, &request, &response);
    if (res < SUCCESS_RETURN) {
        return;
    }

    dm_msg_response(DM_MSG_DEST_CLOUD, &request, &response, "{}", strlen("{}"), NULL);
}

void dm_client_thing_disable(int fd, const char *topic, const char *payload, unsigned int payload_len, void *context)
{
    int res = 0;
    dm_msg_source_t source;
    dm_msg_dest_t dest;
    dm_msg_request_payload_t request;
    dm_msg_response_t response;

    memset(&source, 0, sizeof(dm_msg_source_t));
    memset(&dest, 0, sizeof(dm_msg_dest_t));
    memset(&request, 0, sizeof(dm_msg_request_payload_t));
    memset(&response, 0, sizeof(dm_msg_response_t));

    source.uri = topic;
    source.payload = (unsigned char *)payload;
    source.payload_len = payload_len;
    source.context = NULL;

    dest.uri_name = DM_URI_THING_DISABLE_REPLY;

    res = dm_msg_proc_thing_disable(&source, &dest, &request, &response);
    if (res < SUCCESS_RETURN) {
        return;
    }

    dm_msg_response(DM_MSG_DEST_CLOUD, &request, &response, "{}", strlen("{}"), NULL);
}

void dm_client_thing_enable(int fd, const char *topic, const char *payload, unsigned int payload_len, void *context)
{
    int res = 0;
    dm_msg_source_t source;
    dm_msg_dest_t dest;
    dm_msg_request_payload_t request;
    dm_msg_response_t response;

    memset(&source, 0, sizeof(dm_msg_source_t));
    memset(&dest, 0, sizeof(dm_msg_dest_t));
    memset(&request, 0, sizeof(dm_msg_request_payload_t));
    memset(&response, 0, sizeof(dm_msg_response_t));

    source.uri = topic;
    source.payload = (unsigned char *)payload;
    source.payload_len = payload_len;
    source.context = NULL;

    dest.uri_name = DM_URI_THING_ENABLE_REPLY;

    res = dm_msg_proc_thing_enable(&source, &dest, &request, &response);
    if (res < SUCCESS_RETURN) {
        return;
    }

    dm_msg_response(DM_MSG_DEST_CLOUD, &request, &response, "{}", strlen("{}"), NULL);
}

void dm_client_thing_delete(int fd, const char *topic, const char *payload, unsigned int payload_len, void *context)
{
    int res = 0;
    dm_msg_source_t source;
    dm_msg_dest_t dest;
    dm_msg_request_payload_t request;
    dm_msg_response_t response;

    memset(&source, 0, sizeof(dm_msg_source_t));
    memset(&dest, 0, sizeof(dm_msg_dest_t));
    memset(&request, 0, sizeof(dm_msg_request_payload_t));
    memset(&response, 0, sizeof(dm_msg_response_t));

    source.uri = topic;
    source.payload = (unsigned char *)payload;
    source.payload_len = payload_len;
    source.context = NULL;

    dest.uri_name = DM_URI_THING_DELETE_REPLY;

    res = dm_msg_proc_thing_delete(&source, &dest, &request, &response);
    if (res < SUCCESS_RETURN) {
        return;
    }

    dm_msg_response(DM_MSG_DEST_CLOUD, &request, &response, "{}", strlen("{}"), NULL);
}

void dm_client_thing_gateway_permit(int fd, const char *topic, const char *payload, unsigned int payload_len,
                                    void *context)
{
    int res = 0;
    dm_msg_source_t source;
    dm_msg_dest_t dest;
    dm_msg_request_payload_t request;
    dm_msg_response_t response;

    memset(&source, 0, sizeof(dm_msg_source_t));
    memset(&dest, 0, sizeof(dm_msg_dest_t));
    memset(&request, 0, sizeof(dm_msg_request_payload_t));
    memset(&response, 0, sizeof(dm_msg_response_t));

    source.uri = topic;
    source.payload = (unsigned char *)payload;
    source.payload_len = payload_len;
    source.context = NULL;

    dest.uri_name = DM_URI_THING_GATEWAY_PERMIT_REPLY;

    res = dm_msg_proc_thing_gateway_permit(&source, &dest, &request, &response);
    if (res < SUCCESS_RETURN) {
        return;
    }

    dm_msg_response(DM_MSG_DEST_CLOUD, &request, &response, "{}", strlen("{}"), NULL);
}

void dm_client_thing_sub_register_reply(int fd, const char *topic, const char *payload, unsigned int payload_len,
                                        void *context)
{
    dm_msg_source_t source;

    memset(&source, 0, sizeof(dm_msg_source_t));

    source.uri = topic;
    source.payload = (unsigned char *)payload;
    source.payload_len = payload_len;
    source.context = NULL;

    dm_msg_proc_thing_sub_register_reply(&source);
}

void dm_client_thing_proxy_product_register_reply(int fd, const char *topic, const char *payload,
        unsigned int payload_len,
        void *context)
{
    dm_msg_source_t source;

    memset(&source, 0, sizeof(dm_msg_source_t));

    source.uri = topic;
    source.payload = (unsigned char *)payload;
    source.payload_len = payload_len;
    source.context = NULL;

    dm_msg_proc_thing_proxy_product_register_reply(&source);
}

void dm_client_thing_sub_unregister_reply(int fd, const char *topic, const char *payload, unsigned int payload_len,
        void *context)
{
    dm_msg_source_t source;

    memset(&source, 0, sizeof(dm_msg_source_t));

    source.uri = topic;
    source.payload = (unsigned char *)payload;
    source.payload_len = payload_len;
    source.context = NULL;

    dm_msg_proc_thing_sub_unregister_reply(&source);
}

void dm_client_thing_topo_add_reply(int fd, const char *topic, const char *payload, unsigned int payload_len,
                                    void *context)
{
    dm_msg_source_t source;

    memset(&source, 0, sizeof(dm_msg_source_t));

    source.uri = topic;
    source.payload = (unsigned char *)payload;
    source.payload_len = payload_len;
    source.context = NULL;

    dm_msg_proc_thing_topo_add_reply(&source);
}

void dm_client_thing_topo_delete_reply(int fd, const char *topic, const char *payload, unsigned int payload_len,
                                       void *context)
{
    dm_msg_source_t source;

    memset(&source, 0, sizeof(dm_msg_source_t));

    source.uri = topic;
    source.payload = (unsigned char *)payload;
    source.payload_len = payload_len;
    source.context = NULL;

    dm_msg_proc_thing_topo_delete_reply(&source);
}

void dm_client_thing_topo_get_reply(int fd, const char *topic, const char *payload, unsigned int payload_len,
                                    void *context)
{
    dm_msg_source_t source;

    memset(&source, 0, sizeof(dm_msg_source_t));

    source.uri = topic;
    source.payload = (unsigned char *)payload;
    source.payload_len = payload_len;
    source.context = NULL;

    dm_msg_proc_thing_topo_get_reply(&source);
}

void dm_client_thing_list_found_reply(int fd, const char *topic, const char *payload, unsigned int payload_len,
                                      void *context)
{
    dm_msg_source_t source;

    memset(&source, 0, sizeof(dm_msg_source_t));

    source.uri = topic;
    source.payload = (unsigned char *)payload;
    source.payload_len = payload_len;
    source.context = NULL;

    dm_msg_proc_thing_list_found_reply(&source);
}

void dm_client_combine_login_reply(int fd, const char *topic, const char *payload, unsigned int payload_len,
                                   void *context)
{
    dm_msg_source_t source;

    memset(&source, 0, sizeof(dm_msg_source_t));

    source.uri = topic;
    source.payload = (unsigned char *)payload;
    source.payload_len = payload_len;
    source.context = NULL;

    dm_msg_proc_combine_login_reply(&source);
}

void dm_client_combine_logout_reply(int fd, const char *topic, const char *payload, unsigned int payload_len,
                                    void *context)
{
    dm_msg_source_t source;

    memset(&source, 0, sizeof(dm_msg_source_t));

    source.uri = topic;
    source.payload = (unsigned char *)payload;
    source.payload_len = payload_len;
    source.context = NULL;

    dm_msg_proc_combine_logout_reply(&source);
}
#endif
