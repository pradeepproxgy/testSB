/*******************************************************************************
 * Copyright (c) 2009, 2018 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution. 
 *
 * The Eclipse Public License is available at 
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at 
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs, Allan Stockdill-Mander - initial implementation 
 *    Ian Craggs - SNI support
 *    Ian Craggs - post connect checks and CApath
 *******************************************************************************/
#include "paho_config.h"
#if defined(MBEDTLS) || defined(OPENSSL)

#if defined(WIN32) || defined(WIN64)
#define ssl_mutex_type HANDLE
#else
#include <pthread.h>
/*zyzyzy 20180929 #include <semaphore.h>*/
#define ssl_mutex_type pthread_mutex_t
#endif
#include "Clients.h"
#include "SocketBuffer.h"

/*
#include "mbedtls/ssl.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/timing.h"*/

#define URI_SSL "ssl://"

SSL_SESSION *SSL_get1_session(SSL *ssl);
int SSL_set_session(SSL *ssl, SSL_SESSION *session);

int SSLSocket_setSocketForSSL(networkHandles *net, MQTTClient_SSLOptions *opts, const char *hostname, size_t hostname_len);

int SSLSocket_getch(SSL *ssl, int socket, char *c);
//char *SSLSocket_getdata(SSL *ssl, int socket, size_t bytes, size_t *actual_len);
char *SSLSocket_getdata(SSL *ssl, int socket, size_t bytes, size_t *actual_len, int *rc);

int SSLSocket_close(networkHandles *net);
//int SSLSocket_putdatas(SSL *ssl, int socket, char *buf0, size_t buf0len, int count, char **buffers, size_t *buflens, int *frees);
int SSLSocket_putdatas(SSL *ssl, int socket, char *buf0, size_t buf0len, PacketBuffers bufs);
int SSLSocket_connect(SSL *ssl, int sock, const char *hostname, int verify, int (*cb)(const char *str, size_t len, void *u), void *u);

void SSLSocket_addPendingRead(int sock);
int SSLSocket_getPendingRead(void);
int SSLSocket_continueWrite(pending_writes *pw);
void SSLSocket_terminate(void);
void SSLSocket_handleOpensslInit(int bool_value);
int SSLSocket_initialize(void);

extern void mbedtls_debug_set_threshold(int threshold);

#endif
