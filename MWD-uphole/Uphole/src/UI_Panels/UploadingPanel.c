/*******************************************************************************
*       @brief      This file contains the implementation for the Uploading
*                   Panel.
*       @file       Uphole/src/UI_Panels/UploadingPanel.c
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
#include "UI_DataTab.h"

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

static void UploadingPaint(TAB_ENTRY* tab);

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

PANEL Uploading = { GetEmptyMenu, 0, UploadingPaint };

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void UploadingPaint(TAB_ENTRY* tab)
{
    char strValue[64];
    TabWindowPaint(tab);
    if (GetLastRecordNumber() == 0)
    {
        snprintf(strValue, 64, "%s ...", GetTxtString(TXT_MAIN_GET_HOLE_DATA));
    }
    else
    {
        snprintf(strValue, 64, "Loading Surveys");
    }
    ShowStatusMessage(strValue);
}
