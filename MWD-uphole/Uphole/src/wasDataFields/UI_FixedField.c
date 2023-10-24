/*!
********************************************************************************
*       @brief      Source file for UI_FixedField.c.
*                   Documented by Josh Masters Dec. 2014
*       @file       Uphole/src/DataFields/UI_FixedField.c
*       @author     Chris Walker
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <stdio.h>
#include <stdlib.h>
#include "rtc.h"
#include "FixedPointValue.h"
#include "SecureParameters.h"
#include "UI_FixedField.h"
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

void FixedDisplay(MENU_ITEM* item)
{
    FRAME* frame = (FRAME*)item->valueFrame;
    UI_ClearLCDArea(&frame->area, LCD_FOREGROUND_PAGE);
    if (!item->editing)
    {
        item->fixed.value = item->fixed.GetValue();
    }
    UI_DisplayStringLeftJustified(FixedValueFormat(&item->fixed), &frame->area);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

void FixedBeginEdit(MENU_ITEM* item)
{
    char* value = FixedValueFormat(&item->fixed);

    item->editing = TRUE;
    item->fixed.position = 0;
    for(int i=0; item->fixed.position < item->fixed.numberDigits; i++)
    {
        if (value[i] == '-' )
        {
            continue;
        }
        if (value[i] == ' ')
        {
            item->fixed.position++;
            continue;
        }
        break;
    }
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
    U_BYTE offset = UI_GetTextSize(BeginString(FixedValueFormat(&item->fixed), index));
    U_BYTE xSize = UI_GetTextSize(BeginString(FixedValueFormat(&item->fixed), index + 1));

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

void FixedHighlight(MENU_ITEM* item)
{
    int position = item->fixed.position;
    if (position >= item->fixed.numberDigits - item->fixed.fractionDigits)
    {
        position++;
    }
    if (item->fixed.value < 0)
    {
        position++;
    }
    UI_InvertLCDArea(GetCharacterRect(item, position), LCD_FOREGROUND_PAGE);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

void FixedFinishEdit(MENU_ITEM* item)
{
    item->fixed.SetValue(item->fixed.value);
    item->editing = FALSE;
    item->NextFrame(item);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

void FixedKeyPressed(MENU_ITEM* item, BUTTON_VALUE keyPressed)
{
    switch (keyPressed)
    {
    case BUTTON_DOWN:
        FixedValueDecrement(&item->fixed);
        PaintNow(item->valueFrame);
        break;
    case BUTTON_UP:
        FixedValueIncrement(&item->fixed);
        PaintNow(item->valueFrame);
        break;

    case BUTTON_RIGHT:
        FixedValueNextPosition(&item->fixed);
        PaintNow(item->valueFrame);
        break;
    case BUTTON_LEFT:
        FixedValuePrevPosition(&item->fixed);
        PaintNow(item->valueFrame);
        break;

    case BUTTON_PERIOD:
        break;

    case BUTTON_DASH:
        if (item->fixed.minValue < 0)
        {
            INT32 newValue = -item->fixed.value;
            if (VALID_VALUE(&item->fixed, newValue))
            {
                item->fixed.value = newValue;
                PaintNow(item->valueFrame);
            }
        }
        break;

        // numeric keys
    default:
        if (FixedValueEdit(&item->fixed, DIGIT_VALUE(keyPressed)))
        {
            if (item->fixed.position < MAX_POSITION(&item->fixed))
            {
                item->fixed.position++;
            }
            else
            {
                item->FinishEdit(item);
            }
            PaintNow(item->valueFrame);
        }
        break;
    }
}