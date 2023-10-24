/*******************************************************************************
*       @brief      This module provides system RTC initialization, interrupt
*                   and accessor functions.
*       @file       Uphole/src/rtc.c
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <intrinsics.h>
#include <stm32f4xx_rtc.h>
#include <stm32f4xx_pwr.h>
#include <stm32f4xx.h>
#include <stdbool.h>
#include <stdio.h>
#include "portable.h"
#include "rtc.h"
#include "timer.h"
// #include "wdt.h"
#include "NVIC.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

const U_BYTE g_nDaysOfYearNonLeap[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
const U_BYTE g_nDaysOfYearInLeap[12] = {31,29,31,30,31,30,31,31,30,31,30,31};

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

static void setEpochSeconds(void);

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

// The value of the RTC is not valid
static BOOL m_bTimeIsSuspect = false;

// Global time in seconds since midnight Jan. 1, 2001.
static TIME_RT m_nSecondsRT = 0;

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   VerifyRTC()
;
; Description:
;   This will test and report to the error log if the RTC time is out of scope
;   or the RTC hardware is not running.
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void VerifyRTC(void)
{
    RTC_TimeTypeDef RTC_TimeStruct;
    RTC_DateTypeDef RTC_DateStruct;

    U_INT32 nSecondsStart, nSecondsNow;
    U_INT32 nRTCTestCycleCounter = 250000; // * 5 uS ~= 1.25 S

    RTC_GetTime(RTC_Format_BIN, &RTC_TimeStruct);
    RTC_GetDate(RTC_Format_BIN, &RTC_DateStruct);

    if(RTC_DateStruct.RTC_Date == 0 || RTC_DateStruct.RTC_Month == 0 || RTC_DateStruct.RTC_WeekDay == 0 || RTC_DateStruct.RTC_Year == 0)
    {
      RTC_DateStruct.RTC_Year = 1;
      RTC_DateStruct.RTC_Month = 1;
      RTC_DateStruct.RTC_Date = 1;
      RTC_DateStruct.RTC_WeekDay = 1;
    }

    nSecondsStart = nSecondsNow = RTC_TimeStruct.RTC_Seconds;

    // Continue to test the RTC, looking for the counter to tick by 1 second.
    // Don't bother if we already know that the clock was bad at init time
    // so we don't keep logging errors on every power cycle.
//    if(!SuspectRealTime())
    if(!m_bTimeIsSuspect)
    {
        while(nSecondsNow == nSecondsStart)
        {
            RTC_GetTime(RTC_Format_BIN, &RTC_TimeStruct);
            nSecondsNow = RTC_TimeStruct.RTC_Seconds;

            // KickWatchdog();
            Delay5us(); //This Func does not use resources to count

            // Keep doing for 250,000x that way @ 5uS more than 1S
            // has passed and we know timer is not running properly
            if(--nRTCTestCycleCounter == 0)
            {
//                SetSuspectRealTime(true);
//                LogErrorAndContinue(ERR_RTC_STOPPED);

                m_bTimeIsSuspect = true;
                return;
            }
        }

        if(((RTC_DateStruct.RTC_Year + CURRENT_CENTURY) < MINIMUM_YEAR_VALUE) ||
           ((RTC_DateStruct.RTC_Year + CURRENT_CENTURY) > MAXIMUM_YEAR_VALUE))
        {
            RTC_TimeStructInit(&RTC_TimeStruct);
            RTC_DateStructInit(&RTC_DateStruct);

            RTC_DateStruct.RTC_Year = 1;

            RTC_SetTime(RTC_Format_BIN, &RTC_TimeStruct);
            RTC_SetDate(RTC_Format_BIN, &RTC_DateStruct);

            // Now set the bit because it is suspect.
//            SetSuspectRealTime(true);
//            LogErrorAndContinue(ERR_RTC_VALUE);

            m_bTimeIsSuspect = true;
            return;
        }

        setEpochSeconds();
    }
}

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   setEpochSeconds()
;
; Description:
;    Sets the Epoch Time variable from the RTC time (in seconds since 2001)
;
; Parameter:
;   U_INT32 number of seconds since
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
static void setEpochSeconds(void)
{
    RTC_TimeTypeDef RTC_TimeStruct;
    RTC_DateTypeDef RTC_DateStruct;
    RTC_TIME_DATE   nSystemRT;
    BOOL bLeap = false;
    U_INT32 nDayOfYear = 0;
    U_INT32 nLeapDays = 0;
    U_INT32 nSecondsRT = 0;
    U_INT32 i;

    RTC_GetTime(RTC_Format_BIN, &RTC_TimeStruct);
    RTC_GetDate(RTC_Format_BIN, &RTC_DateStruct);

    if(RTC_DateStruct.RTC_Date == 0 || RTC_DateStruct.RTC_Month == 0 || RTC_DateStruct.RTC_WeekDay == 0 || RTC_DateStruct.RTC_Year == 0)
    {
      RTC_DateStruct.RTC_Year = 1;
      RTC_DateStruct.RTC_Month = 1;
      RTC_DateStruct.RTC_Date = 1;
      RTC_DateStruct.RTC_WeekDay = 1;
    }

    nSystemRT.nYear = RTC_DateStruct.RTC_Year + CURRENT_CENTURY;
    nSystemRT.nMonth = RTC_DateStruct.RTC_Month;
    nSystemRT.nDay = RTC_DateStruct.RTC_Date;
    nSystemRT.nDayOfWeek = RTC_DateStruct.RTC_WeekDay;
    nSystemRT.nHours = RTC_TimeStruct.RTC_Hours;
    nSystemRT.nMinutes = RTC_TimeStruct.RTC_Minutes;
    nSystemRT.nSeconds = RTC_TimeStruct.RTC_Seconds;

    //
    // If the year of the RTC value is before 2010, then the RTC battery
    // may be dead.
    //
    if((nSystemRT.nYear < MINIMUM_YEAR_VALUE) && !m_bTimeIsSuspect)
    {
        //
        // A small RTC value indicates that the battery may be dead.
        // The RTC is only ticking when the power is on and does not retain
        // its value through power cycles.  Set the Suspect flag.
        //
//        SetSuspectRealTime(true);
//        LogErrorAndContinue(ERR_RTC_VALUE);
        m_bTimeIsSuspect = true;
    }

    // Determine if current year is a leap year
    // (2004 is the first leap year after baseline 2001)
    if (nSystemRT.nYear >= FIRST_LEAP_YEAR)
    {
        bLeap = ((nSystemRT.nYear % 4) == 0);
    }

    nDayOfYear = (nSystemRT.nDay - 1);
    for(i = 0; i < (nSystemRT.nMonth - 1); i++ )
    {
        nDayOfYear += bLeap ? g_nDaysOfYearInLeap[i] : g_nDaysOfYearNonLeap[i];
    }

    // Now add leap days that have occured prior to this year.
    if (nSystemRT.nYear <= FIRST_LEAP_YEAR)
    {
        nLeapDays = 0;
    }
    else
    {
        nLeapDays = (((nSystemRT.nYear - 1) - FIRST_LEAP_YEAR) / 4) + 1;
    }

    nSecondsRT = nSystemRT.nSeconds + (nSystemRT.nMinutes * SECONDS_PER_MINUTE) + (nSystemRT.nHours * SECONDS_PER_HOUR);
    nSecondsRT += ((nLeapDays + nDayOfYear) * SECONDS_PER_DAY);
    nSecondsRT += ((nSystemRT.nYear - BASELINE_YEAR) * SECONDS_PER_YEAR);

    m_nSecondsRT = (TIME_RT)nSecondsRT;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   RTC_GetSeconds ()
;
; Description:
;   Returns the number seconds since 2001 as stored in the RTC
;
; Returns:
;   U_INT32 number of seconds since 2001
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
U_INT32 RTC_GetSeconds(void)
{
    return (U_INT32)m_nSecondsRT;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   UpdateRTC()
;
; Description:
;
; Parameters:
;
; Returns:
;
; Reentrancy:
;   No.
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void UpdateRTC(void)
{
    static RTC_TimeTypeDef RTC_TimePrevious = {0, 0, 0, 0};
    static RTC_TimeTypeDef RTC_TimeNow = {0, 0, 0, 0};

    RTC_GetTime(RTC_Format_BIN, &RTC_TimeNow);

    if(RTC_TimeNow.RTC_Seconds != RTC_TimePrevious.RTC_Seconds)
    {
        RTC_TimePrevious = RTC_TimeNow;
        setEpochSeconds();
    }

}//end UpdateRTC
