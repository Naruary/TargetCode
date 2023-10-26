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
//#include "UI_ChangePipeLengthDecisionPanel.h"
#include "UI_EnterSurveyDecisionPanel.h"
#include "UI_EnterSurvey.h"
#include "UI_RecordDataPanel.h"
#include "UI_DataTab.h"
#include "UI_GroupBox.h"
#include "SysTick.h"
#include "UI_EnterNewPipeLength.h"

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

static void EnterSurvey_Decision_Paint(TAB_ENTRY* tab);
static void EnterSurvey_Decision_Show(TAB_ENTRY* tab);
static void EnterSurvey_Decision_KeyPressed(TAB_ENTRY* tab, BUTTON_VALUE key);
static void EnterSurvey_Decision_TimerElapsed(TAB_ENTRY* tab);
static MENU_ITEM* EnterSurvey_Decision_Menu(U_BYTE index);
static void Yes_EnterSurvey_Decision(MENU_ITEM* item);
static void No_EnterSurvey_Decision(MENU_ITEM* item);
void ShowEnterSurveyDecisionMessage(char* message);
void ShowEnterSurveyInfoMessage(char* message1, char* message2, char* message3);

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

static MENU_ITEM EnterSurvey_DecisionMenu[] =
{
    CREATE_MENU_ITEM(TXT_YES,    &LabelFrame1, Yes_EnterSurvey_Decision),
    CREATE_MENU_ITEM(TXT_NO,     &LabelFrame2, No_EnterSurvey_Decision)
};

BOOL EnterSurvey_DecisionPanelActive = false;

PANEL EnterSurvey_DecisionPanel = {EnterSurvey_Decision_Menu, sizeof(EnterSurvey_DecisionMenu) / sizeof(MENU_ITEM), EnterSurvey_Decision_Paint, EnterSurvey_Decision_Show, EnterSurvey_Decision_KeyPressed, EnterSurvey_Decision_TimerElapsed};

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*!
********************************************************************************
*       @details
*******************************************************************************/

void setEnterSurveyDecisionPanelActive(BOOL bFlag)
{
    EnterSurvey_DecisionPanelActive = bFlag;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

BOOL getEnterSurveyDecisionPanelActive(void)
{
    return EnterSurvey_DecisionPanelActive;    
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void EnterSurvey_Decision_Paint(TAB_ENTRY* tab)
{
    EnterSurvey_Decision_Show(tab);
    TabWindowPaint(tab);
    //GroupBoxPaint(&surveyGroup);
    ShowEnterSurveyDecisionMessage("Are you sure. Enter Survey(s) Manually?");
    ShowEnterSurveyInfoMessage("Enter Survey(s) Manually.", "Remember: DO NOT RESTART Uphole box","until the next survey is taken.");
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void EnterSurvey_Decision_Show(TAB_ENTRY* tab)
{
    MENU_ITEM* item = tab->MenuItem(tab, 0);
    UI_SetActiveFrame(item->labelFrame);
    SetActiveLabelFrame(item->labelFrame->eID);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void EnterSurvey_Decision_KeyPressed(TAB_ENTRY* tab, BUTTON_VALUE key)
{
    switch(key)
    {
        case BUTTON_DASH:
        {
            setEnterSurveyDecisionPanelActive(false);
            RepaintNow(&WindowFrame);
            break;
        }
    }
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void EnterSurvey_Decision_TimerElapsed(TAB_ENTRY* tab)
{
    //RepaintNow(&WindowFrame);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static MENU_ITEM* EnterSurvey_Decision_Menu(U_BYTE index)
{
    return &EnterSurvey_DecisionMenu[index];
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void Yes_EnterSurvey_Decision(MENU_ITEM* item)
{
    setEnterSurveyDecisionPanelActive(false);
    setEnterSurveyPanelActive(true);
    //RECORD_InitNewHole();
    //SetLoggingState(START_NEW_HOLE);
    //tCallChangePipeLength = ElapsedTimeLowRes(0);
    RepaintNow(&WindowFrame);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void No_EnterSurvey_Decision(MENU_ITEM* item)
{
    //RECORD_InitBranchParam();
    setEnterSurveyDecisionPanelActive(false);
    SetLoggingState(LOGGING);
    RepaintNow(&WindowFrame);
    SetActiveLabelFrame(LABEL5);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
TIME_LR GetEnterSurveyTimer(void)
{
  return tCallEnterSurvey;
}
*/
/*!
********************************************************************************
*       @details
*******************************************************************************/

void ShowEnterSurveyDecisionMessage(char* message)
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

void ShowEnterSurveyInfoMessage(char* message1, char* message2, char* message3)
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
