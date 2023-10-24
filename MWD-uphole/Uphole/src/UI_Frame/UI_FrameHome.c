/*******************************************************************************
*       @brief      Implementation file for the HOME Frame.
*       @file       Uphole/src/UI_Frame/UI_FrameHome.c
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <stdbool.h>
#include "portable.h"
#include "buzzer.h"
#include "lcd.h"
#include "PeriodicEvents.h"
#include "Systick.h"
#include "TextStrings.h"
#include "UI_api.h"
#include "UI_Defs.h"
#include "UI_DataStructures.h"
#include "UI_Frame.h"
#include "UI_ScreenUtilities.h"
#include "UI_Primitives.h"

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

static void drawHomeBorder(void);

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
 ;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 ; Function:
 ;   HomeFrameHandler()
 ;
 ; Description:
 ;   Handles events for the HOME frame
 ;
 ; Parameters:
 ;   pEvent  =>  Periodic event to be handled.
 ;
 ; Reentrancy:
 ;   No.
 ;
 ;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
void HomeFrameHandler(PERIODIC_EVENT *pEvent)
{
    if (pEvent == NULL)
    {
        return;
    }

    switch (pEvent->Action.eActionType)
    {
        case PUSH:
        {
            BOOL bSilenceBuzzer = false;
            TAB_ENTRY* tab = GetActiveTab();
//            switch (pEvent->Action.ButtonPush.eButtonType)
            {
//                case KEYPAD:
//                    switch (pEvent->Action.ButtonPush.eDepressType)
                    {
//                        case SHORT_DEPRESS:
                            if (tab->KeyPressed)
                            {
                                tab->KeyPressed(tab, pEvent->Action.nValue);
                            }
//                            break;
                    }
//                    break;
            }

            if (!bSilenceBuzzer)
            {
                //Turn on the LCD backlight for any activity.
                BuzzerKeypress();
                M_Turn_LCD_On_And_Reset_Timer();
            }
        }
            break;

        case NO_ACTION:
        default:
            break;
    }
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

void HomePaint(FRAME* frame)
{
    clearLCD();
    LCD_Refresh(LCD_FOREGROUND_PAGE);
    LCD_Refresh(LCD_BACKGROUND_PAGE);
    drawHomeBorder();
    for (U_BYTE i = 0; i < GetTabCount(); i++)
    {
        PaintNow(tabs[i]->frame);
    }
    PaintNow(&StatusFrame);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
 ;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 ; Function:
 ;   drawHomeBorder()
 ;
 ; Description:
 ;   Handles drawing the home border
 ;
 ; Parameters:
 ;   pEvent  =>  Periodic event to be handled.
 ;
 ; Reentrancy:
 ;   No.
 ;
 ;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
static void drawHomeBorder(void)
{
    RECT rect;
    memcpy(&rect, &HomeFrame.area, sizeof(RECT));
    UI_DrawRectangle(rect, LCD_BACKGROUND_PAGE);

    rect.ptTopLeft.nRow++;
    rect.ptTopLeft.nCol++;
    rect.ptBottomRight.nRow--;
    rect.ptBottomRight.nCol--;
    UI_DrawRectangle(rect, LCD_BACKGROUND_PAGE);
    LCD_Refresh(LCD_BACKGROUND_PAGE);
}
