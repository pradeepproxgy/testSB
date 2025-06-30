/*******************************************************************************
** File Name:       Integrator.h
** Author:          Hongxin liu
** Date:            2015-11-15
** Copyright:       2008 Spreatrum, Incoporated. All Rights Reserved.
** Description:     This file is the header file of integrator.cpp.
********************************************************************************
********************************************************************************
**                              Edit History
** -----------------------------------------------------------------------------
** DATE                         NAME                      DESCRIPTION
** 2015/11/15            Hongxin liu                        Create.
*******************************************************************************/
#ifndef __XEOD_CG_CORE_INTEGRATOR_H
#define __XEOD_CG_CORE_INTEGRATOR_H

#include "integrator_am.h"
#include "integrator_rk.h"
#include "definition.h"

void IntegratorInit(int dim);
int IntegratorSetInitState(double dmjd0_tt, double *pstate);
unsigned int IntegrateTo(double dmjd1_tt);
double *IntegratorGetStateBuffer();

#endif //__XEOD_CG_CORE_INTEGRATOR_H