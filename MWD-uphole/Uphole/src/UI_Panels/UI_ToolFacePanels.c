/*******************************************************************************
*       @brief      This file contains the implementation for the
*                   Tool Face zero panels and routines
*       @file       Uphole/inc/UI_Panels/UI_ToolFacedPanels.c
*       @date       January 2019
*       @copyright  COPYRIGHT (c) 2019 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "UI_Alphabet.h"
#include "UI_api.h"
#include "UI_DataTab.h"
#include "Manager_DataLink.h"
#include "UI_ToolFacePanels.h"
#include "SysTick.h"
#include "FlashMemory.h"
#include "PCDataTransfer.h"

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

static void ToolFaceZero_Decision_Paint(TAB_ENTRY* tab);
static void ToolFaceZero_Decision_Show(TAB_ENTRY* tab);
static void ToolFaceZero_Decision_KeyPressed(TAB_ENTRY* tab, BUTTON_VALUE key);
static void ToolFaceZero_Decision_TimerElapsed(TAB_ENTRY* tab);
static MENU_ITEM* setToolFaceZero_Decision_Menu(U_BYTE index);
static void Yes_setToolFaceZero_Decision(MENU_ITEM* item);
static void No_setToolFaceZero_Decision(MENU_ITEM* item);
void ShowToolFaceZeroDecisionMessage(char* message);
//void ShowClearHoleInfoMessage(char* message1, char* message2, char* message3);

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

static MENU_ITEM setToolFaceZero_DecisionMenu[] =
{
    CREATE_MENU_ITEM(TXT_YES,    &LabelFrame1, Yes_setToolFaceZero_Decision),
    CREATE_MENU_ITEM(TXT_NO,     &LabelFrame2, No_setToolFaceZero_Decision)
};

BOOL ToolFaceZero_DecisionPanelActive = false;
static TIME_LR tToolFaceZero = 0;
PANEL ToolFaceZero_DecisionPanel = {setToolFaceZero_Decision_Menu, sizeof(setToolFaceZero_DecisionMenu) / sizeof(MENU_ITEM), ToolFaceZero_Decision_Paint, ToolFaceZero_Decision_Show, ToolFaceZero_Decision_KeyPressed, ToolFaceZero_Decision_TimerElapsed};

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*******************************************************************************
*       @details
*******************************************************************************/
void setToolFaceZeroDecisionPanelActive(BOOL bFlag)
{
	ToolFaceZero_DecisionPanelActive = bFlag;
}

/*******************************************************************************
*       @details
*******************************************************************************/
BOOL getToolFaceZeroDecisionPanelActive(void)
{
	return ToolFaceZero_DecisionPanelActive;
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void ToolFaceZero_Decision_Paint(TAB_ENTRY* tab)
{
	ToolFaceZero_Decision_Show(tab);
	TabWindowPaint(tab);
//	GroupBoxPaint(&surveyGroup);
	ShowToolFaceZeroDecisionMessage("Are you sure. Set Tool Face Reference?");
//	ShowClearHoleInfoMessage("ERASE data from Memory PERMANENTLY.", "Remember: to SET correct JOB","values before taking any survey.");
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void ToolFaceZero_Decision_Show(TAB_ENTRY* tab)
{
	MENU_ITEM* item = tab->MenuItem(tab, 0);
	UI_SetActiveFrame(item->labelFrame);
	SetActiveLabelFrame(item->labelFrame->eID);
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void ToolFaceZero_Decision_KeyPressed(TAB_ENTRY* tab, BUTTON_VALUE key)
{
	switch(key)
	{
		case BUTTON_DASH:
		{
			setToolFaceZeroDecisionPanelActive(false);
			RepaintNow(&WindowFrame);
			break;
		}
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void ToolFaceZero_Decision_TimerElapsed(TAB_ENTRY* tab)
{
//	RepaintNow(&WindowFrame);
}

/*******************************************************************************
*       @details
*******************************************************************************/
static MENU_ITEM* setToolFaceZero_Decision_Menu(U_BYTE index)
{
	return &setToolFaceZero_DecisionMenu[index];
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void Yes_setToolFaceZero_Decision(MENU_ITEM* item)
{
	// put your logic here
	setToolFaceZeroDecisionPanelActive(false);
//	LoggingManager_StartLogging();
//	NVRAM_data.loggingState = CLEAR_ALL_HOLE;
	tToolFaceZero = ElapsedTimeLowRes(0);
	RepaintNow(&WindowFrame);
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void No_setToolFaceZero_Decision(MENU_ITEM* item)
{
	// put your logic here
	//RECORD_InitBranchParam();
	setToolFaceZeroDecisionPanelActive(false);
//	NVRAM_data.loggingState = LOGGING;
	RepaintNow(&WindowFrame);
	SetActiveLabelFrame(LABEL5);
}

/*******************************************************************************
*       @details
*******************************************************************************/
TIME_LR GetToolFaceZeroTimer(void)
{
	return tToolFaceZero;
}

/*******************************************************************************
*       @details
*******************************************************************************/
void SetToolFaceZeroFinalValue(MENU_ITEM* item)
{
	// sec param is subtracted from actual for working value.
	// so capture current value of roll so that it zeros
  	RepaintNow(&WindowFrame);
	GrabToolfaceCompensation();
}
//whs 14Feb2022 removed ...nobody uses it
//void DownloadToUSB(MENU_ITEM* item)
//{
//	//USBDownloadFlag = 1;
//	PCPORT_ReceiveDataUSB();
//}

/*******************************************************************************
*       @details
*******************************************************************************/
INT16 GetToolFaceValue(void)
{
	// sec param is subtracted from actual for working value.
	// so capture current value of roll so that it zeros
	return GetToolfaceCompensation();
}

/*******************************************************************************
*       @details
*******************************************************************************/
void ClearToolFaceZero(MENU_ITEM* item)
{
	// sec param is subtracted from actual for working value,
	// so clear the offset by setting it to 0
	RepaintNow(&WindowFrame);
	ClearToolfaceCompensation();
}

/*******************************************************************************
*       @details
*******************************************************************************/
void ShowToolFaceZeroDecisionMessage(char* message)
{
	RECT area;
	const FRAME* frame = &WindowFrame;
	area.ptTopLeft.nCol = frame->area.ptTopLeft.nCol + 5;
	area.ptTopLeft.nRow = frame->area.ptBottomRight.nRow - 70;
	area.ptBottomRight.nCol = frame->area.ptBottomRight.nCol - 5;
	area.ptBottomRight.nRow = area.ptTopLeft.nRow + 15;
	UI_DisplayStringCentered(message, &area);
}

INT16 GetGTFMain(void)
{
	INT16 GTFMain;
	INT16 Toolface;
	if ((float)GetToolFaceValue() > 3600)
	{
		Toolface = GetToolFaceValue() - 3600;
	}
	else
	{
		Toolface = GetToolFaceValue();
	}
	
	GTFMain = Toolface + GetSurveyRoll();
	
	if (GTFMain >= 3600)
	{
		GTFMain -= 3600;
	}
	return GTFMain;
}
