#ifndef __FFW_GB2312_AND_UNICODE__
#define __FFW_GB2312_AND_UNICODE__

#include "fibo_opencpu_comm.h"

/**
 * @brief gd2312码转换为unicode码
 * 
 * @param gb_char  gb码
 * @param gb_length gb码长度
 * @param unicode_char unicode码
 * @param unicode_length unicode码长度
 * @return INT32 
 * @retval 0 成功
 * @retval -1 失败
 */

INT32 fibo_gb2312_to_unicode(UINT16 gb_char, INT32 gb_length, UINT16* unicode_char, INT32* unicode_length);

/**
 * @brief 
 * 
 * @param unicode_char unicode码
 * @param unicode_length unicode码长度
 * @param gb_char gb码
 * @param gb_length gb码长度
 * @return INT32 
 * @retval 0 成功
 * @retval -1 失败
 */

INT32 fibo_unicode_to_gb2312(UINT16 unicode_char, INT32 unicode_length, UINT16* gb_char, INT32 *gb_length);

#endif /* __FFW_GB2312_AND_UNICODE__ */
