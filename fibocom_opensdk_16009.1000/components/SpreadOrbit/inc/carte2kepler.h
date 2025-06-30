/*******************************************************************************
** File Name:       Common_inc.h
** Author:          Hongxin liu
** Date:            2015-11-15
** Copyright:       2017 Spreatrum, Incoporated. All Rights Reserved.
** Description:     This is header file of carte2kepler.cpp 
********************************************************************************
********************************************************************************
**                              Edit History
** -----------------------------------------------------------------------------
** DATE                         NAME                      DESCRIPTION
** 2015/11/15            Hongxin liu                        Create.
*******************************************************************************/
#ifndef ORBIT_KEPLER_H
#define ORBIT_KEPLER_H

#include "definition.h"
//#include "MathLib.h"

#define NDIM 15

#define FITHOURS 4

#define simTimeStep 900

#define simTimeCountLimit (FITHOURS*3600/simTimeStep + 1)
#define simTimeMidPoint (FITHOURS*1800/simTimeStep)
#define FitInterval (FITHOURS*3600)
#define halfFitInterval FitInterval/2

#define SIM_TIME_STEP				( simTimeStep )//seconds
#define SIM_TIME_COUNT_LIMIT		( simTimeCountLimit )
#define SIM_TIME_MID_POINT			( simTimeMidPoint )
#define FIT_INTERVAL				( FitInterval )
#define HALF_FIT_INTERVAL			( halfFitInterval )

#define NEQN simTimeCountLimit*3

//20160915 add GEO
#ifdef BDS
#define BDSFITHOURS 2

#define bdssimTimeCountLimit (BDSFITHOURS*3600/simTimeStep + 1)
#define bdssimTimeMidPoint (BDSFITHOURS*1800/simTimeStep)
#define BDSFitInterval (BDSFITHOURS*3600)
#define bdshalfFitInterval BDSFitInterval/2

#define BDS_SIM_TIME_COUNT_LIMIT		( bdssimTimeCountLimit )
#define BDS_SIM_TIME_MID_POINT			( bdssimTimeMidPoint )
#define BDS_FIT_INTERVAL				( BDSFitInterval )
#define BDS_HALF_FIT_INTERVAL			( bdshalfFitInterval )

#define BDS_NEQN bdssimTimeCountLimit*3
#endif
//20160915

#define	errMagLimit 12.0
#define ITER_LIMIT 20

unsigned int getKeplerParameters(int svID, int week, int weekOfAlmanac, double toe,
						 double posIGS[simTimeCountLimit][3], tSVD_Kepler_Ephemeris alm,
						 tSVD_Kepler_Ephemeris *eph, int numberOfEqns, int DeltFitInterval);
#ifdef BDS
//modify by hongxin in 20160915->to make clear GPS or BDS
unsigned int getBdsKeplerParameters(int svID, int week, int weekOfAlmanac, double toe,
						 double posIGS[bdssimTimeCountLimit][3], tSVD_Kepler_Ephemeris alm,
						 tSVD_Kepler_Ephemeris *eph, int numberOfEqns, int DeltFitInterval);
//20160915
#endif
void getPartialDerivatives(double tk, double argumentVector[NDIM+1], double tempValue[14],
						   double posPartials[4][NDIM+1]);

//modify by hongxin in 20160907->to make clear GPS or BDS
void getGeoPartialDerivatives(double tk, double argumentVector[NDIM+1], double tempValue[14],
						   double posPartials[4][NDIM+1]);
//20160907

void bksub(int n, double r[][NDIM], double x[NDIM], double y[NDIM]);

void mgs(int m, int n, double a[][NDIM], double r[][NDIM]);
extern int isNaN(double f);

#endif
