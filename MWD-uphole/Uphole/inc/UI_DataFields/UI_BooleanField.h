/*******************************************************************************
*       @brief      Header File for UI_BooleanField.c.
*       @file       Uphole/inc/UI_DataFields/UI_BooleanField.h
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef UI_BOOLEAN_FIELD_H
#define UI_BOOLEAN_FIELD_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"
#include "TextStrings.h"
#include "UI_ScreenUtilities.h"

//============================================================================//
//      MACROS                                                                //
//============================================================================//

#define CREATE_BOOLEAN_FIELD(Label, LabelFrame, ValueFrame, NextFrame, Getter, Setter) \
    {Label, LabelFrame, EditValue, ValueFrame, NextFrame, BooleanDisplay, BooleanBeginEdit, BooleanFinishEdit, BooleanKeyPressed, BooleanHighlight, .boolean = {0xFF, Getter, Setter}}

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C"
{
#endif

    void BooleanDisplay(MENU_ITEM* item);
    void BooleanBeginEdit(MENU_ITEM* item);
    void BooleanKeyPressed(MENU_ITEM* item, BUTTON_VALUE keyPressed);
    void BooleanHighlight(MENU_ITEM* item);
    void BooleanFinishEdit(MENU_ITEM* item);

#ifdef __cplusplus
}
#endif

#endif // UI_BOOLEAN_FIELD_H
