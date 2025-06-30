#ifndef C96BA979_F430_4A1C_B02E_236C843D03CB
#define C96BA979_F430_4A1C_B02E_236C843D03CB

#include "app.h"

typedef enum
{
    APP_LBS_TYPE_GPRS = 6,

    APP_LBS_TYPE_WIFI = 7

}app_lbs_type_t;

/**
* @brief 
* 
* @param lbstype 
* @param lbsinfo 
* @param timeout 
* @return int 
 */
int app_lbs_getgisinfo(app_lbs_type_t lbstype, fibo_lbs_res_t **lbsinfo, int timeout);

/**
* @brief 
* 
* @param lbsinfo 
* @return int 
 */
int app_lbs_freegisinfo(fibo_lbs_res_t *lbsinfo);

/**

* @brief 
* 
 */
void app_lbs_demo();

#endif /* C96BA979_F430_4A1C_B02E_236C843D03CB */
