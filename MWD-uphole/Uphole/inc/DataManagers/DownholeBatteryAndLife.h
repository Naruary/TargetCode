/*******************************************************************************
*       @brief      Header File for downhole battery and on time
*       @file       Uphole/inc/DataManagers/DownholeBatteryAndLife.h
*       @date       December 2019
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef DOWNHOLEBATANDLIFE_H
#define DOWNHOLEBATANDLIFE_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

	void SetDownholeBatteryVoltage(U_INT16 BatVoltage);
        void SetDownholeBattery2Voltage(U_INT16 Batvoltage2);
	void SetDownholeSignalStrength(U_INT16 SignalStrength);
//	void SetDownholeTotalOnTime(U_INT32 TotalOnTime);
	U_INT16 GetDownholeBatteryVoltage(void);
        U_INT16 GetDownholeBattery2Voltage(void);
	U_INT16 GetDownholeSignalStrength(void);
//	U_INT32 GetDownholeTotalOnTime(void);
	void SetAwakeTimeSetting(INT16 AwakeTimeSetting);
	INT16 GetAwakeTimeSetting(void);
	void SetCurrentAwakeTime(U_INT16 CurrentAwakeTime);
	INT16 GetAwakeTimeLeft(void);

#ifdef __cplusplus
}
#endif

#endif // DOWNHOLEBATANDLIFE_H
