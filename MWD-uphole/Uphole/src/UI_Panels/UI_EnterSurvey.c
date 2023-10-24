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
#include "UI_EnterSurvey.h"
#include "UI_RecordDataPanel.h"
#include "UI_DataTab.h"
#include "UI_GroupBox.h"
#include "SysTick.h"
#include "UI_BooleanField.h"
#include "UI_FixedField.h"
#include "UI_StringField.h"
#include "UI_ChangePipeLengthCorrectDecisionPanel.h"

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

static void EnterSurvey_Paint(TAB_ENTRY* tab);
static void EnterSurvey_Show(TAB_ENTRY* tab);
static void EnterSurvey_KeyPressed(TAB_ENTRY* tab, BUTTON_VALUE key);
static void EnterSurvey_TimerElapsed(TAB_ENTRY* tab);
static MENU_ITEM* EnterSurvey_MenuPanel(U_BYTE index);
void ShowEnterSurveyMessage(char* message);
void ShowEnterSurveyInfoMessageAPR(char* message1, char* message2, char* message3);
static void ReturnToMain(MENU_ITEM* item);
static void FinishEditing(MENU_ITEM* item);
static void ChangePipeLength(MENU_ITEM* item);

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

//static INT16 NewPipeLength = 0;
static INT16 ManualAzimuth = 0;
static REAL32 ManualPitch = 0;
static INT16 ManualToolface = 0;
//static INT16 ManualSurveyFlag = 0;
volatile BOOL ManualChangePipeLengthFlag = false;


static MENU_ITEM EnterSurvey_Menu[] =
{
    CREATE_FIXED_FIELD(TXT_AZIMUTH,  &LabelFrame1, &ValueFrame1, CurrrentLabelFrame, GetEnterAzimuth, SetManualAzimuth, 4, 1, 0, 9999),
    CREATE_FIXED_FIELD(TXT_PITCH,  &LabelFrame2, &ValueFrame2, CurrrentLabelFrame, GetEnterPitch, SetManualPitch, 4, 1, -999, 999), //1000,
//    CREATE_FIXED_FIELD(TXT_TOOLFACE,  &LabelFrame3, &ValueFrame3, CurrrentLabelFrame, GetEnterToolface, SetManualToolface, 4, 1, 0, 9999), //1000,
    CREATE_MENU_ITEM(TXT_CHANGE_PIPE_LNGT,    &LabelFrame3, ChangePipeLength),
    CREATE_MENU_ITEM(TXT_ENTER_NEXT_SURVEY, &LabelFrame4, FinishEditing),
    CREATE_MENU_ITEM(TXT_BACK, &LabelFrame5, ReturnToMain),
};

BOOL EnterSurvey_PanelActive = false;

static TIME_LR tCallEnterSurvey = 0;

PANEL EnterSurvey_Panel = {EnterSurvey_MenuPanel, sizeof(EnterSurvey_Menu) / sizeof(MENU_ITEM), EnterSurvey_Paint, EnterSurvey_Show, EnterSurvey_KeyPressed, EnterSurvey_TimerElapsed};

static GroupBox surveyGroup =
{
    TXT_SURVEY,
    {
        { 67, 180 },
        { 212, 310 }
    },
    {
        { TXT_RECORDNUM,             DisplayUint32Value,      .uint32 = GetLastRecordNumber },
        { TXT_LENGTH,                DisplayUint32Value,      .uint32 = GetLastLengthuInt32 },
//        { TXT_LENGTH,                DisplayReal32Value,      .real32 = GetLastLength },
        { TXT_AZIMUTH,               DisplayReal32Value,      .real32 = GetLastAzimuth },
        { TXT_PITCH,                 DisplayReal32Value,      .real32 = GetLastPitch },
        { TXT_TOOLFACE,              DisplayReal32Value,      .real32 = GetLastRoll },
        { TXT_DOWNTRACK,             DisplayReal32Value,      .real32 = GetLastDepth },
        { TXT_LEFTRIGHT,             DisplayReal32Value,      .real32 = GetLastEasting },
        { TXT_UPDOWN,                DisplayReal32Value,      .real32 = GetLastNorthing },
        { TXT_GAMMA,                 DisplaySurveyInt16Value, .int16  = GetLastGamma},
    }
};

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*!
********************************************************************************
*       @details
*******************************************************************************/

void setEnterSurveyPanelActive(BOOL bFlag)
{
    EnterSurvey_PanelActive = bFlag;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

BOOL getEnterSurveyPanelActive(void)
{
    return EnterSurvey_PanelActive;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void EnterSurvey_Paint(TAB_ENTRY* tab)
{
    EnterSurvey_Show(tab);
    TabWindowPaint(tab);
    GroupBoxPaint(&surveyGroup);
    ShowEnterSurveyMessage("Last Survey Taken ->");//When done hit \"End Edit\"");
//    ShowEnterSurveyInfoMessageAPR("Once Branch is set here", "subsequent records will", "become inactive records");//"Enter the Survey values.","Remember: Different Pipe Lengths need","to be entered from the Main Tab.");
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void EnterSurvey_Show(TAB_ENTRY* tab)
{
    MENU_ITEM* item = tab->MenuItem(tab, 0);
    UI_SetActiveFrame(item->labelFrame);
    SetActiveLabelFrame(item->labelFrame->eID);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void EnterSurvey_KeyPressed(TAB_ENTRY* tab, BUTTON_VALUE key)
{
    switch(key)
    {
        case BUTTON_DASH:
        {
            setEnterSurveyPanelActive(false);
            RepaintNow(&WindowFrame);
            break;
        }
    }
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void EnterSurvey_TimerElapsed(TAB_ENTRY* tab)
{
    //RepaintNow(&WindowFrame);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static MENU_ITEM* EnterSurvey_MenuPanel(U_BYTE index)
{
    return &EnterSurvey_Menu[index];
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

TIME_LR GetEnterSurveyTimer(void)
{
  return tCallEnterSurvey;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

void SetManualAzimuth(INT16 length)
{
    ManualAzimuth = length;
}

void SetManualPitch(INT16 length)
{
    ManualPitch = length;
}

void SetManualToolface(INT16 length)
{
    ManualToolface = length;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

INT16 GetEnterAzimuth(void)
{
    INT16 value;
    value = ManualAzimuth;
    return value;
}

INT16 GetEnterPitch(void)
{
    INT16 value;
    value = ManualPitch;
    return value;
}

INT16 GetEnterToolface(void)
{
    INT16 value;
    value = ManualToolface;
    return value;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void ReturnToMain(MENU_ITEM* item)
{
    ManualChangePipeLengthFlag = false;
    setEnterSurveyPanelActive(false);
    setEnterNewPipeLengthPanelActive(false);
    SetLoggingState(LOGGING);
    RepaintNow(&WindowFrame);
    SetActiveLabelFrame(LABEL1);
}


/*!
********************************************************************************
*       @details
*******************************************************************************/

static void FinishEditing(MENU_ITEM* item)
{
//    setEnterSurveyPanelActive(false);
//    setChangePipeLengthCorrectDecisionPanelActive(true);

      STRUCT_RECORD_DATA record;
  
      record.nAzimuth = ManualAzimuth;
      record.nPitch = ManualPitch;
      record.nRoll = ManualToolface;
      record.nGamma = 0;
//      RECORD_TakeSurveyMWD();
     LoggingManager_RecordRetrieved(&record, GetSurveyGamma());
  
//    ManualSurveyFlag = 1;
//    LoggingManager_TakeSurvey();
    
   // setEnterSurveyPanelActive(true);
   // RepaintNow(&WindowFrame);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

void ShowEnterSurveyMessage(char* message)
{
    RECT area;
    const FRAME* frame = &WindowFrame;
    area.ptTopLeft.nCol = frame->area.ptTopLeft.nCol + 5;
    area.ptTopLeft.nRow = frame->area.ptBottomRight.nRow - 85; //Used to be - 70
    area.ptBottomRight.nCol = frame->area.ptBottomRight.nCol - 120;
    area.ptBottomRight.nRow = area.ptTopLeft.nRow + 15;
    UI_DisplayStringCentered(message, &area);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

void ShowEnterSurveyInfoMessageAPR(char* message1, char* message2, char* message3)
{
    RECT area;
    const FRAME* frame = &WindowFrame;
    area.ptTopLeft.nCol = frame->area.ptTopLeft.nCol + 5;
    area.ptTopLeft.nRow = frame->area.ptBottomRight.nRow - 50; //Used to be - 140
    area.ptBottomRight.nCol = frame->area.ptBottomRight.nCol - 150;
    area.ptBottomRight.nRow = area.ptTopLeft.nRow + 15;
    UI_DisplayStringCentered(message1, &area);
//    area.ptTopLeft.nRow = frame->area.ptBottomRight.nRow - 110; //Used to be - 100
//    UI_DisplayStringCentered(message2, &area);
//    area.ptTopLeft.nRow = frame->area.ptBottomRight.nRow - 80; //Used to be - 60
 //   UI_DisplayStringCentered(message3, &area);
}

static void ChangePipeLength(MENU_ITEM* item)
{
	setChangePipeLengthDecisionPanelActive(true);
	PaintNow(&HomeFrame);
    
        ManualChangePipeLengthFlag = true;
        setEnterSurveyPanelActive(false);
        SetLoggingState(LOGGING);
        RepaintNow(&WindowFrame);
        SetActiveLabelFrame(LABEL1);
}