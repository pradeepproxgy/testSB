/*----------------------------------------------+
 |												|
 |	ivPlatform.h - Platform Config				|
 |												|
 |		Platform: ADS1.2 (ARM)					|
 |												|
 |		Copyright (c) 1999-2007, iFLYTEK Ltd.	|
 |		All rights reserved.					|
 |												|
 +----------------------------------------------*/

#ifndef __IVPLATFROM_H__
#define __IVPLATFROM_H__
/*
 *	TODO: ���������Ŀ��ƽ̨������Ҫ�Ĺ���ͷ�ļ�
 */
#include <string.h>

/*
 *	TODO: ����Ŀ��ƽ̨�����޸����������ѡ��
 */

#define IV_UNIT_BITS 8  /* �ڴ������Ԫλ�� */
#define IV_BIG_ENDIAN 0 /* �Ƿ��� Big-Endian �ֽ��� */
#define IV_PTR_GRID 4   /* ���ָ�����ֵ */

#define IV_PTR_PREFIX                   /* ָ�����ιؼ���(����ȡֵ�� near | far, ����Ϊ��) */
#define IV_CONST const                  /* �����ؼ���(����Ϊ��) */
#define IV_EXTERN extern                /* �ⲿ�ؼ��� */
#define IV_STATIC static                /* ��̬�����ؼ���(����Ϊ��) */
#define IV_INLINE __inline              /* �����ؼ���(����ȡֵ�� inline, ����Ϊ��) */
#define IV_CALL_STANDARD /*__stdcall*/  /* ��ͨ�������ιؼ���(����ȡֵ�� stdcall | fastcall | pascal, ����Ϊ��) */
#define IV_CALL_REENTRANT /*__stdcall*/ /* �ݹ麯�����ιؼ���(����ȡֵ�� stdcall | reentrant, ����Ϊ��) */
#define IV_CALL_VAR_ARG /*__cdecl*/     /* ��κ������ιؼ���(����ȡֵ�� cdecl, ����Ϊ��) */

#define IV_TYPE_INT8 char   /* 8λ�������� */
#define IV_TYPE_INT16 short /* 16λ�������� */
#define IV_TYPE_INT24 int   /* 24λ�������� */
#define IV_TYPE_INT32 long  /* 32λ�������� */

#if 1                           /* 48/64 λ���������ǿ�ѡ��, ��Ǳ�Ҫ��Ҫ����, ��ĳЩ 32 λƽ̨��, ʹ��ģ�ⷽʽ�ṩ�� 48/64 λ������������Ч�ʺܵ� */
#define IV_TYPE_INT48 long long /* 48λ�������� */
#define IV_TYPE_INT64 long long /* 64λ�������� */
#endif

#define IV_TYPE_ADDRESS unsigned int /* ��ַ�������� */
#define IV_TYPE_SIZE unsigned int    /* ��С�������� */

#define IV_ANSI_MEMORY 1 /* �Ƿ�ʹ�� ANSI �ڴ������ */
#define IV_ANSI_STRING 0 /* �Ƿ�ʹ�� ANSI �ַ��������� */

#define IV_ASSERT(exp) /*_ASSERT(exp)*/ /* ���Բ���(����Ϊ��) */
#define IV_YIELD /*Sleep(0)*/           /* ���в���(��Э��ʽ����ϵͳ��Ӧ����Ϊ�����л�����, ����Ϊ��) */

#define IV_ARCH_ARM 1 /* ����ARM����ζ���� */

/* ����ƽ̨����ѡ������Ƿ�֧�ֵ��� */
#if defined(DEBUG) || defined(_DEBUG)
#define IV_DEBUG 0 /* �Ƿ�֧�ֵ��� */
#else
#define IV_DEBUG 0 /* �Ƿ�֧�ֵ��� */
#endif

/* ���Է�ʽ���������־ */
#ifndef IV_LOG
#define IV_LOG IV_DEBUG /* �Ƿ������־ */
#endif

/* ����ƽ̨����ѡ������Ƿ��� Unicode ��ʽ���� */
#if defined(UNICODE) || defined(_UNICODE)
#define IV_UNICODE 1 /* �Ƿ��� Unicode ��ʽ���� */
#else
#define IV_UNICODE 0 /* �Ƿ��� Unicode ��ʽ���� */
#endif

#endif
