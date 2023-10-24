/*******************************************************************************
*       @brief      Implementation file for the Labels within the Window Frame
*       @file       Uphole/src/UI_Frame/UI_FrameLabel.c
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <stm32f4xx.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "portable.h"
#include "buzzer.h"
#include "lcd.h"
#include "PeriodicEvents.h"
#include "RecordManager.h"
#include "Systick.h"
#include "TextStrings.h"
#include "UI_api.h"
#include "UI_Defs.h"
#include "UI_DataStructures.h"
#include "UI_Alphabet.h"
#include "UI_Frame.h"
#include "UI_ScreenUtilities.h"
#include "UI_Primitives.h"
#include "UI_LCDScreenInversion.h"
#include "Graph_Plot.h"
#include "LoggingManager.h"
#include "FlashMemory.h"
#include "Compass_Panel.h"
#include "UI_SurveyEditPanel.h"
#include "UI_StartNewHoleDecisionPanel.h"
#include "UI_DeleteLastSurveyDecisionPanel.h"
#include "UI_BranchPointSetDecisionPanel.h"
#include "UI_ClearAllHoleDecisionPanel.h"
#include "UI_UpdateDiagnosticDownholeDecisionPanel.h"

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

static FRAME_ID m_eActiveLabelFrameID;

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*!
********************************************************************************
*       @details
*******************************************************************************/

void SetActiveLabelFrame(FRAME_ID frame)
{
    m_eActiveLabelFrameID = frame;
}

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 ; Function:
 ;   LabelFrameHandler()
 ;
 ; Description:
 ;   Handles events for the LABEL frame
 ;
 ; Parameters:
 ;   pEvent  =>  Periodic event to be handled.
 ;
 ; Reentrancy:
 ;   No.
 ;
 ;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void LabelFrameHandler(PERIODIC_EVENT *pEvent)
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
			switch (pEvent->Action.nValue)
			{
				case BUTTON_LEFT:
				case BUTTON_RIGHT:
					bSilenceBuzzer = true;
					AddButtonEventWithFrame(
						(pEvent->Action.nValue),
						GetActiveTabFrame());
					break;
				case BUTTON_DOWN:
					if((m_eActiveLabelFrameID - LABEL1) < tab->MenuSize(tab) - 1)
					{
						FRAME_ID oldLabel = m_eActiveLabelFrameID;
						m_eActiveLabelFrameID++;
						PaintNow(GetFrame(oldLabel));
						PaintNow(GetFrame(m_eActiveLabelFrameID));
					}
					else
					{
						if(tab)
						{
							if(tab->KeyPressed)
							{
								tab->KeyPressed(tab, BUTTON_DOWN);
							}
						}
					}
					break;
				case BUTTON_UP:
					if ((m_eActiveLabelFrameID > LABEL1) && (tab->MenuSize(tab) > m_eActiveLabelFrameID - LABEL1))
					{
						FRAME_ID oldLabel = m_eActiveLabelFrameID;
						m_eActiveLabelFrameID--;
						PaintNow(GetFrame(oldLabel));
						PaintNow(GetFrame(m_eActiveLabelFrameID));
					}
					else
					{
						if(tab)
						{
							if (tab->KeyPressed)
							{
								tab->KeyPressed(tab, BUTTON_UP);
							}
						}
					}
					break;
				case BUTTON_SELECT:
					if(tab->MenuSize(tab) > 0 && m_eActiveLabelFrameID != NO_FRAME)
					{
						MENU_ITEM* item = tab->MenuItem(tab, m_eActiveLabelFrameID - LABEL1);
						item->Selected(item);
					}
					break;
				case BUTTON_SHIFT:
                                        if (GetGammaPoweredState()) 
                                        {
                                          // If Gamma is powered ON (assuming the function returns a non-zero value when ON)
        
                                          SetLoggingState(COMPASS_LOGGING);
                                          setCompassDecisionPanelActive(true);
                                          setSurveyEditPanelActive(false);
                                          setClearAllHoleDataDecisionPanelActive(false);
                                          setBranchPointSetDecisionPanelActive(false);
                                          setStartNewHoleDecisionPanelActive(false);
                                          setClearAllHoleDataDecisionPanelActive(false);
                                          setDeleteLastSurveyDecisionPanelActive(false);
                                          } 
                                        else 
                                          { 
                                            // If Gamma is not powered ON
                                            if (tab->MenuSize(tab) > 0 && m_eActiveLabelFrameID != NO_FRAME) 
                                              {
                                                MENU_ITEM* item = tab->MenuItem(tab, m_eActiveLabelFrameID - LABEL1);
                                                item->Selected(item);
                                              }
                                          }
                                       break;
				default:
					if(tab)
					{
						if(tab->KeyPressed)
						{
							tab->KeyPressed(tab, pEvent->Action.nValue);
						}
					}
					break;
			}
			if (!bSilenceBuzzer)
			{
				//Turn on the LCD backlight for any activity.
				BuzzerKeypress();
				M_Turn_LCD_On_And_Reset_Timer();
			}
			break;
		}
		case NO_ACTION:
		default:
			break;
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
void LabelInitialize(FRAME* frame)
{
	SetActiveLabelFrame(NO_FRAME);
}

/*******************************************************************************
*       @details
*******************************************************************************/
void LabelPaint(FRAME* frame)
{
	TAB_ENTRY* tab = GetActiveTab();
	MENU_ITEM *item = tab->MenuItem(tab, frame->eID - LABEL1);
	UI_ClearLCDArea(&frame->area, LCD_FOREGROUND_PAGE);
	UI_DisplayStringLeftJustified(GetTxtString(item->label), &frame->area);
	if (m_eActiveLabelFrameID == frame->eID)
	{
		UI_InvertLCDArea(&frame->area, LCD_FOREGROUND_PAGE);
	}
	LCD_Refresh(LCD_FOREGROUND_PAGE);
}
