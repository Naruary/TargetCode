/*******************************************************************************
*       @brief      Source file for UI_StringField.c.
*       @file       Uphole/src/UI_DataFields/UI_StringField.c
*       @date       December 2014
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
#include "FlashMemory.h"
#include "UI_StringField.h"
#include "UI_Alphabet.h"
#include "UI_LCDScreenInversion.h"
#include "UI_Frame.h"
#include "UI_api.h"

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

typedef struct _CHARACTER_WHEEL_
{
    U_BYTE Minimum;
    U_BYTE Maximum;
    U_BYTE Current;
    U_BYTE (*NextCharacter)(struct _CHARACTER_WHEEL_* wheel);
    U_BYTE (*PreviousCharacter)(struct _CHARACTER_WHEEL_* wheel);
} CHARACTER_WHEEL;

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

static U_BYTE Next(CHARACTER_WHEEL* wheel);
static U_BYTE Previous(CHARACTER_WHEEL* wheel);

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

static CHARACTER_WHEEL Editor = {32, 125, 65, Next, Previous}; //176 previously which incudes chinese characters
#if 0
static char wheel1[] = {'A','B','C','D','E','F','G','H','I','J','K','L','M',\
	'N','O','P','Q','R','S','T','U','V','W','X','Y','Z','0','1','2','3','4',\
	'5','6','7','8','9','-','+','_','#','$'};
#endif
//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*******************************************************************************
*       @details
*******************************************************************************/
static U_BYTE Next(CHARACTER_WHEEL* wheel)
{
	if (wheel->Current < wheel->Maximum - 1)
	{
		wheel->Current++;
	}
	else
	{
		wheel->Current = wheel->Minimum;
	}
	return wheel->Current;
}

/*******************************************************************************
*       @details
*******************************************************************************/
static U_BYTE Previous(CHARACTER_WHEEL* wheel)
{
	if (wheel->Current > wheel->Minimum + 1)
	{
		wheel->Current--;
	}
	else
	{
		wheel->Current = wheel->Maximum;
	}
	return wheel->Current;
}

/*******************************************************************************
*       @details
*******************************************************************************/
void StringDisplay(MENU_ITEM* item)
{
	FRAME* frame = (FRAME*)item->valueFrame;

	UI_ClearLCDArea(&frame->area, LCD_FOREGROUND_PAGE);
	if (!item->editing)
	{
		strcpy(item->string.value, item->string.GetValue());
	}
	UI_DisplayStringLeftJustified(item->string.value, &frame->area);
}

/*******************************************************************************
*       @details
*******************************************************************************/
void StringBeginEdit(MENU_ITEM* item)
{
	item->editing = true;
	item->string.position = 0;
	strcpy(item->string.value, item->string.GetValue());
}

/*******************************************************************************
*       @details
*******************************************************************************/
static char* BeginString(char* string, int endIndex)
{
	static char before[9];
	before[endIndex] = 0;
	if (endIndex > 0)
	{
		strncpy(before, string, endIndex);
	}
	return before;
}

/*******************************************************************************
*       @details
*******************************************************************************/
static RECT* GetCharacterRect(MENU_ITEM* item)
{
	static RECT rect;
	RECT* frame = (RECT*)&item->valueFrame->area;
	U_INT16 offset = UI_GetTextSize(BeginString(item->string.value, item->string.position));
	U_INT16 xSize = UI_GetTextSize(BeginString(item->string.value, item->string.position + 1));
	rect.ptTopLeft.nCol = frame->ptTopLeft.nCol + offset;
	rect.ptTopLeft.nRow = frame->ptTopLeft.nRow + 1;
	rect.ptBottomRight.nCol = frame->ptTopLeft.nCol + xSize + 2;
	rect.ptBottomRight.nRow = frame->ptBottomRight.nRow - 2;
	return &rect;
}

/*******************************************************************************
*       @details
*******************************************************************************/
void StringFinishEdit(MENU_ITEM* item)
{
	item->string.SetValue(item->string.value);
	item->editing = false;
	item->NextFrame(item);
}

/*******************************************************************************
*       @details
*******************************************************************************/
void StringKeyPressed(MENU_ITEM* item, BUTTON_VALUE keyPressed)
{
	int stringLength = strlen(item->string.value);

	switch (keyPressed)
	{
		case BUTTON_DOWN:
			Editor.Current = item->string.value[item->string.position];
			item->string.value[item->string.position] = Editor.PreviousCharacter(&Editor);
			PaintNow(item->valueFrame);
			break;
		case BUTTON_UP:
			Editor.Current = item->string.value[item->string.position];
			item->string.value[item->string.position] = Editor.NextCharacter(&Editor);
			PaintNow(item->valueFrame);
			break;
		case BUTTON_RIGHT:
			if (item->string.position < stringLength - 1)
			{
				item->string.position++;
				Editor.Current = item->string.value[item->string.position];
			}
			else if (item->string.position < MAX_STRING_CHARS - 2)
			{
				item->string.position++;
				item->string.value[item->string.position] = Editor.Current;
				item->string.value[item->string.position + 1] = 0;
			}
			else
			{
				item->string.position = 0;
				Editor.Current = item->string.value[item->string.position];
			}
			PaintNow(item->valueFrame);
			break;
		case BUTTON_LEFT:
			if (item->string.position == 0)
			{
				if (stringLength > 0)
				{
					item->string.position = stringLength - 1;
				}
			}
			else
			{
				item->string.position--;
			}
			Editor.Current = item->string.value[item->string.position];
			PaintNow(item->valueFrame);
			break;
		case BUTTON_DASH:
			if (item->string.position == stringLength - 1)
			{
				item->string.value[item->string.position] = 0;
				if (item->string.position > 0)
				{
					item->string.position--;
				}
			}
			else if (stringLength > 0)
			{
				item->string.value[item->string.position] = 0;
				strcat(item->string.value, item->string.value + item->string.position + 1);
				item->string.position--;
			}
			PaintNow(item->valueFrame);
			break;
		case BUTTON_PERIOD:
			break;
		// numeric keys
		default:
			break;
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
void StringHighlight(MENU_ITEM* item)
{
	UI_InvertLCDArea(GetCharacterRect(item), LCD_FOREGROUND_PAGE);
}
