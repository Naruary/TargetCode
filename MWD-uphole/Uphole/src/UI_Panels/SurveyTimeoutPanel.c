/*******************************************************************************
*       @brief      This file contains the implementation for the Survey Timeout
*                   Panel.
*       @file       Uphole/src/UI_Panels/SurveyTimeoutPanel.c
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
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

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

static void SurveyTimeoutPaint(TAB_ENTRY* tab);

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

PANEL SurveyTimeout = { GetEmptyMenu, 0, SurveyTimeoutPaint };

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void SurveyTimeoutPaint(TAB_ENTRY* tab)
{
    TabWindowPaint(tab);
    ShowOperationStatusMessage("Unable to Read Sensors, Try Again");
}
