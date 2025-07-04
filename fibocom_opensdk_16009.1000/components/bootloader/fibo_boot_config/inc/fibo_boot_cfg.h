/* Copyright (C) 2017 RDA Technologies Limited and/or its affiliates("RDA").
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

#ifndef _FIBO_BOOT_CFG_H_
#define _FIBO_BOOT_CFG_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "boot_config.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PWKEY_OVER7S_RESET 1 // 0:RESET,1 NOT RESET,DEFAULT:1

int fibo_get_pwkey_over7s_reset();
#ifdef __cplusplus
}
#endif
#endif
