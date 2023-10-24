/*!
********************************************************************************
*       @brief      Header File for NV_Power.C
*       @file       Downhole/inc/NVRAM/NV_Power.h
*       @date       July 2013
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef NV_POWER_H
#define NV_POWER_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "main.h"

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

    ///@brief
    ///@param
    ///@return
    void NVPower_Initialize(void);

    void Set_Reset_RTC_WAKUP_TIMER_FLAG(void);

    BOOL Get_RTC_WAKUP_TIMER_FLAG(void);

#ifdef __cplusplus
}
#endif
#endif