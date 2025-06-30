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

#ifndef _HALO_REC_DIAG_H_
#define _HALO_REC_DIAG_H_

#include <stdint.h>
#include "diag.h"

uint8_t HALO_factory_CheckLicense(const diagMsgHead_t *cmd);
uint8_t HALO_factory_ReadUid(const diagMsgHead_t *cmd);
uint8_t HALO_factory_WriteLicense(const diagMsgHead_t *cmd);

#endif