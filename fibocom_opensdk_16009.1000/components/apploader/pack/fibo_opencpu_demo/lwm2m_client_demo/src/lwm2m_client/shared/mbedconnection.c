/*******************************************************************************
 *
 * Copyright (c) 2015 Intel Corporation and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * The Eclipse Distribution License is available at
 *    http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    David Navarro, Intel Corporation - initial API and implementation
 *    Christian Renz - Please refer to git log
 *
 *******************************************************************************/
#include "fibo_opencpu.h"
#include <stdlib.h>
#include <string.h>
#include "mbedconnection.h"
#include "internals.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/timing.h"
#include "mbedtls/debug.h"
#include "app_lwm2m.h"
#include "app_pdp.h"

#define COAP_PORT "5683"
#define COAPS_PORT "5684"
#define URI_LENGTH 256

#define MAX_PACKET_SIZE 1440
#define APP_LOG_INFO(format, ...) fibo_textTrace("[lwm2m_app][info][%s:%d]-" format " \n", __FUNCTION__, __LINE__, ##__VA_ARGS__)

static int get_port(struct sockaddr *x)
{
   if (x->sa_family == AF_INET)
   {
       return ((struct sockaddr_in *)x)->sin_port;
   }
   else if (x->sa_family == AF_INET6) {
       return ((struct sockaddr_in6 *)x)->sin6_port;
   }
   else {
       APP_LOG_INFO("non IPV4 or IPV6 address\n");
       return  -1;
   }
}

static int sockaddr_cmp(struct sockaddr *x, struct sockaddr *y)
{
    int portX = get_port(x);
    int portY = get_port(y);

    // if the port is invalid of different
    if (portX == -1 || portX != portY)
    {
        return 0;
    }

    // IPV4?
    if (x->sa_family == AF_INET)
    {
        // is V4?
        if (y->sa_family == AF_INET)
        {
            // compare V4 with V4
            return ((struct sockaddr_in *)x)->sin_addr.s_addr == ((struct sockaddr_in *)y)->sin_addr.s_addr;
        } else {
            return 0;
        }
    }
    else if (x->sa_family == AF_INET6 && y->sa_family == AF_INET6) {
        // IPV6 with IPV6 compare
        return memcmp(((struct sockaddr_in6 *)x)->sin6_addr.s6_addr, ((struct sockaddr_in6 *)y)->sin6_addr.s6_addr, 16) == 0;
    }
    else {
        // unknown address type
        APP_LOG_INFO("non IPV4 or IPV6 address\n");
        return 0;
    }
}

/********************* Security Obj Helpers **********************/
char *security_get_uri(lwm2m_context_t *lwm2mH, lwm2m_object_t *obj, int instanceId, char *uriBuffer, int bufferSize)
{
    int size = 1;
    lwm2m_data_t *dataP = lwm2m_data_new(size);
    dataP->id = 0; // security server uri

    obj->readFunc(lwm2mH, instanceId, &size, &dataP, obj);
    if (dataP != NULL &&
        dataP->type == LWM2M_TYPE_STRING &&
        dataP->value.asBuffer.length > 0)
    {
        if (bufferSize > dataP->value.asBuffer.length)
        {
            memset(uriBuffer, 0, dataP->value.asBuffer.length + 1);
            strncpy(uriBuffer, (char *)(dataP->value.asBuffer.buffer), dataP->value.asBuffer.length);
            lwm2m_data_free(size, dataP);
            return uriBuffer;
        }
    }
    lwm2m_data_free(size, dataP);
    return NULL;
}

int64_t security_get_mode(lwm2m_context_t * lwm2mH, lwm2m_object_t *obj, int instanceId)
{
    int64_t mode = 0;
    int size = 1;
    lwm2m_data_t *dataP = lwm2m_data_new(size);
    dataP->id = 2; // security mode

    obj->readFunc(lwm2mH, instanceId, &size, &dataP, obj);
    if (0 != lwm2m_data_decode_int(dataP, &mode))
    {
        lwm2m_data_free(size, dataP);
        return mode;
    }

    lwm2m_data_free(size, dataP);
    APP_LOG_INFO("Unable to get security mode : use not secure mode");
    return LWM2M_SECURITY_MODE_NONE;
}

char *security_get_public_id(lwm2m_context_t *lwm2mH, lwm2m_object_t *obj, int instanceId, int *length)
{
    int size = 1;
    lwm2m_data_t *dataP = lwm2m_data_new(size);
    if (dataP == NULL)
        return NULL;
    dataP->id = 3; // public key or id

    obj->readFunc(lwm2mH, instanceId, &size, &dataP, obj);
    if (dataP != NULL &&
        dataP->type == LWM2M_TYPE_OPAQUE)
    {
        char *buff;

        buff = (char *)lwm2m_malloc(dataP->value.asBuffer.length);
        if (buff != NULL)
        {
            memcpy(buff, dataP->value.asBuffer.buffer, dataP->value.asBuffer.length);
            *length = dataP->value.asBuffer.length;
        }
        lwm2m_data_free(size, dataP);

        return buff;
    }
    else
    {
        lwm2m_data_free(size, dataP);
        return NULL;
    }
}

char *security_get_secret_key(lwm2m_context_t *lwm2mH, lwm2m_object_t *obj, int instanceId, int *length)
{
    int size = 1;
    lwm2m_data_t *dataP = lwm2m_data_new(size);
    if (dataP == NULL)
        return NULL;
    dataP->id = 5; // secret key
    APP_LOG_INFO("security_get_secret_key");
    obj->readFunc(lwm2mH, instanceId, &size, &dataP, obj);
    if (dataP != NULL &&
        dataP->type == LWM2M_TYPE_OPAQUE)
    {
        char *buff;
        buff = (char *)lwm2m_malloc(dataP->value.asBuffer.length);
        if (buff != 0)
        {
            memcpy(buff, dataP->value.asBuffer.buffer, dataP->value.asBuffer.length);
            *length = dataP->value.asBuffer.length;
        }
        lwm2m_data_free(size, dataP);
        return buff;
    }
    else
    {
        lwm2m_data_free(size, dataP);
        return NULL;
    }
}
/********************* Security Obj Helpers Ends **********************/

int send_data(dtls_connection_t *connP,
              uint8_t *buffer,
              size_t length)
{
    int nbSent;
    size_t offset;

#ifdef WITH_LOGS
    char s[INET6_ADDRSTRLEN];
    in_port_t port;

    s[0] = 0;

    if (AF_INET == connP->addr.sin6_family)
    {
        struct sockaddr_in *saddr = (struct sockaddr_in *)&connP->addr;
        inet_ntop(saddr->sin_family, &saddr->sin_addr, s, INET6_ADDRSTRLEN);
        port = saddr->sin_port;
    }
    else if (AF_INET6 == connP->addr.sin6_family)
    {
        struct sockaddr_in6 *saddr = (struct sockaddr_in6 *)&connP->addr;
        inet_ntop(saddr->sin6_family, &saddr->sin6_addr, s, INET6_ADDRSTRLEN);
        port = saddr->sin6_port;
    }

    APP_LOG_INFO("Sending %d bytes to [%s]:%hu\r\n", length, s, ntohs(port));

#endif

    offset = 0;

    while (offset != length)
    {

        nbSent = fibo_sock_sendto(connP->sock, buffer + offset, length - offset, 0, 
             (struct sockaddr *)&(connP->addr), connP->addrLen);
        if (nbSent == -1)
            return -1;
        offset += nbSent;
    }
    connP->lastSend = lwm2m_gettime();
    return 0;
}

static int net_would_block(const mbedtls_net_context *ctx)
{
    /*
     * Never return 'WOULD BLOCK' on a non-blocking socket
     */
    if ((fcntl(ctx->fd, F_GETFL, 0) & O_NONBLOCK) != O_NONBLOCK)
        return (0);

    switch (errno)
    {
#if defined EAGAIN
    case EAGAIN:
#endif
#if defined EWOULDBLOCK && EWOULDBLOCK != EAGAIN
    case EWOULDBLOCK:
#endif
        return (1);
    }
    return (0);
}

/*
 * Write at most 'len' characters
 */
int mbeddtls_lwm2m_send(void *connP, const unsigned char *buf, size_t len)
{
    APP_LOG_INFO("mbeddtls_lwm2m_send enter");
    dtls_connection_t *ctx = (dtls_connection_t *)connP;
    int fd = ctx->sock;
    APP_LOG_INFO("mbeddtls_lwm2m_send enter,fd=%d", fd);
    mbedtls_net_context *mbed_ctx = ctx->server_fd;

    if (fd < 0)
        return (MBEDTLS_ERR_NET_INVALID_CONTEXT);

    int nbSent = -1;
    size_t offset;

    char s[INET6_ADDRSTRLEN];
    in_port_t port = 0;

    s[0] = 0;
    if (AF_INET == ctx->addr.sin_family)
    {
        struct sockaddr_in *saddr = (struct sockaddr_in *)&ctx->addr;
        inet_ntop(saddr->sin_family, &saddr->sin_addr, s, INET_ADDRSTRLEN);
        port = saddr->sin_port;
    }
    if (AF_INET6 == ctx->addr6.sin6_family)
    {
        struct sockaddr_in6 *saddr = (struct sockaddr_in6 *)&ctx->addr;
        inet_ntop(saddr->sin6_family, &saddr->sin6_addr, s, INET6_ADDRSTRLEN);
        port = saddr->sin6_port;
    }
    APP_LOG_INFO("Sending %d bytes,\r\n", len);
    APP_LOG_INFO("sending port=%u\r\n", port);

    offset = 0;

    while (offset != len)
    {
        nbSent = fibo_sock_sendto(fd, buf + offset, len - offset, 0, (struct sockaddr *)&(ctx->addr), ctx->addrLen);
        if (nbSent < 0)
        {
            if (net_would_block(mbed_ctx) != 0)
                return (MBEDTLS_ERR_SSL_WANT_WRITE);

#if (defined(_WIN32) || defined(_WIN32_WCE)) && !defined(EFIX64) && \
    !defined(EFI32)
            if (WSAGetLastError() == WSAECONNRESET)
                return (MBEDTLS_ERR_NET_CONN_RESET);
#else
            if (errno == EPIPE || errno == ECONNRESET)
                return (MBEDTLS_ERR_NET_CONN_RESET);

            if (errno == EINTR)
                return (MBEDTLS_ERR_SSL_WANT_WRITE);
#endif
            APP_LOG_INFO("++++Send error errno:%d\r\n", errno);
            return (MBEDTLS_ERR_NET_SEND_FAILED);
        }
        offset += nbSent;
    }
    APP_LOG_INFO("return len = %d", offset);
    return offset;
}

/*
 * Read at most 'len' characters
 */
int mbeddtls_lwm2m_recv(void *ctx, unsigned char *buffer, size_t len)
{

    int fd = ((dtls_connection_t *)ctx)->sock;
    APP_LOG_INFO("mbeddtls_lwm2m_recv enter,fd=%d", fd);
    mbedtls_net_context *mbed_ctx = ((dtls_connection_t *)ctx)->server_fd;
    if (fd < 0)
        return (MBEDTLS_ERR_NET_INVALID_CONTEXT);

    /*
     * This part will set up an interruption until an event happen on SDTIN or the socket until "tv" timed out (set
     * with the precedent function)
     */
    int numBytes;

    struct sockaddr_storage addr;
    socklen_t addrLen;
    addrLen = sizeof(addr);

    /*
    * We retrieve the data received
    */
    numBytes = fibo_sock_recvfrom_v2(fd, buffer, MAX_PACKET_SIZE, 0, (struct sockaddr *)&addr, (uint32_t *)&addrLen);

    if (0 > numBytes)
    {
        APP_LOG_INFO("recv numbytes < 0");

        if (net_would_block(mbed_ctx) != 0)
        {
            APP_LOG_INFO("mbedtls recv would block");
            return (MBEDTLS_ERR_SSL_WANT_READ);
        }

#if (defined(_WIN32) || defined(_WIN32_WCE)) && !defined(EFIX64) && \
    !defined(EFI32)
        if (WSAGetLastError() == WSAECONNRESET)
            return (MBEDTLS_ERR_NET_CONN_RESET);
#else
        if (errno == EPIPE || errno == ECONNRESET)
        {
            APP_LOG_INFO("mbedtls net connect reset");
            return (MBEDTLS_ERR_NET_CONN_RESET);
        }

        if (errno == EINTR)
        {
            APP_LOG_INFO("mbedtls ssl wait read");
            return (MBEDTLS_ERR_SSL_WANT_READ);
        }
#endif

        APP_LOG_INFO("mbedtls net recv failed");

        return (MBEDTLS_ERR_NET_RECV_FAILED);
    }
    else if (0 < numBytes)
    {
        APP_LOG_INFO("mbeddtls_lwm2m_recv ,numBytes=%d", numBytes);
        char s[INET6_ADDRSTRLEN];
        in_port_t port = 0;
        if (AF_INET == addr.ss_family)
        {
            struct sockaddr_in *saddr = (struct sockaddr_in *)&addr;
            inet_ntop(saddr->sin_family, &saddr->sin_addr, s, INET_ADDRSTRLEN);
            port = saddr->sin_port;
        }
        if (AF_INET6 == addr.ss_family)
        {
            struct sockaddr_in6 *saddr = (struct sockaddr_in6 *)&addr;
            inet_ntop(saddr->sin6_family, &saddr->sin6_addr, s, INET6_ADDRSTRLEN);
            port = saddr->sin6_port;
        }
        APP_LOG_INFO("mbeddtls_lwm2m_recv:%d bytes received from [%s]:%hu\r\n", numBytes, s, ntohs(port));
    }
    return numBytes;
}

int mbeddtls_lwm2m_recv_timeout(void *ctx, unsigned char *buf, size_t len,
                                uint32_t timeout)
{
    int ret;
    struct timeval tv;
    fd_set read_fds;
    uint32_t selectms = 500;
    //int fd = ((mbedtls_net_context *) ctx)->fd;
    int fd = ((dtls_connection_t *)ctx)->sock;

    APP_LOG_INFO("mbedtls_lwm2m_recv_timeout entering");
    if (fd < 0)
        return (MBEDTLS_ERR_NET_INVALID_CONTEXT);

    //tv.tv_sec = timeout / 1000;
    //tv.tv_usec = (timeout % 1000) * 1000;

    //ret = fibo_sock_select(fd + 1, &read_fds, NULL, NULL, timeout == 0 ? NULL : &tv);
    uint32_t trytimes = timeout/selectms;
    uint32_t leftms = timeout%selectms;
    tv.tv_sec = selectms / 1000;
    tv.tv_usec = (tv.tv_sec == 0)? (selectms*1000):(selectms%1000 * 1000);
    for(int i = 0; i< trytimes ; i++){
        if(((dtls_connection_t *)ctx)->ssl->isquit == 1)
            return MBEDTLS_ERR_SSL_QUIT_FORCED;
        FD_ZERO(&read_fds);
        FD_SET(fd, &read_fds);
        ret = fibo_sock_select(fd + 1, &read_fds, NULL, NULL, timeout == 0 ? NULL : &tv);
        if(ret != 0)
            break;
    }
    if(ret == 0 && leftms > 0){
        if(((dtls_connection_t *)ctx)->ssl->isquit == 1)
            return MBEDTLS_ERR_SSL_QUIT_FORCED;
        FD_ZERO(&read_fds);
        FD_SET(fd, &read_fds);
        tv.tv_sec = leftms/1000;
        tv.tv_usec = (tv.tv_sec == 0)?(leftms*1000):(leftms%1000 * 1000);
        ret = fibo_sock_select(fd + 1, &read_fds, NULL, NULL, timeout == 0 ? NULL : &tv);
    }
    /* Zero fds ready means we timed out */
    if (ret == 0)
        return (MBEDTLS_ERR_SSL_TIMEOUT);

    if (ret < 0)
    {
#if (defined(_WIN32) || defined(_WIN32_WCE)) && !defined(EFIX64) && \
    !defined(EFI32)
        if (WSAGetLastError() == WSAEINTR)
            return (MBEDTLS_ERR_SSL_WANT_READ);
#else
        if (errno == EINTR)
            return (MBEDTLS_ERR_SSL_WANT_READ);
#endif

        return (MBEDTLS_ERR_NET_RECV_FAILED);
    }

    /* This call will not block */
    return (mbeddtls_lwm2m_recv(ctx, buf, len));
}

static void lwm2m_debug(void *ctx, int level,
                        const char *file, int line,
                        const char *param)
{
    ((void)level);
    APP_LOG_INFO("%s", param);
}
mbedtls_ssl_context * g_sslContext = NULL;
static const int lwm2m_ciphersuite_preference[] =
{
    MBEDTLS_TLS_PSK_WITH_AES_128_CCM_8,
    MBEDTLS_TLS_PSK_WITH_AES_128_CBC_SHA256,
    MBEDTLS_TLS_PSK_WITH_AES_128_CBC_SHA,
    MBEDTLS_TLS_PSK_WITH_AES_256_CBC_SHA384,
    MBEDTLS_TLS_PSK_WITH_AES_256_CBC_SHA,
    MBEDTLS_TLS_PSK_WITH_3DES_EDE_CBC_SHA,
    MBEDTLS_TLS_RSA_WITH_AES_256_CBC_SHA256,
    MBEDTLS_TLS_RSA_WITH_AES_256_CBC_SHA,
    MBEDTLS_TLS_RSA_WITH_AES_128_CBC_SHA256,
    MBEDTLS_TLS_RSA_WITH_AES_128_CBC_SHA,
    MBEDTLS_TLS_RSA_WITH_3DES_EDE_CBC_SHA,
    
};

int get_dtls_context(lwm2m_context_t *lwm2mH, dtls_connection_t *connList)
{
    int ret;
    const char *pers = "lwm2mclient";

    connList->server_fd = lwm2m_malloc(sizeof(mbedtls_net_context));
    if (connList->server_fd == NULL)
    {
        APP_LOG_INFO("connList->server_fd MALLOC FAILED:%d!\n", sizeof(mbedtls_net_context));
        return -1;
    }
    mbedtls_net_init(connList->server_fd);

    connList->ssl = lwm2m_malloc(sizeof(mbedtls_ssl_context));
    if (connList->ssl == NULL)
    {
        APP_LOG_INFO("connList->ssl MALLOC FAILED:%d!\n", sizeof(mbedtls_ssl_context));
        return -1;
    }

    APP_LOG_INFO("create ssl transport %p", connList->ssl);

    mbedtls_ssl_init(connList->ssl);
    g_sslContext = connList->ssl;
    connList->conf = lwm2m_malloc(sizeof(mbedtls_ssl_config));
    if (connList->conf == NULL)
    {
        APP_LOG_INFO("connList->conf MALLOC FAILED:%d!\n", sizeof(mbedtls_ssl_config));
        return -1;
    }
    mbedtls_ssl_config_init(connList->conf);

    connList->cacert = lwm2m_malloc(sizeof(mbedtls_x509_crt));
    if (connList->cacert == NULL)
    {
        APP_LOG_INFO("connList->cacert MALLOC FAILED:%d!\n", sizeof(mbedtls_x509_crt));
        return -1;
    }
    mbedtls_x509_crt_init(connList->cacert);

    connList->ctr_drbg = lwm2m_malloc(sizeof(mbedtls_ctr_drbg_context));
    if (connList->ctr_drbg == NULL)
    {
        APP_LOG_INFO("connList->ctr_drbg MALLOC FAILED:%d!\n", sizeof(mbedtls_ctr_drbg_context));
        return -1;
    }
    mbedtls_ctr_drbg_init(connList->ctr_drbg);

    connList->entropy = lwm2m_malloc(sizeof(mbedtls_entropy_context));
    if (connList->entropy == NULL)
    {
        APP_LOG_INFO("connList->entropy MALLOC FAILED:%d!\n", sizeof(mbedtls_entropy_context));
        return -1;
    }
    mbedtls_entropy_init(connList->entropy);

    if ((ret = mbedtls_ctr_drbg_seed(connList->ctr_drbg, mbedtls_entropy_func, connList->entropy,
                                     (const unsigned char *)pers, strlen(pers))) != 0)
    {
        APP_LOG_INFO("mbedtls_ctr_drbg_seed failed...,ret=%d\n", ret);
        return ret;
    }

    connList->server_fd->fd = connList->sock;
    APP_LOG_INFO("mbedtls_use the sock=%d\n", connList->sock);
    if ((ret = mbedtls_ssl_config_defaults(connList->conf,
                                           MBEDTLS_SSL_IS_CLIENT,
                                           MBEDTLS_SSL_TRANSPORT_DATAGRAM,
                                           MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
    {
        APP_LOG_INFO("mbedtls_ssl_config_defaults failed ret = %d\n", ret);
        return ret;
    }
    ret = mbedtls_net_set_block(connList->server_fd);
    mbedtls_timing_delay_context *timer = lwm2m_malloc(sizeof(mbedtls_timing_delay_context));
    if (timer == NULL)
        APP_LOG_INFO("get_dtls_context timer malloc failed:%d\n", sizeof(mbedtls_timing_delay_context));
    mbedtls_ssl_set_timer_cb(connList->ssl, timer, mbedtls_timing_set_delay,
                             mbedtls_timing_get_delay);

    mbedtls_ssl_conf_ciphersuites(connList->conf, lwm2m_ciphersuite_preference);

    int length = 0;
    int id_len = 0;
    char *psk = security_get_secret_key(lwm2mH, connList->securityObj, connList->securityInstId, &length);
    if(psk == NULL)
    {
        APP_LOG_INFO("security_get_secret_key get psk is null\n");
    }
    
    char *psk_id = security_get_public_id(lwm2mH, connList->securityObj, connList->securityInstId, &id_len);
    if(psk_id == NULL)
    {
        APP_LOG_INFO("security_get_public_id get psk_id is null\n");
    }

    if ((ret = mbedtls_ssl_conf_psk(connList->conf, (const unsigned char *)psk, length,
                                    (const unsigned char *)psk_id,
                                    id_len)) != 0)
    {
        lwm2m_free(psk);
        lwm2m_free(psk_id);
        APP_LOG_INFO(" failed! mbedtls_ssl_conf_psk returned %d\n\n", ret);
        return ret;
    }
    APP_LOG_INFO(" mbedtls_ssl_conf_psk returned %d,psk=%s,psk_id=%s\n\n", ret, psk, psk_id);
    lwm2m_free(psk);
    lwm2m_free(psk_id);
    /* OPTIONAL is not optimal for security,
    * but makes interop easier in this simplified example */
    mbedtls_ssl_conf_legacy_renegotiation(connList->conf, MBEDTLS_SSL_LEGACY_ALLOW_RENEGOTIATION);
    mbedtls_ssl_conf_renegotiation(connList->conf, MBEDTLS_SSL_RENEGOTIATION_ENABLED);
    mbedtls_ssl_conf_authmode(connList->conf, MBEDTLS_SSL_VERIFY_NONE);
    mbedtls_ssl_conf_ca_chain(connList->conf, connList->cacert, NULL);
    mbedtls_ssl_conf_rng(connList->conf, mbedtls_ctr_drbg_random, connList->ctr_drbg);
    mbedtls_ssl_conf_dbg(connList->conf, lwm2m_debug, NULL);
    mbedtls_debug_set_threshold(3);

    if ((ret = mbedtls_ssl_setup(connList->ssl, connList->conf)) != 0)
    {
        APP_LOG_INFO("mbedtls_ssl_setup failed ret = %d\n", ret);
        return ret;
    }
    mbedtls_ssl_conf_read_timeout(connList->conf, 60000);
    mbedtls_ssl_set_bio(connList->ssl, connList, mbeddtls_lwm2m_send, mbeddtls_lwm2m_recv, mbeddtls_lwm2m_recv_timeout);
    /*
    * 4. Handshake
    */
    while ((ret = mbedtls_ssl_handshake(connList->ssl)) != 0)
    {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
        {
            APP_LOG_INFO(" failed ! mbedtls_ssl_handshake returned %x\n\n", -ret);
            return ret;
        }
    }
    APP_LOG_INFO(" success ! mbedtls_ssl_handshake returned %x\n\n", -ret);
    if ((ret = mbedtls_ssl_get_session(connList->ssl, &connList->saved_session)) != 0)
    {
        APP_LOG_INFO(" failed\n  ! mbedtls_ssl_get_session returned -0x%x\n\n", -ret);
    }
    connList->send_firstData = 1;
    APP_LOG_INFO(" ok\n");
    return 0;
}

int create_socket(const char *portStr, int ai_family)
{
    APP_LOG_INFO("Enterring create_socket");

    int ret = 0;
    char *localipv4 = NULL;
    char *localipv6 = NULL;
    fibo_addrinfo_t *local_addr = NULL;
    int s = -1;
    int port_nr = atoi(portStr);
    if ((port_nr <= 0) || (port_nr > 0xffff)) {
      return -1;
    }

    // Retrieve the IPv4 address for the default PDP context
    ret = app_pdp_get_ipaddress(CID_DEFAULT, &localipv4, &localipv6);
    if (ret != 0)
    {
        APP_LOG_INFO("get cid %d local address fail", CID_DEFAULT);
        return -1;
    }

    if(ai_family == AF_INET)
    {
        fibo_addrinfo_para localparas = {
            .host_name = localipv4,
            .host_port = 0,
            .ip_type = ai_family,
            .dnsServerIp1 = NULL,
            .dnsServerIp2= NULL,
            .peer_addr = &local_addr,
            .timeout = 0
        };
        ret = fibo_get_addrinfo(&localparas);
        if (ret != 0){
            APP_LOG_INFO("get local addr(%s) fail %d", localipv4, ret);
            return -1;
        }
    }
    else if(ai_family == AF_INET6)
    {
        fibo_addrinfo_para localparas = {
            .host_name = localipv6,
            .host_port = 0,
            .ip_type = ai_family,
            .dnsServerIp1 = NULL,
            .dnsServerIp2= NULL,
            .peer_addr = &local_addr,
            .timeout = 0
        };
        ret = fibo_get_addrinfo(&localparas);
        if (ret != 0){
            APP_LOG_INFO("get local addr(%s) fail %d", localipv6, ret);
            return -1;
        }
    }
    else
    {
        return -1;
    }

    /* Do name resolution with both IPv6 and IPv4 */
    s = (int) fibo_sock_socket(ai_family, SOCK_DGRAM, IPPROTO_UDP);
    if( s < 0 )
    {
        if(local_addr != NULL)
        {
            fibo_sock_freeaddrinfo(local_addr);
        }
        ret = MBEDTLS_ERR_NET_SOCKET_FAILED;
        return ret;
    }
    else
    {
        if(-1 == fibo_sock_bind_v2(s, (struct sockaddr *)local_addr->ai_addr, local_addr->ai_addrlen))
        {
            fibo_sock_close_v2(s);
            s = -1;
        }
    }

    if(local_addr != NULL)
    {
        fibo_sock_freeaddrinfo(local_addr);
    }
    return s;
}

dtls_connection_t *connection_new_incoming(dtls_connection_t *connList,
                                           int sock,
                                           const struct sockaddr *addr,
                                           size_t addrLen)
{
    dtls_connection_t *connP = NULL;
    connP = (dtls_connection_t *)lwm2m_malloc(sizeof(dtls_connection_t));
    if (connP != NULL)
    {
        memset(connP, 0, sizeof(dtls_connection_t));
        connP->sock = sock;
        memcpy(&(connP->addr), addr, addrLen);
        connP->addrLen = addrLen;
        connP->next = connList;
        APP_LOG_INFO("new_incoming");
    }
    else
    {
        APP_LOG_INFO("new_incoming,malloc failed!");
    }

    return connP;
}

// dtls_connection_t *connection_find(dtls_connection_t *connList, int sock)
dtls_connection_t *connection_find(dtls_connection_t *connList,
                                   const struct sockaddr_storage *addr,
                                   size_t addrLen)
{
    dtls_connection_t *connP;

    connP = connList;
    while (connP != NULL)
    {
        if (sockaddr_cmp((struct sockaddr *)(&connP->addr), (struct sockaddr *)addr))
        {
            return connP;
        }

        connP = connP->next;
    }

    return connP;
}

dtls_connection_t *connection_create(dtls_connection_t *connList,
                                     int sock,
                                     lwm2m_object_t *securityObj,
                                     int instanceId,
                                     lwm2m_context_t *lwm2mH,
                                     int addressFamily)
{
    int ret = 0;
    fibo_addrinfo_t *servinfo = NULL;
    dtls_connection_t *connP = NULL;
    char uriBuf[URI_LENGTH];
    char *uri;
    char *host;
    char *port;

    uri = security_get_uri(lwm2mH, securityObj, instanceId, uriBuf, URI_LENGTH);
    if (uri == NULL)
        return NULL;

    // parse uri in the form "coaps://[host]:[port]"
    char *defaultport;
    if (0 == strncmp(uri, "coaps://", strlen("coaps://")))
    {
        host = uri + strlen("coaps://");
        defaultport = COAPS_PORT;
    }
    else if (0 == strncmp(uri, "coap://", strlen("coap://")))
    {
        host = uri + strlen("coap://");
        defaultport = COAP_PORT;
    }
    else
    {
        return NULL;
    }
    port = strrchr(host, ':');
    if (port == NULL)
    {
        port = defaultport;
    }
    else
    {
        // remove brackets
        if (host[0] == '[')
        {
            host++;
            if (*(port - 1) == ']')
            {
                *(port - 1) = 0;
            }
            else
            {
                return NULL;
            }
        }
        // split strings
        *port = 0;
        port++;
    }
    uint16_t port_s = atoi(port);

    fibo_addrinfo_para paras = {host, port_s, addressFamily, NULL, NULL, &servinfo, 0};
    ret = fibo_get_addrinfo(&paras);
    if (ret != 0)
    {
        APP_LOG_INFO("get dns addr(%s) fail %d", host, ret);
        return NULL;
    }
    if(servinfo == NULL)
    {
        APP_LOG_INFO("servinfo is NULL!");
        return NULL;
    }

    if (sock >= 0)
    {
        connP = connection_new_incoming(connList, sock, servinfo->ai_addr, servinfo->ai_addrlen);
        if (connP != NULL)
        {
            APP_LOG_INFO("connP is %p lwm2m_context %p", connP, lwm2mH);
            connP->securityObj = securityObj;
            connP->securityInstId = instanceId;
            connP->lwm2mH = lwm2mH;

            if (security_get_mode(lwm2mH, connP->securityObj, connP->securityInstId) != LWM2M_SECURITY_MODE_NONE)
            {
                connP->issecure = 1;
                int ret = get_dtls_context(lwm2mH, connP);
                if (ret != 0)
                {
                    APP_LOG_INFO(" lwm2m get_dtls_context ret=%d", ret);
                    if (NULL != servinfo)
                        fibo_sock_freeaddrinfo(servinfo);
                    dtls_connection_t *targetP = connP;
                    connP = targetP->next;
                    targetP->next = NULL;
                    connection_free(targetP);
                    if(MBEDTLS_ERR_SSL_TIMEOUT != ret){
                        // lwm2mH->tlspeer_refused = 1;
                    }      
                    return NULL;
                }
            }
            else
            {
                // no dtls session
                connP->issecure = 0;
            }
        }
    }

    if (NULL != servinfo)
        fibo_sock_freeaddrinfo(servinfo);
    return connP;
}

void connection_free(dtls_connection_t *connList)
{
    APP_LOG_INFO("Enter connection_free");
    //dtls_connection_t *nextQ;

    //nextQ = connList->next;
    while (connList)
    {
        APP_LOG_INFO("connList != NULL");

        dtls_connection_t *nextP;

        nextP = connList->next;

        if (connList->server_fd)
        {

            mbedtls_net_free(connList->server_fd);
            lwm2m_free(connList->server_fd);
            connList->server_fd = NULL;
        }

        if (connList->entropy)
        {
            mbedtls_entropy_free(connList->entropy);
            lwm2m_free(connList->entropy);
            connList->entropy = NULL;
        }
        if (connList->conf)
        {
            mbedtls_ssl_config_free(connList->conf);
            lwm2m_free(connList->conf);
            connList->conf = NULL;
        }
        if (connList->cacert)
        {
            mbedtls_x509_crt_free(connList->cacert);
            lwm2m_free(connList->cacert);
            connList->cacert = NULL;
        }
#if 0
        if (connList->clicert)
        {
            mbedtls_x509_crt_free(connList->clicert);
            lwm2m_free(connList->clicert);
            connList->clicert = NULL;

        }
        if (connList->clikey)
        {
            mbedtls_pk_free(connList->clikey);
            lwm2m_free(connList->clikey);
            connList->clikey = NULL;

        }
#endif
        if (connList->ssl)
        {
            if (connList->ssl->p_timer)
                lwm2m_free(connList->ssl->p_timer);
            mbedtls_ssl_free(connList->ssl);
            lwm2m_free(connList->ssl);
            connList->ssl = NULL;
            g_sslContext = NULL;
        }
        if (connList->ctr_drbg)
        {
            mbedtls_ctr_drbg_free(connList->ctr_drbg);
            lwm2m_free(connList->ctr_drbg);
            connList->ctr_drbg = NULL;
        }
        mbedtls_ssl_session_free( &connList->saved_session);

        APP_LOG_INFO("M2M# connection_free connList = %p", connList);

        lwm2m_free(connList);
        connList = nextP;
    }
}

int connection_send(dtls_connection_t *connP, uint8_t *buffer, size_t length)
{
    int ret = -1;
    if (connP->issecure == 0)
    {
        if (0 != send_data(connP, buffer, length))
        {
            return -1;
        }
    }
    else
    {
        time_t currentTime = lwm2m_gettime();
        if (!connP->send_firstData && (((30 < currentTime - connP->lastSend))))
        { //mbedtls_ssl_renegotiate( connP->ssl);
            if ((ret = mbedtls_ssl_session_reset(connP->ssl)) != 0)
            {
                APP_LOG_INFO(" failed\n  ! mbedtls_ssl_session_reset returned -0x%x\n\n", -ret);
                return ret;
            }
            if ((ret = mbedtls_ssl_set_session(connP->ssl, &connP->saved_session)) != 0)
            {
                APP_LOG_INFO(" failed\n  ! mbedtls_ssl_conf_session returned -0x%x\n\n", -ret);
                return ret;
            }
            ret = mbedtls_net_set_block(connP->server_fd);
            mbedtls_ssl_set_bio(connP->ssl, connP, mbeddtls_lwm2m_send, mbeddtls_lwm2m_recv, mbeddtls_lwm2m_recv_timeout);
            while ((ret = mbedtls_ssl_handshake(connP->ssl)) != 0)
            {
                if (ret != MBEDTLS_ERR_SSL_WANT_READ &&
                    ret != MBEDTLS_ERR_SSL_WANT_WRITE)
                {
                    APP_LOG_INFO(" failed\n  ! mbedtls_ssl_handshake returned -0x%x\n\n", -ret);
                    return ret;
                }
            }   
            if ((ret = mbedtls_ssl_get_session(connP->ssl, &connP->saved_session)) != 0)
            {
                APP_LOG_INFO(" failed\n  ! after rehandshake mbedtls_ssl_get_session returned -0x%x\n\n", -ret);
            }
            APP_LOG_INFO(" rehandshake,ok\n");
        }

        connP->send_firstData = 0;
        if (0 > (ret = mbedtls_ssl_write(connP->ssl, buffer, length)))
        {
            return ret;
        }
        connP->lastSend = lwm2m_gettime();
    }
    APP_LOG_INFO("connection_send success");
    return 0;
}

int connection_handle_packet(dtls_connection_t *connP, uint8_t *buffer, size_t numBytes)
{

    if (connP->issecure != 0) //(connP->dtlsSession != NULL)
    {
        APP_LOG_INFO("security mode, ssl transport %p", connP->ssl);

        connP->lastSend = lwm2m_gettime();
        mbedtls_net_set_nonblock(connP->server_fd);
        //mbedtls_ssl_set_bio( connList->ssl, connList, mbeddtls_lwm2m_send, mbeddtls_lwm2m_recv, mbeddtls_lwm2m_recv_timeout);
        mbedtls_ssl_set_bio(connP->ssl, connP, mbeddtls_lwm2m_send, mbeddtls_lwm2m_recv, NULL);
        int result = mbedtls_ssl_read(connP->ssl, buffer, MAX_PACKET_SIZE);
        if (result <= 0)
        {
            APP_LOG_INFO("error dtls handling message %d\n", result);
            return result;
        }

        APP_LOG_INFO("connP is %p lwm2m_context %p", connP, connP->lwm2mH);

        // APP_LOG_INFO("after mbedtls_ssl_read,relsult=%d", result);
        lwm2m_handle_packet(connP->lwm2mH, buffer, result, (void *)connP);
        return 0;
    }
    else
    {
        // no security, just give the plaintext buffer to liblwm2m
        lwm2m_handle_packet(connP->lwm2mH, buffer, numBytes, (void *)connP);
        return 0;
    }
}

uint8_t lwm2m_buffer_send(void *sessionH,
                          uint8_t *buffer,
                          size_t length,
                          void *userdata)
{
    dtls_connection_t *connP = (dtls_connection_t *)sessionH;

    if (connP == NULL)
    {
        APP_LOG_INFO("#> failed sending %lu bytes, missing connection\r\n", length);
        return COAP_500_INTERNAL_SERVER_ERROR;
    }

    if (0 > connection_send(connP, buffer, length))
    {
        APP_LOG_INFO("#> failed sending %lu bytes\r\n", length);
        return COAP_500_INTERNAL_SERVER_ERROR;
    }
    APP_LOG_INFO("lwm2m_buffer_send success");
    return COAP_NO_ERROR;
}

bool lwm2m_session_is_equal(void *session1,
                            void *session2,
                            void *userData)
{
    return (session1 == session2);
}

