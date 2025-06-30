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
#include "fibo_opencpu.h"
#include "string.h"

/*
  * set the value to execute different process
  * 1 : test app update
  * 2 : test fimware update
  * 3 : test app download and update
  * 4 : test firmware download and update
  * 5 : test app fota update
  * 6 : app+fw fota update
  */
#define TEST_TYPE                6

#define def_max_ServerAdrLen    256
#define def_max_VersionLen      50

typedef struct
{
        bool download_flag;
        uint8_t ServerAdrLen;
        uint8_t ServerAdr[def_max_ServerAdrLen];
        uint8_t VersionLen;
        uint8_t Version[def_max_VersionLen];
        uint8_t tag[def_max_VersionLen];
        uint8_t md5[33];
}OtaParaTypeDef;

struct pdphandle{
    UINT32 sem;
    char success;
};

OtaParaTypeDef otaData;
struct pdphandle g_pdp_handle;


static void prvInvokeGlobalCtors(void)
{
    extern void (*__init_array_start[])();
    extern void (*__init_array_end[])();

    size_t count = __init_array_end - __init_array_start;
    for (size_t i = 0; i < count; ++i)
        __init_array_start[i]();
}


void app_download_test()
{
    int ret;
    char *url = "http://xatest.fibocom.com:8000/httpfota/test1/lobin/hello_flash.img";

    ret = fibo_app_dl(0, (UINT8*)url, NULL, NULL, NULL);
    if(ret < 0){
        fibo_textTrace("fibo_app_dl fail.");
        goto fexit;
    }
    fibo_textTrace("fibo_app_dl is OK.");

    for (int n = 0; n < 5; n++)
    {
        fibo_textTrace("hello world %d", n);
        fibo_taskSleep(1000);
    }

    ret = fibo_app_update();
    if(ret < 0){
        fibo_textTrace("fibo_app_update fail.");
        fibo_textTrace("error code is %d", fibo_ota_errcode());
        goto fexit;
    }
    fibo_textTrace("fibo_app_update is OK.");
fexit:
    return;
}

void firmware_http_download_test()
{
    int ret;
    char *url = "http://xatest.fibocom.com:8000/httpfota/test1/lobin/fota.bin";

    ret = fibo_firmware_dl(0, (UINT8*)url, NULL, NULL, NULL);
    if(ret < 0){
        fibo_textTrace("fibo_firmware_dl fail.");
        fibo_textTrace("error code is %d", fibo_ota_errcode());
        goto fexit;
    }
    fibo_textTrace("fibo_firmware_dl is OK.");

    for (int n = 0; n < 5; n++)
    {
        fibo_textTrace("hello world %d", n);
        fibo_taskSleep(1000);
    }

    ret = fibo_firmware_update();
    if(ret < 0){
        fibo_textTrace("fibo_firmware_update fail.");
        fibo_textTrace("error code is %d", fibo_ota_errcode());
        goto fexit;
    }
    fibo_textTrace("fibo_firmware_update is OK.");
fexit:
    return;
}

void firmware_ftp_download_test()
{
    int ret;
    char *url = "47.110.xxx.xx:21";
    char *filename = "test/test.bin";
    char *username = "test";
    char *passwd = "test";

    ret = fibo_firmware_dl(1, (UINT8*)url, (UINT8*)filename, (UINT8*)username, (UINT8*)passwd);
    if(ret < 0){
        fibo_textTrace("fibo_firmware_dl fail.");
        fibo_textTrace("error code is %d", fibo_ota_errcode());
        goto fexit;
    }
    fibo_textTrace("fibo_firmware_dl is OK.");

    for (int n = 0; n < 5; n++)
    {
        fibo_textTrace("hello world %d", n);
        fibo_taskSleep(1000);
    }

    ret = fibo_firmware_update();
    if(ret < 0){
        fibo_textTrace("fibo_firmware_update fail.");
        fibo_textTrace("error code is %d", fibo_ota_errcode());
        goto fexit;
    }
    fibo_textTrace("fibo_firmware_update is OK.");
fexit:
    return;
}

void app_update_test()
{
    int ret;
    char *url = "http://xatest.fibocom.com:8000/httpfota/test1/lobin/zr_flashV02_1596091410.img";
     ret = fibo_open_ota(0, (UINT8*)url, NULL, NULL, NULL);
    if(ret < 0){
        fibo_textTrace("fibo_open_ota fail.");
        goto fexit;
    }
    fibo_textTrace("fibo_open_ota is OK.");

    for (int n = 0; n < 10; n++)
    {
        fibo_textTrace("hello world %d", n);
        fibo_taskSleep(1000);
    }

fexit:
    return;
}

void appfota_update_test()
{
    int ret;
    char *url = "http://xatest.fibocom.com:8000/httpfota/test1/lobin/appfota.bin";

     ret = fibo_appfota_ota(0, (UINT8*)url, NULL, NULL, NULL);
    if(ret < 0){
        fibo_textTrace("fibo_appfota_ota fail.");
        fibo_textTrace("appfota error code = %d", fibo_ota_errcode());
        goto fexit;
    }
    fibo_textTrace("fibo_appfota_ota is OK.");

    for (int n = 0; n < 500; n++)
    {
        fibo_textTrace("new.bin print here %d", n);
        fibo_taskSleep(1000);
    }

fexit:
    return;
}


void firmware_update_test()
{
    int ret;
    char *url = "http://xatest.fibocom.com:8000/httpfota/test1/lobin/fota.bin";

    ret = fibo_firmware_ota(0, (UINT8*)url, (UINT8*)"/test1/lobin/hello_flash.img", (UINT8*)"httpfota", (UINT8*)"test");
    if(ret < 0){
        fibo_textTrace("fibo_firmware_ota fail.");
        goto fexit;
    }
    fibo_textTrace("fibo_firmware_ota is OK.");

    for (int n = 0; n < 10; n++)
    {
        fibo_textTrace("hello world %d", n);
        fibo_taskSleep(1000);
    }

fexit:
    return;
}

void appfw_update_test()
{
    int ret;
    char *url = "http://xatest.fibocom.com:8000/httpfota/test1/lobin/merge.bin";

    ret = fibo_appfw_ota(0, (UINT8*)url, NULL, NULL, NULL);
    if(ret < 0){
        fibo_textTrace("fibo_appfw_ota fail.");
        fibo_textTrace("appfota error code = %d", fibo_ota_errcode());
        goto fexit;
    }
    fibo_textTrace("fibo_appfw_ota is OK.");

    for (int n = 0; n < 500; n++)
    {
        fibo_textTrace("new.bin print here %d", n);
        fibo_taskSleep(1000);
    }

fexit:
    return;
}




void otaThreadEntry_firmware(void *param)
{
    if(TEST_TYPE == 1)
        app_update_test();
    else if(TEST_TYPE == 2)
        firmware_update_test();
    else if(TEST_TYPE == 3)
        app_download_test();
    else if(TEST_TYPE == 4)
        firmware_http_download_test();
    else if(TEST_TYPE == 5)
        firmware_ftp_download_test();
    else if(TEST_TYPE == 6)
        appfota_update_test();
    else if(TEST_TYPE == 7)
        appfw_update_test();
    else
        fibo_textTrace("TEST_TYPE value is setting error, please set again!");

    fibo_thread_delete();
}


static void prvThreadEntry(void *param)
{
    int             ret = 0;
    int             test = 1;
    reg_info_t         reg_info;
    UINT8             ip[50];
    UINT8             cid_status;

    INT8             cid = 1;
    CFW_SIM_ID         sim_id = CFW_SIM_0;

    memset(&ip, 0, sizeof(ip));
    memset(&g_pdp_handle, 0, sizeof(g_pdp_handle));
    memset(&otaData, 0, sizeof(otaData));

    ret = fibo_PDPStatus(cid, ip, &cid_status, sim_id);
    if(ret != 0){
        fibo_textTrace("[%s-%d]cid or sim input param error.",
                                __FUNCTION__, __LINE__);
        goto p_exit;
    }

    g_pdp_handle.sem = fibo_sem_new(0);
    if(cid_status == 1){
        /* already active PDP */
    }
    else{
        while (test)
        {
            fibo_reg_info_get(&reg_info, 0);
            fibo_taskSleep(1000);
            fibo_textTrace("[%s-%d]now creg is not OK...", __FUNCTION__, __LINE__);
            if (reg_info.nStatus == 1)
            {
                fibo_textTrace("[%s-%d]now creg is OK, begin active ...", __FUNCTION__, __LINE__);
                ret = fibo_PDPActive(1, NULL, NULL, NULL, 0, 0, NULL);
                fibo_textTrace("[%s-%d] ret = %d", __FUNCTION__, __LINE__, ret);
                fibo_sem_wait(g_pdp_handle.sem);
                fibo_textTrace("[%s-%d] ret = %d", __FUNCTION__, __LINE__, ret);
                if(g_pdp_handle.success == 1){
                    test = 0;
                    fibo_sem_free(g_pdp_handle.sem);
                }
            }
        }
    }
    fibo_taskSleep(3000);
    fibo_thread_create(otaThreadEntry_firmware, "ota-thread", 1024 * 16, NULL, OSI_PRIORITY_NORMAL);

p_exit:
    fibo_thread_delete();
}


int appimg_enter(void *param)
{
    fibo_textTrace("application image enter, param 0x%x", param);

    prvInvokeGlobalCtors();

    fibo_thread_create(prvThreadEntry, "mythread", 1024 * 4, NULL, OSI_PRIORITY_NORMAL);
    return 0;
}


void appimg_exit(void)
{
    fibo_textTrace("application image exit");
}
