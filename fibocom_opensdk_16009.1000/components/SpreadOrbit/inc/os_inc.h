#ifndef _OS_INC_H
#define _OS_INC_H
#include "osi_api.h"
#include "gnss_config.h"
#include "osi_log.h"
//#include "os_api.h"
//#include "cutils/log.h"
// Type define.
//---------------------------------------------
//typedef osiMutex_t      *SCI_MUTEX_PTR;

#ifndef SCI_TRACE_LOW
#define SCI_TRACE_LOW(FMT, ...)          \
    do                                   \
    {                                    \
        OSI_PRINTFI(FMT, ##__VA_ARGS__); \
    } while (0);
#endif

#define _ALOGE(FMT, args...) SCI_TRACE_LOW(FMT, ##args)
#define _ALOGD(FMT, args...) SCI_TRACE_LOW(FMT, ##args)
#define E(FMT, args...)  _ALOGE(FMT, ##args) //SCI_TRACE_LOW(fmt, ...)//
#define D(FMT, args...)  _ALOGD(FMT, ##args) //SCI_TRACE_LOW(fmt, ...)// need lc

#endif  // End of  _OS_INC_H
