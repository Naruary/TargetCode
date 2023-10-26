/*******************************************************************************
*       @brief      This module contains functionality for the tone generator.
*       @file       Uphole/src/HardwareInterfaces/tone_generator.c
*       @date       September 2021
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

//#include <intrinsics.h>
#include <stm32f4xx.h>
//#include "portable.h"
#include "board.h"
#include "tone_generator.h"

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

//static BOOL bModemIsPresent = false;
extern void SetStatusLEDState(BOOL state);

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   ModemDriver_InitPins()
;
; Description:
;
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void tone_generator_InitPins(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_StructInit(&GPIO_InitStructure);
    // tone generator pin set up parameters.
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Pin  = TONE_PIN;
    GPIO_Init(TONE_PORT, &GPIO_InitStructure);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

void tone_generator_setstate(BOOL bState)
{
	SetStatusLEDState(bState);
	GPIO_WriteBit(TONE_PORT, TONE_PIN, ((bState) ? Bit_SET : Bit_RESET));
}

