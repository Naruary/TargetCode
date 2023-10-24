/*******************************************************************************
*       @brief      Implementation file for the Window Frame
*       @file       Uphole/src/UI_Frame/UI_FrameWindow.c
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
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

static void drawWindowBorder(FRAME* frame);

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 ; Function:
 ;   WindowFrameHandler()
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
 ;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void WindowFrameHandler(PERIODIC_EVENT *pEvent)
{
	if (pEvent == NULL)
	{
		return;
	}
	switch (pEvent->Action.eActionType)
	{
		case PUSH:
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
void WindowPaint(FRAME* frame)
{
	TAB_ENTRY* tab = GetActiveTab();
	UI_ClearLCDArea(&frame->area, LCD_FOREGROUND_PAGE);
	drawWindowBorder(frame);
	tab->Paint(tab);
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void drawWindowBorder(FRAME* frame)
{
	RECT thisRect;
	memcpy(&thisRect, &frame->area, sizeof(RECT));
	UI_DrawRectangle(thisRect, LCD_FOREGROUND_PAGE);
	LCD_Refresh(LCD_FOREGROUND_PAGE);
}
