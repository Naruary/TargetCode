/*!
********************************************************************************
*       @brief      Provides prototypes for public functions in 
*                   FixedPointValue.c.  Documented by Josh Masters Dec. 2014
*       @file       Uphole/inc/DataFields/FixedPointValue.h
*       @author     Chris Walker
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef FIXED_POINT_VALUE_H
#define FIXED_POINT_VALUE_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

#define ASCII_DIGIT_OFFSET  48

//============================================================================//
//      MACROS                                                                //
//============================================================================//

#define MAX_POSITION(fixed) ((fixed)->numberDigits - 1)
#define VALID_VALUE(fixed, newValue) (newValue <= (fixed)->maxValue && newValue >= (fixed)->minValue)
#define DIGIT_VALUE(ascii) (ascii - ASCII_DIGIT_OFFSET)
#define DIGIT_ASCII(value) (value + ASCII_DIGIT_OFFSET)

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

typedef struct _FIXED_POINT_DATA
{
    INT16 (*GetValue)(void);
    void (*SetValue)(INT16 value);
    U_BYTE numberDigits;
    U_BYTE fractionDigits;
    INT16 minValue;
    INT16 maxValue;
    INT16 value;
    U_BYTE position;
} FIXED_POINT_DATA;

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

    ///@brief  
    ///@param  
    ///@return 
    U_BYTE FixedValueDigit(FIXED_POINT_DATA* fixed);
    
    ///@brief  
    ///@param  
    ///@return 
    BOOL FixedValueEdit(FIXED_POINT_DATA* fixed, U_BYTE digit);
    
    ///@brief  
    ///@param  
    ///@return 
    char* FixedValueFormat(FIXED_POINT_DATA* fixed);
    
    ///@brief  
    ///@param  
    ///@return 
    BOOL FixedValueDecrement(FIXED_POINT_DATA* fixed);
    
    ///@brief  
    ///@param  
    ///@return 
    BOOL FixedValueIncrement(FIXED_POINT_DATA* fixed);
    
    ///@brief  
    ///@param  
    ///@return 
    void FixedValueNextPosition(FIXED_POINT_DATA* fixed);
    
    ///@brief  
    ///@param  
    ///@return 
    void FixedValuePrevPosition(FIXED_POINT_DATA* fixed);

#ifdef __cplusplus
}
#endif
#endif