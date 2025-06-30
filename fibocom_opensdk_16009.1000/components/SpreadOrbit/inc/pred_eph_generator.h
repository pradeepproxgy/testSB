/*******************************************************************************
** File Name:       Pred_eph_generator.h
** Author:          Hongxin liu
** Date:            2015-11-15
** Copyright:       2008 Spreatrum, Incoporated. All Rights Reserved.
** Description:     This file is the header file of pred_eph_generator.cpp
********************************************************************************
********************************************************************************
**                              Edit History
** -----------------------------------------------------------------------------
** DATE                         NAME                      DESCRIPTION
** 2015/11/15            Hongxin liu                        Create.
*******************************************************************************/
#ifndef __XEOD_CG_CORE_PRED_EPH_GENERATOR_H
#define __XEOD_CG_CORE_PRED_EPH_GENERATOR_H

#include "satellite_accel.h"
#include "integrator.h"
#include "sun.h"
#include "moon.h"
#include "earth.h"
#include "ref_sys.h"
#include "definition.h"
#include "obs_module.h"
//20151121 add by hongxin.liu
#include "integrator1obs.h"
//20151121

void PredEphGeneratorInit();
void PredEphGeneratorDeinit();
void PredEphGeneratorInitSvParam(int prn,int obs_num);
void PredEphGeneratorInitNewPrn(int prn=1, double pred_days=6, int partials_on=TRUE, int one_obs=0);
unsigned int PredEphGeneratorEstimate();
unsigned int PredEphGeneratorPredict(EphBlock *peph_block);

#ifdef GLONASS
unsigned int GloPredEphGeneratorPredict(GloEphBlock *peph_block);
#endif

#ifdef BDS
unsigned int BdsPredEphGeneratorPredict(BdsEphBlock *peph_block);
#endif

#endif //__XEOD_CG_CORE_PRED_EPH_GENERATOR_H
