/*******************************************************************************
* Company		  :  unicom-IOT
* Author		  :  author
* Version		  :  V1.0
* Date			  :  2021-05-27
* Description	  :  util_type.h
********************************************************************************/

/********************************* Include File ********************************/
#ifndef _UTIL_TYPES_H_
#define _UTIL_TYPES_H_

#include <stddef.h>
#include "util_config.h"

/********************************* Macro Definition ****************************/



/********************************* Type Definition *****************************/

#if !defined(STDINT_ON_SYSTEM)

    typedef unsigned char       uint8_t;
    typedef signed char         int8_t;
    typedef unsigned short      uint16_t;
    typedef signed short        int16_t;
    typedef unsigned int        uint32_t;
    typedef signed int          int32_t;
    typedef unsigned long int   uint64_t;
    typedef signed long int     int64_t;
    typedef unsigned int        uintptr_t;

#else

    #include <stdint.h>

#endif  

/********************************* Variables ***********************************/

/********************************* Function ************************************/


#endif

