/*******************************************************************************
*       @brief      Contains header information for the Downhole Power
*       @file       Uphole/inc/HardwareInterfaces/DownPower
*       @date       June 2021
*       @author     Micah Butler
*       @copyright  COPYRIGHT (c) 2021 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef DOWNPOWER_H
#define DOWNPOWER_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"
#include "keypad.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

#define DownPower_
#define DownPower_


//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef _cplusplus
extern "C" {
#endif
  
        void DownPow_On(void);
        void DownPow_Off(void);
        BOOL DownPow_Status(void);
        
#ifdef _cplusplus
}
#endif

