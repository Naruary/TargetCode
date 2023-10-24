/*******************************************************************************
*       @brief      This file contains the implementation for the Main
*                   tab on the Uphole LCD screen.
*       @file       Uphole/src/UI_Tabs/logging/UI_MainTab.c
*       @date       November 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <string.h>
#include <stdio.h>
#include "ModemDriver.h"
#include "RecordManager.h"
#include "FlashMemory.h"
#include "SysTick.h"
#include "UI_Alphabet.h"
#include "UI_ScreenUtilities.h"
#include "UI_Frame.h"
#include "UI_LCDScreenInversion.h"
#include "UI_api.h"
#include "MWD_LoggingPanel.h"
#include "LoggingManager.h"
#include "UploadingPanel.h"
#include "WaitingForSurveyPanel.h"
#include "SurveyTimeoutPanel.h"
#include "UI_SurveyEditPanel.h"
#include "textStrings.h"
#include "UI_StartNewHoleDecisionPanel.h"
#include "UI_ClearAllHoleDecisionPanel.h"
#include "UI_UpdateDiagnosticDownholeDecisionPanel.h"
#include "UpdateDownholeTimeoutPanel.h"
#include "UpdateDownholeSuccessPanel.h"
#include "TakeSurveySuccessPanel.h"
#include "ClearAllHoleSuccessPanel.h"
#include "StartNewHoleSuccessPanel.h"
#include "Compass_Panel.h"
#include "UI_ChangePipeLengthDecisionPanel.h"
#include "UI_EnterNewPipeLength.h"
#include "UI_ChangePipeLengthCorrectDecisionPanel.h"
#include "UI_EnterSurveyDecisionPanel.h"
#include "UI_EnterSurvey.h"

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

static MENU_ITEM* GetMainMenuItem(TAB_ENTRY* tab, U_BYTE index);
static U_BYTE GetMainMenuSize(TAB_ENTRY* tab);
static void MainTabShow(TAB_ENTRY* tab);
static void MainTabPaint(TAB_ENTRY* tab);
static void TimerElapsed(TAB_ENTRY* tab);
static void MainTabKeyPressed(TAB_ENTRY* tab, BUTTON_VALUE key);
//void ShowDownholeLife(char* message);

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

const TAB_ENTRY MainTab = {
	&TabFrame1,
	TXT_MAIN,
	ShowTab,
	GetMainMenuItem,
	GetMainMenuSize,
	MainTabPaint,
	MainTabShow,
	TimerElapsed,
	MainTabKeyPressed
};

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*******************************************************************************
*       @details
*******************************************************************************/
static PANEL* CurrentState(void)
{
	if(getStartNewHoleDecisionPanelActive())
	{
		return &StartNewHole_DecisionPanel;
	}
	else if(getClearAllHoleDataDecisionPanelActive())
	{
		return &ClearAllHoleData_DecisionPanel;
	}
//	else if(getUpdateDiagnosticDownholeDecisionPanelActive())
//	{
//		return &UpdateDiagnosticDownhole_DecisionPanel;
//	}
	else if(getCompassDecisionPanelActive())
	{
		return &Compass_DecisionPanel;
	}
	else if(getChangePipeLengthDecisionPanelActive())
	{
		return &ChangePipeLength_DecisionPanel;
	}
	else if(getEnterSurveyDecisionPanelActive())
	{
		return &EnterSurvey_DecisionPanel;
	}
	else if(getEnterSurveyPanelActive())
	{
		return &EnterSurvey_Panel;
	}
	else if(getEnterNewPipeLengthPanelActive())
	{
		return &EnterNewPipeLength_Panel;
	}
	else if(getChangePipeLengthCorrectDecisionPanelActive())
	{
		return &ChangePipeLengthCorrect_DecisionPanel;
	}
	else
	{
		switch(NVRAM_data.loggingState)
		{
			case WAITING_FOR_SURVEY:
				return &WaitingPanel;
			case SURVEY_REQUEST_TIMEOUT:
				return &SurveyTimeout;
			case START_NEW_HOLE:
				return &WaitingPanel;
			case CLEAR_ALL_HOLE:
				return &WaitingPanel;
			case UPDATE_DOWNHOLE:
				return &WaitingPanel;
			case UPDATE_DOWNHOLE_FAILED:
				return &UpdateDownholeTimeout;
			case WAKEUP_DOWNHOLE:
				return &WaitingPanel;
			case UPDATE_DOWNHOLE_SUCCESS:
				return &UpdateDownholeSuccess;
			case WAITING_TO_WAKEUP_DOWNHOLE:
				return &WaitingPanel;
			case SURVEY_REQUEST_SUCCESS:
				return &TakeSurveySuccess;
			case CLEAR_ALL_HOLE_SUCCESS:
				return &ClearAllHoleSuccessPanel;
			case START_NEW_HOLE_SUCCESS:
				return &StartNewHoleSuccessPanel;
			case COMPASS_LOGGING:
				return &Compass_DecisionPanel;
		}
	}
	return &MWDLogging;
}

/*******************************************************************************
*       @details
*******************************************************************************/
static MENU_ITEM* GetMainMenuItem(TAB_ENTRY* tab, U_BYTE index)
{
	PANEL *apanel;
	if(index < tab->MenuSize(tab))
	{
		apanel = CurrentState();
		if(apanel == NULL) return NULL;
		return CurrentState()->MenuItem(index);
	}
	return NULL;
}

/*******************************************************************************
*       @details
*******************************************************************************/
static U_BYTE GetMainMenuSize(TAB_ENTRY* tab)
{
	return CurrentState()->MenuCount;
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void MainTabPaint(TAB_ENTRY* tab)
{
	CurrentState()->Paint(tab);
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void TimerElapsed(TAB_ENTRY* tab)
{
	if(CurrentState()->TimerElapsed)
	{
		CurrentState()->TimerElapsed(tab);
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void MainTabShow(TAB_ENTRY* tab)
{
	if(CurrentState()->Show)
	{
		CurrentState()->Show(tab);
	}
	PaintNow(&HomeFrame);
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void MainTabKeyPressed(TAB_ENTRY* tab, BUTTON_VALUE key)
{
	if(CurrentState()->KeyPressed)
	{
		CurrentState()->KeyPressed(tab, key);
	}
}

// used by PANELS attached to this TAB
/*******************************************************************************
*       @details
*******************************************************************************/
void ShowStatusMessage(char* message)
{
	RECT area;
	const FRAME* frame = &WindowFrame;
	area.ptTopLeft.nCol = frame->area.ptTopLeft.nCol;// + 5;
	area.ptTopLeft.nRow = frame->area.ptBottomRight.nRow - 18;
	area.ptBottomRight.nCol = frame->area.ptBottomRight.nCol;// - 5;
	area.ptBottomRight.nRow = area.ptTopLeft.nRow + 15;
	UI_DisplayStringCentered(message, &area);
}

// used by PANELS attached to this TAB
/*******************************************************************************
*       @details
*******************************************************************************/
void ShowOperationStatusMessage(char* message)
{
	RECT area;
	const FRAME* frame = &WindowFrame;
	area.ptTopLeft.nCol = frame->area.ptTopLeft.nCol + 5;
	area.ptTopLeft.nRow = frame->area.ptBottomRight.nRow - 100;
	area.ptBottomRight.nCol = frame->area.ptBottomRight.nCol - 5;
	area.ptBottomRight.nRow = area.ptTopLeft.nRow + 15;

	GLCD_Line(15, 100, 305, 100);
	GLCD_Line(16, 101, 304, 101);
	GLCD_Line(17, 102, 303, 102);
	GLCD_Line(18, 103, 302, 103);
	GLCD_Line(19, 104, 301, 104);

	GLCD_Line(15, 150, 305, 150);
	GLCD_Line(16, 149, 304, 149);
	GLCD_Line(17, 148, 303, 148);
	GLCD_Line(18, 147, 302, 147);
	GLCD_Line(19, 146, 301, 146);

	GLCD_Line(15, 100, 15, 150);
	GLCD_Line(16, 100, 16, 150);
	GLCD_Line(17, 100, 17, 150);
	GLCD_Line(18, 100, 18, 150);
	GLCD_Line(19, 100, 19, 150);

	GLCD_Line(305, 100, 305, 150);
	GLCD_Line(304, 100, 304, 150);
	GLCD_Line(303, 100, 303, 150);
	GLCD_Line(302, 100, 302, 150);
	GLCD_Line(301, 100, 301, 150);

	UI_DisplayStringCentered(message, &area);
}

/*******************************************************************************
*       @details
*******************************************************************************/
//REAL32 RealValue(INT16 value)
//{
//	return (REAL32) (value / 10.);
//}

// used by PANELS attached to this TAB
/*******************************************************************************
*       @details
*******************************************************************************/
MENU_ITEM* GetEmptyMenu(U_BYTE index)
{
	return NULL;
}

/*******************************************************************************
*       @details
*******************************************************************************/
#if 0
void ShowNumberOfHoleMessage(char* message)
{
	RECT area;
	const FRAME* frame = &WindowFrame;
	area.ptTopLeft.nCol = frame->area.ptTopLeft.nCol + 5;
	area.ptTopLeft.nRow = frame->area.ptBottomRight.nRow - 32;
	area.ptBottomRight.nCol = frame->area.ptBottomRight.nCol - 5;
	area.ptBottomRight.nRow = area.ptTopLeft.nRow + 15;
	UI_DisplayStringCentered(message, &area);
}
#endif 
///1
void ShowArmedStatusMessage(char* message)
{
	RECT area;
	const FRAME* frame = &WindowFrame;
	area.ptTopLeft.nCol = frame->area.ptTopLeft.nCol;// + 5;
	area.ptTopLeft.nRow = frame->area.ptBottomRight.nRow - 114;
	area.ptBottomRight.nCol = frame->area.ptBottomRight.nCol - 75;// - 5;
	area.ptBottomRight.nRow = area.ptTopLeft.nRow + 17;
	UI_DisplayStringCentered(message, &area);
}
///1