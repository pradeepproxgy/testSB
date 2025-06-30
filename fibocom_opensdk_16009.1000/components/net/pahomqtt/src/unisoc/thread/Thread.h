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
 *    Ian Craggs - initial implementation
 *    Ian Craggs, Allan Stockdill-Mander - async client updates
 *    Ian Craggs - fix for bug #420851
 *    Ian Craggs - change MacOS semaphore implementation
 *******************************************************************************/
#include "UniTime.h"
#include "MQTTClient.h"

#if !defined(__THREAD_H__)
#define __THREAD_H__

#define thread_id_type void *

osiThread_t *Thread_start(osiCallback_t fn, void *parameter);
int Thread_stop(void);
osiThread_t *Thread_getid(void);

osiMutex_t *Thread_create_mutex(void);
void Thread_lock_mutex(osiMutex_t *_pstMutex);
void Thread_unlock_mutex(osiMutex_t *_pstMutex);
void Thread_destroy_mutex(osiMutex_t *_pstMutex);

osiSemaphore_t *Thread_create_cond(void);
void Thread_signal_cond(osiSemaphore_t *_pstCond);
int Thread_wait_cond(osiSemaphore_t *_pstCond, int timeout);
void Thread_destroy_cond(osiSemaphore_t *_pstCond);

#endif
