/*******************************************************************************
*       @brief      This module contains common UI utilities that are used by
*                   all the different screen handlers.
*       @file       Uphole/src/UI_Tools/UI_ScreenUtilities.c
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
#include "timer.h"
#include "FlashMemory.h"
#include "UI_Frame.h"
#include "UI_ScreenUtilities.h"
#include "UI_Frame.h"
#include "UI_api.h"

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

// whether or not the startup animation has completed
static BOOL m_bStartupComplete;

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   UI_SetStartupComplete()
;
; Description:
;   sets whether the startup sequence is complete
;
; Parameter:
;   BOOL - true - startup complete.  false - startup no complete
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void UI_SetStartupComplete(BOOL bComplete)
{
	m_bStartupComplete = bComplete;
}

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   UI_StartupComplete()
;
; Description:
;   Gets whether the startup sequence is complete
;
; Return:
;   BOOL - true - startup complete.  false - startup not complete
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
BOOL UI_StartupComplete(void)
{
	return m_bStartupComplete;
}

/*******************************************************************************
*       @details
*******************************************************************************/
BOOL ShowTab(TAB_ENTRY* tab)
{
	return (true);
}

/*******************************************************************************
*       @details
*******************************************************************************/
void UI_Initialize(void)
{
	SetAllowKeypadActions(false);
	UI_SetStartupComplete(false);
	for(FRAME_ID i=NO_FRAME; i<LAST_FRAME; i++)
	{
		FRAME* frame = (FRAME*)GetFrame(i);
		if (frame->Initialize)
		{
			frame->Initialize(frame);
		}
	}
	UI_SetActiveFrame(&StartupFrame);
	AddScreenEvent(ANIMATION, STARTUP, TRIGGER_TIME_NOW);
}

/*******************************************************************************
*       @details
*******************************************************************************/
void EditValue(MENU_ITEM* item)
{
	UI_SetActiveFrame(item->valueFrame);
	SetActiveValueFrame(item->valueFrame->eID);
	if (item->BeginEdit)
	{
		item->highlight = true;
		item->BeginEdit(item);
		AddScreenEvent(BLINK_CURSOR, item->valueFrame->eID, TRIGGER_TIME_NOW);
	}
	PaintNow(item->valueFrame);
}
