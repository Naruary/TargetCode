/*******************************************************************************
*       @brief      This header file contains callable functions and public
*                   data for the Setup tab on the Uphole box.
*       @file       Uphole/inc/UI_Tabs/UI_BoxSetupTab.h
*       @date       July 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef UI_SETUP_TAB_H
#define UI_SETUP_TAB_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"
#include "UI_ScreenUtilities.h"

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

extern const TAB_ENTRY BoxSetupTab;

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

	void SetKeyBeeperAvailable(BOOL bState);
	BOOL GetKeyBeeperAvailable(void);
	void SetBacklightOnTime(INT16 OnTime);
	INT16 GetBacklightOnTime(void);
	void SetLCDOnTime(INT16 OnTime);
	INT16 GetLCDOnTime(void);
//	void SetCheckShot(BOOL value);
//	BOOL GetCheckShot(void);
	void SetErrorCorrect(BOOL OnTime);
	BOOL GetErrCorrect(void);

#ifdef __cplusplus
}
#endif

#endif // UI_SETUP_TAB_H
