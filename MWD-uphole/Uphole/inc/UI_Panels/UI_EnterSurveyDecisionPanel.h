/*******************************************************************************
*       @brief      This header file contains callable functions and public
*                   data for the Survey Edit Panel under the Data tab.
*       @file       Uphole/inc/UI_Panels/UI_ChangePipeLengthDecisionPanel.h
*       @date       November 2015
*       @copyright  COPYRIGHT (c) 2015 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef UI_ENTERSURVEY_DECISION_PANEL_H
#define UI_ENTERSURVEY_DECISION_PANEL_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "UI_MainTab.h"

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

extern PANEL EnterSurvey_DecisionPanel;

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

	void setEnterSurveyDecisionPanelActive(BOOL bFlag);
	BOOL getEnterSurveyDecisionPanelActive(void);
	TIME_LR GetEnterSurveyTimer(void);

#ifdef __cplusplus
}
#endif
#endif
