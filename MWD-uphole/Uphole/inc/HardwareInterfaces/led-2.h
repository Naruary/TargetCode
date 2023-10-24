/*!
********************************************************************************
*       @brief      Contains header information for the led module.
*                   Documented by Josh Masters Dec. 2014
*       @file       Uphole/inc/HardwareInterfaces/led.h
*       @author     Bill Kamienik
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef LED_H
#define LED_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

typedef enum __LED_VALUES__
{
    FIRST_LED_VALUE,
    LED_RED = FIRST_LED_VALUE,
    LED_ORANGE,
    LED_YELLOW,
    LED_GREEN,
    MAX_LED// <---- Must be the LAST entry
} LED_VALUES;

typedef enum __LED_ASSIGNMENT__
{
    STATUS_NONE,
    STATUS_1MS,
    STATUS_10MS,
    STATUS_100MS,
    STATUS_1000MS,
    STATUS_RTC,
    STATUS_SENSOR,
    STATUS_SURVEY,
    STATUS_DATA_LINK,
    MAX_STATUS// <---- Must be the LAST entry
} LED_ASSIGNMENT;

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

    ///@brief  
    ///@param  
    ///@return
    void LED_InitPins(void);

    ///@brief  
    ///@param  
    ///@return
    void StatusLEDManager(void);

    ///@brief  
    ///@param  
    ///@return
    void ChangeStatusLEDAssignment(LED_VALUES eLed, LED_ASSIGNMENT eNew);

    ///@brief  
    ///@param  
    ///@return
    void LEDToggle(LED_VALUES eLed, LED_ASSIGNMENT eOwner);

    ///@brief  
    ///@param  
    ///@return
    void SetStatusLEDState(LED_VALUES eLed, BOOL bState);

#ifdef __cplusplus
}
#endif
#endif