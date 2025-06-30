
#ifndef _FIBO_LOG_H
#define _FIBO_LOG_H

#include "osi_log.h"

#define FIBO_LOGE(fmt, ...) __OSI_LOGB(OSI_LOG_LEVEL_ERROR, 0, fmt, ##__VA_ARGS__)
#define FIBO_LOGW(fmt, ...) __OSI_LOGB(OSI_LOG_LEVEL_WARN, 0, fmt, ##__VA_ARGS__)
#define FIBO_LOGI(fmt, ...) __OSI_LOGB(OSI_LOG_LEVEL_INFO, 0, fmt, ##__VA_ARGS__)
#define FIBO_LOGD(fmt, ...) __OSI_LOGB(OSI_LOG_LEVEL_DEBUG, 0, fmt, ##__VA_ARGS__)
#define FIBO_LOGV(fmt, ...) __OSI_LOGB(OSI_LOG_LEVEL_VERBOSE, 0, fmt, ##__VA_ARGS__)

#define FIBO_LOGXE(fmt, ...) __OSI_LOGX(OSI_LOG_LEVEL_ERROR, OSI_LOGPAR_S, 0, fmt, ##__VA_ARGS__)
#define FIBO_LOGXW(fmt, ...) __OSI_LOGX(OSI_LOG_LEVEL_WARN, OSI_LOGPAR_S, 0, fmt, ##__VA_ARGS__)
#define FIBO_LOGXI(fmt, ...) __OSI_LOGX(OSI_LOG_LEVEL_INFO, OSI_LOGPAR_S, 0, fmt, ##__VA_ARGS__)
#define FIBO_LOGXD(fmt, ...) __OSI_LOGX(OSI_LOG_LEVEL_DEBUG, OSI_LOGPAR_S, 0, fmt, ##__VA_ARGS__)
#define FIBO_LOGXV(fmt, ...) __OSI_LOGX(OSI_LOG_LEVEL_VERBOSE, OSI_LOGPAR_S, 0, fmt, ##__VA_ARGS__)

#define dp FIBO_LOGI
#define ds FIBO_LOGXI
#define df(fmt, ...) __OSI_LOGX(OSI_LOG_LEVEL_INFO, OSI_LOGPAR_F, 0, fmt, ##__VA_ARGS__)

/* lobin add, for log to uart */
extern void du(char *fmt, ...);
#endif
