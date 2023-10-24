/*!
********************************************************************************
*       @brief      Source file for UI_BooleanField.c.
*                   Documented by Josh Masters Dec. 2014
*       @file       Uphole/src/DataFields/UI_BooleanField.c
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
#include "SecureParameters.h"
#include "UI_BooleanField.h"
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

static char* BooleanFormat(MENU_ITEM* item)
{
    return GetTxtString(item->bool.value ? TXT_ON : TXT_OFF);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

void BooleanDisplay(MENU_ITEM* item)
{
    FRAME* frame = (FRAME*) item->valueFrame;

    UI_ClearLCDArea(&frame->area, LCD_FOREGROUND_PAGE);
    if (!item->editing)
    {
        item->bool.value = item->bool.GetValue();
    }
    UI_DisplayStringLeftJustified(BooleanFormat(item), &frame->area);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

void BooleanBeginEdit(MENU_ITEM* item)
{
    item->editing = TRUE;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static RECT* GetFieldRect(MENU_ITEM* item)
{
    static RECT rect;
    U_BYTE xSize = UI_GetTextSize(BooleanFormat(item));

    rect.ptTopLeft.nCol = item->valueFrame->area.ptTopLeft.nCol;
    rect.ptTopLeft.nRow = item->valueFrame->area.ptTopLeft.nRow + 1;
    rect.ptBottomRight.nCol = item->valueFrame->area.ptTopLeft.nCol + xSize + 2;
    rect.ptBottomRight.nRow = item->valueFrame->area.ptBottomRight.nRow - 2;
    return &rect;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

void BooleanHighlight(MENU_ITEM* item)
{
    UI_InvertLCDArea(GetFieldRect(item), LCD_FOREGROUND_PAGE);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

void BooleanFinishEdit(MENU_ITEM* item)
{
    item->bool.SetValue(item->bool.value);
    item->editing = FALSE;
    item->NextFrame(item);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

void BooleanKeyPressed(MENU_ITEM* item, BUTTON_VALUE keyPressed)
{
    switch (keyPressed)
    {
        case BUTTON_DOWN:
            item->bool.value = !item->bool.value;
            PaintNow(item->valueFrame);
            break;
        case BUTTON_UP:
            item->bool.value = !item->bool.value;
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