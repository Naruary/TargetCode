/*******************************************************************************
*       @brief      Source file for UI_BooleanField.c.
*       @file       Uphole/src/UI_DataFields/UI_BooleanField.c
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
#include "UI_BooleanField.h"
#include "UI_Alphabet.h"
#include "UI_LCDScreenInversion.h"
#include "UI_Frame.h"
#include "UI_api.h"

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*******************************************************************************
*       @details
*******************************************************************************/
static char* BooleanFormat(MENU_ITEM* item)
{
	return GetTxtString(item->boolean.value ? TXT_ON : TXT_OFF);
}

/*******************************************************************************
*       @details
*******************************************************************************/
void BooleanDisplay(MENU_ITEM* item)
{
	FRAME* frame = (FRAME*) item->valueFrame;

	UI_ClearLCDArea(&frame->area, LCD_FOREGROUND_PAGE);
	if (!item->editing)
	{
		item->boolean.value = item->boolean.GetValue();
	}
	if(item->boolean.value == 1)
	{
		UI_DisplayStringLeftJustified("On", &frame->area);
	}
	else
	{
		UI_DisplayStringLeftJustified("Off", &frame->area);
	}
//	UI_DisplayStringLeftJustified(BooleanFormat(item), &frame->area);
}

/*******************************************************************************
*       @details
*******************************************************************************/
void BooleanBeginEdit(MENU_ITEM* item)
{
	item->editing = true;
}

/*******************************************************************************
*       @details
*******************************************************************************/
static RECT* GetFieldRect(MENU_ITEM* item)
{
	static RECT rect;
	U_INT16 xSize = UI_GetTextSize(BooleanFormat(item));

	rect.ptTopLeft.nCol = item->valueFrame->area.ptTopLeft.nCol;
	rect.ptTopLeft.nRow = item->valueFrame->area.ptTopLeft.nRow + 1;
	rect.ptBottomRight.nCol = item->valueFrame->area.ptTopLeft.nCol + xSize + 2;
	rect.ptBottomRight.nRow = item->valueFrame->area.ptBottomRight.nRow - 2;
	return &rect;
}

/*******************************************************************************
*       @details
*******************************************************************************/
void BooleanHighlight(MENU_ITEM* item)
{
	UI_InvertLCDArea(GetFieldRect(item), LCD_FOREGROUND_PAGE);
}

/*******************************************************************************
*       @details
*******************************************************************************/
void BooleanFinishEdit(MENU_ITEM* item)
{
	item->boolean.SetValue(item->boolean.value);
	item->editing = false;
	item->NextFrame(item);
}

/*******************************************************************************
*       @details
*******************************************************************************/
void BooleanKeyPressed(MENU_ITEM* item, BUTTON_VALUE keyPressed)
{
	switch (keyPressed)
	{
		case BUTTON_DOWN:
			item->boolean.value = !item->boolean.value;
			PaintNow(item->valueFrame);
			break;
		case BUTTON_UP:
			item->boolean.value = !item->boolean.value;
			PaintNow(item->valueFrame);
			break;
		case BUTTON_RIGHT:
			break;
		case BUTTON_LEFT:
			break;
		case BUTTON_PERIOD:
		case BUTTON_DASH:
			break;
		// numeric keys
		default:
			break;
	}
}
