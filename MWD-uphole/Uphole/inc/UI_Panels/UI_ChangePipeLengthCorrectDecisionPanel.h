/*******************************************************************************
*       @brief      This header file contains callable functions and public
*                   data for the Change Pipe Length under the Main tab.
*       @file       Uphole/inc/UI_Panels/UI_ChangePipeLengthCorrectDecisionPanel.h
*       @date       November 2015
*       @copyright  COPYRIGHT (c) 2015 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef UI_CHANGEPIPELENGTHCORRECT_DECISION_PANEL_H
#define UI_CHANGEPIPELENGTHCORRECT_DECISION_PANEL_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "UI_MainTab.h"

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

extern PANEL ChangePipeLengthCorrect_DecisionPanel;

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

	void setChangePipeLengthCorrectDecisionPanelActive(BOOL bFlag);
	BOOL getChangePipeLengthCorrectDecisionPanelActive(void);
	TIME_LR GetChangePipeLengthCorrectTimer(void);
	BOOL GetChangePipeLengthFlag(void);
	void SetChangePipeLengthFlag(BOOL);

#ifdef __cplusplus
}
#endif
#endif
