typedef int INT32;
typedef unsigned int UINT32;
typedef char INT8;
typedef unsigned char UINT8;
typedef unsigned short UINT16;
typedef short INT16;
typedef bool BOOL;
typedef unsigned long long  UINT64;



INT32 fibo_firmware_handle_no_reboot(INT8 *data, UINT32 len);
INT32 fibo_ota_handle_no_reboot(INT8 *data, UINT32 len);
INT32 fibo_appfota_handle_no_reboot(INT8 *data, UINT32 len);
INT32 fibo_appfw_handle_no_reboot(INT8 *data, UINT32 len);
INT32 fibo_ble_handle(INT8 *data, UINT32 len);
INT32 fibo_ble_ota(UINT8 type, UINT8 *url,UINT8 *filename,UINT8 *username,UINT8 *password);
INT32 fibo_appfw_handle_by_path_ex(INT8 *path);
