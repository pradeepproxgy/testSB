#include <stdint.h>
#include "app_ftp.h"

typedef enum ftp_client_status_s ftp_client_status_t;
typedef enum ftp_client_rw_status_s ftp_client_rw_status_t;

enum ftp_client_status_s
{
    FTP_STATUS_IDLE = 0,
    FTP_STATUS_CONNECTING = 1,
    FTP_STATUS_CONNECTED = 2,
};

enum ftp_client_rw_status_s
{
    FTP_STATUS_RW_IDLE = 0,
    FTP_STATUS_RW_READING,
    FTP_STATUS_RW_READ_FINISH,
    FTP_STATUS_RW_READ_FAIL,
    FTP_STATUS_RW_WRITING,
    FTP_STATUS_RW_WRITE_FINISH,
    FTP_STATUS_RW_WRITE_FAIL
};

static UINT32 g_sig_ftp_sem = 0;
static UINT32 g_sig_ftp_lock = 0;
static ftp_client_status_t g_ftp_client_status = FTP_STATUS_IDLE;
static ftp_client_rw_status_t g_ftp_client_rw_status = FTP_STATUS_RW_IDLE;

static void ftp_login_ok_callback(GAPP_SIGNAL_ID_T sig, va_list arg)
{
    fibo_mutex_lock(g_sig_ftp_lock);
    if (g_ftp_client_status == FTP_STATUS_CONNECTING)
    {
        g_ftp_client_status = FTP_STATUS_CONNECTED;
    }
    fibo_mutex_unlock(g_sig_ftp_lock);
    fibo_sem_signal(g_sig_ftp_sem);
}

static void ftp_login_fail_callback(GAPP_SIGNAL_ID_T sig, va_list arg)
{
    fibo_mutex_lock(g_sig_ftp_lock);
    if (g_ftp_client_status == FTP_STATUS_CONNECTING)
    {
        g_ftp_client_status = FTP_STATUS_IDLE;
    }
    fibo_mutex_unlock(g_sig_ftp_lock);
    fibo_sem_signal(g_sig_ftp_sem);
}

static void ftp_read_ok_callback(GAPP_SIGNAL_ID_T sig, va_list arg)
{
    fibo_mutex_lock(g_sig_ftp_lock);
    if (g_ftp_client_rw_status == FTP_STATUS_RW_READING)
    {
        g_ftp_client_rw_status = FTP_STATUS_RW_READ_FINISH;
    }
    fibo_mutex_unlock(g_sig_ftp_lock);
    fibo_sem_signal(g_sig_ftp_sem);
}

static void ftp_read_fail_callback(GAPP_SIGNAL_ID_T sig, va_list arg)
{
    fibo_mutex_lock(g_sig_ftp_lock);
    if (g_ftp_client_rw_status == FTP_STATUS_RW_READING)
    {
        g_ftp_client_rw_status = FTP_STATUS_RW_READ_FAIL;
    }
    fibo_mutex_unlock(g_sig_ftp_lock);
    fibo_sem_signal(g_sig_ftp_sem);
}

static void ftp_write_ok_callback(GAPP_SIGNAL_ID_T sig, va_list arg)
{
    fibo_mutex_lock(g_sig_ftp_lock);
    if (g_ftp_client_rw_status == FTP_STATUS_RW_WRITING)
    {
        g_ftp_client_rw_status = FTP_STATUS_RW_WRITE_FINISH;
    }
    fibo_mutex_unlock(g_sig_ftp_lock);
    fibo_sem_signal(g_sig_ftp_sem);
}

static void ftp_write_fail_callback(GAPP_SIGNAL_ID_T sig, va_list arg)
{
    fibo_mutex_lock(g_sig_ftp_lock);
    if (g_ftp_client_rw_status == FTP_STATUS_RW_WRITING)
    {
        g_ftp_client_rw_status = FTP_STATUS_RW_WRITE_FAIL;
    }
    fibo_mutex_unlock(g_sig_ftp_lock);
    fibo_sem_signal(g_sig_ftp_sem);
}

int app_ftp_init()
{
    g_sig_ftp_sem = fibo_sem_new(0);
    if (g_sig_ftp_sem == 0)
    {
        APP_LOG_INFO("sem new fail");
        goto error;
    }

    g_sig_ftp_lock = fibo_mutex_create();
    if (g_sig_ftp_lock == 0)
    {
        APP_LOG_INFO("mutex new fail");
        goto error;
    }

    app_register_sig_callback(GAPP_SIG_FTPOPEN_SUCCESS, ftp_login_ok_callback);
    app_register_sig_callback(GAPP_SIG_FTPOPEN_FAIL, ftp_login_fail_callback);
    app_register_sig_callback(GAPP_SIG_FTPREAD_SUCCESS, ftp_read_ok_callback);
    app_register_sig_callback(GAPP_SIG_FTPREAD_FALI, ftp_read_fail_callback);
    app_register_sig_callback(GAPP_SIG_FTPWRITE_SUCCESS, ftp_write_ok_callback);
    app_register_sig_callback(GAPP_SIG_FTPWRITE_FAIL, ftp_write_fail_callback);

    return 0;

error:
    fibo_mutex_delete(g_sig_ftp_lock);
    g_sig_ftp_lock = 0;
    fibo_sem_free(g_sig_ftp_sem);
    g_sig_ftp_sem = 0;
    return -1;
}

int app_ftp_open(char *server_addr, char *username, char *passwd)
{
    int ret = 0;
    fibo_mutex_lock(g_sig_ftp_lock);
    if (g_ftp_client_status == FTP_STATUS_CONNECTED)
    {
        fibo_mutex_unlock(g_sig_ftp_lock);
        APP_LOG_INFO("ftp have login");
        return 0;
    }

    ret = fibo_ftp_open(server_addr, username, passwd);
    if ((0 != ret))
    {
        fibo_mutex_unlock(g_sig_ftp_lock);
        APP_LOG_INFO("ftp login fail");
        return -1;
    }

    g_ftp_client_status = FTP_STATUS_CONNECTING;
    APP_WAIT_EXPECT_STATUS(g_sig_ftp_lock, g_sig_ftp_sem, g_ftp_client_status == FTP_STATUS_CONNECTING, g_ftp_client_status);
    ret = (g_ftp_client_status == FTP_STATUS_CONNECTED) ? 0 : -1;
    fibo_mutex_unlock(g_sig_ftp_lock);
    return ret;
}

int app_ftp_download(char *server_path, char *local_path)
{
    int ret = 0;
    fibo_mutex_lock(g_sig_ftp_lock);
    if (g_ftp_client_status != FTP_STATUS_CONNECTED)
    {
        fibo_mutex_unlock(g_sig_ftp_lock);
        APP_LOG_INFO("ftp not login");
        return -1;
    }

    APP_WAIT_EXPECT_STATUS(g_sig_ftp_lock, g_sig_ftp_sem, g_ftp_client_rw_status != FTP_STATUS_RW_IDLE, g_ftp_client_rw_status);
    g_ftp_client_rw_status = FTP_STATUS_RW_READING;
    ret = fibo_ftp_read(server_path, local_path);
    if ((0 != ret))
    {
        
        g_ftp_client_rw_status = FTP_STATUS_RW_IDLE;
        fibo_mutex_unlock(g_sig_ftp_lock);
        APP_LOG_INFO("ftp read fail");
        return -1;
    }

    APP_WAIT_EXPECT_STATUS(g_sig_ftp_lock, g_sig_ftp_sem, g_ftp_client_rw_status == FTP_STATUS_RW_READING, g_ftp_client_rw_status);
    ret = g_ftp_client_rw_status == FTP_STATUS_RW_READ_FINISH ? 0 : -1;
    g_ftp_client_rw_status = FTP_STATUS_RW_IDLE;
    fibo_mutex_unlock(g_sig_ftp_lock);
    return ret;
}

int app_ftp_uploadload(char *server_path, char *local_path)
{
    int ret = 0;
    fibo_mutex_lock(g_sig_ftp_lock);
    if (g_ftp_client_status != FTP_STATUS_CONNECTED)
    {
        fibo_mutex_unlock(g_sig_ftp_lock);
        APP_LOG_INFO("ftp not login");
        return -1;
    }

    APP_WAIT_EXPECT_STATUS(g_sig_ftp_lock, g_sig_ftp_sem, g_ftp_client_rw_status != FTP_STATUS_RW_IDLE, g_ftp_client_rw_status);
    g_ftp_client_rw_status = FTP_STATUS_RW_WRITING;
    ret = fibo_ftp_write(server_path, local_path);
    if ((0 != ret))
    {
        g_ftp_client_rw_status = FTP_STATUS_RW_IDLE;
        fibo_mutex_unlock(g_sig_ftp_lock);
        APP_LOG_INFO("ftp read fail");
        return -1;
    }
    
    APP_WAIT_EXPECT_STATUS(g_sig_ftp_lock, g_sig_ftp_sem, g_ftp_client_rw_status == FTP_STATUS_RW_WRITING, g_ftp_client_rw_status);
    ret = g_ftp_client_rw_status == FTP_STATUS_RW_WRITE_FINISH ? 0 : -1;
    g_ftp_client_rw_status = FTP_STATUS_RW_IDLE;
    fibo_mutex_unlock(g_sig_ftp_lock);
    return ret;
}

int app_ftp_close()
{
    int ret = 0;
    fibo_mutex_lock(g_sig_ftp_lock);
    if (g_ftp_client_status != FTP_STATUS_CONNECTED)
    {
        fibo_mutex_unlock(g_sig_ftp_lock);
        APP_LOG_INFO("ftp not login");
        return -1;
    }

    while (g_ftp_client_rw_status != FTP_STATUS_RW_IDLE)
    {
        fibo_mutex_unlock(g_sig_ftp_lock);
        fibo_sem_try_wait(g_sig_ftp_sem, 1000);
        fibo_mutex_lock(g_sig_ftp_lock);
        APP_LOG_INFO("wait ftp idle");
    }

    fibo_ftp_close();

    return ret;
}

void app_ftp_demo()
{
    char serv_addr[] = "47.110.234.36";
    char username[] = "ftpuser1";
    char password[] = "test";
    char localfile[] = "/testftp.txt";
    char localfile1[] = "/testftp1.txt";
    char servfile[] = "/server.txt";

    INT32 fd = fibo_file_open(localfile, (O_CREAT | O_RDWR));
    if (fd > 0)
    {
        fibo_file_write(fd, (UINT8 *)"test ftpxxxxxxxx", sizeof("test ftpxxxxxxxx"));
        fibo_file_close(fd);
    }

    int ret = app_ftp_open(serv_addr, username, password);
    if (ret != 0)
    {
        APP_LOG_INFO("ftp login fail");
        return;
    }

    ret = app_ftp_uploadload(servfile, localfile);
    if (ret != 0)
    {
        APP_LOG_INFO("ftp upload fail");
        return;
    }

    app_ftp_download(servfile, localfile1);
    if (ret != 0)
    {
        APP_LOG_INFO("ftp download fail");
        return;
    }
}