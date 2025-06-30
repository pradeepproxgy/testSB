#ifndef COMPONENTS_FIBOCOM_INCLUDE_FIBOCOM_WS_H
#define COMPONENTS_FIBOCOM_INCLUDE_FIBOCOM_WS_H

/**
* @file fibo_ws.h
* @author shiyongkun (charles.shi@fibocom.com)
* @brief websocket AT
* @version 1.0
* @date 2022-10-26
*
* Copyright (c) 2022 Fibocom. All rights reserved
*
 */

#include <stdbool.h>
#include <stdint.h>

#include "fibo_at_params.h"

#define WS_LOG(format, ...)                                                               \
    do                                                                                      \
    {                                                                                       \
        ffw_log("[wscmd][%s:%d]-" format, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    } while (0)


#define CHK_PARAM(params, code, cond)       \
    do                                   \
    {                                    \
        if (!(cond))                     \
        {                                \
            WS_LOG("check failed");    \
            fibo_atcmd_return_err(code); \
            return;                      \
        }                                \
    } while (0)

void fibocom_websocket_WSOPEN(fibo_at_cmd_t *pParam);
void fibocom_websocket_WSSEND(fibo_at_cmd_t *pParam);
void fibocom_websocket_WSCLOSE(fibo_at_cmd_t *pParam);
void fibocom_websocket_WSRSET(fibo_at_cmd_t *pParam);
void fibocom_websocket_WSREAD(fibo_at_cmd_t *pParam);
void fibocom_websocket_WSHEAD(fibo_at_cmd_t *pParam);

#endif
