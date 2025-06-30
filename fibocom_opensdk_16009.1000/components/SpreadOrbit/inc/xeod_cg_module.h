/*******************************************************************************
** File Name:       Xeod_cg_module.h
** Author:          Hongxin liu
** Date:            2015-11-15
** Copyright:       2008 Spreatrum, Incoporated. All Rights Reserved.
** Description:     This file is the header file of API function.
********************************************************************************
********************************************************************************
**                              Edit History
** -----------------------------------------------------------------------------
** DATE                         NAME                      DESCRIPTION
** 2015/11/15            Hongxin liu                        Create.
** Year/Month/Data        ***                           modified for  ***
*******************************************************************************/
#ifndef __XEOD_CG_CORE_XEOD_CG_MODULE_H
#define __XEOD_CG_CORE_XEOD_CG_MODULE_H

#include "definition.h"
//20151222 add by hongxin.liu->for the output of orbit parameters from the requirement of firmware team 
//#include "WTYPES.h"
#include "xeod_cg_api.h"
//20151222

unsigned int XeodCgModuleInit(const XeodFileHeader *pobs_header, const char* pwork_path);
unsigned int XeodCgModuleDeinit();
unsigned int XeodCgModuleGeneratePredEph(EphBlock *peph_block, const ObsBlock *pobs_block, unsigned int step_size);
//20151222 add by hongxin.liu->for the output of orbit parameters from the requirement of firmware team 
unsigned int XeodCgModuleGetKeplerEEBlock(EphBlock *peph_block);//, BYTE *pEEBlock, short *pWNBlock
//20151222

#ifdef GLONASS
unsigned int XeodCgModuleInit(const GlonassFileHeader *pgloobs_header, const char* pwork_path);
unsigned int GlonassModuleGeneratePredEph(GloEphBlock *peph_block, const GloObsBlock *pobs_block, unsigned int step_size);
#endif

#ifdef BDS
unsigned int XeodCgModuleInit(const BdsFileHeader *pbdsobs_header, const char* pwork_path);
unsigned int BdsModuleGeneratePredEph(BdsEphBlock *peph_block, const BdsObsBlock *pobs_block, unsigned int step_size);
//20160829 begin for BDS kepler output
unsigned int XeodCgModuleGetBdsKeplerEEBlock(BdsEphBlock *peph_block);//, BYTE *pEEBlock, short *pWNBlock
//20160829 end   for BDS kepler output
#endif

#endif //__XEOD_CG_CORE_XEOD_CG_MODULE_H