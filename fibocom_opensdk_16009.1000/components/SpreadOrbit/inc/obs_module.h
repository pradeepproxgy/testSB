/*******************************************************************************
** File Name:       Obs_module.h
** Author:          Hongxin liu
** Date:            2015-11-15
** Copyright:       2008 Spreatrum, Incoporated. All Rights Reserved.
** Description:     This file is the header file of obs_module.cpp
********************************************************************************
********************************************************************************
**                              Edit History
** -----------------------------------------------------------------------------
** DATE                         NAME                      DESCRIPTION
** 2015/11/15            Hongxin liu                        Create.
*******************************************************************************/
#ifndef __XEOD_CG_CORE_OBS_MODULE_H
#define __XEOD_CG_CORE_OBS_MODULE_H

#include "definition.h"
#include "xeod_cg_module.h"
#include "ref_sys.h"

void ObsModuleInit(const XeodFileHeader *pobs_header);
void ObsModuleDeinit();

void ObsModuleGetMaxTimePeriod(int *mjd0_tt, double *dmjd0_tt, double *dmjd1_tt);
void ObsModuleGetObsTimePeriod(int prn, double *dmjd0, double *dmjd1);

int  ObsModuleGetSampleNum();  
int  ObsModuleGetValidNum();

#ifdef GLONASS
unsigned int GloObsModuleLoadObs(const GloObsBlock *pobs_block);
void GloObsModuleInit(const GlonassFileHeader *pobs_header);
void GloObsModuleDeinit();
GLO_EPH * GetLatestGloObs();
#endif

#ifdef BDS
unsigned int BdsObsModuleLoadObs(const BdsObsBlock *pobs_block);
void BdsObsModuleInit(const BdsFileHeader *pobs_header);
void BdsObsModuleDeinit();
BDS_EPH * GetLatestBdsObs();
#endif

unsigned int ObsModuleLoadObs(const ObsBlock *pobs_block);
int ObsModuleValidateOrbitPoint(unsigned int svid,int mjd,double delta_mjd,double x, double y, double z, double vx, double vy, double vz);

//20151222 add by hongxin.liu->for the output of orbit parameters from the requirement of firmware team 
EphKepler * GetLatestObs();
//20151222
EphKepler * GetObs(int *pCount);
#endif //__XEOD_CG_CORE_OBS_MODULE_H