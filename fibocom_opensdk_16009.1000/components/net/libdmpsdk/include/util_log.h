/*******************************************************************************
* Author		  :  author
* Version		  :  V1.0
* Date			  :  2021-07-27
* Description	  :  util_log.h
********************************************************************************/

#ifndef __UTIL_LOG_H__
#define __UTIL_LOG_H__

/********************************* Include File ********************************/
#include "HAL_OS.h"

#include <string.h>

//HAL_Printf("[%s][%d][%s]: " fmt, g_logLevelStr[level], line, function, ##__VA_ARGS__);
//HAL_Printf("[%s][%s][%s][%d][%s]: " fmt, HAL_GetTimeFormat24(), g_logLevelStr[level], filename, line, function, ##__VA_ARGS__);
//HAL_Printf("[%s][%s]%s(%03d):" fmt,g_logLevelStr[level],g_logModuleStr[module],function,line,##__VA_ARGS__);
//HAL_Printf("[%s][%s]%s(%03d):" fmt,HAL_GetTimeFormat24(),g_logLevelStr[level],function,line,##__VA_ARGS__);

/********************************* Macro Definition ****************************/
#if 1
#define log_print(module, level, filename, line, function, fmt, ...) \
do{\
	if(module < LOG_MOD_MAX && level < LOG_LEV_MAX) {\
		if(level <= g_logLevelSet[module]) {\
			HAL_Printf("[%s][%d][%s]: " fmt, g_logLevelStr[level], line, function, ##__VA_ARGS__);\
		    if(strstr((char *)fmt, "\n") == NULL) HAL_Printf("\n");\
		}\
	}\
}while (0)

#else 
#define log_print log_out
#endif
//__FILENAME__
#define LOGM_ERR(mod, fmt, ...) do{log_print(mod, LOG_LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__);}while (0)
#define LOGM_WAR(mod, fmt, ...) do{log_print(mod, LOG_LEVEL_WARN,  __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__);}while (0)
#define LOGM_INF(mod, fmt, ...) do{log_print(mod, LOG_LEVEL_INFO,  __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__);}while (0)
#define LOGM_DEB(mod, fmt, ...) do{log_print(mod, LOG_LEVEL_DEBUG, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__);}while (0)
#define LOG_ERR(fmt, ...)       LOGM_ERR(LOGDEFAULT, fmt, ##__VA_ARGS__)
#define LOG_WAR(fmt, ...)       LOGM_WAR(LOGDEFAULT, fmt, ##__VA_ARGS__)
#define LOG_INF(fmt, ...)       LOGM_INF(LOGDEFAULT, fmt, ##__VA_ARGS__)
#define LOG_DEB(fmt, ...)       LOGM_DEB(LOGDEFAULT, fmt, ##__VA_ARGS__)

#define LOG_LEV_MAX     5
#define LOG_MOD_MAX     8

																							 
/********************************* Type Definition *****************************/
typedef enum
{
	LOG_LEVEL_NONE  = 0,
	LOG_LEVEL_ERROR = 1,	
    LOG_LEVEL_WARN  = 2,
    LOG_LEVEL_INFO  = 3,
    LOG_LEVEL_DEBUG = 4,
    LOG_LEVEL_MAX   = LOG_LEV_MAX,  // must no use
}LogLevel;

typedef enum
{
	LOGDEFAULT  = 0,               // common
	MQTT        = 1,               // uni mqtt Module
	MQTTC		= 2,
	SERACK      = 3,               // uni server Module
	REGIST		= 4,			   //uni regist Module
	SHADOW      = 5,               // uni device shadow
	MEDTLS		= 6,
	LOGMODMAX   = LOG_MOD_MAX,     // must no use
}LogModList;


/********************************* Variables ***********************************/
extern char *g_logLevelStr[LOG_LEV_MAX];
extern uint8_t g_logLevelSet[LOG_MOD_MAX];


/********************************* Function Prototype Definition ***************/
void log_out(uint8_t module, uint8_t level, const char *filename, uint32_t line, const char *function, const char *fmt, ...);
void UNI_SetLogLevel(LogModList module, LogLevel level);
void UNI_SetAllLogLevel(LogLevel level);


#endif
