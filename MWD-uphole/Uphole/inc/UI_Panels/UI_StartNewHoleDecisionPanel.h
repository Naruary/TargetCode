/*******************************************************************************
*       @brief      This header file contains callable functions and public
*                   data for the Survey Edit Panel under the Data tab.
*       @file       Uphole/inc/UI_Panels/UI_SartNewHoleDecisionPanel.h
*       @date       November 2015
*       @copyright  COPYRIGHT (c) 2015 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef UI_STARTNEWHOLE_DECISION_PANEL_H
#define UI_STARTNEWHOLE_DECISION_PANEL_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "UI_MainTab.h"

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

extern PANEL StartNewHole_DecisionPanel;

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

	void setStartNewHoleDecisionPanelActive(BOOL bFlag);
	BOOL getStartNewHoleDecisionPanelActive(void);
	TIME_LR GetStartNewHoleTimer(void);

#ifdef __cplusplus
}
#endif
#endif
