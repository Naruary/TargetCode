/*******************************************************************************
*       @brief      This module provides system WDT initialization, interrupt
*                   and accessor functions.
*       @file       Downhole/src/wdt.c
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
#include "main.h"
#include "SysTick.h"
#include "wdt.h"

BOOL watchdog_in_use = FALSE;
//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   KickWatchdog()
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void KickWatchdog(void)
{
	if(watchdog_in_use == FALSE) return;
#if 1
	IWDG_ReloadCounter();
#else // this was the WWDG method..
	// Changed from 0x55 to 0x50 for &2 Mhz clock to not reset WWDG
	if((WWDG->CR & 0x7F) == 0x50)
	{
		WWDG_SetCounter(0x7f);
	}
#endif
}

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   StartIWDT()
;
; Description:
;   Start the IWDG watchdog timer.
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void StartIWDT(void)
{
	// Min/max IWDG timeout period at 32 kHz (LSI)
	// These timings are given for a 32 kHz clock but the microcontroller’s internal RC frequency can vary from 30
	// to 60 kHz. Moreover, given an exact RC oscillator frequency, the exact timings still depend on the phasing
	// of the APB interface clock versus the LSI clock so that there is always a full RC period of uncertainty.
	// Divider		PR[2:0]		Min (ms) RL[11:0]=0x000		Max (ms) RL[11:0]=0xFFF
	// /4			0			0.125 						512
	// /8			1			0.25						1024
	// /16			2			0.5							2048
	// /32			3			1							4096
	// /64			4			2							8192
	// /128			5			4							16384
	// /256			6			8							32768
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	IWDG_SetPrescaler(6);
	IWDG_SetReload(0xFFF);
	IWDG_Enable();
//    RCC_APB1PeriphClockCmd(RCC_APB1Periph_WWDG,ENABLE);
	watchdog_in_use = TRUE;
	KickWatchdog();
}
