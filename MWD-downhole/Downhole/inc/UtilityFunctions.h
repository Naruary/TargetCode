/*******************************************************************************
*       @brief      Header File for UtilityFunctions.c.
*       @file       Downhole/inc/RASP/UtilityFunctions.h
*       @date       July 2013
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef UTILITY_FUNCTIONS_H
#define UTILITY_FUNCTIONS_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <stm32f4xx.h>
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
    U_INT16 GetUnsignedShort(U_BYTE* packet);

    INT16 GetSignedShort(U_BYTE* packet);

    U_BYTE GetUnsignedByte(U_BYTE* packet);



#ifdef __cplusplus
}
#endif
#endif