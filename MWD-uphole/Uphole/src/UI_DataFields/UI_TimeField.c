/*******************************************************************************
*       @brief      Source file for UI_TimeField.c.
*       @file       Uphole/src/UI_DataFields/UI_TimeField.c
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
#include "UI_TimeField.h"
#include "UI_Alphabet.h"
#include "UI_LCDScreenInversion.h"
#include "UI_Frame.h"
#include "UI_api.h"

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*!
********************************************************************************
*       @details
*******************************************************************************/

static RTC_TimeTypeDef* GetCurrentTime(MENU_ITEM* item)
{
    if (!item->editing)
    {
        item->time.GetTime(&item->time.time);
    }
    return &item->time.time;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

char* TimeFormat(RTC_TimeTypeDef* now)
{
    static char sBuffer[20];
    snprintf(sBuffer, 20, "%02d:%02d:%02d", now->RTC_Hours, now->RTC_Minutes, now->RTC_Seconds);
    return sBuffer;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

void TimeDisplay(MENU_ITEM* item)
{
    FRAME* thisFrame = (FRAME*)item->valueFrame;
    UI_ClearLCDArea(&thisFrame->area, LCD_FOREGROUND_PAGE);
    UI_DisplayStringLeftJustified(TimeFormat(GetCurrentTime(item)), &thisFrame->area);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

void TimeBeginEdit(MENU_ITEM* item)
{
    item->time.GetTime(&item->time.time);
    item->editing = true;
    item->time.position = 0;
    item->time.field = HOURS;
}

/*!
********************************************************************************
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

/*!
********************************************************************************
*       @details
*******************************************************************************/

static RECT* GetCharacterRect(MENU_ITEM* item, U_BYTE index)
{
    static RECT rect;
    RECT* frame = (RECT*)&item->valueFrame->area;
    U_INT16 offset = UI_GetTextSize(BeginString(TimeFormat(&item->time.time), index + item->time.position));
    U_INT16 xSize = UI_GetTextSize(BeginString(TimeFormat(&item->time.time), index + item->time.position + 1));

    rect.ptTopLeft.nCol = frame->ptTopLeft.nCol + offset;
    rect.ptTopLeft.nRow = frame->ptTopLeft.nRow + 1;
    rect.ptBottomRight.nCol = frame->ptTopLeft.nCol + xSize + 2;
    rect.ptBottomRight.nRow = frame->ptBottomRight.nRow - 2;

    return &rect;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

void TimeHighlight(MENU_ITEM* item)
{
    switch (item->time.field)
    {
        case HOURS:
            UI_InvertLCDArea(GetCharacterRect(item, 0), LCD_FOREGROUND_PAGE);
            break;
        case MINUTES:
            UI_InvertLCDArea(GetCharacterRect(item, 3), LCD_FOREGROUND_PAGE);
            break;
        case SECONDS:
            UI_InvertLCDArea(GetCharacterRect(item, 6), LCD_FOREGROUND_PAGE);
            break;
    }
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void DecrementValue(U_BYTE position, U_BYTE* value, U_BYTE rollover)
{
    int increment = 1, newValue;
    if (position == 0)
    {
        increment = 10;
    }
    newValue = (int) *value - increment;
    if (newValue >= 0)
    {
        *value = newValue;
    }
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void IncrementValue(U_BYTE position, U_BYTE* value, U_BYTE rollover)
{
    U_BYTE increment = 1, newValue;
    if (position == 0)
    {
        increment = 10;
    }
    newValue = *value + increment;
    if (newValue <= rollover)
    {
        *value = newValue & 0xFF;
    }
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static BOOL NewValue(U_BYTE* value, U_BYTE position, U_BYTE digit, U_BYTE rollover)
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
    if (newValue <= rollover)
    {
        *value = newValue;
        return true;
    }
    return false;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/
void TimeFinishEdit(MENU_ITEM* item)
{
    item->time.SetTime(&item->time.time);
    item->editing = false;
    item->NextFrame(item);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/
void TimeKeyPressed(MENU_ITEM* item, BUTTON_VALUE keyPressed)
{
    switch (keyPressed)
    {
        case BUTTON_RIGHT:
            if (item->time.position < 1)
            {
                item->time.position++;
                item->highlight = true;
            }
            else if (item->time.field < SECONDS)
            {
                item->time.field++;
                item->time.position = 0;
                item->highlight = true;
            }
            PaintNow(item->valueFrame);
            break;
        case BUTTON_LEFT:
            if (item->time.position > 0)
            {
                item->time.position--;
                item->highlight = true;
            }
            else if (item->time.field > HOURS)
            {
                item->time.field--;
                item->time.position = 1;
                item->highlight = true;
            }
            PaintNow(item->valueFrame);
            break;
        case BUTTON_DOWN:
            switch (item->time.field)
            {
                case HOURS:
                    DecrementValue(item->time.position, &item->time.time.RTC_Hours, 23);
                    break;
                case MINUTES:
                    DecrementValue(item->time.position, &item->time.time.RTC_Minutes, 59);
                    break;
                case SECONDS:
                    DecrementValue(item->time.position, &item->time.time.RTC_Seconds, 59);
                    break;
            }
            PaintNow(item->valueFrame);
            break;
        case BUTTON_UP:
            switch (item->time.field)
            {
                case HOURS:
                    IncrementValue(item->time.position, &item->time.time.RTC_Hours, 23);
                    break;
                case MINUTES:
                    IncrementValue(item->time.position, &item->time.time.RTC_Minutes, 59);
                    break;
                case SECONDS:
                    IncrementValue(item->time.position, &item->time.time.RTC_Seconds, 59);
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
            switch (item->time.field)
            {
                case HOURS:
                    valid = NewValue(&item->time.time.RTC_Hours, item->time.position, keyPressed - 48, 23);
                    break;
                case MINUTES:
                    valid = NewValue(&item->time.time.RTC_Minutes, item->time.position, keyPressed - 48, 59);
                    break;
                case SECONDS:
                    valid = NewValue(&item->time.time.RTC_Seconds, item->time.position, keyPressed - 48, 59);
                    break;
            }
            if (valid)
            {
                if (item->time.position == 0)
                {
                    item->time.position++;
                }
                else if (item->time.field < SECONDS)
                {
                    item->time.position = 0;
                    item->time.field++;
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
