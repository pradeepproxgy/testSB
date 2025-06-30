#include "fibo_opencpu.h"
#include "local_flash.h"
#include "cJSON.h"
#include "fb_uart.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "log_service.h"



#define LOCAL_APP_PARAM_FILE        "/tmp/app_param.json"
#define KEY_AUTH_KEY                "authkey"
#define KEY_REGISTER_STAUS          "registerstatus"


//dev_param.json  文件保存设备三元组  mqtt协议
#define MAX_SIZE_PRODUCT_ID         10
#define MAX_SIZE_DEVICE_NAME        48
#define MAX_SIZE_DEVICE_SECRET      64

#define LOCAL_DEV_PARAM_FILE        "/tmp/dev_param.json"
#define KEY_PRODUCT_ID              "product_id"
#define KEY_PRODUCT_SECRET          "product_secret"
#define KEY_DEVICE_NAME             "device_name"
#define KEY_DEVICE_SECRET           "device_secret"


//设备状态保存  锁状态  圈数脉冲计数

#define LOCAL_DEV_INFO_FILE        "/tmp/dev_info_sta.json"
#define KEY_BAT_LOCK_STA            "bat_lock"
#define KEY_MOTOR_LOCK_STA          "motor_lock"
#define KEY_TANGZE_LOCK_STA         "tangze_lock"
#define KEY_COLLECT_TOTAL           "collect_total"



int fb_Snprintf(char *str, const int len, const char *fmt, ...)
{
    va_list args;
    int     rc;

    va_start(args, fmt);
    rc = fibo_vsnprintf(str, len, fmt, args);
    va_end(args);
    return rc;
}

//设备三元组  获取设备三元组信息
int fb_get_dev_params(char *product_id,char *product_secret, char *device_name, char *device_secret)
{
    int   len, rlen;
    char  json_doc[MAX_SIZE_FILE_CONTENT] = {0};
    INT32 fp;
    cJSON *ProductId = NULL;
    cJSON *ProductSecret = NULL;
    cJSON *DevicesName = NULL;
    cJSON *DeviceSecret = NULL;

    /* check if file exists */
    if (fibo_file_exist(LOCAL_DEV_PARAM_FILE)<0) {
        fb_printf("Param file %s not exist. use default info", LOCAL_DEV_PARAM_FILE);
        return -1;
    }
        len= fibo_file_getSize(LOCAL_DEV_PARAM_FILE);
   // len = ftell(fp);
    if (len == 0) {
        fb_printf("Param file %s empty. use default info", LOCAL_DEV_PARAM_FILE);
        return -1;
    } else if (len > MAX_SIZE_FILE_CONTENT) {
        fb_printf("file len %d is too big!!", len);
        len = MAX_SIZE_FILE_CONTENT;
    }

    fp = fibo_file_open(LOCAL_DEV_PARAM_FILE, FS_O_RDONLY);
    if (fp<0) {
        fb_printf("fail to open file %s", LOCAL_DEV_PARAM_FILE);
        return -1;
    }

     
    rlen = fibo_file_read(fp,(UINT8 *)json_doc,(UINT32)len);
    if (len != rlen) {
        fb_printf("read len: %d not equal: %d", rlen, len);
    }
     fibo_file_close(fp);

   cJSON *req = cJSON_Parse(json_doc);
    if (req == NULL || !cJSON_IsObject(req)) {
        return -1;
    }

     ProductId = cJSON_GetObjectItem(req,KEY_PRODUCT_ID);
    if (ProductId != NULL && cJSON_IsString(ProductId)) {
        /* process property ProductId here */
        memcpy(product_id,ProductId->valuestring,strlen(ProductId->valuestring));
    }
    
     ProductSecret = cJSON_GetObjectItem(req,KEY_PRODUCT_SECRET);
    if (ProductSecret != NULL && cJSON_IsString(ProductSecret)) {
        /* process property ProductId here */
        memcpy(product_secret,ProductSecret->valuestring,strlen(ProductSecret->valuestring));
    }

    DevicesName = cJSON_GetObjectItem(req,KEY_DEVICE_NAME);
    if (DevicesName != NULL && cJSON_IsString(DevicesName)) {
        /* process property DevicesName here */
        memcpy(device_name,DevicesName->valuestring,strlen(DevicesName->valuestring));
    }


    DeviceSecret = cJSON_GetObjectItem(req,KEY_DEVICE_SECRET);
    if (DeviceSecret != NULL && cJSON_IsString(DeviceSecret)) {
        /* process property DeviceSecret here */
        memcpy(device_secret,DeviceSecret->valuestring,strlen(DeviceSecret->valuestring));
    }

   
    cJSON_Delete(req);
    return 0;
}



#define LOCAL_DEV_CONFIG_FILE           "/tmp/dev_config_param.json"
#define KEY_HEART_INTERVAL              "HeartInterval"
#define KEY_LOACTION_UPDATA_TIME        "LoactionUpdateTime"
#define KEY_EVENT_INTERVAL              "EventInterval"



int fb_set_dev_config_info(int *heart_interval,int *loaction_update_time,int *event_interval)
{
    size_t wlen, len;
    int    ret = 0;
    char   buf[MAX_SIZE_FILE_CONTENT];
    INT32   fp;

    memset(buf, 0, MAX_SIZE_FILE_CONTENT);
    fb_Snprintf(buf, MAX_SIZE_FILE_CONTENT, "{\"HeartInterval\":%d,\"LoactionUpdateTime\":%d,\"EventInterval\":%d}\n",
    *heart_interval,*loaction_update_time,*event_interval);
    len = strlen(buf);
    fb_printf("fb_set_dev_info_sta: %s\r\n", buf);
 	if(fibo_file_exist(LOCAL_DEV_CONFIG_FILE)<0) {
        fp = fibo_file_open(LOCAL_DEV_CONFIG_FILE,FS_O_RDWR|FS_O_CREAT);
    }
    else{
        fp = fibo_file_open(LOCAL_DEV_CONFIG_FILE,FS_O_RDWR|FS_O_TRUNC);
    }
        
    if (fp<0) {
        fb_printf("fail to open file %s", LOCAL_DEV_CONFIG_FILE);
        return -1;
    }

    wlen = fibo_file_write(fp,(void *)buf, len );
    if (wlen != len) {
        fb_printf("save device info to file err %u %u", len, wlen);
        ret = -1;
    }

    fibo_file_close(fp);

    return ret;
}

int fb_get_dev_config_info(int *heart_interval,int *loaction_update_time,int *event_interval)
{
    int   len, rlen;
    char  json_doc[MAX_SIZE_FILE_CONTENT] = {0};
    INT32 fp;
    cJSON *HeartInterval = NULL;
    cJSON *LoactionUpdateTime = NULL;
    cJSON *EventInterval = NULL;

    /* check if file exists */
    if (fibo_file_exist(LOCAL_DEV_CONFIG_FILE)<0) {
        fb_printf("Param file %s not exist. use default info", LOCAL_DEV_CONFIG_FILE);
        return -1;
    }

    len= fibo_file_getSize(LOCAL_DEV_CONFIG_FILE);
    if (len == 0) {
        fb_printf("Param file %s empty. use default info", LOCAL_DEV_CONFIG_FILE);
        return -1;
    } else if (len > MAX_SIZE_FILE_CONTENT) {
        fb_printf("file len %d is too big!!", len);
        len = MAX_SIZE_FILE_CONTENT;
    }

    fp = fibo_file_open(LOCAL_DEV_CONFIG_FILE, FS_O_RDONLY);
    if (fp<0) {
        fb_printf("fail to open file %s", LOCAL_DEV_CONFIG_FILE);
        return -1;
    }

     
    rlen = fibo_file_read(fp,(UINT8 *)json_doc,(UINT32)len);
    if (len != rlen) {
        fb_printf("read len: %d not equal: %d", rlen, len);
    }

    fibo_file_close(fp);
    fb_printf("fb_set_dev_info_sta  get: %s\r\n", json_doc);
    cJSON *req = cJSON_Parse(json_doc);
    if (req == NULL || !cJSON_IsObject(req)) {
        return -1;
    }

     HeartInterval = cJSON_GetObjectItem(req,KEY_HEART_INTERVAL);
    if (HeartInterval != NULL && cJSON_IsNumber(HeartInterval)) {
        /* process property ProductId here */
        *heart_interval=HeartInterval->valueint;
    }
    
     LoactionUpdateTime = cJSON_GetObjectItem(req,KEY_LOACTION_UPDATA_TIME);
    if (LoactionUpdateTime != NULL && cJSON_IsNumber(LoactionUpdateTime)) {
        /* process property ProductId here */ 
        *loaction_update_time=LoactionUpdateTime->valueint;
    }

     EventInterval = cJSON_GetObjectItem(req,KEY_EVENT_INTERVAL);
    if (EventInterval != NULL && cJSON_IsNumber(EventInterval)) {
        /* process property ProductId here */ 
        *event_interval=EventInterval->valueint;
    }


    cJSON_Delete(req);
    return 0;
}




#define LOCAL_DEV_INFO_FILE        "/tmp/dev_info_sta.json"
#define KEY_BAT_LOCK_STA            "bat_lock"
#define KEY_MOTOR_LOCK_STA          "motor_lock"
#define KEY_TANGZE_LOCK_STA         "tangze_lock"
#define KEY_ROTATE_TOTAL            "rotate_total"

//设备状态保存  锁状态  圈数脉冲计数


int fb_set_dev_info_sta(int *bat_lock,int *motor_lock, int *tangze_lock, int *rotate_total)
{
    size_t wlen, len;
    int    ret = 0;
    char   buf[MAX_SIZE_FILE_CONTENT];
    INT32   fp;

    memset(buf, 0, MAX_SIZE_FILE_CONTENT);
    fb_Snprintf(buf, MAX_SIZE_FILE_CONTENT, "{\"bat_lock\":%d,\"motor_lock\":%d,\"tangze_lock\":%d,\"rotate_total\":%d}\n",
    *bat_lock,*motor_lock,*tangze_lock,*rotate_total);
    len = strlen(buf);
    fb_printf("fb_set_dev_info_sta: %s\r\n", buf);
 	if(fibo_file_exist(LOCAL_DEV_INFO_FILE)<0) {
        fp = fibo_file_open(LOCAL_DEV_INFO_FILE,FS_O_RDWR|FS_O_CREAT);
    }
    else{
        fp = fibo_file_open(LOCAL_DEV_INFO_FILE,FS_O_RDWR|FS_O_TRUNC);
    }
        
    if (fp<0) {
        fb_printf("fail to open file %s", LOCAL_DEV_INFO_FILE);
        return -1;
    }

    wlen = fibo_file_write(fp,(void *)buf, len );
    if (wlen != len) {
        fb_printf("save device info to file err %u %u", len, wlen);
        ret = -1;
    }

    fibo_file_close(fp);

    return ret;
}

int fb_get_dev_info_sta(int *bat_lock,int *motor_lock, int *tangze_lock, int *rotate_total)
{
    int   len, rlen;
    char  json_doc[MAX_SIZE_FILE_CONTENT] = {0};
    INT32 fp;
    cJSON *BatLock = NULL;
    cJSON *MotorLock = NULL;
    cJSON *TangzeLock = NULL;
    cJSON *CotateTotal = NULL;
    
    /* check if file exists */
    if (fibo_file_exist(LOCAL_DEV_INFO_FILE)<0) {
        fb_printf("Param file %s not exist. use default info", LOCAL_DEV_INFO_FILE);
        return -1;
    }

    len= fibo_file_getSize(LOCAL_DEV_INFO_FILE);
    if (len == 0) {
        fb_printf("Param file %s empty. use default info", LOCAL_DEV_INFO_FILE);
        return -1;
    } else if (len > MAX_SIZE_FILE_CONTENT) {
        fb_printf("file len %d is too big!!", len);
        len = MAX_SIZE_FILE_CONTENT;
    }

    fp = fibo_file_open(LOCAL_DEV_INFO_FILE, FS_O_RDONLY);
    if (fp<0) {
        fb_printf("fail to open file %s", LOCAL_DEV_INFO_FILE);
        return -1;
    }

    rlen = fibo_file_read(fp,(UINT8 *)json_doc,(UINT32)len);
    if (len != rlen) {
        fb_printf("read len: %d not equal: %d", rlen, len);
    }

    fibo_file_close(fp);
    fb_printf("fb_set_dev_info_sta  get: %s\r\n", json_doc);
    cJSON *req = cJSON_Parse(json_doc);
    if (req == NULL || !cJSON_IsObject(req)) {
        return -1;
    }

     BatLock = cJSON_GetObjectItem(req,KEY_BAT_LOCK_STA);
    if (BatLock != NULL && cJSON_IsNumber(BatLock)) {
        /* process property ProductId here */
        *bat_lock=BatLock->valueint;
    }
    
     MotorLock = cJSON_GetObjectItem(req,KEY_MOTOR_LOCK_STA);
    if (MotorLock != NULL && cJSON_IsNumber(MotorLock)) {
        /* process property ProductId here */ 
        *motor_lock=MotorLock->valueint;

    }

    TangzeLock = cJSON_GetObjectItem(req,KEY_TANGZE_LOCK_STA);
    if (TangzeLock != NULL && cJSON_IsNumber(TangzeLock)) {
        /* process property DevicesName here */
        *tangze_lock=TangzeLock->valueint;
    }


    CotateTotal = cJSON_GetObjectItem(req,KEY_ROTATE_TOTAL);
    if (CotateTotal != NULL && cJSON_IsNumber(CotateTotal)) {
        /* process property DeviceSecret here */
        *rotate_total=CotateTotal->valueint;
    }

    cJSON_Delete(req);
    return 0;
}



//JT808 鉴权码
int fb_set_auth_key(char *register_status,char *auth_key)
{
 size_t wlen, len;
    int    ret = 0;
    char   buf[MAX_SIZE_FILE_CONTENT];
    INT32   fp;

    memset(buf, 0, MAX_SIZE_FILE_CONTENT);
    fb_Snprintf(buf, MAX_SIZE_FILE_CONTENT, "{\"registerstatus\":%d, \"authkey\":\"%s\"}\n",*register_status,auth_key);
    len = strlen(buf);

 	if(fibo_file_exist(LOCAL_APP_PARAM_FILE)<0) {
        fp = fibo_file_open(LOCAL_APP_PARAM_FILE,FS_O_RDWR|FS_O_CREAT);
    }
    else{
        fp = fibo_file_open(LOCAL_APP_PARAM_FILE,FS_O_RDWR|FS_O_TRUNC);
    }
        
    if (fp<0) {
        fb_printf("fail to open file %s", LOCAL_APP_PARAM_FILE);
        return -1;
    }

    wlen = fibo_file_write(fp,(void *)buf, len );
    if (wlen != len) {
        fb_printf("save device info to file err %u %u", len, wlen);
        ret = -1;
    }

    fibo_file_close(fp);

    return ret;



}

int fb_get_devices_config( char *register_status,char *auth_key)
{
    int   len, rlen;
    char  json_doc[MAX_SIZE_FILE_CONTENT] = {0};
    INT32 fp;

    /* check if file exists */
    if (fibo_file_exist(LOCAL_APP_PARAM_FILE)<0) {
        fb_printf("Param file %s not exist. use default info", LOCAL_APP_PARAM_FILE);
        return -1;
    }
    fp = fibo_file_open(LOCAL_APP_PARAM_FILE, FS_O_RDONLY);
    if (fp<0) {
        fb_printf("fail to open file %s", LOCAL_APP_PARAM_FILE);
        return -1;
    }

    len= fibo_file_getSize(LOCAL_APP_PARAM_FILE);
    if (len == 0) {
        fb_printf("Param file %s empty. use default info", LOCAL_APP_PARAM_FILE);
        fibo_file_close(fp);
        return -1;
    } else if (len > MAX_SIZE_FILE_CONTENT) {
        fb_printf("file len %d is too big!!", len);
        len = MAX_SIZE_FILE_CONTENT;
    }

    rlen = fibo_file_read(fp,(UINT8 *)json_doc,(UINT32)len);
    if (len != rlen) {
        fb_printf("read len: %d not equal: %d", rlen, len);
    }

    fibo_file_close(fp);
    fb_printf("read file %s", json_doc);
    cJSON *req = cJSON_Parse((const char *)json_doc);
    if (req == NULL) {
        return -1;
    }

    cJSON *v = cJSON_GetObjectItem(req, KEY_AUTH_KEY);
    if (v != NULL) 
    {
        memset(auth_key, 0, MAX_SIZE_AUTH_KEY+1);
        strncpy(auth_key, v->valuestring, strlen(v->valuestring));
    }

    v = cJSON_GetObjectItem(req, KEY_REGISTER_STAUS);
    if (v != NULL) 
    {
        *register_status= v->valueint;
    }
    cJSON_Delete(req);
    return 0;

}