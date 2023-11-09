/*******************************************************************************
*       @brief      This file contains the implementation for the Job
*                   tab on the Uphole LCD screen.
*       @file       Uphole/src/UI_Tabs/UI_JobTab.c
*       @date       July 2014
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
#include "lcd.h"
#include "Manager_DataLink.h"
#include "TextStrings.h"
#include "UI_Alphabet.h"
#include "UI_ScreenUtilities.h"
#include "UI_LCDScreenInversion.h"
#include "UI_Frame.h"
#include "UI_api.h"
#include "UI_BooleanField.h"
#include "UI_ToolFacePanels.h"
#include "UI_InitiateField.h"
#include "UI_FixedField.h"
#include "UI_StringField.h"
#include "UI_JobTab.h"
#include "RecordManager.h"
#include "FlashMemory.h"

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

static MENU_ITEM* GetJobMenuItem(TAB_ENTRY* tab, U_BYTE index);
static U_BYTE GetJobMenuSize(TAB_ENTRY* tab);
static void JobTabPaint(TAB_ENTRY* tab);
static void JobTabMakeRequest(TAB_ENTRY* tab);
static void JobTabShow(TAB_ENTRY* tab);
//void ShowJobTabInfoMessage(char* message1, char* message2, char* message3);
//static void ShowStatusMessageTabJob(char* message);
ANGLE_TIMES_TEN GetCorrectToolFaceValue(void);
//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

const TAB_ENTRY JobTab = { &TabFrame3, TXT_PARAM, ShowTab, GetJobMenuItem, GetJobMenuSize, JobTabPaint, JobTabShow, JobTabMakeRequest};

static MENU_ITEM menu[] =
{
	CREATE_STRING_FIELD(TXT_HOLE_NAME,			&LabelFrame1, &ValueFrame1,
		CurrrentLabelFrame, GetBoreholeName,      SetBoreholeName),
	CREATE_FIXED_FIELD(TXT_SETUP_PIPE_LENGTH,	&LabelFrame2, &ValueFrame2,
		CurrrentLabelFrame, GetDefaultPipeLength, SetDefaultPipeLength, 2, 0, 0, 99), // 500
	CREATE_FIXED_FIELD(TXT_DECLINATION,			&LabelFrame3, &ValueFrame3,
		CurrrentLabelFrame, GetDeclination,       SetDeclination,       4, 1, -999, 999), // -25.0 to +25.0
	CREATE_FIXED_FIELD(TXT_DESIRED_AZIMUTH ,	&LabelFrame4, &ValueFrame4,
		CurrrentLabelFrame, GetDesiredAzimuth,    SetDesiredAzimuth,    4, 1, 0, 9999), // 3600 (THREE_SIXTY_TIMES_TEN)
	CREATE_FIXED_FIELD(TXT_ENTER_TOOLFACE,	&LabelFrame5, &ValueFrame5,
		CurrrentLabelFrame, GetCorrectToolFaceValue,    SetToolface,    4, 1, 0, 9999),
	CREATE_MENU_ITEM(TXT_SET_TOOLFACE_ZERO, &LabelFrame6, SetToolFaceZeroFinalValue),
	CREATE_MENU_ITEM(TXT_CLEAR_TOOLFACE_ZERO, &LabelFrame7, ClearToolFaceZero),
	CREATE_MENU_ITEM(TXT_USB_CRF, &LabelFrame8, CreateFile),  // whs 27Jan2022 dumps all stored shots to the USB
	CREATE_MENU_ITEM(TXT_DATA_UPLOAD, &LabelFrame9, UploadFile) //ZD 21September2023 UI Upload file option. This will allow a user to actually select this as an option within the tab and can start an event chain.
};

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

#define MENU_SIZE (sizeof(menu) / sizeof(MENU_ITEM))

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*******************************************************************************
*       @details
*******************************************************************************/
static MENU_ITEM* GetJobMenuItem(TAB_ENTRY* tab, U_BYTE index)
{
	if (index < tab->MenuSize(tab))
	{
		return &menu[index];
	}
	return NULL ;
}

/*******************************************************************************
*       @details
*******************************************************************************/
static U_BYTE GetJobMenuSize(TAB_ENTRY* tab)
{
	return MENU_SIZE;
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void JobTabPaint(TAB_ENTRY* tab)
{
	char text[100];

	TabWindowPaint(tab);
        
	if (((float)GetToolFaceValue()/10.0f) > 360.0f)     
	{   // the above line and below .. if else statement was added 19Nov2019 whs
        snprintf(text, 100, "%.1f",((double)GetToolFaceValue()/10.0f) - 360.0f);
	}
	else
	{   //previously only the below line was here before 19Nov2019 whs
		snprintf(text, 100, "%.1f",(double)GetToolFaceValue()/10.0f);
	}   // without the added statements the Uphole Job Tab LCD would show > 360 degrees
//	ShowStatusMessageTabJobLocate(text, (nMenuCount * 15)+4 );
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void JobTabMakeRequest(TAB_ENTRY* tab)
{
	MENU_ITEM* time = &menu[0];
	if ((!time->editing) && (UI_GetActiveFrame()->eID != ALERT_FRAME))
	{
		RepaintNow(time->valueFrame);
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void JobTabShow(TAB_ENTRY* tab)
{
	UI_SetActiveFrame(&LabelFrame1);
	SetActiveLabelFrame(LABEL1);
	PaintNow(&HomeFrame);
}

#if 0
/*******************************************************************************
*       @details
*******************************************************************************/
void ShowStatusMessageTabJob(char* message)
{
	RECT area;
	const FRAME* frame = &WindowFrame;
	area.ptTopLeft.nCol = frame->area.ptTopLeft.nCol + 5;
	area.ptTopLeft.nRow = frame->area.ptBottomRight.nRow - 50;
	area.ptBottomRight.nCol = frame->area.ptBottomRight.nCol - 5;
	area.ptBottomRight.nRow = area.ptTopLeft.nRow + 15;
	UI_DisplayStringCentered(message, &area);
}
#endif


/*******************************************************************************
*       @details
*******************************************************************************/
void SetDefaultPipeLength(INT16 length)
{
	// pipe length is stored in feet
	NVRAM_data.nDefaultPipeLengthFeet = length;
}

/*******************************************************************************
*       @details
*******************************************************************************/
INT16 GetDefaultPipeLength(void)
{
	// pipe length is stored in feet
	INT16 value;
	value = NVRAM_data.nDefaultPipeLengthFeet;
	if(value<0) value=0;
	return value;
}

/*******************************************************************************
*       @details
*******************************************************************************/
INT16 GetDesiredAzimuth(void)
{
	return NVRAM_data.nDesiredAzimuth;
}

/*******************************************************************************
*       @details
*******************************************************************************/
void SetDesiredAzimuth(INT16 value)
{
		NVRAM_data.nDesiredAzimuth = value;
}
/*******************************************************************************
*       @details
*******************************************************************************/
void SetDeclination(INT16 length)
{
	NVRAM_data.nDeclination = length;
}

/*******************************************************************************
*       @details
*******************************************************************************/
INT16 GetDeclination(void)
{
	return NVRAM_data.nDeclination;
}

/*******************************************************************************
*       @details
*******************************************************************************/
void SetBoreholeName(char* value)
{
	strncpy((char *)NVRAM_data.sBoreholeName, value, MAX_BOREHOLE_NAME_BYTES);
}

/*******************************************************************************
*       @details
*******************************************************************************/
char* GetBoreholeName(void)
{
	BOOL endFound = false;
	for(int i=0; i<MAX_BOREHOLE_NAME_BYTES; i++)
	{
		if (NVRAM_data.sBoreholeName[i] == 0)
		{
			endFound = true;
		}
	}
	if (!endFound || NVRAM_data.sBoreholeName[0] == 0)
	{
		strcpy((char *)NVRAM_data.sBoreholeName, "HOLE           "); //eventually need to remove trailing spaces
	}
	return (char *)NVRAM_data.sBoreholeName;
}

ANGLE_TIMES_TEN GetCorrectToolFaceValue(void)
{
    if ((GetToolFaceValue()/10) > 360)
	{
		return GetToolFaceValue() - 3600;
	}     
	else
	{
		return GetToolFaceValue();
	}
}

