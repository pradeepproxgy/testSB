#include "osi_log.h"
#include "osi_api.h"
#include "fibo_opencpu.h"
#include "oc_sms.h"

static void sig_res_callback(GAPP_SIGNAL_ID_T sig, va_list arg)
{
    fibo_textTrace("sig_res_callback  sig = %d", sig);
}

static void at_res_callback(UINT8 *buf, UINT16 len)
{
    fibo_textTrace("FIBO <--%s", buf);
}

static FIBO_CALLBACK_T user_callback = {
    .fibo_signal = sig_res_callback,
    .at_resp = at_res_callback};

static void user_sms_event_callback(uint32_t ind_flag, void *ind_msg_buf, uint32_t ind_msg_len)
{
    switch(ind_flag)
    {
        case FIBO_SMS_INIT_OK_IND:
        {
            break;
        }
        case FIBO_SMS_NEW_MSG_IND:
        {
            fibo_sms_msg_t *sms_msg = (fibo_sms_msg_t *)ind_msg_buf;
            fibo_textTrace("[%s-%d] sms_msg->phoneNum:%s", __FUNCTION__, __LINE__, sms_msg->phoneNum);
            fibo_textTrace("[%s-%d] sms_msg->message:%s", __FUNCTION__, __LINE__, sms_msg->message);
            break;
        }
        case FIBO_SMS_LIST_IND:
        {
            break;
        }
        case FIBO_SMS_LIST_EX_IND:
        {
            break;
        }
        case FIBO_SMS_LIST_END_IND:
        {
            break;
        }
        case FIBO_SMS_MEM_FULL_IND:
        {
            break;
        }
        case FIBO_SMS_REPORT_IND:
        {
            break;
        }
        default :
            break;
    }
}


static void hello_world_demo()
{
    fibo_textTrace("application thread enter");

    fibo_taskSleep(5000);
    fibo_taskSleep(5000);
    fibo_taskSleep(5000);
    fibo_taskSleep(5000);

    fibo_sms_service_center_cfg_t service_center;
    memcpy(service_center.service_center_addr,"+8613010815500",strlen("+8618064310809"));
    memcpy(service_center.service_center_addr_type,"145",strlen("129"));

    fibo_sms_setsmscenteraddress(0, &service_center);

    memset(&service_center, 0, sizeof(fibo_sms_service_center_cfg_t));
    fibo_sms_getsmscenteraddress(0, &service_center);

    fibo_textTrace("fibo_sms_getsmscenteraddress1: %s", service_center.service_center_addr);
    fibo_textTrace("fibo_sms_getsmscenteraddress2: %s", service_center.service_center_addr_type);


    fibo_textTrace("fibo_sms_send_message");
    fibo_sms_send_msg_info_t *sms_info = malloc(sizeof(fibo_sms_send_msg_info_t));
    memset(sms_info, 0, sizeof(fibo_sms_send_msg_info_t));
    sms_info->format = 0;
    sms_info->message_class_valid = 0;
    memcpy(sms_info->mobile_number, "19929018341", strlen("19929018341"));
    memcpy(sms_info->message_text, "fibocom", strlen("fibocom"));
    sms_info->length = strlen("fibocom");
    int ret = fibo_sms_send_message(0, sms_info);
    fibo_textTrace("fibo_sms_send_message ret:%d", ret);
    fibo_sms_addrxmsghandler(0, user_sms_event_callback);

    fibo_sms_read_message_info_t read_info;
    fibo_sms_read_message_req_info_t read_req_info;
    fibo_sms_storage_info_t sms_storage;
    memset(&read_info, 0, sizeof(fibo_sms_read_message_info_t));
    memset(&read_req_info, 0, sizeof(fibo_sms_read_message_req_info_t));
    read_req_info.storage_index = 1;
    read_req_info.storage_type = 0;
    fibo_sms_read_message(0, &read_info, &read_req_info);
    fibo_textTrace("fibo_sms_read_message,read_info.message_text:%s",read_info.message_text);
    fibo_textTrace("fibo_sms_read_message,read_info.mobile_number:%s",read_info.mobile_number);
    fibo_textTrace("fibo_sms_read_message,index:%d",read_req_info.storage_index);
    fibo_textTrace("fibo_sms_read_message,time:%d:%d:%d:%d:%d:%d:%d",read_info.timestamp.year,read_info.timestamp.month,read_info.timestamp.day,read_info.timestamp.hours,read_info.timestamp.minutes,read_info.timestamp.seconds,read_info.timestamp.timezone);

    sms_storage.e_storage=E_FIBO_SMS_STORAGE_TYPE_UIM;
    sms_storage.storage_idx = 6;
    ret = fibo_sms_deletefromstorage(0, &sms_storage);
    fibo_textTrace("fibo_sms_deletefromstorage,ret:%d",ret);
}


void *appimg_enter(void *param)
{
    fibo_textTrace("app image enter");

    fibo_thread_create(hello_world_demo, "mythread", 10*1024, NULL, OSI_PRIORITY_NORMAL);
    return (void *)&user_callback;
}

void appimg_exit(void)
{
    OSI_LOGI(0, "application image exit");
}

