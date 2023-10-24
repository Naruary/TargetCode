/*!
********************************************************************************
*       @brief      This file contains the implementation for the Diag
*                   tab on the Uphole LCD screen.
*       @file       Uphole/src/UI_Tabs/UI_DiagTab.c
*       @author     Walter Rodrigues
*       @date       January 2016
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <stdio.h>
#include <stdlib.h>
#include "intf0003.h"
#include "lcd.h"
#include "adc.h"
#include "Manager_DataLink.h"
#include "RASP.h"
#include "TextStrings.h"
#include "UI_Alphabet.h"
#include "UI_ScreenUtilities.h"
#include "UI_LCDScreenInversion.h"
#include "UI_Frame.h"
#include "UI_api.h"
#include "UI_BooleanField.h"
#include "UI_FixedField.h"
#include "UI_StringField.h"
#include "SecureParameters.h"
#include "RecordManager.h"
#include "intf0008.h"

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

///@brief  
///@param  
///@return 
static MENU_ITEM* GetDiagMenuItem(TAB_ENTRY* tab, U_BYTE index);

///@brief  
///@param  
///@return 
static U_BYTE GetDiagMenuSize(TAB_ENTRY* tab);

///@brief  
///@param  
///@return 
static void DiagTabPaint(TAB_ENTRY* tab);

///@brief  
///@param  
///@return 
static void DiagTabMakeRequest(TAB_ENTRY* tab);

///@brief  
///@param  
///@return 
static void DiagTabShow(TAB_ENTRY* tab);

///@brief  
///@param  
///@return 
void ShowStatusMessageTabDiag(char* message1, char* message2);

///@brief  
///@param  
///@return 
void ShowDiagTabInfoMessage(char* message1, char* message2, char* message3);

///@brief  
///@param  
///@return 
void ShowDownholeVoltageTabDiag(char* message1);

///@brief  
///@param  
///@return 
void ShowUpholeVoltageTabDiag(char* message1);


//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

///@brief
const TAB_ENTRY DiagTab = { &TabFrame5, TXT_DIAG, ShowTab, GetDiagMenuItem, GetDiagMenuSize, DiagTabPaint, DiagTabShow, DiagTabMakeRequest};

///@brief
static MENU_ITEM menu[] =
{
    CREATE_FIXED_FIELD(TXT_DOWNHOLE_OFF_TIME,           &LabelFrame1, &ValueFrame1, CurrrentLabelFrame, GetDownholeOffTime, SetDownholeOffTime, 4, 1, 0, 9999), //1000
    CREATE_FIXED_FIELD(TXT_DOWNHOLE_ON_TIME,            &LabelFrame2, &ValueFrame2, CurrrentLabelFrame, GetDownholeOnTime,  SetDownholeOnTime,  4, 1, 0, 9999), //1000
    //CREATE_FIXED_FIELD(TXT_DOWNHOLE_LIFE,               &LabelFrame3, &ValueFrame3, CurrrentLabelFrame, GetDownholeBatteryLife, SetDownholeBatteryLife, 4, 2, 0, 9999), //100
    //CREATE_FIXED_FIELD(TXT_UPHOLE_LIFE,                 &LabelFrame4, &ValueFrame4, CurrrentLabelFrame, GetUpholeBatteryLife, SetUpholeBatteryLife, 4, 2, 0, 9999), //100
    CREATE_BOOLEAN_FIELD(TXT_DOWNHOLE_DEEP_SLEEP,       &LabelFrame3, &ValueFrame3, CurrrentLabelFrame, GetDeepSleepMode,   SetDeepSleepMode),
    CREATE_BOOLEAN_FIELD(TXT_GAMMA_ON_OFF,              &LabelFrame4, &ValueFrame4, CurrrentLabelFrame, GetGammaOnOff,      SetGammaOnOff),
};

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

#define MENU_SIZE (sizeof(menu) / sizeof(MENU_ITEM))
#define BDL_MENU_SIZE 1

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*!
********************************************************************************
*       @details
*******************************************************************************/

static MENU_ITEM* GetDiagMenuItem(TAB_ENTRY* tab, U_BYTE index)
{
    if (index < tab->MenuSize(tab))
    {
        return &menu[index];
    }
    return NULL ;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static U_BYTE GetDiagMenuSize(TAB_ENTRY* tab)
{
    return MENU_SIZE;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void DiagTabPaint(TAB_ENTRY* tab)
{
    char text[21];
    REAL32 PerctSave;
    REAL32 OnTime;
    REAL32 OffTime;
    OnTime = GetDownholeOnTime()*1.0;
    OffTime = GetDownholeOffTime()*1.0;
    TabWindowPaint(tab);
    PerctSave = (OffTime/(OffTime+OnTime))*100.0;

    sprintf(text, "Downhole Voltage          %.1f", GetDownholeBatteryLife());
    ShowDownholeVoltageTabDiag(text);
    sprintf(text, "Uphole Voltage             %.1f", GetUpholeBatteryLife());
    ShowUpholeVoltageTabDiag(text);
    sprintf(text,"Power Saving = %.1f %", PerctSave);
    ShowStatusMessageTabDiag("DEFAULT: OFF Time: 120, ON Time: 60", text);    
    ShowDiagTabInfoMessage("WARNING: Never set ON Time < 60", "Small ON Time disconnects downhole", "Deep Sleep: Use for Night Shutdown");
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void DiagTabMakeRequest(TAB_ENTRY* tab)
{
    MENU_ITEM* time = &menu[0];
    if ((!time->editing) && (UI_GetActiveFrame()->eID != ALERT_FRAME))
    {
        RepaintNow(time->valueFrame);
    }
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void DiagTabShow(TAB_ENTRY* tab)
{
    UI_SetActiveFrame(&LabelFrame1);
    SetActiveLabelFrame(LABEL1);
    PaintNow(&HomeFrame);
}

void ShowStatusMessageTabDiag(char* message1, char* message2)
{
    RECT area;
    const FRAME* frame = &WindowFrame;
    area.ptTopLeft.nCol = frame->area.ptTopLeft.nCol + 5;
    area.ptTopLeft.nRow = frame->area.ptBottomRight.nRow - 80;
    area.ptBottomRight.nCol = frame->area.ptBottomRight.nCol - 5;
    area.ptBottomRight.nRow = area.ptTopLeft.nRow + 15;
    UI_DisplayStringCentered(message1, &area);
    area.ptTopLeft.nRow = frame->area.ptBottomRight.nRow - 50;
    UI_DisplayStringCentered(message2, &area);
}


void ShowDiagTabInfoMessage(char* message1, char* message2, char* message3)
{
    RECT area;
    const FRAME* frame = &WindowFrame;
    area.ptTopLeft.nCol = frame->area.ptTopLeft.nCol + 5;
    area.ptTopLeft.nRow = frame->area.ptBottomRight.nRow - 50;
    area.ptBottomRight.nCol = frame->area.ptBottomRight.nCol - 5;
    area.ptBottomRight.nRow = area.ptTopLeft.nRow + 20;
    UI_DisplayStringCentered(message1, &area);
    area.ptTopLeft.nRow = frame->area.ptBottomRight.nRow - 25;
    UI_DisplayStringCentered(message2, &area);
    area.ptTopLeft.nRow = frame->area.ptBottomRight.nRow - 0;
    UI_DisplayStringCentered(message3, &area);
}


void ShowDownholeVoltageTabDiag(char* message1)
{
    RECT area;
    const FRAME* frame = &WindowFrame;
    area.ptTopLeft.nCol = frame->area.ptTopLeft.nCol + 2;
    area.ptTopLeft.nRow = frame->area.ptBottomRight.nRow - 128;
    area.ptBottomRight.nCol = frame->area.ptBottomRight.nCol - 5;
    area.ptBottomRight.nRow = area.ptTopLeft.nRow + 15;
    UI_DisplayStringLeftJustified(message1, &area);
}

void ShowUpholeVoltageTabDiag(char* message1)
{
    RECT area;
    const FRAME* frame = &WindowFrame;
    area.ptTopLeft.nCol = frame->area.ptTopLeft.nCol + 2;
    area.ptTopLeft.nRow = frame->area.ptBottomRight.nRow - 113;
    area.ptBottomRight.nCol = frame->area.ptBottomRight.nCol - 5;
    area.ptBottomRight.nRow = area.ptTopLeft.nRow + 15;
    UI_DisplayStringLeftJustified(message1, &area);
}