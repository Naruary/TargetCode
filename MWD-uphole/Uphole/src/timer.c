/*******************************************************************************
*       @brief      This module provides system Timer initialization and
*                   interrupts.
*       @file       Uphole/src/timer.c
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
#include "Portable.h"
#include "timer.h"

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   Delay5us()
;
; Description:
;   Delays 5 uS by busy waiting.
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void Delay5us(void)
{
    // Empirical testing shows that a count of about 168 yields 5 uS.
    // However, since we want to make sure that we delay at least 5 uS,
    // we're setting the count slightly higher.  175 seems good.
    U_INT32 x = 175;

    while(x--)
    {
        ;
    }
}

void DelayHalfSecond(void)
{
    // Empirical testing shows that a count of about 168 yields 5 uS.
    // However, since we want to make sure that we delay at least 5 uS,
    // we're setting the count slightly higher.  175 seems good.
    U_INT32 x = 17500000;

    while(x--)
    {
        ;
    }
}