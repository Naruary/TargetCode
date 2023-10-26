/*******************************************************************************
*       @brief      This file contains the implementation for the
*                   Downhole main panel
*       @file       Uphole/src/UI_Panels/UI_DownholeMainPanel.c
*       @date       June 2019
*       @copyright  COPYRIGHT (c) 2019 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "SysTick.h"
#include "DownholeBatteryAndLife.h"
#include "GammaSensor.h"
#include "TextStrings.h"
#include "TargetProtocol.h"
#include "UI_ScreenUtilities.h"
#include "UI_Frame.h"
#include "UI_FixedField.h"
#include "UI_BooleanField.h"
#include "UI_api.h"
#include "UI_Alphabet.h"
#include "UI_MainTab.h"
#include "UI_DownholeMainPanel.h"
#include "version.h"
#include "LoggingManager.h"

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

static MENU_ITEM* GetMenu(U_BYTE index);
static void Paint(TAB_ENTRY* tab);
static void Show(TAB_ENTRY* tab);
static void TimerElapsed(TAB_ENTRY* tab);
static void ShowDownholeVoltageTabDiag(char* message1, int rowbit);
static void ShowDownholeVoltageTabDiag2(char* message1, int rowbit);
//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//
static MENU_ITEM DownholeMenu[] =
{
//	CREATE_FIXED_FIELD(TXT_DOWNHOLE_OFF_TIME,		&LabelFrame1, &ValueFrame1,
//		CurrrentLabelFrame,	GetDownholeOffTime,		SetDownholeOffTime, 4, 0, 0, 9999), //1000
//      15Oct2019 whs commented out above two lines to take Downhole Stay On Time from DWN Tab ....
//	CREATE_YESNO_FIELD(TXT_DOWNHOLE_DEEP_SLEEP,	&LabelFrame3, &ValueFrame3,
//		CurrrentLabelFrame,	GetDeepSleepMode,		SetDeepSleepMode),
	CREATE_BOOLEAN_FIELD(TXT_GAMMA_ON_OFF,			&LabelFrame1, &ValueFrame1,
		CurrrentLabelFrame,     GetGammaPoweredState,	TargProtocol_RequestSendGammaEnable),
//	CREATE_FIXED_FIELD(TXT_DOWNHOLE_ON_TIME,		&LabelFrame2, &ValueFrame2,
//		CurrrentLabelFrame,	GetAwakeTimeSetting,	SetAwakeTimeSetting,  4, 0, 0, 9999),
//	CREATE_MENU_ITEM(TXT_UPDATE_DOWNHOLE, &LabelFrame5, UpdateDownHoleSettings),
//	CREATE_MENU_ITEM(TXT_MAIN_SURVEY,         &LabelFrame1, TakeSurvey),
//	CREATE_MENU_ITEM(TXT_CHANGE_PIPE_LNGT,    &LabelFrame2, ChangePipeLength),
//	CREATE_MENU_ITEM(TXT_UPDATE_DOWNHOLE,     &LabelFrame3, UpdateDownHoleData),
//	CREATE_MENU_ITEM(TXT_START_NEW_HOLE,      &LabelFrame3, StartNewHole),
//	CREATE_MENU_ITEM(TXT_CLEAR_ALL_HOLE_DATA, &LabelFrame4, ClearHoleData),
};

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//
#define MENU_SIZE (sizeof(DownholeMenu) / sizeof(MENU_ITEM))

PANEL DownholePanel = {
	GetMenu,
	MENU_SIZE,
	Paint,
	Show,
	0,
	TimerElapsed
};

/*static GroupBox sensorGroup =
{
    TXT_CHECK_SHOT,
    {
      { 115, 10 },
      { 182, 140 }
    },
    {
        { TXT_AZIMUTH,  DisplayInt16Value,  .int16  = GetSurveyAzimuth },
        { TXT_PITCH,    DisplayInt16Value,  .int16  = GetSurveyPitch },
        { TXT_TOOLFACE, DisplayInt16Value,  .int16  = GetSurveyRoll },
//        { TXT_GAMMA,    DisplayUint16Value, .uint16 = GetSurveyGamma },
        { TXT_TEMPERATURE,    DisplayUint16Value, .int16 = GetSurveyTemperature },
    }
};*/

/*static GroupBox surveyGroup =
{
    TXT_SURVEY,
    {
        { 35, 180 },
        { 182, 310 }
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
};*/

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*******************************************************************************
*       @details
*******************************************************************************/
static MENU_ITEM* GetMenu(U_BYTE index)
{
	if(index >= MENU_SIZE) return NULL;
	return &DownholeMenu[index];
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void Paint(TAB_ENTRY* tab)
{
    char text[100];
    INT16 awakeTime;

	TabWindowPaint(tab);
	U_BYTE nMenuCount = tab->MenuSize(tab);

	// convert seconds of run time to hours
//	snprintf(text, 100, "Downhole Run Time:     %lu Hours", GetDownholeTotalOnTime()/3600ul);
//	ShowDownholeVoltageTabDiag(text, ((nMenuCount+0) * 15)+4 );

	snprintf(text, 100, "Downhole Voltage:         %.2f", (double)GetDownholeBatteryVoltage()/1000);
	ShowDownholeVoltageTabDiag(text, ((nMenuCount+0) * 15)+4 );
        
    snprintf(text, 100, "Downhole Batt 2 Voltage:         %.2f", (double)GetDownholeBattery2Voltage()/1000);
	ShowDownholeVoltageTabDiag2(text, ((nMenuCount+1) * 15)+4 );

	snprintf(text, 100, "Downhole Signal Strength:        %d", GetDownholeSignalStrength());
	ShowDownholeVoltageTabDiag(text, ((nMenuCount+2) * 15)+4 );

	snprintf(text, 100, "Dwn Software Version:     %s  %s", GetDownholeSWVersion(),GetDownholeSWDate());
	ShowDownholeVoltageTabDiag(text, ((nMenuCount+3) * 15)+4 );

	snprintf(text, 100, "Uph Software Version:     %s", GetSWVersion());
	ShowDownholeVoltageTabDiag(text, ((nMenuCount+4) * 15)+4 );

	if(LoggingManager_IsConnected()) // whs 10Dec2021 yitran modem is connected to Downhole
	{
		awakeTime = GetAwakeTimeLeft();
		if(awakeTime < 0) awakeTime = 0;
		snprintf(text, 100, "Down on press Survey - Sleeps in > %d", awakeTime);
		ShowStatusMessage(text);
	}
	else
	{
		ShowStatusMessage("Downhole Disconnected or Dead");
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void Show(TAB_ENTRY* tab)
{
#if 1
	UI_SetActiveFrame(&LabelFrame1);
    SetActiveLabelFrame(LABEL1);
    PaintNow(&HomeFrame);
#else
	MENU_ITEM* takeSurvey = tab->MenuItem(tab, 0);
	UI_SetActiveFrame(takeSurvey->labelFrame);
	SetActiveLabelFrame(takeSurvey->labelFrame->eID);
#endif
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void TimerElapsed(TAB_ENTRY* tab)
{
	MENU_ITEM* time = &DownholeMenu[0];
	if(time == NULL) return;
    {
        RepaintNow(time->valueFrame);
        RepaintNow(&HomeFrame);
    }
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void ShowDownholeVoltageTabDiag(char* message1, int rowbit)
{
    RECT area;
    const FRAME* frame = &WindowFrame;
    area.ptTopLeft.nCol = frame->area.ptTopLeft.nCol + 2;
    area.ptTopLeft.nRow = frame->area.ptTopLeft.nRow + rowbit;
    area.ptBottomRight.nCol = frame->area.ptBottomRight.nCol - 5;
    area.ptBottomRight.nRow = area.ptTopLeft.nRow + 15;
    UI_DisplayStringLeftJustified(message1, &area);
}
static void ShowDownholeVoltageTabDiag2(char* message1, int rowbit)
{
    RECT area;
    const FRAME* frame = &WindowFrame;
    area.ptTopLeft.nCol = frame->area.ptTopLeft.nCol + 3;
    area.ptTopLeft.nRow = frame->area.ptTopLeft.nRow + rowbit;
    area.ptBottomRight.nCol = frame->area.ptBottomRight.nCol - 6;
    area.ptBottomRight.nRow = area.ptTopLeft.nRow + 16;
    UI_DisplayStringLeftJustified(message1, &area);
}
