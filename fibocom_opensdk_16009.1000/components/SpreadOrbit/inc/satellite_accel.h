/*******************************************************************************
** File Name:       Satellite_accel.h
** Author:          Hongxin liu
** Date:            2015-11-15
** Copyright:       2008 Spreatrum, Incoporated. All Rights Reserved.
** Description:     This file is the header file of satellite_accel.cpp
********************************************************************************
********************************************************************************
**                              Edit History
** -----------------------------------------------------------------------------
** DATE                         NAME                      DESCRIPTION
** 2015/11/15            Hongxin liu                        Create.
*******************************************************************************/
#ifndef __XEOD_CG_CORE_SATELLITE_ACCEL_H
#define __XEOD_CG_CORE_SATELLITE_ACCEL_H
#pragma once
#include "definition.h"
#include "sun.h"
#include "earth.h"
#include "moon.h"
#include "ref_sys.h"
#include "integrator.h"

//20151121 add by hongxin.liu for integrator1obs algorithm->无法解决两个头文件相互调用的问题，不得不将下面代码移出去
#include "integrator1obs.h"
//20151121

void Deriv_kk_k0(double *pstate, int tail, int dim, int is_rk=FALSE);

unsigned int    SatelliteAccelInit();
void    SatelliteAccelInitNewPrn(int prn=1, int partials_on=FALSE, int one_obs=0);
void    SatelliteAccelInitIntegrator();
void    SatelliteAccelUpdateRadParam();
void    SatelliteAccelGetDeriv(double delta_mjd, double *r_sat, double *deriv);
double  SatelliteAccelGetDMjd0();

#endif //__XEOD_CG_CORE_SATELLITE_ACCEL_H