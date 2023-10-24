/*******************************************************************************
*       @brief      Contains header information for the led module.
*       @file       Downhole/inc/HardwareInterfaces/led.h
*       @date       July 2013
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef LED_H
#define LED_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "main.h"

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

/*typedef enum __LED_ASSIGNMENT__
{
	STATUS_NONE,
	STATUS_1MS,
	STATUS_10MS,
	STATUS_100MS,
	STATUS_1000MS,
	MAX_STATUS// <---- Must be the LAST entry
} LED_ASSIGNMENT;
*/
//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

    void LED_InitPins(void);
    void LEDToggle(void);
    void SetStatusLEDState(BOOL bState);

#ifdef __cplusplus
}
#endif

#endif // LED_H
