/* Copyright (C) 2022 FIBOCOM Technologies Limited and/or its affiliates("FIBOCOM").
 * All rights reserved.
 *
 * This software is supplied "AS IS" without any warranties.
 * FIBOCOM assumes no responsibility or liability for the use of the software,
 * conveys no license or title under any patent, copyright, or mask work
 * right to the product. FIBOCOM reserves the right to make changes in the
 * software without notification.  FIBOCOM also make no representation or
 * warranty that such application will be suitable for the specified use
 * without further testing or modification.
 */


#define OSI_LOG_TAG OSI_MAKE_LOG_TAG('M', 'Y', 'A', 'P')
#include "fibo_opencpu.h"

#include "osi_log.h"
#include "osi_api.h"
#include "oc_gnss.h"


static void prvInvokeGlobalCtors(void)
{
    extern void (*__init_array_start[])();
    extern void (*__init_array_end[])();

    size_t count = __init_array_end - __init_array_start;
    for (size_t i = 0; i < count; ++i)
        __init_array_start[i]();
}

static void fibo_get_nmea_data_and_gnss_status(void)
{
    uint8_t nmea[1024] = {0};
    char *nmea_buff = NULL;
    char *str = NULL;
    char *str_tmp = NULL;
    int i = 0;
    hal_rtc_time_t time_value;
    hal_rtc_time_t time_value_1;
    bool location_ok = false;
    char nmea_rmc[20][20] = {0};

    fibo_getRTC(&time_value);
    fibo_textTrace("GNSS TEST: min:%d, sec:%d", time_value.min, time_value.sec);
    
    for (int n = 0; n < 10000; n++)
    {
        fibo_taskSleep(1000);
        fibo_gnss_nmea_get(nmea, 1024);
        fibo_textTrace("[%s-%d]  nmea data: %s",__FUNCTION__, __LINE__, nmea);

        /*Judge that the positioning is successful, change the status of RMC to A, and obtain longitude, latitude and other parameters*/

        nmea_buff = (char *)nmea;
        while ((str = strtok(nmea_buff, "$")) != NULL)
        {
            if (strstr((const char *)str, "RMC") != NULL)
            {
                i = 0;
                while ((str_tmp = strtok(str, ",")) != NULL)
                {
                    memcpy(nmea_rmc[i++], str_tmp, strlen(str_tmp));
                    if (i >12)
                    {
                        fibo_textTrace("GNSS TEST: nmea data parse error");
                        break;
                    }
                    str = NULL;
                }

                if (strstr(nmea_rmc[2], "A") != NULL)
                {
                    fibo_textTrace("GNSS TEST: location is ok, RMC status is %s, Latitude:%s:%s, longitude:%s:%s",
                    nmea_rmc[2], nmea_rmc[3], nmea_rmc[4], nmea_rmc[5], nmea_rmc[6]);

                    location_ok = true;
                    break;
                }
            }
            nmea_buff = NULL;
        }

        if (location_ok)
        {
            fibo_getRTC(&time_value_1);

            fibo_textTrace("GNSS TEST: location time: %d:%d:%d", 
                        time_value_1.hour-time_value.hour,
                        time_value_1.min-time_value.min, 
                        time_value_1.sec-time_value.sec);
            fibo_textTrace("GNSS TEST: location ok, stop GNSS, location time:%d", 
                (time_value_1.hour-time_value.hour)*3600 + (time_value_1.min-time_value.min)*60 + (time_value_1.sec-time_value.sec));
            break;
        }
    }
}
static void gnss_thread_entry(void *param)
{
    fibo_textTrace("application thread enter, param 0x%x", param);

    reg_info_t reginfo = {0};
    int32_t satellite = 0xFF;

    /*Judge whether the network injection is successful, and initiate positioning after the network injection is successful*/
    while(1)
    {
        fibo_reg_info_get(&reginfo, 0);

        if (reginfo.nStatus == 1)
        {
            fibo_textTrace("reg status is success");
            break;
        }
        fibo_taskSleep(1000);
    }

    /*GNSS cold start*/
    fibo_textTrace("GNSS TEST: STEP 1");
    fibo_gnss_enable(1);
    fibo_get_nmea_data_and_gnss_status(); /*Obtain GNSS data and judge the successful positioning duration*/
    fibo_gnss_enable(0);

    /*Wait 2S, set AGNSS and cold start AGNSS*/
    fibo_taskSleep(2000);
    fibo_textTrace("GNSS TEST: STEP 2");
    fibo_gnss_set_agnss_mode(1);
    fibo_gnss_enable(1);
    fibo_get_nmea_data_and_gnss_status(); /*Obtain GNSS data and judge the successful positioning duration*/
    fibo_gnss_enable(0);

    /*Wait for 2S, turn off AGNSS and hot start GNSS*/
    fibo_taskSleep(2000);
    fibo_textTrace("GNSS TEST: STEP 3");
    fibo_gnss_set_agnss_mode(0);
    fibo_gnss_enable(2);
    fibo_get_nmea_data_and_gnss_status(); /*Obtain GNSS data and judge the successful positioning duration*/
    fibo_gnss_enable(0);

    /*Wait for 2S and obtain the positioning satellite combination. The default satellite combination is GPS + Beidou + Galileo*/
    fibo_taskSleep(2000);
    fibo_textTrace("GNSS TEST: STEP 4");
    satellite = fibo_gnss_get_satellite();
    fibo_textTrace("GNSS TEST: satellite is %d",satellite);

    /*Set the type of NMEA reporting statement. Generally, there is no need to set it. All NMEA data types will be reported to APP*/
    fibo_textTrace("GNSS TEST: STEP 5");
    fibo_gnss_nmea_config(NULL, 0);
    fibo_taskSleep(5000);

    
    fibo_thread_delete();
}

int appimg_enter(void *param)
{
    fibo_textTrace("application image enter, param 0x%x", param);

    prvInvokeGlobalCtors();

    fibo_thread_create(gnss_thread_entry, "mythread", 409600, NULL, OSI_PRIORITY_NORMAL);
    return 0;
}

void appimg_exit(void)
{
    fibo_textTrace("application image exit");
}

