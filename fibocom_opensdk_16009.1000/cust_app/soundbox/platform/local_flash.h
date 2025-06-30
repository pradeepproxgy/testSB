#ifndef _LOCAL_FLASH_H_
#define _LOCAL_FLASH_H_

#include"fibo_opencpu.h"
#define MAX_SIZE_AUTH_KEY        100
#define MAX_SIZE_FILE_CONTENT      256



void create_json(void);
int fb_set_auth_key(char *register_status,char *auth_key);
int fb_set_dev_info_sta(int *bat_lock,int *motor_lock, int *tangze_lock, int *rotate_total);
int fb_get_devices_config( char *register_status,char *auth_key);
int fb_get_dev_params(char *product_id,char *product_secret, char *device_name, char *device_secret);
int fb_get_dev_info_sta(int *bat_lock,int *motor_lock, int *tangze_lock, int *rotate_total);

int fb_set_dev_config_info(int *heart_interval,int *loaction_update_time,int *event_interval);
int fb_get_dev_config_info(int *heart_interval,int *loaction_update_time,int *event_interval);
#endif
