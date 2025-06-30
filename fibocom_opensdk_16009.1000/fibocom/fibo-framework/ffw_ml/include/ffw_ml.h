/**
 * @file ffw_ml.h
 * @author sundaqing (sundaqing@fibocom.com)
 * @brief 编码转换工具函数，支持 utf8 utf16 gbk gsm的转换
 * @version 0.1
 * @date 2021-10-26
 * 
 * 
 */

#ifndef CD2988FA_1BAF_4D37_94BF_36A62E1DA06B
#define CD2988FA_1BAF_4D37_94BF_36A62E1DA06B

/* Copyright (C) 2016 RDA Technologies Limited and/or its affiliates("RDA").
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

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum
{
    FFW_ML_ISO8859_1,
    FFW_ML_UTF8,
    FFW_ML_UTF16BE,
    FFW_ML_UTF16LE,
    FFW_ML_GSM,
    FFW_ML_CP936
};

void ffw_mlInit(void);

/**
 * convert a string charset
 *
 * When \a from_charset or \a to_charset is unknown or unsupported,
 * return NULL.
 *
 * The returned pointer is allocated inside, caller should free it.
 *
 * At most \a from_size byte count will be processed. When \a from_size
 * is -1, process till null chracter.
 *
 * null character will always be inserted into the output string.
 * Invalid characters are replaced silently.
 *
 * \a to_size can be NULL in case that the output byte count is not needed.
 *
 * @param from          input string
 * @param from_size     input string byte count
 * @param from_chset    input string charset
 * @param to_chset      output string charset
 * @param to_size       output string byte count
 * @return
 *      - NULL: invalid parameters
 *      - output string
 */
void *ffw_mlConvertStr(const void *from, int from_size, unsigned from_chset, unsigned to_chset, int *to_size);

int ffw_mlCharCount(const void *s, unsigned chset);

int ffw_mlStrBytes(const void *s, unsigned chset);

uint16_t ffw_mlGetUnicode(uint16_t from, uint32_t from_chset);
uint16_t ffw_mlGetOEM(uint32_t unicd, uint32_t to_chset);

#ifdef __cplusplus
}
#endif


#endif /* CD2988FA_1BAF_4D37_94BF_36A62E1DA06B */
