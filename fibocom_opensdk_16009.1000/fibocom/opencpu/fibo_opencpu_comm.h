#ifndef COMPONENTS_FIBOCOM_OPENCPU_FIBO_OPENCPU_COMM_H
#define COMPONENTS_FIBOCOM_OPENCPU_FIBO_OPENCPU_COMM_H

#include <stdbool.h>
#include <stdint.h>
#include <osi_log.h>

typedef int INT32;
typedef unsigned int UINT32;
typedef char INT8;
typedef unsigned char UINT8;
typedef unsigned short UINT16;
typedef short INT16;
typedef bool BOOL;
typedef unsigned long long  UINT64;
#define SAPP_IO_ID_T uint16_t

typedef void (*fota_download_cb_t)(UINT32 size, UINT32 total);

/**
 * @brief User callback for at response
 * 
 */
typedef void (*fibo_at_resp_t)(UINT8 *rsp, UINT16 rsplen);
typedef enum
{
    GAPP_SIG_PDP_RELEASE_IND = 0,
    GAPP_SIG_PDP_ACTIVE_IND = 10,
    GAPP_SIG_PDP_ACTIVE_OR_DEACTIVE_FAIL_IND = 11,
    GAPP_SIG_PDP_DEACTIVE_ABNORMALLY_IND = 12,
    GAPP_SIG_LWM2M_TIANYI_REG_SUCCESS = 16,
    GAPP_SIG_LWM2M_TIANYI_OBSERVE_SUCCESS = 17,
    GAPP_SIG_LWM2M_TIANYI_UPDATE_SUCCESS = 18,
    GAPP_SIG_LWM2M_TIANYI_DEREG_SUCCESS = 19,
    GAPP_SIG_LWM2M_TIANYI_REG_FAIL = 20,
    GAPP_SIG_LWM2M_TIANYI_NOTIFY_SUCCESS = 21,
    GAPP_SIG_LWM2M_TIANYI_RCV_DATA = 22,
    GAPP_SIG_LWM2M_TIANYI_NOTIFY_FAIL = 23,
    GAPP_SIG_ONENET_EVENT_BASE = 30,
    GAPP_SIG_ONENET_EVENT_BOOTSTRAP_START = 31,
    GAPP_SIG_ONENET_EVENT_BOOTSTRAP_SUCCESS = 32,
    GAPP_SIG_ONENET_EVENT_BOOTSTRAP_FAILED = 33,
    GAPP_SIG_ONENET_EVENT_CONNECT_SUCCESS = 34,
    GAPP_SIG_ONENET_EVENT_CONNECT_FAILED = 35,
    GAPP_SIG_ONENET_EVENT_REG_SUCCESS = 36,
    GAPP_SIG_ONENET_EVENT_REG_FAILED = 37,
    GAPP_SIG_ONENET_EVENT_REG_TIMEOUT = 38,
    GAPP_SIG_ONENET_EVENT_LIFETIME_TIMEOUT = 39,
    GAPP_SIG_ONENET_EVENT_STATUS_HALT = 40,
    GAPP_SIG_ONENET_EVENT_UPDATE_SUCCESS = 41,
    GAPP_SIG_ONENET_EVENT_UPDATE_FAILED = 42,
    GAPP_SIG_ONENET_EVENT_UPDATE_TIMEOUT = 43,
    GAPP_SIG_ONENET_EVENT_UPDATE_NEED = 44,
    GAPP_SIG_ONENET_EVENT_UNREG_DONE = 45,
    GAPP_SIG_ONENET_EVENT_RESPONSE_FAILED = 50,
    GAPP_SIG_ONENET_EVENT_RESPONSE_SUCCESS = 51,
    GAPP_SIG_ONENET_EVENT_NOTIFY_FAILED = 55,
    GAPP_SIG_ONENET_EVENT_NOTIFY_SUCCESS = 56,
    GAPP_SIG_ONENET_EVENT_OBSERVER_REQUEST = 57,
    GAPP_SIG_ONENET_EVENT_DISCOVER_REQUEST = 58,
    GAPP_SIG_ONENET_EVENT_READ_REQUEST = 59,
    GAPP_SIG_ONENET_EVENT_WRITE_REQUEST = 60,
    GAPP_SIG_ONENET_EVENT_EXECUTE_REQUEST = 61,
    GAPP_SIG_ONENET_EVENT_SET_PARAM_REQUEST = 62,
    GAPP_SIG_CONNECT_RSP = 63,
    GAPP_SIG_CLOSE_RSP = 64,
    GAPP_SIG_SUB_RSP = 65,
    GAPP_SIG_UNSUB_RSP = 66,
    GAPP_SIG_PUB_RSP = 67,
    GAPP_SIG_INCOMING_DATA_RSP = 68,
    GAPP_SIG_PDP_ACTIVE_ADDRESS = 69,
    GAPP_SIG_DNS_QUERY_IP_ADDRESS = 70,
    GAPP_SIG_MPING_RECV = 71,
    GAPP_SIG_MPINGSTAT_RECV = 72,
    GAPP_SIG_TTS_END = 73,
    GAPP_SIG_OPENCPU_API_TEST = 200,
    GAPP_SIG_REPORT_URC_TEXT = 201,
    GAPP_SIG_REPORT_SIM1_HOTPLUG=202,
    GAPP_SIG_BT_ON_IND = 300,
    GAPP_SIG_BT_OFF_IND,
    GAPP_SIG_BT_VISIBILE_IND,
    GAPP_SIG_BT_HIDDEN_IND,
    GAPP_SIG_BT_SET_LOCAL_NAME_IND,
    GAPP_SIG_BT_SET_LOCAL_ADDR_IND,
    GAPP_SIG_BT_SCAN_DEV_REPORT,
    GAPP_SIG_BT_SCAN_COMP_IND,
    GAPP_SIG_BT_INQUIRY_CANCEL_IND,
    GAPP_SIG_BT_PAIR_COMPLETE_IND,
    GAPP_SIG_BT_DELETE_DEV_IND,
    GAPP_SIG_BT_SSP_NUM_IND,
    GAPP_SIG_BT_CONNECT_IND,
    GAPP_SIG_BT_DISCONNECT_IND,
    GAPP_SIG_BT_DATA_RECIEVE_IND,
    GAPP_SIG_BLE_SET_ADDR_IND = 350,
    GAPP_SIG_BLE_ADD_CLEAR_WHITELIST_IND,
    GAPP_SIG_BLE_CONNECT_IND,
    GAPP_SIG_BLE_SET_ADV_IND,
    GAPP_SIG_BLE_SET_SCAN_IND,
    GAPP_SIG_BLE_SCAN_REPORT,
    GAPP_SIG_AUDIO_LIST_PLAY_END,
    GAPP_SIG_FTPOPEN_SUCCESS,
    GAPP_SIG_FTPOPEN_FAIL,
    GAPP_SIG_FTPWRITE_SUCCESS,
    GAPP_SIG_FTPWRITE_FAIL,
    GAPP_SIG_FTPREAD_SUCCESS,
    GAPP_SIG_FTPREAD_FALI,
    GAPP_SIG_FTPCLOSE_SUCCESS,
    GAPP_SIG_FTPCLOSE_FAIL,
    GAPP_SIG_BLE_REMOTE_MTU_IND,
    GAPP_SIG_AUDREC_END,
    GAPP_SIG_KEYPAD_VALUE,
    GAPP_SIG_CONNECT_RSP1,
    GAPP_SIG_CLOSE_RSP1,
    GAPP_SIG_SUB_RSP1,
    GAPP_SIG_UNSUB_RSP1,
    GAPP_SIG_PUB_RSP1,
    GAPP_SIG_INCOMING_DATA_RSP1,
    GAPP_SIG_GNSSOPEN_SUCCESS,
    GAPP_SIG_REG_STATUS_IND,
    GAPP_SIG_PWRKEY_VALUE,
    GAPP_SIG_HDBD_CMD,
    GAPP_SIG_HTTP_REQUEST_END,
    GAPP_SIG_HTTP_REQUEST_ERROR,
    GAPP_SIG_HTTP_REQUEST_RES,
    GAPP_SIG_HTTP_REQUEST_CONNECT,
    GAPP_SIG_MQTT_CLOUD_AUTH_ALI,
    GAPP_SIG_MQTT_CLOUD_AUTH_TENCENT,
    GAPP_SIG_MQTT_CLOUD_AUTH_HUAWEI,
    GAPP_SIG_MQTT_CLOUD_AUTH_TUYA,
    GAPP_SIG_MQTT_CLOUD_AUTH_ONENET,
    GAPP_SIG_MQTT_CLOUD_AUTH_BAIDU,
    GAPP_SIG_MQTT_CLOUD_AUTH_AEP,
    GAPP_SIG_MQTT_CLOUD_TUYA_EVENT,
    GAPP_SIG_MQTT_CLOUD_DYN_ALI,
    GAPP_SIG_MQTT_CLOUD_DYN_TENCENT,
    GAPP_SIG_RRC_STATUS_REPORT,
    GAPP_SIG_RTC_ALARM,
    GAPP_SIG_AUDIO_PLAY_ERROR,
    GAPP_SIG_CC_RING_IND, //Incoming call
    GAPP_SIG_CC_CONNECT_IND,//call setup
    GAPP_SIG_CC_ERROR_IND,//call abnormality
    GAPP_SIG_CC_NOCARRIER_IND,//disconnct call
    GAPP_SIG_BLE_DISCONNECT_IND,
    GAPP_SIG_BLE_READ_REQ_IND = 400,
    GAPP_SIG_BLE_RECV_DATA_IND,
    GAPP_SIG_BLE_UPDATA_SUCCESS,
    GAPP_SIG_BLE_UPDATA_FAILED,
    GAPP_SIG_REG_REJECT_IND,
    GAPP_SIG_CDRX_NWSUPPORT_IND,
    GAPP_SIG_PSM_STATUS_IND,
    GAPP_SIG_NW_SIGNAL_QUALITY_IND,
    GAPP_SIG_MAX,
} GAPP_SIGNAL_ID_T;

typedef void (*fibo_signal_t)(GAPP_SIGNAL_ID_T sig, va_list arg);
typedef struct FIBO_CALLBACK_S FIBO_CALLBACK_T;

struct FIBO_CALLBACK_S
{
    fibo_at_resp_t at_resp;
    fibo_signal_t fibo_signal;
};

#define FIBO_UNUSED(v) (void)(v)

#define FIBO_LOG(format, ...)                                                  \
    do                                                                         \
    {                                                                          \
        OSI_PRINTFI("[%s:%d]-" format, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    } while (0)

#define FIBO_CHECK(condition, err_code, format, ...) \
    do                                          \
    {                                           \
        if (condition)                          \
        {                                       \
            FIBO_LOG(format, ##__VA_ARGS__);    \
            ret = err_code;                     \
            goto error;                         \
        }                                       \
    } while (0);
#ifndef MIN
#define MIN(a, b)              \
    ({                         \
        typeof(a) __a = (a);   \
        typeof(b) __b = (b);   \
        __a > __b ? __b : __a; \
    })
#endif
#ifndef MAX
#define MAX(a, b)              \
    ({                         \
        typeof(a) __a = (a);   \
        typeof(b) __b = (b);   \
        __a > __b ? __a : __b; \
    })
#endif



enum fibo_result_s
{
    FIBO_R_FAILED = -1,
    FIBO_R_SUCCESS = 0,
};

typedef enum fibo_result_s fibo_result_t;


extern FIBO_CALLBACK_T *g_user_callback;
void gapp_dispatch(GAPP_SIGNAL_ID_T sig, ...);


#endif /* COMPONENTS_FIBOCOM_OPENCPU_FIBO_OPENCPU_COMM_H */
