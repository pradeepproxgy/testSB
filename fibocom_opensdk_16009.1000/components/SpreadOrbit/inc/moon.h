/*******************************************************************************
** File Name:       Moon.h
** Author:          Hongxin liu
** Date:            2015-11-15
** Copyright:       2008 Spreatrum, Incoporated. All Rights Reserved.
** Description:     This file is the header file of moon.cpp
********************************************************************************
********************************************************************************
**                              Edit History
** -----------------------------------------------------------------------------
** DATE                         NAME                      DESCRIPTION
** 2015/11/15            Hongxin liu                        Create.
*******************************************************************************/
#ifndef __XEOD_CG_CORE_MOON_H
#define __XEOD_CG_CORE_MOON_H

#include "definition.h"

int MoonGetAccel(double delta_mjd_tt, double *r_sat, double *acc, double *dAdR=NULL);
void MoonGetPos(double *r_moon);

#endif 
