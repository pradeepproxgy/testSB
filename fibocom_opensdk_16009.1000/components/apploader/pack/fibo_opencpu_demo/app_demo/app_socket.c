#include <stdint.h>

#include "app_socket.h"
#include "app_pdp.h"

#include "sockets.h"

static UINT32 g_socket_sem = 0;
static UINT32 g_socket_lock = 0;
static bool g_ntp_is_busy = false;

int app_socket_init()
{
    g_socket_sem = fibo_sem_new(0);
    if (g_socket_sem == 0)
    {
        APP_LOG_INFO("sem new fail");
        goto error;
    }

    g_socket_lock = fibo_mutex_create();
    if (g_socket_lock == 0)
    {
        APP_LOG_INFO("mutex new fail");
        goto error;
    }

    return 0;

error:
    fibo_mutex_delete(g_socket_lock);
    g_socket_lock = 0;
    fibo_sem_free(g_socket_sem);
    g_socket_sem = 0;
    return -1;
}

int app_socket_connect(int cid, app_socket_type_t sock_type, char *hostname, uint16_t port, uint32_t timeout)
{
    int ret = 0;
    int sock = 0;
    int non_block = 1;
    uint64_t now_tick = fibo_get_sys_tick();
    uint32_t timeoutleft = timeout;
    char *localipv4 = NULL;

    // 获取当前cid对应的地址
    ret = app_pdp_get_ipaddress(cid, &localipv4, NULL);
    if (ret != 0)
    {
        APP_LOG_INFO("get cid%d local address fail", cid);
        return ret;
    }

    fibo_addrinfo_t *local_addr = NULL;
    fibo_addrinfo_para localparas = {localipv4, 0, AF_INET, NULL, NULL, &local_addr, 0};
    ret = fibo_get_addrinfo(&localparas);
    if (ret != 0)
    {
        APP_LOG_INFO("get local addr(%s) fail %d", localipv4, ret);
        return ret;
    }

    // dns解析，如果是IP地址，直接转换成地址
    char *mydnssever1 = "114.114.114.114"; /// 自定义的DNS主服务器，如果不想自定义，这里置为NULL，则使用网络分配的DNS服务器
    char *mydnssever2 = "8.8.8.8"; /// 自定义的DNS备服务器，如果不想自定义，这里置为NULL，则使用网络分配的DNS服务器
    fibo_addrinfo_t *peer_addr = NULL;
    fibo_addrinfo_t *cur_addr = NULL;
    fibo_addrinfo_para paras = {hostname, port, AF_INET, mydnssever1, mydnssever2, &peer_addr, timeout};
    ret = fibo_get_addrinfo(&paras);
    if (ret != 0)
    {
        APP_LOG_INFO("dns resove fail %d", ret);
        return ret;
    }

    if (timeout > 0)
    {
        timeoutleft = timeoutleft - ((fibo_get_sys_tick() - now_tick) / 1000);
        if (timeoutleft < 0)
        {
            APP_LOG_INFO("dns resove timeout %d", (int)timeout);
            fibo_sock_freeaddrinfo(peer_addr);
            return ret;
        }
    }
    APP_LOG_INFO("dns take %lu", (unsigned long)(fibo_get_sys_tick() - now_tick));

    for (cur_addr = peer_addr; cur_addr; cur_addr = cur_addr->ai_next)
    {
        if (sock_type == APP_SOCKET_TYPE_TCP)
        {
            sock = fibo_sock_socket(peer_addr->ai_family, SOCK_STREAM, IPPROTO_TCP);
        }
        else
        {
            sock = fibo_sock_socket(peer_addr->ai_family, SOCK_DGRAM, IPPROTO_UDP);
        }
        if (sock < 0)
        {
            APP_LOG_INFO("create socket fail");
            continue;
        }

        ret = fibo_sock_bind_v2(sock, (struct sockaddr *)local_addr->ai_addr, local_addr->ai_addrlen);
        if (ret < 0)
        {
            APP_LOG_INFO("bind socket fail");
            goto error;
        }

        ret = fibo_sock_ioctl(sock, FIONBIO, &non_block);
        if (ret != 0)
        {
            APP_LOG_INFO("set socket to unblock fail");
            goto error;
        }

        struct timeval tm = {0};
        tm.tv_sec = timeoutleft / 1000;
        tm.tv_usec = (timeoutleft % 1000) * 1000;
        fd_set wset;
        FD_ZERO(&wset);
        FD_SET(sock, &wset);
        ret = fibo_sock_connect_v2(sock, (struct sockaddr *)cur_addr->ai_addr, cur_addr->ai_addrlen);
        if (ret != 0 || fibo_get_socket_error() == EINPROGRESS)
        {
            if (fibo_get_socket_error() != EINPROGRESS)
            {
                APP_LOG_INFO("connect failed:%s,%d", strerror(fibo_get_socket_error()), fibo_get_socket_error());
                goto error;
            }
            else
            {
                ret = fibo_sock_select(sock + 1, NULL, &wset, NULL, timeout > 0 ? &tm : NULL);
                if (ret < 0)
                {
                    APP_LOG_INFO("select failed:%s,%d", strerror(fibo_get_socket_error()), fibo_get_socket_error());
                    goto error;
                }
                else if (ret == 0)
                {
                    fibo_sock_close(sock);
                    APP_LOG_INFO("connect time out");
                    continue;
                }
                else if (ret > 0)
                {
                    if (!FD_ISSET(sock, &wset))
                    {
                        APP_LOG_INFO("connect error when select:%s,%d", strerror(fibo_get_socket_error()), fibo_get_socket_error());
                        goto error;
                    }
                }
            }
        }
        else
        {
            APP_LOG_INFO("tcp connect directly");
            break;
        }
    }

    fibo_sock_freeaddrinfo(peer_addr);
    fibo_sock_freeaddrinfo(local_addr);
    return sock;

error:
    fibo_sock_close(sock);
    fibo_sock_freeaddrinfo(peer_addr);
    fibo_sock_freeaddrinfo(local_addr);
    return -1;
}

int app_socket_send(int sock, const uint8_t *data, uint32_t len)
{
    int ret = 0;
    uint32_t sent = 0;
    struct timeval tm = {0};
    fd_set wset;

    const uint8_t *send_data = (const uint8_t *)data;
    while (sent < len)
    {
        ret = (int)fibo_sock_send_v2(sock, &send_data[sent], len - sent, 0);
        if (ret < 0)
        {
            if ((EAGAIN == fibo_get_socket_error()) || (EWOULDBLOCK == fibo_get_socket_error()))
            {
                APP_LOG_INFO("wait send buffer ava");
                FD_ZERO(&wset);
                FD_SET(sock, &wset);
                tm.tv_sec = 1;
                tm.tv_usec = 0;
                fibo_sock_select(sock + 1, NULL, &wset, NULL, &tm);
            }
            else
            {
                APP_LOG_INFO("send data failed:%s %d send=%d len=%u", strerror(fibo_get_socket_error()), fibo_get_socket_error(), (int)(len - sent), len);
                ret = -1;
                return ret;
            }
        }
        else
        {
            sent += (uint32_t)ret;
            APP_LOG_INFO("try send:send=%ld", sent);
        }
    }

    return ret;
}

int app_socket_recv(int sock, uint8_t *data, uint32_t size, uint32_t *len)
{
    int iret = fibo_sock_recv_v2(sock, data, size, 0);
    if (iret > 0)
    {
        *len = iret;
        return APP_SOCKET_RESULT_SUCC;
    }
    else if (iret == 0)
    {
        APP_LOG_INFO("socket close sock=%d", sock);
        return APP_SOCKET_RESULT_DISCONNECTTED;
    }
    else if ((EAGAIN == fibo_get_socket_error()) || (EWOULDBLOCK == fibo_get_socket_error()))
    {
        return APP_SOCKET_RESULT_READ_AGAIN;
    }
    else
    {
        APP_LOG_INFO("socket adnormal=%d", sock);
        return APP_SOCKET_RESULT_FAILED;
    }
}

int app_socket_recv_fixed(int sock, uint8_t *data, uint32_t size, uint32_t *len, int32_t timeout)
{
    int ret = 0;
    uint32_t recvlen = 0;
    uint32_t recvtotallen = 0;
    uint64_t now_tick = fibo_get_sys_tick();
    int32_t timeoutleft = timeout;
    *len = 0;

    while (recvtotallen < size)
    {
        now_tick = fibo_get_sys_tick();

        recvlen = 0;
        ret = app_socket_recv(sock, data + recvtotallen, size - recvtotallen, &recvlen);
        if (ret == APP_SOCKET_RESULT_SUCC)
        {
            recvtotallen += recvlen;
            APP_LOG_INFO("recv total=%lu", (unsigned long)recvtotallen);
        }
        else if (ret == APP_SOCKET_RESULT_READ_AGAIN)
        {
            struct timeval tm = {0};
            fd_set rset;
            APP_LOG_INFO("wait recv buffer ava");
            FD_ZERO(&rset);
            FD_SET(sock, &rset);
            tm.tv_sec = 1;
            tm.tv_usec = 0;
            fibo_sock_select(sock + 1, &rset, NULL, NULL, &tm);
        }
        else
        {
            APP_LOG_INFO("recv fail");
            *len = recvtotallen;
            return ret;
        }

        if (timeout > 0)
        {
            timeoutleft = timeoutleft - ((fibo_get_sys_tick() - now_tick) / 1000);
            if (timeoutleft < 0)
            {
                APP_LOG_INFO("recv timeout %d", (int)timeout);
                *len = recvtotallen;
                return APP_SOCKET_RESULT_TIMEOUT;
            }
        }
    }

    *len = recvtotallen;
    return APP_SOCKET_RESULT_SUCC;
}

static uint64_t *g_timestamp = NULL;
static int g_ntp_result = 0;
static void ntp_callback(void *param)
{
    fibo_mutex_lock(g_socket_lock);
    fibo_ntp_rsp *p_ntp_rsp = (fibo_ntp_rsp *)param;
    if (g_timestamp)
    {
        *g_timestamp = p_ntp_rsp->time;
    }
    g_ntp_result = p_ntp_rsp->result;
    APP_LOG_INFO("ntpresult=%d, time=%lld", p_ntp_rsp->result, (long long int)p_ntp_rsp->time);
    g_ntp_is_busy = false;
    fibo_mutex_unlock(g_socket_lock);
}

int app_socket_ntp(int cid, char *ntpserver, uint16_t port, uint64_t *timestamp)
{
    int simid = 0;

    fibo_mutex_lock(g_socket_lock);
    APP_WAIT_EXPECT_STATUS(g_socket_lock, g_socket_sem, g_ntp_is_busy, g_ntp_is_busy);
    g_timestamp = timestamp;
    fibo_multi_pdn_cfg(FIBO_E_NTP, cid, simid);
    int ret = fibo_sock_ntp(ntpserver, port, simid, ntp_callback);
    if (ret != 0)
    {
        APP_LOG_INFO("ntp fail");
        fibo_mutex_unlock(g_socket_lock);
        return ret;
    }

    g_ntp_is_busy = true;
    APP_WAIT_EXPECT_STATUS(g_socket_lock, g_socket_sem, g_ntp_is_busy, g_ntp_is_busy);
    fibo_mutex_unlock(g_socket_lock);

    return g_ntp_result ? 0 : -1;
}

static int g_sock_listen = -1;
static struct sockaddr_in g_server_listen_addr;
static uint32_t g_server_listen_addrlen = sizeof(g_server_listen_addr);
static void create_tcp_server(struct sockaddr *addr, int len)
{
    fibo_mutex_lock(g_socket_lock);
    if (g_sock_listen < 0)
    {
        int non_block = 1;
        g_sock_listen = fibo_sock_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        fibo_sock_ioctl(g_sock_listen, FIONBIO, &non_block);
        APP_LOG_INFO("this tcp pair");
        fibo_sock_bind_v2(g_sock_listen, addr, len);
        fibo_sock_getsockname(g_sock_listen, (struct sockaddr *)&g_server_listen_addr, &g_server_listen_addrlen);
        fibo_sock_listen_v2(g_sock_listen, 10);
    }
    fibo_mutex_unlock(g_socket_lock);
}

int app_socket_socketpair(app_socket_type_t sock_type, int sv[2])
{
    int non_block = 1;
    int sock_client = 0;
    int sock_server = 0;
    int sock_server_conn = 0;
    struct sockaddr_in client_addr = {0};
    struct sockaddr_in server_addr = {0};
    struct sockaddr_in server_connect_addr = {0};
    uint32_t client_addrlen = sizeof(client_addr);
    uint32_t server_addrlen = sizeof(server_addr);
    uint32_t server_connect_addrlen = sizeof(server_connect_addr);

    fibo_addrinfo_t *peer_addr = NULL;
    fibo_addrinfo_para paras = {"127.0.0.1", 0, AF_INET, NULL, NULL, &peer_addr, 10000};
    fibo_get_addrinfo(&paras);

    if (sock_type == APP_SOCKET_TYPE_TCP)
    {
        create_tcp_server((struct sockaddr *)peer_addr->ai_addr, peer_addr->ai_addrlen);

        sock_client = fibo_sock_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        fibo_sock_bind_v2(sock_client, (struct sockaddr *)peer_addr->ai_addr, peer_addr->ai_addrlen);
        fibo_sock_connect_v2(sock_client, (struct sockaddr *)&g_server_listen_addr, g_server_listen_addrlen);
        fibo_sock_ioctl(sock_client, FIONBIO, &non_block);
        APP_LOG_INFO("connect");

        fd_set rset;
        FD_ZERO(&rset);
        FD_SET(g_sock_listen, &rset);
        APP_LOG_INFO("wait accept");
        fibo_sock_select(g_sock_listen + 1, &rset, NULL, NULL, NULL);

        sock_server_conn = fibo_sock_accept_v2(g_sock_listen, (struct sockaddr *)&server_connect_addr, &server_connect_addrlen);
        fibo_sock_ioctl(sock_server_conn, FIONBIO, &non_block);
        APP_LOG_INFO("accept");
        sv[0] = sock_client;
        sv[1] = sock_server_conn;
    }
    else
    {
        sock_client = fibo_sock_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        sock_server = fibo_sock_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        fibo_sock_bind_v2(sock_client, (struct sockaddr *)peer_addr->ai_addr, peer_addr->ai_addrlen);
        fibo_sock_bind_v2(sock_server, (struct sockaddr *)peer_addr->ai_addr, peer_addr->ai_addrlen);
        fibo_sock_getsockname(sock_client, (struct sockaddr *)&client_addr, &client_addrlen);
        fibo_sock_getsockname(sock_server, (struct sockaddr *)&server_addr, &server_addrlen);
        fibo_sock_connect_v2(sock_client, (struct sockaddr *)&server_addr, server_addrlen);
        fibo_sock_ioctl(sock_server, FIONBIO, &non_block);
        fibo_sock_ioctl(sock_client, FIONBIO, &non_block);
        sv[0] = sock_client;
        sv[1] = sock_server;
    }
    fibo_sock_freeaddrinfo(peer_addr);

    return 0;
}

// static void sock_pairtest(void *arg)
// {
//     uint8_t data[32];
//     uint32_t size = sizeof(data);
//     uint32_t len;
//     int fd = *((int *)arg);

//     while (true)
//     {
//         int ret = app_socket_recv_fixed(fd, data, size, &len, 500);
//         if (ret == APP_SOCKET_RESULT_SUCC || ret == APP_SOCKET_RESULT_TIMEOUT)
//         {
//             APP_LOG_INFO("server recv %d %s", (int)len, (char *)data);
//         }
//     }
// }

void app_socket_demo()
{
    int cid = 1;
    uint32_t datalen = 0;
    int timeout = 10 * 1000; // 10s
    int sock = app_socket_connect(cid, APP_SOCKET_TYPE_TCP, "file.mofcom.gov.cn", 80, timeout);
    if (sock < 0)
    {
        APP_LOG_INFO("connect fail");
        return;
    }

    uint8_t content[] = "GET HTTP/1.1\r\nHost: file.mofcom.gov.cn\r\n\r\n";
    int ret = app_socket_send(sock, content, sizeof(content) - 1);
    if (ret < 0)
    {
        fibo_sock_close_v2(sock);
        APP_LOG_INFO("send fail");
        return;
    }

    uint8_t buf[800] = {0};
    ret = app_socket_recv_fixed(sock, buf, sizeof(buf) - 1, &datalen, 10000);
    if (ret < 0)
    {
        APP_LOG_INFO("recv fail");
        fibo_sock_close_v2(sock);
        return;
    }

    fibo_sock_close_v2(sock);
    APP_LOG_INFO("recv %d %s", (int)datalen, (char *)buf);

    uint64_t timestamp = 0;
    ret = app_socket_ntp(cid, "ntp1.aliyun.com", 123, &timestamp);
    if (ret < 0)
    {
        APP_LOG_INFO("recv fail");
        return;
    }
    APP_LOG_INFO("timestamp=%llu", (unsigned long long)timestamp);

    // int sockpair[2] = {0};
    // ret = app_socket_socketpair(APP_SOCKET_TYPE_TCP, sockpair);
    // if (ret < 0)
    // {
    //     APP_LOG_INFO("sock pair fail");
    //     return;
    // }
    // APP_LOG_INFO("get pair ok sv1=%d sv2=%d", sockpair[0], sockpair[1]);
    // fibo_thread_create(sock_pairtest, "sockpairtask", 10 * 1024, &sockpair[1], OSI_PRIORITY_NORMAL);

    // while (true)
    // {
    //     uint8_t senddata[] = "clientsenddataclientsenddata456";
    //     app_socket_send(sockpair[0], senddata, sizeof(senddata));
    //     fibo_taskSleep(1000);
    // }
}