
#ifndef _FIBO_FOTA_H
#define _FIBO_FOTA_H

#include "fibocom.h"

/* app img start addr */



#if defined (CONFIG_FIBOCOM_EXTAPP_FOTA) || defined (CONFIG_FIBOCOM_EXTFLASH_FOTA)
#define APP_IMG_START_ADDR (CONFIG_APPIMG_FLASH_ADDRESS - CONFIG_APPIMG_FLASH_ADDRESS)
#else
#define APP_IMG_START_ADDR (CONFIG_APPIMG_FLASH_ADDRESS - CONFIG_SPL_FLASH_ADDRESS)
#endif
#define APP_IMG_MAX_LEN    (CONFIG_APPIMG_FLASH_SIZE)

/* fota patch total length */
#define PATCH_TOTAL_LEN    550

/* file path */
#if defined(CONFIG_FIBOCOM_EXTFLASH_FOTA) || defined(CONFIG_FIBOCOM_APP_IN_EXTFLASH_FOTA)
#define DENTRY_APPFOTA    "/ext/appfota/"
#define FILE_FOTA_BIN    "/ext/appfota/fota.bin"
#define FILE_BLOCK_BAK    "/ext/appfota/block_bak.bin"
#define FILE_A_FLAG     "/ext/appfota/a_flag"
#define FILE_B_FLAG    "/ext/appfota/b_flag"
#define FILE_FOTA_APPFW_FLAG    "/ext/fota_appfw_flag"
#else
#define DENTRY_APPFOTA    "/appfota/"
#define FILE_FOTA_BIN    "/appfota/fota.bin"
#define FILE_BLOCK_BAK    "/appfota/block_bak.bin"
#define FILE_A_FLAG     "/appfota/a_flag"
#define FILE_B_FLAG    "/appfota/b_flag"
#define FILE_FOTA_APPFW_FLAG    "/fota_appfw_flag"
#endif
/* file max len*/
#define FOTA_BIN_MAX_LEN    (PATCH_TOTAL_LEN * 1024)

/* define action after FOTA */
#define APP_FOTA_SUCC    0x00
#define APP_FOTA_FAIL    0x01

#define NO_CRC_MEM_ADDR    30

struct fotabin_mem{
    char *data;
    unsigned int dlen;
};

#endif
