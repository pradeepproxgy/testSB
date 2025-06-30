/*******************************************************************************
* Author		  :  author
* Version		  :  V1.0
* Date			  :  2021-07-27
* Description	  :  util_timer.h
********************************************************************************/

#ifndef __UTIL_TIMER_H__
#define __UTIL_TIMER_H__

/********************************* Include File ********************************/
#include "HAL_OS.h"


/********************************* Macro Definition ****************************/


/********************************* Type Definition *****************************/
typedef struct
{
	uint64_t endTime;
}Timer;


/********************************* Function Prototype Definition ***************/
void TimerInit(Timer *timer);
void TimerCountdownMS(Timer *timer, uint32_t timeoutMs);
void TimerCountdown(Timer *timer, uint32_t timeoutS);
char TimerIsExpired(Timer *timer);
uint32_t TimerLeftMS(Timer *timer);


#endif
