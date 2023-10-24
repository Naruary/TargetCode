/*!
********************************************************************************
*       @brief      This file contains the implementation for the Setup
*                   tab on the Uphole LCD screen.
*       @file       Uphole/src/UI_Tabs/UI_SetupTab.c
*       @author     Chris Walker
*       @author     Josh Masters
*       @date       July 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <string.h>
#include "ModemDriver.h"
#include "RecordManager.h"
#include "SecureParameters.h"
#include "TextStrings.h"
#include "UI_ScreenUtilities.h"
#include "UI_Frame.h"
#include "UI_api.h"
#include "UI_TimeField.h"
#include "UI_DateField.h"
#include "UI_BooleanField.h"
#include "UI_FixedField.h"
#include "UI_ListField.h"

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

///@brief  
///@param  
///@return 
static void GetTime(RTC_TimeTypeDef* time);

///@brief  
///@param  
///@return 
static void SetTime(RTC_TimeTypeDef* time);

///@brief  
///@param  
///@return 
static void GetDate(RTC_DateTypeDef* date);

///@brief  
///@param  
///@return 
static void SetDate(RTC_DateTypeDef* date);

///@brief  
///@param  
///@return 
static void SetLanguage(U_BYTE newValue);

///@brief  
///@param  
///@return 
static U_BYTE GetLanguage(void);

///@brief  
///@param  
///@return 
static MENU_ITEM* GetSetupMenuItem(TAB_ENTRY* tab, U_BYTE index);

///@brief  
///@param  
///@return 
static U_BYTE GetSetupMenuSize(TAB_ENTRY* tab);

///@brief  
///@param  
///@return 
static void SetupTabRefresh(TAB_ENTRY* tab);

///@brief  
///@param  
///@return 
static void SetupTabShow(TAB_ENTRY* tab);

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

///@brief
const TAB_ENTRY SetupTab = { &TabFrame4, TXT_SETUP, ShowTab, GetSetupMenuItem, GetSetupMenuSize, TabWindowPaint, SetupTabShow, SetupTabRefresh };

///@brief
static LIST_ITEM languages[] =
{
    { USE_ENGLISH, TXT_ENGLISH },
    //{ USE_CHINESE, TXT_CHINESE }, 
};

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

#define NUM_LANGUAGES (sizeof(languages)/sizeof(LIST_ITEM))

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

///@brief
static MENU_ITEM menu[] =
{
    CREATE_TIME_FIELD(TXT_SETUP_SET_TIME,            &LabelFrame1, &ValueFrame1, CurrrentLabelFrame, GetTime, SetTime),
    CREATE_DATE_FIELD(TXT_SETUP_SET_DATE,            &LabelFrame2, &ValueFrame2, CurrrentLabelFrame, GetDate, SetDate),
    CREATE_LIST_FIELD(TXT_SETUP_CHANGE_LANGUAGE,     &LabelFrame3, &ValueFrame3, CurrrentLabelFrame, languages, NUM_LANGUAGES, GetLanguage, SetLanguage),
    CREATE_BOOLEAN_FIELD(TXT_SETUP_CHANGE_BUZZER,    &LabelFrame4, &ValueFrame4, CurrrentLabelFrame, GetBuzzerAvailable, SetBuzzerAvailable),
    CREATE_BOOLEAN_FIELD(TXT_SETUP_CHANGE_BACKLIGHT, &LabelFrame5, &ValueFrame5, CurrrentLabelFrame, GetBacklightAvailable, SetBacklightAvailable), 
    CREATE_BOOLEAN_FIELD(TXT_CHECK_SHOT,             &LabelFrame6, &ValueFrame6, CurrrentLabelFrame, GetCheckShot, SetCheckShot),
};

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*!
********************************************************************************
*       @details
*******************************************************************************/

static MENU_ITEM* GetSetupMenuItem(TAB_ENTRY* tab, U_BYTE index)
{
    if (index < tab->MenuSize(tab))
    {
        return &menu[index];
    }
    return NULL;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static U_BYTE GetSetupMenuSize(TAB_ENTRY* tab)
{
    return sizeof(menu) / sizeof(MENU_ITEM);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void GetTime(RTC_TimeTypeDef* time)
{
    RTC_GetTime(RTC_Format_BIN, time);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void SetTime(RTC_TimeTypeDef* time)
{
    RTC_SetTime(RTC_Format_BIN, time);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void GetDate(RTC_DateTypeDef* date)
{
    RTC_GetDate(RTC_Format_BIN, date);
    
    if(date ->RTC_Date == 0 || date->RTC_Month == 0 || date->RTC_WeekDay == 0 || date->RTC_Year == 0)
    {
      date->RTC_Year = 1;
      date->RTC_Month = 1;
      date->RTC_Date = 1;
      date->RTC_WeekDay = 1;
    }
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void SetDate(RTC_DateTypeDef* date)
{
    if(date->RTC_Date == 0 || date->RTC_Month == 0 || date->RTC_WeekDay == 0 || date->RTC_Year == 0)
    {
      date->RTC_Year = 1;
      date->RTC_Month = 1;
      date->RTC_Date = 1;
      date->RTC_WeekDay = 1;
    }
    RTC_SetDate(RTC_Format_BIN, date);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void SetLanguage(U_BYTE newValue)
{
    SetCurrentLanguage((LANGUAGE_SETTING) newValue);
    PaintNow(&HomeFrame);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static U_BYTE GetLanguage(void)
{
    return CurrentLanguage();
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void SetupTabRefresh(TAB_ENTRY* tab)
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

static void SetupTabShow(TAB_ENTRY* tab)
{
    UI_SetActiveFrame(&LabelFrame1);
    SetActiveLabelFrame(LABEL1);
    PaintNow(&HomeFrame);
}