/*******************************************************************************
*       @brief      This file contains the implementation for the Start New Hole
*                   Decision Panel on the Main tab. That gives a option to select Yes/No
*       @file       Uphole/inc/UI_Panels/UI_SartNewHoleDecisionPanel.h
*       @date       December 2015
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
#include "UI_BranchPointSetDecisionPanel.h"
#include "UI_RecordDataPanel.h"
#include "UI_DataTab.h"
#include "UI_GroupBox.h"
#include "SysTick.h"
#include "UI_SurveyEditPanel.h"

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

static void BranchPointSet_Decision_Paint(TAB_ENTRY* tab);
static void BranchPointSet_Decision_Show(TAB_ENTRY* tab);
static void BranchPointSet_Decision_KeyPressed(TAB_ENTRY* tab, BUTTON_VALUE key);
static void BranchPointSet_Decision_TimerElapsed(TAB_ENTRY* tab);
static MENU_ITEM* BranchPointSet_Decision_Menu(U_BYTE index);
static void Yes_BranchPointSet_Decision(MENU_ITEM* item);
static void No_BranchPointSet_Decision(MENU_ITEM* item);
void ShowBranchPointSetDecisionMessage(char* message);
void ShowBranchPointSetInfoMessage(char* message1, char* message2, char* message3);

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

static MENU_ITEM BranchPointSet_DecisionMenu[] =
{
    CREATE_MENU_ITEM(TXT_YES,    &LabelFrame1, Yes_BranchPointSet_Decision),
    CREATE_MENU_ITEM(TXT_NO,     &LabelFrame2, No_BranchPointSet_Decision)
};

BOOL BranchPointSet_DecisionPanelActive = false;

static TIME_LR tCallBranchPointSet = 0;

PANEL BranchPointSet_DecisionPanel = {BranchPointSet_Decision_Menu, sizeof(BranchPointSet_DecisionMenu) / sizeof(MENU_ITEM), BranchPointSet_Decision_Paint, BranchPointSet_Decision_Show, BranchPointSet_Decision_KeyPressed, BranchPointSet_Decision_TimerElapsed};

static GroupBox surveyGroup =
{
    TXT_SURVEY,
    {
        { 35, 180 },
        { 180, 310 }
    },
    {
        { TXT_HASH, DisplayUint32Value, .uint32 = RECORD_getSelectSurveyRecordNumber },
        { TXT_LENGTH,                DisplayReal32Value, .real32 = RECORD_GetSelectSurveyLength },
        { TXT_AZIMUTH,               DisplayReal32Value, .real32 = RECORD_GetSelectSurveyAzimuth },
        { TXT_INCLINATION,           DisplayReal32Value, .real32 = RECORD_GetSelectSurveyPitch },
        { TXT_TOOLFACE,              DisplayReal32Value, .real32 = RECORD_GetSelectSurveyRoll },
        { TXT_UPDOWN,                DisplayReal32Value, .real32 = RECORD_GetSelectSurveyNorthing },
        { TXT_LEFTRIGHT,             DisplayReal32Value, .real32 = RECORD_GetSelectSurveyEasting },
        { TXT_GAMMA,                 DisplayReal32Value, .real32 = RECORD_GetSelectSurveyGamma},
        { TXT_DEPTH,                 DisplayReal32Value, .real32 = RECORD_GetSelectSurveyDepth },
    }
};

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*!
********************************************************************************
*       @details
*******************************************************************************/

void setBranchPointSetDecisionPanelActive(BOOL bFlag)
{
    BranchPointSet_DecisionPanelActive = bFlag;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

BOOL getBranchPointSetDecisionPanelActive(void)
{
    return BranchPointSet_DecisionPanelActive;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void BranchPointSet_Decision_Paint(TAB_ENTRY* tab)
{
    BranchPointSet_Decision_Show(tab);
    TabWindowPaint(tab);
    GroupBoxPaint(&surveyGroup);
    ShowBranchPointSetDecisionMessage("Set Branch Point Here?");
    ShowBranchPointSetInfoMessage("Once Branch is set here", "subsequent records will", "become inactive records");
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void BranchPointSet_Decision_Show(TAB_ENTRY* tab)
{
    MENU_ITEM* item = tab->MenuItem(tab, 0);
    UI_SetActiveFrame(item->labelFrame);
    SetActiveLabelFrame(item->labelFrame->eID);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void BranchPointSet_Decision_KeyPressed(TAB_ENTRY* tab, BUTTON_VALUE key)
{
    switch(key)
    {
        case BUTTON_DASH:
        {
            setBranchPointSetDecisionPanelActive(false);
            RepaintNow(&WindowFrame);
            break;
        }
    }
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void BranchPointSet_Decision_TimerElapsed(TAB_ENTRY* tab)
{
    //RepaintNow(&WindowFrame);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static MENU_ITEM* BranchPointSet_Decision_Menu(U_BYTE index)
{
    return &BranchPointSet_DecisionMenu[index];
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void Yes_BranchPointSet_Decision(MENU_ITEM* item)
{

    // put your logic here
    RECORD_SetBranchPointIndex(RECORD_getSelectSurveyRecordNumber());
    RECORD_InitBranchParam();
    SetLoggingState(BRANCH_POINT_SET_SUCCESS);
    tCallBranchPointSet = ElapsedTimeLowRes(0);
    setBranchPointSetDecisionPanelActive(false);
    RepaintNow(&WindowFrame);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void No_BranchPointSet_Decision(MENU_ITEM* item)
{
    setBranchPointSetDecisionPanelActive(false);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

TIME_LR GetBranchPointSetTimer(void)
{
  return tCallBranchPointSet;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

void ShowBranchPointSetDecisionMessage(char* message)
{
    RECT area;
    const FRAME* frame = &WindowFrame;
    area.ptTopLeft.nCol = frame->area.ptTopLeft.nCol + 5;
    area.ptTopLeft.nRow = frame->area.ptBottomRight.nRow - 70;
    area.ptBottomRight.nCol = frame->area.ptBottomRight.nCol - 150;
    area.ptBottomRight.nRow = area.ptTopLeft.nRow + 15;
    UI_DisplayStringCentered(message, &area);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

void ShowBranchPointSetInfoMessage(char* message1, char* message2, char* message3)
{
    RECT area;
    const FRAME* frame = &WindowFrame;
    area.ptTopLeft.nCol = frame->area.ptTopLeft.nCol + 5;
    area.ptTopLeft.nRow = frame->area.ptBottomRight.nRow - 140;
    area.ptBottomRight.nCol = frame->area.ptBottomRight.nCol - 150;
    area.ptBottomRight.nRow = area.ptTopLeft.nRow + 15;
    UI_DisplayStringCentered(message1, &area);
    area.ptTopLeft.nRow = frame->area.ptBottomRight.nRow - 110;
    UI_DisplayStringCentered(message2, &area);
    area.ptTopLeft.nRow = frame->area.ptBottomRight.nRow - 80;
    UI_DisplayStringCentered(message3, &area);
}
