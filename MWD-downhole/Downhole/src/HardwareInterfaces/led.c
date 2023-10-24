/*******************************************************************************
*       @brief      This module provides functions to handle the use of the
*                   onboard LED.
*       @file       Downhole/src/HardwareInterfaces/led.c
*       @date       July 2013
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
#include "board.h"
#include "led.h"

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   LED_InitPins()
;
; Description:
;   Sets up LED driver pin as an output and turns off the LED
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void LED_InitPins(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_StructInit(&GPIO_InitStructure);
	// GPIO LED status Pins
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Pin  = GREEN_LED_PIN;
	GPIO_Init(GREEN_LED_PORT, &GPIO_InitStructure);
	GPIO_WriteBit(GREEN_LED_PORT, GREEN_LED_PIN, Bit_RESET);
} // end LED_InitPins

/*******************************************************************************
*       @details
*******************************************************************************/
void LEDToggle(void)
{
	GPIO_WriteBit(
		GREEN_LED_PORT,
		GREEN_LED_PIN,
		(GPIO_ReadOutputDataBit(GREEN_LED_PORT, GREEN_LED_PIN) == Bit_RESET ? Bit_SET : Bit_RESET)
	);
}//end LEDToggle

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   SetStatusLEDState()
;
; Description:
;   Turns the onboard LED on or off
;
; Parameters:
;   BOOL bState =>  the ON / OFF request state for the onboard LED.
;
; Reentrancy:
;   No.
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void SetStatusLEDState(BOOL bState)
{
	GPIO_WriteBit(GREEN_LED_PORT, GREEN_LED_PIN, bState ? Bit_SET : Bit_RESET);
} // end SetStatusLEDState
