/*******************************************************************************
** File Name:       Xeod_cg_api.h
** Author:          Hongxin liu
** Date:            2015-11-15
** Copyright:       2008 Spreatrum, Incoporated. All Rights Reserved.
** Description:     This file is the header file of API interface.
********************************************************************************
********************************************************************************
**                              Edit History
** -----------------------------------------------------------------------------
** DATE                         NAME                      DESCRIPTION
** 2015/11/15            Hongxin liu                        Create.
*******************************************************************************/
#ifndef __XEOD_CG_API_H
#define __XEOD_CG_API_H

#include "gnss_config.h"

#if defined(WIN32) || defined(_WIN64)
#if defined(DLL_EXPORTS)
#define DLL_API_INT _declspec(dllexport) int __stdcall
#elif defined(DLL_IMPORTS)
#define DLL_API_INT _declspec(dllimport) int __stdcall
#else
#define DLL_API_INT int
#endif
#else
#define DLL_API_INT int
#endif

#ifdef WIN32
//20151222 add by hongxin.liu->for the output of orbit parameters from the requirement of firmware team 
#include "WTYPES.h"
#include "common_inc.h"
//20151222
#endif

#define XEOD_FILE_VERSION       1
/////////////////////////////////////////////////////////////////////////////////////////
// XEOD_CG related constants
/*
#define EGM_ORDER               8
#define RK_ORDER                10
#define AM_ORDER                12
#define MAX_RK_ORDER            10                    //Maximum RK Integrator Order
#define MAX_AM_ORDER            12                    //Maximum AM Integrator Order
#define MAX_INT_ORDER           12                    //Maximum Integration Order
#define MAX_INT_DIM             100                   //Maximum Integration Dimension, (v,a,partials) or (r,v,DesignMatrix)
*/
#define INTEG_STEPSIZE          100
#define PRED_CART_STEPSIZE      900
//20151123 add by hongxin.liu for Integrator_1Obs case
#define INTEG_STEPSIZE_1OBS     900
//20151123


#ifdef SPREAD_ORBIT_PRED_DAYS1
#define PRED_DAYS               1
#else
#define PRED_DAYS               2
#endif

#define MAX_SV_PRN              32
#define OBS_DAYS                3
//#define PRED_DAYS               3
#define MAX_OBS_PER_DAY         12
#define SAMPLE_INTERVAL         900
#define MAX_OBS_NUM             (MAX_OBS_PER_DAY * OBS_DAYS)
#define MAX_EPOCH_NUM           ((int)((OBS_DAYS+PRED_DAYS)/(INTEG_STEPSIZE*SEC2DAY))+1)

#define SAMPLE_PER_OBS          5
#define MIN_OBS_SAMPLE_OFFSET   (4*3600)
/////////////////////////////////////////////////////////////////////////////////////////

#ifdef GLONASS
#define GLO_MAX_SV_PRN              24
#define GLO_MAX_OBS_PER_DAY         48

#define GLO_MAX_OBS_NUM             (GLO_MAX_OBS_PER_DAY * OBS_DAYS)

#define GLO_SAMPLE_PER_OBS          1
#define GLO_MIN_OBS_SAMPLE_OFFSET   3600
//20170815 begin
#define GPS_GLO_SV_NUM              (MAX_SV_PRN + GLO_MAX_SV_PRN)   
//20170815 end
#else //if // GLONASS Disable
#define GLO_MAX_SV_PRN              0
#define GLO_MAX_OBS_PER_DAY         0

#define GLO_MAX_OBS_NUM             (GLO_MAX_OBS_PER_DAY * OBS_DAYS)

#define GLO_SAMPLE_PER_OBS          0
#define GLO_MIN_OBS_SAMPLE_OFFSET   0
#define GPS_GLO_SV_NUM              (MAX_SV_PRN + GLO_MAX_SV_PRN)//32+0 = 32
#endif

#ifdef BDS
#define BDS_MAX_SV_PRN              37
#define BDS_MAX_OBS_PER_DAY         24

#define BDS_MAX_OBS_NUM             (BDS_MAX_OBS_PER_DAY * OBS_DAYS)

#define BDS_SAMPLE_PER_OBS          3
#define BDS_MIN_OBS_SAMPLE_OFFSET   7200

//20160916 add by hongxin.liu
#define PRED_BDS_KEPL_STEPSIZE      7200   //2 hour
//20160916 
//20170815 begin
#define GEO_SV_PRN                  5
#define IGSO_SV_PRN                 5
#define MEO_SV_PRN                  27
#define GPS_GLO_GEO_SV_NUM          (MAX_SV_PRN + GLO_MAX_SV_PRN + GEO_SV_PRN) //32+24+5 = 61
#define GPS_GLO_IGSO_SV_NUM         (MAX_SV_PRN + GLO_MAX_SV_PRN + GEO_SV_PRN + IGSO_SV_PRN)//32+24+5+5 = 66
#define GPS_GLO_MEO_SV_NUM          (MAX_SV_PRN + GLO_MAX_SV_PRN + GEO_SV_PRN + IGSO_SV_PRN + MEO_SV_PRN) //32+24+5+5+4 = 70
//#define GPS_GLO_BDS_SV_NUM          (MAX_SV_PRN + GLO_MAX_SV_PRN + BDS_MAX_SV_PRN)
//20170815 end
#else //if // BDS disable
#define BDS_MAX_SV_PRN              0
#define BDS_MAX_OBS_PER_DAY         0

#define BDS_MAX_OBS_NUM             (BDS_MAX_OBS_PER_DAY * OBS_DAYS)

#define BDS_SAMPLE_PER_OBS          0
#define BDS_MIN_OBS_SAMPLE_OFFSET   0

#define PRED_BDS_KEPL_STEPSIZE      0   
#define GEO_SV_PRN                  0
#define IGSO_SV_PRN                 0
#define MEO_SV_PRN                  0
#define GPS_GLO_GEO_SV_NUM          (MAX_SV_PRN + GLO_MAX_SV_PRN + GEO_SV_PRN) //32+24?+0 = ?
#define GPS_GLO_IGSO_SV_NUM         (MAX_SV_PRN + GLO_MAX_SV_PRN + GEO_SV_PRN + IGSO_SV_PRN)//32+24?+0+0 = ?
#define GPS_GLO_MEO_SV_NUM          (MAX_SV_PRN + GLO_MAX_SV_PRN + GEO_SV_PRN + IGSO_SV_PRN + MEO_SV_PRN) //32+24?+0+0+0 = ?
#endif

//2016.1.5 add by hongxin.liu for the union of cartesian and kepler
#define ORBIT_CARTESIAN			0		// Cartesian form for predicted Eph 
#define ORBIT_KEPLERIAN			1		// Keplerian form for predicted Eph	
#define PRED_KEPL_STEPSIZE      14400   //4 hour
//2016.1.5

/////////////////////////////////////////////////////////////////////////////////////////
// Error & warning message constants
#define XEOD_CG_SUCCESS         0
#define XEOD_CG_NOT_INITED      1
#define XEOD_CG_ERR_MEM         2
#define XEOD_CG_ERR_PARAM       3
#define XEOD_CG_ERR_OBS         4
#define XEOD_CG_ERR_EST         5
//20151222 add by hongxin.liu->for the output of orbit parameters from the requirement of firmware team 
//20151222
//20151222 add by hongxin.liu->for the output of orbit parameters from the requirement of firmware team 
#define XEOD_CG_ERR_KEPLER_FIT  6
#define XEOD_CG_ERR_KEPLER_FIT_INVALID_PARAM  7
#define XEOD_CG_ERR_NOVALID_DATA  8
#define XEOD_CG_ERR_KEPLER_ICD_PACKED  9
//20151222
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
// General constants
#define PI2                     6.283185307179586476925286766559
#define RAD2ARCS                2.062648062470964e+005            //Radian to Arcseconds
#define ARCS2RAD                4.848136811095355e-006            //Arcsecond to Radian
#define DEG2RAD                 0.01745329251994                  //Degree to Radian
#define RAD2DEG                 57.29577951308232                 //Rad to Degree
#define SEC2DAY                 1.1574074074074073e-005           //Second to Day
#define DAY2SEC                 86400.0                           //Day to Second
#define WEEK2SEC                604800                            //Week to Second
//20151120 add by hongxin.liu for 1Obs ref_sys algorithm
#define REFJD                   240000.5
#define GM_EARTH_SQRT           1.996498184321740e+7              //square root of GM_EARTH
#define WGSRelCorrConst         -4.442807633E-10                  //F - relativisdtic correction term constant
//20151120

#define MJD_J2000               51544.5
#define MJD_GPST0               44244

#define GM_EARTH                0.3986004415E+15    // Gravity Constant * Earth mass, unit: m^3/s^2
#define GM_MOON                 4.9027989E+12       // Gravity Constant * Lunar mass, unit: m^3/s^2 4902.799
#define GM_SUN                  1.3271244E+20       // Gravity Constant * Solar mass, unit: m^3/s^2                        
#define RADIUS_SUN              696000000.0         //The potent radius of sun , unit:m
#define RADIUS_MOON             1738000.0           //The potent radius of moon, unit:m
#define AUNIT                   1.49597870691E+11   // Astronomy Unit: meters
#define AUNIT2                  2.2379522915281E+022// Astronomy Unit^2: meter^2
#define AE                      6378136.3           // EQUATORIAL RADIUS OF EARTH  in m 6378137.0
#define CLIGHT                  299792458.0         //speed of light, unit:m/s
#define WGS84_OMEGDOTE          7.2921151467E-5
#define WGS_SQRT_GM 19964981.8432173887
#define WGS_F_GTR -4.442807633e-10
#define DAY2SEC 86400.0
/////////////////////////////////////////////////////////////////////////////////////////

#ifdef GLONASS
#define SECONDS_IN_DAY  86400
#define GLO_EPH_MAX_POS 27000
#define GLO_EPH_MIN_POS 24000

 /* NOTE! the following paramters may differ from WGS84, but they MUST be used */
#define MU     398600.4418     /* km/s2 - Gravitational constant */
#define T_NOM  43200.0         /* half a day */
#define GLO_AE     6378.136        /* Semi-major axix of earth ellipsoid km */
#define WE     0.7292115e-4    /* Earth rotation rate rads/s */
#define J02    1082.62575e-6   /* 2nd Zonal Harmonic of geopotential */

/* Standard Helmert 7-parameter datum transformation parameters for
   converting the PZ-90 coordinates of the glonass satellites into WGS84 frame
   that GPS uses. (and vice-versa) There are many sources of the offsets/rotations, 
   but the most current seem to be based "Mitrikas 99." */
/* Axis Offsets (m) */
#define GLO_GPS_DELTA_X (-0.470)
#define GLO_GPS_DELTA_Y (-0.510)
#define GLO_GPS_DELTA_Z (-1.560)
/* Axis rotations (radians) */
#define GLO_GPS_ROT_X (-0.076e-6)
#define GLO_GPS_ROT_Y (-0.017e-6)
#define GLO_GPS_ROT_Z ( 1.728e-6)
/* Datum scale (unitless) */
#define GLO_GPS_SCALE (1.000000022)

#endif
#if 1//BDS
#define BDS_GEO_FIRSTPRN             (150)
#define BDS_GEO_LASTPRN              (154)
   
#define BDS_IGSO_FIRSTPRN            (155)
#define BDS_IGSO_LASTPRN             (159)
   
#define BDS_MEO_FIRSTPRN             (38)
#define BDS_MEO_LASTPRN              (64)
#define BDS_MEO_CURR_MAXPRN		     (64) // At this point, there are only 4 MEO satellites. So a total of 14 BDS satellites.

#define BDS_GEO_PRN_SIZE             (BDS_GEO_LASTPRN-BDS_GEO_FIRSTPRN+1)//5
#define BDS_IGSO_PRN_SIZE            (BDS_IGSO_LASTPRN-BDS_IGSO_FIRSTPRN+1)//5
#define BDS_MEO_PRN_SIZE             (BDS_MEO_LASTPRN-BDS_MEO_FIRSTPRN+1) /* 27 SVs*/
#define BDS_MEO_CURR_SIZE			 (BDS_MEO_CURR_MAXPRN - BDS_MEO_FIRSTPRN+1) /*27 SVs*/
#define BDS_NONGEO_PRN_SIZE          (BDS_IGSO_PRN_SIZE + BDS_MEO_PRN_SIZE)//32

#define BDS_PRN_SIZE                 (BDS_GEO_PRN_SIZE+BDS_IGSO_PRN_SIZE+BDS_MEO_PRN_SIZE)//3//37
#define BDS_CURR_PRN_SIZE			 (BDS_GEO_PRN_SIZE+BDS_IGSO_PRN_SIZE+BDS_MEO_CURR_SIZE)//37
#define mBdsGeoPrnRangeOk(prn)       ((prn >= BDS_GEO_FIRSTPRN) && (prn <= BDS_GEO_LASTPRN))
#define mBdsIgsoPrnRangeOk(prn)      ((prn >= BDS_IGSO_FIRSTPRN) && (prn <= BDS_IGSO_LASTPRN))
#define mBdsMeoPrnRangeOk(prn)       ((prn >= BDS_MEO_FIRSTPRN) && (prn <= BDS_MEO_LASTPRN))
#define mBdsGeoIgsoPrnRangeOk(prn)   ((mBdsGeoPrnRangeOk(prn)) || (mBdsIgsoPrnRangeOk(prn)))
#define mBdsNongeoPrnRangeOk(prn)    ((mBdsMeoPrnRangeOk(prn)) || (mBdsIgsoPrnRangeOk(prn)))
#define mBdsPrnRangeOk(prn)          ((mBdsGeoPrnRangeOk(prn)) || (mBdsIgsoPrnRangeOk(prn)) ||(mBdsMeoPrnRangeOk(prn)))
#define mBdsGeoIdxGet(prn)           ((prn) - BDS_GEO_FIRSTPRN)
#define mBdsMeoIdxGet(prn)           ((prn) - BDS_MEO_FIRSTPRN)
#define mBdsIdxGet(prn)              ((mBdsGeoIgsoPrnRangeOk(prn))?(mBdsGeoIdxGet(prn)):((mBdsMeoIdxGet(prn)) + BDS_GEO_PRN_SIZE + BDS_IGSO_PRN_SIZE))
#define BDS_EPH_VALIDITY_IN_SECONDS	  3600.0
#define BDS_GEO_MASS				  4600.0
#define BDS_IGSO_MASS				  4200.0
#define BDS_MEO_MASS				  2200.0

#define BDS_MIN_MEO_A      (20000000) /* unit: meter. By statistics, MEO is about 27905km */
#define BDS_MAX_MEO_A      (30000000) /* unit: meter. By statistics, MEO is about 27905km */
#define BDS_MIN_GEO_IGSO_A (40000000) /* unit: meter. By statistics, GEO and IGSO are about 42163km*/
#define BDS_MAX_GEO_IGSO_A (45000000) /* unit: meter. By statistics, GEO and IGSO are about 42163km*/\

/***************************************************************************
*
*   Common physical constants
*
***************************************************************************/

#define SECONDS_PER_WEEK      604800   /* # of seconds in a week   */
#define SECONDS_IN_WEEK       604800   /* # of seconds in a week   */
#define SECONDS_IN_DAY        86400    /* # of seconds in a day    */
#define SECONDS_IN_HOUR       3600     /* # of seconds in an hour  */
#define SECONDS_IN_MINUTE       60          /* # of seconds in a minute */

#define MAX_GPS_WEEK          (5218)   /* Weeks from GPS week 0 (2080) */

#define NLC     (299792458.0)               /* speed of light   (m/s)                       */
#define NLCC    (NLC * NLC)                 /* speed of light squared                   */
#define NLRC    (1.0/NLC)                   /* Reciprocal speed of light */
#define NLGM    (3.9860015e14)              /* surface gravitational constant       */
#define NLERTE  (7.2921151467e-5)           /* earth's rotation rate                    */
#define NLFLAT  (1.0/298.2572235630)        /* flattening coef of ref ellipsoid.    */
#define NLMAJA  (6378137.0)                 /* len of semi major axis of ref ellips*/
#define NLMINB  (NLMAJA * (1.0-NLFLAT)) /* len of semi minor axis of ref ellips*/
#define NLEQG   (9.78033267714)         /* absolute equatorial gravity          */
#define NLESQR  (NLFLAT * (2.0-NLFLAT)) /* 1st eccentricity squared             */
#define NLOMES  (1.0 - NLESQR)              /* 1 minus eccentricity squared         */
#define NLEFOR  (NLESQR * NLESQR)           /* Sqr of the 1st eccentricity squared  */
#define NLASQR  (NLMAJA * NLMAJA)           /* semi major axis squared = nlmaja**   */
#define NLJ2    (1.08263e-3)                /* j2 gravity model coefficient         */
#define NLMU    (3.986005e14)               /* universal gravity parameter          */
#define NLSQMU  (1.9964981843217389e+7) /* root universal gravity parameter     */

#define NLBBAA  ((NLMINB * NLMINB) / (NLMAJA * NLMAJA)) /* bxb / axa = nlminb**2 / nlmaja**2    */

#define LIGHT   NLC

#define NLRF        (4.442807633e-10)   /* relativistic clock correl coef*/
#define BDS_NLERTE  (7.292115e-5)           /** earth's rotation rate, as defined in CGCS2000 */
#define BDS_NLSQMU  (1.996498038566530e+7)  /** sqrt of gravitation coeff for CGCS 2000 */

#define BDS_WN_TO_GPS_WN           (1356.0)         /* GPS week number = BDS week number + 1356*/
#define BDS_GPS_OFFSET			   (14.0)
#endif
#define CGS2000_SQRT_GM		19964980.3856652962	// square root of GM
#define CGS2000_OMEGDOTE	7.292115e-5			// earth rotate rate
/////////////////////////////////////////////////////////////////////////////////////////
#define S2P55  3.602879701896400e+16
#define S2P54  1.801439850948200e+16
#define S2P53  9.007199254740990e+15
#define S2P52  4.503599627370500e+15
#define S2P51  2.251799813685250e+15
#define S2P50  1.125899906842620e+15
#define S2P49  5.629499534213120e+14
#define S2P48  2.814749767106560e+14
#define S2P47  1.407374883553280e+14
#define S2P46  7.036874417766400e+13
#define S2445  3.518437208883200e+13
#define S2P44  1.759218604441600e+13
#define S2P43  8.796093022208000e+12
#define S2P42  4.398046511104000e+12
#define S2P41  2.199023255552000e+12
#define S2P40  1.099511627776000e+12
#define S2P39  5.497558138880000e+11
#define S2P38  2.748779069440000e+11
#define S2P37  1.374389534720000e+11
#define S2P36  6.871947673600000e+10
#define S2P35  3.435973836800000e+10
#define S2P34  1.717986918400000e+10
#define S2P33  8.589934592000000e+09
#define S2P32  4.294967296000000e+09
#define S2P31  2.147483648000000e+09
#define S2P30  1.073741824000000e+09
#define S2P29  5.368709120000000e+08
#define S2P28  2.684354560000000e+08
#define S2P27  1.342177280000000e+08
#define S2P26  6.710886400000000e+07
#define S2P25  3.355443200000000e+07
#define S2P24  1.677721600000000e+07
#define S2P23  8.388608000000000e+06
#define S2P22  4.194304000000000e+06
#define S2P21  2.097152000000000e+06
#define S2P20  1.048576000000000e+06
#define S2P19  5.242880000000000e+05
#define S2P18  2.621440000000000e+05
#define S2P17  1.310720000000000e+05
#define S2P16  6.553600000000000e+04
#define S2P15  3.276800000000000e+04
#define S2P14  1.638400000000000e+04
#define S2P13  8.192000000000000e+03
#define S2P12  4.096000000000000e+03
#define S2P11  2.048000000000000e+03
#define S2P10  1.024000000000000e+03
#define S2P9   5.120000000000000e+02
#define S2P8   2.560000000000000e+02
#define S2P7   1.280000000000000e+02
#define S2P6   6.400000000000000e+01
#define S2P5   3.200000000000000e+01
#define S2P4   1.600000000000000e+01
#define S2P3   8.000000000000000e+00
#define S2P2   4.000000000000000e+00
#define S2P1   2.000000000000000e+00
#define S2P0   1.000000000000000e+00
#define S2PN1  5.000000000000000e-01
#define S2PN2  2.500000000000000e-01
#define S2PN3  1.250000000000000e-01
#define S2PN4  6.250000000000000e-02
#define S2PN5  3.125000000000000e-02
#define S2PN6  1.562500000000000e-02
#define S2PN7  7.812500000000000e-03
#define S2PN8  3.906250000000000e-03
#define S2PN9  1.953125000000000e-03
#define S2PN10 9.765625000000000e-04
#define S2PN11 4.882812500000000e-04
#define S2PN12 2.441406250000000e-04
#define S2PN13 1.220703125000000e-04
#define S2PN14 6.103515625000000e-05
#define S2PN15 3.051757812500000e-05
#define S2PN16 1.525878906250000e-05
#define S2PN17 7.629394531250000e-06
#define S2PN18 3.814697265625000e-06
#define S2PN19 1.907348632812500e-06
#define S2PN20 9.536743164062500e-07
#define S2PN21 4.768371582031250e-07
#define S2PN22 2.384185791015620e-07
#define S2PN23 1.192092895507810e-07
#define S2PN24 5.960464477539060e-08
#define S2PN25 2.980232238769530e-08
#define S2PN26 1.490116119384770e-08
#define S2PN27 7.450580596923830e-09
#define S2PN28 3.725290298461910e-09
#define S2PN29 1.862645149230960e-09
#define S2PN30 9.313225746154790e-10
#define S2PN31 4.656612873077390e-10
#define S2PN32 2.328306436538700e-10
#define S2PN33 1.164153218269350e-10
#define S2PN34 5.820766091346740e-11
#define S2PN35 2.910383045673370e-11
#define S2PN36 1.455191522836690e-11
#define S2PN37 7.275957614183430e-12
#define S2PN38 3.637978807091710e-12
#define S2PN39 1.818989403545860e-12
#define S2PN40 9.094947017729280e-13
#define S2PN41 4.547473508864640e-13
#define S2PN42 2.273736754432320e-13
#define S2PN43 1.136868377216160e-13
#define S2PN44 5.684341886080800e-14
#define S2PN45 2.842170943040400e-14
#define S2PN46 1.421085471520200e-14
#define S2PN47 7.105427357601000e-15
#define S2PN48 3.552713678800500e-15
#define S2PN49 1.776356839400250e-15
#define S2PN50 8.881784197001250e-16
#define S2PN51 4.440892098500630e-16
#define S2PN52 2.220446049250310e-16
#define S2PN53 1.110223024625160e-16
#define S2PN54 5.551115123125780e-17
#define S2PN55 2.775557561562890e-17

#define ICD_PI                (3.1415926535898)    /* as defined in ICD-GPS-200 */
/*
*/
typedef int					IN32;
typedef unsigned  int		UIN32;
typedef short int			IN16;
typedef unsigned short int	UIN16;
typedef double				DOUBLE64;
typedef float				FLOAT32;	

typedef signed char			IN8;
typedef unsigned char		UIN8;
typedef signed long			SIN32;
typedef signed short		SIN16;
typedef signed char			SIN8;
typedef struct
{
    UIN8	EphFlag;
    UIN8	SVPRN;
    UIN8	URA_IND;
    UIN8	IODE;

	UIN8	C_RS_0;
	UIN8	C_RS_1;
	UIN8	C_RS_2;
	UIN8	C_RC_0;
	
	UIN8	C_RC_1;
	UIN8	C_RC_2;
	UIN8	C_US_0;
	UIN8	C_US_1;
	
	UIN8	C_US_2;
	UIN8	C_UC_0;
	UIN8	C_UC_1;
	UIN8	C_UC_2;
	
	UIN8	C_IS_0;
	UIN8	C_IS_1;
	UIN8	C_IS_2;
 	UIN8	C_IC_0;
	
	UIN8	C_IC_1;
	UIN8	C_IC_2;
    IN8	    T_GD;
    UIN8	AF2;

///    int	C_RS;
///    int	C_UC;
///    int	C_US;
///    int	C_IC;
///    int	C_IS;
///    int	C_RC;


    int  	M0;
    UIN32	Eccentricity;

    UIN32	A_SQRT;
    int 	Omega0;
 
    int 	AngleInclination;
    int 	Omega;

    int 	OmegaDOT;
    int 	AF0;

    short	IDOT;
	UIN16	TOE;
    UIN16	TOC;   
    short	DeltaN;
///20181203    int		DeltaN;

    short	AF1;
	UIN16   WN;
} PackICDEph; //total 26 param

typedef struct
{
    UIN8	EphFlag;
    UIN8	SVPRN;
    UIN8	URA_IND;
    UIN8	IODE;

	UIN8	C_RS_0;
	UIN8	C_RS_1;
	UIN8	C_RS_2;
	UIN8	C_RC_0;
	
	UIN8	C_RC_1;
	UIN8	C_RC_2;
	UIN8	C_US_0;
	UIN8	C_US_1;
	
	UIN8	C_US_2;
	UIN8	C_UC_0;
	UIN8	C_UC_1;
	UIN8	C_UC_2;
	
	UIN8	C_IS_0;
	UIN8	C_IS_1;
	UIN8	C_IS_2;
 	UIN8	C_IC_0;
	
	UIN8	C_IC_1;
	UIN8	C_IC_2;
    IN8		T_GD;
    UIN8	AF2;

///    int	C_RS;
///    int	C_UC;
///    int	C_US;
///    int	C_IC;
///    int	C_IS;
///    int	C_RC;

    int 	M0;
    UIN32	Eccentricity;

	UIN32	A_SQRT;
    int 	Omega0;

    int 	AngleInclination;
    int 	Omega;

    int 	OmegaDOT;
	int 	AF0;

    short	IDOT;
    UIN16	TOE;
    UIN16	TOC;   
    short	DeltaN;
///20181203    int		DeltaN;

    short	AF1;
	UIN16   WN;
//add
	UIN8    Health;
	UIN8    Available;
} PackICDObs; //total 28 param
#ifdef GLONASS
typedef struct
{
    UIN8	SVPRN;
	UIN8    Health;
    UIN16	TOE;

	UIN16   WN;
    short	AF1;

	int    rv[6];

    int	AF0;
	short   acceleration[3];

	UIN8    Available;
} PackGloICDObs;
typedef struct
{
//  long   Dmjd;
  UIN32   Dmjd;
  int   Rv[6];
  int   Clk;
  UIN16  Mjd0;
  UIN8   Quality;
} PackGloICDEph;
#endif
// enum and structures
/*enum IntDirection
{
  kIntBackward=-1,
  kIntForward=1
};

enum TimeSystem
{
  kTimeSysTT=0,
  kTimeSysTAI,
  kTimeSysGPST,
  kTimeSysUTC,
  kTimeSysUT1
};
*/
typedef double DOUBLE64;
#ifndef NULL
#define NULL 0
#endif
#pragma pack(push,4)

typedef struct tagXEOD_PARAM
{
  unsigned int mode;                      /* prediction mode, now only support 0, which is client generated */
  unsigned int max_obs_days;              /* max observation days that is used in prediction, now only support 3 days */
  unsigned int max_pred_days;             /* max prediction days to be estimated, now only support 6 days */
  unsigned int sample_interval;           /* sample interval for the obs, unit in second, now only support 900 */
  unsigned int step_size;                 /* step size that is used in prediction, unit in second, now only support 900 */
} XEOD_PARAM;

/*
typedef struct tagSatObs
{
  double    delta_mjd_tt;
  double    rv[6];
  double    clk;
  double    clkdot;

  unsigned long aodc;
  unsigned int  aoec;
  unsigned char fit_interval;
  unsigned char iode2;
  unsigned char iode3;
} SatObs;

typedef struct tagSatMass
{
  int       block_id;
  double    mass;
} SatMass;

typedef struct tagSatParam
{
  int    svid;
  int    mjd;

  double delta_mjd_tt;
  double rv[6];
  double rad[9];
  double af[3];
} SatParam;

typedef struct tagSatXYZT
{
  double delta_mjd;
  double x;
  double y;
  double z;
  double vx;
  double vy;
  double vz;
  double x_crs;
  double y_crs;
  double z_crs;
  double vx_crs;
  double vy_crs;
  double vz_crs;
  double week_sec;
  unsigned short week_no;
} SatXYZT;


typedef struct tagEcefPosition
{
  double x;
  double y;
  double z;
  unsigned char available;
} EcefPosition;
*/
//mjd0 + dmjd_end (days) < 1/12(2hour)
typedef struct tagXeodFileHeader
{
  int eph_valid[MAX_SV_PRN];
  int mjd0;
  double dmjd_start[MAX_SV_PRN];
  double dmjd_end[MAX_SV_PRN];
  int leap_sec;
  char reserved[1024-648];  // In case further fields will be added, 648 is the total size of the former parameters
} XeodFileHeader;

typedef struct tagKeplFileHeader
{
  UIN8 eph_valid[MAX_SV_PRN];//int eph_valid[MAX_SV_PRN];
  UIN16 mjd0;//int mjd0;
  UIN16 leap_sec;//int leap_sec;
  UIN32 dmjd_start[MAX_SV_PRN];//double dmjd_start[MAX_SV_PRN];
  UIN32 dmjd_end[MAX_SV_PRN];//double dmjd_end[MAX_SV_PRN];
//  char reserved[1024-648];  // In case further fields will be added, 648 is the total size of the former parameters
} KeplFileHeader;
typedef struct tagXeodFileTail
{
  char           label[8];                  // "raw.obs" or "ext.eph"
  unsigned int   file_version;              // file version number
  unsigned int   check_sum;
} XeodFileTail;

#ifndef _spreadOrbitDef_
#define _spreadOrbitDef_
typedef struct tagEphKepler
{
  short          svid;
  unsigned short week_no;
  unsigned short acc;

  unsigned char  iode2;
  unsigned char  iode3;
  double         M0;
  double         deltaN;
  double         ecc;
  double         Ek;
  double         sqrtA;
  double         OMEGA0;
  double         i0;
  double         w;
  double         OMEGADOT;
  double         IDOT;
  double         Cuc;
  double         Cus;
  double         Crc;
  double         Crs;
  double         Cic;
  double         Cis;

  double         group_delay;
  double         af0;
  double         af1;
  double         af2;
  unsigned int  aodc;      // LTE64BIT  unsigned long
  int            health;
  unsigned int  toc;    // LTE64BIT  unsigned long
  unsigned int  toe;   // LTE64BIT   unsigned long

  unsigned char  available;
  unsigned char  repeat;
  unsigned char  update;
  unsigned char  fit_interval;
} EphKepler;

typedef struct tagICDObsBlock
{
  unsigned int  sv_id;
  unsigned int  num_of_obs;
  PackICDObs*    eph_kep;
} ICDObsBlock;
/* position parameters to kepler parameters data structure */
typedef struct tagKepler_Ephemeris
{
   double     af0;
   double    m0; 
   double    e; 
   double    sqrta; 
   double    omega0;
   double    i0;
   double    omega;
   double    omegaDot; 
   double     tgd;  
   double     af2;
   double     af1; 
   double     Crs;
   double     deltan; 
   double     Cuc; 
   double     Cus;
   double     Cic;  
   double     Cis; 
   double     Crc; 
   double     idot;
   unsigned int       toc; //LTE64BIT long int
   unsigned int       toe; //LTE64BIT long int 
   unsigned int      wnZCount;//LTE64BIT unsigned long int 
   unsigned int      iodc; //LTE64BIT unsigned short int 
   unsigned short    weekNo; 
   signed char        fitint; 
   signed char        accuracy;   
   unsigned char       svid;   
   unsigned char       age:4;   
   signed char        status:4; 
} tSVD_Kepler_Ephemeris;

#ifndef _glo_eph_
#define _glo_eph_
typedef struct tagGloEph
{
  short          svid;
  int			 mjd0;
  double         dmjd;
  double         rv[6];
  double         acceleration[3];
  double         toe;
  int            weekNo;
  double         af0;
  double         af1;
  int            health;
  int            available;
} GLO_EPH;
#endif

typedef struct tagBdsEph
{
  char           available;
  char			 ephConfirmed;
  double         URA;
  int            prn;
  int			 mjd0;
  double         dmjd;
  int            weekNo;
  char           health;
  int            IODC;
  int			 IODE;

  double         toe;
  double         A;
  double         dA;
  double         Adot;
  double         dn;
  double         dndot;
  double         M0;
  double         ec;
  double         w;
  double         Cuc;
  double         Cus;
  double         Crc;
  double         Crs;
  double         Cic;
  double         Cis;
  double         i0;
  double         Idot;
  double         Omega0;
  double         Omegadot;
  double         tg1;
  double         tg2;

  double         Toc;;
  double         af0;
  double         af1;
  double         af2;

  int            navTemp[10];

  int            state;
} BDS_EPH;
#endif

typedef struct aliEph
{
    unsigned char	codeL2;
    unsigned char	L2Pdata;
    unsigned char	udre;// unkonw, may ura, need to confirm
    unsigned char	IODC;
    double	n;
    double	r1me2;// unkonw,  need to confirm
    double	OMEGA_n;// unkonw, need to confirm
    double	ODOT_n;// unkonw, need to confirm
}AliEph;

typedef struct ephextent
{
    EphKepler ekepler;
    AliEph alieph;
}EphExt;

typedef struct tagEphCartesian
{
  int           mjd0;
  double        delta_mjd;
  double        rv[6];
  double        clk; 
  unsigned int  quality;
} EphCartesian;

typedef struct tagObsBlock
{
  unsigned int  sv_id;
  unsigned int  num_of_obs;
  EphKepler*    eph_kep;
} ObsBlock;

typedef struct tagEphBlock
{
  unsigned int       sv_id;
  int                mjd0;
  double             dmjd0;
  unsigned int       obs_num;
  double             obs_span;
  double             pos_rms;
  double             clk_rms;
//20160105 add by hongxin.liu for the union of cartesian and kepler
//  int	OrbForm;				//ORBIT_CARTESIAN/ORBIT_KEPLERIAN
//  union
//  {
	  PackICDEph*                eph_icd;
	  EphCartesian*				eph_cart;
//  };
//20160105
} EphBlock;
#ifndef _gps_ephemeris_
#define _gps_ephemeris_
typedef struct // GPS ephemeris, also used by BD2                           
{
         unsigned short         iodc;
         unsigned char  iode2;
         unsigned char  iode3;

         unsigned char  ura;
         unsigned char  flag; // bit0 means ephemeris valid, bit1 means ephemeris verified, bit5 means pre_ephemeris 
         unsigned char  health;
         unsigned char  svid;

         int    toe;
         int    toc;
         int    week;

         // variables decoded from stream data
         DOUBLE64 M0;                      // Mean Anomaly at Reference Time
         DOUBLE64 delta_n;              // Mean Motion Difference from Computed Value
         DOUBLE64 ecc;                      // Eccentricity
         DOUBLE64 sqrtA;                  // Square Root of the Semi-Major Axis
         DOUBLE64 omega0;             // Longitude of Ascending Node of Orbit Plane at Weekly Epoch
         DOUBLE64 i0;                         // Inclination Angle at Reference Time
         DOUBLE64 w;                         // Argument of Perigee
         DOUBLE64 omega_dot;       // Rate of Right Ascension
         DOUBLE64 idot;            // Rate of Inclination Angle
         DOUBLE64 cuc;                      // Amplitude of the Cosine Harmonic Correction Term to the Argument of Latitude
         DOUBLE64 cus;                      // Amplitude of the Sine Harmonic Correction Term to the Argument of Latitude
         DOUBLE64 crc;                       // Amplitude of the Cosine Harmonic Correction Term to the Orbit Radius
         DOUBLE64 crs;                       // Amplitude of the Sine Harmonic Correction Term to the Orbit Radius
         DOUBLE64 cic;                       // Amplitude of the Cosine Harmonic Correction Term to the Angle of Inclination
         DOUBLE64 cis;                       // Amplitude of the Sine Harmonic Correction Term to the Angle of Inclination
         DOUBLE64 tgd;                      // Group Delay
         DOUBLE64 tgd2;          // Group Delay for B2
         DOUBLE64 af0;                      // Satellite Clock Correction
         DOUBLE64 af1;                      // Satellite Clock Correction
         DOUBLE64 af2;                      // Satellite Clock Correction

         // variables derived from basic data, avoid calculate every time
         DOUBLE64 axis;            // Semi-major Axis of Orbit
         DOUBLE64 n;                          // Corrected Mean Angular Rate
         DOUBLE64 root_ecc;  // Square Root of One Minus Ecc Square
         DOUBLE64 omega_t;           // Longitude of Ascending Node of Orbit Plane at toe
         DOUBLE64 omega_delta;   // Delta Between omega_dot and WGS_OMEGDOTE
         DOUBLE64 Ek;                        // Ek, derived from Mk
} GPS_EPHEMERIS, *PGPS_EPHEMERIS;
#endif
typedef struct tagApEphBlock
{
	unsigned int       sv_id;
	int                mjd0;
	double             dmjd0;
	unsigned int       obs_num;
	double             obs_span;
	double             pos_rms;
	double             clk_rms;
}ApEphBlock;
typedef struct tagECEFPOSITION
{
  long double x;
  long double y;
  long double z;
  unsigned char available;
} ECEFPOSITION;

#ifdef GLONASS
typedef struct tagGlonassFileHeader
{
  int eph_valid[GLO_MAX_SV_PRN];
  int mjd0;
  double dmjd_start[GLO_MAX_SV_PRN];
  double dmjd_end[GLO_MAX_SV_PRN];
  int leap_sec;
  char reserved[1024-488];  // In case further fields will be added, 488 is the total size of the former parameters
} GlonassFileHeader;

typedef struct tagICDGlonassFileHeader
{
  UIN8 eph_valid[GLO_MAX_SV_PRN];
  UIN16 mjd0;
  UIN16 leap_sec;
  UIN32 dmjd_start[GLO_MAX_SV_PRN];
  UIN32 dmjd_end[GLO_MAX_SV_PRN];
//  char reserved[1024-488];  // In case further fields will be added, 288 is the total size of the former parameters
} ICDGlonassFileHeader;
#endif

typedef struct tagGlonassFileTail
{
  char           label[8];                  // "raw.obs" or "ext.eph"
  unsigned int   file_version;              // file version number
  unsigned int   check_sum;
} GlonassFileTail;

#ifndef _kinematic_info_
#define _kinematic_info_
typedef struct
{
	DOUBLE64 x, y, z;
	DOUBLE64 vx, vy, vz;
	DOUBLE64 ve, vn, vu, Speed, Course, Elev;
	DOUBLE64 ClkDrifting;
#if SVN8416_BEFORE
	DOUBLE64 HDop, VDop;
	DOUBLE64 vHDop, vVDop;
#endif
} KINEMATIC_INF, *PKINEMATIC_INF;
#endif

#ifndef _glonass_ephemeris_
#define _glonass_ephemeris_
typedef struct
{
	unsigned char flag;	// bit0 means ephemeris valid
	// bit1 means position and velocity at tc valid
	signed char freq;	// frequency number of satellite
	unsigned char P;	// place P1, P2, P3, P4, ln, P from LSB at bit
	//      0/1,  2,  3,  4,  5, 6
	unsigned char M;	// satellite type 00 - GLONASS, 01 - GLONASS-M
	unsigned char Ft;	// indicator of accuracy of measurements
	unsigned char n;	// slot number that transmit signal
	unsigned char Bn;	// healthy flag
	unsigned char En;	// age of the immediate information
	unsigned int tb;	// index of interval within current day
	unsigned short tk;  // h:b76-b72,m:b71-b66,s:b65 * 30
	DOUBLE64 gamma;		// relative deviation of predicted carrier frequency
	DOUBLE64 tn;			// satellite clock error
	DOUBLE64 dtn;			// time difference between L1 and L2
	DOUBLE64 x, y, z;		// posistion in PZ-90 at instant tb
	DOUBLE64 vx, vy, vz;	// velocity in PZ-90 at instant tb
	DOUBLE64 ax, ay, az;	// acceleration in PZ-90 at instant tb

	// derived variables
	DOUBLE64 tc;			// reference time giving the following position and velocity
	KINEMATIC_INF PosVelT;	// position and velocity in CIS coordinate at instant tc
} GLN_EPHEMERIS, *PGLN_EPHEMERIS;  //GLONASS_EPHEMERIS, *PGLONASS_EPHEMERIS;
#endif

typedef struct tagGloObsBlock
{
  unsigned int  sv_id;
  unsigned int  num_of_obs;
  GLO_EPH*    GloEph;
} GloObsBlock;

typedef struct tagICDGloObsBlock
{
  unsigned int  sv_id;
  unsigned int  num_of_obs;
#ifdef GLONASS
  PackGloICDObs*    GloEph;
#endif
} ICDGloObsBlock;

typedef struct tagGloEphBlock
{
  unsigned int       sv_id;
  int                mjd0;
  double             dmjd0;
  unsigned int       obs_num;
  double             obs_span;
  double             pos_rms;
  double             clk_rms;
#ifdef GLONASS
  PackGloICDEph*		 eph_cart;
#endif
} GloEphBlock;

#ifdef BDS
typedef struct tagBdsFileHeader
{
  int eph_valid[BDS_MAX_SV_PRN];
  int mjd0;
  double dmjd_start[BDS_MAX_SV_PRN];
  double dmjd_end[BDS_MAX_SV_PRN];
  int leap_sec;
  char reserved[1024-288];  // In case further fields will be added, 288 is the total size of the former parameters
} BdsFileHeader;

typedef struct tagKeplBdsFileHeader
{
  UIN8 eph_valid[BDS_MAX_SV_PRN];
  UIN16 mjd0;
  UIN32 dmjd_start[BDS_MAX_SV_PRN];
  UIN32 dmjd_end[BDS_MAX_SV_PRN];
  UIN16 leap_sec;
//  char reserved[1024-288];  // In case further fields will be added, 288 is the total size of the former parameters
} KeplBdsFileHeader;

typedef struct tagBdsFileTail
{
  char           label[8];                  // "raw.obs" or "ext.eph"
  unsigned int   file_version;              // file version number
  unsigned int   check_sum;
} BdsFileTail;

typedef struct tagBdsObsBlock
{
  unsigned int  sv_id;
  unsigned int  num_of_obs;
  BDS_EPH*    BdsEph;
} BdsObsBlock;

typedef struct tagICDBdsObsBlock
{
  unsigned int  sv_id;
  unsigned int  num_of_obs;
  PackICDObs*    BdsEph;
} ICDBdsObsBlock;

typedef struct tagBdsEphBlock
{
  unsigned int       sv_id;
  int                mjd0;
  double             dmjd0;
  unsigned int       obs_num;
  double             obs_span;
  double             pos_rms;
  double             clk_rms;

	  PackICDEph*                eph_icd;
//20160829 begin for BDS kepler output
  EphCartesian*		 eph_cart;
//20160829 end   for BDS kepler output
} BdsEphBlock;
#endif
/*
typedef struct
{
	double x, y, z;
	double vx, vy, vz;
	double ve, vn, vu, Speed, Course, Elev;
	double ClkDrifting;

} TKINEMATIC_INFO, *PTKINEMATIC_INFO;

typedef struct 
{
	unsigned char svid;
	unsigned char PredictQuality;
	unsigned short SatInfoFlag;
	double IonoCorr,TropCorr;
	double ClockCorr;
	double PsrCorr;
	double PsrPredict,TransmitTime;
	int Doppler,CodePhase;
	float e1,az;
	TKINEMATIC_INFO PosVel;
	double ReceiverTime;
	int WeekNumber;
	unsigned int BlockCount;
	unsigned int bitNumber;
	unsigned int timeStamp;
	double CalcTimeLast;
}SATELLITE_INFO,*PSATELLITE_INFO;


*/
/*
typedef struct {
unsigned short iodc;
unsigned char iode2;
unsigned char iode3;
unsigned char ura;
unsigned char flag;
unsigned char health;
unsigned char svid;
int toe;
int toc;
int week;
double M0,delta_n,ecc,sqrtA,omega0,i0,w,omega_dot;
double idot,cuc,cus,crc,crs,cic,cis,tgd,tgd2,af0;
double af1,af2,axis,n,root_ecc,omega_t,omega_delta,Ek;
}GPS_EPHEMERIS,*PGPS_EPHEMERIS;
*/
#pragma pack(pop)

#ifdef __cplusplus
extern "C"
{
#endif

DLL_API_INT XEOD_Init(const XeodFileHeader* pobs_header, const XEOD_PARAM* pParams, const char* pwork_path);
DLL_API_INT XEOD_GeneratePredEph(EphBlock *peph_block, const ObsBlock *pobs_block, unsigned int step_size, unsigned int orbit_form);
DLL_API_INT XEOD_Deinit();

#ifdef GLONASS
DLL_API_INT GLONASS_Init(const GlonassFileHeader* pgloobs_header, const XEOD_PARAM* pParams=NULL, const char* pwork_path=NULL);
//following codes is the same as GPS-only, so we can disable them
/*
DLL_API_INT XEOD_Deinit();
*/
DLL_API_INT GLONASS_GeneratePredEph(GloEphBlock *peph_block, const GloObsBlock *pobs_block, unsigned int step_size, unsigned int orbit_form);
#endif

#ifdef BDS
DLL_API_INT BDS_Init(const BdsFileHeader* pbdsobs_header, const XEOD_PARAM* pParams, const char* pwork_path);

DLL_API_INT BDS_GeneratePredEph(BdsEphBlock *peph_block, const BdsObsBlock *pobs_block, unsigned int step_size, unsigned int orbit_form);
//20160829 begin for BDS kepler output
DLL_API_INT XEOD_GetBdsKeplerEEBlock(BdsEphBlock *peph_block);//, BYTE *pEEBlock, short *pWNBlock
//20160829 end   for BDS kepler output
#endif

//20151222 add by hongxin.liu->for the output of orbit parameters from the requirement of firmware team 
DLL_API_INT XEOD_GetKeplerEEBlock(EphBlock *peph_block);//, BYTE *pEEBlock, short *pWNBlock
//20151222
DLL_API_INT GetMJDate(int WN, double sow, double *delta_mjd);

#ifdef __cplusplus
}
#endif

#endif //__XEOD_CG_API_H
