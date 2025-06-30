/*******************************************************************************
** File Name:       SoonerFix.h
** Author:          Zhicheng Li	
** Date:            2015-02-12
** Copyright:       2014 Spreatrum, Incoporated. All Rights Reserved.
** Description:     This file define the  function, parameters used for  
**                  SoonerFix
********************************************************************************
********************************************************************************
**                              Edit History
** -----------------------------------------------------------------------------
** DATE                         NAME                      DESCRIPTION
** Year/Month/Data        ***                               Create.
*******************************************************************************/
#ifndef __SPREAD_ORBIT_H
#define __SPREAD_ORBIT_H

/*******************************************************************************
INCLUDED FILES
*******************************************************************************/
#ifdef WIN32
#include <Windows.h>
//#include "basictype.h"
#include "./xeod_cg_api.h"
#include "gnss_config.h"
#include "os_api.h"

extern ObsBlock g_tObsBlock[MAX_SV_PRN];
extern EphBlock g_tEphBlock[MAX_SV_PRN];

#ifdef GLONASS
extern GloObsBlock g_tGloObsBlock[GLO_MAX_SV_PRN];
extern GloEphBlock g_tGloEphBlock[GLO_MAX_SV_PRN];
#endif

#ifdef BDS
extern BdsObsBlock g_tBdsObsBlock[BDS_MAX_SV_PRN];
extern BdsEphBlock g_tBdsEphBlock[BDS_MAX_SV_PRN];
#endif

extern HANDLE g_hObsUpdateEvent;
extern CRITICAL_SECTION g_SFCriticalSection;
#else
#include "xeod_cg_api.h"
#include "SpreadOrbit_config.h"
#include "osi_log.h"
#define SPREAD_ORBIT_EXTEND_KEPLER 1
#endif
/*******************************************************************************
CONSTANTS AND MACROS DEFINE
*******************************************************************************/
#ifndef MAX_PATH_SIZE
#define MAX_PATH_SIZE   256
#endif

#define OBS_FILE_NAME   ("raw.obs")
#define EPH_FILE_NAME   ("ext.eph")

#ifdef GLONASS
#define GLO_OBS_FILE_NAME   ("glo.obs")
#define GLO_EPH_FILE_NAME   ("glo.eph")
#endif

#ifdef BDS
#define BDS_OBS_FILE_NAME   ("bds.obs")
#define BDS_EPH_FILE_NAME   ("bds.eph")
#define BDS_KEPL_EPH_FILE_NAME   ("bds_kep.eph")
#endif

//20160106 add by hongxin.liu
#define KEPL_EPH_FILE_NAME   ("kep.eph")
//20160106
#define LTE_RELEASE_VER		"GNSS_SPO_W21.25.1"

typedef void (*ext_orbit_callback)(int type, char* buff);
typedef struct {
	/** set to sizeof(GnssLteCallbacks) */
	size_t      size;
	ext_orbit_callback ext_orbit_cb;
} GnssLteCallbacks;

/*******************************************************************************
EXTERN DECLARATION of FUNCTIONS 
*******************************************************************************/
#ifdef WIN32
DWORD WINAPI SoonerFix_PredictThread(LPVOID lpParam);
#else
extern void SoonerFix_PredictThread(void *arg);
#endif
//common
void SF_setExitFlag(int flag); 
int SF_isThreadExited();
void SF_setNewSvMask(int svid);
void SF_clearNewSvMask(int svid);
extern int SF_getSvPredictPosVel( int svid,  int week,  int tow, char *output);
extern void SF_updateObsTask(char *buf,int lenth);
//common
void SF_setSaveEphMask(int svid);
void SF_clearSaveEphMask(int svid);

void SF_setNewGloSvMask(int svid);
void SF_clearNewGloSvMask(int svid);
//common
void SF_setSaveGloEphMask(int svid);
void SF_clearSaveGloEphMask(int svid);
extern int SF_getGloSvPredictPosVel(int svid, int leapyear, int daynumber, int tod, char * output);
extern void SF_updateGloObsTask(char *buf,int lenth);

void SF_setNewBdsSvMask(int svid);
void SF_clearNewBdsSvMask(int svid);
//common
void SF_setSaveBdsEphMask(int svid);
void SF_clearSaveBdsEphMask(int svid);
extern int SF_getBdsSvPredictPosVel( int svid,  int week,  int tow, char *output);
extern void SF_updateBdsObsTask(char *buf,int lenth);
#ifndef WIN32
int SF_init(GnssLteCallbacks* callbacks);
#else
int SF_init(void);
#endif

	///GPS and BDS use the same fuction
	int ICDEph2Eph(PackICDEph *eph_icd, tSVD_Kepler_Ephemeris *eph);
	int ICDObs2Obs(PackICDObs obs_icd, EphKepler *eph);
	int Obs2ICDObs( EphKepler eph, PackICDObs *obs_icd);
	int ICDObsHeader2ObsHeader(KeplFileHeader *icd_obsheader, XeodFileHeader *obsheader);
	int ObsHeader2ICDObsHeader( XeodFileHeader *obsheader, KeplFileHeader *icd_obsheader);
	#ifdef BDS
	int ICDBDSObs2BDSObs( PackICDObs obs_icd, BDS_EPH *eph);
	int BDSObs2ICDBDSObs( BDS_EPH eph, PackICDObs *obs_icd);
	int ICDBDSObsHeader2BDSObsHeader(KeplBdsFileHeader *icd_obsheader, BdsFileHeader *obsheader);
	int BDSObsHeader2ICDBDSObsHeader( BdsFileHeader *obsheader, KeplBdsFileHeader *icd_obsheader);
	#endif
	
	#ifdef GLONASS
	int ICDGLOEph2GLOEph(PackGloICDEph *eph_icd, EphCartesian *eph);
	int ICDGLOObs2GLOObs( PackGloICDObs obs_icd, GLO_EPH *eph);
	int GLOObs2ICDGLOObs( GLO_EPH eph, PackGloICDObs *obs_icd);
	int ICDGLOObsHeader2GLOObsHeader(ICDGlonassFileHeader *icd_obsheader, GlonassFileHeader *obsheader);
	int GLOObsHeader2ICDGLOObsHeader( GlonassFileHeader *obsheader, ICDGlonassFileHeader *icd_obsheader);
	#endif
#endif //__SPREAD_ORBIT_H
