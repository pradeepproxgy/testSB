/* Copyright (C) 2019 RDA Technologies Limited and/or its affiliates("RDA").
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

#ifndef _NVM_INTERNAL_H_
#define _NVM_INTERNAL_H_

#include "hal_config.h"
#include "osi_compiler.h"

OSI_EXTERN_C_BEGIN

typedef struct
{
    uint16_t crc; // big endian
    uint16_t reserved;
} nvBinHeader_t;

typedef struct
{
    uint16_t nvid;
    uint16_t size;
} nvBinItemHeader_t;

typedef enum
{
    FIXNV_COMPRESS_ENABLE = (1 << 0),
    RUNNINGNV_COMPRESS_ENABLE = (1 << 1),
    NV_COMPRESS_ENABLE = FIXNV_COMPRESS_ENABLE | RUNNINGNV_COMPRESS_ENABLE,
} nvFlag_t;

typedef struct
{
    uint16_t nvid;             ///< 16bits NV ID
    uint16_t flags;            ///< flags
    const char *fname;         ///< file name, not including directory name, NULL for "nv%04x.bin"
    const char *dname;         ///< directory name
    const char *running_dname; ///< directory name for runningnv, NULL for not writable
} nvDescription_t;

#ifdef CONFIG_SOC_8811
#include "nvm_8811.h"
#endif

#ifdef CONFIG_SOC_8821
#include "nvm_8821.h"
#endif

#ifdef CONFIG_SOC_8910
#include "nvm_8910.h"
#endif

#ifdef CONFIG_SOC_8850
#include "nvm_8850.h"
#endif

/**
 * \brief fill header and tail of IMEI in binary format
 *
 * The type at head and check digit in tail will be modified
 */
void nvmImeiBinFillHeadTail(nvmImeiBin_t *bin);

/**
 * \brief fill header and tail of IMEISV in binary format
 *
 * The type at head and svn in tail will be modified
 */
void nvmImeisvBinFillHeadTail(nvmImeisvBin_t *bin);

/**
 * \brief convert IMEI binary format to text format
 *
 * Check digit will be converted directly.
 *
 * \param bin       binary format of IMEI
 * \param str       text format of IMEI
 * \return
 *      - true on success
 *      - false on invalid parameter
 */
bool nvmImeiBinToStr(const nvmImeiBin_t *bin, nvmImeiStr_t *str);

/**
 * \brief convert IMEISV binary format to text format
 *
 * Check digit will be converted directly.
 *
 * \param bin       binary format of IMEISV
 * \param str       text format of IMEISV
 * \return
 *      - true on success
 *      - false on invalid parameter
 */
bool nvmImeisvBinToStr(const nvmImeisvBin_t *binsv, nvmImeisvStr_t *str);

/**
 * \brief convert IMEI text format to binary format
 *
 * \p str can only contain chracters of '0' to '9'. Check digit
 * will be converted directly.
 *
 * \param str       text format of IMEI
 * \param bin       binary format of IMEI
 * \return
 *      - true on success
 *      - false on invalid parameter, or invalid character
 */
bool nvmImeiStrToBin(const nvmImeiStr_t *str, nvmImeiBin_t *bin);

OSI_EXTERN_C_END
#endif
