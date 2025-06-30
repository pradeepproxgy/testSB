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

#define OSI_LOG_TAG OSI_MAKE_LOG_TAG('F', 'T', 'P', 'C')

#include "fibo_opencpu.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

static void prvInvokeGlobalCtors(void)
{
    extern void (*__init_array_start[])();
    extern void (*__init_array_end[])();

    size_t count = __init_array_end - __init_array_start;
    for (size_t i = 0; i < count; ++i)
        __init_array_start[i]();
}

#define THREAD_STACK_SIZE 16 * 1024

#define FTP_R_SUCCESS 0
#define FTP_R_FAILED -1

#define SERVER_PATH "./ftpopen_test.txt"
#define LOCAL_PATH "./ftpopen_local.txt"
#define SERVER_PATH2 "./ftpopen_server_path2.txt"

#define fibo_textTrace(format, ...)                                                              \
    do                                                                                           \
    {                                                                                            \
        fibo_textTrace("[ftpdemo][%s:%d]" format "\r\n", __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    } while (0);

typedef enum ftp_client_status_s ftp_client_status_t;

enum ftp_client_status_s
{
    FTP_STATUS_IDLE = 0,
    FTP_STATUS_CONNECTING = 1,
    FTP_STATUS_CONNECTED = 2,
    FTP_STATUS_READING = 3,
    FTP_STATUS_READ_FINISH = 4,
    FTP_STATUS_WRITING = 5,
    FTP_STATUS_WRITE_FINISH = 6,
    FTP_STATUS_ABNORMAL = 7
};

static UINT32 g_sem = 0;
static ftp_client_status_t g_ftp_client_status = FTP_STATUS_IDLE;
static void ftp_signal_progress(GAPP_SIGNAL_ID_T sig, va_list arg);

static FIBO_CALLBACK_T g_ftp_user_callback =
{
    .at_resp = NULL,
    .fibo_signal = ftp_signal_progress
};

static void ftp_signal_progress(GAPP_SIGNAL_ID_T sig, va_list arg)
{
    fibo_textTrace("sig:%lu", sig);

    switch (sig)
    {
        case GAPP_SIG_FTPOPEN_SUCCESS:
            if (g_ftp_client_status == FTP_STATUS_CONNECTING)
            {
                g_ftp_client_status = FTP_STATUS_CONNECTED;
                if (g_sem)
                {
                    fibo_sem_signal(g_sem);
                }
            }
            break;
        case GAPP_SIG_FTPOPEN_FAIL:
            if (g_sem)
            {
                g_ftp_client_status = FTP_STATUS_ABNORMAL;
                fibo_sem_signal(g_sem);
            }
            break;
        case GAPP_SIG_FTPREAD_SUCCESS:
            if (g_ftp_client_status == FTP_STATUS_READING)
            {
                g_ftp_client_status = FTP_STATUS_READ_FINISH;
                if (g_sem)
                {
                    fibo_sem_signal(g_sem);
                }
            }
            break;
        case GAPP_SIG_FTPREAD_FALI:
            if (g_sem)
            {
                g_ftp_client_status = FTP_STATUS_ABNORMAL;
                fibo_sem_signal(g_sem);
            }
            break;
        case GAPP_SIG_FTPWRITE_SUCCESS:
            if (g_ftp_client_status == FTP_STATUS_WRITING)
            {
                g_ftp_client_status = FTP_STATUS_WRITE_FINISH;
                if (g_sem)
                {
                    fibo_sem_signal(g_sem);
                }
            }
            break;
        case GAPP_SIG_FTPWRITE_FAIL:
            if (g_sem)
            {
                g_ftp_client_status = FTP_STATUS_ABNORMAL;
                fibo_sem_signal(g_sem);
            }
            break;
        default:
            break;
    }
    return;
}

static void pdp_thread_entry(void *arg)
{
    int ret = FTP_R_SUCCESS;
    bool stop = false;

    char *server_addr = "47.110.xxx.xx:21";
    char *username = "test";
    char *passwd = "test";

    reg_info_t reginfo = {0};
    uint8_t simid = fibo_get_dualsim();

    fibo_textTrace("thread:%p, simid:%u", osiThreadCurrent(), simid);

    while (true)
    {
        fibo_reg_info_get(&reginfo, 0);
        fibo_textTrace("reginfo.nStatus:%u", reginfo.nStatus);

        if (reginfo.nStatus == 1)
        {
            fibo_pdp_active(1, NULL, 0);
            fibo_taskSleep(10 * 1000);
            if (g_sem == 0)
            {
                g_sem = fibo_sem_new(0);
                fibo_textTrace("g_sem create");
            }
            break;
        }
        fibo_taskSleep(1000);
    }

    // Establish FTP connection
    g_ftp_client_status = FTP_STATUS_IDLE;
    ret = fibo_ftp_open(server_addr, username, passwd);

    if ((FTP_R_SUCCESS == ret) && (g_ftp_client_status == FTP_STATUS_IDLE))
    {
        fibo_textTrace("ftp open success, but the signal GAPP_SIG_FTPOPEN_SUCCESS no recived!");
        g_ftp_client_status = FTP_STATUS_CONNECTING;
    }
    else
    {
        fibo_textTrace("ftp open failed, ret:%d, g_ftp_client_status:%lu", ret, g_ftp_client_status);
        goto error;
    }

    while(!stop)
    {
        fibo_sem_wait(g_sem);
        fibo_textTrace("recv g_sem:%p, g_ftp_client_status:%lu", g_sem, g_ftp_client_status);

        switch (g_ftp_client_status)
        {
            // Connection complete processing Download
            case FTP_STATUS_CONNECTED:
                g_ftp_client_status = FTP_STATUS_READING;
                ret = fibo_ftp_read(SERVER_PATH, LOCAL_PATH);
                if (ret == FTP_R_FAILED)
                {
                    fibo_textTrace("ftp read failed!");
                    goto error;
                }
                break;
            // Download complete processing upload
            case FTP_STATUS_READ_FINISH:
                fibo_taskSleep(60*1000);
                g_ftp_client_status = FTP_STATUS_WRITING;
                ret = fibo_ftp_write(SERVER_PATH2, LOCAL_PATH);
                if (ret == FTP_R_FAILED)
                {
                    fibo_textTrace("ftp write failed!");
                    goto error;
                }
                break;
            //Upload completed
            case FTP_STATUS_WRITE_FINISH:
                stop = true;
                fibo_textTrace("ftp progress finish exit, stop:%u", stop);
                break;
            default:
                goto error;
        }
    }

error:
        g_ftp_client_status = FTP_STATUS_IDLE;
        fibo_sem_free(g_sem);
        fibo_ftp_close();
        fibo_thread_delete();
        return;
}

/**
 * @brief application enter function
 *
 */
int appimg_enter(void *arg)
{
    fibo_textTrace("application start exec and arg:%p", arg);
    prvInvokeGlobalCtors();

    // Create thread to process FTP business
    if (FTP_R_SUCCESS == fibo_thread_create(pdp_thread_entry, "demo_thread", THREAD_STACK_SIZE, NULL, OSI_PRIORITY_BELOW_NORMAL))
    {
        fibo_textTrace("demo thread create success!");
    }
    else
    {
        fibo_textTrace("demo create failed!");
    }

    return (int)(&g_ftp_user_callback);
}

/**
 * @brief application exit function
 *
 */
void appimg_exit(void)
{
    fibo_textTrace("application exit exec!");
    return;
}