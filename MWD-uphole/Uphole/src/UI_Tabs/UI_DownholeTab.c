/*******************************************************************************
*       @brief      This file contains the implementation for the Diag
*                   tab on the Uphole LCD screen.
*       @file       Uphole/src/UI_Tabs/UI_DownholeTab.c
*       @date       January 2016
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <stdio.h>
#include <stdlib.h>
#include "adc.h"
#include "Manager_DataLink.h"
#include "TextStrings.h"
#include "UI_Alphabet.h"
#include "UI_ScreenUtilities.h"
#include "UI_LCDScreenInversion.h"
#include "UI_Frame.h"
#include "UI_api.h"
#include "UI_BooleanField.h"
#include "UI_YesNoField.h"
#include "UI_FixedField.h"
#include "UI_StringField.h"
#include "FlashMemory.h"
#include "RecordManager.h"
#include "UI_UpdateDiagnosticDownholeDecisionPanel.h"
#include "UI_DownholeTab.h"
#include "UI_DownholeMainPanel.h"
#include "DownholeBatteryAndLife.h"
#include "TargetProtocol.h"
#include "GammaSensor.h"
#include "version.h"

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

static PANEL *CurrentState(void);
static MENU_ITEM *GetMenuItem(TAB_ENTRY* tab, U_BYTE index);
static U_BYTE GetMenuSize(TAB_ENTRY* tab);
static void TablePaint(TAB_ENTRY* tab);
static void TableShow(TAB_ENTRY* tab);
static void TableTimeElapsed(TAB_ENTRY* tab);
static void TableKeyPressed(TAB_ENTRY* tab, BUTTON_VALUE key);
void ShowStatusMessageTabDiag(char* message1, char* message2);

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

enum { UI_DOWNHOLE_MAIN, };

U_INT16 panelIndex = UI_DOWNHOLE_MAIN;

const TAB_ENTRY DownholeTab = {
	&TabFrame5,
	TXT_DIAG,
	ShowTab,
	GetMenuItem,
	GetMenuSize,
	TablePaint,
	TableShow,
	TableTimeElapsed,
	TableKeyPressed
};



//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//
// will move to panel
#define MENU_SIZE (sizeof(menu) / sizeof(MENU_ITEM))

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*******************************************************************************
*       @details
*******************************************************************************/
static PANEL *CurrentState(void)
{
	switch(panelIndex)
	{
		case UI_DOWNHOLE_MAIN:
			return &DownholePanel;
			break;
	}
	return NULL;
#if 0
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
		// can do main screen stuff here if necessary
	}
	return &DownholePanel;
#endif
}

/*******************************************************************************
*       @details
*******************************************************************************/
static MENU_ITEM* GetMenuItem(TAB_ENTRY* tab, U_BYTE index)
{
#if 0
    if (index < tab->MenuSize(tab))
    {
        return &menu[index];
    }
    return NULL;
#else
	PANEL *apanel;
	if(index < tab->MenuSize(tab))
	{
		apanel = CurrentState();
		if(apanel == NULL) return NULL;
		return CurrentState()->MenuItem(index);
	}
	return NULL;
#endif
}

/*******************************************************************************
*       @details
*******************************************************************************/
static U_BYTE GetMenuSize(TAB_ENTRY* tab)
{
#if 0
    return MENU_SIZE;
#else
	return CurrentState()->MenuCount;
#endif
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void TablePaint(TAB_ENTRY* tab)
{
#if 0
	char text[100];
	TabWindowPaint(tab);
	U_BYTE nMenuCount = tab->MenuSize(tab);
//	snprintf(text, 100, "Downhole Voltage          %.1f", (float)GetDownholeBatteryVoltage()/1000);
//	ShowDownholeVoltageTabDiag(text, (nMenuCount * 15)+4 );
//      15Oct2019 whs Removing the above two lines does not !!!!!!! remove the message from the screen
	// convert seconds of run time to hours
//	snprintf(text, 100, "Downhole Run Time         %lu", GetDownholeTotalOnTime()/3600ul);
//      15Oct2019 removing the previous line does not remove the message from the DOW screen
	ShowDownholeVoltageTabDiag(text, ((nMenuCount+1) * 15)+4 );
	snprintf(text, 100, "Downhole Software Version: %s", GetDownholeSWVersion());
	ShowDownholeVoltageTabDiag(text, ((nMenuCount+2) * 15)+4 );
	snprintf(text, 100, "Downhole Software Build Date: %s", GetDownholeSWDate());
	ShowDownholeVoltageTabDiag(text, ((nMenuCount+3) * 15)+4 );
#else
	CurrentState()->Paint(tab);
#endif
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void TableShow(TAB_ENTRY* tab)
{
#if 0
    UI_SetActiveFrame(&LabelFrame1);
    SetActiveLabelFrame(LABEL1);
    PaintNow(&HomeFrame);
#else
	if(CurrentState()->Show)
	{
		CurrentState()->Show(tab);
	}
	PaintNow(&HomeFrame);
#endif
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void TableTimeElapsed(TAB_ENTRY* tab)
{
#if 0
    MENU_ITEM* time = &menu[0];
    if ((!time->editing) && (UI_GetActiveFrame()->eID != ALERT_FRAME))
    {
        RepaintNow(time->valueFrame);
    }
#else
	if(CurrentState()->TimerElapsed)
	{
		CurrentState()->TimerElapsed(tab);
	}
#endif
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void TableKeyPressed(TAB_ENTRY* tab, BUTTON_VALUE key)
{
#if 0
	// original non panelized did nothing here
#else
	if(CurrentState()->KeyPressed)
	{
		CurrentState()->KeyPressed(tab, key);
	}
#endif
}

/*******************************************************************************
*       @details
*******************************************************************************/
void ShowStatusMessageTabDiag(char* message1, char* message2)
{
    RECT area;
    const FRAME* frame = &WindowFrame;
    area.ptTopLeft.nCol = frame->area.ptTopLeft.nCol + 5;
    area.ptTopLeft.nRow = frame->area.ptBottomRight.nRow - 80;
    area.ptBottomRight.nCol = frame->area.ptBottomRight.nCol - 5;
    area.ptBottomRight.nRow = area.ptTopLeft.nRow + 15;
    UI_DisplayStringCentered(message1, &area);
    area.ptTopLeft.nRow = frame->area.ptBottomRight.nRow - 50;
    UI_DisplayStringCentered(message2, &area);
}

/*******************************************************************************
*       @details
*******************************************************************************/
