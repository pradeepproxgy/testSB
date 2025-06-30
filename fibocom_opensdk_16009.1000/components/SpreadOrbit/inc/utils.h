/*******************************************************************************
** File Name:       Utils.h
** Author:          Hongxin liu
** Date:            2015-11-15
** Copyright:       2008 Spreatrum, Incoporated. All Rights Reserved.
** Description:     This file is the header file of utils.cpp
********************************************************************************
********************************************************************************
**                              Edit History
** -----------------------------------------------------------------------------
** DATE                         NAME                      DESCRIPTION
** 2015/11/15            Hongxin liu                        Create.
*******************************************************************************/
#ifndef __XEOD_CG_CORE_UTILS_H
#define __XEOD_CG_CORE_UTILS_H

#include "xeod_cg_api.h"
#include "common_inc.h"
//#include "definition.h"
#ifdef WIN32
extern void E(char *fmt, ...);
extern void D(char *fmt, ...);
#endif

double  dMul(double dNum1, double dNum2);
double  dDouble(double dNum);
double  dHalf(double dNum);
double  dSinD(double dDegree);
double  dSinR(double dRadian);
double  dCosR(double dRadian);

double dot(double *a, double *b);
double amag3(double *a);
void cross(double *a, double *b, double *c);
void cross_unit(double *a, double *b, double *c);
void normalise(double *vect, double *norm_vect);
double angvec(double *a, double *b, double *c);
double my_frac(double x);

int GetMJDate(int WN, double sow, double *delta_mjd);
void MJDate2WeeknoSow(int mjd, double delta_mjd, unsigned short * pweek_num, double * psow);
void NOAA_SatPosVel(EphKepler *ep, double sow, int weekno, EcefPosition *pos, EcefPosition *speed);

#ifdef GLONASS
void gloPZ90toWGS84( double *posWGS84 );
void ComputeGlonassSvPosEphemeris(GLO_EPH *eph, double ti, EcefPosition *svPos, EcefPosition *svSpeed, double *clkOffset, double *clkDrift);
#endif


//20151222 add by hongxin.liu->for the output of orbit parameters from the requirement of firmware team 
tSVD_Kepler_Ephemeris EphConvC2S(EphKepler *pEph);
// compute SV state of a given satellite at a given time
void computeSVState (tSVD_SVState *st, short  svId, 
                          tSVD_Kepler_Ephemeris *eph,  tGPSTime *gTime);

void setEphemerisConstants(tSVD_Kepler_EphemerisConsts *constptr,
                                  short svid,
                                  tSVD_Kepler_Ephemeris *eph,
                                  tGPSTime *gTime);
//20151222

#ifdef BDS
void validateGPSTOW_tGPSTime(tGPSTime *t);
void BDS2GPS_Time(tGPSTime *t);
void GPS2BDS_Time(tGPSTime *t);
void setEphemerisConstantsBDS(tSVD_Kepler_EphemerisConsts *constptr,
                                  short svid,
                                  BDS_EPH *eph,
                                  tGPSTime *gTime);
void ComputeBdsSvPosEphemeris(double *state, int num_states, BDS_EPH *eph, int prn, double t, int week, ephTempType* interType);
//20160829 begin for BDS kepler output
tSVD_Kepler_Ephemeris BdsEphConvC2S(BDS_EPH *pEph);
//20160829 end   for BDS kepler output
#endif

int Eph2ICDEph( tSVD_Kepler_Ephemeris eph, PackICDEph *eph_icd);
//int ICDEph2Eph( PackICDEph eph_icd, tSVD_Kepler_Ephemeris *eph);

//#ifdef __cplusplus
//extern "C"
//{
//#endif

///GPS and BDS use the same fuction
int ICDEph2Eph(PackICDEph *eph_icd, tSVD_Kepler_Ephemeris *eph);


int ICDObs2Obs(PackICDObs obs_icd, EphKepler *eph);
int Obs2ICDObs( EphKepler eph, PackICDObs *obs_icd);
int ICDObsHeader2ObsHeader(KeplFileHeader *icd_obsheader, XeodFileHeader *obsheader);
int ObsHeader2ICDObsHeader( XeodFileHeader *obsheader, KeplFileHeader *icd_obsheader);

#ifdef BDS
int BDSEph2ICDBDSEph( tSVD_Kepler_Ephemeris eph, PackICDEph *eph_icd);
int ICDBDSObs2BDSObs( PackICDObs obs_icd, BDS_EPH *eph);
int BDSObs2ICDBDSObs( BDS_EPH eph, PackICDObs *obs_icd);
int ICDBDSObsHeader2BDSObsHeader(KeplBdsFileHeader *icd_obsheader, BdsFileHeader *obsheader);
int BDSObsHeader2ICDBDSObsHeader( BdsFileHeader *obsheader, KeplBdsFileHeader *icd_obsheader);
#endif

#ifdef GLONASS
int ICDGLOEph2GLOEph(PackGloICDEph *eph_icd, EphCartesian *eph);
int GLOEph2ICDGLOEph(EphCartesian eph, PackGloICDEph *eph_icd);
int ICDGLOObs2GLOObs( PackGloICDObs obs_icd, GLO_EPH *eph);
int GLOObs2ICDGLOObs( GLO_EPH eph, PackGloICDObs *obs_icd);
int ICDGLOObsHeader2GLOObsHeader(ICDGlonassFileHeader *icd_obsheader, GlonassFileHeader *obsheader);
int GLOObsHeader2ICDGLOObsHeader( GlonassFileHeader *obsheader, ICDGlonassFileHeader *icd_obsheader);
#endif
//#ifdef __cplusplus
//}
//#endif

#endif //__XEOD_CG_CORE_UTILS_H
