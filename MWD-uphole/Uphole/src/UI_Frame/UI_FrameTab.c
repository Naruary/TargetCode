/*******************************************************************************
*       @brief      Implementation file for the UI TAB Frames section on the
*                   screen.
*       @file       Uphole/src/UI_Frame/UI_FrameTab.c
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
#include "portable.h"
#include "buzzer.h"
#include "lcd.h"
#include "PeriodicEvents.h"
#include "Systick.h"
#include "TextStrings.h"
#include "LoggingManager.h"
#include "UI_api.h"
#include "UI_Defs.h"
#include "UI_DataStructures.h"
#include "UI_Alphabet.h"
#include "UI_Frame.h"
#include "UI_LCDScreenInversion.h"
#include "UI_ScreenUtilities.h"
#include "UI_Primitives.h"
#include "UI_MainTab.h"
#include "UI_DataTab.h"
#include "UI_JobTab.h"
#include "UI_BoxSetupTab.h"
#include "UI_DownholeTab.h"
#include "Plan_Tab_Graph.h"
#include "Side_Tab_Graph.h"
#include "UI_RecordDataPanel.h"
#include "Graph_Plot.h"
#include "Compass_Plot.h"
#include "FlashMemory.h"
#include "Compass_Panel.h"
#include "UI_SurveyEditPanel.h"
#include "UI_StartNewHoleDecisionPanel.h"
#include "UI_DeleteLastSurveyDecisionPanel.h"
#include "UI_BranchPointSetDecisionPanel.h"
#include "UI_ClearAllHoleDecisionPanel.h"
#include "UI_UpdateDiagnosticDownholeDecisionPanel.h"
#include "Gamma_Compass.h"
#include "Gamma_Tab_Graph.h"
#include "SideGamma_Tab_Graph.h"

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

static void drawTab(TAB_ENTRY* tab);

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

static FRAME_ID m_eActiveTabFrameID;

const TAB_ENTRY* const tabs[] =
{
	&MainTab,
	&DataTab,
	&JobTab,
	&BoxSetupTab,
	&DownholeTab,
	&PlanTab,
	&SideTab,
	&GammaTab,
	&SideGammaTab,
	&GammaCompassTab,
};

#define NUMBER_OF_TABS (sizeof(tabs)/sizeof(TAB_ENTRY const *))

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*******************************************************************************
*       @details
*******************************************************************************/
FRAME_ID GetActiveTabFrame(void)
{
// get here every time the periodic timer in main expires, typ 1 second ehhhh
// whs 27Dec2021 above - ehhh sort of true - u get here by press rt button in UI_FrameLabel.c/line 101
// a periodic event triggers 
	return m_eActiveTabFrameID;
}

/*******************************************************************************
*       @details
*******************************************************************************/
static TAB_ENTRY* GetTab(FRAME_ID frame)
{
	BYTE index = frame - TAB1;
	if (index < 0 || index >= NUMBER_OF_TABS)
	{
		return NULL;
	}
	return (TAB_ENTRY*) tabs[index];
}

/*******************************************************************************
*       @details
*******************************************************************************/
U_BYTE GetTabCount(void)
{
	return NUMBER_OF_TABS;
}

/*******************************************************************************
*       @details
*******************************************************************************/
TAB_ENTRY* GetActiveTab(void)
{
	return (TAB_ENTRY*)GetTab(m_eActiveTabFrameID);
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void TabChanged(void)
{
	RecordDataPanelInit();
	TAB_ENTRY* tab = GetActiveTab();
	tab->Show(tab);
}

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 ; Function:
 ;   TabFrameHandler()
 ;
 ; Description:
 ;   Handles events for the four individual Tabs in the HOME frame
 :  whs 27Dec2021 there are now 10 Tabs
 ;
 ; Parameters:
 ;   pEvent  =>  Periodic event to be handled.
 ;
 ; Reentrancy:
 ;   No.
 ;
 ;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void TabFrameHandler(PERIODIC_EVENT *pEvent)
{
	TAB_ENTRY* tab;
	if(pEvent == NULL)
	{
		return;
	}
	switch (pEvent->Action.eActionType)
	{
		case PUSH:
		{
			BuzzerKeypress();
			M_Turn_LCD_On_And_Reset_Timer();
			switch (pEvent->Action.nValue)
			{
				case BUTTON_RIGHT:
					tab = GetTab((FRAME_ID) (m_eActiveTabFrameID + 1));
					if(tab)
					{
						if(tab->IsVisible(tab))
						{
							m_eActiveTabFrameID++;
							TabChanged();
						}
					}
					break;
				case BUTTON_LEFT:
					if (m_eActiveTabFrameID > TAB1)
					{
						m_eActiveTabFrameID--;
						TabChanged();
					}
					break;
				default:
					tab = GetActiveTab();
					if(tab)
					{
						if (tab->KeyPressed)
						{
							tab->KeyPressed(tab, pEvent->Action.nValue);
						}
					}
					break;
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
void TabInitialize(FRAME* frame)
{
	m_eActiveTabFrameID = TAB1;
}

/*******************************************************************************
*       @details
*******************************************************************************/
void TabPaint(FRAME* frame)
{
	TAB_ENTRY* tab = GetTab(frame->eID);
	UI_ClearLCDArea(&frame->area, LCD_FOREGROUND_PAGE);
	if (tab->IsVisible(tab))
	{
		drawTab(tab);
	}
	if (m_eActiveTabFrameID == tab->frame->eID)
	{
		UI_InvertLCDArea(&frame->area, LCD_FOREGROUND_PAGE);
		PaintNow(&WindowFrame);
	}
	LCD_Refresh(LCD_FOREGROUND_PAGE);
}

/*******************************************************************************
*       @details
*******************************************************************************/
void TabWindowPaint(TAB_ENTRY* tab)
{
	U_BYTE nMenuCount = tab->MenuSize(tab);
	if (nMenuCount >= NUMBER_OF_MENU_POSN)
	{
		nMenuCount = NUMBER_OF_MENU_POSN;
	}
	for (int i = 0; i < nMenuCount; i++)
	{
		PaintNow(GetFrame((FRAME_ID) (LABEL1 + i)));
		PaintNow(GetFrame((FRAME_ID) (VALUE1 + i)));
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 ; Function:
 ;   drawTab()
 ;
 ; Description:
 ;   draws either the quads text or icon
 ;
 ; parameter
 ;    nFrameIndex - deteremines which code to draw
 ;   qString - qString to be drawn
 ;
 ; Reentrancy
 ;    No
 ;
 ;;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static void drawTab(TAB_ENTRY* tab)
{
	FRAME* thisFrame = (FRAME*) tab->frame;
	char* sTxtString = GetTxtString(tab->label);
	UI_DisplayStringCentered(sTxtString, &thisFrame->area);
}

/*******************************************************************************
*       @details
*******************************************************************************/
void MainToParam(void)
{
	TAB_ENTRY* tab = GetTab((FRAME_ID) (m_eActiveTabFrameID + 2));
	if (tab && tab->IsVisible(tab))
	{
		m_eActiveTabFrameID++;
		m_eActiveTabFrameID++;
		TabChanged();
	}
}
