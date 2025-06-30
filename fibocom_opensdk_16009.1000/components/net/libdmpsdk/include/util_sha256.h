/*******************************************************************************
* Company		  :  unicom-IOT
* Author		  :  author
* Version		  :  V1.0
* Date			  :  2021-05-27
* Description	  :  util_sha256.h
********************************************************************************/

/********************************* Include File ********************************/

#ifndef __UTIL_SHA256_H__
#define __UTIL_SHA256_H__

#include <stdint.h>

/********************************* Macro Definition ****************************/
#define UNI_SHA256_KEY_SIZE   (64)
#define UNI_SHA256_TARGET_SIZE      (32)

/********************************* Type Definition *****************************/
typedef struct {
    uint32_t total[2];          
    uint32_t state[8];          
    unsigned char buffer[64];   
    int type;                  /*0:SHA-256, not 0: SHA-224 */
} uni_sha256_s;

/********************************* Variables ***********************************/

/********************************* Function ************************************/
void utils_sha256_init(uni_sha256_s *tex);
void utils_sha256_starts(uni_sha256_s *tex);
void utils_sha256_update(uni_sha256_s *tex, const unsigned char *in, uint32_t in_len);
void utils_sha256_finish(uni_sha256_s *tex, uint8_t out[32]);
#endif
