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

#ifndef _FTP_CONNECTION_H_
#define _FTP_CONNECTION_H_

#include "sockets.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _ftp_recv_ret_e
{
    FTP_RECV_SUCC = 0,
    FTP_RECV_FAIL,
    FTP_RECV_TIMEOUT
} ftp_recv_ret_e;

typedef void (*ftp_net_callback_dns_t)(void *context, bool ret);
typedef void (*ftp_net_callback_connect_t)(void *context, bool ret);
typedef void (*ftp_net_callback_disconnect_t)(void *context);
typedef void (*ftp_net_callback_recv_t)(void *context, ftp_recv_ret_e ret, uint8_t *buf, uint32_t buflen);
typedef void (*ftp_net_callback_exception_t)(void *context);

typedef struct _ftp_net_callback_t
{
    ftp_net_callback_dns_t onDns;
    ftp_net_callback_connect_t onConnect;
    ftp_net_callback_disconnect_t onDisconnect;
    ftp_net_callback_recv_t onRead;
    ftp_net_callback_exception_t onException;
} ftp_net_callback_t;

typedef enum _ftp_sock_e
{
    FTP_CTRL_SOCK = 0,
    FTP_DATA_SOCK,
    FTP_IPC_SOCK,
    FTP_MAX_SOCK
} ftp_sock_e;

bool ftp_conn_create();
void ftp_conn_destroy();
bool ftp_conn_connect(char *host, char *port, ftp_net_callback_t *cb, void *context, ftp_sock_e flag);
bool ftp_conn_disconnect(ftp_sock_e flag);
bool ftp_conn_write(uint8_t *buf, uint32_t buflen, ftp_sock_e flag);
int32_t ftp_conn_read(uint8_t *buf, uint32_t buflen, ftp_sock_e flag);
void ftp_conn_SetTimeout(uint32_t tm, ftp_sock_e flag);
void ftp_conn_KillTimeout(ftp_sock_e flag);
#ifdef CONFIG_FTP_OVER_SSL_SUPPORT
typedef void (*ftp_net_callback_handshake_t)(void *context, int32_t ret);
bool ftp_conn_SSLconnect(ftp_sock_e flag, ftp_net_callback_handshake_t cb);
#endif

#ifdef __cplusplus
}
#endif

#endif
