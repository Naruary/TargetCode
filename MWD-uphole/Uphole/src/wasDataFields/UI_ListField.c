/*!
********************************************************************************
*       @brief      Source file for UI_ListField.c.
*                   Documented by Josh Masters Dec. 2014
*       @file       Uphole/src/DataFields/UI_ListField.c
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
#include "UI_ListField.h"
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

static char* ListFormat(MENU_ITEM* item)
{
    for(U_BYTE i=0; i<item->list.count; i++)
    {
        if (item->list.list[i].value == item->list.value)
        {
            return GetTxtString(item->list.list[i].textStringIndex);
        }
    }
    return "???";
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

void ListDisplay(MENU_ITEM* item)
{
    FRAME* frame = (FRAME*)item->valueFrame;

    UI_ClearLCDArea(&frame->area, LCD_FOREGROUND_PAGE);
    if (item->list.value == 0xff)
    {
        item->list.value = item->list.GetValue();
    }
    UI_DisplayStringLeftJustified(ListFormat(item), &frame->area);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

void ListBeginEdit(MENU_ITEM* item)
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
    RECT* frame = (RECT*)&item->valueFrame->area;
    U_BYTE xSize = UI_GetTextSize(ListFormat(item));

    rect.ptTopLeft.nCol = frame->ptTopLeft.nCol;
    rect.ptTopLeft.nRow = frame->ptTopLeft.nRow + 1;
    rect.ptBottomRight.nCol = frame->ptTopLeft.nCol + xSize + 2;
    rect.ptBottomRight.nRow = frame->ptBottomRight.nRow - 2;

    return &rect;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static U_BYTE GetValueIndex(LIST_DATA* list)
{
    for(U_BYTE i=0; i<list->count; i++)
    {
        if (list->list[i].value == list->value)
        {
            return i;
        }
    }
    return 0;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

void ListFinishEdit(MENU_ITEM* item)
{
    item->list.SetValue(item->list.value);
    item->editing = FALSE;
    item->NextFrame(item);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

void ListKeyPressed(MENU_ITEM* item, BUTTON_VALUE keyPressed)
{
    switch (keyPressed)
    {
        case BUTTON_DOWN:
        {
            U_BYTE index = GetValueIndex(&item->list);
            if (index < (item->list.count - 1))
            {
                item->list.value = item->list.list[index + 1].value;
            }
            else
            {
                item->list.value = item->list.list[0].value;
            }
            PaintNow(item->valueFrame);
        }
            break;
        case BUTTON_UP:
        {
            U_BYTE index = GetValueIndex(&item->list);
            if (index > 0)
            {
                item->list.value = item->list.list[index - 1].value;
            }
            else
            {
                item->list.value = item->list.list[item->list.count - 1].value;
            }
            PaintNow(item->valueFrame);
        }
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

/*!
********************************************************************************
*       @details
*******************************************************************************/

void ListHighlight(MENU_ITEM* item)
{
    UI_InvertLCDArea(GetFieldRect(item), LCD_FOREGROUND_PAGE);
}