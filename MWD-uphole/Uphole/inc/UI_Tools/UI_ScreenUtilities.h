/*******************************************************************************
*       @brief      Implementation File for the UI Screens.
*       @file       Uphole/inc/UI_Tools/UI_ScreenUtilities.h
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef UI_SCREEN_UTILITIES_H
#define UI_SCREEN_UTILITIES_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"
#include "PeriodicEvents.h"
#include "TextStrings.h"
#include "rtc.h"
#include "UI_FixedPointValue.h"
#include "UI_Frame.h"

//============================================================================//
//      MACROS                                                                //
//============================================================================//

#define CREATE_MENU_ITEM(Label, LabelFrame, Selected) {Label, LabelFrame, Selected}
#define CREATE_DISPLAY_FIELD(Label, LabelFrame, ValueFrame, Display) {Label, LabelFrame, NoSelection, ValueFrame, NoSelection, Display}

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

typedef struct _LIST_ITEM
{
    U_BYTE value;
    TXT_VALUES textStringIndex;
} LIST_ITEM;

typedef struct _LIST_DATA
{
    LIST_ITEM* list;
    U_BYTE count;
    U_BYTE (*GetValue)(void);
    void (*SetValue)(U_BYTE newValue);
    U_BYTE value;
} LIST_DATA;

#define MAX_STRING_CHARS 16 //shortened length from 32 to 16
typedef struct _STRING_DATA
{
    char* (*GetValue)(void);
    void (*SetValue)(char* newValue);
    char value[MAX_STRING_CHARS];
    U_BYTE position;
} STRING_DATA;

typedef enum _TIME_FIELD
{
    HOURS, MINUTES, SECONDS
} TIME_FIELD;

typedef struct _TIME_DATA
{
    void (*GetTime)(RTC_TimeTypeDef *time);
    void (*SetTime)(RTC_TimeTypeDef *time);
    RTC_TimeTypeDef time;
    U_BYTE position;
    TIME_FIELD field;
} TIME_DATA;

typedef enum _DATE_FIELD
{
    MONTH, DAY, YEAR
} DATE_FIELD;

typedef struct _DATE_DATA
{
    void (*GetDate)(RTC_DateTypeDef *date);
    void (*SetDate)(RTC_DateTypeDef *date);
    RTC_DateTypeDef date;
    U_BYTE position;
    DATE_FIELD field;
} DATE_DATA;

typedef struct _BOOLEAN_DATA
{
    BOOL value;
    BOOL (*GetValue)(void);
    void (*SetValue)(BOOL newValue);
} BOOLEAN_DATA;

// Definition for a menu entry
typedef struct _MENU_ITEM
{
    TXT_VALUES label;
    const FRAME* labelFrame;
    void (*Selected)(struct _MENU_ITEM* item);
    const FRAME* valueFrame;
    void (*NextFrame)(struct _MENU_ITEM* item);
    void (*DisplayValue)(struct _MENU_ITEM* item);
    void (*BeginEdit)(struct _MENU_ITEM* item);
    void (*FinishEdit)(struct _MENU_ITEM* item);
    void (*KeyPressed)(struct _MENU_ITEM* item, BUTTON_VALUE keyPressed);
    void (*HighlightValue)(struct _MENU_ITEM* item);
    union
    {
        TIME_DATA time;
        DATE_DATA date;
        FIXED_POINT_DATA fixed;
        BOOLEAN_DATA boolean;
        LIST_DATA list;
        STRING_DATA string;
    };
    BOOL editing;
    BOOL highlight;
} MENU_ITEM;

// Definition for a Tab
typedef struct _TAB_ENTRY
{
    const FRAME* frame;
    TXT_VALUES label;
    BOOL (*IsVisible)(struct _TAB_ENTRY* tab);
    MENU_ITEM* (*MenuItem)(struct _TAB_ENTRY* tab, U_BYTE index);
    U_BYTE (*MenuSize)(struct _TAB_ENTRY* tab);
    void (*Paint)(struct _TAB_ENTRY* tab);
    void (*Show)(struct _TAB_ENTRY* tab);
    void (*OneSecondTimerElapsed)(struct _TAB_ENTRY* tab);
    void (*KeyPressed)(struct _TAB_ENTRY* tab, BUTTON_VALUE key);
} TAB_ENTRY;

typedef struct _PANEL
{
	MENU_ITEM* (*MenuItem)(U_BYTE index);
	U_BYTE MenuCount;
	void (*Paint)(TAB_ENTRY* tab);
	void (*Show)(TAB_ENTRY* tab);
	void (*KeyPressed)(TAB_ENTRY* tab, BUTTON_VALUE key);
	void (*TimerElapsed)(TAB_ENTRY* tab);
} PANEL;

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

	void UI_Initialize(void);
	void UI_SetStartupComplete(BOOL bComplete);
	BOOL UI_StartupComplete(void);
	BOOL ShowTab(TAB_ENTRY* tab);
	TAB_ENTRY* GetActiveTab(void);
	void EditValue(MENU_ITEM* item);

#ifdef __cplusplus
}
#endif
#endif
