/*******************************************************************************
** File Name:       Definition.h
** Author:          Hongxin liu
** Date:            2015-11-15
** Copyright:       2008 Spreatrum, Incoporated. All Rights Reserved.
** Description:     Thid header file defined some common variable
********************************************************************************
********************************************************************************
**                              Edit History
** -----------------------------------------------------------------------------
** DATE                         NAME                      DESCRIPTION
** 2015/11/15            Hongxin liu                        Create.
*******************************************************************************/
#ifndef __XEOD_CG_CORE_DEFINITION_H
#define __XEOD_CG_CORE_DEFINITION_H
#include "os_inc.h"

#ifndef WIN32
//#include <cutils/log.h>
//#define  E(...)   ALOGE(__VA_ARGS__)
//#define  D(...)   ALOGD(__VA_ARGS__)
//#include "os_inc.h"
#endif

/////////////////////////////////////////////////////////////////////////////////////////
// XEOD_CG related constants
#define EGM_ORDER               8
#define RK_ORDER                10
#define AM_ORDER                12
#define MAX_RK_ORDER            10                    //Maximum RK Integrator Order
#define MAX_AM_ORDER            12                    //Maximum AM Integrator Order
#define MAX_INT_ORDER           12                    //Maximum Integration Order
#define MAX_INT_DIM             100                   //Maximum Integration Dimension, (v,a,partials) or (r,v,DesignMatrix)

/////////////////////////////////////////////////////////////////////////////////////////
#define PI                      3.1415926535897932
//20151121 add by hongxin.liu for integrator_1obs algorithm
#define ALP1 0.238095238095238
#define ALP2 0.476190476190476
#define ALP3 0.142857142857143
#define ALP4 0.714285714285714
#define ALP5 0.827326835353988
#define ALP6 0.500000000000000
#define ALP7 0.172673164646011
#define ALP8 0.827326835353988
#define ALP9 1.000000000000000

#define G10  0.283446712018141e-01

#define G20  0.377928949357521e-01
#define G21  0.755857898715042e-01

#define G30  0.744897959183673e-02
#define G31  0.346938775510204e-02
#define G32 -0.714285714285714e-03

#define G40  0.212585034013605e-01
#define G42  0.819970845481050e-01
#define G43  0.151846452866861

#define G50  0.237030482317090e-01
#define G52  0.117839679152030
#define G53  0.183899839035716
#define G54  0.167922798289677e-01

#define G60  0.102329153264700e-01
#define G63  0.104602615893388
#define G64  0.222860077262258e-01
#define G65 -0.121215389460835e-01

#define G70  0.129643116717323e-01
#define G73 -0.322573966497032e-02
#define G74 -0.217859244674782e-01
#define G75  0.103950659297987e-01
#define G76  0.165602974253519e-01

#define G80  0.563233091263477e-01
#define G83 -0.133398085175373
#define G84 -0.732233052187797e-02
#define G85  0.117115008659951e-01
#define G86  0.114051947472607
#define G87  0.300868504480723

#define G90  0.418367346938776e-01
#define G93  0.100000000000000
#define G94 -0.111111111111111e-01
#define G96  0.192290249433107
#define G97  0.125641061472715
#define G98  0.513430655114124e-01

#define C0   0.500000000000000e-01
#define C6   0.177777777777777
#define C7   0.225216749624141
#define C8   0.470054725980809e-01

#define CD0  0.500000000000000e-01
#define CD6  0.355555555555556
#define CD7  0.272222222222222
#define CD8  0.272222222222222
#define CD9  0.500000000000000e-01


//#define 





/////////////////////////////////////////////////////////////////////////////////////////
// enum and structures
enum IntDirection
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

#pragma pack(push, 4)

typedef int BOOL;

//#ifdef BOOL
//typedef int bool;
//#endif

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
  BOOL available;
} EcefPosition;

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

  double         Toc;
  double         af0;
  double         af1;
  double         af2;

  int            navTemp[10];

  int            state;
  }BDS_EPH;
#endif

//20151222 add by hongxin.liu->for the output of orbit parameters from the requirement of firmware team 

/* bit definitions for status (ephemeris status) */
typedef enum
{
    EPHSTATUS_NO_DATA =   0x0000,
    EPHSTATUS_EPH_AVAIL = 0x0001,
    EPHSTATUS_ALM_AVAIL = 0x0002,
    EPHSTATUS_GOOD_CN0 =  0x0004
}tSVD_EphStatusEnum;

//Temp variables as required to solve inverse Kepler problem.
typedef struct ephTempType
{
	double	sinEk;	
	double	cosEk;	
	double	sin2Phik;	
	double	cos2Phik;
	double	sinUk;
	double	cosUk;
	double	rk;
	double  sinIk;
	double  cosIk;
	double  sinOmegak;
	double  cosOmegak;
	double	xp;
	double	yp;
} ephTempType;

/* status of SV state */
typedef enum
{
    SVSTATE_NO_DATA,
    SVSTATE_FROM_EPH,
    SVSTATE_FROM_ALM,
    SVSTATE_FROM_ACQ_ASSIST,
    SVSTATE_FROM_CURRENT_ALM,
    SVSTATE_FROM_EPH_EXTENSION,
    MAX_SVSTATE_ENUM
}tSVD_SVStateEnum;

/** SV State structure **/
typedef struct
{
   double            pos[3];  /* ECEF pos. of the Xmtr kept within the table.  */
                     /* Meters, Range: 1.5e7 to 3.0e7                 */
   double            vel[3];  /* Velocity of the transmitter (satellite or     */
                     /* ground). (meters/sec, -5000 to 5000)          */
   float             acc[3];  /* Acceleration of the transmitter.              */
                     /* unit: meters/sec/sec, range: -1.0 to 1.0      */
   float             jrk[3];  /* Change in acceleration of the satellite.      */
                     /* units: meters/sec**2/sec, range: -1.0 to 1.0  */
   double            af0;     /* Clock correction coefficient.                 */
                     /* units: sec, range: +-(2**31) to +-[(2**21)*   */
                     /*                       (2**-31)] (ref icd200)  */
   double            af1;     /* Clock correction coefficient.                 */
                     /* units: sec/sec,range:+-(2**-43) to +-[(2**15) */
                     /*                        (2**-43)] (ref icd200) */
   double            af2;     /* Clock correction coefficient.                 */
                     /* units: sec/sec**2,  +-(2**-55) to +-[(2**7)*	*/
                     /* (2**-55)] (ref icd200)                        */
   float             tcr;     /* Clock correction term due to the following    */
                     /* relativistic effects: velocity and gravita-   */
                     /* tional field. units: msec, range: 0 - 1.2     */
   float             rcd;     /* Rate of change of the rel_clock_corr          */
                     /* coefficient. units: msec/sec, -0.5 to 0.5     */
   float             posVar;  /* Anticipated variance in the reported position */
                     /* of the transmitter. [meters**2]               */
   float             clkVar;  /* Anticipated variance in the reported value of */
                     /* the clock bias.units: meters**2               */
   float             tgd;     /* the group delay from eph page 1               */

   double            gct;     /* The time in the gps reference frame when the	*/
                     /* transmitter state is computed.                */
                     /* [sec, 0 - 604800]                             */
   double            clockBias;  /* SV clock bias,units: sec                   */
   double            clockDrift; /* SV clock drift, units: sec/sec             */
   long int          tow;     /* GPS time referenced from start of the week.	*/
                     /* [sec, 0 - 604800], toe or toa                 */
   long int          cct;     /* This is the time when the correction          */
                     /* coefficients af0, af1, and af2 are valid.     */
                     /* [sec, 0 - 604800], toc                        */
   short int         wno;     /* The GPS week number, Eph or Alm weekno        */
                     /* [weeks, 0 - life time of GPS in weeks]        */
   short int         gcw;     /* The week number in the gps reference frame	*/
                     /* when the transmitter state calculation was	*/
                     /* made. [weeks, 0 - life time of gps in weeks]	*/
   unsigned char     iode;    /* This variable only has meaning if the         */
                     /* transmitter data is ephemeris. it represents	*/
                     /* the data set id number associated with the	*/
                     /* ephemeris. transmitted to the receiver.       */
                     /* the ephemeris data set changes every 4 or 6	*/
                     /* hours. [n/a,  0 - 255]                        */
   signed char       slw;     /* This flag indicates that a slew greater than	*/
                     /* 6 microseconds	has occurreed and will affect	*/
                     /* computations based on elapsed time.           */
                     /* [n/a, true/false]                             */
   unsigned char     svid;    /* The svid (prn) number of the satellite        */
   unsigned char     ephAge:4;     /* 0 mean not the extended ephemeris */
   tSVD_SVStateEnum  dataAvail:4;
                     /* This indicates the presence of data and if    */
                     /* there is data, the type of data present.      */
}tSVD_SVState;

typedef struct
{
   double   sqrta_2;	/* square of sqrta field */
   double   kepn;
   double   keps1;
   double   keps2;
   double   keps3;
   double   keps4;
   double   keps5;
   double   keps6;
   double   keps7;
   double   kepc2;
   double   kepc3;
   double   kepc4;
   double   kepc5;
   double   kepc6;
   double   kepc7;
   double   kep1e2;     /* 1.0 - ephe2 NOT USED OUTSIDE */
   double   keprx;      /* sqrta_2 * kep1e2 ONLY USED ONCE */
   double   kepk;       /* sqrt (kep1e2) */
   double   kepk2;      /* kepk * kep1e2 */
   double   kepsw;      /* sin (ephloc_1.omega) */
   double   kepcw;      /* cos (ephloc_1.omega) */
   double   kepl;       /* omega0 - omegaDot * toe NOT USED OUTSIDE */
   double   kepld;      /* omegaDot-WGS84_OMEGDOTE */
   double   kepl2;      /* WGS84_OMEGDOTE * toe */
   double   ntecos;     /* -WGSRelCorrConst * ephe * sqrta ONLY USED ONCE */
} tSVD_Kepler_EphemerisConsts;

typedef struct
{
   double time;
   long int week;
   BOOL isWeekValid;
   BOOL isTOWValid;
} tGPSTime;

#pragma pack(pop)
#endif //__XEOD_CG_CORE_DEFINITION_H
