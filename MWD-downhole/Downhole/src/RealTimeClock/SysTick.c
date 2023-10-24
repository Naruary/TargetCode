/*******************************************************************************
*       @brief      This module provides system initialization, the non-periodic
*                   main loop, and the periodic cycle handler.
*       @file       Downhole/src/SysTick.c
*       @date       July 2013
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <stm32f4xx.h>
#include "main.h"
#include "SysTick.h"

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

volatile BOOL Ten_mS_tick_flag=0;
volatile BOOL Hundred_mS_tick_flag=0;
volatile BOOL Thousand_mS_tick_flag=0;
volatile TIME_RT makeupSystemTicks = 0;
TIME_RT m_nRunTimeTicks = 0;
static TIME_RT m_nSystemTicks = 0;

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   SysTick_Handler()
;
; Description:
;   Globally named ISR for SysTicks
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void Process_SysTick_Events(void)
{
	static U_INT16 runTimeDiv = 0;

	runTimeDiv++;
	if(runTimeDiv > 1000)
	{
		runTimeDiv=0;
		m_nRunTimeTicks++;
	}
	m_nSystemTicks++;
	// while sleeping and clocks are off, the system ticks are not working.
	// tack on the lost time.
	if(makeupSystemTicks)
	{
		m_nSystemTicks += makeupSystemTicks;
		makeupSystemTicks=0;
	}
	// The SysTick_Handler is enabled for a brief period during initialization
	// to test the watchdog and low res. timer. During this test only the
	// tick count is incremented and the remainder of the routine is disabled.
	if((m_nSystemTicks % TEN_MILLI_SECONDS) == 0)
	{
		Ten_mS_tick_flag=1;
		if((m_nSystemTicks % HUNDRED_MILLI_SECONDS) == 0)
		{
			Hundred_mS_tick_flag=1;
			if((m_nSystemTicks % ONE_SECOND) == 0)
			{
				Thousand_mS_tick_flag=1;
			}
		}
	}
}// End SysTick_Handler()

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   SysTick_Init()
;
; Description:
;   Initialized the SysTick interrupt to fire every ms
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void SysTick_Init(void)
{
	RCC_ClocksTypeDef RCC_Clocks;
	RCC_GetClocksFreq(&RCC_Clocks);
	SysTick_Config(RCC_Clocks.HCLK_Frequency / 1000);
	NVIC_SetPriority(SysTick_IRQn, 1);
} // End SysTick_Init()

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   ElapsedTimeLowRes()
;
; Description:
;   Calculates the amount of time (in 1ms ticks) that has elapsed
;   since tOldTime, which is passed to this routine. This function can be used
;   to implement timers.  The following code snippet illustrates the simple,
;   yet effective use:
;
;       // start the timer
;       tMyTimer = ElapsedTimeLowRes(0)     // return the current time
;
;       // test the timer for expiration
;       if (ElapsedTimeLowRes(tMyTimer) > MY_TIMEOUT)
;       {
;           ...
;       }
;
;   NOTE: Please use the constants (or combinations of) defined in timer.h
;   for timeouts to allow simple changes to the constants if the time base
;   ever needs to change.
;
; Parameters:
;   TIME_RT tOldTime => the reference time from which now has elapsed.
;
; Returns:
;   TIME_RT => the time that has elapsed since nOldTime.
;
; Reentrancy:
;   Yes
;
; Assumptions:
;   Word access is used to load m_tTimerTicks in a single assembler
;   instruction, therefore interrupts do not need to be disabled.
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
TIME_RT ElapsedTimeLowRes(TIME_RT tOldTime)
{
	return (m_nSystemTicks - tOldTime);
}// End ElapsedTimeLowRes()

