/*******************************************************************************
*       @brief      This module provides functions to handle the use of the
*                   test io.
*       @file       Downhole/src/HardwareInterfaces/testio.c
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
#include "testio.h"

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
;   TEST_InitPins()
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void TESTIOInitPins(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_StructInit(&GPIO_InitStructure);
	// GPIO LED status Pins
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
//	GPIO_InitStructure.GPIO_Pin  = TEST_PIN_PIN;
//	GPIO_Init(TEST_PIN_PORT, &GPIO_InitStructure);
//	GPIO_WriteBit(TEST_PIN_PORT, TEST_PIN_PIN, Bit_RESET);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Pin  = WAKEIN_PIN;
	GPIO_Init(WAKEIN_PORT, &GPIO_InitStructure);
	GPIO_WriteBit(WAKEIN_PORT, WAKEIN_PIN, Bit_RESET);
}

/*******************************************************************************
*       @details
*******************************************************************************/
void TESTIOToggle(void)
{
//	GPIO_WriteBit(
//		TEST_PIN_PORT,
//		TEST_PIN_PIN,
//		(GPIO_ReadOutputDataBit(TEST_PIN_PORT, TEST_PIN_PIN) == Bit_RESET ? Bit_SET : Bit_RESET)
//	);
}

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   SetTESTState()
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void TESTIOSetState(BOOL bState)
{
//	GPIO_WriteBit(TEST_PIN_PORT, TEST_PIN_PIN, bState ? Bit_SET : Bit_RESET);
}

/*******************************************************************************
*       @details
*******************************************************************************/
BOOL WAKEIORead(void)
{
	return (BOOL)GPIO_ReadInputDataBit(WAKEIN_PORT, WAKEIN_PIN);
}

