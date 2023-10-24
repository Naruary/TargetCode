/*******************************************************************************
*       @brief      This module provides system initialization, the non-periodic
*                   main loop, and the periodic cycle handler.
*       @file       Uphole/src/SysTick.c
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <intrinsics.h>
#include <stm32f4xx.h>
#include <stdio.h>
#include "portable.h"
#include "Timer.h"
#include "keypad.h"
#include "systick.h"

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

volatile BOOL Ten_mS_tick_flag = 0;
volatile BOOL Hundred_mS_tick_flag = 0;
volatile BOOL Thousand_mS_tick_flag = 0;
volatile TIME_LR makeupSystemTicks = 0;
static TIME_LR m_nSystemTicks = 0;
volatile INT16 TakeSurvey_Time_Out_Seconds = 0;
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
    m_nSystemTicks++;
	// while sleeping and clocks are off, the system ticks are not working.
	// tack on the lost time.
    if(makeupSystemTicks)
	{
		m_nSystemTicks += makeupSystemTicks;
		makeupSystemTicks = 0;
	}
	KeyPadManager();
	if((m_nSystemTicks % TEN_MILLI_SECONDS) == 0)
	{
		Ten_mS_tick_flag = 1;
		if((m_nSystemTicks % HUNDRED_MILLI_SECONDS) == 0)
		{
			Hundred_mS_tick_flag = 1;
                        // whs 10dec2021 if m_nSys div by ONE_sec remainder == 0 inc
			if((m_nSystemTicks % ONE_SECOND) == 0)
			{
				Thousand_mS_tick_flag = 1;
				TakeSurvey_Time_Out_Seconds++;
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
/* Setup SysTick Timer for 1 msec interrupts.
     ------------------------------------------
    1. The SysTick_Config() function is a CMSIS function which configures:
       - The SysTick Reload register with value passed as function parameter.
       - Configure the SysTick IRQ priority to the lowest value (0x0F).
       - Reset the SysTick Counter register.
       - Configure the SysTick Counter clock source to be Core Clock Source (HCLK).
       - Enable the SysTick Interrupt.
       - Start the SysTick Counter.

    2. You can change the SysTick Clock source to be HCLK_Div8 by calling the
       SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8) just after the
       SysTick_Config() function call. The SysTick_CLKSourceConfig() is defined
       inside the misc.c file.

    3. You can change the SysTick IRQ priority by calling the
       NVIC_SetPriority(SysTick_IRQn,...) just after the SysTick_Config() function
       call. The NVIC_SetPriority() is defined inside the core_cm3.h file.

    4. To adjust the SysTick time base, use the following formula:
       Reload Value = SysTick Counter Clock (Hz) x  Desired Time base (s)
       - Reload Value is the parameter to be passed for SysTick_Config() function
       - Reload Value should not exceed 0xFFFFFF   */
	if (SysTick_Config(SystemCoreClock / 1000))  // 1mS systick interrupts
	{
		// true is an error, so we will reset.
		NVIC_SystemReset();
	}
	NVIC_SetPriority(SysTick_IRQn, 1);
}// End SysTick_Init()

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
;   TIME_LR tOldTime => the reference time from which now has elapsed.
;
; Returns:
;   TIME_LR => the time that has elapsed since nOldTime.
;
; Reentrancy:
;   Yes
;
; Assumptions:
;   Word access is used to load m_tTimerTicks in a single assembler
;   instruction, therefore interrupts do not need to be disabled.
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
TIME_LR ElapsedTimeLowRes(TIME_LR tOldTime)
{
	return (m_nSystemTicks - tOldTime);
}// End ElapsedTimeLowRes()

