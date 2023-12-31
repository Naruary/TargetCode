/*!
********************************************************************************
*       @brief      Header File for adc.c.
*                   Documented by Josh Masters Dec. 2014
*       @file       Uphole/inc/HardwareInterfaces/adc.h
*       @author     Bill Kamienik
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef ADC_H
#define ADC_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

// Constants for over voltage protection
#define SIXTEEN_VOLTS_IN_ADCOUNTS 2101
#define SEVENTEEN_VOLTS_IN_ADCOUNTS 2232
#define EIGHTEEN_VOLTS_IN_ADCOUNTS 2364
#define NINETEEN_VOLTS_IN_ADCOUNTS 2495
#define TWENTY_VOLTS_IN_ADCOUNTS 2626
#define TWENTYONE_VOLTS_IN_ADCOUNTS 2758
#define TWENTY_TWO_VOLTS_IN_ADCOUNTS 2889
#define TWENTY_THREE_VOLTS_IN_ADCOUNTS 3020

#define _1_5V 186 // 0.15 volts
#define _2V   248 // 0.2  volts
#define _3V   372
#define _4V   496 // 0.4  volts
#define _6V   744 // 0.6  volts
#define _8V   992 // 0.8  volts

#define BEMF_DETECTION_THRESHOLD _4V

#define ADC_TO_VOLTS_GAIN       0.00761f

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif
  
    ///@brief  
    ///@param  
    ///@return 
    void ADC_InitPins(void);
    
    ///@brief  
    ///@param  
    ///@return 
    void ADC_Initialize(void);
    
    ///@brief  
    ///@param  
    ///@return 
    void ADC_Start(void);
    
    ///@brief  
    ///@param  
    ///@return 
    REAL32 GetUpholeBatteryLife(void);
    
    ///@brief  
    ///@param  
    ///@return 
    //void SetUpholeBatteryLife(REAL32 value);

#ifdef __cplusplus
}
#endif
#endif