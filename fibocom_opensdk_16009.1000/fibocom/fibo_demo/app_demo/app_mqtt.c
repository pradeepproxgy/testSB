#include <stdint.h>
#include "app_mqtt.h"

#define APP_MQTT_CHANNEL_MAX (2) /// 目前最多支持两路
#define APP_MQTT_MSG_NUM_MAX (5)

typedef enum ftp_client_status_s ftp_client_status_t;
typedef enum ftp_client_rw_status_s ftp_client_rw_status_t;

enum ftp_client_status_s
{
    MQTT_STATUS_IDLE = 0,
    MQTT_STATUS_CONNECTING = 1,
    MQTT_STATUS_CONNECTED = 2,
};

enum ftp_client_rw_status_s
{
    MQTT_STATUS_RW_IDLE = 0,
    MQTT_STATUS_RW_SUBING,
    MQTT_STATUS_RW_SUB_FINISH,
    MQTT_STATUS_RW_SUB_FAIL,
    MQTT_STATUS_RW_UNSUBING,
    MQTT_STATUS_RW_UNSUB_FINISH,
    MQTT_STATUS_RW_UNSUB_FAIL,
    MQTT_STATUS_RW_PUBTING,
    MQTT_STATUS_RW_PUB_FINISH,
    MQTT_STATUS_RW_PUB_FAIL
};

static UINT32 g_sig_mqtt_sem = 0;
static UINT32 g_sig_mqtt_lock = 0;
static UINT32 g_mqtt_queue = 0;
static ftp_client_status_t g_mqtt_client_status[APP_MQTT_CHANNEL_MAX] = {MQTT_STATUS_IDLE};
static ftp_client_rw_status_t g_mqtt_client_rw_status[APP_MQTT_CHANNEL_MAX] = {MQTT_STATUS_RW_IDLE};

static void mqtt_login_ok_callback(GAPP_SIGNAL_ID_T sig, va_list arg)
{
    fibo_mutex_lock(g_sig_mqtt_lock);
    int id = va_arg(arg, int);
    int res_result = va_arg(arg, int);
    if (g_mqtt_client_status[id] == MQTT_STATUS_CONNECTING)
    {
        g_mqtt_client_status[id] = res_result ? MQTT_STATUS_CONNECTED : MQTT_STATUS_RW_IDLE;
    }
    fibo_mutex_unlock(g_sig_mqtt_lock);
    fibo_sem_signal(g_sig_mqtt_sem);
}

static void mqtt_break_callback(GAPP_SIGNAL_ID_T sig, va_list arg)
{
    fibo_mutex_lock(g_sig_mqtt_lock);
    int id = va_arg(arg, int);
    if (g_mqtt_client_status[id] == MQTT_STATUS_CONNECTED)
    {
        g_mqtt_client_status[id] = MQTT_STATUS_IDLE;
    }
    fibo_mutex_unlock(g_sig_mqtt_lock);
    fibo_sem_signal(g_sig_mqtt_sem);
}

static void mqtt_sub_resp_callback(GAPP_SIGNAL_ID_T sig, va_list arg)
{
    fibo_mutex_lock(g_sig_mqtt_lock);
    int id = va_arg(arg, int);
    int res_result = va_arg(arg, int);
    if (g_mqtt_client_rw_status[id] == MQTT_STATUS_RW_SUBING)
    {
        g_mqtt_client_rw_status[id] = res_result ? MQTT_STATUS_RW_SUB_FINISH : MQTT_STATUS_RW_SUB_FAIL;
    }
    fibo_mutex_unlock(g_sig_mqtt_lock);
    fibo_sem_signal(g_sig_mqtt_sem);
}

static void mqtt_unsub_resp_callback(GAPP_SIGNAL_ID_T sig, va_list arg)
{
    fibo_mutex_lock(g_sig_mqtt_lock);
    int id = va_arg(arg, int);
    int res_result = va_arg(arg, int);

    if (g_mqtt_client_rw_status[id] == MQTT_STATUS_RW_UNSUBING)
    {
        g_mqtt_client_rw_status[id] = res_result ? MQTT_STATUS_RW_UNSUB_FINISH : MQTT_STATUS_RW_UNSUB_FAIL;
    }
    fibo_mutex_unlock(g_sig_mqtt_lock);
    fibo_sem_signal(g_sig_mqtt_sem);
}

static void mqtt_pub_resp_callback(GAPP_SIGNAL_ID_T sig, va_list arg)
{
    fibo_mutex_lock(g_sig_mqtt_lock);

    int id = va_arg(arg, int);
    int res_result = va_arg(arg, int);

    if (g_mqtt_client_rw_status[id] == MQTT_STATUS_RW_PUBTING)
    {
        g_mqtt_client_rw_status[id] = res_result ? MQTT_STATUS_RW_PUB_FINISH : MQTT_STATUS_RW_PUB_FAIL;
    }
    fibo_mutex_unlock(g_sig_mqtt_lock);
    fibo_sem_signal(g_sig_mqtt_sem);
}

static void mqtt_incoming_callback(GAPP_SIGNAL_ID_T sig, va_list arg)
{
    fibo_mutex_lock(g_sig_mqtt_lock);

    app_mqtt_msg_t msg = {0};

    msg.id = va_arg(arg, int);
    char *topic = va_arg(arg, char *);
    msg.qos = va_arg(arg, int);
    char *messge = va_arg(arg, char *);
    msg.len = va_arg(arg, uint32_t);

    int topiclen = strlen(topic);
    msg.topic = fibo_malloc(msg.len + topiclen + 2);
    msg.message = msg.topic + topiclen + 1;
    strcpy(msg.topic, topic);
    memcpy(msg.message, messge, msg.len);
    msg.message[msg.len] = 0;
    msg.msg_type = APP_MQTT_MSG_TYPE_DATA;

    int ret = fibo_queue_put(g_mqtt_queue, &msg, 100);
    if (ret != 0)
    {
        APP_LOG_INFO("drop msg for queue is full %s", msg.topic);
        fibo_free(msg.topic);
    }

    fibo_mutex_unlock(g_sig_mqtt_lock);
}

int app_mqtt_init()
{
    g_sig_mqtt_sem = fibo_sem_new(0);
    if (g_sig_mqtt_sem == 0)
    {
        APP_LOG_INFO("sem new fail");
        goto error;
    }

    g_sig_mqtt_lock = fibo_mutex_create();
    if (g_sig_mqtt_lock == 0)
    {
        APP_LOG_INFO("mutex new fail");
        goto error;
    }

    g_mqtt_queue = fibo_queue_create(APP_MQTT_MSG_NUM_MAX, sizeof(app_mqtt_msg_t));
    if (g_mqtt_queue == 0)
    {
        APP_LOG_INFO("create queue fail");
        goto error;
    }

    app_register_sig_callback(GAPP_SIG_CONNECT_RSP1, mqtt_login_ok_callback);
    app_register_sig_callback(GAPP_SIG_CLOSE_RSP1, mqtt_break_callback);
    app_register_sig_callback(GAPP_SIG_SUB_RSP1, mqtt_sub_resp_callback);
    app_register_sig_callback(GAPP_SIG_UNSUB_RSP1, mqtt_unsub_resp_callback);
    app_register_sig_callback(GAPP_SIG_PUB_RSP1, mqtt_pub_resp_callback);
    app_register_sig_callback(GAPP_SIG_INCOMING_DATA_RSP1, mqtt_incoming_callback);

    return 0;

error:
    fibo_mutex_delete(g_sig_mqtt_lock);
    g_sig_mqtt_lock = 0;
    fibo_sem_free(g_sig_mqtt_sem);
    g_sig_mqtt_sem = 0;
    return -1;
}

int app_mqtt_set_will(UINT8 id, INT8 *topic, UINT8 qos, BOOL retain, INT8 *message)
{
    return fibo_mqtt_will1(id, topic, qos, retain, message);
}

int app_mqtt_open(UINT8 id, INT8 *usr, INT8 *pwd, INT8 *ClientID, INT8 *DestAddr, UINT16 DestPort, UINT8 CLeanSession, UINT16 keepalive, UINT8 UseTls)
{
    int ret = 0;
    fibo_mutex_lock(g_sig_mqtt_lock);

    if (id > APP_MQTT_CHANNEL_MAX || id == 0)
    {
        fibo_mutex_unlock(g_sig_mqtt_lock);
        APP_LOG_INFO("invalid id %d", id);
        return 0;
    }

    if (g_mqtt_client_status[id] == MQTT_STATUS_CONNECTED)
    {
        fibo_mutex_unlock(g_sig_mqtt_lock);
        APP_LOG_INFO("mqtt have login");
        return 0;
    }

    ret = fibo_mqtt_set1(id, usr, pwd);
    if ((0 != ret))
    {
        fibo_mutex_unlock(g_sig_mqtt_lock);
        APP_LOG_INFO("mqtt set user pass fail");
        return -1;
    }

    ret = fibo_mqtt_connect1(id, ClientID, DestAddr, DestPort, CLeanSession, keepalive, UseTls);
    if ((0 != ret))
    {
        fibo_mutex_unlock(g_sig_mqtt_lock);
        APP_LOG_INFO("mqtt login fail");
        return -1;
    }

    g_mqtt_client_status[id] = MQTT_STATUS_CONNECTING;
    APP_WAIT_EXPECT_STATUS(g_sig_mqtt_lock, g_sig_mqtt_sem, g_mqtt_client_status[id] == MQTT_STATUS_CONNECTING, g_mqtt_client_status[id]);
    ret = (g_mqtt_client_status[id] == MQTT_STATUS_CONNECTED) ? 0 : -1;
    fibo_mutex_unlock(g_sig_mqtt_lock);
    return ret;
}

int app_mqtt_is_connectted(UINT8 id)
{
    fibo_mutex_lock(g_sig_mqtt_lock);
    int ret = (g_mqtt_client_status[id] == MQTT_STATUS_CONNECTED);
    fibo_mutex_unlock(g_sig_mqtt_lock);
    return ret;
}

int app_mqtt_pub(UINT8 id, INT8 *topic, UINT8 qos, BOOL retain, INT8 *message, UINT16 msglen)
{
    int ret = 0;
    fibo_mutex_lock(g_sig_mqtt_lock);

    if (id > APP_MQTT_CHANNEL_MAX || id == 0)
    {
        fibo_mutex_unlock(g_sig_mqtt_lock);
        APP_LOG_INFO("invalid id %d", id);
        return 0;
    }

    if (g_mqtt_client_status[id] != MQTT_STATUS_CONNECTED)
    {
        fibo_mutex_unlock(g_sig_mqtt_lock);
        APP_LOG_INFO("mqtt not login");
        return -1;
    }

    APP_WAIT_EXPECT_STATUS(g_sig_mqtt_lock, g_sig_mqtt_sem, g_mqtt_client_rw_status[id] != MQTT_STATUS_RW_IDLE, g_mqtt_client_rw_status[id]);
    ret = fibo_mqtt_pub1(id, topic, qos, retain, message, msglen);
    if ((0 != ret))
    {
        fibo_mutex_unlock(g_sig_mqtt_lock);
        APP_LOG_INFO("ftp read fail");
        return -1;
    }

    g_mqtt_client_rw_status[id] = MQTT_STATUS_RW_PUBTING;
    APP_WAIT_EXPECT_STATUS(g_sig_mqtt_lock, g_sig_mqtt_sem, g_mqtt_client_rw_status[id] == MQTT_STATUS_RW_PUBTING, g_mqtt_client_rw_status[id]);
    ret = g_mqtt_client_rw_status[id] == MQTT_STATUS_RW_PUB_FINISH ? 0 : -1;
    g_mqtt_client_rw_status[id] = MQTT_STATUS_RW_IDLE;
    fibo_mutex_unlock(g_sig_mqtt_lock);
    return ret;
}

int app_mqtt_sub(UINT8 id, INT8 *topic, UINT8 qos)
{
    int ret = 0;
    fibo_mutex_lock(g_sig_mqtt_lock);

    if (id > APP_MQTT_CHANNEL_MAX || id == 0)
    {
        fibo_mutex_unlock(g_sig_mqtt_lock);
        APP_LOG_INFO("invalid id %d", id);
        return 0;
    }

    if (g_mqtt_client_status[id] != MQTT_STATUS_CONNECTED)
    {
        fibo_mutex_unlock(g_sig_mqtt_lock);
        APP_LOG_INFO("ftp not login");
        return -1;
    }

    APP_WAIT_EXPECT_STATUS(g_sig_mqtt_lock, g_sig_mqtt_sem, g_mqtt_client_rw_status[id] != MQTT_STATUS_RW_IDLE, g_mqtt_client_rw_status[id]);
    ret = fibo_mqtt_sub1(id, topic, qos);
    if ((0 != ret))
    {
        fibo_mutex_unlock(g_sig_mqtt_lock);
        APP_LOG_INFO("ftp read fail");
        return -1;
    }

    g_mqtt_client_rw_status[id] = MQTT_STATUS_RW_SUBING;
    APP_WAIT_EXPECT_STATUS(g_sig_mqtt_lock, g_sig_mqtt_sem, g_mqtt_client_rw_status[id] == MQTT_STATUS_RW_SUBING, g_mqtt_client_rw_status[id]);
    ret = g_mqtt_client_rw_status[id] == MQTT_STATUS_RW_SUB_FINISH ? 0 : -1;
    g_mqtt_client_rw_status[id] = MQTT_STATUS_RW_IDLE;
    fibo_mutex_unlock(g_sig_mqtt_lock);
    return ret;
}

int app_mqtt_unsub(UINT8 id, INT8 *topic)
{
    int ret = 0;
    fibo_mutex_lock(g_sig_mqtt_lock);

    if (id > APP_MQTT_CHANNEL_MAX || id == 0)
    {
        fibo_mutex_unlock(g_sig_mqtt_lock);
        APP_LOG_INFO("invalid id %d", id);
        return 0;
    }

    if (g_mqtt_client_status[id] != MQTT_STATUS_CONNECTED)
    {
        fibo_mutex_unlock(g_sig_mqtt_lock);
        APP_LOG_INFO("ftp not login");
        return -1;
    }

    APP_WAIT_EXPECT_STATUS(g_sig_mqtt_lock, g_sig_mqtt_sem, g_mqtt_client_rw_status[id] != MQTT_STATUS_RW_IDLE, g_mqtt_client_rw_status[id]);
    ret = fibo_mqtt_unsub1(id, topic);
    if ((0 != ret))
    {
        fibo_mutex_unlock(g_sig_mqtt_lock);
        APP_LOG_INFO("ftp read fail");
        return -1;
    }

    g_mqtt_client_rw_status[id] = MQTT_STATUS_RW_UNSUBING;
    APP_WAIT_EXPECT_STATUS(g_sig_mqtt_lock, g_sig_mqtt_sem, g_mqtt_client_rw_status[id] == MQTT_STATUS_RW_UNSUBING, g_mqtt_client_rw_status[id]);
    ret = g_mqtt_client_rw_status[id] == MQTT_STATUS_RW_UNSUB_FINISH ? 0 : -1;
    g_mqtt_client_rw_status[id] = MQTT_STATUS_RW_IDLE;
    fibo_mutex_unlock(g_sig_mqtt_lock);
    return ret;
}

int app_mqtt_close(int id)
{
    int ret = 0;
    fibo_mutex_lock(g_sig_mqtt_lock);
    if (g_mqtt_client_status[id] != MQTT_STATUS_CONNECTED)
    {
        fibo_mutex_unlock(g_sig_mqtt_lock);
        APP_LOG_INFO("mqtt not login");
        return -1;
    }
    fibo_mqtt_close1(id);
    g_mqtt_client_status[id] = MQTT_STATUS_IDLE;
    fibo_mutex_unlock(g_sig_mqtt_lock);

    return ret;
}

int app_mqtt_read_msg(app_mqtt_msg_t *msg, uint32_t timeout)
{
    int ret = 0;

    ret = fibo_queue_get(g_mqtt_queue, msg, timeout);
    if (ret == 0)
    {
        APP_LOG_INFO("recv mqtt msg topic(%s)", msg->topic);
    }
    return ret;
}

int app_mqtt_free_msg(app_mqtt_msg_t *msg)
{
    if (msg)
    {
        fibo_free(msg->topic);
        msg->topic = NULL;
    }
    return 0;
}

#define TEST_CA_FILE "-----BEGIN CERTIFICATE-----\r\n"                                      \
                     "MIID9TCCAt2gAwIBAgIJAOEIwHHcR9K7MA0GCSqGSIb3DQEBBQUAMIGPMQswCQYD\r\n" \
                     "VQQGEwJDTjEPMA0GA1UECAwGc2hhbnhpMQ0wCwYDVQQHDAR4aWFuMRAwDgYDVQQK\r\n" \
                     "DAdmaWJvY29tMRAwDgYDVQQLDAdmaWJvY29tMRYwFAYDVQQDDA00Ny4xMTAuMjM0\r\n" \
                     "LjM2MSQwIgYJKoZIhvcNAQkBFhV2YW5zLndhbmdAZmlib2NvbS5jb20wIBcNMTkw\r\n" \
                     "ODMwMDY1MjUwWhgPMjExOTA4MDYwNjUyNTBaMIGPMQswCQYDVQQGEwJDTjEPMA0G\r\n" \
                     "A1UECAwGc2hhbnhpMQ0wCwYDVQQHDAR4aWFuMRAwDgYDVQQKDAdmaWJvY29tMRAw\r\n" \
                     "DgYDVQQLDAdmaWJvY29tMRYwFAYDVQQDDA00Ny4xMTAuMjM0LjM2MSQwIgYJKoZI\r\n" \
                     "hvcNAQkBFhV2YW5zLndhbmdAZmlib2NvbS5jb20wggEiMA0GCSqGSIb3DQEBAQUA\r\n" \
                     "A4IBDwAwggEKAoIBAQC6UMQfxHL0oW9pY1cGvq5QPdw8OU7dX2YsCbPdEiXePKce\r\n" \
                     "E6AN3IKqOuZhEd1iIypXG2AywzIu9bd5w1d4COjjSC/Tpf2AKYw+jqfxHsQAvSKt\r\n" \
                     "Rvwp1wrS5IvWy8yEG9lNpyVJHBUWlVpU/tUf02MYYU5xUBS+mJE9Tc10j7kd/uV7\r\n" \
                     "aEfM0pYhm7VmHPWDHXeXj3LKYigjttNxUgpDh2UVpq6ejzzHA5T/k2+XtKtWu7Pb\r\n" \
                     "ag6lONzz6Zxya9htVLBy7I4uTFrcRPxNgc/KF2BuwEVc4rqGUZ4vpVdwmCyKGIua\r\n" \
                     "fvit1nsvnhvYMu01HhWuK6e3IO6hOpeyR1wk75ofAgMBAAGjUDBOMB0GA1UdDgQW\r\n" \
                     "BBTT9RodyqsY/C2WS/7k8GFWidQrlTAfBgNVHSMEGDAWgBTT9RodyqsY/C2WS/7k\r\n" \
                     "8GFWidQrlTAMBgNVHRMEBTADAQH/MA0GCSqGSIb3DQEBBQUAA4IBAQCkg9dUYBRs\r\n" \
                     "uqCz71Q75B2n768icIeMfQmf969fNVK/mwaUnFxjqq+4Xw3zADdUdnTZ8FEfjAt2\r\n" \
                     "LQaxmsiGlM3KmUhXy/k1xKypIu2KecxEX/NqdG02SYcBmrIAP6ZxOxyyJZXbPRBt\r\n" \
                     "-----END CERTIFICATE-----\r\n"

#define TEST_CLIENT_FILE "-----BEGIN CERTIFICATE-----\r\n"                                      \
                         "MIID9TCCAt2gAwIBAgIJAOEIwHHcR9K7MA0GCSqGSIb3DQEBBQUAMIGPMQswCQYD\r\n" \
                         "VQQGEwJDTjEPMA0GA1UECAwGc2hhbnhpMQ0wCwYDVQQHDAR4aWFuMRAwDgYDVQQK\r\n" \
                         "DAdmaWJvY29tMRAwDgYDVQQLDAdmaWJvY29tMRYwFAYDVQQDDA00Ny4xMTAuMjM0\r\n" \
                         "LjM2MSQwIgYJKoZIhvcNAQkBFhV2YW5zLndhbmdAZmlib2NvbS5jb20wIBcNMTkw\r\n" \
                         "ODMwMDY1MjUwWhgPMjExOTA4MDYwNjUyNTBaMIGPMQswCQYDVQQGEwJDTjEPMA0G\r\n" \
                         "A1UECAwGc2hhbnhpMQ0wCwYDVQQHDAR4aWFuMRAwDgYDVQQKDAdmaWJvY29tMRAw\r\n" \
                         "DgYDVQQLDAdmaWJvY29tMRYwFAYDVQQDDA00Ny4xMTAuMjM0LjM2MSQwIgYJKoZI\r\n" \
                         "hvcNAQkBFhV2YW5zLndhbmdAZmlib2NvbS5jb20wggEiMA0GCSqGSIb3DQEBAQUA\r\n" \
                         "A4IBDwAwggEKAoIBAQC6UMQfxHL0oW9pY1cGvq5QPdw8OU7dX2YsCbPdEiXePKce\r\n" \
                         "E6AN3IKqOuZhEd1iIypXG2AywzIu9bd5w1d4COjjSC/Tpf2AKYw+jqfxHsQAvSKt\r\n" \
                         "Rvwp1wrS5IvWy8yEG9lNpyVJHBUWlVpU/tUf02MYYU5xUBS+mJE9Tc10j7kd/uV7\r\n" \
                         "aEfM0pYhm7VmHPWDHXeXj3LKYigjttNxUgpDh2UVpq6ejzzHA5T/k2+XtKtWu7Pb\r\n" \
                         "ag6lONzz6Zxya9htVLBy7I4uTFrcRPxNgc/KF2BuwEVc4rqGUZ4vpVdwmCyKGIua\r\n" \
                         "fvit1nsvnhvYMu01HhWuK6e3IO6hOpeyR1wk75ofAgMBAAGjUDBOMB0GA1UdDgQW\r\n" \
                         "BBTT9RodyqsY/C2WS/7k8GFWidQrlTAfBgNVHSMEGDAWgBTT9RodyqsY/C2WS/7k\r\n" \
                         "8GFWidQrlTAMBgNVHRMEBTADAQH/MA0GCSqGSIb3DQEBBQUAA4IBAQCkg9dUYBRs\r\n" \
                         "uqCz71Q75B2n768icIeMfQmf969fNVK/mwaUnFxjqq+4Xw3zADdUdnTZ8FEfjAt2\r\n" \
                         "LQaxmsiGlM3KmUhXy/k1xKypIu2KecxEX/NqdG02SYcBmrIAP6ZxOxyyJZXbPRBt\r\n" \
                         "-----END CERTIFICATE-----\r\n"

#define TEST_CLIENT_KEY_FILE "-----BEGIN RSA PRIVATE KEY-----\r\n"                                  \
                             "MIIEowIBAAKCAQEArwZekEwabeqU3vfWXfSjs1MSUF20AqrQ3iXc5nsKWtmtKakr\r\n" \
                             "T7DHgd/jNJNzobHnfRwDwnJuvpu3CEdg5VHuYtZ40R3qMR3NodW4UtJlzyaODYCx\r\n" \
                             "aiI6q33XoPmwQo/Ids3F1x/Oyj8wm1Fzkzm0/60lCxxvHI5vrUp2II1h4yK3TpQa\r\n" \
                             "peX4HV23+XmCApyPGERiXFjvlAw+9NNV6LWZ7VbIK63HovnA/HyPuor8RW4iy0Ng\r\n" \
                             "lOc82NGT5hb20QcESvKDPJPNutcBIyrMVI4hWMegmLX7gs4lU5XgajrQvQNH0q6e\r\n" \
                             "+6I6LxUt7pqqopOUdbgs0AjmWV6cpMii/DpEawIDAQABAoIBADrTbjcbjQqCfJTQ\r\n" \
                             "QdmEXvznn9EpHVaEKP1xRmSk2B8e6GeHN0pqhTOul0PVh1jCXaacIttY8MXZulPr\r\n" \
                             "AbMxrWjE4wiOAGePt8x7857KnnNYZwg8x+R/Kq729eFh6o8EmoDrumIKi8tIH8Mk\r\n" \
                             "Ri8mhyIkBL5OST4U1Y4t57QbMNpRA3bSAIoD6/QxaZ9t2/m7IUyClf1KKFbXCaTL\r\n" \
                             "6FZh68mXoJpPNV75rXTq9TNYtVO2k7h6oW8iuu5UwnQQpXBkLloOm6VMmFRCFuC+\r\n" \
                             "fh2LBgxTEkFdfDJnJpeuEVCc558zPonLvKVmD8rkaCLHIETePm9R5JOC/Rv9ROH1\r\n" \
                             "tBgrQwkCgYEA1DoL2Qu1A2xg8jJfHPOqv17vlrxa4ENq2xG1yG0siKOpd6X1MRwc\r\n" \
                             "dc3uh5DAA/80KTn7xFArB1KHweosUGIO9fmU/gkvXfLlGQnNsgIdKFZvFyM1sTP5\r\n" \
                             "fdXcsJS5fKAcC1CsvPVvpxJyiT3LhGCRO4hfmcl6jJ9OzsnW/eplIh8CgYEA0x//\r\n" \
                             "csZ0V1OoXQeTsJ5QRod8Qh0dPwkLqDQsVZyb4bllO2TkC/dLyqVowijpHxBh5Elq\r\n" \
                             "jCa/jjNSYGo4hiKdvlriYEFTDebDC3SXxdcv86ixC0vH4zKti8NDKhunVIG/8fCi\r\n" \
                             "iqvB/Tjf7SQEGWRqjykyLRVeH4kDF03kLJW3TDUCgYEAh4wzeQMzL+aO3OIzQYiX\r\n" \
                             "6/a0y++tk0M8AoODOWoRYYw2dwb2XdF4k/1ddhSLr4HWTOaN2Uri0KBzuPTaLNUU\r\n" \
                             "fSJVeRNgv36duKo8SI91FAhwl7STXIS3uxlXBSlYdzLD9q4mReH02B6+LM3dKMWM\r\n" \
                             "vRtTBCRdM2ekrAraV/7XbT0CgYBAy+dIwKPgUWqw8qxfXpdgriBy4iChwhLz0t9w\r\n" \
                             "fxpQkugA7JwZGBMI5O9b99ZklFCXEflDfnj4GcRElxU2BdXIIHit9h6Ze6ONFoGm\r\n" \
                             "VL8A11tPDjkQ//LHnGw2tjoK86+Hf8VDLifhod0IGS+w42LZAVnHAHHc1948/sjy\r\n" \
                             "7hhNqQKBgBs5xypuEz9MMof6+6vxFAWDmTMuZr7CVrgR3AaksHHD1w5Pz0J6wp6I\r\n" \
                             "kOs3Yx8d14lzom1voiO/MxBpiFssymxLl/0XAtY6NwwMYpvxgGntjABZJsTZ+xsv\r\n" \
                             "SVqKuwoZ7c0EXnBRKTcWwIhN/YgrEZ2ljqnseDAARdXFEY+Ga070\r\n"             \
                             "-----END RSA PRIVATE KEY-----\r\n"

#define MQTT_TASK_STATUS_RUNNING 0
#define MQTT_TASK_STATUS_STOPPED 1

static int g_mqtt_task_status = MQTT_TASK_STATUS_STOPPED;
int app_mqtt_start_demo()
{
    int id = 1;
    INT8 client_id[] = "lens_0ptfvqWWCzblt9phGtddgOdO3z3";
    char serv_addr[] = "broker.emqx.io";
    UINT16 port = 1883;
    INT8 username[] = "test";
    INT8 password[] = "123";
    UINT8 clear_session = 0;
    UINT16 keepalive = 10;
    UINT8 UseTls = false;
    UINT8 qos = 0;
    BOOL retain = 0;
    INT8 *topic = "topic";
    INT8 pubmessg[] = "pubmesg test";

    app_mqtt_msg_t msg = {0};

    fibo_mutex_lock(g_sig_mqtt_lock);
    APP_WAIT_EXPECT_STATUS(g_sig_mqtt_lock, g_sig_mqtt_sem, g_mqtt_task_status != MQTT_TASK_STATUS_STOPPED, g_mqtt_task_status);

    if (UseTls)
    {
        /// 除非特殊情况，最好使用TLS 1.2
        fibo_ssl_set_ver(4); /// 设置SSL版本为TLS 1.2

        /// 除非特殊情况，最好设置为验证服务器证书(默认不验证)
        fibo_ssl_chkmode_set(1);

        /// 如果fibo_ssl_chkmode_set设置为1，则需要加载CA证书（pem格式），否则会连接失败
        fibo_ssl_write_file("TRUSTFILE", (UINT8 *)TEST_CA_FILE, sizeof(TEST_CA_FILE));

        /// 如果服务器需要验证客户端，则需要加载客户端的公约和私钥（pem格式）
        fibo_ssl_write_file("CAFILE", (UINT8 *)TEST_CLIENT_FILE, sizeof(TEST_CLIENT_FILE));
        fibo_ssl_write_file("CAKEY", (UINT8 *)TEST_CLIENT_KEY_FILE, sizeof(TEST_CLIENT_KEY_FILE));
    }

    g_mqtt_task_status = MQTT_TASK_STATUS_RUNNING;
    while (g_mqtt_task_status == MQTT_TASK_STATUS_RUNNING)
    {
        fibo_mutex_unlock(g_sig_mqtt_lock);
        int ret = app_mqtt_open(id, username, password, client_id, serv_addr, port, clear_session, keepalive, UseTls);
        if (ret != 0)
        {
            APP_LOG_INFO("mqtt login fail");
            fibo_taskSleep(1000);
            fibo_mutex_lock(g_sig_mqtt_lock);
            continue;
        }

        ret = app_mqtt_sub(id, topic, qos);
        if (ret != 0)
        {
            APP_LOG_INFO("mqtt sub fail");
        }

        ret = app_mqtt_sub(id, "unsubtopic", qos);
        if (ret != 0)
        {
            APP_LOG_INFO("mqtt sub fail");
        }

        ret = app_mqtt_unsub(id, "unsubtopic");
        if (ret != 0)
        {
            APP_LOG_INFO("mqtt unsub fail");
        }

        ret = app_mqtt_pub(id, topic, qos, retain, pubmessg, sizeof(pubmessg) - 1);
        if (ret != 0)
        {
            APP_LOG_INFO("mqtt pub fail");
        }

        while (app_mqtt_is_connectted(id))
        {
            int ret = app_mqtt_read_msg(&msg, 1000);
            if (ret == 0)
            {
                if (msg.msg_type == APP_MQTT_MSG_TYPE_DATA)
                {
                    APP_LOG_INFO("recv %s %s", msg.topic, msg.message);
                }
                else if (msg.msg_type == APP_MQTT_MSG_TYPE_EXIT)
                {
                    APP_LOG_INFO("recv exit cmd");
                    app_mqtt_close(id);
                    fibo_mutex_lock(g_sig_mqtt_lock);
                    g_mqtt_task_status = MQTT_TASK_STATUS_STOPPED;
                    fibo_mutex_unlock(g_sig_mqtt_lock);
                    break;
                }
                app_mqtt_free_msg(&msg);
            }
        }
        fibo_mutex_lock(g_sig_mqtt_lock);
        APP_LOG_INFO("MQTT DISCONNECT"); // 断开了
    }

    fibo_mutex_unlock(g_sig_mqtt_lock);
    fibo_sem_signal(g_sig_mqtt_sem);
    APP_LOG_INFO("MQTT TASK EXIT"); // MQTT 任务退出
    return 0;
}

int app_mqtt_stop_demo()
{
    app_mqtt_msg_t msg = {0};

    fibo_mutex_lock(g_sig_mqtt_lock);
    if (g_mqtt_task_status != MQTT_TASK_STATUS_RUNNING)
    {
        APP_LOG_INFO("NOT RUNNING"); // 断开了
        fibo_mutex_unlock(g_sig_mqtt_lock);
    }

    msg.msg_type = APP_MQTT_MSG_TYPE_EXIT;
    fibo_queue_put(g_mqtt_queue, &msg, 0);

    APP_WAIT_EXPECT_STATUS(g_sig_mqtt_lock, g_sig_mqtt_sem, g_mqtt_task_status != MQTT_TASK_STATUS_STOPPED, g_mqtt_task_status);
    fibo_mutex_unlock(g_sig_mqtt_lock);

    return 0;
}

static void mqtt_task(void *arg)
{
    APP_LOG_INFO("enter mqtt task");
    app_mqtt_start_demo();
    APP_LOG_INFO("exit mqtt task");
}

int app_mqtt_demo()
{
    while (true)
    {
        fibo_thread_create(mqtt_task, "mqtttask", 10 * 1024, NULL, OSI_PRIORITY_NORMAL + 1);

        fibo_taskSleep(10 * 1000);

        app_mqtt_stop_demo();
    }
}