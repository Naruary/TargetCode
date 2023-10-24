/*!
********************************************************************************
*       @brief      Header File for UI_BooleanField.c.
*       @file       Uphole/inc/UI_DataFields/UI_InitiateField.h
*       @date       January 2019
*       @copyright  COPYRIGHT (c) 2019 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef UI_INITIATE_FIELD_H
#define UI_INITIATE_FIELD_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"
#include "TextStrings.h"
#include "UI_ScreenUtilities.h"

//============================================================================//
//      MACROS                                                                //
//============================================================================//

//#define CREATE_BOOLEAN_FIELD(Label, LabelFrame, ValueFrame, NextFrame, Getter, Setter) \
//	{Label, LabelFrame, EditValue, ValueFrame, NextFrame, BooleanDisplay, BooleanBeginEdit, BooleanFinishEdit, BooleanKeyPressed, BooleanHighlight, .boolean = {0xFF, Getter, Setter}}
//#define CREATE_INITIATE_FIELD(Label, LabelFrame, ValueFrame, NextFrame, Getter, Setter) \
//	{Label, LabelFrame, EditValue, ValueFrame, NextFrame, InitiateDisplay, InitiateBeginEdit, InitiateFinishEdit, InitiateKeyPressed, InitiateHighlight, .boolean = {0xFF, Getter, Setter}}
#define CREATE_INITIATE_FIELD(Label, LabelFrame, ValueFrame, NextFrame, Getter, Setter) \
	{Label, LabelFrame, EditValue, ValueFrame, NextFrame, InitiateDisplay, InitiateBeginEdit, InitiateFinishEdit, InitiateKeyPressed, InitiateHighlight, .boolean = {0xFF, Getter, Setter}}

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
    void InitiateDisplay(MENU_ITEM* item);

    ///@brief
    ///@param
    ///@return
    void InitiateBeginEdit(MENU_ITEM* item);

    ///@brief
    ///@param
    ///@return
    void InitiateKeyPressed(MENU_ITEM* item, BUTTON_VALUE keyPressed);

    ///@brief
    ///@param
    ///@return
    void InitiateHighlight(MENU_ITEM* item);

    ///@brief
    ///@param
    ///@return
    void InitiateFinishEdit(MENU_ITEM* item);

#ifdef __cplusplus
}
#endif

#endif // UI_INITIATE_FIELD_H
