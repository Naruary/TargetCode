/*******************************************************************************
*       @brief      This module provides functions to handle the use of the
*                   onboard buzzer.
*       @file       Uphole/src/HardwareInterfaces/buzzer.c
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
#include "portable.h"
#include "board.h"
#include "buzzer.h"
#include "FlashMemory.h"
#include "systick.h"
#include "timer.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

//============================================================================//
//      MACROS                                                                //
//============================================================================//

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

static TIME_LR tBeepTime;

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   BUZZER_InitPins()
;
; Description:
;   Initializes the Buzzer by initializing the GPIO's
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void BUZZER_InitPins(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_StructInit(&GPIO_InitStructure);
	// GPIO LED status Pins
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Pin  = AUDIBLE_ALARM_PIN;
	GPIO_Init(AUDIBLE_ALARM_PORT, &GPIO_InitStructure);
	GPIO_WriteBit(AUDIBLE_ALARM_PORT, AUDIBLE_ALARM_PIN, Bit_RESET);
	tBeepTime = ElapsedTimeLowRes(START_LOW_RES_TIMER);
} // end BUZZER_InitPins

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   BuzzerHandler()
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void BuzzerHandler(void)
{
	// are we in the midst of timing out a tone?
	if( ElapsedTimeLowRes(tBeepTime) >= 10*TEN_MILLI_SECONDS )
	{
		M_BuzzerOff();
	}
	else
	{
		if(NVRAM_data.fKeyBeeperEnable)
		{
			M_BuzzerOn();
		}
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
void BuzzerKeypress(void)
{
	tBeepTime = ElapsedTimeLowRes(START_LOW_RES_TIMER);
}

/*******************************************************************************
*       @details
*******************************************************************************/
void BuzzerAlarm(void)
{
	tBeepTime = ElapsedTimeLowRes(START_LOW_RES_TIMER);
}
