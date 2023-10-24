/*******************************************************************************
*       @brief      Header File for UI_YesNoField.c.
*       @file       Uphole/inc/UI_DataFields/UI_BooleanField.h
*       @date       December 2019
*       @copyright  COPYRIGHT (c) 2019 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef UI_YESNO_FIELD_H
#define UI_YESNO_FIELD_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"
#include "TextStrings.h"
#include "UI_ScreenUtilities.h"

//============================================================================//
//      MACROS                                                                //
//============================================================================//

#define CREATE_YESNO_FIELD(Label, LabelFrame, ValueFrame, NextFrame, Getter, Setter) \
    {Label, LabelFrame, EditValue, ValueFrame, NextFrame, YesNoDisplay, YesNoBeginEdit, YesNoFinishEdit, YesNoKeyPressed, YesNoHighlight, .boolean = {0xFF, Getter, Setter}}

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C"
{
#endif

    void YesNoDisplay(MENU_ITEM* item);
    void YesNoBeginEdit(MENU_ITEM* item);
    void YesNoKeyPressed(MENU_ITEM* item, BUTTON_VALUE keyPressed);
    void YesNoHighlight(MENU_ITEM* item);
    void YesNoFinishEdit(MENU_ITEM* item);

#ifdef __cplusplus
}
#endif

#endif // UI_YESNO_FIELD_H
