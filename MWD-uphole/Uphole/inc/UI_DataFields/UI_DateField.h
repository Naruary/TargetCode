/*!
********************************************************************************
*       @brief      Header File for UI_DateField.c.
*       @file       Uphole/inc/UI_DataFields/UI_DateField.h
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef UI_DATE_FIELD_H
#define UI_DATE_FIELD_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"
#include "UI_ScreenUtilities.h"

//============================================================================//
//      MACROS                                                                //
//============================================================================//

#define CREATE_DATE_FIELD(Label, LabelFrame, ValueFrame, NextFrame, GetDate, SetDate) \
    {Label, LabelFrame, EditValue, ValueFrame, NextFrame, DateDisplay, DateBeginEdit, DateFinishEdit, DayKeyPressed, DateHighlight, .date = {GetDate, SetDate}}

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

    ///@brief
    ///@param
    ///@return
    void DateDisplay(MENU_ITEM* item);

    ///@brief
    ///@param
    ///@return
    void DateBeginEdit(MENU_ITEM* item);

    ///@brief
    ///@param
    ///@return
    void DateFinishEdit(MENU_ITEM* item);

    ///@brief
    ///@param
    ///@return
    void DayKeyPressed(MENU_ITEM* item, BUTTON_VALUE keyPressed);

    ///@brief
    ///@param
    ///@return
    void DateHighlight(MENU_ITEM* item);

#ifdef __cplusplus
}
#endif
#endif