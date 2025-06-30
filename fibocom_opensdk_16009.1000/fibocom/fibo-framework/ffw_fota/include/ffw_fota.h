#include "ffw_types.h"
#include "fibocom.h"
#ifndef _FIBO_FOTA_H_
#define _FIBO_FOTA_H_

/* download max len of packet */
/*#ifdef FOTABACKUP_ENABLE*/
#define FOTA_PATH "/FBFOTA"
#define FOTA_URL "/FBFOTA/URL"
#define FOTA_BK "/FBFOTA/bk.bin"
#define FOTA_POINT "/FBFOTA/breakend.txt"
#define FOTA_TOTOLELEN "/FBFOTA/len.txt"

#define FOTA_SIZE 10*1024

typedef struct {
    /* save download data */
    char *  data;
    int temp_len;
    int len;
} fota_info_ext;

typedef struct {
    char *  data;
    uint32_t write_len;
    int temp_len;
    int total_len;
    int is_fota_bk;
} fota_bk_info;
/*#endif*/

#define MAX_LEN_FOTABIN     (1024 * 1024)
#define FOTA_ENABLE 1
#define FOTA_DISABLE 0
#define MAX_URL_LEN         255

#define STATE_RECV_RSP      1
#define STATE_RECV_DATA     2

typedef void (*FOTA_FUNCTION)(void);
typedef void (*FOTA_OPENCPU)(int err, char *data, int len);

struct fota_callback {
    FOTA_FUNCTION   fwrite_err;
    FOTA_FUNCTION   check_err;
    FOTA_FUNCTION   reboot_before;
};

struct fota_http_callback {
    void    (*cb_connect)(void);
    int     (*cb_response)(int total_len);
    void    (*cb_data)(char *data, int len);
    void    (*cb_error)(void);
};

#define FTP_MAX_LEN  256
struct ftp_para {
    char    host[FTP_MAX_LEN];
    uint16_t    port;
    char    username[FTP_MAX_LEN];
    char    passwd[FTP_MAX_LEN];
    char    path[FTP_MAX_LEN];
    char    tls;
};

struct fota_ftp_callback {
    void    (*cb_connect)(void);
    int     (*cb_response)(int total_len);
    void    (*cb_data)(char *data, int len);
    void    (*cb_error)(void);
};

typedef struct {
    ffw_handle_t engine;
    char    url[512];

    /* is in fota? */
    int enable;

    /* save download data */
    char *  data;
    int len;
    int total_len;
    int last_per;
    int state;
    int fail_log;

    /* save ftp parameter */
    struct ftp_para fpara;
} fota_info;

/*ifdef FOTABACKUP_ENABLE*/
void fibo_fota_delffs(void);
int fibo_fw_update(const void *data, unsigned size, struct fota_callback *cb);
/*#endif*/
int fota_http_handle(char *url, struct fota_http_callback *cb);
int gnss_http_handle(char *url, struct fota_http_callback *cb);
int fota_ftp_handle(struct ftp_para *para, struct fota_ftp_callback *cb);
int fibo_is_foat(void);
void cbb_gtota(int cmd_type, void* params, int params_count, void* engine);
void ffw_fota_report(void *param);
void cbb_qfotadl(int cmd_type, void* params, int params_count, void* engine);
char ffw_is_fota_enable(void);
void fibo_fota_del();
#ifdef CONFIG_FIBOCOM_BLE_SUPPORT
void cbb_ecota(int cmd_type, void* params, int params_count, void* engine);
void ble_ota_thread();
int fibo_ble_ota_check(const char *data);
#endif
#endif
