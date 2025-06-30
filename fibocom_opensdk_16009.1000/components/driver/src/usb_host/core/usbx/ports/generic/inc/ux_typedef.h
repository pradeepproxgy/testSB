/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/

/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** USBX Component                                                        */
/**                                                                       */
/**   Port Specific                                                       */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#ifndef UX_TYPEDEF_H
#define UX_TYPEDEF_H

typedef unsigned char U8;
typedef unsigned char u8;
typedef unsigned char UCHAR;
typedef char CHAR;

typedef unsigned short U16;
typedef unsigned short u16;
typedef unsigned short USHORT;
typedef short SHORT;

typedef unsigned int UINT;
typedef unsigned int U32;
typedef unsigned int u32;
typedef int INT;
typedef void VOID;

typedef unsigned long ULONG;
typedef long LONG;
typedef unsigned long long u64;

typedef unsigned int ALIGN_TYPE;

typedef unsigned long dma_addr_t;

typedef unsigned int size_t;
#if 0
typedef char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
#endif
#ifndef NULL
#define NULL 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef EREMOTEIO
#define EREMOTEIO 1121 /* Remote I/O error */
#endif

#endif
