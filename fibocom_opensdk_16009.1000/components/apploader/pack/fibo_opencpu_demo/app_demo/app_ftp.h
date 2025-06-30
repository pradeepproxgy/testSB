#ifndef FCCF9E60_F616_4E32_987C_E979C73D97AB
#define FCCF9E60_F616_4E32_987C_E979C73D97AB

#include "app.h"

int app_ftp_init();

int app_ftp_open(char *server_addr, char *username, char *passwd);

int app_ftp_download(char *server_path, char *local_path);

int app_ftp_uploadload(char *server_path, char *local_path);

int app_ftp_close();

void app_ftp_demo();


#endif /* FCCF9E60_F616_4E32_987C_E979C73D97AB */
