/* Copyright (C) 2018 RDA Technologies Limited and/or its affiliates("RDA").
 * All rights reserved.
 *
 * This software is supplied "AS IS" without any warranties.
 * RDA assumes no responsibility or liability for the use of the software,
 * conveys no license or title under any patent, copyright, or mask work
 * right to the product. RDA reserves the right to make changes in the
 * software without notification.  RDA also make no representation or
 * warranty that such application will be suitable for the specified use
 * without further testing or modification.
 */
#include "at_cfg.h"
#ifdef CFW_GPRS_SUPPORT
#include "stdio.h"
#include "osi_api.h"
#include "osi_log.h"
#include "net_config.h"
#ifdef CONFIG_NET_USSL_SUPPORT
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>

#include "sockets.h"
#include "ssl_socket.h"
#include "vfs.h"

#define TLS_TASK_PRIORITY OSI_PRIORITY_NORMAL
#define TLS_TASK_STACK_SIZE (2048)

#define ERR_SUCCESS 0

#define min(a, b) ((a) < (b) ? (a) : (b))

//keepalive Parameters
int g_socketkeepalive = 0;
int g_socketkeepidle = 7200;
int g_socketkeepinterval = 75;
int g_socketkeepcount = 9;

typedef struct _TLS_CRT_T
{
    char *crt;
    int32_t len;
} TLS_CRT_T;

TLS_CONTEXT g_tls_context[TLS_CONTEXT_MAX_NUM] = {{30, 1, 0, 0XFFFF, MBEDTLS_SSL_MINOR_VERSION_0, MBEDTLS_SSL_MINOR_VERSION_3, 4}, {30, 1, 0, 0XFFFF, MBEDTLS_SSL_MINOR_VERSION_0, MBEDTLS_SSL_MINOR_VERSION_3, 4}, {30, 1, 0, 0XFFFF, MBEDTLS_SSL_MINOR_VERSION_0, MBEDTLS_SSL_MINOR_VERSION_3, 4}, {30, 1, 0, 0XFFFF, MBEDTLS_SSL_MINOR_VERSION_0, MBEDTLS_SSL_MINOR_VERSION_3, 4}, {30, 1, 0, 0XFFFF, MBEDTLS_SSL_MINOR_VERSION_0, MBEDTLS_SSL_MINOR_VERSION_3, 4}, {30, 1, 0, 0XFFFF, MBEDTLS_SSL_MINOR_VERSION_0, MBEDTLS_SSL_MINOR_VERSION_3, 4}};
CLIENT_CONTEXT g_client_context[TLS_CLIENT_MAX_NUM] = {0};

osiThread_t *g_tls_task_id = NULL;
static osiSemaphore_t *g_read_sema = NULL;

#define TLSSOCKET_TRACE(fmt, ...)                                 \
    do                                                            \
    {                                                             \
        sys_arch_printf("[mbedtls_socket] " #fmt, ##__VA_ARGS__); \
    } while (0)

int tcpip_Connect(uint32_t client_id);
struct netif *getGprsNetIf(uint8_t nSim, uint8_t nCid);
struct netif *getEtherNetIf(uint8_t nCid);
void _client_free(uint32_t client_id);
extern int32_t lwip_getRecvAvailSize(int nSocket);
int32_t tls_dataleft(uint32_t client_id);

static uint8_t *i8tostring(uint8_t value)
{
    static uint8_t buff[4];
    uint8_t b, s, g, i = 0;
    b = value / 100;
    s = value % 100 / 10;
    g = value % 100 % 10;
    if (b > 0)
        buff[i++] = '0' + b;
    if (b + s > 0)
        buff[i++] = '0' + s;
    buff[i++] = '0' + g;
    buff[i] = 0;
    return buff;
}

static void tls_ReadEvent(uint32_t client_id, uint32_t length)
{
    CLIENT_CONTEXT *client_context = &g_client_context[client_id];

    osiCallback_t func = client_context->notify_callback;
    if (func == NULL)
    {
        osiEvent_t ev = {0, 0, 0, 0};
        ev.id = EV_CFW_SSL_DATA_EVENT_IND;
        ev.param1 = length;
        ev.param2 = client_id;
        osiEventSend(client_context->notify_task, &ev);
    }
    else
    {
        TLSSOCKET_TRACE("++++Mbedsocket_ReadEvent,EV_CFW_TLSSOCK_READ_EVENT_IND");
        osiEvent_t *ev = malloc(sizeof(osiEvent_t));
        ev->id = EV_CFW_SSL_DATA_EVENT_IND;
        ev->param1 = length;
        ev->param2 = client_id;
        ev->param3 = (uint32_t)client_context->funcParam;
        osiThreadCallback(client_context->notify_task, func, ev);
    }
}

static void tls_DataSentEvent(uint32_t client_id)
{
    CLIENT_CONTEXT *client_context = &g_client_context[client_id];

    osiCallback_t func = client_context->notify_callback;
    if (func == NULL)
    {
        osiEvent_t ev = {0, 0, 0, 0};
        ev.id = EV_CFW_SSL_SENT_RSP_EVENT_IND;
        ev.param2 = client_id;

        osiEventSend(client_context->notify_task, &ev);
    }
    else
    {
        TLSSOCKET_TRACE("Mbedsocket_DataSentEvent,osiThreadCallback send event");
        osiEvent_t *ev = malloc(sizeof(osiEvent_t));
        ev->id = EV_CFW_SSL_SENT_RSP_EVENT_IND;
        ev->param1 = 0;
        ev->param2 = client_id;
        ev->param3 = (uint32_t)client_context->funcParam;
        osiThreadCallback(client_context->notify_task, func, ev);
    }
}
static void tls_ConnectEvent(uint32_t client_id, int32_t error_code)
{

    CLIENT_CONTEXT *client_context = &g_client_context[client_id];

    /*if (1 == g_tlsConnectStopFlag[ssid])
                {
                    TLSSOCKET_TRACE("Mbedsocket_ConnectEvent 1 == g_tlsConnectStopFlag !!!!");
                    return;
                }*/
    TLSSOCKET_TRACE("++++++tls_ConnectEvent send,error_code=%ld", error_code);
    osiCallback_t func = client_context->notify_callback;
    if (func == NULL)
    {
        osiEvent_t ev = {0, 0, 0, 0};
        ev.id = EV_CFW_SSL_CONNECT_RSP_EVENT_IND;
        ev.param1 = error_code;
        ev.param2 = client_id;
        osiEventSend(client_context->notify_task, &ev);
    }
    else
    {
        osiEvent_t *ev = malloc(sizeof(osiEvent_t));
        ev->id = EV_CFW_SSL_CONNECT_RSP_EVENT_IND;
        ev->param1 = error_code;
        ev->param2 = client_id;
        ev->param3 = (uint32_t)client_context->funcParam;
        osiThreadCallback((osiThread_t *)client_context->notify_task, func, ev);
    }
}

static void tls_CloseEvent(uint32_t client_id)
{
    CLIENT_CONTEXT *client_context = &g_client_context[client_id];

    osiCallback_t func = client_context->notify_callback;
    TLSSOCKET_TRACE("tls_CloseEvent ...");
    if (func == NULL)
    {
        osiEvent_t ev = {0, 0, 0, 0};
        ev.id = EV_CFW_SSL_CLOSE_RSP_EVENT_IND;
        ev.param2 = client_id;

        osiEventSend(client_context->notify_task, &ev);
    }
    else
    {
        osiEvent_t *ev = malloc(sizeof(osiEvent_t));
        ev->id = EV_CFW_SSL_CLOSE_RSP_EVENT_IND;

        ev->param2 = client_id;
        ev->param3 = (uint32_t)client_context->funcParam;
        osiThreadCallback(client_context->notify_task, func, ev);
    }
}
#if 0
static void tcpip_dnsReq_callback(void *paramCos)
{
    osiEvent_t *ev = (osiEvent_t *)paramCos;
    uint32_t client_id = ev->param3;
    CLIENT_CONTEXT *client_context = &g_client_context[client_id];

    TLSSOCKET_TRACE("tcpip_connect(), CFW_GethostbynameEX() tcpip_dnsReq_callback\n");
    if (ev->id == EV_CFW_DNS_RESOLV_SUC_IND)
    {
        int iResult = 0;
        ip_addr_t *ipaddr = (ip_addr_t *)ev->param1;
        char *pcIpAddr = ipaddr_ntoa(ipaddr);

        strncpy(client_context->uaIPAddress, pcIpAddr, AT_TCPIP_HOSTLEN);
        iResult = tcpip_Connect(client_id);
        if (iResult != ERR_SUCCESS)
        {
            TLSSOCKET_TRACE("tcpip_connect(), CFW_GethostbynameEX() tcpip_dnsReq_callback tcpip_Connect failed\n");
            return -1;
        }
    }
    else if (ev->id == EV_CFW_DNS_RESOLV_ERR_IND)
    {
        client_context->state = TLS_STATE_INITIAL;
    }
    free(ev);
}
#endif
static int32_t mbedtls_connect(CLIENT_CONTEXT *client_context)
{
    int ret = 0;
    uint32_t flags;

    client_context->server_fd.fd = client_context->uSocket;

    //mbedtls_net_set_block(ctx);
    mbedtls_net_set_block(&(client_context->server_fd));

    /** 2. Setup stuff **/

    mbedtls_ssl_set_bio(&(client_context->ssl), &(client_context->server_fd),
                        mbedtls_net_send, mbedtls_net_recv, mbedtls_net_recv_timeout);

    /** 3. Handshake **/
    TLSSOCKET_TRACE("TlsSock: Performing the SSL/TLS handshake...\r\n");

    while ((ret = mbedtls_ssl_handshake(&(client_context->ssl))) != 0)
    {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
        {
            TLSSOCKET_TRACE("TlsSock: failed --- mbedtls_ssl_handshake returned -0x%x", -ret);

            goto exit;
        }
    }
    TLSSOCKET_TRACE("TlsSock: Handshake ok\r\n");

    /** 4. Verify the server certificate **/
    TLSSOCKET_TRACE("TlsSock:  Verifying peer X.509 certificate...");

    /* In real life, we probably want to bail out when ret != 0 */
    if ((flags = mbedtls_ssl_get_verify_result(&(client_context->ssl))) != 0)
    {
        char vrfy_buf[512];

        TLSSOCKET_TRACE("TlsSock: Verifying peer X.509 certificate failed\n");

        mbedtls_x509_crt_verify_info(vrfy_buf, sizeof(vrfy_buf), "  ! ", flags);

        TLSSOCKET_TRACE("TlsSock: %s\n", vrfy_buf);
    }
    else
    {
        TLSSOCKET_TRACE("TlsSock: Verifying peer X.509 certificate ok\n");
    }
    if ((ret = mbedtls_ssl_get_session(&(client_context->ssl), &client_context->savedSession)) != 0)
    {
        TLSSOCKET_TRACE(" failed\n  ! mbedtls_ssl_get_session returned -0x%x\n\n", -ret);
        goto exit;
    }
exit:
    return ret;
}

static void tcpip_rsp(void *param)
{
    osiEvent_t *pEvent = (osiEvent_t *)param;
    TLSSOCKET_TRACE("tcpip_rsp Got : %ld,%ld,%ld,%ld",
                    pEvent->id, pEvent->param1, pEvent->param2, pEvent->param3);

    uint8_t uSocket = (uint8_t)pEvent->param1;
    uint32_t client_id = pEvent->param3;
    CLIENT_CONTEXT *client_context = &g_client_context[client_id];

    switch (pEvent->id)
    {
    case EV_CFW_TCPIP_SOCKET_CONNECT_RSP:
        if (client_context->uSocket == uSocket)
        {
            int ret;
            CFW_TcpipSocketSetParam(uSocket, NULL, client_id);

            ret = mbedtls_connect(client_context);
            CFW_TcpipSocketSetParam(uSocket, (osiCallback_t)tcpip_rsp, client_id);

            tls_ConnectEvent(client_id, ret); //TLS connect ok
            if (ret != 0)
            {
                _client_free(client_id);
                if (g_client_context[client_id].uSocket != INVALID_SOCKET)
                {

                    CFW_TcpipSocketClose(g_client_context[client_id].uSocket);
                }
                else
                    tls_CloseEvent(client_id);
            }
        }
        break;

    case EV_CFW_TCPIP_SOCKET_CLOSE_RSP:
        TLSSOCKET_TRACE("RECEIVED EV_CFW_TCPIP_SOCKET_CLOSE_RSP ...");
        tls_CloseEvent(client_id);
        //tlssock_ptr->server_fd.fd = INVALID_SOCKET;
        if (g_client_context[client_id].isUsed == 1)
            _client_free(client_id);
        int i = 0;
        for (i = 0; i < TLS_CLIENT_MAX_NUM; i++)
        {
            if (g_client_context[i].isUsed == 1)
                break;
        }
        if (i >= TLS_CLIENT_MAX_NUM)
        {
            osiEvent_t ev = {0, 0, 0, 0};
            ev.id = EV_CFW_TLS_FREE_REQ;
            ev.param2 = client_id;
            osiEventSend(g_tls_task_id, &ev);
        }
        break;
    case EV_CFW_TCPIP_ERR_IND:
    case EV_CFW_TCPIP_BEARER_LOSING_IND:
        if (client_context->uSocket == uSocket)
        {
            TLSSOCKET_TRACE("pEvent->id:%ld", pEvent->id);
            int8_t err = (int8_t)pEvent->param2;
            const char *string = lwip_strerr(err);

            //if (client_context->server_fd.fd != INVALID_SOCKET)
            //{
            //CFW_TcpipSocketClose(client_context->server_fd.fd);
            //}
            TLSSOCKET_TRACE("EV_CFW_TCPIP_ERR_IND:%s", string);
            tls_ConnectEvent(client_id, (int32_t)err);

            _client_free(client_id);
            if (g_client_context[client_id].uSocket != INVALID_SOCKET)
            {

                //CFW_TcpipSocketClose(g_client_context[client_id].uSocket);
                tls_Close(client_id);
            }
            else
                tls_CloseEvent(client_id);
        }
        break;

    case EV_CFW_TCPIP_CLOSE_IND:
        TLSSOCKET_TRACE("EV_CFW_TCPIP_CLOSE_IND");
        if (client_context->uSocket == uSocket)
        {
            //if (client_context->server_fd.fd != INVALID_SOCKET)
            //{
            // CFW_TcpipSocketClose(client_context->server_fd.fd);
            //}
            /*tlssock_ptr->server_fd.fd = INVALID_SOCKET;*/
            //if (client_context->state == TLSSOCK_STATE_CONNECTING)
            //{
            tls_ConnectEvent(client_id, -1);

            ///_client_free(client_id);
            if (g_client_context[client_id].uSocket != INVALID_SOCKET)
            {
                // CFW_TcpipSocketClose(g_client_context[client_id].uSocket);
                tls_Close(client_id);
            }
            else
                tls_CloseEvent(client_id);

            //}
            /*if (client_context->state == TLSSOCK_STATE_CONNECTED)
            {
                Mbedsocket_CloseEvent((TLSSOCK_HANDLE)tlssock_ptr);
            }*/
            TLSSOCKET_TRACE("EV_CFW_TCPIP_CLOSE_IND");
        }
        break;

    case EV_CFW_TCPIP_SOCKET_SEND_RSP:
        if (client_context->uSocket == uSocket)
        {

            TLSSOCKET_TRACE("goto Mbedsocket_DataSentEvent");
            tls_DataSentEvent(client_id);
        }
        TLSSOCKET_TRACE("EV_CFW_TCPIP_SOCKET_SEND_RSP");
        break;

    case EV_CFW_TCPIP_REV_DATA_IND:
        TLSSOCKET_TRACE("EV_CFW_TCPIP_REV_DATA_IND");
        if (client_context->uSocket == uSocket)
        {
            client_context->dataEventHandler = 0;
#if 0
            uint16_t uDataSize = 0;
            //mbedtls_ssl_read(&client_context->ssl, , )
            uDataSize = (uint16_t)pEvent->param2;
            tls_ReadEvent(client_id, uDataSize);
#endif
            osiSemaphoreAcquire(client_context->tlsbuf_sem);
            uint32_t data_left = client_context->databuf.put - client_context->databuf.get;
            uint32_t space_left = PLAIN_TEXT_RINGBUF_SIZE - data_left;
            if (space_left > 0)
            {
                TLSSOCKET_TRACE("EV_CFW_TCPIP_REV_DATA_IND,space_left=%ld", space_left);
                char data[PLAIN_TEXT_RINGBUF_SIZE] = {0};
                mbedtls_ssl_set_bio(&client_context->ssl, &(client_context->server_fd), mbedtls_net_send, mbedtls_net_recv, mbedtls_net_recv_timeout);
                int32_t read_len = mbedtls_ssl_read(&(client_context->ssl), (uint8_t *)data, space_left);

                TLSSOCKET_TRACE("EV_CFW_TCPIP_REV_DATA_IND,read_len=%ld", read_len);
                if (read_len > 0)
                {
                    TLSSOCKET_TRACE("EV_CFW_TCPIP_REV_DATA_IND,read_len=%ld,client_context->databuf.get=%ld,client_context->databuf.put=%ld", read_len, client_context->databuf.get, client_context->databuf.put);
                    if (client_context->databuf.get == 0)
                    {
                        memcpy(client_context->databuf.buf + client_context->databuf.put, data, read_len);
                        client_context->databuf.put += read_len;
                    }
                    else
                    {
                        memcpy(client_context->databuf.buf, client_context->databuf.buf + client_context->databuf.get, data_left);
                        memcpy(client_context->databuf.buf + data_left, data, read_len);
                        client_context->databuf.put = data_left + read_len;
                    }
                    client_context->databuf.get = 0;

                    tls_ReadEvent(client_id, read_len);
                }
            }

            osiSemaphoreRelease(client_context->tlsbuf_sem);
        }
        break;
    default:
        TLSSOCKET_TRACE("unexpect asynchrous event/response %ld", pEvent->id);
        break;
    }
    free(pEvent);
}

int tcpip_Connect(uint32_t client_id)
{
    int iResult;
    int iResultBind;
    //uint8_t nSim = handle->nSimid;// TODO :SIMID
    union sockaddr_aligned nDestAddr;
    CLIENT_CONTEXT *client_context = &(g_client_context[client_id]);

    client_context->state = TLS_STATE_OPENING;
    ip_addr_t nIpAddr;
    //IP4_ADDR(&nIpAddr, 111, 205, 140, 139);

    iResult = CFW_GethostbynameEX(client_context->uaIPAddress, &nIpAddr, client_context->nCid, client_context->nSimid, NULL, (void *)NULL);
    //iResult = RESOLV_COMPLETE;
    if (iResult == RESOLV_QUERY_INVALID)
    {
        TLSSOCKET_TRACE("tcpip_connect gethost by name error");
        client_context->state = TLS_STATE_INITIAL;
        return -1;
    }
    else if (iResult == RESOLV_COMPLETE)
    {
        TLSSOCKET_TRACE("tcpip_connect, CFW_GethostbynameEX() RESOLV_COMPLETE\n");

        client_context->uSocket = INVALID_SOCKET;
        struct netif *netif = NULL;
        struct ifreq iface;
        memset(&iface, 0, sizeof(struct ifreq));
        if (client_context->nCid == 0x11)
            netif = getEtherNetIf(client_context->nCid);
        else
            netif = getGprsNetIf(client_context->nSimid, client_context->nCid);
        if (netif == NULL)
        {
            client_context->state = TLS_STATE_INITIAL;
            return -1;
        }

        iface.ifr_name[0] = netif->name[0];
        iface.ifr_name[1] = netif->name[1];
        strcat(iface.ifr_name, (const char *)i8tostring(netif->num));
        TLSSOCKET_TRACE("tcpip_connect() netif:%s\n", iface.ifr_name);

#if LWIP_IPV4
        if (IP_IS_V4(&(nIpAddr)))
        {
            struct sockaddr_in *to4 = (struct sockaddr_in *)&(nDestAddr);
            to4->sin_len = sizeof(struct sockaddr_in);
            to4->sin_family = AF_INET;
            to4->sin_port = htons(client_context->fport);
            inet_addr_from_ip4addr(&to4->sin_addr, ip_2_ip4(&(nIpAddr)));
            if (client_context->isDtls == 0)
            {
                iResult = CFW_TcpipSocketEX(AF_INET, CFW_TCPIP_SOCK_STREAM, CFW_TCPIP_IPPROTO_TCP, tcpip_rsp, (uint32_t)client_id);
            }
            else
            {
                iResult = CFW_TcpipSocketEX(AF_INET, CFW_TCPIP_SOCK_DGRAM, CFW_TCPIP_IPPROTO_UDP, tcpip_rsp, (uint32_t)client_id);
            }
            if (SOCKET_ERROR == iResult)
            {
                TLSSOCKET_TRACE("tcpip_connect(), CFW_TcpipSocket() failed\n");
                client_context->state = TLS_STATE_INITIAL;
                return iResult;
            }
            int n = 1;
            CFW_TcpipSocketSetsockopt(iResult, SOL_SOCKET, SO_REUSEADDR, (void *)&n, sizeof(n));
            ip4_addr_t *ip_addr = (ip4_addr_t *)netif_ip4_addr(netif);

            struct sockaddr_in stLocalAddr = {0};
            stLocalAddr.sin_len = sizeof(struct sockaddr_in);
            stLocalAddr.sin_family = CFW_TCPIP_AF_INET;
            /*if (tcpipParas->uProtocol == CFW_TCPIP_IPPROTO_UDP)
                stLocalAddr.sin_port = htons(gClocalport[nMuxIndex].udp_port);
            else
                stLocalAddr.sin_port = htons(gClocalport[nMuxIndex].tcp_port);*/
            stLocalAddr.sin_addr.s_addr = ip_addr->addr;

            iResultBind = CFW_TcpipSocketBind(iResult, (CFW_TCPIP_SOCKET_ADDR *)&stLocalAddr, sizeof(CFW_TCPIP_SOCKET_ADDR));
            if (SOCKET_ERROR == iResultBind)
            {
                TLSSOCKET_TRACE("tcpip_connect(), CFW_TcpipSocketBind() failed\n");
                int iRetValue;
                iRetValue = CFW_TcpipSocketClose(iResult);
                if (iRetValue == ERR_SUCCESS)
                {
                    TLSSOCKET_TRACE("tcpip_connect(): close success");
                }
                else
                {
                    TLSSOCKET_TRACE("tcpip_connect(): CSW execute wrong");
                }
                return iResultBind;
            }
        }
#endif

#if LWIP_IPV6
        if (IP_IS_V6(&(nIpAddr)))
        {
            struct sockaddr_in6 *to6 = (struct sockaddr_in6 *)&(nDestAddr);
            to6->sin6_len = sizeof(struct sockaddr_in6);
            to6->sin6_family = AF_INET6;
            to6->sin6_port = htons(client_context->fport);
            inet6_addr_from_ip6addr(&to6->sin6_addr, ip_2_ip6(&(nIpAddr)));
            if (client_context->isDtls == 0)
            {
                iResult = CFW_TcpipSocketEX(AF_INET6, CFW_TCPIP_SOCK_STREAM, CFW_TCPIP_IPPROTO_TCP, tcpip_rsp, (uint32_t)client_id);
            }
            else
            {
                iResult = CFW_TcpipSocketEX(AF_INET6, CFW_TCPIP_SOCK_DGRAM, CFW_TCPIP_IPPROTO_UDP, tcpip_rsp, (uint32_t)client_id);
            }
            if (SOCKET_ERROR == iResult)
            {
                TLSSOCKET_TRACE("tcpip_connect(), CFW_TcpipSocket() failed\n");
                client_context->state = TLS_STATE_INITIAL;
                return iResult;
            }
            int n = 1;
            CFW_TcpipSocketSetsockopt(iResult, SOL_SOCKET, SO_REUSEADDR, (void *)&n, sizeof(n));
            ip6_addr_t *ip_addr = (ip6_addr_t *)netif_ip6_addr(netif, 0);
            ip6_addr_t *ip_addr_1 = (ip6_addr_t *)netif_ip6_addr(netif, 1);
            if (ip6_addr_isany_val(*(ip_addr_1)) != true)
            {
                ip_addr = ip_addr_1;
            }
            struct sockaddr_in6 stLocalAddr = {0};
            stLocalAddr.sin6_len = sizeof(struct sockaddr_in6);
            stLocalAddr.sin6_family = AF_INET6;
            /*if (tcpipParas->uProtocol == CFW_TCPIP_IPPROTO_UDP)
                stLocalAddr.sin6_port = htons(gClocalport[nMuxIndex].udp_port);
            else
                stLocalAddr.sin6_port = htons(gClocalport[nMuxIndex].tcp_port);*/

            inet6_addr_from_ip6addr(&stLocalAddr.sin6_addr, ip_addr);
            iResultBind = CFW_TcpipSocketBind(iResult, (CFW_TCPIP_SOCKET_ADDR *)&stLocalAddr, sizeof(struct sockaddr_in6));
            if (SOCKET_ERROR == iResultBind)
            {
                CFW_TcpipSocketClose(iResult);
                TLSSOCKET_TRACE("tcpip_connect(), CFW_TcpipSocketBind() failed\n");
                return iResultBind;
            }
        }
#endif

        char *pcIpAddr = ipaddr_ntoa(&nIpAddr);
        strncpy(client_context->uaIPAddress, pcIpAddr, AT_TCPIP_HOSTLEN);
        client_context->uSocket = iResult;

        if (g_socketkeepalive == 1)
        {
            CFW_TcpipSocketSetsockopt(iResult, SOL_SOCKET, SO_KEEPALIVE, (void *)&g_socketkeepalive, sizeof(g_socketkeepalive));
            CFW_TcpipSocketSetsockopt(iResult, IPPROTO_TCP, TCP_KEEPIDLE, (void *)&g_socketkeepidle, sizeof(g_socketkeepidle));
            CFW_TcpipSocketSetsockopt(iResult, IPPROTO_TCP, TCP_KEEPINTVL, (void *)&g_socketkeepinterval, sizeof(g_socketkeepinterval));
            CFW_TcpipSocketSetsockopt(iResult, IPPROTO_TCP, TCP_KEEPCNT, (void *)&g_socketkeepcount, sizeof(g_socketkeepcount));
        }
        else
        {
            CFW_TcpipSocketSetsockopt(iResult, SOL_SOCKET, SO_KEEPALIVE, (void *)&g_socketkeepalive, sizeof(g_socketkeepalive));
        }
        //if(client_context->isDtls == 0)
        iResult = CFW_TcpipSocketConnect(client_context->uSocket, (CFW_TCPIP_SOCKET_ADDR *)&nDestAddr, sizeof(nDestAddr));
        if (SOCKET_ERROR == iResult)
        {
            TLSSOCKET_TRACE("tcpip_connect, CFW_TcpipSocketConnect() failed\n");
            return iResult;
        }
        //atCmdSetTimeoutHandler(engine, 60000, _timeoutabortTimeout);
    }
    else if (iResult == RESOLV_QUERY_QUEUED)
    {
        //atCmdSetTimeoutHandler(engine, 60000, _timeoutabortTimeout);
        return ERR_SUCCESS;
    }
    return ERR_SUCCESS;
}

static void tls_debug(void *ctx, int level,
                      const char *file, int line,
                      const char *str)
{
    //((void) level);
    sys_arch_printf("%s:%04d: %s", file, line, str);
}
#if 0
static const int ciphersuite_preference[] =
    {
        MBEDTLS_TLS_RSA_WITH_AES_128_CBC_SHA,
        MBEDTLS_TLS_RSA_WITH_RC4_128_SHA,
        MBEDTLS_TLS_RSA_WITH_RC4_128_MD5,
        MBEDTLS_TLS_RSA_WITH_NULL_SHA};
#endif
static uint8_t caCrt_filename[] = "/caCrt.pem";
static uint8_t clientCrt_filename[] = "/clientCrt.pem";
static uint8_t clientPrivateKey_filename[] = "/privateKey.pem";

#define TLS_CA_CRT 0
#define TLS_CLIENT_CRT 1
#define TLS_CLIENT_PRIVATE_KEY 2

static uint8_t *ca_pem = NULL;
static uint8_t *cert_pem = NULL;
static uint8_t *key_pem = NULL;

static bool Write_to_file(uint8_t *filename, char *pemData, uint32_t pemLen)
{
    TLSSOCKET_TRACE("enter  Write_to_file. file: %s,pemLen:%ld \n", filename, pemLen);

    int fd = -1;
    fd = vfs_open((const char *)filename, O_WRONLY | O_TRUNC, 0);
    if (fd < 0)
    {
        fd = vfs_open((const char *)filename, O_CREAT | O_WRONLY | O_TRUNC);
        if (fd < 0)
        {
            TLSSOCKET_TRACE("[%s]openfailing.h=%d", filename, fd);
            return false;
        }
    }

    if (vfs_write(fd, pemData, pemLen) != pemLen)
    {
        TLSSOCKET_TRACE("write [%s] error", filename);
        vfs_close(fd);
        return false;
    }
    vfs_close(fd);
    return true;
}

bool tls_saveCrttoFile(char *pemData, uint32_t pemLen, uint32_t pemtype)
{
    TLSSOCKET_TRACE("enter  tls_saveCrttoFile. pemLen:%ld,pemtype:%ld \n", pemLen, pemtype);

    switch (pemtype)
    {
    case TLS_CA_CRT: //set ca certificate filename
        return Write_to_file(caCrt_filename, pemData, pemLen);
    case TLS_CLIENT_CRT: //set client certificate filename
        return Write_to_file(clientCrt_filename, pemData, pemLen);
    case TLS_CLIENT_PRIVATE_KEY: //set client private key filename
        return Write_to_file(clientPrivateKey_filename, pemData, pemLen);
    default:
        TLSSOCKET_TRACE("pemtype ERROR \n");
        return false;
    }
}

static bool Read_from_file(uint8_t *filename, uint8_t **crtPem)
{
    TLSSOCKET_TRACE("filename:%s. \n", filename);
    int fd = -1;
    uint32_t fileSize = 0;
    int result;
    //ileSize = vfs_file_size(filename);
    fd = vfs_open((const char *)filename, O_RDONLY, 0);
    if (fd < 0)
    {
        //Http_WriteUart("ERROR! not set certificate.\n",strlen("ERROR! not set certificate.\n"));
        //Http_WriteUart("\n",strlen("\n"));
        TLSSOCKET_TRACE("vfs_open error,fd:%d\n", fd);
        return false;
    }
    struct stat st;
    vfs_fstat(fd, &st);
    fileSize = st.st_size;
    TLSSOCKET_TRACE("fd:%d,vfs_file_size:%ld\n", fd, fileSize);
    if (fileSize < 0)
    {
        TLSSOCKET_TRACE("fileSize error\n");
        vfs_close(fd);
        return false;
    }
    if (*crtPem != NULL)
    {
        TLSSOCKET_TRACE("free crtPem!! \n");
        free(*crtPem);
        *crtPem = NULL;
    }
    *crtPem = (uint8_t *)malloc(fileSize + 1);
    if (NULL == *crtPem)
    {
        TLSSOCKET_TRACE("malloc error!! \n");
        vfs_close(fd);
        return false;
    }
    memset(*crtPem, 0, fileSize + 1);
    // Read  crt data from file to buffer
    result = vfs_read(fd, *crtPem, fileSize);
    if (result != fileSize)
    {
        TLSSOCKET_TRACE("not read all data!! \n");
        free(*crtPem);
        *crtPem = NULL;
        vfs_close(fd);
        return false;
    }
    vfs_close(fd);
    return true;
}

bool tls_setCrt(uint32_t pemtype, uint8_t **crtPem)
{
    TLSSOCKET_TRACE("+++++++enter  tls_setCrt. pemtype:%ld \n", pemtype);

    switch (pemtype)
    {
    case TLS_CA_CRT: //set ca certificate
        return Read_from_file(caCrt_filename, crtPem);
    case TLS_CLIENT_CRT: //set client certificate
        return Read_from_file(clientCrt_filename, crtPem);
    case TLS_CLIENT_PRIVATE_KEY: //set client private key
        return Read_from_file(clientPrivateKey_filename, crtPem);
    default:
        TLSSOCKET_TRACE("pemtype ERROR \n");
        return false;
    }
}

static int32_t mbedtls_init(uint32_t ssid, uint32_t client_id, uint8_t isDtls, char *psk_id, char *psk)
{
    TLS_CONTEXT *tls_context = &g_tls_context[ssid];
    CLIENT_CONTEXT *client_context = &g_client_context[client_id];
    const char *pers = "tls_socket";
    //char *pBuf = NULL;
    int ret;
    //sprd_crypto_init();
    client_context->server_fd.fd = INVALID_SOCKET;

    /* Initialize the RNG and the session data */
    TLSSOCKET_TRACE("mbedtls_init: mbedtls init start!!!");
    client_context->isUsed = 1;
    client_context->isDtls = isDtls;
    mbedtls_net_init(&(client_context->server_fd));
    mbedtls_ssl_init(&(client_context->ssl));
    mbedtls_ctr_drbg_init(&(client_context->ctr_drbg));
    mbedtls_entropy_init(&(client_context->entropy));

    if ((ret = mbedtls_ctr_drbg_seed(&(client_context->ctr_drbg), mbedtls_entropy_func,
                                     &(client_context->entropy), (const unsigned char *)pers,
                                     strlen(pers))) != 0)
    {
        TLSSOCKET_TRACE("mbedtls_init: failed --- mbedtls_ctr_drbg_seed returned %d", ret);
        //free(pBuf);
        return ret;
    }

    mbedtls_ssl_config_init(&(client_context->conf));

    if (isDtls)
    {
        if ((ret = mbedtls_ssl_config_defaults(&(client_context->conf),
                                               MBEDTLS_SSL_IS_CLIENT,
                                               MBEDTLS_SSL_TRANSPORT_DATAGRAM,
                                               MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
        {
            TLSSOCKET_TRACE("mbedtls_init: failed --- mbedtls_ssl_config_defaults returned %d", ret);
            return ret;
        }
        client_context->timer = malloc(sizeof(mbedtls_timing_delay_context));
        if (client_context->timer == NULL)
            TLSSOCKET_TRACE("get_dtls_context timer malloc failed:%d\n", sizeof(mbedtls_timing_delay_context));
        mbedtls_ssl_set_timer_cb(&(client_context->ssl), client_context->timer, mbedtls_timing_set_delay,
                                 mbedtls_timing_get_delay);

        int i;
        int len_tmp = 0;
        len_tmp = strlen(psk) % 2;
        int pskLen = len_tmp ? (strlen(psk) / 2 + 1) : (strlen(psk) / 2);
        char *pskBuffer = malloc(pskLen);

        if (NULL == pskBuffer)
        {
            TLSSOCKET_TRACE("Failed to create PSK binary buffer");
            return -1;
        }
        memset(pskBuffer, 0, pskLen);
        // Hex string to binary
        char *h = psk;
        char *b = pskBuffer;
        char xlate[] = "0123456789ABCDEF";
        if (len_tmp == 1)
        {
            char *l = strchr(xlate, toupper((unsigned char)(*h)));
            if (!l)
            {
                TLSSOCKET_TRACE("Failed to parse Pre-Shared-Key HEXSTRING");
                return -1;
            }
            *b = l - xlate; //((l - xlate) << 4);//
            h++;
            b++;
        }
        for (i = 0; i < (pskLen - len_tmp); h += 2, ++b, i++)
        {
            char *l = strchr(xlate, toupper((unsigned char)(*h)));
            char *r = strchr(xlate, toupper((unsigned char)(*(h + 1))));

            if (!r || !l)
            {
                TLSSOCKET_TRACE("Failed to parse Pre-Shared-Key HEXSTRING");
                return -1;
            }

            *b = ((l - xlate) << 4) + (r - xlate);
        }
        if ((ret = mbedtls_ssl_conf_psk(&(client_context->conf), (const unsigned char *)pskBuffer, pskLen,
                                        (const unsigned char *)psk_id,
                                        strlen(psk_id))) != 0)
        {

            TLSSOCKET_TRACE(" failed! mbedtls_ssl_conf_psk returned %d\n\n", ret);
            return ret;
        }
    }
    else
    {
        if ((ret = mbedtls_ssl_config_defaults(&(client_context->conf),
                                               MBEDTLS_SSL_IS_CLIENT,
                                               MBEDTLS_SSL_TRANSPORT_STREAM,
                                               MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
        {
            TLSSOCKET_TRACE("mbedtls_init: failed --- mbedtls_ssl_config_defaults returned %d", ret);
            return ret;
        }

        client_context->authmode = tls_context->authmode;
        if (0 == tls_context->authmode)
            mbedtls_ssl_conf_authmode(&client_context->conf, MBEDTLS_SSL_VERIFY_NONE);
        else
            mbedtls_ssl_conf_authmode(&client_context->conf, MBEDTLS_SSL_VERIFY_REQUIRED);

        //mbedtls_ssl_conf_ciphersuites(&client_context->conf, (const int *)tls_context->conf->ciphersuit); //TODO
        mbedtls_ssl_conf_min_version(&client_context->conf, MBEDTLS_SSL_MAJOR_VERSION_3, (int)tls_context->min_ver);
        mbedtls_ssl_conf_max_version(&client_context->conf, MBEDTLS_SSL_MAJOR_VERSION_3, (int)tls_context->max_ver);

        mbedtls_ssl_conf_read_timeout(&(client_context->conf), tls_context->timeout * 1000);
        mbedtls_ssl_conf_ignore_time(&(client_context->conf), tls_context->ignore_time);

        mbedtls_x509_crt_init(&(client_context->cacert));
        mbedtls_x509_crt_init(&(client_context->clicert));

        mbedtls_pk_init(&(client_context->pkey));

        bool result = false;
        //set caCrt through read certificate from file
        result = tls_setCrt(TLS_CA_CRT, &ca_pem);
        if (!result || NULL == ca_pem)
        {
            TLSSOCKET_TRACE("caCrt set fail!");
            //return -1;
        }
        //set clientCrt through read certificate from file
        result = tls_setCrt(TLS_CLIENT_CRT, &cert_pem);
        if (!result || NULL == cert_pem)
        {
            TLSSOCKET_TRACE("clientCrt set fail!");
            //return -1;
        }
        //set caCrt through read certificate from file
        result = tls_setCrt(TLS_CLIENT_PRIVATE_KEY, &key_pem);
        if (!result || NULL == key_pem)
        {
            free(cert_pem);
            cert_pem = NULL;
            TLSSOCKET_TRACE("client private key set fail!");
            //return -1;
        }

        ret = mbedtls_x509_crt_parse(&(client_context->cacert), (const unsigned char *)ca_pem,
                                     (ca_pem == NULL) ? 0 : (strlen((const char *)ca_pem) + 1));
        TLSSOCKET_TRACE("mbedtls_x509_crt_parse ca_pem failed...,ret=0x%x\n", -ret);

        if (ret < 0)
        {
            free(ca_pem);
            ca_pem = NULL;
        }
        else
        {
            mbedtls_ssl_conf_ca_chain(&client_context->conf, &client_context->cacert, NULL);
        }
        if (client_context->authmode == 2)
        {
            if (NULL != key_pem && NULL != cert_pem)
            {
                ret = mbedtls_x509_crt_parse(&(client_context->clicert), (const unsigned char *)cert_pem,
                                             (cert_pem == NULL) ? 0 : (strlen((const char *)cert_pem) + 1));
                TLSSOCKET_TRACE("mbedtls_x509_crt_parse cert_pem failed...,ret=0x%x\n", -ret);

                if (ret < 0)
                {
                    free(cert_pem);
                    cert_pem = NULL;
                    //return ret;
                }
            }
            if (NULL != key_pem && NULL != cert_pem)
            {
                ret = mbedtls_pk_parse_key(&(client_context->pkey), (const unsigned char *)key_pem,
                                           (key_pem == NULL) ? 0 : (strlen((const char *)key_pem) + 1), NULL, 0);

                TLSSOCKET_TRACE("mbedtls_pk_parse_key key_pem failed...,ret=0x%x\n", -ret);

                if (ret < 0)
                {
                    free(key_pem);
                    key_pem = NULL;
                    //return ret;
                }
            }
            ret = mbedtls_ssl_conf_own_cert(&client_context->conf, &client_context->clicert, &client_context->pkey);
            TLSSOCKET_TRACE("mbedtls_ssl_conf_own_cert failed...,ret=0x%x\n", -ret);
            if (ret < 0)
            {
                free(cert_pem);
                cert_pem = NULL;
                free(key_pem);
                key_pem = NULL;
            }
        }
        if (ca_pem != NULL)
        {
            free(ca_pem);
            ca_pem = NULL;
            TLSSOCKET_TRACE("free ca_pem");
        }
        if (cert_pem != NULL)
        {
            free(cert_pem);
            cert_pem = NULL;
            TLSSOCKET_TRACE("free cert_pem");
        }
        if (key_pem != NULL)
        {
            free(key_pem);
            key_pem = NULL;
            TLSSOCKET_TRACE("free key_pem");
        }
    }
    mbedtls_ssl_conf_rng(&(client_context->conf), mbedtls_ctr_drbg_random, &(client_context->ctr_drbg));
    if (tls_context->ciphersuit != 0xffff)
        mbedtls_ssl_conf_ciphersuites(&(client_context->conf), (const int *)&(tls_context->ciphersuit));
#if defined(MBEDTLS_SSL_SESSION_TICKETS)
    mbedtls_ssl_conf_session_tickets(&(client_context->conf), MBEDTLS_SSL_SESSION_TICKETS_DISABLED);
#endif
    mbedtls_ssl_conf_dbg(&client_context->conf, tls_debug, NULL);
    mbedtls_debug_set_threshold(5);

    if ((ret = mbedtls_ssl_setup(&(client_context->ssl), &(client_context->conf))) != 0)
    {
        TLSSOCKET_TRACE("TlsSock: failed --- mbedtls_ssl_setup returned %d", ret);
        //ret = TLSSOCK_ERR_SSL_SETUP_FAIL;
        return -1;
    }

    //mbedtls_platform_set_printf(sys_arch_printf);

#if 0
            /**  Initialize CA certificates **/
            TLSSOCKET_TRACE("MbedtlsSock: Loading the CA root certificate ...len=%d", strlen(iotx_ca_crt) + 1);
            TLSSOCKET_TRACE("MbedtlsSock: +++&(tlssock_ptr->cacert)=%p,tlssock_ptr=%p", &(tlssock_ptr->cacert), tlssock_ptr);
        
            ret = mbedtls_x509_crt_parse(&(tlssock_ptr->cacert),
                                         (const unsigned char *)iotx_ca_crt,
                                         strlen(iotx_ca_crt) + 1);
            if (ret < 0)
            {
                TLSSOCKET_TRACE("MbedtlsSock: failed --- mbedtls_x509_crt_parse returned -0x%x", -ret);
                //free(pBuf);
                return -1;
            }
        
            ret = mbedtls_x509_crt_parse(&(tlssock_ptr->clicert),
                                         (const unsigned char *)client_cert,
                                         strlen(client_cert) + 1);
            if (ret < 0)
            {
                TLSSOCKET_TRACE("TLSSOCK_CFG_TYPE_CLI_CERT: failed --- mbedtls_x509_crt_parse returned -0x%x", -ret);
                return -1;
            }
        
            ret = mbedtls_pk_parse_key(&(tlssock_ptr->pkey), (const unsigned char *)client_key,
                                       strlen(client_key) + 1, NULL, 0);
            if (ret < 0)
            {
                TLSSOCKET_TRACE("TLSSOCK_CFG_TYPE_CLI_KEY: failed --- mbedtls_pk_parse_key returned -0x%x", -ret);
                return -1;
            }
        
            TLSSOCKET_TRACE("MbedtlsSock: Loading the certificate ...\r\n");
#endif

    /* Enable ARC4 */
    // mbedtls_ssl_conf_arc4_support(&(tlssock_ptr->conf), MBEDTLS_SSL_ARC4_ENABLED);
    client_context->ssid = ssid;
    TLSSOCKET_TRACE("mbedtls_init: end!!!");

    return 0;
}

void _client_free(uint32_t client_id)
{
    CLIENT_CONTEXT *client_context = &g_client_context[client_id];
    TLSSOCKET_TRACE("_client_free");
    mbedtls_ssl_close_notify(&(client_context->ssl));

    mbedtls_x509_crt_free(&(client_context->cacert));
    mbedtls_x509_crt_free(&(client_context->clicert));
    mbedtls_pk_free(&(client_context->pkey));
    if (client_context->ssl.p_timer)
    {
        free(client_context->ssl.p_timer);
        client_context->ssl.p_timer = NULL;
    }
    mbedtls_ssl_free(&(client_context->ssl));
    mbedtls_ssl_config_free(&(client_context->conf));
    mbedtls_ctr_drbg_free(&(client_context->ctr_drbg));
    mbedtls_entropy_free(&(client_context->entropy));
    client_context->ssid = 0;
    if (client_context->timer != NULL)
    {
        free(client_context->timer);
        client_context->timer = NULL;
    }
    //mbedtls_net_free(&(client_context->server_fd));
    client_context->server_fd.fd = -1;
    osiSemaphoreAcquire(client_context->tlsbuf_sem);
    memset(&client_context->databuf, 0, sizeof(PLAIN_RING_BUF));
    osiSemaphoreRelease(client_context->tlsbuf_sem);
    client_context->dataEventHandler = 0;
    mbedtls_ssl_session_free(&(client_context->savedSession));
    client_context->isUsed = 0;
}

static void tls_TaskEntry(void *pData)
{
    //uint32_t rx_buf_size;
    int32_t ret;

    osiEvent_t event = {0, 0, 0, 0};
    uint32_t client_id;
    uint32_t isExit = 0;

    osiThreadSleep(10);

    while (!isExit)
    {
        TLSSOCKET_TRACE("tls_TaskEntry enter ");
        if (osiEventWait(g_tls_task_id, &event) != true)
        {
            TLSSOCKET_TRACE("tls_TaskEntry osiEventWait is false ");
        }
        else
        {
            //AT_CosEvent2CfwEvent(&event, &CfwEv);
            TLSSOCKET_TRACE("tls_TaskEntry nEventId:%ld", event.id);

            switch (event.id)
            {
            case EV_CFW_TLS_CREATE_REQ:
                TLSSOCKET_TRACE("EV_CFW_TLS_CREATE_REQ");
                //notify_task = (osiThread_t*)event.param1;
                //ssid = event.param2;
                //client_id = event.param3;

                //rx_buf_size = event.nParam3;
                //ret = mbedtls_init(ssid,client_id);

                //osiSemaphoreRelease(g_client_context[client_id].tls_sem);
                break;

            case EV_CFW_TLS_CLOSE_REQ:
                TLSSOCKET_TRACE("EV_CFW_TLS_CLOSE_REQ");
                client_id = event.param2;
                if (g_client_context[client_id].access_mode != 0 && 1 == tls_dataleft(client_id))
                {
                    g_client_context[client_id].state = TLS_STATE_PEER_REFUSED;
                    break;
                }
                if (g_client_context[client_id].isUsed == 1)
                    _client_free(client_id);
                //osiSemaphoreDelete(g_read_sema);
                //g_read_sema = NULL;
                if (g_client_context[client_id].uSocket != INVALID_SOCKET)
                    CFW_TcpipSocketClose(g_client_context[client_id].uSocket);
                else
                    tls_CloseEvent(client_id);
                break;

            case EV_CFW_TLS_CONNECT_REQ:
                TLSSOCKET_TRACE("EV_CFW_TLSSOCK_CONNECT_REQ");
                client_id = event.param2;

                ret = tcpip_Connect(client_id);
                if (ret != 0)
                {
                    TLSSOCKET_TRACE("EV_CFW_TLSSOCK_CONNECT_REQ,tcpip_Connect fail");
                    tls_ConnectEvent(client_id, ret);

                    _client_free(client_id);
                    if (g_client_context[client_id].uSocket != INVALID_SOCKET)
                    {

                        CFW_TcpipSocketClose(g_client_context[client_id].uSocket);
                    }
                    else
                        tls_CloseEvent(client_id);
                }
                else
                {
                    /*if(g_client_context[client_id].isDtls == 1){

                            CFW_TcpipSocketSetParam(g_client_context[client_id].uSocket, NULL, client_id);

                            ret = mbedtls_connect(&g_client_context[client_id]);
                            CFW_TcpipSocketSetParam(g_client_context[client_id].uSocket, (osiCallback_t)tcpip_rsp, client_id);

                            tls_ConnectEvent(client_id, ret); //TLS connect ok
                            if (ret != 0)
                            {
                                _client_free(client_id);
                                if (g_client_context[client_id].uSocket != INVALID_SOCKET)
                                {

                                    CFW_TcpipSocketClose(g_client_context[client_id].uSocket);
                                }
                                else
                                    tls_CloseEvent(client_id);
                            }
                        }*/
                }

                break;
            case EV_CFW_TLS_FREE_REQ:
                isExit = 1;
                g_tls_task_id = NULL;
                break;

            default:
                TLSSOCKET_TRACE("tls_TaskEntry sig=%ld isn't handled!!!", event.id);
                break;
            }
        }
    }
    osiThreadExit();
}

void tls_Init(void)
{
    if (g_tls_task_id != NULL)
        return;
    //s_read_sema = COS_CreateSemaphore(1);
    if (g_read_sema != NULL)
    {
        osiSemaphoreDelete(g_read_sema);
        g_read_sema = NULL;
    }
    g_read_sema = osiSemaphoreCreate(1, 1);

    g_tls_task_id = osiThreadCreate("TLS Task", tls_TaskEntry, NULL,
                                    TLS_TASK_PRIORITY, TLS_TASK_STACK_SIZE * 10,
                                    32);
}

int tls_Setup(uint32_t ssid, uint32_t client_id, osiCallback_t func, void *funcParam, uint8_t isDtls, char *psk_id, char *psk)
{
    CLIENT_CONTEXT *client_context = &g_client_context[client_id];

    //uint32_t i;
    //osiEvent_t ev = {0, 0, 0, 0};
    uint8_t result = false;

    TLSSOCKET_TRACE("tls_Setup");
    client_context->funcParam = funcParam;
    client_context->notify_callback = func;
    result = mbedtls_init(ssid, client_id, isDtls, psk_id, psk);
    if (result == 0)
        client_context->ssid = ssid;

    //ev.id = EV_CFW_TLS_CREATE_REQ;
    //ev.param1 = (uint32_t)notify_task;
    //ev.param2 = (uint32_t)ssid;
    //ev.param3 = (uint32_t)client_id;
    //ev.nParam3 = rx_buf_size;

    if (client_context->tlsbuf_sem == NULL)
    {
        client_context->tlsbuf_sem = osiSemaphoreCreate(1, 1);
        TLSSOCKET_TRACE("tls_Start:tls_Start[%ld]=%p", client_id, client_context->tlsbuf_sem);
    }
    /*osiEventSend(g_tls_task_id, &ev);
    osiSemaphoreAcquire(client_context->tls_sem);
    osiSemaphoreDelete(client_context->tls_sem]);*/
    //TLSSOCKET_TRACE("TlsSock_Createa: delete g_tls_sema[%d]",client_id);
    //client_context->tls_sem = NULL;
    //g_tlsConnectStopFlag[ssid] = 0;
    //g_sockFreeIsUsedFlag[ssid] = 0;

    return result;
}

int32_t tls_Cfg(uint32_t ssid, TLS_CFG_TYPE_E type, uint32_t param)
{
    TLS_CONTEXT *tls_context = (TLS_CONTEXT *)(&g_tls_context[ssid]);
    int32_t ret = 0;
    //TLS_CRT_T *pCrt;

    switch (type)
    {
    case TLS_CFG_TYPE_VERSION:
        if (param > 4)
        {
            TLSSOCKET_TRACE("Tls Cfg: failed --- TLS_CFG_TYPE_VERSION param error:%ld", param);
            return -1;
        }

        tls_context->ver = param;
        if ((param <= MBEDTLS_SSL_MINOR_VERSION_3) && (param >= MBEDTLS_SSL_MINOR_VERSION_0))
        {
            tls_context->min_ver = param;
            tls_context->max_ver = param;
        }
        else
        {
            tls_context->min_ver = MBEDTLS_SSL_MINOR_VERSION_0;
            tls_context->max_ver = MBEDTLS_SSL_MINOR_VERSION_3;
        }
        break;
    case TLS_CFG_TYPE_AUTHMODE:
        if (param > 2)
        {
            TLSSOCKET_TRACE("Tls Cfg: failed --- TLS_CFG_TYPE_AUTHMODE param error:%ld", param);
            return -1;
        }
        tls_context->authmode = param;

        break;

    case TLS_CFG_TYPE_CIPHER_SUITE:
    {
        TLSSOCKET_TRACE("Tls Cfg: failed --- TLS_CFG_TYPE_CIPHER_SUITE param:%lx", param);
        const mbedtls_ssl_ciphersuite_t *ciphersuite_info = mbedtls_ssl_ciphersuite_from_id(param); //= mbedtls_ssl_get_ciphersuite_name((const int)param);//
        if (NULL == ciphersuite_info && param != 0xffff)
        {
            ret = -1;
            break;
        }

        tls_context->ciphersuit = (uint32_t)param;
    }
    break;
    case TLS_CFG_TYPE_NEGO_TIMEOUT:
        tls_context->timeout = param;

        break;
    case TLS_CFG_TYPE_IGNORE_TIME:
        tls_context->ignore_time = param;
        break;

    default:
        ret = -1;
        break;
    }

    return ret;
}

int32_t tls_Connect(uint32_t client_id)
{
    int32_t ret = 0;
    //CLIENT_CONTEXT *client_context = &g_client_context[client_id];
    osiEvent_t ev = {0, 0, 0, 0};

    ev.id = EV_CFW_TLS_CONNECT_REQ;
    ev.param2 = client_id;

    osiEventSend(g_tls_task_id, &ev);
    return ret;
}
int32_t tls_Send(uint32_t client_id, uint8_t *buf, uint32_t len)
{
    CLIENT_CONTEXT *client_context = &g_client_context[client_id];
    int32_t ret = -1;

    TLSSOCKET_TRACE("tls_Send,socket=%ld", client_context->uSocket);
    CFW_TcpipSocketSetParam(client_context->uSocket, (osiCallback_t)tcpip_rsp, client_id);
    while ((ret = mbedtls_ssl_write(&(client_context->ssl), buf, len)) <= 0)
    {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
        {
            TLSSOCKET_TRACE("tls_Send: failed\n  ! mbedtls_ssl_write returned %ld\n\n", ret);
            return ret;
        }
    }
    TLSSOCKET_TRACE("tls_Send: Send ret =%ld", ret);
    return ret;
}
#if 0
int32_t tls_Recv(uint32_t client_id, uint8_t *buf, uint32_t len)
{
    CLIENT_CONTEXT *client_context = &g_client_context[client_id];
    int32_t ret = -1;
    TLSSOCKET_TRACE("tls_Recv,len=%ld", len);
    mbedtls_net_set_nonblock(&client_context->server_fd);
    mbedtls_ssl_set_bio(&client_context->ssl, &(client_context->server_fd), mbedtls_net_send, mbedtls_net_recv, NULL);
    ret = mbedtls_ssl_read(&(client_context->ssl), buf, len);
    if (ret < 0)
    {
        TLSSOCKET_TRACE("error tls_Recv %ld\n", ret);
        return ret;
    }

    TLSSOCKET_TRACE("tls_Recv: Recv ret =%ld", ret);

    return ret;
}
#endif
int32_t tls_Recv(uint32_t client_id, uint8_t *buf, uint32_t len)
{
    CLIENT_CONTEXT *client_context = &g_client_context[client_id];
    TLSSOCKET_TRACE("tls_Recv,len=%ld", len);
    osiSemaphoreAcquire(client_context->tlsbuf_sem);
    uint32_t data_left = client_context->databuf.put - client_context->databuf.get;
    TLSSOCKET_TRACE("tls_Recv,data_left=%ld", data_left);
    uint32_t read_len = data_left > len ? len : data_left;
    if (read_len > 0)
    {
        memcpy(buf, client_context->databuf.buf + client_context->databuf.get, read_len);
        client_context->databuf.get += read_len;
    }
    osiSemaphoreRelease(client_context->tlsbuf_sem);
    TLSSOCKET_TRACE("tls_Recv: Recv ret =%ld", read_len);
    int32_t avail_size = mbedtls_ssl_get_bytes_avail(&(client_context->ssl));
    TLSSOCKET_TRACE("tls_Recv: %ld bytes left in tls", avail_size);
    int32_t lwip_avail_size = CFW_TcpipGetRecvAvailable(client_context->uSocket);
    TLSSOCKET_TRACE("tls_Recv: %ld bytes left in lwip", lwip_avail_size);
    if (((0 < avail_size) || (0 < lwip_avail_size)) && (client_context->dataEventHandler == 0))
    {
        osiEvent_t *ev = malloc(sizeof(osiEvent_t));
        client_context->dataEventHandler = 1;
        ev->id = EV_CFW_TCPIP_REV_DATA_IND;
        ev->param1 = client_context->uSocket;
        ev->param3 = client_id;
        ///osiEventSend(g_tls_task_id, &ev);
        bool ret = osiThreadCallback(g_tls_task_id, (osiCallback_t)tcpip_rsp, (void *)ev);
        if (false == ret)
        {
            OSI_LOGI(0x10009514, "tls_Recv:osiThreadCallback failed");
            free(ev);
        }
    }
    else if ((0 != client_context->access_mode) && (0 == avail_size) && (0 == lwip_avail_size) && (client_context->state == TLS_STATE_PEER_REFUSED))
    {
        tls_Close(client_id);
    }

    return read_len;
}

int32_t tls_dataleft(uint32_t client_id)
{
    CLIENT_CONTEXT *client_context = &g_client_context[client_id];
    osiSemaphoreAcquire(client_context->tlsbuf_sem);
    uint32_t data_left = client_context->databuf.put - client_context->databuf.get;
    TLSSOCKET_TRACE("tls_dataleft,data_left=%ld", data_left);
    osiSemaphoreRelease(client_context->tlsbuf_sem);
    int32_t avail_size = mbedtls_ssl_get_bytes_avail(&(client_context->ssl));
    TLSSOCKET_TRACE(" %ld bytes left in tls", avail_size);
    int32_t lwip_avail_size = lwip_getRecvAvailSize(client_context->uSocket);
    TLSSOCKET_TRACE(" %ld bytes left in lwip", lwip_avail_size);
    if ((0 >= avail_size) && (0 >= data_left) && (0 >= lwip_avail_size))
    {
        return 0;
    }

    return 1;
}

void tls_Close(uint32_t client_id)
{
    osiEvent_t ev = {0, 0, 0, 0};

    TLSSOCKET_TRACE("tls_Close");

    ev.id = EV_CFW_TLS_CLOSE_REQ;
    ev.param2 = client_id;

    TLSSOCKET_TRACE("tls_Close 1");
    osiEventSend(g_tls_task_id, &ev);
    TLSSOCKET_TRACE("tls_Close 1");
}

int32_t tls_SetSimCid(uint32_t client_id, uint8_t nSimid, uint8_t nCid)
{
    int32_t ret = 0;
    CLIENT_CONTEXT *client_context = &g_client_context[client_id];

    client_context->nCid = nCid;
    client_context->nSimid = nSimid;
    return ret;
}
#endif
#endif
