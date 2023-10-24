/*******************************************************************************
*       @brief      This header file contains callable functions and public
*                   data for the MWD Logging Panel.
*       @file       Uphole/inc/UI_Panels/MWD_LoggingPanel.h
*       @date       November 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef MWD_LOGGING_PANEL_H
#define MWD_LOGGING_PANEL_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "UI_MainTab.h"
#include "keypad.h" //MB

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

extern PANEL MWDLogging;

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

	TIME_LR GetDecisionTimer(void);
	TIME_LR GetWaitingForDownholeTimer(void);
	void ClearDownholeStatusTimer(void);
	void SetDownholeOnStatus(BOOL Status);
	void SetDownholeOffStatus(BOOL Status);
   //   void KeyPushed(BUTTON_VALUE key); //MB 6/25/21

#ifdef __cplusplus
}
#endif
#endif
