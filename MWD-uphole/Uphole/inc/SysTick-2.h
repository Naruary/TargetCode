/*!
********************************************************************************
*       @brief      Header file for SysTick.c.
*                   Documented by Josh Masters Dec. 2014
*       @file       Uphole/inc/SysTick.h
*       @author     Bill Kamienik
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
//      CONSTANTS                                                             //
//============================================================================//

#define TRIGGER_1MS_IRQ         EXTI_Line0
#define TRIGGER_10MS_IRQ        EXTI_Line1
#define TRIGGER_100MS_IRQ       EXTI_Line2
#define TRIGGER_1000MS_IRQ      EXTI_Line3

#define TRIGGER_ALL_EXTI_IRQS   (TRIGGER_1MS_IRQ | TRIGGER_10MS_IRQ | TRIGGER_100MS_IRQ | TRIGGER_1000MS_IRQ)

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

    ///@brief  
    ///@param  
    ///@return
    void SysTick_Init(void);
    
    ///@brief  
    ///@param  
    ///@return
    void Process_SysTick_Events(void);

    ///@brief  
    ///@param  
    ///@return
    TIME_LR ElapsedTimeLowRes(TIME_LR nOldTime);

    ///@brief  
    ///@param  
    ///@return
    void Permit10msInterrupt(void);

    ///@brief  
    ///@param  
    ///@return
    void PermitSysTickRoutines(void);
    
    void DoNotPermitSysTickRoutines(void);

#ifdef __cplusplus
}
#endif
#endif