/*******************************************************************************
** File Name:       jplbin.h
** Author:          Hongxin liu
** Date:            2015-11-15
** Copyright:       2008 Spreatrum, Incoporated. All Rights Reserved.
** Description:     This header file is used to compute JPL parameters.
********************************************************************************
********************************************************************************
**                              Edit History
** -----------------------------------------------------------------------------
** DATE                         NAME                      DESCRIPTION
** 2015/11/15            Hongxin liu                        Create.
*******************************************************************************/

/* UNCOMMENT ONE AND ONLY ONE OF THE FOLLOWING DENUM DEFINITIONS: */

/*#define DENUM 200*/
/*#define DENUM 403*/
/*#define DENUM 404*/
#define DENUM 405
/*#define DENUM 406*/

#if   DENUM==200
#define KSIZE 1652
#elif DENUM==403 || DENUM==405
#define KSIZE 2036
#elif DENUM==404 || DENUM==406
#define KSIZE 1456
#endif

#define NRECL 4
#define RECSIZE (NRECL*KSIZE)
#define NCOEFF (KSIZE/2)
#define TRUE 1
#define FALSE 0

/* we use long int instead of int for DOS-Linux compatibility */

struct rec1{
         char ttl[3][84];
         char cnam[400][6];
         double ss[3];
         long int ncon;
         double au;
         double emrat;
         long int ipt[12][3];
         long int numde;
         long int lpt[3];
       };
 typedef struct TagR1{
         struct rec1 r1;
         char spare[RECSIZE-sizeof(struct rec1)];
       }DefR1;

 struct rec2{
         double cval[400];
       };
 typedef struct TagR2{
         struct rec2 r2;
         char spare[RECSIZE-sizeof(struct rec2)];
       }DefR2;

