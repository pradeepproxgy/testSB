/*******************************************************************************
** File Name:       Sun.h
** Author:          Hongxin liu
** Date:            2015-11-15
** Copyright:       2008 Spreatrum, Incoporated. All Rights Reserved.
** Description:     This file is the header file of sun.cpp
********************************************************************************
********************************************************************************
**                              Edit History
** -----------------------------------------------------------------------------
** DATE                         NAME                      DESCRIPTION
** 2015/11/15            Hongxin liu                        Create.
*******************************************************************************/
#ifndef __XEOD_CG_CORE_SUN_H
#define __XEOD_CG_CORE_SUN_H

#include "definition.h"

int SunGetAccel(double delta_mjd, double *r_sat, double *acc, double *dAdR=NULL);
void SunGetPos(double *r_sun);

#endif //__XEOD_CG_CORE_SUN_H