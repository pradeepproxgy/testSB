#ifndef __CBB_CMD_LBS_H__
#define __CBB_CMD_LBS_H__
#include "ffw_lbs.h"

// function(result,info,arg)
typedef void(* lbs_get_location_cb_t)(int, ffw_lbs_info_t*, void*);
typedef void(* lbs_get_location_raw_cb_t)(int, char*, void*);

int cbb_cmd_lbs_set_key(char* key);
char* cbb_cmd_lbs_get_key();
int cbb_cmd_lbs_get_key_length();
int cbb_cmd_lbs_get_location(int mapType, lbs_get_location_cb_t cb, void* arg);
int cbb_cmd_lbs_get_location_raw(int mapType, lbs_get_location_raw_cb_t cb, void* arg);

#endif
