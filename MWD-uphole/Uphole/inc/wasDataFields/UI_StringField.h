/*!
********************************************************************************
*       @brief      Header File for UI_StringField.c.
*                   Documented by Josh Masters Dec. 2014
*       @file       Uphole/inc/DataFields/UI_StringField.h
*       @author     Chris Walker
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef UI_STRING_FIELD_H
#define UI_STRING_FIELD_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"
#include "TextStrings.h"
#include "UI_ScreenUtilities.h"

//============================================================================//
//      MACROS                                                                //
//============================================================================//

#define CREATE_STRING_FIELD(Label, LabelFrame, ValueFrame, NextFrame, Getter, Setter) \
    {Label, LabelFrame, EditValue, ValueFrame, NextFrame, StringDisplay, StringBeginEdit, StringFinishEdit, StringKeyPressed, StringHighlight, .string = {Getter, Setter} }

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
    void StringDisplay(MENU_ITEM* item);
    
    ///@brief  
    ///@param  
    ///@return
    void StringBeginEdit(MENU_ITEM* item);
    
    ///@brief  
    ///@param  
    ///@return 
    void StringFinishEdit(MENU_ITEM* item);
    
    ///@brief  
    ///@param  
    ///@return 
    void StringKeyPressed(MENU_ITEM* item, BUTTON_VALUE keyPressed);
    
    ///@brief  
    ///@param  
    ///@return 
    void StringHighlight(MENU_ITEM* item);

#ifdef __cplusplus
}
#endif
#endif