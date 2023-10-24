/*******************************************************************************
*       @brief      This file contains the implementation for the Start New Hole
*                   Decision Panel on the Main tab. That gives a option to select Yes/No
*       @file       Uphole/inc/UI_Panels/UI_ChangePipeLengthDecisionPanel.h
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
#include "FlashMemory.h"
#include "UI_Alphabet.h"
#include "UI_ScreenUtilities.h"
#include "UI_Frame.h"
#include "UI_LCDScreenInversion.h"
#include "UI_api.h"
#include "UI_ChangePipeLengthDecisionPanel.h"
#include "UI_RecordDataPanel.h"
#include "UI_DataTab.h"
#include "UI_GroupBox.h"
#include "SysTick.h"
#include "UI_EnterNewPipeLength.h"

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

static void ChangePipeLength_Decision_Paint(TAB_ENTRY* tab);
static void ChangePipeLength_Decision_Show(TAB_ENTRY* tab);
static void ChangePipeLength_Decision_KeyPressed(TAB_ENTRY* tab, BUTTON_VALUE key);
static void ChangePipeLength_Decision_TimerElapsed(TAB_ENTRY* tab);
static MENU_ITEM* ChangePipeLength_Decision_Menu(U_BYTE index);
static void Yes_ChangePipeLength_Decision(MENU_ITEM* item);
static void No_ChangePipeLength_Decision(MENU_ITEM* item);
void ShowChangePipeLengthDecisionMessage(char* message);
void ShowChangePipeLengthInfoMessage(char* message1, char* message2, char* message3);

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

static MENU_ITEM ChangePipeLength_DecisionMenu[] =
{
    CREATE_MENU_ITEM(TXT_YES,    &LabelFrame1, Yes_ChangePipeLength_Decision),
    CREATE_MENU_ITEM(TXT_NO,     &LabelFrame2, No_ChangePipeLength_Decision)
};

BOOL ChangePipeLength_DecisionPanelActive = false;

static TIME_LR tCallChangePipeLength = 0;

PANEL ChangePipeLength_DecisionPanel = {ChangePipeLength_Decision_Menu, sizeof(ChangePipeLength_DecisionMenu) / sizeof(MENU_ITEM), ChangePipeLength_Decision_Paint, ChangePipeLength_Decision_Show, ChangePipeLength_Decision_KeyPressed, ChangePipeLength_Decision_TimerElapsed};

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*!
********************************************************************************
*       @details
*******************************************************************************/

void setChangePipeLengthDecisionPanelActive(BOOL bFlag)
{
    ChangePipeLength_DecisionPanelActive = bFlag;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

BOOL getChangePipeLengthDecisionPanelActive(void)
{
    return ChangePipeLength_DecisionPanelActive;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void ChangePipeLength_Decision_Paint(TAB_ENTRY* tab)
{
    ChangePipeLength_Decision_Show(tab);
    TabWindowPaint(tab);
    //GroupBoxPaint(&surveyGroup);
    ShowChangePipeLengthDecisionMessage("Are you sure. Change Pipe Length?");
    ShowChangePipeLengthInfoMessage("Edit Pipe Length for next survey.", "Remember: DO NOT RESTART Uphole box","until the next survey is taken.");
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void ChangePipeLength_Decision_Show(TAB_ENTRY* tab)
{
    MENU_ITEM* item = tab->MenuItem(tab, 0);
    UI_SetActiveFrame(item->labelFrame);
    SetActiveLabelFrame(item->labelFrame->eID);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void ChangePipeLength_Decision_KeyPressed(TAB_ENTRY* tab, BUTTON_VALUE key)
{
    switch(key)
    {
        case BUTTON_DASH:
        {
            setChangePipeLengthDecisionPanelActive(false);
            RepaintNow(&WindowFrame);
            break;
        }
    }
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void ChangePipeLength_Decision_TimerElapsed(TAB_ENTRY* tab)
{
    //RepaintNow(&WindowFrame);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static MENU_ITEM* ChangePipeLength_Decision_Menu(U_BYTE index)
{
    return &ChangePipeLength_DecisionMenu[index];
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void Yes_ChangePipeLength_Decision(MENU_ITEM* item)
{
    setChangePipeLengthDecisionPanelActive(false);
    setEnterNewPipeLengthPanelActive(true);
    //RECORD_InitNewHole();
    //SetLoggingState(START_NEW_HOLE);
    //tCallChangePipeLength = ElapsedTimeLowRes(0);
    RepaintNow(&WindowFrame);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void No_ChangePipeLength_Decision(MENU_ITEM* item)
{
    if(ManualChangePipeLengthFlag)
    {
      setChangePipeLengthDecisionPanelActive(false);
      setEnterSurveyPanelActive(true);
      ManualChangePipeLengthFlag = false;
      RepaintNow(&WindowFrame);
    }
    else
    {
    //RECORD_InitBranchParam();
    setChangePipeLengthDecisionPanelActive(false);
    SetLoggingState(LOGGING);
    RepaintNow(&WindowFrame);
    SetActiveLabelFrame(LABEL2);
    }
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

TIME_LR GetChangePipeLengthTimer(void)
{
  return tCallChangePipeLength;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

void ShowChangePipeLengthDecisionMessage(char* message)
{
    RECT area;
    const FRAME* frame = &WindowFrame;
    area.ptTopLeft.nCol = frame->area.ptTopLeft.nCol + 5;
    area.ptTopLeft.nRow = frame->area.ptBottomRight.nRow - 70;
    area.ptBottomRight.nCol = frame->area.ptBottomRight.nCol - 5;
    area.ptBottomRight.nRow = area.ptTopLeft.nRow + 15;
    UI_DisplayStringCentered(message, &area);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

void ShowChangePipeLengthInfoMessage(char* message1, char* message2, char* message3)
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
