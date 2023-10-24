/*******************************************************************************
*       @brief      Header file for system RTC functions
*       @file       Uphole/inc/rtc.h
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef RTC_H
#define RTC_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <intrinsics.h>
#include <stm32f4xx.h>
#include "portable.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

#define MINUTES_PER_HOUR              60
#define SECONDS_PER_MINUTE            60
#define SECONDS_PER_HOUR            ( 60 * (MINUTES_PER_HOUR))
#define SECONDS_PER_DAY             ( 24 * (SECONDS_PER_HOUR))    // 84600
#define SECONDS_PER_YEAR            (365 * (SECONDS_PER_DAY))

#define CURRENT_CENTURY             2000
#define BASELINE_YEAR               2001
#define FIRST_LEAP_YEAR             2004
#define MINIMUM_YEAR_VALUE          2010
#define MAXIMUM_YEAR_VALUE          2050
#define START_REAL_TIMER            ((TIME_RT) 0L)

//============================================================================//
//      MACROS                                                                //
//============================================================================//

// Converts seconds to and from absolute RTC timer ticks.
// Currently, 1 RTC tick is 1 second.
#define M_SecondsToRealTime(x)    ((TIME_RT)x)
#define M_RealTimeToSeconds(x)    ((U_INT32)x)

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

typedef U_INT32 TIME_RT;

typedef struct __RTC_TIME_DATE__
{
    U_INT32 nYear;
    U_INT32 nMonth;
    U_INT32 nDay;
    U_INT32 nDayOfWeek;
    U_INT32 nHours;
    U_INT32 nMinutes;
    U_INT32 nSeconds;
}RTC_TIME_DATE;

//============================================================================//
//      GLOBAL DATA                                                           //
//============================================================================//

extern const U_BYTE g_nDaysOfYearNonLeap[12];
extern const U_BYTE g_nDaysOfYearInLeap[12];

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

	void VerifyRTC(void);
	U_INT32 RTC_GetSeconds(void);
	void UpdateRTC(void);

#ifdef __cplusplus
}
#endif
#endif
