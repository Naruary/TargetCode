/*******************************************************************************
*       @brief      This file contains the implementation for the Start New Hole
*                   Decision Panel on the Main tab. That gives a option to select Yes/No
*       @file       Uphole/inc/UI_Panels/UI_SartNewHoleDecisionPanel.h
*       @date       November 2015
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
#include "UI_StartNewHoleDecisionPanel.h"
#include "UI_RecordDataPanel.h"
#include "UI_DataTab.h"
#include "UI_GroupBox.h"
#include "SysTick.h"

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

static void StartNewHole_Decision_Paint(TAB_ENTRY* tab);
static void StartNewHole_Decision_Show(TAB_ENTRY* tab);
static void StartNewHole_Decision_KeyPressed(TAB_ENTRY* tab, BUTTON_VALUE key);
static void StartNewHole_Decision_TimerElapsed(TAB_ENTRY* tab);
static MENU_ITEM* StartNewHole_Decision_Menu(U_BYTE index);
static void Yes_StartNewHole_Decision(MENU_ITEM* item);
static void No_StartNewHole_Decision(MENU_ITEM* item);
void ShowStartNewHoleDecisionMessage(char* message);
void ShowStartNewHoleInfoMessage(char* message1, char* message2, char* message3);

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

static MENU_ITEM StartNewHole_DecisionMenu[] =
{
    CREATE_MENU_ITEM(TXT_YES,    &LabelFrame1, Yes_StartNewHole_Decision),
    CREATE_MENU_ITEM(TXT_NO,     &LabelFrame2, No_StartNewHole_Decision)
};

BOOL StartNewHole_DecisionPanelActive = false;

static TIME_LR tCallStartNewHole = 0;

PANEL StartNewHole_DecisionPanel = {StartNewHole_Decision_Menu, sizeof(StartNewHole_DecisionMenu) / sizeof(MENU_ITEM), StartNewHole_Decision_Paint, StartNewHole_Decision_Show, StartNewHole_Decision_KeyPressed, StartNewHole_Decision_TimerElapsed};

//static GroupBox surveyGroup =
//{
//    TXT_SURVEY,
//    {
//        { 35, 180 },
//        { 180, 310 }
//    },
//    {
//        { TXT_HASH, DisplayUint32Value, .uint32 = RECORD_getSelectSurveyRecordNumber },
//        { TXT_LENGTH,                DisplayReal32Value, .real32 = RECORD_GetSelectSurveyLength },
//        { TXT_AZIMUTH,               DisplayReal32Value, .real32 = RECORD_GetSelectSurveyAzimuth },
//        { TXT_INCLINATION,           DisplayReal32Value, .real32 = RECORD_GetSelectSurveyPitch },
//        { TXT_TOOLFACE,              DisplayReal32Value, .real32 = RECORD_GetSelectSurveyRoll },
//        { TXT_UPDOWN,                DisplayReal32Value, .real32 = RECORD_GetSelectSurveyNorthing },
//        { TXT_LEFTRIGHT,             DisplayReal32Value, .real32 = RECORD_GetSelectSurveyEasting },
//        { TXT_GAMMA,                 DisplayReal32Value, .real32 = RECORD_GetSelectSurveyGamma},
//        { TXT_DEPTH,                 DisplayReal32Value, .real32 = RECORD_GetSelectSurveyDepth },
//    }
//};

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*!
********************************************************************************
*       @details
*******************************************************************************/

void setStartNewHoleDecisionPanelActive(BOOL bFlag)
{
    StartNewHole_DecisionPanelActive = bFlag;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

BOOL getStartNewHoleDecisionPanelActive(void)
{
    return StartNewHole_DecisionPanelActive;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void StartNewHole_Decision_Paint(TAB_ENTRY* tab)
{
    StartNewHole_Decision_Show(tab);
    TabWindowPaint(tab);
    //GroupBoxPaint(&surveyGroup);
    ShowStartNewHoleDecisionMessage("Are you sure. Start New Borehole?");
    ShowStartNewHoleInfoMessage("Current Borehole will be saved.", "Remember to SET correct JOB","values before taking any survey.");
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void StartNewHole_Decision_Show(TAB_ENTRY* tab)
{
    MENU_ITEM* item = tab->MenuItem(tab, 0);
    UI_SetActiveFrame(item->labelFrame);
    SetActiveLabelFrame(item->labelFrame->eID);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void StartNewHole_Decision_KeyPressed(TAB_ENTRY* tab, BUTTON_VALUE key)
{
    switch(key)
    {
        case BUTTON_DASH:
        {
            setStartNewHoleDecisionPanelActive(false);
            RepaintNow(&WindowFrame);
            break;
        }
    }
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void StartNewHole_Decision_TimerElapsed(TAB_ENTRY* tab)
{
    //RepaintNow(&WindowFrame);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static MENU_ITEM* StartNewHole_Decision_Menu(U_BYTE index)
{
    return &StartNewHole_DecisionMenu[index];
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void Yes_StartNewHole_Decision(MENU_ITEM* item)
{
    setStartNewHoleDecisionPanelActive(false);
    RECORD_InitNewHole();
    SetLoggingState(START_NEW_HOLE);
    tCallStartNewHole = ElapsedTimeLowRes(0);
    //RecordDataPanelInit();
    RepaintNow(&WindowFrame);
    //PaintNow(&HomeFrame);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void No_StartNewHole_Decision(MENU_ITEM* item)
{
    //RECORD_InitBranchParam();
    setStartNewHoleDecisionPanelActive(false);
    SetLoggingState(LOGGING);
    RepaintNow(&WindowFrame);
    SetActiveLabelFrame(LABEL3);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

TIME_LR GetStartNewHoleTimer(void)
{
  return tCallStartNewHole;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

void ShowStartNewHoleDecisionMessage(char* message)
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

void ShowStartNewHoleInfoMessage(char* message1, char* message2, char* message3)
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
