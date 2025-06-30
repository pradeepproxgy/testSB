/*******************************************************************************
** File Name:       ldpc_dec.h
** Author:          Xufeng Guo
** Date:            2017/3/10
** Copyright:       2014 Spreadtrum, Incoporated. All Rights Reserved.
** Description:     Defines and functions for LDPC decoding.
********************************************************************************
********************************************************************************
**                              Edit History
** -----------------------------------------------------------------------------
** DATE                         NAME                      DESCRIPTION
** Year/Month/Data        ***                               Create.
** Year/Month/Data        ***                           modified for  ***
*******************************************************************************/

#ifndef _LDPC_DEC_H_
#define _LDPC_DEC_H_
#ifndef _VAR_TYPE_DEF_
#define _VAR_TYPE_DEF_
#define CONSTANT64(value) ((long long unsigned int)value)		//@Zhongwei.zhan 2014-11-11

typedef int                     BOOL;
typedef long long               S64;
typedef unsigned long long      U64;
typedef unsigned long long      UINT64;
typedef signed   long long      INT64;
typedef unsigned int            U32;
typedef signed int              S32;
typedef unsigned short          U16;
typedef signed short            S16;
typedef unsigned char            U8;
typedef signed char             S8;

typedef float FLOAT;	
typedef double DOUBLE;
#endif

typedef struct
{
	int maxIter;
	int earlyTermiAlg;
	int minIterET;
	float maxLLRTh;

	// NB LDPC
	int Nm;
	float offset;
	
	//fxp
	int fxpEnable;
	int fracBW;
	int vnBW;
	int cnBW;
}LDPC_DEC_PAR;
typedef struct
{
    U8 svid;
    U8 sub2Symbols[1200];
}B1C_FRAME2_SYMBOL, *PB1C_FRAME2_SYMBOL;
typedef struct
{
    U8 svid;
    U8 sub3Symbols[528];
}B1C_FRAME3_SYMBOL, *PB1C_FRAME3_SYMBOL;
typedef struct
{
    U8 svid;
    U8 result;
    U8 sub2Symbols[600];
}B1C_FRAME2_BIT, *PB1C_FRAME2_BIT;
typedef struct
{
    U8 svid;
    U8 result;
    U8 sub3Symbols[264];
}B1C_FRAME3_BIT, *PB1C_FRAME3_BIT;

extern int decodeLDPC(char *input, char *output, int idxCL);
#endif
//EOF
