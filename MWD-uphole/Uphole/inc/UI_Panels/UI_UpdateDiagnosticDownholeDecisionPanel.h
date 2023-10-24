/*******************************************************************************
*       @brief      This header file contains callable functions and public
*                   data for the Survey Edit Panel under the Data tab.
*       @file       Uphole/inc/UI_Panels/UI_UpdateDiagnosticDownholeDecisionPanel.h
*       @date       January 2016
*       @copyright  COPYRIGHT (c) 2016 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef UI_UPDATE_DOWNHOLE_DECISION_PANEL_H
#define UI_UPDATE_DOWNHOLE_DECISION_PANEL_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "UI_MainTab.h"

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

extern PANEL UpdateDiagnosticDownhole_DecisionPanel;

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

	void setUpdateDiagnosticDownholeDecisionPanelActive(BOOL bFlag);
	BOOL getUpdateDiagnosticDownholeDecisionPanelActive(void);
	TIME_LR GetUpdateDownHoleRequestTimer(void);

#ifdef __cplusplus
}
#endif
#endif
