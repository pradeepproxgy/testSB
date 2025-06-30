/* Copyright (C) 2022 FIBOCOM Technologies Limited and/or its affiliates("FIBOCOM").
 * All rights reserved.
 *
 * This software is supplied "AS IS" without any warranties.
 * FIBOCOM assumes no responsibility or liability for the use of the software,
 * conveys no license or title under any patent, copyright, or mask work
 * right to the product. FIBOCOM reserves the right to make changes in the
 * software without notification.  FIBOCOM also make no representation or
 * warranty that such application will be suitable for the specified use
 * without further testing or modification.
 */


#define OSI_LOG_TAG OSI_MAKE_LOG_TAG('M', 'Y', 'A', 'P')

#include "osi_log.h"
#include "osi_api.h"
#include "fibo_opencpu.h"
#include "oc_gprs.h"

static void mqtt_cloud_thread_entry(void *param)
{
    fibo_textTrace("application thread enter, param 0x%x", param);
    reg_info_t reg_info;
    while (1)
    {
        fibo_reg_info_get(&reg_info, 0);
        fibo_taskSleep(1000);
        fibo_textTrace("[fibocom %s-%d]", __FUNCTION__, __LINE__);
        if (reg_info.nStatus == 1)
        {
            fibo_pdp_active(1, NULL, NULL, NULL, 0, 0, NULL);
            fibo_taskSleep(1000);
            fibo_textTrace("[fibocom %s-%d]", __FUNCTION__, __LINE__);
            break;
        }
    }
    fibo_textTrace("[fibocom ======= tuya cloud test ===================");
    fibo_mqtt_tuya_config(1, 
        (const uint8_t *)"78iln8xbfqhhjsrk",
        (const uint8_t *)"tuya0bce44f023d59393",
        (const uint8_t *)"BvSknOsD5DG2cBASZe0Q2kKYnJdMcjA8");
#if 0


    fibo_textTrace("[fibocom ======= ali cloud test ===================");
    fibo_mqtt_aliyun_config((const uint8_t *)"a1h0Epe3Ebv",
        (const uint8_t *)"AUTODEVICE1", (const uint8_t *)"2ef11cc897bc5f5d6e57963180e8143d", false);

    fibo_textTrace("[fibocom ======= ali cloud dyn test ===============");
    fibo_mqtt_aliyun_dynamic(
        (const uint8_t *)"a1h0Epe3Ebv",(const uint8_t *)"6Wc4YGp2raz6YRAt",(const uint8_t *)"AUTODEVICE1", 
        (const uint8_t *)"iot-as-mqtt.cn-shanghai.aliyuncs.com", 1883);


    fibo_thread_sleep(4000);

    fibo_textTrace("[fibocom ======= tecent cloud test ===================");
    fibo_mqtt_tecent_config((const uint8_t *)"IO8GAP1NFS", (const uint8_t *)"TestDevice",
        (const uint8_t *)"M5KJOGqyQIOMvXzUFb2Yig==");

    fibo_textTrace("[fibocom ======= tecent cloud dyn test ===============");
    fibo_mqtt_tecent_dynamic((const uint8_t *)"IO8GAP1NFS",(const uint8_t *)"TestDevice",
        (const uint8_t *)"IXcbsCxtjPHYqG5YJ6p7LXv3",
        (const uint8_t *)"http://ap-guangzhou.gateway.tencentdevices.com/device/register");

    fibo_thread_sleep(4000);
    fibo_textTrace("[fibocom ======= huawei cloud test ===================");
    fibo_mqtt_huawei_config((const uint8_t *)"5ff66e644f04b003099af6e4_TestDevice",
        (const uint8_t *)"d31e5727f2d84f67b6c65941ab91a85a");

    fibo_thread_sleep(1000);
    fibo_textTrace("[fibocom ======= onenet cloud test ===================");
    fibo_mqtt_onenet_config(
        (const uint8_t *)"449920", 
        (const uint8_t *)"MQTTtest1", 
        (const uint8_t *)"Xm6uV526478hndQbnZNc4A8uaE+k+4zisCPuJUkGfds=");

    fibo_thread_sleep(1000);
    fibo_textTrace("[fibocom ======= baidu cloud test ====================");
    fibo_mqtt_baidu_config(
        (const uint8_t *)"avslzjf", 
        (const uint8_t *)"device1", 
        (const uint8_t *)"lWvGGoMxvbDwavDc");

    fibo_thread_sleep(1000);
    fibo_textTrace("[fibocom ======= aep cloud test ======================");
    fibo_mqtt_aep_config(
        (const uint8_t *)"15070808", 
        (const uint8_t *)"867567040115156", 
        (const uint8_t *)"URegX8RjS3at4Qr1kg_1h58ZG0YbrlstJkqY09PldtA",
        (const uint8_t *)"niuyz");
#endif

	fibo_thread_delete();
}

static void sig_res_callback(GAPP_SIGNAL_ID_T sig, va_list arg)
{
    char *host = NULL;
    uint16_t port = 0;
    char *username = NULL;
    char *password = NULL;
    char *clientid = NULL;
    char *product_key = NULL;
    char *device_name = NULL;
    char *device_secret = NULL;
    int decrypt_type = 0;
    char *psk = NULL;
    switch (sig)
    {
        case GAPP_SIG_MQTT_CLOUD_AUTH_ALI:
        case GAPP_SIG_MQTT_CLOUD_AUTH_TENCENT:
        case GAPP_SIG_MQTT_CLOUD_AUTH_HUAWEI:
        case GAPP_SIG_MQTT_CLOUD_AUTH_ONENET:
        case GAPP_SIG_MQTT_CLOUD_AUTH_BAIDU:
        case GAPP_SIG_MQTT_CLOUD_AUTH_AEP:
        {
            fibo_textTrace("fibocom mqtt cloud request finish sig: %d",sig);
            username = va_arg(arg, char *);
            password = va_arg(arg, char *);
            clientid = va_arg(arg, char *);
            fibo_textTrace("fibocom cloud username: %s",username);
            fibo_textTrace("fibocom cloud password: %s",password);
            fibo_textTrace("fibocom cloud clientid: %s",clientid);
            break;
        }
        case GAPP_SIG_MQTT_CLOUD_AUTH_TUYA:
        {
            fibo_textTrace("fibocom tuya cloud request finish sig: %d",sig);
            host = va_arg(arg, char *);
            port = va_arg(arg, int);
            username = va_arg(arg, char *);
            password = va_arg(arg, char *);
            clientid = va_arg(arg, char *);
            fibo_textTrace("fibocom cloud host: %s",host);
            fibo_textTrace("fibocom cloud port: %d",port);
            fibo_textTrace("fibocom cloud username: %s",username);
            fibo_textTrace("fibocom cloud password: %s",password);
            fibo_textTrace("fibocom cloud clientid: %s",clientid);
            break;
        }
        case GAPP_SIG_MQTT_CLOUD_TUYA_EVENT:
        {
            fibo_textTrace("fibocom tuya cloud request finish sig: %d",sig);
            int evid = va_arg(arg, int);
            fibo_textTrace("fibocom tuya event: %d",evid);
            break;
        }
        case GAPP_SIG_MQTT_CLOUD_DYN_ALI:
        {
            fibo_textTrace("fibocom mqtt cloud dyn request finish sig: %d",sig);
            product_key = va_arg(arg, char *);
            device_name = va_arg(arg, char *);
            device_secret = va_arg(arg, char *);
            fibo_textTrace("fibocom alicloud product_key: %s",product_key);
            fibo_textTrace("fibocom alicloud device_name: %s",device_name);
            fibo_textTrace("fibocom alicloud device_secret: %s",device_secret);
            break;
        }
        case GAPP_SIG_MQTT_CLOUD_DYN_TENCENT:
        {
            fibo_textTrace("fibocom tencent cloud dyn request finish sig: %d",sig);
            decrypt_type = va_arg(arg, int);
            fibo_textTrace("fibocom tencent cloud decrypt_type: %d",decrypt_type);
            if(decrypt_type == 2)
            {
                psk = va_arg(arg, char *);
                fibo_textTrace("fibocom tencent cloud psk: %s",psk);
            }
            break;
        }
        default:
        {
            break;
        }
    }
}

static FIBO_CALLBACK_T user_callback = {
    .fibo_signal = sig_res_callback};


void *appimg_enter(void *param)
{
    fibo_textTrace("application image enter, param 0x%x", param);
    fibo_thread_create(mqtt_cloud_thread_entry, "mythread", 1024 * 4, NULL, OSI_PRIORITY_NORMAL);
    return (void *)&user_callback;
}

void appimg_exit(void)
{
    fibo_textTrace("application image exit");
}
