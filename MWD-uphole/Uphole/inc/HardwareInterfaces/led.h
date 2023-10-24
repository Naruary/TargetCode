/*******************************************************************************
*       @brief      Contains header information for the led module.
*       @file       Uphole/inc/HardwareInterfaces/led.h
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
