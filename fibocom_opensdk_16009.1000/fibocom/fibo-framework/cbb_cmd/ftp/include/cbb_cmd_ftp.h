#ifndef __CBB_CMD_FTP_H__
#define __CBB_CMD_FTP_H__

#include "ffw_url.h"
#include "ffw_ftpc.h"


#define FTP_SEND_MAX_SIZE 3072
//#define FTP_GET_MAX_SIZE 513024    //501k?

#define CBB_FTPCACHE_MINSIZE 1024
#define CBB_FTPCACHE_MAXSIZE 513024

typedef void (*on_ftp_progress_t)(uint32_t total, uint32_t recv_len, void *arg);

typedef struct cbb_ftp_context_s
{
    int engine_mode;
    bool psudo;
    void *at_engine;
    bool odm_mode;
} cbb_ftp_context_t;


enum FTP_CMDNAME
{
    FTP_FUN_OPEN,
    FTP_FUN_LIST,
    FTP_FUN_RETR,
    FTP_FUN_GET,
    FTP_FUN_STOR,
    FTP_FUN_PUT,
    FTP_FUN_REST,
    FTP_FUN_CDUP,
    FTP_FUN_NOOP,
    FTP_FUN_SYST,
    FTP_FUN_PWD,
    FTP_FUN_CWD,
    FTP_FUN_DELE,
    FTP_FUN_MKD,
    FTP_FUN_RMD,
    FTP_FUN_STAT,
    FTP_FUN_REN,
    FTP_FUN_IDLE,
    FTP_FUN_DL,
    FTP_FUN_UPL,
    FTP_FUN_SIZE,
};

struct FTP_Param
{
    ffw_url_t _url;
    ffw_ftpcopt_t opt;
    ffw_ftpc_t ftp_client;
    enum FTP_CMDNAME ftp_cmdname;
    uint32_t putlen;
    ffw_rbuffer_t *putdatafifo;
    ffw_rbuffer_t *getdatafifo;
    uint32_t pos;
    uint8_t putdataclose;
    // osiMutex_t *access_lock;
    bool firstput;
    uint32_t getremain,getsize;
    uint32_t storend;
    // osiTimer_t *uploadtimer;
    bool istmstart;
    bool indication;
    char downpath[256];
    char getpath[256];
    uint8_t file_type;
    void *FTPTimer;
    uint8_t mode;
    uint8_t ftp_datamode;
    uint8_t get_datamode;
    char dir[256];
    // extension features under test
    bool engineisuart;
    int32_t baud;
    // ..
    char localpath[256];
    uint32_t cachesize;
};



int32_t cbb_cmd_ftpopen( const char *url,  const char *username,  const char *passwd,  const char *account, uint8_t sftpmode, uint16_t source_ctrl_port, uint16_t dest_ctrl_port, uint16_t source_data_port,void *ctx);


int32_t cbb_cmd_ftpclose();

int32_t cbb_cmd_ftpopen_read();

int32_t cbb_cmd_ftpstor( const char *filename,int32_t filetype,uint32_t offset);

int32_t cbb_cmd_ftpput(const char *filename ,uint32_t length,uint8_t eof ,uint8_t filetype,uint32_t offset);

int32_t cbb_cmd_ftpretr(const char *filename,uint8_t filetype,uint32_t offset,int32_t size, bool ofsenable);

int32_t cbb_cmd_ftpget_read();

int32_t cbb_cmd_ftp_get(const char *filename,uint8_t filetype,uint32_t offset,int32_t size);

int32_t cbb_cmd_ftprecv(uint32_t readsize,void *params);

int32_t cbb_cmd_ftplist(const char *path);

int32_t cbb_cmd_ftpcdup();

int32_t cbb_cmd_ftpnoop();

int32_t cbb_cmd_ftpsyst();

int32_t cbb_cmd_ftpmode(uint8_t mode);

int32_t cbb_cmd_ftppwd();

int32_t cbb_cmd_ftpcwd(const char *dir);

int32_t cbb_cmd_ftpdele(const char *filename);

int32_t cbb_cmd_ftpmkd(const char *dir);

int32_t cbb_cmd_ftprmd(const char *dir);

int32_t cbb_cmd_ftpstat(const char *dir);

int32_t cbb_cmd_ftpinfo(int32_t indication);

int32_t cbb_cmd_ftprest(uint32_t offset);

int32_t cbb_cmd_ftpren(const char *name_from,const char *name_to);

int32_t cbb_cmd_ftpdl(const char *name_from, const char *name_to);

int32_t cbb_cmd_ftpupl(const char *name_from, const char *name_to);

int32_t cbb_cmd_ftpcache(uint32_t cache);

int32_t cbb_cmd_ftpcache_read();

int32_t cbb_cmd_ftpsize(const char *servpath);


#endif // __CBB_CMD_FTP_H__