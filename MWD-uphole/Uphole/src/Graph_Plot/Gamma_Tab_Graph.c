/*******************************************************************************
*       @brief      Implementation file for the UI TAB Frames section on the
*                   screen from Plots
*       @file       Uphole/src/Graph_Plot/Gamma_Tab_Graph.c
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
#include "Gamma_Tab_Graph.h"
#include "Gamma_Graph_Plot.h"

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

static MENU_ITEM* GetGammaMenuItem(TAB_ENTRY* tab, U_BYTE index);
static U_BYTE GetGammaMenuSize(TAB_ENTRY* tab);
static void GammaTabPaint(TAB_ENTRY* tab);
static void GammaTabMakeRequest(TAB_ENTRY* tab);
static void GammaTabShow(TAB_ENTRY* tab);

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

const TAB_ENTRY GammaTab = { &TabFrame8, TXT_GAMMAPLT, ShowTab, GetGammaMenuItem, GetGammaMenuSize, GammaTabPaint, GammaTabShow, GammaTabMakeRequest};

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*!
********************************************************************************
*       @details
*******************************************************************************/

static MENU_ITEM* GetGammaMenuItem(TAB_ENTRY* tab, U_BYTE index)
{
    return NULL ;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static U_BYTE GetGammaMenuSize(TAB_ENTRY* tab)
{
     return NULL ;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void GammaTabPaint(TAB_ENTRY* tab)
{
    TabWindowPaint(tab);
    DrawGammaGraph();
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void GammaTabMakeRequest(TAB_ENTRY* tab)
{
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void GammaTabShow(TAB_ENTRY* tab)
{
    PaintNow(&HomeFrame);
}
