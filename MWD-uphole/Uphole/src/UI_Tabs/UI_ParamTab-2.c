/*!
********************************************************************************
*       @brief      This file contains the implementation for the Param
*                   tab on the Uphole LCD screen.
*       @file       Uphole/src/UI_Tabs/UI_ParamTab.c
*       @author     Josh Masters
*       @date       July 2014
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

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

///@brief  
///@param  
///@return 
static MENU_ITEM* GetParamMenuItem(TAB_ENTRY* tab, U_BYTE index);

///@brief  
///@param  
///@return 
static U_BYTE GetParamMenuSize(TAB_ENTRY* tab);

///@brief  
///@param  
///@return 
static void ParamTabPaint(TAB_ENTRY* tab);

///@brief  
///@param  
///@return 
static void ParamTabMakeRequest(TAB_ENTRY* tab);

///@brief  
///@param  
///@return 
static void ParamTabShow(TAB_ENTRY* tab);

///@brief  
///@param  
///@return 
void ShowParamTabInfoMessage(char* message1, char* message2, char* message3);


//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

///@brief
const TAB_ENTRY ParamTab = { &TabFrame3, TXT_PARAM, ShowTab, GetParamMenuItem, GetParamMenuSize, ParamTabPaint, ParamTabShow, ParamTabMakeRequest};

///@brief
static MENU_ITEM menu[] =
{
    CREATE_STRING_FIELD(TXT_HOLE_NAME,         &LabelFrame1, &ValueFrame1, CurrrentLabelFrame, GetBoreholeName,      SetBoreholeName),
    CREATE_FIXED_FIELD(TXT_SETUP_PIPE_LENGTH,  &LabelFrame2, &ValueFrame2, CurrrentLabelFrame, GetDefaultPipeLength, SetDefaultPipeLength, 4, 1,     0, 9999), //1000
    CREATE_FIXED_FIELD(TXT_DECLINATION,        &LabelFrame3, &ValueFrame3, CurrrentLabelFrame, GetDeclination,       SetDeclination,       4, 1, -9999, 9999), //-1800 to +1800
    CREATE_FIXED_FIELD(TXT_DESIRED_AZIMUTH ,   &LabelFrame4, &ValueFrame4, CurrrentLabelFrame, GetDesiredAzimuth,    SetDesiredAzimuth,    4, 1,     0, 9999), // 3600
    CREATE_FIXED_FIELD(TXT_TOOLFACE_OFFSET,    &LabelFrame5, &ValueFrame5, CurrrentLabelFrame, GetToolface,          SetToolface,          4, 1,     0, 9999), // 3600

    //new input fields created for 3 extra variables that needed to be customized in the UI
    CREATE_FIXED_FIELD(TXT_90_ERROR,           &LabelFrame6, &ValueFrame6, CurrrentLabelFrame, Get90DegErr,          Set90DegErr,          4, 1, -9999, 9999), // -20.0 to  20.0
    CREATE_FIXED_FIELD(TXT_270_ERROR,          &LabelFrame7, &ValueFrame7, CurrrentLabelFrame, Get270DegErr,         Set270DegErr,         4, 1, -9999, 9999), // -20.0 to  20.0
    CREATE_FIXED_FIELD(TXT_MAX_ERROR,          &LabelFrame8, &ValueFrame8, CurrrentLabelFrame, GetMaxErr,            SetMaxErr,            4, 1,     0, 9999), //   0.0 to 360.0
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

static MENU_ITEM* GetParamMenuItem(TAB_ENTRY* tab, U_BYTE index)
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

static U_BYTE GetParamMenuSize(TAB_ENTRY* tab)
{
    return MENU_SIZE;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void ParamTabPaint(TAB_ENTRY* tab)
{
    TabWindowPaint(tab);
    if(InitNewHole_KeyPress() || IsClearHoleSelected())
    {
      ShowStatusMessageTabParam("REMEMBER: SET CORRECT PARAMETERS");
    }
    
    ShowParamTabInfoMessage("WARNING: changing parameters values", "between surveys incorrectly affects", "calculations in an irreversible way!");
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void ParamTabMakeRequest(TAB_ENTRY* tab)
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

static void ParamTabShow(TAB_ENTRY* tab)
{
    UI_SetActiveFrame(&LabelFrame1);
    SetActiveLabelFrame(LABEL1);
    PaintNow(&HomeFrame);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

void ShowStatusMessageTabParam(char* message)
{
    RECT area;
    const FRAME* frame = &WindowFrame;
    area.ptTopLeft.nCol = frame->area.ptTopLeft.nCol + 5;
    area.ptTopLeft.nRow = frame->area.ptBottomRight.nRow - 70;
    area.ptBottomRight.nCol = frame->area.ptBottomRight.nCol - 5;
    area.ptBottomRight.nRow = area.ptTopLeft.nRow + 15;
    UI_DisplayStringCentered(message, &area);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

void ShowParamTabInfoMessage(char* message1, char* message2, char* message3)
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