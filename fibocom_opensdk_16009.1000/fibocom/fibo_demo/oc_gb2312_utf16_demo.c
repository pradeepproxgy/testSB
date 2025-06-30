/* Copyright (C) 2022 FIBOCOM Technologies Limited and/or its affiliates("FIBOCOM").
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

#define OSI_LOG_TAG OSI_MAKE_LOG_TAG('G', 'B', 'U', 'T')

#include "fibo_opencpu.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#define THREAD_STACK_SIZE 16 * 1024

#define E_RES_SUCCESS 0
#define E_RES_FAILED -1

#define DEMO_LOG(format, ...)                                                              \
    do                                                                                     \
    {                                                                                      \
        fibo_textTrace("[demo][%s:%d]" format "\r\n", __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    } while (0);

static void prvInvokeGlobalCtors(void)
{
    extern void (*__init_array_start[])();
    extern void (*__init_array_end[])();

    size_t count = __init_array_end - __init_array_start;
    for (size_t i = 0; i < count; ++i)
        __init_array_start[i]();
}

static void prvThreadEntry(void *param)
{
    // 汉字(严)<-->gb2312(0xD1CF)<-->uft16BE(0x4E25)

    UINT16 gb_char1 = 0xD1CF;
    INT32 gb_len1 = 2;
    UINT16 uft16_char1;
    INT32 uft16_length1;

    // gb2312 --> uft16BE
    if(E_RES_SUCCESS == fibo_gb2312_to_unicode(gb_char1, gb_len1, &uft16_char1, &uft16_length1))
    {
        DEMO_LOG("from gb2312 to utf16LE success, uft16_length1:%d, uft16_char1:0x%x", uft16_length1, uft16_char1);
    }
    else
    {
        DEMO_LOG("from gb2312 to utf16LE fail!");
    }

    UINT16 uft16_char2 = 0x4E25;
    INT32 uft16_length2 = 2;
    UINT16 gb_char2;
    INT32 gb_len2;

    // uft16BE --> gb2312
    if (E_RES_SUCCESS == fibo_unicode_to_gb2312(uft16_char2, uft16_length2, &gb_char2, &gb_len2))
    {
        DEMO_LOG("from utf16LE to success, gb_len2:%d, gb_char2:0x%x", gb_len2, gb_char2);
    }
    else
    {
        DEMO_LOG("from utf16LE to gb2312 fail!");
    }

    fibo_thread_delete();
}


/**
 * @brief application enter function
 *
 */
int appimg_enter(void *arg)
{
    DEMO_LOG("application start exec and arg:%p", arg);
    prvInvokeGlobalCtors();

    // 创建线程处理ftp业务
    if (E_RES_SUCCESS == fibo_thread_create(prvThreadEntry, "demo_thread", THREAD_STACK_SIZE, NULL, OSI_PRIORITY_BELOW_NORMAL))
    {
        DEMO_LOG("demo thread create success!");
    }
    else
    {
        DEMO_LOG("demo create failed!");
    }

    return (int)(0);
}

/**
 * @brief application exit function
 *
 */
void appimg_exit(void)
{
    DEMO_LOG("application exit exec!");
    return;
}