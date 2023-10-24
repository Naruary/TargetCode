/*!
********************************************************************************
*       @brief      Header File for UI_FixedField.c.
*                   Documented by Josh Masters Dec. 2014
*       @file       Uphole/inc/DataFields/UI_FixedField.h
*       @author     Chris Walker
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef UI_FIXED_FIELD_H
#define UI_FIXED_FIELD_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"
#include "TextStrings.h"
#include "UI_ScreenUtilities.h"

//============================================================================//
//      MACROS                                                                //
//============================================================================//

#define CREATE_FIXED_FIELD(Label, LabelFrame, ValueFrame, NextFrame, GetValue, SetValue, Digits, Fraction, Min, Max) \
    {Label, LabelFrame, EditValue, ValueFrame, NextFrame, FixedDisplay, FixedBeginEdit, FixedFinishEdit, FixedKeyPressed, FixedHighlight, .fixed = {GetValue, SetValue, Digits, Fraction, Min, Max}}

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C"
{
#endif

    ///@brief  
    ///@param  
    ///@return 
    void FixedDisplay(MENU_ITEM* item);
    
    ///@brief  
    ///@param  
    ///@return 
    void FixedBeginEdit(MENU_ITEM* item);
    
    ///@brief  
    ///@param  
    ///@return 
    void FixedFinishEdit(MENU_ITEM* item);
    
    ///@brief  
    ///@param  
    ///@return
    void FixedKeyPressed(MENU_ITEM* item, BUTTON_VALUE keyPressed);
    
    ///@brief  
    ///@param  
    ///@return 
    void FixedHighlight(MENU_ITEM* item);

#ifdef __cplusplus
}
#endif
#endif