/*******************************************************************************
*       @brief      This module provides support and configuration for the
*                   device info.
*       @file       Downhole/src/RASP/UtilityFunctions.c
*       @date       July 2013
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <stm32f4xx.h>
#include <string.h>
#include "main.h"
#include "UtilityFunctions.h"

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*******************************************************************************
*       @details
*******************************************************************************/

U_INT16 GetUnsignedShort(U_BYTE* packet)
{
    U_INT16 value = 0;
    memcpy((void *)&value, (const void *)packet, sizeof(value));
    return value;
}

INT16 GetSignedShort(U_BYTE* packet)
{
    INT16 value = 0;
    memcpy((void *)&value, (const void *)packet, sizeof(value));
    return value;
}

U_BYTE GetUnsignedByte(U_BYTE* packet)
{
    U_BYTE value = 0;
    memcpy((void *)&value, (const void *)packet, sizeof(value));
    return value;
}