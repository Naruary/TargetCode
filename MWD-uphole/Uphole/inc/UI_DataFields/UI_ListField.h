/*!
********************************************************************************
*       @brief      Header File for UI_ListField.c.
*       @file       Uphole/inc/UI_DataFields/UI_ListField.h
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef UI_LIST_FIELD_H
#define UI_LIST_FIELD_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"
#include "TextStrings.h"
#include "UI_ScreenUtilities.h"

//============================================================================//
//      MACROS                                                                //
//============================================================================//

#define CREATE_LIST_FIELD(Label, LabelFrame, ValueFrame, NextFrame, List, Count, Getter, Setter) \
    {Label, LabelFrame, EditValue, ValueFrame, NextFrame, ListDisplay, ListBeginEdit, ListFinishEdit, ListKeyPressed, ListHighlight, .list = {List, Count, Getter, Setter, 0xFF} }

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
    void ListDisplay(MENU_ITEM* item);

    ///@brief
    ///@param
    ///@return
    void ListBeginEdit(MENU_ITEM* item);

    ///@brief
    ///@param
    ///@return
    void ListFinishEdit(MENU_ITEM* item);

    ///@brief
    ///@param
    ///@return
    void ListKeyPressed(MENU_ITEM* item, BUTTON_VALUE keyPressed);

    ///@brief
    ///@param
    ///@return
    void ListHighlight(MENU_ITEM* item);

#ifdef __cplusplus
}
#endif
#endif