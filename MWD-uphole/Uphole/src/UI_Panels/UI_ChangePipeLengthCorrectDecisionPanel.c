/*******************************************************************************
*       @brief      This file contains the implementation for the Change Pipe Length
*                   Decision Panel on the Main tab. That gives a option to select Yes/No
*       @file       Uphole/inc/UI_Panels/UI_ChangePipeLengthCorrectDecisionPanel.h
*       @date       May 2016
*       @copyright  COPYRIGHT (c) 2015 Target Drilling Inc. All rights are
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
#include "LoggingManager.h"
#include "RecordManager.h"
#include "UI_Alphabet.h"
#include "UI_ScreenUtilities.h"
#include "UI_Frame.h"
#include "UI_LCDScreenInversion.h"
#include "UI_api.h"
#include "UI_ChangePipeLengthCorrectDecisionPanel.h"
#include "UI_RecordDataPanel.h"
#include "UI_DataTab.h"
#include "UI_GroupBox.h"
#include "SysTick.h"
#include "UI_EnterNewPipeLength.h"

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

static void ChangePipeLengthCorrect_Decision_Paint(TAB_ENTRY* tab);
static void ChangePipeLengthCorrect_Decision_Show(TAB_ENTRY* tab);
static void ChangePipeLengthCorrect_Decision_KeyPressed(TAB_ENTRY* tab, BUTTON_VALUE key);
static void ChangePipeLengthCorrect_Decision_TimerElapsed(TAB_ENTRY* tab);
static MENU_ITEM* ChangePipeLengthCorrect_Decision_Menu(U_BYTE index);
static void Yes_ChangePipeLengthCorrect_Decision(MENU_ITEM* item);
static void No_ChangePipeLengthCorrect_Decision(MENU_ITEM* item);
void ShowChangePipeLengthCorrectDecisionMessage(char* message);
void ShowChangePipeLengthCorrectInfoMessage(char* message1, char* message2, char* message3);

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

static MENU_ITEM ChangePipeLengthCorrect_DecisionMenu[] =
{
    CREATE_MENU_ITEM(TXT_YES,    &LabelFrame1, Yes_ChangePipeLengthCorrect_Decision),
    CREATE_MENU_ITEM(TXT_NO,     &LabelFrame2, No_ChangePipeLengthCorrect_Decision)
};

BOOL ChangePipeLengthCorrect_DecisionPanelActive = false;

static BOOL ChangePipeLengthFlag = false;

static TIME_LR tCallChangePipeLengthCorrect = 0;

PANEL ChangePipeLengthCorrect_DecisionPanel = {ChangePipeLengthCorrect_Decision_Menu, sizeof(ChangePipeLengthCorrect_DecisionMenu) / sizeof(MENU_ITEM), ChangePipeLengthCorrect_Decision_Paint, ChangePipeLengthCorrect_Decision_Show, ChangePipeLengthCorrect_Decision_KeyPressed, ChangePipeLengthCorrect_Decision_TimerElapsed};

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*******************************************************************************
*       @details
*******************************************************************************/
void setChangePipeLengthCorrectDecisionPanelActive(BOOL bFlag)
{
    ChangePipeLengthCorrect_DecisionPanelActive = bFlag;
}

/*******************************************************************************
*       @details
*******************************************************************************/
BOOL getChangePipeLengthCorrectDecisionPanelActive(void)
{
    return ChangePipeLengthCorrect_DecisionPanelActive;
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void ChangePipeLengthCorrect_Decision_Paint(TAB_ENTRY* tab)
{
	char stringValue[40];
	ChangePipeLengthCorrect_Decision_Show(tab);
	TabWindowPaint(tab);
	//GroupBoxPaint(&surveyGroup);
	snprintf(stringValue, 40, "%s %d%s", "Pipe Length =", GetNewPipeLength(), ", Are you sure?");
	ShowChangePipeLengthCorrectDecisionMessage(stringValue);
	ShowChangePipeLengthCorrectInfoMessage("New Length in effect for one survey.", "Remember: DO NOT RESTART Uphole box","until the next survey is taken.");
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void ChangePipeLengthCorrect_Decision_Show(TAB_ENTRY* tab)
{
	MENU_ITEM* item = tab->MenuItem(tab, 0);
	UI_SetActiveFrame(item->labelFrame);
	SetActiveLabelFrame(item->labelFrame->eID);
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void ChangePipeLengthCorrect_Decision_KeyPressed(TAB_ENTRY* tab, BUTTON_VALUE key)
{
	switch(key)
	{
		case BUTTON_DASH:
			setChangePipeLengthCorrectDecisionPanelActive(false);
			RepaintNow(&WindowFrame);
			break;
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void ChangePipeLengthCorrect_Decision_TimerElapsed(TAB_ENTRY* tab)
{
//	RepaintNow(&WindowFrame);
}

/*******************************************************************************
*       @details
*******************************************************************************/
static MENU_ITEM* ChangePipeLengthCorrect_Decision_Menu(U_BYTE index)
{
    return &ChangePipeLengthCorrect_DecisionMenu[index];
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void Yes_ChangePipeLengthCorrect_Decision(MENU_ITEM* item)
{
      if(ManualChangePipeLengthFlag)
      {
	setChangePipeLengthCorrectDecisionPanelActive(false);
	ChangePipeLengthFlag = true;
        
        setEnterSurveyPanelActive(true);
        ManualChangePipeLengthFlag = false;
	//setEnterNewPipeLengthPanelActive(true);
	//RECORD_InitNewHole();
	//SetLoggingState(START_NEW_HOLE);
	//tCallChangePipeLengthCorrect = ElapsedTimeLowRes(0);
	RepaintNow(&WindowFrame);
      }
      else
      {
	setChangePipeLengthCorrectDecisionPanelActive(false);
	ChangePipeLengthFlag = true; 
        RepaintNow(&WindowFrame);
      }
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void No_ChangePipeLengthCorrect_Decision(MENU_ITEM* item)
{
	setChangePipeLengthCorrectDecisionPanelActive(false);
	setEnterNewPipeLengthPanelActive(true);
	RepaintNow(&WindowFrame);
	SetActiveLabelFrame(LABEL1);
}

/*******************************************************************************
*       @details
*******************************************************************************/
TIME_LR GetChangePipeLengthCorrectTimer(void)
{
	return tCallChangePipeLengthCorrect;
}

/*******************************************************************************
*       @details
*******************************************************************************/
void ShowChangePipeLengthCorrectDecisionMessage(char* message)
{
	RECT area;
	const FRAME* frame = &WindowFrame;
	area.ptTopLeft.nCol = frame->area.ptTopLeft.nCol + 5;
	area.ptTopLeft.nRow = frame->area.ptBottomRight.nRow - 70;
	area.ptBottomRight.nCol = frame->area.ptBottomRight.nCol - 5;
	area.ptBottomRight.nRow = area.ptTopLeft.nRow + 15;
	UI_DisplayStringCentered(message, &area);
}

/*******************************************************************************
*       @details
*******************************************************************************/
void ShowChangePipeLengthCorrectInfoMessage(char* message1, char* message2, char* message3)
{
	RECT area;
	const FRAME* frame = &WindowFrame;
	area.ptTopLeft.nCol = frame->area.ptTopLeft.nCol + 5;
	area.ptTopLeft.nRow = frame->area.ptBottomRight.nRow - 140;
	area.ptBottomRight.nCol = frame->area.ptBottomRight.nCol - 5;
	area.ptBottomRight.nRow = area.ptTopLeft.nRow + 15;
	UI_DisplayStringCentered(message1, &area);
	area.ptTopLeft.nRow = frame->area.ptBottomRight.nRow - 100;
	UI_DisplayStringCentered(message2, &area);
	area.ptTopLeft.nRow = frame->area.ptBottomRight.nRow - 60;
	UI_DisplayStringCentered(message3, &area);
}

/*******************************************************************************
*       @details
*******************************************************************************/
BOOL GetChangePipeLengthFlag(void)
{
	return ChangePipeLengthFlag;
}

/*******************************************************************************
*       @details
*******************************************************************************/
void SetChangePipeLengthFlag(BOOL Flag)
{
	ChangePipeLengthFlag = Flag;
}

