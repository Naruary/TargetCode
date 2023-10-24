/*******************************************************************************
*       @brief      Header file for SysTick.c.
*       @file       Uphole/inc/SysTick.h
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef SYS_TICK_H
#define SYS_TICK_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <stm32f4xx.h>
#include "portable.h"
#include "timer.h"

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

extern volatile BOOL Ten_mS_tick_flag;
extern volatile BOOL Hundred_mS_tick_flag;
extern volatile BOOL Thousand_mS_tick_flag;
extern volatile TIME_LR makeupSystemTicks;
extern volatile INT16 TakeSurvey_Time_Out_Seconds;

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

//============================================================================//
//      Exposed variables                                                     //
//============================================================================//

	extern volatile BOOL SurveyTakenFlag;
	extern volatile BOOL SystemArmedFlag;

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

	void SysTick_Init(void);
	void Process_SysTick_Events(void);
	TIME_LR ElapsedTimeLowRes(TIME_LR nOldTime);

#ifdef __cplusplus
}
#endif

#endif // SYS_TICK_H
