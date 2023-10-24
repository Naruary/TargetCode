/*******************************************************************************
*       @brief      This header file contains callable functions and public
*                   data for the Survey Edit Panel under the Data tab.
*       @file       Uphole/inc/UI_Panels/UI_SurveyEditPanel.h
*       @date       August 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef UI_SURVEY_EDIT_PANEL_H
#define UI_SURVEY_EDIT_PANEL_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "UI_DataTab.h"

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

extern PANEL SurveyEditPanel;

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

	void setSurveyEditPanelActive(BOOL bFlag);
	BOOL getSurveyEditPanelActive(void);

#ifdef __cplusplus
}
#endif
#endif
