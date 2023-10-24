/*******************************************************************************
*       @brief      Header file for textStrings.c.
*       @file       Uphole/inc/DataManagers/TextStrings.h
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef TEXTSTRINGS_H
#define TEXTSTRINGS_H

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

typedef enum __MSG_VALUES__
{
	TXT_NONE,
	TXT_INFO,
	TXT_MAIN,
	TXT_MAIN_GET_HOLE_DATA,
	TXT_MAIN_START_HOLE,
	TXT_MAIN_STOP_HOLE,
	TXT_MAIN_SURVEY,
	TXT_SETUP,
	TXT_SETUP_CHANGE_BACKLIGHT,
	TXT_SETUP_CHANGE_BUZZER,
	TXT_SETUP_CHANGE_LANGUAGE,
	TXT_SETUP_SET_DATE,
	TXT_SETUP_SET_TIME,
	TXT_TEST,
	TXT_VERSION,
	TXT_SETUP_PIPE_LENGTH,
	TXT_AZIMUTH,
	TXT_PITCH,
	TXT_INCLINATION,
	TXT_ROLL,
	TXT_TOOLFACE,
	TXT_TOOLFACE_OFFSET,
	TXT_SET_TOOLFACE_ZERO,
	TXT_CLEAR_TOOLFACE_ZERO,

	TXT_90_ERROR, //added in for display messages in param tab
	TXT_270_ERROR,
	TXT_MAX_ERROR,

	TXT_DOWNTRACK,
	TXT_AZIMUTH_ABREV,
	TXT_DESIRED_AZIMUTH_ABREV,
	TXT_PITCH_ABREV,
	TXT_INCLINATION_ABREV,
	TXT_TOOLFACE_ABREV,
	TXT_DOWNTRACK_ABREV,
	TXT_DATA,
	TXT_PARAM,
	TXT_SENSOR,
	TXT_SURVEYS,
	TXT_LENGTH,
	TXT_TOTAL_SURVEYS,
	TXT_DELETE_SURVEY,
	TXT_TOTAL_LENGTH,
	TXT_CONNECTED,
	TXT_DISCONNECTED,
	TXT_ON,
	TXT_OFF,
	TXT_BACK,
	TXT_ENGLISH,
	TXT_CHINESE,
	TXT_HASH,
	TXT_DEPTH,
	TXT_ALERT_EXAMPLE,
	TXT_DECLINATION,
	TXT_HOLE_NAME,
	TXT_DESIRED_AZIMUTH,
	TXT_STARTING_AZIMUTH,
	TXT_STARTING_INCLINATION,
	TXT_UPDOWN_ABREV,
	TXT_LEFTRIGHT_ABREV,
	TXT_GAMMA,
	TXT_SURVEY,
	TXT_UPDOWN,
	TXT_LEFTRIGHT,
	TXT_STARTING_DEPTH,
	TXT_LENGTH_ABREV,
	TXT_CALIBRATION,
	TXT_CAL_HARDIRON_COMP,
	TXT_CAL_SOFTIRON_COMP,
	TXT_CAL_AZIMUTH_REVERSAL,
	TXT_CAL_PITCH_REVERSAL,
	TXT_CAL_ROLL_REVERSAL,
	TXT_CAL_ROLL_UNSIGNED,
	TXT_CAL_AZIMUTH_OFFSET,
	TXT_CAL_ROLL_OFFSET,
	TXT_CAL_HARDIRON_DETAIL,
	TXT_CAL_SOFTIRON_DETAIL,
	TXT_CHECK_SHOT,
	TXT_CHECK_SHOT_POLL_TIME,
	TXT_CLEAR_ALL_HOLE_DATA,
	TXT_START_NEW_HOLE,
	TXT_SET_BRANCH_POINT,
	TXT_YES,
	TXT_NO,
	TXT_DIAG,
	TXT_DOWNHOLE_OFF_TIME,
	TXT_DOWNHOLE_ON_TIME,
	TXT_DOWNHOLE_DEEP_SLEEP,
	TXT_UPDATE_DOWNHOLE,
	TXT_CHANGE_PIPE_LNGT,
	TXT_GAMMA_ON_OFF,
	TXT_DOWNHOLE_LIFE,
	TXT_UPHOLE_LIFE,
	TXT_PLAN,
	TXT_SIDE,
	TXT_ENTER_PIPE_LENGTH,
	TXT_FINISH_SET_LENGTH,
	TXT_GAMCOMP,
	TXT_GAMMAPLT,
	TXT_SIDEGAMMA,
	TXT_LCD_BACKLT_OFF_TIME,
	TXT_LCD_OFF_TIME,
        TXT_SETUP_DEBOUNCE_TIME, //ZD 10/09/2023 Added for Debounce Button text
	TXT_RECORDNUM,
	TXT_APPLY_ERR_CORRECT,
	TXT_TEMPERATURE,
	TXT_GTF,
	TXT_ENTER_SURVEY,
	TXT_ENTER_NEXT_SURVEY,
	TXT_G,
	TXT_ENTER_TOOLFACE,
	TXT_USB_DOWNLOAD,
	TXT_USB_CRF,
	TXT_USB_WTF,
	TXT_USB_UNM,
	TXT_DATA_UPLOAD, //ZD 21Spetember2023 This is where the uploading data starts by giving it a text name for .h
	MAX_TXT_MSG// <---- Must be the LAST entry
} TXT_VALUES;

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

	char* GetTxtString(TXT_VALUES eMessage);

#ifdef __cplusplus
}
#endif
#endif
