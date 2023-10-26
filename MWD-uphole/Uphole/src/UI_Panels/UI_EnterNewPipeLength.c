/*******************************************************************************
*       @brief      This file contains the implementation for the Start New Hole
*                   Decision Panel on the Main tab. That gives a option to select Yes/No
*       @file       Uphole/inc/UI_Panels/UI_EnterNewPipeLength.h
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
#include "UI_EnterNewPipeLength.h"
#include "UI_RecordDataPanel.h"
#include "UI_DataTab.h"
#include "UI_GroupBox.h"
#include "SysTick.h"
#include "UI_BooleanField.h"
#include "UI_FixedField.h"
#include "UI_StringField.h"
#include "UI_ChangePipeLengthCorrectDecisionPanel.h"
#include "UI_ChangePipeLengthDecisionPanel.h"
#include "UI_EnterSurvey.h"

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

static void EnterNewPipeLength_Paint(TAB_ENTRY* tab);
static void EnterNewPipeLength_Show(TAB_ENTRY* tab);
static void EnterNewPipeLength_KeyPressed(TAB_ENTRY* tab, BUTTON_VALUE key);
static void EnterNewPipeLength_TimerElapsed(TAB_ENTRY* tab);
static MENU_ITEM* EnterNewPipeLength_MenuPanel(U_BYTE index);
void ShowEnterNewPipeLengthMessage(char* message);
void ShowEnterNewPipeLengthInfoMessage(char* message1, char* message2, char* message3);
static void ReturnToMain(MENU_ITEM* item);
static void FinishEditing(MENU_ITEM* item);

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

static INT16 NewPipeLength = 0;

static MENU_ITEM EnterNewPipeLength_Menu[] =
{
    CREATE_FIXED_FIELD(TXT_ENTER_PIPE_LENGTH,  &LabelFrame1, &ValueFrame1, CurrrentLabelFrame, GetNewPipeLength, SetNewPipeLength, 2, 0, 0, 99), // 1-50
//    CREATE_FIXED_FIELD(TXT_ENTER_PIPE_LENGTH,  &LabelFrame1, &ValueFrame1, CurrrentLabelFrame, GetNewPipeLength, SetNewPipeLength, 4, 1, 0, 9999), //1000,
    CREATE_MENU_ITEM(TXT_FINISH_SET_LENGTH, &LabelFrame2, FinishEditing),
    CREATE_MENU_ITEM(TXT_BACK, &LabelFrame3, ReturnToMain),
};

BOOL EnterNewPipeLength_PanelActive = false;

static TIME_LR tCallEnterNewPipeLength = 0;

PANEL EnterNewPipeLength_Panel = {EnterNewPipeLength_MenuPanel, sizeof(EnterNewPipeLength_Menu) / sizeof(MENU_ITEM), EnterNewPipeLength_Paint, EnterNewPipeLength_Show, EnterNewPipeLength_KeyPressed, EnterNewPipeLength_TimerElapsed};

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*!
********************************************************************************
*       @details
*******************************************************************************/

void setEnterNewPipeLengthPanelActive(BOOL bFlag)
{
    EnterNewPipeLength_PanelActive = bFlag;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

BOOL getEnterNewPipeLengthPanelActive(void)
{
    return EnterNewPipeLength_PanelActive;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void EnterNewPipeLength_Paint(TAB_ENTRY* tab)
{
    EnterNewPipeLength_Show(tab);
    TabWindowPaint(tab);
    //GroupBoxPaint(&surveyGroup);
    ShowEnterNewPipeLengthMessage("When done hit \"End Edit\"");
    ShowEnterNewPipeLengthInfoMessage("Enter the new Pipe Length.","Remember: Pipe Length will set to","new value only for one survey.");
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void EnterNewPipeLength_Show(TAB_ENTRY* tab)
{
    MENU_ITEM* item = tab->MenuItem(tab, 0);
    UI_SetActiveFrame(item->labelFrame);
    SetActiveLabelFrame(item->labelFrame->eID);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void EnterNewPipeLength_KeyPressed(TAB_ENTRY* tab, BUTTON_VALUE key)
{
    switch(key)
    {
        case BUTTON_DASH:
        {
            setEnterNewPipeLengthPanelActive(false);
            RepaintNow(&WindowFrame);
            break;
        }
    }
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void EnterNewPipeLength_TimerElapsed(TAB_ENTRY* tab)
{
    //RepaintNow(&WindowFrame);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static MENU_ITEM* EnterNewPipeLength_MenuPanel(U_BYTE index)
{
    return &EnterNewPipeLength_Menu[index];
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

TIME_LR GetEnterNewPipeLengthTimer(void)
{
  return tCallEnterNewPipeLength;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

void SetNewPipeLength(INT16 length)
{
    NewPipeLength = length;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

INT16 GetNewPipeLength(void)
{
    INT16 value;
    value = NewPipeLength;
    return value;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void ReturnToMain(MENU_ITEM* item)
{
    if(ManualChangePipeLengthFlag)
    {
      	setEnterSurveyPanelActive(true);
	    PaintNow(&HomeFrame);
        ManualChangePipeLengthFlag = false;
    }
    else
    {
        setEnterNewPipeLengthPanelActive(false);
        SetLoggingState(LOGGING);
        RepaintNow(&WindowFrame);
        SetActiveLabelFrame(LABEL2);
    }
}


/*!
********************************************************************************
*       @details
*******************************************************************************/

static void FinishEditing(MENU_ITEM* item)
{
    setEnterNewPipeLengthPanelActive(false);
    setChangePipeLengthCorrectDecisionPanelActive(true);
    RepaintNow(&WindowFrame);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

void ShowEnterNewPipeLengthMessage(char* message)
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

void ShowEnterNewPipeLengthInfoMessage(char* message1, char* message2, char* message3)
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
