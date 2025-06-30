#ifndef _OTA_SERVICE_H_
#define _OTA_SERVICE_H_
#include "fibo_opencpu.h"
extern int app_ota_flag;


#define APP_OTA_FILE_NAME        "ota_app.bin"
#define LOCAL_OTA_FILE_PATH      "/tmp/ota_app.bin"
#define OTA_HTTP_URL             "http://111.231.250.105/L610/HD_GPS_BDS.hdb"



int ota_service_enter(void);
#endif // !_OTA_SERVICE_H_