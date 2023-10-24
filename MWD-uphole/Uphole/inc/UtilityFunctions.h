/*******************************************************************************
*       @brief      Header File for UtilityFunctions.c.
*       @file       Uphole/inc/UtilityFunctions.h
*       @date       December 2014
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
#include "portable.h"

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

    U_INT16 GetUnsignedShort(U_BYTE* packet);
    U_INT32 GetUnsignedLong(U_BYTE* packet);
    INT16 GetSignedShort(U_BYTE* packet);
    void WriteUnsignedInt(U_BYTE* packet, U_INT32 value);
    void WriteUnsignedShort(U_BYTE* packet, U_INT16 value);
    void WriteCharString(U_BYTE* packet, char* value, U_INT16 nDataLen);
    REAL32 GetREAL32(U_BYTE* packet);

#ifdef __cplusplus
}
#endif

#endif
