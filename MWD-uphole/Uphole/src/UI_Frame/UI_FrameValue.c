/*******************************************************************************
*       @brief      Implementation file for the Labels within the Window Frame
*       @file       Uphole/src/UI_Frame/UI_FrameValue.c
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

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
#include "UI_LCDScreenInversion.h"
#include "UI_ScreenUtilities.h"
#include "UI_Primitives.h"

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

// "Highlighted" value in terms of FRAME_ID
static FRAME_ID m_eActiveValueFrame;

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*******************************************************************************
*       @details
*******************************************************************************/
void SetActiveValueFrame(FRAME_ID eNewFrame)
{
	m_eActiveValueFrame = eNewFrame;
}

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 ; Function:
 ;   ValueFrameHandler()
 ;
 ; Description:
 ;   Handles events for the VALUE frame
 ;
 ; Parameters:
 ;   pEvent  =>  Periodic event to be handled.
 ;
 ; Reentrancy:
 ;   No.
 ;
 ;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void ValueFrameHandler(PERIODIC_EVENT *pEvent)
{
    TAB_ENTRY* tab;
    MENU_ITEM* item;

    tab = GetActiveTab();
    
    if (pEvent == NULL)
    {
        return;
    }

    switch (pEvent->Action.eActionType)
    {
#if UI_FIXED_NEW_WAY == 1
        case SCREEN:
            switch (pEvent->Action.ScreenTask)
            {
                case BLINK_CURSOR:
                    if (m_eActiveValueFrame != NO_FRAME)
                    {
                        item = tab->MenuItem(tab, m_eActiveValueFrame - VALUE1);
                        if (item->editing && item->HighlightValue)
                        {
                            item->highlight = !item->highlight;
                            AddScreenEvent(BLINK_CURSOR, item->valueFrame->eID, item->highlight ? 700 : 300);
                            PaintNow(item->valueFrame);
                        }
                    }
                    break;
                default:
                    break;
            }
            break;
#endif
        case PUSH:
            item = tab->MenuItem(tab, m_eActiveValueFrame - VALUE1);
            
            if (pEvent->Action.nValue == BUTTON_SELECT || pEvent->Action.nValue == BUTTON_SHIFT)
            {
                item->FinishEdit(item);
                PaintNow(&WindowFrame);
            }
            else if (item->KeyPressed)
            {
                item->KeyPressed(item, pEvent->Action.nValue);
            }
            
            BuzzerKeypress();
            M_Turn_LCD_On_And_Reset_Timer();
            break;

        case NO_ACTION:
        default:
            break;
    }
}

/*******************************************************************************
*       @details
*******************************************************************************/
void ValueInitialize(FRAME* frame)
{
	m_eActiveValueFrame = NO_FRAME;
}

/*******************************************************************************
*       @details
*******************************************************************************/
void CurrrentLabelFrame(MENU_ITEM* item)
{
        UI_SetActiveFrame(item->labelFrame);
	SetActiveLabelFrame(item->labelFrame->eID);
	SetActiveValueFrame(NO_FRAME);
}

/*******************************************************************************
*       @details
*******************************************************************************/
void ValuePaint(FRAME* frame)
{
	TAB_ENTRY* tab = GetActiveTab();
	if(tab==NULL) return;
	MENU_ITEM* item = tab->MenuItem(tab, frame->eID - VALUE1);
	if(item==NULL) return;
	UI_ClearLCDArea(&frame->area, LCD_FOREGROUND_PAGE);
	if( (item->DisplayValue) )//&& (!item->editing) )
	{
		UI_ClearLCDArea(&frame->area, LCD_FOREGROUND_PAGE);
		item->DisplayValue(item);
	}
//cle
//if(item->editing==false)
{
	if (m_eActiveValueFrame == frame->eID)
	{
		if (item->editing && item->HighlightValue && item->highlight)
		{
			item->HighlightValue(item);
		}
	}
}
	LCD_Refresh(LCD_FOREGROUND_PAGE);
}
