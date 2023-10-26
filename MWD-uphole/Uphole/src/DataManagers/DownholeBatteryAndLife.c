/*******************************************************************************
*       @brief      This module provides downhole battery and on time life code
*       @file       Uphole/src/DataManagers/DownholeBatteryAndLife.c
*       @date       December 2019
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <stm32f4xx.h>
#include "DownholeBatteryAndLife.h"
#include "SysTick.h"
#include "timer.h"

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

static U_INT16 m_nBatteryVoltage = 0;
static U_INT16 m_nBatteryVoltage2 = 0;
static U_INT16 m_nSignalStrength = 0;
static U_INT16 m_nAwakeTimeSetting = 0;
static U_INT16 m_nCurrentAwakeTime = 0;
//static TIME_LR g_tAwakeTimer;

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*******************************************************************************
*       @details
*******************************************************************************/
void SetDownholeBatteryVoltage(U_INT16 BatVoltage)
{
	m_nBatteryVoltage = BatVoltage;
}
/*******************************************************************************
*       @details
*******************************************************************************/
void SetDownholeBattery2Voltage(U_INT16 BatVoltage2)
{
	m_nBatteryVoltage = BatVoltage2;
}

/*******************************************************************************
*       @details
*******************************************************************************/
U_INT16 GetDownholeBatteryVoltage(void)
{
        return m_nBatteryVoltage + 400;
}
/*******************************************************************************
*       @details
*******************************************************************************/
U_INT16 GetDownholeBattery2Voltage(void)
{
        return m_nBatteryVoltage2 + 400;
}

/*******************************************************************************
*       @details
*******************************************************************************/
void SetDownholeSignalStrength(U_INT16 SignalStrength)
{
	m_nSignalStrength = SignalStrength;
}

/*******************************************************************************
*       @details
*******************************************************************************/
U_INT16 GetDownholeSignalStrength(void)
{
        return m_nSignalStrength;
}

#if 0
/*******************************************************************************
*       @details
*******************************************************************************/
void SetDownholeTotalOnTime(U_INT32 TotalOnTime)
{
	m_nTotalOnTime = TotalOnTime;
}

/*******************************************************************************
*       @details
*******************************************************************************/
U_INT32 GetDownholeTotalOnTime(void)
{
	return m_nTotalOnTime;
}
#endif
/*******************************************************************************
*       @details
*******************************************************************************/
void SetAwakeTimeSetting(INT16 AwakeTimeSetting)
{
	m_nAwakeTimeSetting = AwakeTimeSetting;
}

/*******************************************************************************
*       @details
*******************************************************************************/
INT16 GetAwakeTimeSetting(void)
{
	return m_nAwakeTimeSetting;
}

/*******************************************************************************
*       @details
*******************************************************************************/
void SetCurrentAwakeTime(U_INT16 CurrentAwakeTime)
{
	m_nCurrentAwakeTime = CurrentAwakeTime;
//	g_tAwakeTimer = ElapsedTimeLowRes((TIME_LR)0);
}

/*******************************************************************************
*       @details
*******************************************************************************/
INT16 GetAwakeTimeLeft(void)
{ // whs 2Dec2021 this is the amount of time left on the Downhole 30 second power timer
	U_INT16 working;
    working = 30 - m_nCurrentAwakeTime++;// - ElapsedTimeLowRes(g_tAwakeTimer)/1000; // ZD 06/13/2023 I edited the m_nCurrentAwakeTime to be incremental adding the ++ after it, this seemed to fix an issue that made the countdown go by every second instead of what seemed to be a random every 5 seconds.
	return working;
}

