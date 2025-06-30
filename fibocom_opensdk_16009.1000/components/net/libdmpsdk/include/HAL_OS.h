#ifndef _HAL_OS_H_
#define _HAL_OS_H_
#include <stdbool.h>
#include "util_types.h"
#include "HAL_OS_defs.h"
#include <stdarg.h>
#include "osi_log.h"

#define LOG_TAG OSI_MAKE_LOG_TAG('A', 'B', 'C', 'D')

//#define HAL_Printf(...) MG_osiTracePrintf(LOG_TAG,__VA_ARGS__)
#define HAL_Printf(...) osiTracePrintf(LOG_TAG,__VA_ARGS__)


//void HAL_Printf(const char *format, ...);

bool HAL_GetFirmwareVersion(char *version,int *len);
bool HAL_GetHardwareVersion(char *version,int *len);
bool HAL_GetManufactureInfo(char *buf,int *len);
bool HAL_GetModelInfo(char *buf,int *len);
int HAL_GetCsqInfo(uint8_t index,uint8_t *Signalevel,uint8_t *Error);
bool HAL_GetSimImei(uint8_t index,uint8_t *buf,int *len);
bool HAL_GetSimImsi(uint8_t index,uint8_t *buf,int *len);
bool HAL_GetSimIccid(uint8_t index,uint8_t *buf,int *len);

int HAL_GetSimStatus(uint8_t index);
int HAL_GetGprsStatus(uint8_t index);
char *HAL_GetTimeFormat24(void);

bool HAL_GetApn(char *name);
uint8_t HAL_GetNetsiglevel(const char *pApn, uint8_t *siglevel);
void *HAL_Malloc(uint32_t size);
void HAL_Free(void *ptr);

void *HAL_MutexCreate(void);
void HAL_MutexDestroy(void *mutex);
void HAL_MutexLock(void *mutex);
void HAL_MutexUnlock(void *mutex);
uint32_t HAL_Random();
void HAL_SleepMs(uint32_t ms);
int HAL_Snprintf(char *str, const int len, const char *fmt, ...);
void HAL_Srandom(uint32_t seed);
int HAL_TCP_Destroy(int32_t fd);
uintptr_t HAL_TCP_Establish(const char *host, uint16_t port);
uint32_t HAL_GetTimeMs(void);
int32_t HAL_TCP_Read(uintptr_t fd, char *buf, uint32_t len, uint32_t timeout_ms);
int32_t HAL_TCP_Write(uintptr_t fd, const char *buf, uint32_t len, uint32_t timeout_ms);
uint64_t HAL_UptimeMs(void);
uint64_t HAL_EpochtimeMs(void);


int HAL_Vsnprintf(char *str, const int len, const char *format, va_list ap);

int HAL_SetAutoRegPeriod(char *autoregperiod, int32_t size);/*ws add*/
int HAL_GetAutoRegPeriod(char * autoregperiod, int32_t size);/*ws add*/
int HAL_SetProductKey(char *productkey,int32_t size);/*ws add*/
int HAL_GetProductKey(char *productkey,int32_t size);
int HAL_SetProductSecret(char *productsecret,int32_t size);/*ws add*/
int HAL_GetProductSecret(char *productsecret,int32_t size);
int HAL_GetDeviceKey(char *devicetkey,int32_t size);
int HAL_GetDeviceSecret(char *devicesecret,int32_t size);
int HAL_GetDeviceToken(char *devicetoken,int32_t size);
int HAL_SetDeviceKey(const char *devicekey,int32_t size);
int HAL_SetDeviceSecret(const char *devicesecret,int32_t size);
int HAL_SetDeviceToken(const char *devicetoken,int32_t size);
int HAL_GetModuleProductKey(char *productkey,int32_t size);
int HAL_GetModuleProductSecret(char *productsecret,int32_t size);
int HAL_GetModuleDeviceKey(char *devicetkey,int32_t size);
int HAL_GetModuleDeviceSecret(char *devicesecret,int32_t size);
int HAL_GetModuleDeviceToken(char *devicetoken,int32_t size);
int HAL_SetModuleProductKey(const char *devicesecret,int32_t size);
int HAL_SetModuleDeviceKey(const char *devicesecret,int32_t size);
int HAL_SetModuleProductSecret(const char *productsecret,int32_t size);
int HAL_SetModuleDeviceSecret(const char *devicesecret,int32_t size);
int HAL_SetModuleDeviceToken(const char *devicetoken,int32_t size);

int HAL_DelProductKey(void);
int HAL_DelProductSecret(void);
int HAL_DelDeviceKey(void);
int HAL_DelDeviceSecret(void);
int HAL_DelDeviceToken(void);

int HAL_DelModuleProductKey(void);
int HAL_DelModuleProductSecret(void);
int HAL_DelModuleDeviceKey(void);
int HAL_DelModuleDeviceSecret(void);
int HAL_DelModuleDeviceToken(void);


int HAL_SNTP_Get_Time(const char *ntp_addr,void(*Callback)(const char *));
void HAL_GetRsrpRsrq(int *rsrp,float *rsrq,char *cellid);
int UNI_PSM_Operation(int mod);

#if 1
uintptr_t HAL_SSL_Establish(const char *host,
								uint16_t port,
								const char *ca_crt,
								uint32_t ca_crt_len);
int HAL_SSL_Read(uintptr_t handle, char *buf, int len, int timeout_ms);
int HAL_SSL_Write(uintptr_t handle, const char *buf, int len, int timeout_ms);
int32_t HAL_SSL_Destroy(uintptr_t handle);
#endif

#endif
