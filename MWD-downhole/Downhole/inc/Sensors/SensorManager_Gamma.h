/*******************************************************************************
*       @brief      Contains header information for the Gamma Sensor module.
*       @file       Downhole/inc/Sensors/SensorManager_Gamma.h
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef GAMMA_SENSOR_H
#define GAMMA_SENSOR_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "main.h"

extern BOOL bValidGammaValues;

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

    // Initializes Gamma Sensor Pins
    void GammaSensor_InitPins(void);
    // Initializes Gamma Sensor TIM Settings
    void Initialize_Gamma_Sensor(void);
    // Updates Gamma Count
    void UpdateGammaCountsThisPeriod(void);
    // Returns Gamma Count
    U_INT16 GetCurrentGammaCount(void);
	void SetGammaPower(BOOL desiredState);

#ifdef __cplusplus
}
#endif
#endif