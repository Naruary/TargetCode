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
#include "UI_DeleteLastSurveyDecisionPanel.h"
#include "UI_RecordDataPanel.h"
#include "UI_DataTab.h"
#include "UI_GroupBox.h"
#include "SysTick.h"
#include "UI_SurveyEditPanel.h"
#include "UI_EnterSurvey.h"

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

static void DeleteLastSurvey_Decision_Paint(TAB_ENTRY* tab);
static void DeleteLastSurvey_Decision_Show(TAB_ENTRY* tab);
static void DeleteLastSurvey_Decision_KeyPressed(TAB_ENTRY* tab, BUTTON_VALUE key);
static void DeleteLastSurvey_Decision_TimerElapsed(TAB_ENTRY* tab);
static MENU_ITEM* DeleteLastSurvey_Decision_Menu(U_BYTE index);
static void Yes_DeleteLastSurvey_Decision(MENU_ITEM* item);
static void No_DeleteLastSurvey_Decision(MENU_ITEM* item);
void ShowDeleteLastSurveyDecisionMessage(char* message);
void ShowDeleteLastSurveyInfoMessage(char* message1, char* message2);

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

static MENU_ITEM DeleteLastSurvey_DecisionMenu[] =
{
    CREATE_MENU_ITEM(TXT_YES,    &LabelFrame1, Yes_DeleteLastSurvey_Decision),
    CREATE_MENU_ITEM(TXT_NO,     &LabelFrame2, No_DeleteLastSurvey_Decision)
};

BOOL DeleteLastSurvey_DecisionPanelActive = false;

static TIME_LR tCallDeleteLastSurvey = 0;

PANEL DeleteLastSurvey_DecisionPanel = {DeleteLastSurvey_Decision_Menu, sizeof(DeleteLastSurvey_DecisionMenu) / sizeof(MENU_ITEM), DeleteLastSurvey_Decision_Paint, DeleteLastSurvey_Decision_Show, DeleteLastSurvey_Decision_KeyPressed, DeleteLastSurvey_Decision_TimerElapsed};

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


/********************************************************************************
*       @details
*******************************************************************************/

void setDeleteLastSurveyDecisionPanelActive(BOOL bFlag)
{
    DeleteLastSurvey_DecisionPanelActive = bFlag;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

BOOL getDeleteLastSurveyDecisionPanelActive(void)
{
    return DeleteLastSurvey_DecisionPanelActive;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void DeleteLastSurvey_Decision_Paint(TAB_ENTRY* tab)
{
    DeleteLastSurvey_Decision_Show(tab);
    TabWindowPaint(tab);
    GroupBoxPaint(&surveyGroup);
    ShowDeleteLastSurveyDecisionMessage("Delete This Survey?");
    ShowDeleteLastSurveyInfoMessage("Only Last Survey record", "can be Deleted....");
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void DeleteLastSurvey_Decision_Show(TAB_ENTRY* tab)
{
    MENU_ITEM* item = tab->MenuItem(tab, 0);
    UI_SetActiveFrame(item->labelFrame);
    SetActiveLabelFrame(item->labelFrame->eID);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void DeleteLastSurvey_Decision_KeyPressed(TAB_ENTRY* tab, BUTTON_VALUE key)
{
    switch(key)
    {
        case BUTTON_DASH:
        {
            setDeleteLastSurveyDecisionPanelActive(false);
            RepaintNow(&WindowFrame);
            break;
        }
    }
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void DeleteLastSurvey_Decision_TimerElapsed(TAB_ENTRY* tab)
{
    //RepaintNow(&WindowFrame);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static MENU_ITEM* DeleteLastSurvey_Decision_Menu(U_BYTE index)
{
    return &DeleteLastSurvey_DecisionMenu[index];
}

/*!
********************************************************************************
*       @details
*******************************************************************************/
static void Yes_DeleteLastSurvey_Decision(MENU_ITEM* item)
{
    STRUCT_RECORD_DATA lastRecoord;

    if(GetLastRecordNumber() == RECORD_getSelectSurveyRecordNumber()) 
    {
        RECORD_GetRecord(&lastRecoord, GetLastRecordNumber());
        if (lastRecoord.NumOfBranch != 0 || lastRecoord.InvalidDataFlag == true)
        {
            // If the survey is a branch point, show the error panel and do not delete it.
            SetLoggingState(DELETE_LAST_SURVEY_NOT_SUCCESS);
            //return; // Do not proceed with deletion.
        }
        else 
        {
            // If the survey is not a branch point, proceed with deletion.
            RECORD_StoreSelectSurvey(GetLastRecordNumber()-1);
            RECORD_removeLastRecord();
            RecordDataPanelInit();
            SetLoggingState(DELETE_LAST_SURVEY_SUCCESS);
        }
    }
    setDeleteLastSurveyDecisionPanelActive(false);
    tCallDeleteLastSurvey = ElapsedTimeLowRes(0);
    RepaintNow(&WindowFrame);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void No_DeleteLastSurvey_Decision(MENU_ITEM* item)
{
    setDeleteLastSurveyDecisionPanelActive(false);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

TIME_LR GetDeleteLastSurveyTimer(void)
{
  return tCallDeleteLastSurvey;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

void ShowDeleteLastSurveyDecisionMessage(char* message)
{
    RECT area;
    const FRAME* frame = &WindowFrame;
    area.ptTopLeft.nCol = frame->area.ptTopLeft.nCol + 5;
    area.ptTopLeft.nRow = frame->area.ptBottomRight.nRow - 90;
    area.ptBottomRight.nCol = frame->area.ptBottomRight.nCol - 150;
    area.ptBottomRight.nRow = area.ptTopLeft.nRow + 15;
    UI_DisplayStringCentered(message, &area);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

void ShowDeleteLastSurveyInfoMessage(char* message1, char* message2)
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
}
