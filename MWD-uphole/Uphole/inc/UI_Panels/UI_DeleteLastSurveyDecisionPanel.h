/*******************************************************************************
*       @brief      This header file contains callable functions and public
*                   data for the Survey Edit Panel under the Data tab.
*       @file       Uphole/inc/UI_Panels/UI_SartNewHoleDecisionPanel.h
*       @date       December 2015
*       @copyright  COPYRIGHT (c) 2015 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef UI_DELETELASTSURVEY_DECISION_PANEL_H
#define UI_DELETELASTSURVEY_DECISION_PANEL_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "UI_DataTab.h"

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

extern PANEL DeleteLastSurvey_DecisionPanel;

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

	void setDeleteLastSurveyDecisionPanelActive(BOOL bFlag);
	BOOL getDeleteLastSurveyDecisionPanelActive(void);
	TIME_LR GetDeleteLastSurveyTimer(void);

#ifdef __cplusplus
}
#endif
#endif
