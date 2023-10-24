/*******************************************************************************
*       @brief      This file contains the implementation for the Start New Hole
*                   Decision Panel on the Main tab. That gives a option to select Yes/No
*       @file       Uphole/src/Compass_Panel.c
*       @date       Feb 2016
*       @copyright  COPYRIGHT (c) 2015 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "LoggingManager.h"
#include "RecordManager.h"
#include "UI_Alphabet.h"
#include "UI_ScreenUtilities.h"
#include "UI_Frame.h"
#include "UI_LCDScreenInversion.h"
#include "UI_api.h"
#include "Compass_Panel.h"
#include "UI_RecordDataPanel.h"
#include "UI_DataTab.h"
#include "SysTick.h"
#include "Graph_Plot.h"
#include "Compass_Plot.h"





#include "UI_ToolFacePanels.h"
#include "Compass_Panel.h"
#include "UI_DownholeMainPanel.h"
#include "TargetProtocol.h"
#include "GammaSensor.h"

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

static void Compass_Decision_Paint(TAB_ENTRY* tab);
static void Compass_Decision_Show(TAB_ENTRY* tab);
static void Compass_Decision_KeyPressed(TAB_ENTRY* tab, BUTTON_VALUE key);
static void Compass_Decision_TimerElapsed(TAB_ENTRY* tab);



//static MENU_ITEM* GetMenu(U_BYTE index);
//#define MENU_SIZE (sizeof(menuGamma) / sizeof(MENU_ITEM))

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

BOOL Compass_DecisionPanelActive = false;
static TIME_LR tCallCompass = 0;
PANEL Compass_DecisionPanel = {GetEmptyMenu, 0, Compass_Decision_Paint, Compass_Decision_Show, Compass_Decision_KeyPressed, Compass_Decision_TimerElapsed};

//GetEmptyMenu
//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//







/*******************************************************************************
*       @details
*******************************************************************************/
void setCompassDecisionPanelActive(BOOL bFlag)
{
    Compass_DecisionPanelActive = bFlag;
}

/*******************************************************************************
*       @details
*******************************************************************************/
BOOL getCompassDecisionPanelActive(void)
{
    return Compass_DecisionPanelActive;
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void Compass_Decision_Paint(TAB_ENTRY* tab)
{
//	tCallCompass = ElapsedTimeLowRes(0);
//	DrawCompass();
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void Compass_Decision_Show(TAB_ENTRY* tab)
{
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void Compass_Decision_KeyPressed(TAB_ENTRY* tab, BUTTON_VALUE key)
{
	switch(key)
	{
		case BUTTON_DASH:
			setCompassDecisionPanelActive(false);
			break;
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void Compass_Decision_TimerElapsed(TAB_ENTRY* tab)
{
    DrawCompass();
}

/*******************************************************************************
*       @details
*******************************************************************************/
TIME_LR GetCompassTimer(void)
{
	return tCallCompass;
}

