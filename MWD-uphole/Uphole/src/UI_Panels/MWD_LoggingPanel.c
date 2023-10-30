/*******************************************************************************
*       @brief      This file contains the implementation for the MWD
*                   Logging Panel.
*       @file       Uphole/src/UI_Panels/MWD_LoggingPanel.c
*       @date       November 2014
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
#include "RecordManager.h"
#include "FlashMemory.h"
#include "SysTick.h"
#include "MWD_LoggingPanel.h"
#include "LoggingManager.h"
#include "Manager_DataLink.h"
#include "DownholeBatteryAndLife.h"
#include "UI_Alphabet.h"
#include "UI_ScreenUtilities.h"
#include "UI_Frame.h"
#include "UI_LCDScreenInversion.h"
#include "UI_api.h"
#include "UI_MainTab.h"
#include "UI_Primitives.h"
#include "UI_GroupBox.h"
#include "UI_StartNewHoleDecisionPanel.h"
#include "UI_ClearAllHoleDecisionPanel.h"
#include "UI_UpdateDiagnosticDownholeDecisionPanel.h"
#include "UI_ChangePipeLengthDecisionPanel.h"
#include "UI_BoxSetupTab.h"
//#include "keypad.h" //MB
#include "TargetProtocol.h"
#include "tone_generator.h"
#include "UI_EnterSurveyDecisionPanel.h"
#include "GammaSensor.h"
#include "UI_ToolFacePanels.h"
#include "UI_MainTab.h"

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

static void TakeSurvey(MENU_ITEM* item);
static void StartNewHole(MENU_ITEM* item);
static void ClearHoleData(MENU_ITEM* item);
//static void UpdateDownHoleData(MENU_ITEM* item);
static void ChangePipeLength(MENU_ITEM* item);
static MENU_ITEM* GetLoggingMenu(U_BYTE index);
static void LoggingPaint(TAB_ENTRY* tab);
static void LoggingShow(TAB_ENTRY* tab);
static void TimerElapsed(TAB_ENTRY* tab);
//static BOOL GetDownholeOnStatus(void);
//static BOOL GetDownholeOffStatus(void);
INT16 GetToolfaceOffset(void);
static void EnterSurvey(MENU_ITEM* item);

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

static MENU_ITEM LoggingMenu[] =
{
	CREATE_MENU_ITEM(TXT_MAIN_SURVEY,         &LabelFrame1, TakeSurvey),
	CREATE_MENU_ITEM(TXT_CHANGE_PIPE_LNGT,    &LabelFrame2, ChangePipeLength),
//	CREATE_MENU_ITEM(TXT_UPDATE_DOWNHOLE,     &LabelFrame3, UpdateDownHoleData),
	CREATE_MENU_ITEM(TXT_START_NEW_HOLE,      &LabelFrame3, StartNewHole),
	CREATE_MENU_ITEM(TXT_CLEAR_ALL_HOLE_DATA, &LabelFrame4, ClearHoleData),
	CREATE_MENU_ITEM(TXT_ENTER_SURVEY,        &LabelFrame5, EnterSurvey)
};

PANEL MWDLogging = {
	GetLoggingMenu,
	sizeof(LoggingMenu) / sizeof(MENU_ITEM),
	LoggingPaint,
	LoggingShow,
	0,
	TimerElapsed
};

static GroupBox sensorGroup =
{ // puts the data collected into the Check Survey Panel on the Main Screen
    TXT_CHECK_SHOT,
    {
		{ 115, 5 },//,10
		{ 198, 140 }
    },
    {
        { TXT_AZIMUTH,  DisplayInt16Value,  .int16  = GetSurveyAzimuth },
        { TXT_PITCH,    DisplayInt16Value,  .int16  = GetSurveyPitch },
        { TXT_TOOLFACE, DisplayInt16Value,  .int16  = GetSurveyRoll },
        { TXT_GAMMA,    DisplayUint16Value,  .uint16 = GetSurveyGamma },
        { TXT_GTF,      DisplayInt16Value,  .int16 = GetGTFMain }, //GetSurveyGamma
        //{ TXT_TOOLFACE_OFFSET,    DisplayInt16Value, .int16 = GetToolfaceOffset },
    }
};

static GroupBox surveyGroup =
{ // whs 3Dec2021 puts the data collected into the Survey Panel on the Main Screen
    TXT_SURVEY,
    {
        { 35, 180 },
        { 193, 314 } //,310
    },
    {
        { TXT_RECORDNUM,             DisplayUint32Value,      .uint32 = getLastRecordNumber },
        { TXT_LENGTH,                DisplayUint32Value,      .uint32 = GetLastLengthuInt32 },
//        { TXT_LENGTH,                DisplayReal32Value,      .real32 = GetLastLength },
        { TXT_AZIMUTH,               DisplayReal32Value,      .real32 = GetLastAzimuth },
        { TXT_PITCH,                 DisplayReal32Value,      .real32 = GetLastPitch },
        { TXT_TOOLFACE,              DisplayReal32Value,      .real32 = GetLastRoll },
        { TXT_DOWNTRACK,             DisplayReal32Value,      .real32 = GetLastDepth },
        { TXT_LEFTRIGHT,             DisplayReal32Value,      .real32 = GetLastEasting },
        { TXT_UPDOWN,                DisplayReal32Value,      .real32 = GetLastNorthing },
        { TXT_GAMMA,                 DisplaySurveyInt16Value, .int16  = GetLastGamma },
        { TXT_GTF,                   DisplaySurveyInt16Value, .int16 = GetLastGTF },
    }
};

static TIME_LR tSurveyRequest = 0;
static TIME_LR tDecisionState = 0;
static TIME_LR tWaitingForDownhole = 0;
static BOOL bGetParam = false;
volatile BOOL SystemArmedFlag = false;
volatile BOOL SurveyTakenFlag = false;
int DownLockOn = 0;


//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

static MENU_ITEM* GetLoggingMenu(U_BYTE index)
{
	return &LoggingMenu[index];
}

static void TakeSurvey(MENU_ITEM* item)
{
    INT16 awakeTime;  // whs 2De2021 added this line <

//TODO - reinstate the self clearing Check Shot logic
//TEMP    SetCheckShot(false);// temporarily disable the self-clearing Check Shot logic.
	enum {
		TSS_IDLE, // no press yet, or back to no press
		TSS_ONE, // pressed one, waiting on Ytran modem to be connected
		TSS_TWO, // pressed twice, waiting on sensor data to be valid
	};
	static U_BYTE TakeSurveyState = TSS_IDLE;
// no matter what state we are in, if we loose comms, go back to IDLE
    if(!LoggingManager_IsConnected()) // whs 10Dec2021 most importantly the Yitan modem is connected to downhole
    {
		TakeSurveyState = TSS_IDLE;
    }
	// we get here when the survey button is pressed..
	switch(TakeSurveyState)
	{
		case TSS_IDLE: // first Survey button press, we issue a 250K tone for 2 seconds - to wake up Down-hole
			bGetParam = true; // whs 5Jan2022 turns on the Downhole power and the Yitrans should connect
			tSurveyRequest = ElapsedTimeLowRes(0);
			tone_generator_setstate(true);
			PaintNow(&HomeFrame); // prevents us from getting stuck on from previous partial shot
			SystemArmedFlag = false;
			SurveyTakenFlag = false; // whs 3Dec2021 who uses this???? no one I can find >>  TotalAwakeTime_secs = GetAwakeTimeSetting();
			TakeSurveyState = TSS_ONE;
			break;
		case TSS_ONE: // a 2nd survey press got us here - if Ytran connected, allow message to turn on sensor
                        awakeTime = GetAwakeTimeLeft(); // whs 5Jan2022 2nd press should turn on Compass and Gamma - see lights if not lock up happened
                        if(awakeTime <= 5) // whs 6Dec2021 added this If statement
                        {
                                TargProtocol_SetSensorPowerState(false); // turns off Tensteer
                                SystemArmedFlag = false;
                        }
                        else
                        {
                                if(LoggingManager_IsConnected() && SystemArmedFlag == false) // whs 10Dec2021 Yitran modem is connected to downhole
                                {
                                  TargProtocol_SetSensorPowerState(true); //whs 5Jan2022 turns on Tensteer+Gamma+ puts stars on LCD Uphole box
                                  SystemArmedFlag = true; // whs 6Dec2021 only ever set to true here - in all of Uphole code
                                        /*
                                          if(SystemArmedFlag == false)
                                        {
                                                TargProtocol_SetSensorPowerState(true); //whs 5Jan2022 turns on Tensteer+Gamma+ puts stars on LCD Uphole box
                                                SystemArmedFlag = true; // whs 6Dec2021 only ever set to true here - in all of Uphole code
                                        }
                                      */
                                  TakeSurveyState = TSS_TWO;
                                 }
                        }
			break;
		case TSS_TWO: // 3rd survey press got us here - if survey data valid .. take it
			if (GetSurveyCommsState() == true) // whs 6Jan2022 verifies data was received from downhole
                        { // whs 5Jan2022- if Tensteer and Gamma on take data and store it then turn Tensteer and Gamma turn off
                         	LoggingManager_TakeSurvey();  // whs 5Jan2022 need test here to see if Tensteer and Gamma turned on
				TakeSurvey_Time_Out_Seconds = 0;  // whs 5Jan2022 who normally turns it off?  I don't see it here???????
				SurveyTakenFlag = true;
				SystemArmedFlag = false;
				TakeSurveyState = TSS_ONE;
			}// whs 22Dec2021 do we need an else here...to reset TakeSurveyState to top of loop ...TSS_IDLE on next button press
                        break;
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void StartNewHole(MENU_ITEM* item)
{
	setStartNewHoleDecisionPanelActive(true);
	PaintNow(&HomeFrame);
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void ChangePipeLength(MENU_ITEM* item)
{
	setChangePipeLengthDecisionPanelActive(true);
	PaintNow(&HomeFrame);
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void ClearHoleData(MENU_ITEM* item)
{
	setClearAllHoleDataDecisionPanelActive(true);
	PaintNow(&HomeFrame);
}

/*******************************************************************************
*       @details
*******************************************************************************/
#if 0
static void UpdateDownHoleData(MENU_ITEM* item)
{
//	setUpdateDiagnosticDownholeDecisionPanelActive(true);
	PaintNow(&HomeFrame);
}
#endif
/*******************************************************************************
*       @details
*******************************************************************************/
static void LoggingPaint(TAB_ENTRY* tab)
{
    char text[100];
    INT16 awakeTime;

    TabWindowPaint(tab);
    GroupBoxPaint(&sensorGroup);
    GroupBoxPaint(&surveyGroup);
    if(LoggingManager_IsConnected()) // whs 10Dec2021 Yitran modem is connected to downhole
    {
        awakeTime = GetAwakeTimeLeft(); //whs 10Dec2021 time left of max 30 second of Downhole power
        if(awakeTime < 0) (awakeTime = 0);
        if(awakeTime < 5)
        {
// whs 3Dec2021 added the below GroupBoxPaint for testing purpose was to paint over sensor param with XXXs
//          GroupBoxPaint(&sensorGroupX); also sensorGroupX was only created for testing
            snprintf(text, 100, "Too Late - Sleeps in : %d", awakeTime);
            ShowStatusMessage(text);
        }
        else
        {
            if(SystemArmedFlag == true)  // whs6Dec2021 SystemArmedFlag seems to be only tested here
            {
                snprintf(text, 100, "***");
               ShowArmedStatusMessage(text);
                snprintf(text, 100, "When data appears push Survey to record: %d", awakeTime);
                ShowStatusMessage(text);
            }
            else
            {
                snprintf(text, 100, "To get data push Survey - Sleeps in : %d", awakeTime);
                ShowStatusMessage(text);
            }
        }
// whs 18Nov2021. The next two lines Reads NVRAM directly where Downhole LockOn is stored.  Param for this is set
//in Box Tab and was created only for testing. it negates the Downhole pwr on time of 30 seconds but has ripple software
//issues. But at least you dont't have to change the hardware for testing purposes
//       DownLockOn = NVRAM_data.fKeyBeeperEnable;
//       if ((awakeTime <= 10) && (DownLockOn)) tone_generator_setstate(true);
    }
	else
	{ //whs 10Nov2021 and 19Jan2022 changed message below
             ShowStatusMessage("Downhole Off - Push Survey to turn On");
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void LoggingShow(TAB_ENTRY* tab)
{
	MENU_ITEM* takeSurvey = tab->MenuItem(tab, 0);
	UI_SetActiveFrame(takeSurvey->labelFrame);
	SetActiveLabelFrame(takeSurvey->labelFrame->eID);
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void TimerElapsed(TAB_ENTRY* tab)
{
	if(bGetParam && ElapsedTimeLowRes(tSurveyRequest) >= HUNDRED_MILLI_SECONDS)
	{
		SetLoggingState(WAITING_TO_WAKEUP_DOWNHOLE);
		tWaitingForDownhole = ElapsedTimeLowRes(0);
		bGetParam = false;
	}
	RepaintNow(&WindowFrame);
}

/*******************************************************************************
*       @details
*******************************************************************************/
TIME_LR GetDecisionTimer(void)
{
	return tDecisionState;
}

/*******************************************************************************
*       @details
*******************************************************************************/
TIME_LR GetWaitingForDownholeTimer(void)
{
	return tWaitingForDownhole;
}

#if 0
/*******************************************************************************
*       @details
*******************************************************************************/
U_INT16 GetDownholeStatusOnTime(void)
{
	return NVRAM_data.nDownholeOnTime;
}

/*******************************************************************************
*       @details
*******************************************************************************/
U_INT16 GetDownholeStatusOffTime(void)
{
	return NVRAM_data.nDownholeOffTime;
}

/*******************************************************************************
*       @details
*******************************************************************************/
BOOL GetDownholeOnStatus(void)
{
	return bDownholeOnStatus;
}

/*******************************************************************************
*       @details
*******************************************************************************/
BOOL GetDownholeOffStatus(void)
{
	return bDownholeOffStatus;
}
#endif

/*******************************************************************************
*       @details
*******************************************************************************/
#if 0
static void ShowNumberOfHoleMessage(char* message)
{
	RECT area;
	const FRAME* frame = &WindowFrame;
	area.ptTopLeft.nCol = frame->area.ptTopLeft.nCol + 5;
	area.ptTopLeft.nRow = frame->area.ptBottomRight.nRow - 32;
	area.ptBottomRight.nCol = frame->area.ptBottomRight.nCol - 5;
	area.ptBottomRight.nRow = area.ptTopLeft.nRow + 15;
	UI_DisplayStringCentered(message, &area);
}
#endif
/*******************************************************************************
*       @details
*******************************************************************************/
static void EnterSurvey(MENU_ITEM* item)
{
	setEnterSurveyDecisionPanelActive(true);
//      setEnterSurveyPanelActive(true);
	PaintNow(&HomeFrame);
}

INT16 GetToolfaceOffset(void)
{
	INT16 TFOffset;
	if (((float)GetToolFaceValue()/10) > 360)
	{
		TFOffset = (INT16)(((float)GetToolFaceValue()/10 - 360)*10);
	}
	else
	{
		TFOffset = (INT16)(((float)GetToolFaceValue()/10)*10);
	}
	return TFOffset;
}
