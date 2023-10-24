/*******************************************************************************
*       @brief      This file contains the implementation for the Logging
*                   Manager.
*       @file       Uphole/src/Logging/LoggingManager.c
*       @date       December 2015
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "portable.h"
#include "LoggingManager.h"
#include "Manager_DataLink.h"
#include "ModemDriver.h"
#include "MWD_LoggingPanel.h"
#include "PeriodicEvents.h"
#include "RecordManager.h"
#include "FlashMemory.h"
#include "SysTick.h"
#include "UI_Frame.h"
#include "UI_RecordDataPanel.h"
#include "UI_api.h"
#include "UI_StartNewHoleDecisionPanel.h"
#include "UI_ClearAllHoleDecisionPanel.h"
#include "UI_DeleteLastSurveyDecisionPanel.h"
#include "UI_BranchPointSetDecisionPanel.h"
#include "UI_UpdateDiagnosticDownholeDecisionPanel.h"
#include "UI_BoxSetupTab.h"
#include "Compass_Panel.h"
#include "Compass_Plot.h"
#include "TargetProtocol.h"
#include "UI_EnterNewPipeLength.h"
#include "tone_generator.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

#define TONE_POWERUP_SIGNAL_TIME 2000

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

static TIME_LR uploadFinished;
static TIME_LR startUploadWaitStart;
static TIME_LR lastRequest;
static TIME_LR requestTimeout;
static TIME_LR startLoggingWaitStart = (TIME_LR)0;
static TIME_LR stopLoggingRequestStart = (TIME_LR)0;
static TIME_LR tYitranConnected = (TIME_LR)0;
static BOOL connected = false;
static BOOL bFirstAttempt = true;
// Variable for dynamic polling rate; 2 or 10 sec.  Default to faster poll rate.
static TIME_LR tChangeTab = (TIME_LR)0;
static TIME_LR tChangePanel = (TIME_LR)0;
static BOOL CheckShotLock = false;
static BOOL SaveCheckShot = false;
TIME_LR tUpdateDownHole = (TIME_LR)0;
TIME_LR tUpdateDownHoleSuccess = (TIME_LR)0;
volatile BOOL Shift_Button_Pushed_Flag = 0;

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

static void WaitForFlashWrite();
//static void WaitingForLogging(void);
//static void StopLogging(void);
//static void StartingUpload(void);
static void WaitingForUpload(void);
static void RecordRetrieved(void);
static void WaitingForUploadComplete(void);
static void WaitingForSurvey(void);
static void SurveyRequestTimeout(void);
void ChangeToParamTab(void);
void StartNewHoleSelected(void);
void ClearAllHoleSelected(void);
void DeleteLastSurveySelected(void);
void BranchPointSetSelected(void);
void UplaodCompleted(void);
void UpdateDownholeSelected(void);
void DownholeUpdateFailed(void);
void WakeupDownhole(void);
void DownholeUpdateSuccess(void);
void WaitingForDownhole(void);
void SurveyRequestSuccess(void);
void ClearAllHoleSuccess(void);
void StartNewHoleSuccess(void);
void DeleteLastSurveySucess(void);
void DeleteLastSurveyNotSucess(void);
void BranchPointSetSuccess(void);
void CompassLogging(void);
void CompassStopLogging(void);

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*******************************************************************************
*       @details
*******************************************************************************/
void LoggingManager_StartLogging(void)
{
	RECORD_OpenLoggingFile();
	PaintNow(&HomeFrame);
	SetClearHoleFlag();
}

/*******************************************************************************
*       @details
*******************************************************************************/
void LoggingManager_TakeSurvey(void)
{
	// requested survey capture.. wait for resulting capture in next state
	// if survey tool not valid yet, do not go to waiting.
	lastRequest = ElapsedTimeLowRes(0);
	TargProtocol_RequestSensorData_log();
//	SetLoggingState(WAITING_FOR_SURVEY);   // whs 7Jan2022 activated it then deactivated it didn't fix it
	RepaintNow(&HomeFrame);
}

/*******************************************************************************
*       @details
*******************************************************************************/
void LoggingManager_SetConnected(BOOL isConnected)
{
	connected = isConnected;
}

/*******************************************************************************
*       @details
*******************************************************************************/
BOOL LoggingManager_IsConnected(void)  // whs 10Dec2021 yitran modem is connnected to downhole
{
	return connected;
}

/*******************************************************************************
*       @details
*******************************************************************************/
void LoggingManager_RecordNotReceived(void)
{
	RECORD_NextMergeRecord();
	SetLoggingState(RECORD_NOT_RETRIEVED);
}

/*******************************************************************************
*       @details
*******************************************************************************/
void LoggingManager_RecordRetrieved(STRUCT_RECORD_DATA* record, U_INT16 gamma)
{
	RECORD_TakeSurveyMWD();
	SetLoggingState(SURVEY_REQUEST_SUCCESS);
	tChangePanel = ElapsedTimeLowRes(0);
	RECORD_MergeRecordMWD(record);
	RepaintNow(&WindowFrame);
}

/*******************************************************************************
*       @details
*******************************************************************************/
void LoggingManager_UploadComplete(void)
{
	WaitForFlashWrite();
	RECORD_CloseMergeFile();
	RepaintNow(&HomeFrame);
}

/*******************************************************************************
*       @details
*******************************************************************************/
void LoggingManager_Stop(void)
{
	SetLoggingState(STOP_LOGGING);
}

/*******************************************************************************
*       @details
*******************************************************************************/
void LoggingManager_Reset(void)
{
	TAB_ENTRY* tab = GetActiveTab();
	SetLoggingState(NOT_LOGGING);
	tab->Show(tab);
}

/*******************************************************************************
*       @details
*******************************************************************************/
void LoggingManager_StartUpload(void)
{
	SetLoggingState(STARTING_UPLOAD);
}

/*******************************************************************************
*       @details
*******************************************************************************/
void LoggingManager_StartedHole(void)
{
	SetLoggingState(WAITING_FOR_LOGGING);
}

/*******************************************************************************
*       @details
        LoggingManager() - Called every 10 ms in main.c line 230
*******************************************************************************/
void LoggingManager(void)
{
	static TIME_LR tGetSurveyData = (TIME_LR)0;
	switch (GetLoggingState())
	{
		case WAITING_FOR_LOGGING:
			// if time never set, set to now
			if (startLoggingWaitStart == 0)
			{
				startLoggingWaitStart = ElapsedTimeLowRes(0);
			}
			// if 1 second, change to LOGGING
			if (ElapsedTimeLowRes(startLoggingWaitStart) > 1000)
			{   // whs 7jan2022 i changed to 2000 from 1000 didn't fix intermit
				RepaintNow(&HomeFrame);
				startLoggingWaitStart = 0;
				stopLoggingRequestStart = 0;
				SetLoggingState(LOGGING);
			}
			break;
		case STOP_LOGGING:
			startUploadWaitStart = 0;
			// if time never set, set to now
			if (stopLoggingRequestStart == 0)
			{
				stopLoggingRequestStart = ElapsedTimeLowRes(0);
				RECORD_CloseLoggingFile();
			}
			// if 1/2 second, change to LOGGING  whs 7jan2022 tried 1000 nope
			if (ElapsedTimeLowRes(stopLoggingRequestStart) > 500)
			{
				RepaintNow(&HomeFrame);
				stopLoggingRequestStart = 0;
				SetLoggingState(LOGGING);
			}
			break;
		case STARTING_UPLOAD:
			// if time never set, set to now
			if (startUploadWaitStart == 0)
			{
				startUploadWaitStart = ElapsedTimeLowRes(0);
			}
			// if 1 second, change to WAITING_FOR_UPLOAD
			if (ElapsedTimeLowRes(startUploadWaitStart) > 1000)
			{
				SetLoggingState(WAITING_FOR_UPLOAD);
				lastRequest = ElapsedTimeLowRes(0);
				if (!RECORD_BeginMergeRecords())
				{
					LoggingManager_UploadComplete();
				}
				else
				{
					RepaintNow(&HomeFrame);
				}
				startUploadWaitStart = 0;
			}
			break;
		case WAITING_FOR_UPLOAD:
			WaitingForUpload();
			break;
//		case RECORD_RETRIEVED:
		case RECORD_NOT_RETRIEVED:
			RecordRetrieved();
			break;
		case WAITING_UPLOAD_COMPLETE:
			WaitingForUploadComplete();
			break;
		case UPLOAD_COMPLETE:
			UplaodCompleted();
			break;
		case WAITING_FOR_SURVEY:
			WaitingForSurvey();
			break;
		case SURVEY_REQUEST_TIMEOUT:
			SurveyRequestTimeout();
			break;
		case START_NEW_HOLE:
			StartNewHoleSelected();
			break;
		case CHANGE_TO_PARAM_TAB:
			ChangeToParamTab();
			break;
		case CLEAR_ALL_HOLE:
			ClearAllHoleSelected();
			break;
		case DELETE_LAST_SURVEY:
			DeleteLastSurveySelected();
			break;
		case BRANCH_POINT_SET:
			BranchPointSetSelected();
			break;
		case UPDATE_DOWNHOLE:
			UpdateDownholeSelected();
			break;
		case UPDATE_DOWNHOLE_FAILED:
			DownholeUpdateFailed();
			break;
		case WAKEUP_DOWNHOLE:
			WakeupDownhole();
			break;
		case UPDATE_DOWNHOLE_SUCCESS:
			DownholeUpdateSuccess();
			break;
		case WAITING_TO_WAKEUP_DOWNHOLE:
			WaitingForDownhole();
			break;
		case SURVEY_REQUEST_SUCCESS:
			SurveyRequestSuccess();
			break;
		case CLEAR_ALL_HOLE_SUCCESS:
			ClearAllHoleSuccess();
			break;
		case START_NEW_HOLE_SUCCESS:
			StartNewHoleSuccess();
			break;
		case DELETE_LAST_SURVEY_SUCCESS:
			DeleteLastSurveySucess();
			break;
		case DELETE_LAST_SURVEY_NOT_SUCCESS:
			DeleteLastSurveyNotSucess();
			break;
		case BRANCH_POINT_SET_SUCCESS:
			BranchPointSetSuccess();
			break;
		case COMPASS_LOGGING:
			CompassLogging();
			break;
		case STOP_COMPASS_LOGGING:
			CompassStopLogging();
			break;
	}
	// if we are set to automatically read the check data every second, do so
//	if(GetCheckShot())
	{
		if(tGetSurveyData == (TIME_LR)0)
		{
			tGetSurveyData = ElapsedTimeLowRes(0);
			TargProtocol_RequestAllData();
		} // whs 7Jan 2022 below was 1000 made it 2000.  This was a major fix !!!!
                  // Caused  a periodic lockup 
		else if(ElapsedTimeLowRes(tGetSurveyData) > (NVRAM_data.nCheckPollTime_sec*1000) ) 
		{ // the system would periodically lockup using the 1000 setting
			tGetSurveyData = (TIME_LR)0;
		}
	}
	// Edited by walter to to solve issues explained in Oct 5th 2015 day log
	if(ElapsedTimeLowRes(tYitranConnected) >= (THREE_SECOND + ONE_SECOND + THREE_QUARTER_SECOND + TWO_HUNDRED_MILLI_SECONDS) || bFirstAttempt)
	{   // whs 7Jan2022 I added one second ... didn't fix intermit ... removed it
		LoggingManager_SetConnected(false);
		bFirstAttempt = false;
	}
	else
	{
//		m_nPollTime = FIVE_SECOND;
		LoggingManager_SetConnected(true);// whs 10Dec2021 very important Logging Manager is "Yitran On" status 
                // whs 10Dec2021 I think all LoggingManager stuff should be called Yitran power status.
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void WaitForFlashWrite()
{
	RecordDataPanelInit();
	uploadFinished = ElapsedTimeLowRes(0);
	SetLoggingState(WAITING_UPLOAD_COMPLETE);
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void WaitingForUpload(void)
{
	if (ElapsedTimeLowRes(lastRequest) > 500)
	{
//		if(GetCheckShot())
//		{
			SetLoggingState(UPLOAD_COMPLETE);
//		}
		RepaintNow(&HomeFrame);
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void RecordRetrieved(void)
{
	if (ElapsedTimeLowRes(lastRequest) > 50)
	{
		lastRequest = ElapsedTimeLowRes(0);
		if (RECORD_RequestNextMergeRecord())
		{
		}
		else
		{
			LoggingManager_UploadComplete();
		}
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void WaitingForUploadComplete(void)
{
	if (ElapsedTimeLowRes(uploadFinished) > 1000)
	{
		SetLoggingState(UPLOAD_COMPLETE);
		RepaintNow(&HomeFrame);
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void WaitingForSurvey(void)
{
	// Increased to 2000 from 1000 since it was not allowing us to take
	// consective survey without refreshing of the check survey.
        // whs 7Jan2022 made 4000 to 5000 not work
	if (ElapsedTimeLowRes(lastRequest) > 4000)
	{
		requestTimeout = ElapsedTimeLowRes(0);
		SetLoggingState(SURVEY_REQUEST_TIMEOUT);
		RepaintNow(&HomeFrame);
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void SurveyRequestTimeout(void)
{
	if (ElapsedTimeLowRes(requestTimeout) > 5000)
	{
		SetLoggingState(LOGGING);
		RepaintNow(&HomeFrame);
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
void LoggingManager_StartConnectedTimer(void)
{
	bFirstAttempt = false;
	tYitranConnected = ElapsedTimeLowRes(0);
}

/*******************************************************************************
*       @details
*******************************************************************************/
void StartNewHoleSelected(void)
{
	if (ElapsedTimeLowRes(GetStartNewHoleTimer()) > 2000)
	{
		tChangePanel = ElapsedTimeLowRes(0);
		SetLoggingState(START_NEW_HOLE_SUCCESS);
		PaintNow(&WindowFrame);
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
void StartNewHoleSuccess(void)
{
	if (ElapsedTimeLowRes(tChangePanel) > 5000)
	{
		tChangeTab = ElapsedTimeLowRes(0);
		SetLoggingState(CHANGE_TO_PARAM_TAB);
		PaintNow(&HomeFrame);
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
void ClearAllHoleSelected(void)
{
	if (ElapsedTimeLowRes(GetClearAllHoleTimer()) > 2000)
	{
		tChangePanel = ElapsedTimeLowRes(0);
		SetLoggingState(CLEAR_ALL_HOLE_SUCCESS);
		PaintNow(&WindowFrame);
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
void ClearAllHoleSuccess(void)
{
	if (ElapsedTimeLowRes(tChangePanel) > 5000)
	{
		tChangeTab = ElapsedTimeLowRes(0);
		SetLoggingState(CHANGE_TO_PARAM_TAB);
		PaintNow(&HomeFrame);
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
void ChangeToParamTab(void)
{
	if (ElapsedTimeLowRes(tChangeTab) > 1000)
	{
		SetLoggingState(LOGGING);
		MainToParam();
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
void DeleteLastSurveySelected(void)
{
	if((GetLoggingState() != (DELETE_LAST_SURVEY_SUCCESS || DELETE_LAST_SURVEY_NOT_SUCCESS)) && (!getDeleteLastSurveyDecisionPanelActive()))
	{
		LoggingManager_StartUpload();
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
void BranchPointSetSelected(void)
{
	if ((GetLoggingState() != BRANCH_POINT_SET_SUCCESS) && (!getBranchPointSetDecisionPanelActive()))
	{
		LoggingManager_StartUpload();
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
void UplaodCompleted(void)
{
	SetLoggingState(WAITING_FOR_LOGGING);
}

/*******************************************************************************
*       @details
*******************************************************************************/
void UpdateDownholeSelected(void)
{
	if(ElapsedTimeLowRes(tUpdateDownHole) > 2000)
	{
		requestTimeout = ElapsedTimeLowRes(0);
		SetLoggingState(UPDATE_DOWNHOLE_FAILED);
		RepaintNow(&HomeFrame);
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
void DownholeUpdateFailed(void)
{
	if(ElapsedTimeLowRes(requestTimeout) > 5000)
	{
		SetLoggingState(LOGGING);
		PaintNow(&HomeFrame);
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
void WakeupDownhole(void)
{
	if(ElapsedTimeLowRes(GetUpdateDownHoleRequestTimer()) >= 4000)
	{
//		TargProtocol_RequestUpdateDownholeSettings();
		SetLoggingState(UPDATE_DOWNHOLE);
		tUpdateDownHole = ElapsedTimeLowRes(0);
		RepaintNow(&HomeFrame);
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
void DownholeUpdateSuccess(void)
{
	if(ElapsedTimeLowRes(tUpdateDownHoleSuccess) >= 5000)
	{
		SetLoggingState(LOGGING);
		PaintNow(&HomeFrame);
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
void WaitingForDownhole(void)
{
	// used to take a survey, but now wait to turn off tone and return to logging
	if(ElapsedTimeLowRes(GetWaitingForDownholeTimer()) >= TONE_POWERUP_SIGNAL_TIME)
	{
	    tone_generator_setstate(false);
		SetLoggingState(LOGGING);
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
void SurveyRequestSuccess(void)
{
	if(ElapsedTimeLowRes(tChangePanel) >= 3000)
	{
		SetLoggingState(LOGGING);
		RepaintNow(&HomeFrame);
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
void DeleteLastSurveySucess(void)
{
	if(ElapsedTimeLowRes(GetDeleteLastSurveyTimer()) >= 5000)
	{
		LoggingManager_StartUpload();
		RepaintNow(&WindowFrame);
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
void DeleteLastSurveyNotSucess(void)
{
	if(ElapsedTimeLowRes(GetDeleteLastSurveyTimer()) >= 5000)
	{
		LoggingManager_StartUpload();
		RepaintNow(&WindowFrame);
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
void BranchPointSetSuccess(void)
{
	if(ElapsedTimeLowRes(GetBranchPointSetTimer()) >= 5000)
	{
		LoggingManager_StartUpload();
		RepaintNow(&WindowFrame);
	}
}


/*******************************************************************************
*       @details
*******************************************************************************/
void CompassLogging(void)
{
	if(CheckShotLock == false)
	{
//		SaveCheckShot = GetCheckShot(); // Save original state
//		SetCheckShot(true); // turn on check shot
		CheckShotLock = true; // Lock
	}
	LoggingManager_TakeSurvey();
	Shift_Button_Pushed_Flag = 1;
	setCompassDecisionPanelActive(false);
}

/*******************************************************************************
*       @details
*******************************************************************************/
void CompassStopLogging(void)
{
	TAB_ENTRY* tab = GetActiveTab();
	SetLoggingState(NOT_LOGGING);
	tab->Show(tab);
//	PaintNow(&HomeFrame);
//	RepaintNow(&WindowFrame);
}

/*******************************************************************************
*       @details
*******************************************************************************/
BOOL GetSavedCheckShot(void)
{
	return SaveCheckShot;
}
