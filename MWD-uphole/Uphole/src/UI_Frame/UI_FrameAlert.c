/*******************************************************************************
*       @brief      Implementation file for the Home screen in its entirety and
*                   for each individual quad.
*       @file       Uphole/src/UI_Frame/UI_FrameAlert.c
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <stm32f4xx.h>
#include <stdio.h>
#include "portable.h"
#include "BMP.h"
#include "buzzer.h"
#include "lcd.h"
#include "Manager_DataLink.h"
#include "PeriodicEvents.h"
#include "RecordManager.h"
#include "Systick.h"
#include "TextStrings.h"
#include "UI_api.h"
#include "UI_Defs.h"
#include "UI_DataStructures.h"
#include "UI_Frame.h"
#include "UI_ScreenUtilities.h"
#include "UI_Alphabet.h"
#include "UI_LCDScreenInversion.h"
#include "UI_Primitives.h"

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

static void drawAlertBorder(FRAME* frame);

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

static FRAME* m_nHiddenFrame;
static TXT_VALUES m_eAlertMessage;

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*******************************************************************************
*       @details
*******************************************************************************/
/*
 ;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 ; Function:
 ;   StatusFrameHandler()
 ;
 ; Description:
 ;   Handles events for the STATUS frame
 ;
 ; Parameters:
 ;   pEvent  =>  Periodic event to be handled.
 ;
 ; Reentrancy:
 ;   No.
 ;
 ;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void AlertFrameHandler(PERIODIC_EVENT *pEvent)
{
//	TAB_ENTRY* tab;

	if (pEvent == NULL)
	{
		return;
	}

	switch (pEvent->Action.eActionType)
	{
		case PUSH:
//			tab = GetActiveTab();
			switch (pEvent->Action.nValue)
			{
				case BUTTON_SELECT:
					UI_SetActiveFrame(m_nHiddenFrame);
					RepaintNow(&WindowFrame);
					break;
                                case BUTTON_SHIFT:
                          		UI_SetActiveFrame(m_nHiddenFrame);
					RepaintNow(&WindowFrame);
					break;
			}
			BuzzerKeypress();
			M_Turn_LCD_On_And_Reset_Timer();
			break;
		case ALERT:
			m_nHiddenFrame = UI_GetActiveFrame();
			m_eAlertMessage = pEvent->Action.eMessage;
			UI_SetActiveFrame(&AlertFrame);
			AlertPaint((FRAME*)&AlertFrame);
			BuzzerAlarm();
			break;
		case NO_ACTION:
		default:
			break;
    }
}

/*******************************************************************************
*       @details
*******************************************************************************/
void AlertPaint(FRAME* thisFrame)
{
	UI_ClearLCDArea(&thisFrame->area, LCD_FOREGROUND_PAGE);

	drawAlertBorder(thisFrame);

	RECT thisRect = thisFrame->area;

	thisRect.ptTopLeft.nCol += 7;
	thisRect.ptTopLeft.nRow += 7;

	UI_DisplayStringCentered(GetTxtString(m_eAlertMessage), &thisRect);
	LCD_Refresh(LCD_FOREGROUND_PAGE);
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void drawAlertBorder(FRAME* frame)
{
	U_BYTE nThickness = 5;
	RECT thisRect;
	memcpy(&thisRect, &frame->area, sizeof(RECT));

	while(nThickness-- != 0)
	{
		UI_DrawRectangle(thisRect, LCD_FOREGROUND_PAGE);
		thisRect.ptTopLeft.nRow++;
		thisRect.ptTopLeft.nCol++;
		thisRect.ptBottomRight.nRow--;
		thisRect.ptBottomRight.nCol--;
	}
}
