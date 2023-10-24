/*******************************************************************************
*       @brief      This header file contains callable functions and public
*                   data for the Logging Manager.
*       @file       Uphole/inc/Logging/LoggingManager.h
*       @date       November 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef LOGGING_MANAGER_H
#define LOGGING_MANAGER_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"
#include "timer.h"
#include "RecordManager.h"

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

typedef enum
{
	NOT_LOGGING,
	LOGGING,
	WAITING_FOR_LOGGING,
	WAITING_FOR_SURVEY,
	STOP_LOGGING,
	STARTING_UPLOAD,
	WAITING_FOR_UPLOAD,
	RECORD_NOT_RETRIEVED,
	WAITING_UPLOAD_COMPLETE,
	UPLOAD_COMPLETE,
	SURVEY_REQUEST_TIMEOUT,
	START_NEW_HOLE,
	CLEAR_ALL_HOLE,
	CHANGE_TO_PARAM_TAB,
	DELETE_LAST_SURVEY,
	BRANCH_POINT_SET,
	WAKEUP_DOWNHOLE,
	UPDATE_DOWNHOLE,
	UPDATE_DOWNHOLE_SUCCESS,
	UPDATE_DOWNHOLE_FAILED,
	WAITING_TO_WAKEUP_DOWNHOLE,
	SURVEY_REQUEST_SUCCESS,
	CLEAR_ALL_HOLE_SUCCESS,
	START_NEW_HOLE_SUCCESS,
	DELETE_LAST_SURVEY_SUCCESS,
	DELETE_LAST_SURVEY_NOT_SUCCESS,
	BRANCH_POINT_SET_SUCCESS,
	COMPASS_LOGGING,
	STOP_COMPASS_LOGGING,
	NUMBER_OF_LOGGING_STATES,
}STATE_OF_LOGGING;

extern TIME_LR tUpdateDownHole;
extern TIME_LR tUpdateDownHoleSuccess;

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

	void LoggingManager(void);
	void LoggingManager_SetConnected(BOOL isConnected);
	BOOL LoggingManager_IsConnected(void);
	void LoggingManager_StartLogging(void);
	void LoggingManager_RecordRetrieved(STRUCT_RECORD_DATA* record, U_INT16 gamma);
	void LoggingManager_RecordNotReceived(void);
	void LoggingManager_TakeSurvey(void);
	void LoggingManager_StartedHole(void);
	void LoggingManager_Reset(void);
	void LoggingManager_Stop(void);
	void LoggingManager_StartUpload(void);
	void LoggingManager_UploadComplete(void);
	void LoggingManager_StartConnectedTimer(void);
	BOOL GetSavedCheckShot(void);

#ifdef __cplusplus
}
#endif

#endif // LOGGING_MANAGER_H
