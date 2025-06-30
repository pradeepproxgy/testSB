/*******************************************************************************
** File Name:       Intehrator_am.h
** Author:          Hongxin liu
** Date:            2015-11-15
** Copyright:       2008 Spreatrum, Incoporated. All Rights Reserved.
** Description:     This file is the header file of integrator_am.cpp
********************************************************************************
********************************************************************************
**                              Edit History
** -----------------------------------------------------------------------------
** DATE                         NAME                      DESCRIPTION
** 2015/11/15            Hongxin liu                        Create.
*******************************************************************************/
#ifndef __XEOD_CG_CORE_INTEGRATOR_AM_H
#define __XEOD_CG_CORE_INTEGRATOR_AM_H

#include "definition.h"

int IntegratorAmOneStep(double dmjdi, IntDirection dir, int tail);
void IntegratorAmInit(double *pstate, int dim=60, int order=12);

#endif //__XEOD_CG_CORE_INTEGRATOR_AM_H