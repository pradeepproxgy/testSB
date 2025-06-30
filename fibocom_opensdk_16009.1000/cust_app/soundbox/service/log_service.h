
#ifndef __LOG_SERVICE_H__
#define __LOG_SERVICE_H__
#include "fb_config.h"
#include "fibo_opencpu.h"
#include "fb_uart.h"


#if USER_LOG_EN

#define  LOG                                        OSI_LOGI//
int fb_printf(const char *fmt, ...);

#else

#define     fb_printf(...)
#define     LOG(...)

#endif




#endif

