/*******************************************************************************
** File Name:       Earth.h
** Author:          Hongxin liu
** Date:            2015-11-15
** Copyright:       2008 Spreatrum, Incoporated. All Rights Reserved.
** Description:     The header file of earth.cpp
********************************************************************************
********************************************************************************
**                              Edit History
** -----------------------------------------------------------------------------
** DATE                         NAME                      DESCRIPTION
** 2015/11/15            Hongxin liu                        Create.
*******************************************************************************/
#ifndef __XEOD_CG_CORE_EARTH_H
#define __XEOD_CG_CORE_EARTH_H

void EarthGetAccel(double *sat_cts, double *acc, double *dAdR=NULL);

#endif //__XEOD_CG_CORE_EARTH_H