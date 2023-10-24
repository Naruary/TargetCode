/*!
********************************************************************************
*       @brief      Header File for UI_TimeField.c.
*       @file       Uphole/inc/UI_DataFields/UI_TimeField.h
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef UI_TIME_FIELD_H
#define UI_TIME_FIELD_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"
#include "UI_ScreenUtilities.h"

//============================================================================//
//      MACROS                                                                //
//============================================================================//

#define CREATE_TIME_FIELD(Label, LabelFrame, ValueFrame, NextFrame, GetTime, SetTime) \
    {Label, LabelFrame, EditValue, ValueFrame, NextFrame, TimeDisplay, TimeBeginEdit, TimeFinishEdit, TimeKeyPressed, TimeHighlight, .time = {GetTime, SetTime}}

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
    void TimeDisplay(MENU_ITEM* item);

    ///@brief
    ///@param
    ///@return
    void TimeBeginEdit(MENU_ITEM* item);

    ///@brief
    ///@param
    ///@return
    void TimeFinishEdit(MENU_ITEM* item);

    ///@brief
    ///@param
    ///@return
    void TimeKeyPressed(MENU_ITEM* item, BUTTON_VALUE keyPressed);

    ///@brief
    ///@param
    ///@return
    void TimeHighlight(MENU_ITEM* item);

#ifdef __cplusplus
}
#endif
#endif