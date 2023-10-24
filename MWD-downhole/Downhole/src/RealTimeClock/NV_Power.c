/*!
********************************************************************************
*       @brief      This module provides Battery Backed SRAM and RTC
*                   initialization.
*       @file       Downhole/src/NVRAM/NV_Power.c
*       @date       July 2013
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <stm32f4xx.h>
#include <string.h>
#include "main.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

#define BBSRAM_START            BKPSRAM_BASE
#define BBSRAM_DEFAULT_VALUE    0xFF
#define BBSRAM_SIZE             4096

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

///@brief
///@param
///@return
static void powerUpBBSRAM(void);

///@brief
///@param
///@return
static void powerUpRTC(void);

static BOOL RTC_WAKUP_TIMER_FLAG = FALSE;

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
;   NVPower_Initialize()
;
; Description:
;   This function powers up the backplane interface to the NV SRAM and RTC.
;   Once powered it samples the state of the backup power supplies.  If either
;   power supply is not functioning, the backup power has been assumesd to have
;   failed and initialization takes place.
;
; Reentrancy:
;   No.
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void NVPower_Initialize(void)
{
    // Powerup Backplane
    PWR_BackupAccessCmd(ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_BKPSRAM, ENABLE);

    // Check to see if the voltage regulator to the BB_SRAM is functioning
    if(PWR_GetFlagStatus(PWR_FLAG_BRR) == FALSE)
    {
        // If not initialize the BB_SRAM
        powerUpBBSRAM();
    }

    // Check to see if the RTC has been previously initialized.
    if(RTC_GetFlagStatus(RTC_FLAG_INITS) == FALSE )
    {
        // If not initialize the RTC
        powerUpRTC();
    }
    PWR_BackupAccessCmd(DISABLE);
}//end NVPower_Initialize

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   powerUpBBSRAM()
;
; Description:
;   Turns on the BB_SRAM voltage Regulator and sets the 4K of BB_SRAM to 0xFF
;
; Reentrancy:
;   No.
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
static void powerUpBBSRAM(void)
{
    PWR_BackupRegulatorCmd(ENABLE);

    while(PWR_GetFlagStatus(PWR_FLAG_BRR) == FALSE)
        ;

    memset((void *)BBSRAM_START, BBSRAM_DEFAULT_VALUE, BBSRAM_SIZE);
}//end powerUpBBSRAM

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   powerUpRTC()
;
; Description:
;   Turns on the RTC oscilator and configures the RTC to operate.  Once
;   initialized the time and date will be set to the system anchor date.
;   This is Monday Jan. 1 2001 - 00:00:00.
;
; Reentrancy:
;   No.
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
static void powerUpRTC(void)
{
    //RCC_BackupResetCmd(ENABLE); // Make BB_SRAM reload after standby
    //RCC_BackupResetCmd(DISABLE); // Make BB_SRAM reload after standby
    // Check oscillator and turn on if neccessary.
    if(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == FALSE)
    {
        RCC_LSEConfig(RCC_LSE_ON);    // Enable LSE

        // Wait till LSE is ready
        while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
            ;
    }

    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE); // Select LSE as RTC Clock Source
    RCC_RTCCLKCmd(ENABLE);                  // Enable RTC Clock
    RTC_WaitForSynchro();                   // Wait for RTC synchronization

    if(RTC_DeInit() == SUCCESS)
    {
        RTC_InitTypeDef RTC_InitStruct;

        // The straight defaults are what is used.
        // 24H format and max pre-scale for 32.768kHz osc.
        RTC_StructInit(&RTC_InitStruct);

        if(RTC_Init(&RTC_InitStruct) == SUCCESS)
        {
            RTC_TimeTypeDef RTC_TimeStruct;
            RTC_DateTypeDef RTC_DateStruct;

            RTC_TimeStructInit(&RTC_TimeStruct);
            RTC_DateStructInit(&RTC_DateStruct);

            RTC_DateStruct.RTC_Year = 1;

            RTC_SetTime(RTC_Format_BIN, &RTC_TimeStruct);
            RTC_SetDate(RTC_Format_BIN, &RTC_DateStruct);
        }
    }
}//end powerUpRTC

void Set_Reset_RTC_WAKUP_TIMER_FLAG(void)
{
    RTC_WAKUP_TIMER_FLAG = RTC_GetFlagStatus(RTC_FLAG_WUTF);
}

BOOL Get_RTC_WAKUP_TIMER_FLAG(void)
{
    return RTC_WAKUP_TIMER_FLAG;
}
