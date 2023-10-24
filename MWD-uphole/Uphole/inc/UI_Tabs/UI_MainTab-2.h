/*!
********************************************************************************
*       @brief      This header file contains callable functions and public
*                   data for the Main tab on the Uphole box.
*       @file       Uphole/inc/UI_Tabs/UI_MainTab.h
*       @author     Josh Masters
*       @author     Chris Walker
*       @date       November 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef UI_MAIN_TAB_H
#define UI_MAIN_TAB_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"
#include "RecordManager.h"
#include "UI_ScreenUtilities.h"

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

typedef struct _PANEL
{
    MENU_ITEM* (*MenuItem)(U_BYTE index);
    U_BYTE MenuCount;
    void (*Paint)(TAB_ENTRY* tab);
    void (*Show)(TAB_ENTRY* tab);
    void (*KeyPressed)(TAB_ENTRY* tab, BUTTON_VALUE key);
    void (*TimerElapsed)(TAB_ENTRY* tab);
} PANEL;


//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

extern const TAB_ENTRY MainTab;

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif
    
    ///@brief  
    ///@param  
    ///@return 
    MENU_ITEM* GetEmptyMenu(U_BYTE index);
    
    ///@brief  
    ///@param  
    ///@return 
    void ShowStatusMessage(char* message);
    
    ///@brief  
    ///@param  
    ///@return 
    REAL32 RealValue(INT16 value);
    
    ///@brief  
    ///@param  
    ///@return 
    void ShowNumberOfHoleMessage(char* message);
    
    ///@brief  
    ///@param  
    ///@return 
    void ShowOperationStatusMessage(char* message);

#ifdef __cplusplus
}
#endif
#endif