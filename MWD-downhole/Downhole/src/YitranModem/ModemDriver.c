/*******************************************************************************
*       @brief      ModemDriver.c.
*       @file       Downhole/src/YitranModem/ModemDriver.c
*       @date       May 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <intrinsics.h>
#include <stm32f4xx.h>
#include "stm32f4xx_syscfg.h"
#include "stm32f4xx_exti.h"
#include "main.h"
#include "board.h"
#include "power.h"
#include "ModemDriver.h"

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

///@brief
static BOOL bModemIsPresent = FALSE;
static TIME_RT tTimeYtranLive;

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   EXTI0_IRQHandler()
;
; Description:
;   Handles EXTI0 interrupts.
;
; Reentrancy:
;   No
;
; Assumptions:
;   This function must be compiled for ARM (32-bit) instructions.
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void EXTI0_IRQHandler(void)
{
    tTimeYtranLive = ElapsedTimeLowRes(0);
}// End EXTI0_IRQHandler()

uint16_t getYtranLiveGap()
{
    return((uint16_t)(ElapsedTimeLowRes(0) - tTimeYtranLive));
}

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
void ModemDriver_InitPins(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
//    EXTI_InitTypeDef EXTI_InitStructure;
//    NVIC_InitTypeDef NVIC_InitStructure;

    GPIO_StructInit(&GPIO_InitStructure);

    // Modem RESET pin set up parameters.
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

    // GPIO PORTA Pin4: This is the Yitran Modem Hardware Reset
    GPIO_InitStructure.GPIO_Pin  = MODEM_RESET_PIN;
    GPIO_Init(MODEM_RESET_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin  = MODEM_POWER_PIN;
    GPIO_Init(MODEM_POWER_PORT, &GPIO_InitStructure);

    //GPIO_InitStructure.GPIO_Pin  = MODEM_RX_VOLT_PIN;
    //GPIO_Init(MODEM_RX_VOLT_PORT, &GPIO_InitStructure);
#if 0
    /* Configure Ytran live EXTI line */
    // Modem RESET pin set up parameters.
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_Pin  = YTRAN_LIVE_GPIO_PIN;
    GPIO_Init(YTRAN_LIVE_GPIO_PORT, &GPIO_InitStructure);
    SYSCFG_EXTILineConfig(YTRAN_LIVE_EXTI_PORT_SOURCE, YTRAN_LIVE_EXTI_PIN_SOURCE);
    EXTI_InitStructure.EXTI_Line = YTRAN_LIVE_EXTI_LINE;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
    /* Enable and set EXTI Interrupt to the lowest priority */
    NVIC_InitStructure.NVIC_IRQChannel = YTRAN_LIVE_EXTI_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure); 
#endif
    //Turn Modem ON
	EnableModemPower(TRUE);
//    GPIO_WriteBit(MODEM_POWER_PORT, MODEM_POWER_PIN, Bit_RESET);

    ModemDriver_PutInHardwareReset(TRUE);

}//end ModemDriver_InitPins

/*!
********************************************************************************
*       @details
*******************************************************************************/

void ModemDriver_PutInHardwareReset(BOOL bState)
{
    GPIO_WriteBit(MODEM_RESET_PORT, MODEM_RESET_PIN, ((bState) ? Bit_RESET : Bit_SET));
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

void SetModemIsPresent(BOOL bState)
{
    bModemIsPresent = bState;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

BOOL GetModemIsPresent(void)
{
    return bModemIsPresent;
}