/*******************************************************************************
*       @brief      Implementation file for the UI TAB Frames section on the
*                   screen from Plots
*       @file       Uphole/src/Graph_Plot/SideGamma_Tab_Graph.c
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
#include "SideGamma_Graph_Plot.h"

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

static MENU_ITEM* GetSideGammaMenuItem(TAB_ENTRY* tab, U_BYTE index);
static U_BYTE GetSideGammaMenuSize(TAB_ENTRY* tab);
static void SideGammaTabPaint(TAB_ENTRY* tab);
static void SideGammaTabMakeRequest(TAB_ENTRY* tab);
static void SideGammaTabShow(TAB_ENTRY* tab);

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

const TAB_ENTRY SideGammaTab = { &TabFrame9, TXT_SIDEGAMMA, ShowTab, GetSideGammaMenuItem, GetSideGammaMenuSize, SideGammaTabPaint, SideGammaTabShow, SideGammaTabMakeRequest};

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*******************************************************************************
*       @details
*******************************************************************************/
static MENU_ITEM* GetSideGammaMenuItem(TAB_ENTRY* tab, U_BYTE index)
{
    return NULL ;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static U_BYTE GetSideGammaMenuSize(TAB_ENTRY* tab)
{
     return NULL ;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void SideGammaTabPaint(TAB_ENTRY* tab)
{
    TabWindowPaint(tab);
    DrawSideGammaGraph();
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void SideGammaTabMakeRequest(TAB_ENTRY* tab)
{
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void SideGammaTabShow(TAB_ENTRY* tab)
{
    PaintNow(&HomeFrame);
}
