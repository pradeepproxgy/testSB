/*******************************************************************************
** File Name:       Common_inc.h
** Author:          Hongxin liu
** Date:            2015-11-15
** Copyright:       2017 Spreatrum, Incoporated. All Rights Reserved.
** Description:     This is header file of integrator1obs.c 
********************************************************************************
********************************************************************************
**                              Edit History
** -----------------------------------------------------------------------------
** DATE                         NAME                      DESCRIPTION
** 2015/11/15            Hongxin liu                        Create.
*******************************************************************************/
#ifndef __XEOD_CG_CORE_INTEGRATOR1OBS_H
#define __XEOD_CG_CORE_INTEGRATOR1OBS_H
#pragma once
#include "definition.h"

//20151121 add by hongxin.liu for integrator1obs algorithm
#include "satellite_accel.h"
//20151121

void Integrator1ObsInit(int dim);
int Integrator1ObsSetInitState/*Integrator1ObsInit1*/(double delta_mjd_tt, double *pstate);
unsigned int Integrate1ObsTo(double dmjd1_tt);
double *Integrator1ObsGetStateBuffer();

#endif //__XEOD_CG_CORE_INTEGRATOR1OBS_H