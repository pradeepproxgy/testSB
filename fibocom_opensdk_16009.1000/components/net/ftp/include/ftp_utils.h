/* Copyright (C) 2018 RDA Technologies Limited and/or its affiliates("RDA").
 * All rights reserved.
 *
 * This software is supplied "AS IS" without any warranties.
 * RDA assumes no responsibility or liability for the use of the software,
 * conveys no license or title under any patent, copyright, or mask work
 * right to the product. RDA reserves the right to make changes in the
 * software without notification.  RDA also make no representation or
 * warranty that such application will be suitable for the specified use
 * without further testing or modification.
 */

#ifndef _FTP_UTILS_H_
#define _FTP_UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "ftp_config.h"

typedef enum _ftp_log_type_t
{
    FTPLOG_CON = 0,
    FTPLOG_LIB,
    FTPLOG_AT,
} ftp_log_type_t;

void FTPLOGI(ftp_log_type_t t, const char *format, ...);

#if defined(CONFIG_FTP_OVER_SSL_SUPPORT)
#define FTPS_CA_CRT 0
#define FTPS_CLIENT_CRT 1
#define FTPS_CLIENT_PRIVATE_KEY 2
bool FTPS_Utils_SetCrt(char *pemData, uint32_t pemLen, uint8_t pemtype);
bool FTPs_Utils_GetCrt(uint8_t pemtype, uint8_t **crtPem);
#endif

#ifdef __cplusplus
}
#endif

#endif
