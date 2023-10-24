/*******************************************************************************
*       @brief      This file contains the implementation for the Survey Edit
*                   Panel on the Data tab.
*       @file       Uphole/src/UI_Panels/UI_SurveyEditPanel.c
*       @date       December 2015
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
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
#include "UI_SurveyEditPanel.h"
#include "UI_RecordDataPanel.h"
#include "UI_DataTab.h"
#include "UI_GroupBox.h"
#include "UI_DeleteLastSurveyDecisionPanel.h"
#include "UI_BranchPointSetDecisionPanel.h"

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

static void SurveyEdit_Paint(TAB_ENTRY* tab);
static void SurveyEdit_Show(TAB_ENTRY* tab);
static void SurveyEdit_KeyPressed(TAB_ENTRY* tab, BUTTON_VALUE key);
static void SurveyEdit_TimerElapsed(TAB_ENTRY* tab);
static MENU_ITEM* GetSurveyEditMenu(U_BYTE index);
static void DeleteSurvey(MENU_ITEM* item);
static void SetBranchPoint(MENU_ITEM* item);
static void FinishSurveyEdit(MENU_ITEM* item);

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

static MENU_ITEM SurveyEditMenu[] =
{
    CREATE_MENU_ITEM(TXT_DELETE_SURVEY,    &LabelFrame1, DeleteSurvey),
    CREATE_MENU_ITEM(TXT_SET_BRANCH_POINT, &LabelFrame2, SetBranchPoint),
    CREATE_MENU_ITEM(TXT_BACK,             &LabelFrame3, FinishSurveyEdit),
};

BOOL surveyEditPanelActive = false;

PANEL SurveyEditPanel = {GetSurveyEditMenu, sizeof(SurveyEditMenu) / sizeof(MENU_ITEM), SurveyEdit_Paint, SurveyEdit_Show, SurveyEdit_KeyPressed, SurveyEdit_TimerElapsed};

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

void setSurveyEditPanelActive(BOOL bFlag)
{
    surveyEditPanelActive = bFlag;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

BOOL getSurveyEditPanelActive(void)
{
    return surveyEditPanelActive;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void SurveyEdit_Paint(TAB_ENTRY* tab)
{
    if((RECORD_getSelectSurveyRecordNumber()+PreviousHoleEndingRecordNumber()) > PreviousHoleEndingRecordNumber())
    {
        SurveyEdit_Show(tab);
        TabWindowPaint(tab);
        GroupBoxPaint(&surveyGroup);
    }
    else
    {
      setSurveyEditPanelActive(false);
      RepaintNow(&WindowFrame);
    }
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void SurveyEdit_Show(TAB_ENTRY* tab)
{
    MENU_ITEM* item = tab->MenuItem(tab, 0);
    UI_SetActiveFrame(item->labelFrame);
    SetActiveLabelFrame(item->labelFrame->eID);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void SurveyEdit_KeyPressed(TAB_ENTRY* tab, BUTTON_VALUE key)
{
    switch(key)
    {
        case BUTTON_DASH:
        {
            setSurveyEditPanelActive(false);
            RepaintNow(&WindowFrame);
            break;
        }
    }
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void SurveyEdit_TimerElapsed(TAB_ENTRY* tab)
{
    //RepaintNow(&WindowFrame);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static MENU_ITEM* GetSurveyEditMenu(U_BYTE index)
{
    return &SurveyEditMenu[index];
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void DeleteSurvey(MENU_ITEM* item)
{
      SetLoggingState(DELETE_LAST_SURVEY);
      setDeleteLastSurveyDecisionPanelActive(true);
      setSurveyEditPanelActive(false);
      RepaintNow(&WindowFrame);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void SetBranchPoint(MENU_ITEM* item)
{
    SetLoggingState(BRANCH_POINT_SET);
    setBranchPointSetDecisionPanelActive(true);
    setSurveyEditPanelActive(false);
    RepaintNow(&WindowFrame);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void FinishSurveyEdit(MENU_ITEM* item)
{
    setSurveyEditPanelActive(false);
    RepaintNow(&WindowFrame);
}
