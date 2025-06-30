/*******************************************************************************
** File Name:       Integrator_rk.h
** Author:          Hongxin liu
** Date:            2015-11-15
** Copyright:       2008 Spreatrum, Incoporated. All Rights Reserved.
** Description:     This file is the header file of integrator_rk.cpp
********************************************************************************
********************************************************************************
**                              Edit History
** -----------------------------------------------------------------------------
** DATE                         NAME                      DESCRIPTION
** 2015/11/15            Hongxin liu                        Create.
*******************************************************************************/
#ifndef __XEOD_CG_CORE_INTEGRATOR_RK_H
#define __XEOD_CG_CORE_INTEGRATOR_RK_H

#include "definition.h"

void IntegratorRkInit(double *pstate, int dim, int order);
int IntegratorRkSetInitState(double dmjd0, double *state, int dim=6);
int IntegratorRkOneStep(double dmjdi, IntDirection dir, int tail, int last_tail);

#endif //__XEOD_CG_CORE_INTEGRATOR_RK_H
