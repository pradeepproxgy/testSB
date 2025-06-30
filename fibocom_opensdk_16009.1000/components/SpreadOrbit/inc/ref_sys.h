/*******************************************************************************
** File Name:       Ref_sys.h
** Author:          Hongxin liu
** Date:            2015-11-15
** Copyright:       2008 Spreatrum, Incoporated. All Rights Reserved.
** Description:     This file is the header file of ref_sys.cpp
********************************************************************************
********************************************************************************
**                              Edit History
** -----------------------------------------------------------------------------
** DATE                         NAME                      DESCRIPTION
** 2015/11/15            Hongxin liu                        Create.
*******************************************************************************/
#ifndef __XEOD_CG_CORE_REF_SYS_H
#define __XEOD_CG_CORE_REF_SYS_H

#include "definition.h"

void RefSysInit();
int RefSysGetTMatrix(double delta_mjd_tt, double pt_matrix[3][3], double pt_matrix_dot[3][3]);
int RefSysCTS2CIS(double delta_mjd, double *pos_cts, double *vel_cts, double *pos_cis, double *vel_cis);
int RefSysCIS2CTS(double delta_mjd, double *pos_cis, double *vel_cis, double *pos_cts, double *vel_cts);

//20151119 add by hongxin.liu for 1Obs case
int RefSysCTS2CIS_1Obs(double delta_mjd, double *pos_cts, double *vel_cts, double *pos_cis, double *vel_cis);
int RefSysCIS2CTS_1Obs(double delta_mjd, double *pos_cis, double *vel_cis, double *pos_cts, double *vel_cts);
//20151119

#endif //__XEOD_CG_CORE_REF_SYS_H
