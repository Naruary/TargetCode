/*******************************************************************************
*       @brief      This module provides system WDT initialization, interrupt
*                   and accessor functions.
*       @file       Uphole/src/wdt.c
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
#include "SysTick.h"
#include "timer.h"
#include "wdt.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

#define WDT_20MS_VALUE 200
#define WDT_MAX_VALUE  0xFFF

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

typedef struct {
    U_BYTE nPreScale;
    U_INT16 nResetValue;
}WDT_PARAM_STRUCT;

WDT_PARAM_STRUCT nWDTParameters[WDT_MAX_LIST] = {{IWDG_Prescaler_4, WDT_20MS_VALUE},{IWDG_Prescaler_32, WDT_MAX_VALUE}};

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
;   KickWatchdog()
;
; Description:
;   Restart the watchdog if a systick has happened since last restart
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void KickWatchdog(void)
{
    static TIME_LR nLastTick = START_LOW_RES_TIMER;

    if(ElapsedTimeLowRes(nLastTick) >= FIVE_MILLI_SECONDS)
    {
        IWDG_ReloadCounter();

        nLastTick = ElapsedTimeLowRes(START_LOW_RES_TIMER);
    }
} // end KickWatchdog

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   SetWatchdogTimer()
;
; Description:
;   Initalize the WDT to a set of parmaeters
;
; Parameters:
;   WDT_PARAMETERS nParam => An index to a pre defined set of parameters.
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void SetWatchdogTimer(WDT_PARAMETERS nParam)
{
    //Reload so a change to a parameter does not trip the WDT
    IWDG_ReloadCounter();

    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    IWDG_SetReload((uint16_t)nWDTParameters[nParam].nResetValue);
    IWDG_SetPrescaler((uint8_t)nWDTParameters[nParam].nPreScale);
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Disable);

    while (IWDG->SR & (IWDG_SR_PVU | IWDG_SR_RVU))   //uncomment
        ; // wait for possible update to complete    //uncomment

    //Reload so the full WDT value is avaiable when we exit.
    IWDG_ReloadCounter();
} // end SetWatchdogTimer

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   StartIWDT()
;
; Description:
;   Start the watchdog timer.
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void StartIWDT(void)
{
    RCC->CSR |= RCC_CSR_LSION;

    while((RCC->CSR & RCC_CSR_LSIRDY) != RCC_CSR_LSIRDY)
      ;

    IWDG_Enable(); //uncomment

    DBGMCU_Config(( DBGMCU_IWDG_STOP | DBGMCU_WWDG_STOP ), ENABLE);

    SetWatchdogTimer(WDT_MAX_TIMEOUT_VALUE);
}// end StartIWDT
