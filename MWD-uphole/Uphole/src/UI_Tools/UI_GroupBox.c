/*******************************************************************************
*       @brief      This file contains the implementation for the Group Box
*                   pseudo class for displaying boxes of data.
*       @file       Uphole/src/UI_Tools/UI_GroupBox.c
*       @date       August 2014
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
#include "SysTick.h"
#include "RecordManager.h"
#include "FlashMemory.h"
#include "LoggingManager.h"
#include "Manager_DataLink.h"
#include "UI_Alphabet.h"
#include "UI_ScreenUtilities.h"
#include "UI_Frame.h"
#include "UI_LCDScreenInversion.h"
#include "UI_api.h"
#include "UI_MainTab.h"
#include "UI_Primitives.h"
#include "UI_GroupBox.h"
#include "UI_BoxSetupTab.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

#define DISPLAY_FIELD_HEIGHT 15
#define FIELD_SEPARATOR ":"
#define TITLE_SIZE 16

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

static char* ToUppercase(char* string, char* s);
static U_INT16 GetMaxLabelSize(GroupBox* box);

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

static char strValue[30];
static char titleUpper[TITLE_SIZE];

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*******************************************************************************
*       @details
*******************************************************************************/
char* DisplayInt16Value(DisplayField* field)
{
	if (LoggingManager_IsConnected()) // whs 10Dec2021 yitran modem is connected to downhole
	{
		if(GetSurveyCommsState() == false)
		{
			// if comms but no survey data yet, dashes
			return "---";
		}
		else
		{
			INT16 Value = field->int16();
			snprintf(strValue, 30, "%4.1f", (REAL32)Value / 10.0);
			return strValue;
		}
	}
	// if no comms at all, exxes
	return "xxx";
}
// whs 3Dec2021 added the following Display fields to output XXXs in place of data on check survey panel
// had some issues stabbed it out .. left here for future ref.  used by MWD_LoggingPanel.c 
#if 0 
char* DisplayInt16ValueXXX(DisplayField* field)
{
	return "xxx";
}

char* DisplayIntU16ValueXXX(DisplayField* field)
{
	return "xxx";
}
  
#endif
          
/*******************************************************************************
*       @details
*******************************************************************************/
char* DisplayUint16Value(DisplayField* field)
{
	if (LoggingManager_IsConnected()) //whs 10Dec2021 yitan modem is connected to the downhole
	{
		if(GetSurveyCommsState() == false)
		{
			// if comms but no survey data yet, dashes
			return "---";
		}
		else
		{
			snprintf(strValue, 30, "%d", field->uint16());
			return strValue;
		}
	}
	return "xxx"; 	// if no comms at all, exxes
}

/*******************************************************************************
*       @details
*******************************************************************************/
char* DisplayReal32Value(DisplayField* field)
{
	snprintf(strValue, 30, "%4.1f", field->real32());
	return strValue;
}

/*******************************************************************************
*       @details
*******************************************************************************/
char* DisplaySurveyInt16Value(DisplayField* field)
{
	snprintf(strValue, 30, "%d", field->int16());
	return strValue;
}

/*******************************************************************************
*       @details
*******************************************************************************/
char* DisplayUint32Value(DisplayField* field)
{
	snprintf(strValue, 30, "%d", field->uint32());
	return strValue;
}

/*******************************************************************************
*       @details
*******************************************************************************/
static char* ToUppercase(char* string, char* s)
{
	memset(s, 0, TITLE_SIZE);
	for (int i = 0; string[i]; i++)
	{
		if ((string[i] >= 'a') && (string[i] <= 'z'))
		{
			s[i] = string[i] - ('a' - 'A');
		}
		else
		{
			s[i] = string[i];
		}
	}
	return s;
}

/*******************************************************************************
*       @details
*******************************************************************************/
static U_INT16 GetMaxLabelSize(GroupBox* box)
{
	int loopy;
	U_INT16 maxSize = 0;
	U_INT16 labelSize;

	for(loopy = 0; loopy < MAX_FIELDS; loopy++)
	{
		if(box->Fields[loopy].Label == 0)
		{
			break;
		}
		labelSize = UI_GetTextSize(GetTxtString(box->Fields[loopy].Label));
		if (labelSize > maxSize)
		{
			maxSize = labelSize;
		}
	}
	return maxSize;
}

/*******************************************************************************
*       @details
*******************************************************************************/
void GroupBoxPaint(GroupBox* box)
{
	int loopy;
	U_INT16 labelSize = GetMaxLabelSize(box) + 6;
	U_INT16 separatorSize = UI_GetTextSize(FIELD_SEPARATOR);
	char* title = ToUppercase(GetTxtString(box->Title), titleUpper);
	U_INT16 titleSize = UI_GetTextSize(title);
	RECT fieldArea;
	fieldArea.ptTopLeft.nRow = box->Area.ptTopLeft.nRow + (DISPLAY_FIELD_HEIGHT / 2) + 3;
	fieldArea.ptBottomRight.nRow = box->Area.ptTopLeft.nRow + DISPLAY_FIELD_HEIGHT;
	fieldArea.ptTopLeft.nCol = box->Area.ptTopLeft.nCol;
	fieldArea.ptBottomRight.nCol = (box->Area.ptTopLeft.nCol + box->Area.ptBottomRight.nCol) / 2;
	for(loopy = 0; loopy < MAX_FIELDS && box->Fields[loopy].Label != 0; loopy++)
	{
		fieldArea.ptTopLeft.nCol = box->Area.ptTopLeft.nCol;
		fieldArea.ptBottomRight.nCol = box->Area.ptTopLeft.nCol + labelSize - 1;
		UI_ClearLCDArea(&fieldArea, LCD_FOREGROUND_PAGE);
		UI_DisplayStringRightJustified(GetTxtString(box->Fields[loopy].Label), &fieldArea);

		fieldArea.ptTopLeft.nCol = fieldArea.ptBottomRight.nCol + 2;
		fieldArea.ptBottomRight.nCol = fieldArea.ptTopLeft.nCol + separatorSize;
		UI_ClearLCDArea(&fieldArea, LCD_FOREGROUND_PAGE);
		UI_DisplayStringLeftJustified(FIELD_SEPARATOR, &fieldArea);

		fieldArea.ptTopLeft.nCol = fieldArea.ptBottomRight.nCol + 2;
		fieldArea.ptBottomRight.nCol = box->Area.ptBottomRight.nCol;
		UI_ClearLCDArea(&fieldArea, LCD_FOREGROUND_PAGE);
		UI_DisplayStringLeftJustified(box->Fields[loopy].GetFormattedValue(&box->Fields[loopy]), &fieldArea);

		fieldArea.ptTopLeft.nRow += DISPLAY_FIELD_HEIGHT;
		fieldArea.ptBottomRight.nRow += DISPLAY_FIELD_HEIGHT;
	}
	UI_DrawRectangle(box->Area, LCD_FOREGROUND_PAGE);

	fieldArea.ptTopLeft.nRow = box->Area.ptTopLeft.nRow - (DISPLAY_FIELD_HEIGHT / 2);
	fieldArea.ptBottomRight.nRow = fieldArea.ptTopLeft.nRow + DISPLAY_FIELD_HEIGHT;
	fieldArea.ptTopLeft.nCol = box->Area.ptTopLeft.nCol + 5;
	fieldArea.ptBottomRight.nCol = fieldArea.ptTopLeft.nCol + titleSize + 5;
	UI_ClearLCDArea(&fieldArea, LCD_FOREGROUND_PAGE);
	UI_DisplayStringLeftJustified(title, &fieldArea);
}
