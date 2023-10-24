/*!
********************************************************************************
*       @brief      Contains Interrupt processing related definitions.
*       @file       Downhole/inc/InterruptEnabling.h
*       @date       July 2013
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef INTERRUPT_ENABLING_H
#define INTERRUPT_ENABLING_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "main.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

// BOOST_THREAD_PRIORITY will shift the base priority to 3 so that all
// interrupts of priority 3 or lower (higher numeric value meaning lower
// priority) will be disabled. See NVIC.c for interrupt priorities. Using
// this value as the base priority will only allow the motor control, dimmer,
// and I2C interrupts to execute, all other will be disabled until the
// base priority is restored.
#define BOOST_THREAD_PRIORITY ((U_INT32)2)

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

    ///@brief
    ///@param
    ///@return
    U_INT32 ReadInterruptStatusAndDisable(void);

    ///@brief
    ///@param
    ///@return
    void RestoreInterruptStatus(U_INT32 nOldPSW);

    ///@brief
    ///@param
    ///@return
    U_INT32 ReadPriorityStatusAndChange(U_INT32 nPriority);

    ///@brief
    ///@param
    ///@return
    void RestorePriorityStatus(U_INT32 nOldPSW);

#ifdef __cplusplus
}
#endif
#endif