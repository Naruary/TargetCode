/*******************************************************************************
*       @brief      Contains header information for the sensor power module.
*       @file       Downhole/inc/HardwareInterfaces/power.h
*       @date       July 2013
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef POWER_H
#define POWER_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "main.h"
#include "SysTick.h"

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

    void PWRSUP_InitPins(void);
    void EnableCompassPower(BOOL bPower);
    void StartIdleTimer(void);
    TIME_RT GetIdleTimer(void);
    void StartSysOffTimer(void);
    TIME_RT GetSysOffTimer(void);
    void SetProcessorToStandbyModeUSE_RTC_Wakeup_Pin(U_INT16);
    void SetProcessorToStandbyModeUSE_RTC(void);
	void EnableModemPower(BOOL bPower);
	void WAKEUP_InitPins(void);
        extern volatile BOOL PowerFlag;

#ifdef __cplusplus
}
#endif
#endif
