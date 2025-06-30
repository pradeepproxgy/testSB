#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "fibo_opencpu.h"
#include "internals.h"
#include "lwm2m_fota.h"
#include "app_lwm2m.h"

typedef void (*lwm2m_data_process) (void*);
typedef void (*lwm2m_fota_download_callback)(int evid, int reason, void* arg);

#define MAX_URL_LENGTH 256
#define PROTOCOL_LENGTH 32

#define LWM2M_OLD_VERSION "/lwm2m_fota_old_verion"

#define LWM2M_URI_FLAG_OBJECT_ID    (uint8_t)0x04
#define LWM2M_URI_FLAG_INSTANCE_ID  (uint8_t)0x02
#define LWM2M_URI_FLAG_RESOURCE_ID  (uint8_t)0x01
#define RES_M_STATE 3

#define assert_this(do_something) \
    if (thi == NULL) {            \
        do_something;             \
    }

typedef enum {
    LWM2M_DOWNLOAD_IDLE = 0,
    LWM2M_DOWNLOAD_START,
    LWM2M_DOWNLOADING,
    LWM2M_DOWNLOAD_FAIL,
    LWM2M_DOWNLOADED
} lwm2m_download_state_e;

typedef enum
{
	FOTA_RESULT_INIT,                //0 init
	FOTA_RESULT_SUCCESS,             //1 success
	FOTA_RESULT_NOFREE,              //2 no space
	FOTA_RESULT_OVERFLOW,            //3 downloading overflow
	FOTA_RESULT_DISCONNECT,          //4 downloading disconnect
	FOTA_RESULT_CHECKFAIL,           //5 validate fail
	FOTA_RESULT_NOSUPPORT,           //6 unsupport package
	FOTA_RESULT_URIINVALID,          //7 invalid uri
	FOTA_RESULT_UPDATEFAIL,          //8 update fail
	FOTA_RESULT_PROTOCOLFAIL        //9 unsupport protocol
}lwm2m_fota_result;

typedef struct
{
    uint32_t format;
    uint8_t token[8];
    uint32_t tokenLen;
    uint32_t counter;
}lwm2m_observe_info_t;

typedef struct {
    unsigned int format;
    unsigned char data[8]; // 8 is data length
    unsigned int length;
    unsigned int counter;
} lwm2m_fota_str_token;

typedef struct _lwm2m_data_cfg_t
{
    int type;
    int cookie;
    lwm2m_data_process callback;
} lwm2m_data_cfg_t;

typedef struct
{
    char url[MAX_URL_LENGTH + 1];
    lwm2m_fota_download_callback cb;
    void *arg;
}lwm2m_fota_download_t;

lwm2m_fota_download_t g_lwm2m_download = {0};
lwm2m_fota_manager* g_fota_manager = NULL;

static void observe_call_back(lwm2m_context_t * contextP, lwm2m_transaction_t * transacP, void * message)
{
    APP_LOG_INFO("ack_received %d", transacP->ack_received);
    if(transacP->ack_received)
    {
        APP_LOG_INFO("sem %x", g_fota_manager->notify_ack_sem);
        fibo_sem_signal(g_fota_manager->notify_ack_sem);
    }
}

int observe_send_transaction(lwm2m_context_t *contextP, lwm2m_data_cfg_t  *cfg, lwm2m_watcher_t *watcherP,
                             uint8_t *buffer, size_t length)
{
    lwm2m_transaction_t *transaction;
    int ret;

    if(watcherP->server == NULL)
    {
        APP_LOG_INFO("watcher server null");
        return COAP_500_INTERNAL_SERVER_ERROR;
    }

    APP_LOG_INFO("watcherP->server %p", watcherP->server);
    if(watcherP->server->sessionH == NULL)
    {
        APP_LOG_INFO("watcher server null");
        return COAP_500_INTERNAL_SERVER_ERROR;
    }

    APP_LOG_INFO("session %p", watcherP->server->sessionH);
    APP_LOG_INFO("lastMId %d", watcherP->lastMid);
    APP_LOG_INFO("tokenLen %d", watcherP->tokenLen);

    transaction = transaction_new(watcherP->server->sessionH, COAP_205_CONTENT, NULL, NULL, watcherP->lastMid, watcherP->tokenLen, watcherP->token);
    if(NULL == transaction)
    {
        APP_LOG_INFO("transc new null");
        return COAP_500_INTERNAL_SERVER_ERROR;
    }

    transaction->callback = observe_call_back;
    coap_set_header_content_type(transaction->message, watcherP->format);
    coap_set_header_observe(transaction->message, watcherP->counter++);
    coap_set_payload(transaction->message, buffer, length);
    contextP->transactionList = (lwm2m_transaction_t *)LWM2M_LIST_ADD(contextP->transactionList, transaction);
    ret = transaction_send(contextP, transaction);
    APP_LOG_INFO("notify con msg, ret:%d", ret);
    return ret;
}

uint8_t lwm2m_send_notify(lwm2m_context_t *contextP, lwm2m_observe_info_t *observe_info, int firmware_update_state)
{
    lwm2m_uri_t uri;
    int res;
    lwm2m_data_t data;
    lwm2m_media_type_t format;
    uint8_t *buffer = NULL;
    lwm2m_server_t *server;
    lwm2m_watcher_t watcherP;


    if((NULL == observe_info) || (NULL == contextP) )
    {
        APP_LOG_INFO("null pointer");
        return COAP_500_INTERNAL_SERVER_ERROR;
    }

    server = contextP->serverList;
    if(NULL == server)
    {
        APP_LOG_INFO("get registered server f");
        return COAP_500_INTERNAL_SERVER_ERROR;
    }

    uri.objectId = LWM2M_FIRMWARE_UPDATE_OBJECT_ID;
    uri.instanceId = 0;
    uri.resourceId = RES_M_STATE;
    uri.resourceInstanceId = 0;
    // uri.flag = (LWM2M_URI_FLAG_OBJECT_ID | LWM2M_URI_FLAG_INSTANCE_ID | LWM2M_URI_FLAG_RESOURCE_ID);

    format = (lwm2m_media_type_t)observe_info->format;
    memset(&data, 0, sizeof(data));
    data.id = 0;
    lwm2m_data_encode_int(firmware_update_state, &data);
    APP_LOG_INFO("data id %d", data.id);
    res = lwm2m_data_serialize(&uri, 1, &data, &format, &buffer);
    if (res < 0)
    {
        APP_LOG_INFO(" data serialize f");
        if (buffer != NULL)
        {
            lwm2m_free(buffer);
        }
        return COAP_500_INTERNAL_SERVER_ERROR;
    }

    memset(&watcherP, 0, sizeof(watcherP));
    watcherP.lastMid = contextP->nextMID++;
    watcherP.tokenLen = observe_info->tokenLen;
    memcpy(watcherP.token, observe_info->token, sizeof(watcherP.token));
    watcherP.format = format;
    watcherP.counter = observe_info->counter;
    watcherP.server = server;

    APP_LOG_INFO("server %p", server);

    return (uint8_t)observe_send_transaction(contextP, NULL, &watcherP, buffer, res);
}
#define FIRMWARE_UPDATE_RES_ID_3 3
#define LWM2M_INSTANCE_ID_0 0

lwm2m_observed_t* find_503_resource_observed(lwm2m_context_t* context)
{
    lwm2m_observed_t* target = NULL;
    for (target = context->observedList; target != NULL; target = target->next) { // if no 503
        if (target->uri.objectId == LWM2M_FIRMWARE_UPDATE_OBJECT_ID && target->uri.instanceId == LWM2M_INSTANCE_ID_0 &&
            target->uri.resourceId == FIRMWARE_UPDATE_RES_ID_3) {
            break;
        }
    }
    return target;
}

lwm2m_context_t* lwm2m_fota_get_lwm2m_context(lwm2m_fota_manager* thi)
{
    assert_this(return NULL);
    return thi->lwm2m_context;
}

/* After client abnormal reboot during FOTA, ctwing platfom don't observe 5/0/3
  again. When this thing happened, client should get token from KV to notify the
  state; Oceanconnect will observe 5/0/3 again, so the client can get token from
  function "find_503_resource_observed"
*/
static int ota_update_token_handle(lwm2m_context_t* context, lwm2m_fota_str_token* update_token)
{
    lwm2m_observed_t* target = find_503_resource_observed(context);
    if (target == NULL) {
        APP_LOG_INFO("/5/0/3 NOT OBSERVE");
        return LWM2M_FOTA_FAILED;
    }
    lwm2m_watcher_t* watcher = target->watcherList;
    watcher->active = false;
    update_token->counter = watcher->counter;
    memcpy(update_token->data, watcher->token, sizeof(update_token->data));
    update_token->length = watcher->tokenLen;
    update_token->format = watcher->format;
    return LWM2M_FOTA_SUCCESS;
}

static int lwm2m_503_notify(lwm2m_fota_manager* thi)
{
    lwm2m_uri_t uri;
    const char* uri_str = "/5/0/3";
    memset(&uri, 0, sizeof(lwm2m_uri_t));
    (void)lwm2m_stringToUri(uri_str, strlen(uri_str), &uri);
    if (thi->lwm2m_context == NULL) {
        return LWM2M_FOTA_FAILED;
    }
    lwm2m_resource_value_changed(thi->lwm2m_context, &uri);
    lwm2m_fota_str_token* update_token = (lwm2m_fota_str_token*)lwm2m_malloc(sizeof(lwm2m_fota_str_token));
    if (update_token == NULL) {
        APP_LOG_INFO("update_token malloc error!");
        return LWM2M_FOTA_FAILED;
    }

    if (ota_update_token_handle(thi->lwm2m_context, update_token) == LWM2M_FOTA_FAILED) {
        APP_LOG_INFO("ota upd tk handle f");
    }

    lwm2m_send_notify(lwm2m_fota_get_lwm2m_context(thi), (lwm2m_observe_info_t*)update_token, thi->rpt_state);

    APP_LOG_INFO("sem %x", thi->notify_ack_sem);
    fibo_sem_wait(thi->notify_ack_sem);
    lwm2m_free(update_token);
    return LWM2M_FOTA_SUCCESS;
}


int lwm2m_fota_set_state(lwm2m_fota_manager* thi, int state)
{
    assert_this(return LWM2M_FOTA_FAILED);

    if (state > LWM2M_FOTA_UPDATING) {
       APP_LOG_INFO("invalid dl state %d", state);
        return LWM2M_FOTA_FAILED;
    }

    APP_LOG_INFO("dl stat from %d to %d", thi->state, state);
    thi->state = state;
    thi->rpt_state = state;

    lwm2m_503_notify(thi);

    return LWM2M_FOTA_SUCCESS;
}

static int lwm2m_fota_download_event(lwm2m_fota_download_t *fdl, int event, int reason)
{
    if(fdl->cb != NULL)
    {
        fdl->cb(event, reason, fdl->arg);
    }
    return 0;
}

static void fota_download(void *arg)
{
    char protocol[PROTOCOL_LENGTH] = {0};
    char url[MAX_URL_LENGTH + 1] = {0};
    INT32 result = 0;
    lwm2m_fota_download_t *lwm2m_download = (lwm2m_fota_download_t *)arg;

    memcpy(url, lwm2m_download->url, MAX_URL_LENGTH + 1);

    APP_LOG_INFO("http ota url: %s", lwm2m_download->url);
    /* Is url is http */
    char* token;
    char* rest = url;
    token = strtok_r(rest, "://", &rest);
    if (token != NULL) {
        strncpy(protocol, token, sizeof(protocol));
        protocol[sizeof(protocol) - 1] = '\0'; // 确保字符串以null结尾
    } else {
        strcpy(protocol, "unknown");
    }

    if(strcmp(protocol, "https") == 0 || strcmp(protocol, "http") == 0)
    {
        lwm2m_fota_download_event(lwm2m_download, LWM2M_DOWNLOAD_START, FOTA_RESULT_INIT);
        lwm2m_fota_download_event(lwm2m_download, LWM2M_DOWNLOADING, FOTA_RESULT_INIT);

        APP_LOG_INFO("http ota url: %s", lwm2m_download->url);
        result = fibo_firmware_dl(0, (UINT8 *)&lwm2m_download->url, NULL, NULL, NULL);
        if(result < 0)
        {
            lwm2m_fota_download_event(lwm2m_download, LWM2M_DOWNLOAD_FAIL, FOTA_RESULT_DISCONNECT);
        }
        else
        {
            lwm2m_fota_download_event(lwm2m_download, LWM2M_DOWNLOADED, FOTA_RESULT_INIT);
        }
    }
    else
    {
        lwm2m_fota_download_event(lwm2m_download, LWM2M_DOWNLOAD_IDLE, FOTA_RESULT_PROTOCOLFAIL);
    }

    fibo_thread_delete();
    return;
}

int lwm2m_fota_http_download(char* url, int len, lwm2m_fota_download_callback cb, void *arg)
{

    if(url == NULL)
    {
        return LWM2M_FOTA_FAILED;
    }
    snprintf(g_lwm2m_download.url, MAX_URL_LENGTH, "%.*s", len, url);
    g_lwm2m_download.arg = arg;
    g_lwm2m_download.cb = cb;

    fibo_thread_create(fota_download, "fota_download", 8*1024, (void *)&g_lwm2m_download, OSI_PRIORITY_NORMAL);
    return LWM2M_FOTA_SUCCESS;
}

static void download_event_cb(int evid, int reason, void *arg)
{
    if(arg == NULL)
    {
        return;
    }
    APP_LOG_INFO("evid %d reason %d", evid, reason);
    /**
     * @brief 根据事件id进行处理
     * 
     */
    switch (evid) {
        case LWM2M_DOWNLOAD_START:
            APP_LOG_INFO("lwm2m download start");
            // Handle LwM2M download start state
            lwm2m_fota_set_update_result((lwm2m_fota_manager *)arg, UPDATE_RESULT_INIT);
            lwm2m_fota_set_state((lwm2m_fota_manager *)arg, LWM2M_FOTA_DOWNLOADING);
            break;
        case LWM2M_DOWNLOADING:
            APP_LOG_INFO("lwm2m downloading");
            break;
        case LWM2M_DOWNLOAD_FAIL:
            APP_LOG_INFO("lwm2m download fail");
            // Handle LwM2M download fail state
            lwm2m_fota_set_update_result((lwm2m_fota_manager *)arg, UPDATE_RESULT_LOST_CONNECT);
            lwm2m_fota_set_state((lwm2m_fota_manager *)arg, LWM2M_FOTA_IDLE);

            // end of fota dl
            lwm2m_fota_destroy();
            break;
        case LWM2M_DOWNLOADED:
            APP_LOG_INFO("lwm2m downloaded");
            // Handle LwM2M downloaded state
            lwm2m_fota_set_update_result((lwm2m_fota_manager *)arg, UPDATE_RESULT_INIT);
            lwm2m_fota_set_state((lwm2m_fota_manager *)arg, LWM2M_FOTA_DOWNLOADED);
            break;
        default:
            // Handle invalid state
            break;
    }

    return;
}

static int download_start_cb(void* thi, uint8_t* url, int len)
{
    int ret = LWM2M_FOTA_SUCCESS;
    lwm2m_fota_manager* manager = (lwm2m_fota_manager*)thi;

    if(url == NULL || len == 0 || manager == NULL)
    {
        return LWM2M_FOTA_FAILED;
    }
    lwm2m_fota_http_download((char *)url, len, download_event_cb, (void *)manager);

    return ret;
}

static lwm2m_fota_manager* lwm2m_fota_manager_init(lwm2m_context_t *contextP)
{
    if(g_fota_manager != NULL)
    {
        APP_LOG_INFO("fota manager is exist!");
        return g_fota_manager;
    }

    
    g_fota_manager = (lwm2m_fota_manager*)lwm2m_malloc(sizeof(lwm2m_fota_manager));
    if (g_fota_manager == NULL) {
        return NULL;
    }
    memset(g_fota_manager, 0, sizeof(lwm2m_fota_manager));

    g_fota_manager->download_start_callback = download_start_cb;
    g_fota_manager->download_event_callback = download_event_cb;
    g_fota_manager->notify_ack_sem = fibo_sem_new(0);
    g_fota_manager->lwm2m_context = contextP;

    APP_LOG_INFO("sem %x", g_fota_manager->notify_ack_sem);
    return g_fota_manager;
}

void lwm2m_fota_destroy()
{
    if (g_fota_manager != NULL) {
        lwm2m_free(g_fota_manager->ota_uri);
        fibo_sem_free(g_fota_manager->notify_ack_sem);
        lwm2m_free(g_fota_manager);
        g_fota_manager = NULL;
    }
}

lwm2m_fota_manager* lwm2m_get_fota_manager(lwm2m_context_t * contextP)
{
    return lwm2m_fota_manager_init(contextP);
}

int lwm2m_fota_start_download(lwm2m_fota_manager* thi, const char* uri, uint32_t len)
{
    if (uri == NULL || len == 0 || thi == NULL) {
        return LWM2M_FOTA_FAILED;
    }
    if (thi->state != thi->rpt_state) {
        APP_LOG_INFO("start dl state %u rpt state %u", thi->state, thi->rpt_state);
        return LWM2M_FOTA_FAILED;
    }
    if (thi->download_start_callback == NULL) {
        APP_LOG_INFO("start dl func is null");
        return LWM2M_FOTA_FAILED;
    }

    lwm2m_free(thi->ota_uri);
    thi->ota_uri = lwm2m_malloc(len + 1);
    if(thi->ota_uri == NULL)
    {
        return LWM2M_FOTA_FAILED;
    }
    memset(thi->ota_uri, 0, len + 1);
    memcpy(thi->ota_uri, uri, len);

    lwm2m_fota_set_update_result(thi, UPDATE_RESULT_INIT);
    return thi->download_start_callback(thi, (uint8_t *)uri, len);
}

lwm2m_fota_state_e lwm2m_fota_get_state(const lwm2m_fota_manager* thi)
{
    assert_this(return LWM2M_FOTA_IDLE);
    return thi->rpt_state;
}

int lwm2m_firmware_old_version_save()
{
    INT32 fd = -1;
    INT32 ret = -1;
    INT8 *version = fibo_get_sw_verno();

    fd = fibo_file_open(LWM2M_OLD_VERSION, (O_CREAT|O_RDWR));
    if(fd < 0)
    {
        return LWM2M_FOTA_FAILED;
    }

    APP_LOG_INFO("write update old version %s to %s", version, LWM2M_OLD_VERSION);
    if (0 >= (ret = fibo_file_write(fd, (UINT8 *)version, strlen((const char *)version) + 1)))
    {
        APP_LOG_INFO("write update old version error!");
        fibo_file_close(fd);
        return LWM2M_FOTA_FAILED;
    }
    fibo_file_close(fd);
    return LWM2M_FOTA_SUCCESS;
}

static void fota_update(void *arg)
{
    lwm2m_fota_manager* thi = (lwm2m_fota_manager *)arg;

    lwm2m_fota_set_state(thi, LWM2M_FOTA_UPDATING);

    fibo_taskSleep(100);
    lwm2m_firmware_old_version_save();
    fibo_taskSleep(100);

    int ret = fibo_firmware_update();
    if(ret < 0)
    {
        lwm2m_fota_set_update_result(thi, UPDATE_RESULT_UPDATE_FAIL);
        lwm2m_fota_set_state(thi, LWM2M_FOTA_IDLE);

        lwm2m_fota_destroy();
    }
}

int lwm2m_fota_update(void *arg)
{
    fibo_thread_create(fota_update, "fota_update", 8*1024, arg, OSI_PRIORITY_NORMAL);
    return LWM2M_FOTA_SUCCESS;
}

int lwm2m_fota_execute_update(lwm2m_fota_manager* thi)
{
    assert_this(return LWM2M_FOTA_FAILED);

    if(thi->state == LWM2M_FOTA_IDLE){
        return LWM2M_FOTA_FAILED;
    }

    lwm2m_fota_update(thi);
    return LWM2M_FOTA_SUCCESS;
}

void lwm2m_fota_set_update_result(lwm2m_fota_manager* thi, int result)
{
    assert_this(return);
    thi->update_result = result;
}

int lwm2m_fota_get_update_result(const lwm2m_fota_manager* thi)
{
    assert_this(return UPDATE_RESULT_INIT);
    return thi->update_result;
}

char *lwm2m_firmware_version_get()
{
    return (char *)fibo_get_sw_verno();
}

int lwm2m_firmware_update_state()
{
    INT8 *curr_version = fibo_get_sw_verno();
    INT8 *old_version = NULL;
    INT32 fd = -1;
    INT32 ret = -1;

    if(fibo_file_exist(LWM2M_OLD_VERSION) < 0)
    {
        return LWM2M_FOTA_FAILED;
    }

    fd = fibo_file_open(LWM2M_OLD_VERSION, O_RDONLY);
    if(fd < 0)
    {
        return LWM2M_FOTA_FAILED;
    }

    INT32 versize = fibo_file_get_size(LWM2M_OLD_VERSION);
    old_version = (INT8 *)malloc(versize + 1);
    if(old_version == NULL)
    {
        return LWM2M_FOTA_FAILED;
    }

    memset(old_version, 0, versize + 1);
    if (0 >= (ret = fibo_file_read(fd, (UINT8 *)old_version, versize)))
    {
        APP_LOG_INFO("old version %s current version %s", old_version, curr_version);
        if(strcmp(old_version, curr_version) != 0)
        {
            fibo_file_close(fd);
            free(old_version);
            fibo_file_delete(LWM2M_OLD_VERSION);
            return LWM2M_FOTA_UPDATE_SUCCESS;
        }
    }

    fibo_file_close(fd);
    free(old_version);
    fibo_file_delete(LWM2M_OLD_VERSION);
    return LWM2M_FOTA_UPDATE_FAILED;
}