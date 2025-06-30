/*******************************************************************************
* Company		  :  unicom-IOT
* Author		  :  author
* Version		  :  V1.0
* Date			  :  2021-05-27
* Description	  :  util_string.h
********************************************************************************/

/********************************* Include File ********************************/
#ifndef _UTIL_STRING_H_
#define _UTIL_STRING_H_
#include <string.h>
#include "util_types.h"
/********************************* Macro Definition ****************************/



/********************************* Type Definition *****************************/


/********************************* Variables ***********************************/

/********************************* Function ************************************/
void util_hex2str(uint8_t *in, uint16_t in_len, char *out);
char *util_strtok(char *str, const char *wstr);
#endif
