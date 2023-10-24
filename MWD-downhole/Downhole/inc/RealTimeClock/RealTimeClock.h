/*******************************************************************************
*       @brief      Header file for system RTC functions
*       @file       Uphole/inc/RealTimeCLock/RealTimeClock.h
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef RTC_H
#define RTC_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <intrinsics.h>
#include <stm32f4xx.h>
#include "main.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

//============================================================================//
//      MACROS                                                                //
//============================================================================//

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

//============================================================================//
//      GLOBAL DATA                                                           //
//============================================================================//

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

	void RTC_Enable_Line22_interrupt(void);
	void RTC_Enable_Line22_event(void);
	void RTC_Enable_Interrupt(void);

#ifdef __cplusplus
}
#endif
#endif // RTC_H
