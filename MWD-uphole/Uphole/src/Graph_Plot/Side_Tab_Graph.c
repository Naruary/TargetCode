/*!
********************************************************************************
*       @brief      Implementation file for the UI TAB Frames section on the
*                   screen from Plots
*       @file       Uphole/src/Graph_Plot/Side_Tab_Graph.c
*       @date       Feb 2016
*       @copyright  COPYRIGHT (c) 2016 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <stdio.h>
#include <stdlib.h>
#include "lcd.h"
#include "adc.h"
#include "Manager_DataLink.h"
#include "TextStrings.h"
#include "UI_Alphabet.h"
#include "UI_ScreenUtilities.h"
#include "UI_LCDScreenInversion.h"
#include "UI_Frame.h"
#include "UI_api.h"
#include "UI_BooleanField.h"
#include "UI_FixedField.h"
#include "UI_StringField.h"
#include "RecordManager.h"
#include "Graph_Plot.h"
#include "UI_Alphabet.h"
#include "Side_Graph_Plot.h"

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

static MENU_ITEM* GetSideMenuItem(TAB_ENTRY* tab, U_BYTE index);
static U_BYTE GetSideMenuSize(TAB_ENTRY* tab);
static void SideTabPaint(TAB_ENTRY* tab);
static void SideTabMakeRequest(TAB_ENTRY* tab);
static void SideTabShow(TAB_ENTRY* tab);

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

const TAB_ENTRY SideTab = { &TabFrame7, TXT_SIDE, ShowTab, GetSideMenuItem, GetSideMenuSize, SideTabPaint, SideTabShow, SideTabMakeRequest};

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*!
********************************************************************************
*       @details
*******************************************************************************/

static MENU_ITEM* GetSideMenuItem(TAB_ENTRY* tab, U_BYTE index)
{
    return NULL ;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static U_BYTE GetSideMenuSize(TAB_ENTRY* tab)
{
     return NULL ;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void SideTabPaint(TAB_ENTRY* tab)
{
    TabWindowPaint(tab);
    DrawSideGraph();
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void SideTabMakeRequest(TAB_ENTRY* tab)
{
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void SideTabShow(TAB_ENTRY* tab)
{
    PaintNow(&HomeFrame);
}
