/* Copyright (C) 2018 FIBOCOM Technologies Limited and/or its affiliates("FIBOCOM").
 * All rights reserved.
 *
 * This software is supplied "AS IS" without any warranties.
 * FIBOCOM assumes no responsibility or liability for the use of the software,
 * conveys no license or title under any patent, copyright, or mask work
 * right to the product. FIBOCOM reserves the right to make changes in the
 * software without notification.  FIBOCOM also make no representation or
 * warranty that such application will be suitable for the specified use
 * without further testing or modification.
 */

#define OSI_LOG_TAG OSI_MAKE_LOG_TAG('M', 'Y', 'A', 'P')

#include "fibo_opencpu.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "mbedtls/md.h"
#include "mbedtls/sha1.h"
#include "mbedtls/base64.h"
#include "mbedtls/aes.h"


extern void test_printf(void);

static void prvInvokeGlobalCtors(void)
{
    extern void (*__init_array_start[])();
    extern void (*__init_array_end[])();

    size_t count = __init_array_end - __init_array_start;
    for (size_t i = 0; i < count; ++i)
        __init_array_start[i]();
}


#define HMAC_SHA1_LEN 20
#define HMAC_SHA256_LEN 32

void mbedtls_md_init( mbedtls_md_context_t *ctx )
{
    memset( ctx, 0, sizeof( mbedtls_md_context_t ) );
}

void hmac_sha_test(int md_type, const char *api_secret, int key_len, const char *payload, int payload_len, UINT8 *encrypt_result, int len)
{
  int i, j= 0;
  mbedtls_md_context_t ctx;
  unsigned char hmac_hash1[HMAC_SHA1_LEN] = {'\0'};
  unsigned char hmac_hash256[HMAC_SHA256_LEN] = {'\0'};
  {
    mbedtls_md_init(&ctx);
    fibo_mbedtls_md_setup(&ctx, fibo_mbedtls_md_info_from_type(md_type) , 1); //use hmac
    fibo_mbedtls_md_hmac_starts(&ctx, (const unsigned char *)api_secret, key_len);
    fibo_mbedtls_md_hmac_update(&ctx, (const unsigned char *)payload, payload_len);
    
    //convert bytes to hex string
    if(md_type == MBEDTLS_MD_SHA1)
    {
       fibo_mbedtls_md_hmac_finish(&ctx, hmac_hash1);
       for(i=0; i < HMAC_SHA1_LEN; i++)
       {
           fibo_textTrace("%02x", hmac_hash1[i]);
           sprintf((char *)encrypt_result + (i + j), "%02x", hmac_hash1[i]);
           j++;
       }

       }
       else if(MBEDTLS_MD_SHA256)
       {
           fibo_mbedtls_md_hmac_finish(&ctx, hmac_hash256);
           for(i=0; i < HMAC_SHA256_LEN; i++)
           {
               fibo_textTrace("%02x", hmac_hash256[i]);
               sprintf((char *)encrypt_result + (i + j), "%02x", hmac_hash256[i]);
               j++;
           }

       }

    fibo_mbedtls_md_free(&ctx); 
  }
 
}

static void hmac_test_thread(void *param)
{
    fibo_textTrace("application thread enter, param 0x%x", param);
    //srand(100);

    for (int n = 0; n < 10; n++)
    {
        fibo_textTrace("hello world %d", n);
        fibo_taskSleep(1000);
    }
    UINT8 encrypt_result[40]={0};
    //test hmac_sha1
    hmac_sha_test(MBEDTLS_MD_SHA1,"123",3,"abc",3,encrypt_result,20);
    fibo_textTrace("encrypt_result=%s", encrypt_result);
    //test hmac_sha256
    hmac_sha_test(MBEDTLS_MD_SHA256,"123",3,"abc",3,encrypt_result,32);
    fibo_textTrace("encrypt_result=%s", encrypt_result);

    while(1)
    {
        fibo_textTrace("hello world");
        fibo_taskSleep(10000);
    }

    test_printf();
    fibo_thread_delete();
}

void * appimg_enter(void *param)
{
    fibo_textTrace("application image enter, param 0x%x", param);

    prvInvokeGlobalCtors();

    fibo_thread_create(hmac_test_thread, "mythread", 1024*4, NULL, OSI_PRIORITY_NORMAL);
    return 0;
}

void appimg_exit(void)
{
    fibo_textTrace("application image exit");
}
