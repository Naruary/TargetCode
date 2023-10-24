/*******************************************************************************
*       @brief      Header file for SysTick.c.
*       @file       Downhole/inc/SysTick.h
*       @date       July 2013
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef SYS_TICK_H
#define SYS_TICK_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "RealTimeClock.h"

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

extern TIME_RT m_nRunTimeTicks;

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

// the main (low res) timer period is exactly 1 ms.
#define START_LOW_RES_TIMER ((TIME_RT) 0)
#define MILLI_SECOND ((TIME_RT)1)
#define TWO_MILLI_SECONDS ((TIME_RT)2)
#define FOUR_MILLI_SECONDS ((TIME_RT)4)
#define FIVE_MILLI_SECONDS ((TIME_RT)5)
#define TEN_MILLI_SECONDS ((TIME_RT)10)
#define TWENTY_MILLI_SECONDS ((TIME_RT)20)
#define TWENTY_FIVE_MILLI_SECONDS ((TIME_RT)25)
#define FIFTY_MILLI_SECONDS ((TIME_RT)50)
#define HUNDRED_MILLI_SECONDS ((TIME_RT)100)
#define TWO_HUNDRED_MILLI_SECONDS ((TIME_RT)200)
#define TWO_HUNDRED_FIFTY_MILLI_SECONDS ((TIME_RT)250)
#define THREE_HUNDRED_MILLI_SECONDS ((TIME_RT)300)
#define FOUR_HUNDRED_MILLI_SECONDS ((TIME_RT)400)

#define HALF_SECOND             ((TIME_RT)500)
#define THREE_QUARTER_SECOND    ((TIME_RT)750)

#define ONE_SECOND   ((TIME_RT)1000)
#define TWO_SECOND   ((TIME_RT)ONE_SECOND*(TIME_RT)2)
#define THREE_SECOND ((TIME_RT)ONE_SECOND*(TIME_RT)3)
#define FIVE_SECOND  ((TIME_RT)ONE_SECOND*(TIME_RT)5)
#define SIX_SECOND   ((TIME_RT)ONE_SECOND*(TIME_RT)6)
#define SEVEN_SECOND ((TIME_RT)ONE_SECOND*(TIME_RT)7)
#define EIGHT_SECOND ((TIME_RT)ONE_SECOND*(TIME_RT)8)
#define NINE_SECOND  ((TIME_RT)ONE_SECOND*(TIME_RT)9)

#define TEN_SECOND    ((TIME_RT)ONE_SECOND*(TIME_RT)10)
#define TWENTY_SECOND ((TIME_RT)ONE_SECOND*(TIME_RT)20)
#define THIRTY_SECOND ((TIME_RT)ONE_SECOND*(TIME_RT)30)

#define ONE_MINUTE    ((TIME_RT)ONE_SECOND*(TIME_RT)60)
#define TWO_MINUTE   ((TIME_RT)2 * ONE_MINUTE)
#define FIVE_MINUTE  ((TIME_RT)5 * ONE_MINUTE)
#define TENTH_HOUR   ((TIME_RT)6 * ONE_MINUTE)
#define TEN_MINUTE   ((TIME_RT)10 * ONE_MINUTE)
#define FIFTEEN_MINUTE ((TIME_RT)15 * ONE_MINUTE)
#define TEN_THOUSAND_MINUTE ((TIME_RT)10000 * ONE_MINUTE)

#define CYCLE_TIME_MSEC 10  //Number of ms per cycle

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

	void SysTick_Init(void);
	void Process_SysTick_Events(void);
	TIME_RT ElapsedTimeLowRes(TIME_RT nOldTime);

	extern volatile BOOL Ten_mS_tick_flag;
	extern volatile BOOL Hundred_mS_tick_flag;
	extern volatile BOOL Thousand_mS_tick_flag;
	extern volatile TIME_RT makeupSystemTicks;

#ifdef __cplusplus
}
#endif

#endif
