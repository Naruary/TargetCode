/*******************************************************************************
*       @brief      Header File for UI_FixedField.c.
*       @file       Uphole/inc/UI_DataFields/UI_FixedField.h
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

    void FixedDisplay(MENU_ITEM* item);
    void FixedBeginEdit(MENU_ITEM* item);
    void FixedFinishEdit(MENU_ITEM* item);
    void FixedKeyPressed(MENU_ITEM* item, BUTTON_VALUE keyPressed);
    void FixedHighlight(MENU_ITEM* item);

#ifdef __cplusplus
}
#endif

#endif // UI_FIXED_FIELD_H
