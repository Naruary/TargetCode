/*******************************************************************************
*       @brief      This header file contains callable functions and public
*                   data for the Main tab on the Uphole box.
*       @file       Uphole/inc/UI_Tabs/UI_MainTab.h
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

	MENU_ITEM* GetEmptyMenu(U_BYTE index);
	void ShowStatusMessage(char* message);
	void ShowOperationStatusMessage(char* message);
	void ShowArmedStatusMessage(char* message);

#ifdef __cplusplus
}
#endif
#endif
