/*******************************************************************************
*       @brief      This module provides support and configuration for the
*                   device info.
*       @file       Uphole/src/UtilityFunctions.c
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <stm32f4xx.h>
#include <string.h>
#include "portable.h"
#include "UtilityFunctions.h"

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*******************************************************************************
*       @details
*******************************************************************************/
U_INT32 GetUnsignedLong(U_BYTE* packet)
{
	U_INT32 value = 0;
	memcpy((void *)&value, (const void *)packet, sizeof(value));
	return value;
}

/*******************************************************************************
*       @details
*******************************************************************************/
U_INT16 GetUnsignedShort(U_BYTE* packet)
{
	U_INT16 value = 0;
	memcpy((void *)&value, (const void *)packet, sizeof(value));        
	return value;
}

/*******************************************************************************
*       @details
*******************************************************************************/
INT16 GetSignedShort(U_BYTE* packet)
{
	INT16 value = 0;
	memcpy((void *)&value, (const void *)packet, sizeof(value));
	return value;
}

/*******************************************************************************
*       @details
*******************************************************************************/
void WriteUnsignedInt(U_BYTE* packet, U_INT32 value)
{
	memcpy(packet, &value, sizeof(U_INT32));
}

/*******************************************************************************
*       @details
*******************************************************************************/
void WriteUnsignedShort(U_BYTE* packet, U_INT16 value)
{
	memcpy(packet, &value, sizeof(U_INT16));
}

/*******************************************************************************
*       @details
*******************************************************************************/
void WriteCharString(U_BYTE* packet, char* value, U_INT16 nDataLen)
{
	memcpy(packet, value, nDataLen);
}

/*******************************************************************************
*       @details
*******************************************************************************/
REAL32 GetREAL32(U_BYTE* packet)
{
	REAL32 value = 0.0;
	memcpy((void *)&value, (const void *)packet, sizeof(value));
	return value;
}
