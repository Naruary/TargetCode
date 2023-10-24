/*******************************************************************************
*       @brief      This module provides functions to handle the use of the
*                   onboard LED.
*       @file       Uphole/src/HardwareInterfaces/led.c
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
#include "led.h"

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

typedef struct LED_STRUCT
{
    U_INT16 nLedPin;
    BOOL bCurrentState;
    BOOL bChangeRequest;
    BOOL bChangeValue;
} LED_STRUCT;

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//
/*
static LED_STRUCT StatusLeds[MAX_LED] = {
	{RED_LED_PIN, false, false, false},
	{ORANGE_LED_PIN, false, false, false},
	{YELLOW_LED_PIN, false, false, false},
	{GREEN_LED_PIN, false, false, false}
};
*/
//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*******************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   LED_InitPins()
;
; Description:
;   Sets up LED driver pin as an output and turns off the LED
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
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
//    GPIO_InitStructure.GPIO_Pin  = (RED_LED_PIN | ORANGE_LED_PIN | YELLOW_LED_PIN | GREEN_LED_PIN);
    GPIO_Init(LED_PORT, &GPIO_InitStructure);

//    GPIO_WriteBit(LED_PORT, RED_LED_PIN, Bit_RESET);
//    GPIO_WriteBit(LED_PORT, ORANGE_LED_PIN, Bit_RESET);
//    GPIO_WriteBit(LED_PORT, YELLOW_LED_PIN, Bit_RESET);
    GPIO_WriteBit(LED_PORT, GREEN_LED_PIN, Bit_RESET);
} // end LED_InitPins

/*******************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   StatusLEDManager()
;
; Description:
;
;
;
; Reentrancy:
;   No.
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
/*void StatusLEDManager(void)
{
    LED_VALUES nCurrentLed = FIRST_LED_VALUE;

    while(nCurrentLed < MAX_LED)
    {
        if(StatusLeds[nCurrentLed].bChangeRequest)
        {
            StatusLeds[nCurrentLed].bChangeRequest = false;
            SetStatusLEDState(nCurrentLed, StatusLeds[nCurrentLed].bChangeValue);
        }

        nCurrentLed++;
    }
} // end StatusLEDManager
*/
/*******************************************************************************
*       @details
*******************************************************************************/
//void ChangeStatusLEDAssignment(LED_VALUES eLed)
//{
//    SetStatusLEDState(eLed, false);
//} // end ChangeStatusLEDAssignment

/*******************************************************************************
*       @details
*******************************************************************************/
void LEDToggle(void)
{
	GPIO_WriteBit(LED_PORT, GREEN_LED_PIN, (GPIO_ReadOutputDataBit(LED_PORT, GREEN_LED_PIN) == Bit_RESET ? Bit_SET : Bit_RESET));
} // end LEDToggle

/*******************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void SetStatusLEDState(BOOL bState)
{
    GPIO_WriteBit(LED_PORT, GREEN_LED_PIN, bState ? Bit_SET : Bit_RESET);
} // end SetStatusLEDState
