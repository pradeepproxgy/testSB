#ifndef OC_SSL_H
#define OC_SSL_H

/*****************************************************************
*   Copyright (C), 2022, Xian Fibocom Wireless Inc
*                  All rights reserved.
*   FileName    :oc_ssl.h
*   Author      :haopanpan
*   Created     :2022-01-05
*   Description :ssl opencpu api declaration.
*****************************************************************/

#include <stdbool.h>
#include <stdint.h>
#include "fibo_opencpu_comm.h"

INT32 fibo_ssl_set_ver(int ver);
INT32 fibo_ssl_write_file(INT8 *type_str, UINT8 *buff, UINT16 len);
INT32 fibo_ssl_chkmode_set(UINT8 mode);
INT32 fibo_ssl_errcode_get(void);
INT32 fibo_ssl_sock_create(void);
INT32 fibo_ssl_sock_close(INT32 sock);
INT32 fibo_ssl_sock_connect(INT32 sock, const char *addr, UINT16 port);
INT32 fibo_ssl_sock_send(INT32 sock, UINT8 *buff, UINT16 len);
INT32 fibo_ssl_sock_recv(INT32 sock, UINT8 *buff, UINT16 len);
UINT32 fibo_ssl_sock_fd_get(INT32 sock);

void fibo_ssl_default(void);
INT32 fibo_write_ssl_file(INT8 *type_str,UINT8 *buff,UINT16 len);
void fibo_set_ssl_chkmode(UINT8 mode);
INT32 fibo_get_ssl_errcode(void);

#endif

