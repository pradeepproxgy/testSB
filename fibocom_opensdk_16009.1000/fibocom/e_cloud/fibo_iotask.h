/*****************************************************************
*   Copyright (C), 2018, Xian Fibocom Wireless Inc
*   		       All rights reserved.                  
*   FileName    :fibo_iotask.h
*   Author      :taric.wang
*   Created     :2019-12-23
*   Description :fibo_iotask.h
*****************************************************************/





#pragma once
#ifndef _FIBO_IOTASK_H_
#define _FIBO_IOTASK_H_

#include "fibocom_com_api.h"


#ifdef __cplusplus
extern "C"
{
#endif

bool  fibip_add_sock(fib_module_type module, int sock, xQueueHandle handle);
bool  fibip_del_sock(int sock);



#ifdef __cplusplus
}
#endif
#endif //_FIBO_IOTASK_H_


