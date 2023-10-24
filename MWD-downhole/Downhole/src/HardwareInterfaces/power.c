/*******************************************************************************
*       @brief      This module provides system power to the two sensor ports.
*                   3.3V to the compass and 18V to the gamma sensor.
*       @file       Downhole/src/HardwareInterfaces/power.c
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
#include "power.h"
#include "SysTick.h"
#include "RealTimeClock.h"
#include "FlashMemory.h"
#include "ModemDriver.h"

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

static TIME_RT tIdleTimer = 0;
static TIME_RT tSysOffTimer = 0;
volatile BOOL PowerFlag = 0;
//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*******************************************************************************
*       @details
*******************************************************************************/
void PWRSUP_InitPins(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_StructInit(&GPIO_InitStructure);
	// GPIO Test Pins
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	// Gyro Power Supply
	GPIO_InitStructure.GPIO_Pin  = COMPASS_POWER_PIN;
	GPIO_Init(COMPASS_POWER_PORT, &GPIO_InitStructure);
	GPIO_WriteBit(COMPASS_POWER_PORT, COMPASS_POWER_PIN, Bit_RESET);
	// Gamma Power Supply
	GPIO_InitStructure.GPIO_Pin  = GAMMA_POWER_PIN;
	GPIO_Init(GAMMA_POWER_PORT, &GPIO_InitStructure);
	GPIO_WriteBit(GAMMA_POWER_PORT, GAMMA_POWER_PIN, Bit_RESET);
}

/*******************************************************************************
*       @details
*******************************************************************************/
void EnableCompassPower(BOOL bPower)
{
	GPIO_WriteBit(COMPASS_POWER_PORT, COMPASS_POWER_PIN, bPower ? Bit_SET : Bit_RESET);
        PowerFlag = bPower;
}

/*******************************************************************************
*       @details
*******************************************************************************/
void EnableModemPower(BOOL bPower)
{
        GPIO_WriteBit(MODEM_POWER_PORT, MODEM_POWER_PIN, bPower ? Bit_RESET : Bit_SET);
}

/*******************************************************************************
*       @details
*******************************************************************************/
void StartIdleTimer(void)
{
	tIdleTimer = ElapsedTimeLowRes(0);
}

/*******************************************************************************
*       @details
*******************************************************************************/
TIME_RT GetIdleTimer(void)
{
	return tIdleTimer;
}

/*******************************************************************************
*       @details
*******************************************************************************/
void StartSysOffTimer(void)
{
	tSysOffTimer = ElapsedTimeLowRes(0);
}

/*******************************************************************************
*       @details
*******************************************************************************/
TIME_RT GetSysOffTimer(void)
{
	return tSysOffTimer;
}

/*******************************************************************************
*       @details
*******************************************************************************/
 void SetProcessorToStandbyModeUSE_RTC_Wakeup_Pin(U_INT16 AwakeTime)
{
	EXTI_InitTypeDef EXTI_InitStructure;
	RTC_WakeUpCmd(DISABLE); //Disable RTC wake up
	PWR_WakeUpPinCmd(DISABLE); // Disable Wake up pin
	PWR_BackupAccessCmd(ENABLE); // Enable acess to backup RTC registers

	EXTI_ClearITPendingBit(EXTI_Line22);
	EXTI_InitStructure.EXTI_Line = EXTI_Line22;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

//    NVIC_InitIrq(NVIC_RTC);
	RTC_Enable_Interrupt();

	PWR_ClearFlag(PWR_FLAG_WU); // clear PWR wake up flag
	RTC_ClearITPendingBit(RTC_IT_WUT); // clear the RTC IT pending bit
	EXTI_ClearITPendingBit(EXTI_Line22); // clear the EXT IT pending bit
	if(PWR_GetFlagStatus(PWR_FLAG_SB) != RESET)
	{
		PWR_ClearFlag(PWR_FLAG_SB); // Clear flag if wokeup from standby
		RTC_WaitForSynchro(); // wait for sync of RTC
	}
	RTC_ClearFlag(RTC_FLAG_WUTF); // clear RTC wakeup flag

	RTC_WakeUpClockConfig(RTC_WakeUpClock_CK_SPRE_16bits); //Select the clock to provied 1 sec clock
	RTC_SetWakeUpCounter(AwakeTime); // Set the number of seconds for wakeup
	RTC_ITConfig(RTC_IT_WUT, ENABLE); // enable RTC IT for WU

	PWR_FlashPowerDownCmd(ENABLE); // PowerDown Flash Memory

	PWR_WakeUpPinCmd(ENABLE); // Enable Wake Up Pin
	RTC_WakeUpCmd(ENABLE); // Enable RTC Wake up

	PWR_EnterSTANDBYMode(); // enter standby mode
}
 //MB 7/12/21
/*******************************************************************************
*       @details
*******************************************************************************/
 void SetProcessorToStandbyModeUSE_RTC(void)
{
	EXTI_InitTypeDef EXTI_InitStructure;
	RTC_WakeUpCmd(DISABLE); //Disable RTC wake up
	PWR_WakeUpPinCmd(DISABLE); // Disable Wake up pin
	PWR_BackupAccessCmd(ENABLE); // Enable acess to backup RTC registers

	EXTI_ClearITPendingBit(EXTI_Line22);
	EXTI_InitStructure.EXTI_Line = EXTI_Line22;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

//    NVIC_InitIrq(NVIC_RTC);
	RTC_Enable_Interrupt();

	PWR_ClearFlag(PWR_FLAG_WU); // clear PWR wake up flag
	RTC_ClearITPendingBit(RTC_IT_WUT); // clear the RTC IT pending bit
	EXTI_ClearITPendingBit(EXTI_Line22); // clear the EXT IT pending bit
	if(PWR_GetFlagStatus(PWR_FLAG_SB) != RESET)
	{
		PWR_ClearFlag(PWR_FLAG_SB); // Clear flag if wokeup from standby
		RTC_WaitForSynchro(); // wait for sync of RTC
	}
	RTC_ClearFlag(RTC_FLAG_WUTF); // clear RTC wakeup flag

	RTC_WakeUpClockConfig(RTC_WakeUpClock_CK_SPRE_16bits); //Select the clock to provied 1 sec clock
	RTC_SetWakeUpCounter(0); // Set the number of seconds for wakeup
	RTC_ITConfig(RTC_IT_WUT, ENABLE); // enable RTC IT for WU

	PWR_FlashPowerDownCmd(ENABLE); // PowerDown Flash Memory

	//PWR_WakeUpPinCmd(ENABLE); // Enable Wake Up Pin
	RTC_WakeUpCmd(ENABLE); // Enable RTC Wake up

	PWR_EnterSTANDBYMode(); // enter standby mode
}
 //MB 7/12/21