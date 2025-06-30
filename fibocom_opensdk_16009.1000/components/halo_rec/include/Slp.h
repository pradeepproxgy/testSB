/*******************************************************************************
 *  Slp.h
 * <Description of the file>
 *
 *  Copyright (C) 2019 Unisoc Communications Inc.
 *  History:
 *      20220908 yang.lin3
 *      Description
*******************************************************************************/
#ifndef _SLP_H_
#define _SLP_H_

//macro
#define SC_AuthGroup 3

#define SC_VARFLOW1 10
#define SC_VARFLOW2 10
#define SC_VARFLOW3 10

#define SC_FLAG_OK 0xA5
#define SC_FLAG_NOK	0x5A
#define SC_FLAG_XORMASK 0xFF

#define SC_AUTHRANDN 8

//type
typedef int (*pFuncAuth_t)(void);
//extern pFuncAuth_t g_Fun_Authenticate[SC_AuthGroup];
extern pFuncAuth_t g_bgwoxlkcjv[SC_AuthGroup];
#define g_Fun_Authenticate g_bgwoxlkcjv


typedef unsigned char (*pFuncCheck_t)(unsigned char *, unsigned char* , int );
extern pFuncCheck_t g_ghncxphd[SC_AuthGroup];
#define g_pFuncCheck g_ghncxphd

typedef enum
{
	FCHECK_TRUE = 0x5A,
	FCHECK_FALSE = 0xA5
}FCheckRet_t;

//global
extern unsigned char* g_pcuebgxd[SC_AuthGroup];
#define g_ScVarFlowGroup g_pcuebgxd

extern unsigned char* g_woipeho_xgx[SC_AuthGroup];
#define g_ScVarFlowGroup_mask g_woipeho_xgx

extern unsigned char* g_gheouc_wowex[SC_AuthGroup];
#define g_ScVarFlowGroup_check g_gheouc_wowex

extern unsigned int g_ghwwex_guhw[SC_AuthGroup];
#define g_SeVarFlowGroup_N g_ghwwex_guhw

extern unsigned int g_gqpixcglh[SC_AUTHRANDN+2];//flag1+randomdata+flag2
#define g_AuthGroup g_gqpixcglh

//function
#define Scramble_InitGroup(a) S_bbeoxls(a)
extern void S_bbeoxls(int* retRand);

#define Scramble_GetGroup(a) S_ghlsd(a)
extern void S_ghlsd(int* retRand);

#define Feature_Init() F_oweitq()
extern void F_oweitq();

#define Feature_GetLock() F_goerw()
extern FCheckRet_t F_goerw();

#define Feature_verify() F_woes()
extern void F_woes();

#define Feature_GenF() F_nree()
extern void F_nree();

#define Feature_GenL() F_dkjg()
extern void F_dkjg();

//interface
#define Feature_GetBand() F_wowgh()
extern uint8_t F_wowgh();

#define Feature_ABcheck() F_jfjghe()
extern FCheckRet_t F_jfjghe();

#define Feature_is2Waycheck() F_fueog()
extern FCheckRet_t F_fueog();

#define Feature_isMultiOperatorcheck() F_oreww()
extern FCheckRet_t F_oreww();

#define Feature_Voicecheck() F_gdeqp()
extern FCheckRet_t F_gdeqp();

#define Feature_Securecheck() F_qgoige()
extern FCheckRet_t F_qgoige();

#define Feature_Bit2check() F_ghwoer()
extern FCheckRet_t F_ghwoer();

#define Feature_Bit1check() F_xkbvbn()
extern FCheckRet_t F_xkbvbn();

#define Feature_Bit0check() F_zmbg()
extern FCheckRet_t F_zmbg();

#endif

