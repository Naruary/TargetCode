/*!
********************************************************************************
*       @brief      Source file for FixedPointValue.c.
*                   Documented by Josh Masters Dec. 2014
*       @file       Uphole/src/DataFields/FixedPointValue.c
*       @author     Chris Walker
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "FixedPointValue.h"

//============================================================================//
//      MACROS                                                                //
//============================================================================//

#define STRING_SAFE_POSITION(fixed) (fixed->position + 1)

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

///@brief
static char strValue[8];

///@brief
static char format[8];

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*!
********************************************************************************
*       @details
*******************************************************************************/

char* FixedValueFormat(FIXED_POINT_DATA* fixed)
{
    double value = (double) fixed->value / pow(10, fixed->fractionDigits);
    if (value >= 0)
    {
        sprintf(format, "%%%d.%df", fixed->numberDigits + 1, fixed->fractionDigits);
        sprintf(strValue, format, value);
    }
    else
    {
        sprintf(format, "-%%%d.%df", fixed->numberDigits + 1, fixed->fractionDigits);
        sprintf(strValue, format, -value);
    }
    return strValue;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static char* SafeFormatValue(FIXED_POINT_DATA* fixed)
{
    sprintf(format, "%%+0%dd", fixed->numberDigits + 1);
    sprintf(strValue, format, fixed->value);
    return strValue;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static char* FormatDigit(FIXED_POINT_DATA* fixed)
{
    static char digitValue[2];
    sprintf(digitValue, "%c", SafeFormatValue(fixed)[STRING_SAFE_POSITION(fixed)]);
    return digitValue;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

U_BYTE FixedValueDigit(FIXED_POINT_DATA* fixed)
{
    return atoi(FormatDigit(fixed));
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

BOOL FixedValueEdit(FIXED_POINT_DATA* fixed, U_BYTE digit)
{
    char* format = SafeFormatValue(fixed);
    INT32 newValue;

    format[STRING_SAFE_POSITION(fixed)] = DIGIT_ASCII(digit);
    newValue = atoi(format);
    if (VALID_VALUE(fixed, newValue))
    {
        fixed->value = newValue;
        return TRUE;
    }
    return FALSE;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

BOOL FixedValueDecrement(FIXED_POINT_DATA* fixed)
{
    U_BYTE digit = FixedValueDigit(fixed);
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

/*!
********************************************************************************
*       @details
*******************************************************************************/

BOOL FixedValueIncrement(FIXED_POINT_DATA* fixed)
{
    U_BYTE digit = FixedValueDigit(fixed);
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

/*!
********************************************************************************
*       @details
*******************************************************************************/

void FixedValueNextPosition(FIXED_POINT_DATA* fixed)
{
    if (fixed->position < MAX_POSITION(fixed))
    {
        fixed->position++;
    }
    else
    {
        fixed->position = 0;
    }
}

/*!
********************************************************************************
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
        fixed->position = MAX_POSITION(fixed);
    }
}