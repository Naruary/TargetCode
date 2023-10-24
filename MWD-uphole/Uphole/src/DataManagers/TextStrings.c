/*******************************************************************************
*       @brief      This module contains all message strings sent to the sensor.
*       @file       Uphole/src/DataManagers/TextStrings.c
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "textStrings.h"
#include "language.h"

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

static const char* const g_sEnglishTxtStrings[MAX_TXT_MSG]  = {
	" ",
	"INFO",
	"MAN",
	"Get Downhole Records",
	"Start Logging Hole",
	"Stop Logging Hole",
	"Take Survey",
	"BOX",
	"Backlight",
	"Downhole Lock On",   // whs 18Nov2021 used to be just Buzzer.  Now used to turn on and off Downhole
	"Language",
	"Date",
	"Time",
	"TEST",
	"SOFTWARE VERSION: %s",
	"Set Pipe Length",
	"Azimuth",
	"Pitch",
	"Inclination",
	"Roll",
	"Toolface",
	"Offset",
	"Set Toolface (TDC)",
	"Clear Toolface",

	"Error at 90deg", //added in for display messages in param tab
	"Error at 270deg",
	"First Maximum Error",

	"Downtrack",
	"AZ",
	"Desired AZ",
	"PI",
	"INC",
	"TF",
	"DT",
	"DAT",
	"JOB",
	"SENSOR",
	"Surveys",
	"Length",
	"Total Survey",
	"Delete Survey",
	"Total Length",
	"Sensor connected",
	"Sensor not connected",
	"On",
	"Off",
	"Back",
	"English",
	"Chinese",
	"#",
	"Depth",
	"This is ALERT text!",
	"Declination",
	"Hole Name",
	"Desired Azimuth",
	"Starting Azimuth",
	"Starting Inclination",
	"U/D",
	"L/R",
	"Gamma",
	"Survey",
	"Up/Down",
	"Left/Right",
	"Starting Depth",
	"Len",
	"CAL",
	"Hard Iron Compensation",
	"Soft Iron Compensation",
	"Azimuth Output Reverse",
	"Pitch Output Reverse",
	"Roll Output Reverse",
	"Roll Output Unsigned",
	"Azimuth Offset",
	"Roll Offset",
	"Hard Iron Cal Details",
	"Soft Iron Cal Details",
	"Check Survey",
	"Check Surv Poll Time",
	"Clear All Hole Data",
	"Start New Hole",
	"Set Branch Point",
	"Yes",
	"No",
	"DWN", // 15Oct2019 whs changed this from DOW
	"Downhole Turn Off Timer",
	"Downhole Poweron Time",
	"Sleep",
//	"Downhole Deep Sleep",
	"Update Downhole",
	"Change Pipe Length",
	"Gamma Power",
	"Downhole Voltage",
	"Uphole Voltage",
	"PLN",
	"SID",
	"Enter New Pipe Length",
	"End Edit",
	"GMC",
	"GMP",
	"SGP",
	"Backlight On Time",
	"Uphole On Time", //ZD 17July 2023 changed from Display Turn Off Time per Bills request
        "Set Button Debounce", //ZD 9October2023 Added for the Debounce Set in Box Tab
	"Record #",
	"Error Correct Azim.",
	"Temp",
	"Gamma TF",
	"Enter Survey(s)",
	"Enter Next Survey",
	"GA",
	"Enter Toolface Offset",
	"USB Download",
	"Download Data To PC", //whs 15Feb2022 mod to Thumb Drive
	"Write to USB File",
	"Remove Thumb Drive",
	"Upload Data To Magnestar" //ZD 21September2023 This is where the Text from the .h file becomes a displayable UI change with the text displaying what is written here without using a printf
};

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   GetTxtString()
;
; Description:
;   Gets the String associated with the enumerated value in the message
;   table.
;
; Parameters:
;   SENSOR_MESSAGES - Enumerated tag for requested string
;
; Return
;    char *: String value
;
; Reentrancy:
;   No.
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/

char* GetTxtString(TXT_VALUES eMessage)
{
//    switch(CurrentLanguage())
    {
//      case USE_ENGLISH:
        return (char*)g_sEnglishTxtStrings[eMessage];
//      case USE_CHINESE:
//        return (char*)g_sChineseTxtStrings[eMessage];
    }

//    return "UNKNOWN TEXT";
}

