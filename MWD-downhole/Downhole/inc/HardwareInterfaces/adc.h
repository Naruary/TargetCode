/*******************************************************************************
*       @brief      Header File for adc.c.
*       @file       Downhole/inc/HardwareInterfaces/adc.h
*       @date       Jan 2016
*       @copyright  COPYRIGHT (c) 2016 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef ADC_H
#define ADC_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "main.h"

extern U_INT16 PeakDetectInputu16;
extern BOOL PeakDetectInputIsValid;

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

    void ADC_InitPins(void);
    void ADC_SetMeasurementDividerPower(BOOL onoff);
    void ADC_Initialize(void);
    void ADC_Start(void);
    void ADC_Disable(void);
    U_INT16 GetBatteryInputVoltageU16(void);
    U_INT16 GetPeakDetectInputU16(void);

#ifdef __cplusplus
}
#endif

#endif // ADC_H
