/*******************************************************************************
*       @brief      This file contains the implementation for the Clear All Hole Data
*                   Decision Panel on the Main tab. That gives a option to select Yes/No
*       @file       Uphole/inc/UI_Panels/UI_UpdateDiagnosticDownholeDecisionPanel.c
*       @date       January 2016
*       @copyright  COPYRIGHT (c) 2015 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <string.h>
#include <stdbool.h>
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
#include "UI_UpdateDiagnosticDownholeDecisionPanel.h"
#include "UI_RecordDataPanel.h"
#include "UI_DataTab.h"
#include "UI_GroupBox.h"
#include "SysTick.h"
#include "TargetProtocol.h"

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

static void UpdateDiagnosticDownhole_Decision_Paint(TAB_ENTRY* tab);
static void UpdateDiagnosticDownhole_Decision_Show(TAB_ENTRY* tab);
static void UpdateDiagnosticDownhole_Decision_KeyPressed(TAB_ENTRY* tab, BUTTON_VALUE key);
static void UpdateDiagnosticDownhole_Decision_TimerElapsed(TAB_ENTRY* tab);
static MENU_ITEM* UpdateDiagnosticDownhole_Decision_Menu(U_BYTE index);
static void Yes_UpdateDiagnosticDownhole_Decision(MENU_ITEM* item);
static void No_UpdateDiagnosticDownhole_Decision(MENU_ITEM* item);
void ShowUpdateDiagnosticParamDecisionMessage(char* message);
void ShowUpdateDiagnosticParamInfoMessage(char* message1, char* message2, char* message3);

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

static TIME_LR tUpdateDownHoleRequest = 0;

static MENU_ITEM UpdateDiagnosticDownhole_DecisionMenu[] =
{
    CREATE_MENU_ITEM(TXT_YES,    &LabelFrame1, Yes_UpdateDiagnosticDownhole_Decision),
    CREATE_MENU_ITEM(TXT_NO,     &LabelFrame2, No_UpdateDiagnosticDownhole_Decision)
};

BOOL UpdateDiagnosticDownhole_DecisionPanelActive = false;
PANEL UpdateDiagnosticDownhole_DecisionPanel = {UpdateDiagnosticDownhole_Decision_Menu, sizeof(UpdateDiagnosticDownhole_DecisionMenu) / sizeof(MENU_ITEM), UpdateDiagnosticDownhole_Decision_Paint, UpdateDiagnosticDownhole_Decision_Show, UpdateDiagnosticDownhole_Decision_KeyPressed, UpdateDiagnosticDownhole_Decision_TimerElapsed};

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*******************************************************************************
*       @details
*******************************************************************************/
void setUpdateDiagnosticDownholeDecisionPanelActive(BOOL bFlag)
{
    UpdateDiagnosticDownhole_DecisionPanelActive = bFlag;
}

/*******************************************************************************
*       @details
*******************************************************************************/
BOOL getUpdateDiagnosticDownholeDecisionPanelActive(void)
{
    return UpdateDiagnosticDownhole_DecisionPanelActive;
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void UpdateDiagnosticDownhole_Decision_Paint(TAB_ENTRY* tab)
{
    UpdateDiagnosticDownhole_Decision_Show(tab);
    TabWindowPaint(tab);
    ShowUpdateDiagnosticParamDecisionMessage("Are you sure.. Update Downhole?");
//    ShowUpdateDiagnosticParamInfoMessage("Send New configuration to Downhole.", "SET correct DIAG value; WRONG value","may disconnect Downhole PERMANENTLY!");
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void UpdateDiagnosticDownhole_Decision_Show(TAB_ENTRY* tab)
{
    MENU_ITEM* item = tab->MenuItem(tab, 0);
    UI_SetActiveFrame(item->labelFrame);
    SetActiveLabelFrame(item->labelFrame->eID);
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void UpdateDiagnosticDownhole_Decision_KeyPressed(TAB_ENTRY* tab, BUTTON_VALUE key)
{
    switch(key)
    {
        case BUTTON_DASH:
        {
            setUpdateDiagnosticDownholeDecisionPanelActive(false);
            RepaintNow(&WindowFrame);
            break;
        }
    }
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void UpdateDiagnosticDownhole_Decision_TimerElapsed(TAB_ENTRY* tab)
{
}

/*******************************************************************************
*       @details
*******************************************************************************/
static MENU_ITEM* UpdateDiagnosticDownhole_Decision_Menu(U_BYTE index)
{
    return &UpdateDiagnosticDownhole_DecisionMenu[index];
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void Yes_UpdateDiagnosticDownhole_Decision(MENU_ITEM* item)
{
    setUpdateDiagnosticDownholeDecisionPanelActive(false);
    if(LoggingManager_IsConnected()) // whs 10Dec2021 Yitan modem is connected to downhole
    {
//		TargProtocol_RequestUpdateDownholeSettings();
		SetLoggingState(UPDATE_DOWNHOLE);
		tUpdateDownHole = ElapsedTimeLowRes(0);
		RepaintNow(&HomeFrame);
    }
    else
    {
		// used to be a "dummy" Request to wake up Down-hole
        tUpdateDownHoleRequest = ElapsedTimeLowRes(0);
        SetLoggingState(WAKEUP_DOWNHOLE);
        RepaintNow(&WindowFrame);
    }
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void No_UpdateDiagnosticDownhole_Decision(MENU_ITEM* item)
{
    setUpdateDiagnosticDownholeDecisionPanelActive(false);
    SetLoggingState(LOGGING);
    RepaintNow(&WindowFrame);
    SetActiveLabelFrame(LABEL3);
}

/*******************************************************************************
*       @details
*******************************************************************************/
void ShowUpdateDiagnosticParamDecisionMessage(char* message)
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
void ShowUpdateDiagnosticParamInfoMessage(char* message1, char* message2, char* message3)
{
    RECT area;
    const FRAME* frame = &WindowFrame;
    area.ptTopLeft.nCol = frame->area.ptTopLeft.nCol + 5; //+5
    area.ptTopLeft.nRow = frame->area.ptBottomRight.nRow - 140; //140
    area.ptBottomRight.nCol = frame->area.ptBottomRight.nCol - 5; //-5
    area.ptBottomRight.nRow = area.ptTopLeft.nRow + 15;
    UI_DisplayStringCentered(message1, &area);
    area.ptTopLeft.nRow = frame->area.ptBottomRight.nRow - 100;
    UI_DisplayStringCentered(message2, &area);
    area.ptTopLeft.nRow = frame->area.ptBottomRight.nRow - 60; //originally 60
    UI_DisplayStringCentered(message3, &area);
}

/*******************************************************************************
*       @details
*******************************************************************************/
TIME_LR GetUpdateDownHoleRequestTimer(void)
{
    return tUpdateDownHoleRequest;
}
