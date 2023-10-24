/*******************************************************************************
*       @brief      Source file for FixedPointValue.c.
*       @file       Uphole/src/UI_DataFields/UI_FixedPointValue.c
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "UI_FixedPointValue.h"

//============================================================================//
//      MACROS                                                                //
//============================================================================//

#define STRING_SAFE_POSITION(fixed) (fixed->position + 1)

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

static char strValue[50];
static char format[50];

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*******************************************************************************
*       @details
*******************************************************************************/
char* FixedValueFormat(FIXED_POINT_DATA* fixed)
{
	double value = (double) fixed->value / pow(10, fixed->fractionDigits);
	if (value >= 0)
	{
		snprintf(format, 50, "%%%d.%df", fixed->numberDigits + 1, fixed->fractionDigits);
		snprintf(strValue, 50, format, value);
	}
	else
	{
		snprintf(format, 50, "-%%%d.%df", fixed->numberDigits + 1, fixed->fractionDigits);
		snprintf(strValue, 50, format, -value);
	}
	return strValue;
}

/*******************************************************************************
*       @details
*******************************************************************************/
static char* SafeFormatValue(FIXED_POINT_DATA* fixed)
{
	snprintf(format, 50, "%%+0%dd", fixed->numberDigits + 1);
	snprintf(strValue, 50, format, fixed->value);
	return strValue;
}

/*******************************************************************************
*       @details
*******************************************************************************/
static char* FormatDigit(FIXED_POINT_DATA* fixed)
{
	static char digitValue[10];
	snprintf(digitValue, 10, "%c", SafeFormatValue(fixed)[STRING_SAFE_POSITION(fixed)]);
	return digitValue;
}

/*******************************************************************************
*       @details
*******************************************************************************/
//U_BYTE FixedValueDigit(FIXED_POINT_DATA* fixed)
//{
//	return atoi(FormatDigit(fixed));
//}

/*******************************************************************************
*       @details
*******************************************************************************/
BOOL FixedValueEdit(FIXED_POINT_DATA* fixed, U_BYTE digit)
{
	INT32 newValue;
	char* format;

	format = SafeFormatValue(fixed);
	format[STRING_SAFE_POSITION(fixed)] = (digit+'0');
	newValue = atoi(format);
	if( (fixed->maxValue >= newValue) && (fixed->minValue <= newValue) )
	{
		fixed->value = newValue;
		return true;
	}
	return false;
}

/*******************************************************************************
*       @details
*******************************************************************************/
BOOL FixedValueDecrement(FIXED_POINT_DATA *fixed)
{
	U_BYTE digit = atoi(FormatDigit(fixed));
//	U_BYTE digit = FixedValueDigit(fixed);
	if (digit > 0)
	{
		digit--;
	}
	else
	{
		digit = 9;
	}
	return FixedValueEdit(fixed, digit);
}

/*******************************************************************************
*       @details
******************************************************************************/
BOOL FixedValueIncrement(FIXED_POINT_DATA* fixed)
{
	U_BYTE digit = atoi(FormatDigit(fixed));
//	U_BYTE digit = FixedValueDigit(fixed);
	if (digit < 9)
	{
		digit++;
	}
	else
	{
		digit = 0;
	}
	return FixedValueEdit(fixed, digit);
}

/*******************************************************************************
*       @details
*******************************************************************************/
void FixedValueNextPosition(FIXED_POINT_DATA* fixed)
{
	if(fixed->position < (fixed->numberDigits - 1) )
	{
		fixed->position++;
	}
	else
	{
		fixed->position = 0;
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
void FixedValuePrevPosition(FIXED_POINT_DATA* fixed)
{
	if (fixed->position > 0)
	{
		fixed->position--;
	}
	else
	{
		fixed->position = fixed->numberDigits - 1;
	}
}
