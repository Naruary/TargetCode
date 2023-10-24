/*******************************************************************************
*       @brief      This module provides system interrupt enabling and disabling
*                   functions.  Priority shhifting is another means of enabling
*                   and disabling interrupts.
*       @file       Uphole/src/InterruptEnabling.c
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <intrinsics.h>
#include <stm32f4xx.h>
#include "portable.h"
#include "InterruptEnabling.h"

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   ReadInterruptStatusAndDisable()
;
; Description:
;   Allows global disabling of interrupts
;
; Returns:
;   U_INT32 nOldState => The current interrupt status before the change
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
U_INT32 ReadInterruptStatusAndDisable(void)
{
    U_INT32 nOldState = (U_INT32)__get_interrupt_state();
    __disable_interrupt();

    return nOldState;
} // end ReadInterruptStatusAndDisable

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   RestoreInterruptStatus()
;
; Description:
;   Restores global interrupts to a previvously saved state.
;
; Parameters:
;   U_INT32 nOldState => Previously saved interrupt status.
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void RestoreInterruptStatus(U_INT32 nOldState)
{
    __set_interrupt_state((__istate_t)nOldState);
} // end RestoreInterruptStatus

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   ReadPriorityStatusAndChange()
;
; Description:
;   Elevates or depreciates the priority status of the current task running
;
; Parameters:
;   U_INT32 nNewPriority => Newly defined priority evel.
;
; Returns:
;   U_INT32 nOldPriority => The current priority level before the change
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
U_INT32 ReadPriorityStatusAndChange(U_INT32 nNewPriority)
{
    U_INT32 nOldPriority = (U_INT32)__get_BASEPRI();
    __set_BASEPRI(nNewPriority << 4);

    return nOldPriority;
} // end ReadPriorityStatusAndChange

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   RestorePriorityStatus()
;
; Description:
;   Restores the current task to a priority level previously saved.
;
; Parameters:
;   U_INT32 nOldPriority => Previously saved priority level.
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void RestorePriorityStatus(U_INT32 nOldPriority)
{
    __set_BASEPRI((uint32_t)nOldPriority);
} // end RestorePriorityStatus
