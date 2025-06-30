/*******************************************************************************
** File Name:       Global_data.h
** Author:          Hongxin liu
** Date:            2015-11-15
** Copyright:       2008 Spreatrum, Incoporated. All Rights Reserved.
** Description:     This file is the header file of global_data.cpp.
********************************************************************************
********************************************************************************
**                              Edit History
** -----------------------------------------------------------------------------
** DATE                         NAME                      DESCRIPTION
** 2015/11/15            Hongxin liu                        Create.
*******************************************************************************/
#ifndef XEOD_CG_DATA_H
#define XEOD_CG_DATA_H

#include "xeod_cg_module.h"
#include "definition.h"

extern double kRkCoeffCx[9];
extern double kRkCoeffAxx[45];
extern double kRkCoeffBx[MAX_RK_ORDER];
extern double kAmCoeffB[MAX_AM_ORDER];
extern double kAmCoeffM[MAX_AM_ORDER];

extern double g_rk_deriv[MAX_RK_ORDER][MAX_INT_DIM];    
extern double g_prev_deriv[MAX_INT_ORDER][MAX_INT_DIM];

#ifdef BDS
extern SatMass  kSvNav[GPS_GLO_MEO_SV_NUM];                
extern double   kSvParamRadDefault[GPS_GLO_MEO_SV_NUM][9];
extern double   kSvParamRadCrYbias[GPS_GLO_MEO_SV_NUM][2];
extern double   kIERS_TM[5];
extern double   kEopCoeff[15];
extern double   kTableEGM[667][2];
extern const double kTableSlope[MAX_SV_PRN + GLO_MAX_SV_PRN + BDS_MAX_SV_PRN][2];

extern SatParam g_sv_param[GPS_GLO_MEO_SV_NUM];    
#else    
#ifdef GLONASS
extern SatMass  kSvNav[GPS_GLO_SV_NUM];                
extern double   kSvParamRadDefault[GPS_GLO_SV_NUM][9];
extern double   kSvParamRadCrYbias[GPS_GLO_SV_NUM][2];
extern double   kIERS_TM[5];
extern double   kEopCoeff[15];
extern double   kTableEGM[667][2];
extern const double kTableSlope[MAX_SV_PRN + GLO_MAX_SV_PRN][2];

extern SatParam g_sv_param[GPS_GLO_SV_NUM];            
#else 
extern SatMass  kSvNav[MAX_SV_PRN];                
extern double   kSvParamRadDefault[MAX_SV_PRN][9];
extern double   kSvParamRadCrYbias[MAX_SV_PRN][2];
extern double   kIERS_TM[5];
extern double   kEopCoeff[15];
extern double   kTableEGM[667][2];
extern const double kTableSlope[MAX_SV_PRN][2];

extern SatParam g_sv_param[MAX_SV_PRN];            
#endif
#endif

#ifdef BDS
extern double CR_matrix_bds[BDS_MAX_SV_PRN];
#endif

#ifdef BDS //20160606
extern double CurrentSatVel[3];
extern double PrevSatPos[3];
#endif

extern SatObs*  g_psat_obs_cis;
extern int      g_xeod_mjd0;
extern double   g_xeod_delta_mjd0;
extern double   g_xeod_delta_mjd1;

#endif
