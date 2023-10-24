/*******************************************************************************
*       @brief      This file contains the implementation for the UpdateDownhole Timeout
*                   Panel.
*       @file       Uphole/src/UI_Panels/UpdateDownholeTimeoutPanel.c
*       @date       January 2016
*       @copyright  COPYRIGHT (c) 2016 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <string.h>
#include <stdio.h>
#include "LoggingManager.h"
#include "RecordManager.h"
#include "UI_Alphabet.h"
#include "UI_ScreenUtilities.h"
#include "UI_Frame.h"
#include "UI_LCDScreenInversion.h"
#include "UI_api.h"
#include "UI_MainTab.h"
#include "UpdateDownholeTimeoutPanel.h"

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

static void UpdateDownholeTimeoutPaint(TAB_ENTRY* tab);

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

PANEL UpdateDownholeTimeout = { GetEmptyMenu, 0, UpdateDownholeTimeoutPaint };

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void UpdateDownholeTimeoutPaint(TAB_ENTRY* tab)
{
    TabWindowPaint(tab);
    ShowOperationStatusMessage("Unable to Update Downhole");
}
