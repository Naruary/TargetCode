/*******************************************************************************
*       @brief      Source file for UI_DateField.c.
*       @file       Uphole/src/UI_DataFields/UI_DateField.c
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
#include "UI_Alphabet.h"
#include "UI_DateField.h"
#include "UI_LCDScreenInversion.h"
#include "UI_Frame.h"
#include "UI_api.h"

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*******************************************************************************
*       @details
*******************************************************************************/
RTC_DateTypeDef* GetCurrentDate(MENU_ITEM* item)
{
	if (!item->editing)
	{
		RTC_GetDate(RTC_Format_BIN, &item->date.date);
	}
	return &item->date.date;
}

/*******************************************************************************
*       @details
*******************************************************************************/
static U_BYTE GetDaysInMonth(U_BYTE month, U_BYTE year)
{
	static const U_BYTE monthdays[] =
	{ 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	U_BYTE daysInMonth = monthdays[month - 1];
	if (month == 2 && !(year % 4))
		daysInMonth++;
	return daysInMonth;
}

/*******************************************************************************
*       @details
*******************************************************************************/
char* DateFormat(RTC_DateTypeDef* date)
{
	static char sBuffer[20];
	snprintf(sBuffer, 20, "%02d/%02d/%02d", date->RTC_Month, date->RTC_Date, (date->RTC_Year % 100));
	return sBuffer;
}

/*******************************************************************************
*       @details
*******************************************************************************/
void DateDisplay(MENU_ITEM* item)
{
	FRAME* thisFrame = (FRAME*)item->valueFrame;
	UI_ClearLCDArea(&thisFrame->area, LCD_FOREGROUND_PAGE);
	UI_DisplayStringLeftJustified(DateFormat(GetCurrentDate(item)), &thisFrame->area);
}

/*******************************************************************************
*       @details
*******************************************************************************/
void DateBeginEdit(MENU_ITEM* item)
{
	GetCurrentDate(item);
	item->editing = true;
	item->date.field = MONTH;
	item->date.position = 0;
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
static RECT* GetCharacterRect(MENU_ITEM* item, U_BYTE index)
{
	static RECT rect;
	RECT* frame = (RECT*)&item->valueFrame->area;
	U_INT16 offset = UI_GetTextSize(BeginString(DateFormat(&item->date.date), index + item->date.position));
	U_INT16 xSize = UI_GetTextSize(BeginString(DateFormat(&item->date.date), index + item->date.position + 1));

	rect.ptTopLeft.nCol = frame->ptTopLeft.nCol + offset;
	rect.ptTopLeft.nRow = frame->ptTopLeft.nRow + 1;
	rect.ptBottomRight.nCol = frame->ptTopLeft.nCol + xSize + 2;
	rect.ptBottomRight.nRow = frame->ptBottomRight.nRow - 2;

	return &rect;
}

/*******************************************************************************
*       @details
*******************************************************************************/
void DateHighlight(MENU_ITEM* item)
{
	switch (item->date.field)
	{
		case MONTH:
			UI_InvertLCDArea(GetCharacterRect(item, 0), LCD_FOREGROUND_PAGE);
			break;
		case DAY:
			UI_InvertLCDArea(GetCharacterRect(item, 3), LCD_FOREGROUND_PAGE);
			break;
		case YEAR:
			UI_InvertLCDArea(GetCharacterRect(item, 6), LCD_FOREGROUND_PAGE);
			break;
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void DecrementValue(U_BYTE position, U_BYTE* value, U_BYTE max, U_BYTE min)
{
	int increment = 1, newValue;
	if (position == 0)
	{
		increment = 10;
	}
	newValue = (int) *value - increment;
	if (newValue >= min)
	{
		*value -= increment;
	}
	if (newValue == -1)
	{
		*value = max;
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void IncrementValue(U_BYTE position, U_BYTE* value, U_BYTE rollover, U_BYTE min)
{
	U_BYTE increment = 1, newValue;
	if (position == 0)
	{
		increment = 10;
	}
	newValue = *value + increment;
	if (newValue <= rollover)
	{
		*value += increment;
	}
	if (newValue == (rollover + 1))
	{
		*value = min;
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
static BOOL NewValue(U_BYTE* value, U_BYTE position, U_BYTE digit, U_BYTE max)
{
	char string[20], strValue[20];
	U_BYTE newValue;

	snprintf(string, 20, "%02d", *value);
	if (position == 0)
	{
		snprintf(strValue, 20, "%1d0", digit);
	}
	else
	{
		snprintf(strValue, 20, "%c%1d", string[0], digit);
	}
	newValue = atoi(strValue);
	if (newValue <= max)
	{
		*value = newValue;
		return true;
	}
	return false;
}

/*******************************************************************************
*       @details
*******************************************************************************/
void DateFinishEdit(MENU_ITEM* item)
{
	RTC_DateTypeDef tempDate;
	RTC_SetDate(RTC_Format_BIN, &item->date.date);
	while (true)
	{
		RTC_GetDate(RTC_Format_BIN, &tempDate);
		if (item->date.date.RTC_Year == tempDate.RTC_Year &&
			item->date.date.RTC_Month == tempDate.RTC_Month &&
			item->date.date.RTC_Date == tempDate.RTC_Date)
		{
			break;
		}
	}
	item->editing = false;
	item->NextFrame(item);
}

/*******************************************************************************
*       @details
*******************************************************************************/
void DayKeyPressed(MENU_ITEM* item, BUTTON_VALUE keyPressed)
{
    switch (keyPressed)
    {
        case BUTTON_RIGHT:
            if (item->date.position < 1)
            {
                item->date.position++;
                item->highlight = true;
            }
            else if (item->date.field < YEAR)
            {
                item->date.field++;
                item->date.position = 0;
                item->highlight = true;
            }
            PaintNow(item->valueFrame);
            break;
        case BUTTON_LEFT:
            if (item->date.position > 0)
            {
                item->date.position--;
                item->highlight = true;
            }
            else if (item->date.field > MONTH)
            {
                item->date.field--;
                item->date.position = 1;
                item->highlight = true;
            }
            PaintNow(item->valueFrame);
            break;
        case BUTTON_DOWN:
            switch (item->date.field)
            {
                case MONTH:
                    DecrementValue(item->date.position, &item->date.date.RTC_Month, 12, 1);
                    break;
                case DAY:
                    DecrementValue(item->date.position, &item->date.date.RTC_Date, GetDaysInMonth(item->date.date.RTC_Month, item->date.date.RTC_Year), 1);
                    break;
                case YEAR:
                    DecrementValue(item->date.position, &item->date.date.RTC_Year, 99, 0);
                    break;
            }
            PaintNow(item->valueFrame);
            break;
        case BUTTON_UP:
            switch (item->date.field)
            {
                case MONTH:
                    IncrementValue(item->date.position, &item->date.date.RTC_Month, 12, 1);
                    break;
                case DAY:
                    IncrementValue(item->date.position, &item->date.date.RTC_Date, GetDaysInMonth(item->date.date.RTC_Month, item->date.date.RTC_Year), 1);
                    break;
                case YEAR:
                    IncrementValue(item->date.position, &item->date.date.RTC_Year, 99, 0);
                    break;
            }
            PaintNow(item->valueFrame);
            break;

        case BUTTON_PERIOD:
        case BUTTON_DASH:
            break;

         // numeric keys
        default:
        {
            BOOL valid = true;
            switch (item->date.field)
            {
                case MONTH:
                    valid = NewValue(&item->date.date.RTC_Month, item->date.position, keyPressed - 48, 12);
                    break;
                case DAY:
                    valid = NewValue(&item->date.date.RTC_Date, item->date.position, keyPressed - 48, GetDaysInMonth(item->date.date.RTC_Month, item->date.date.RTC_Year));
                    break;
                case YEAR:
                    valid = NewValue(&item->date.date.RTC_Year, item->date.position, keyPressed - 48, 23);
                    break;
            }
            if (valid)
            {
                if (item->date.position == 0)
                {
                    item->date.position++;
                }
                else if (item->date.field < YEAR)
                {
                    item->date.position = 0;
                    item->date.field++;
                }
                else
                {
                    item->FinishEdit(item);
                }
                PaintNow(item->valueFrame);
            }
        }
            break;
    }
}