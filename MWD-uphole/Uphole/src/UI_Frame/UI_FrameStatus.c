/*******************************************************************************
*       @brief      Implementation file for UI_FrameStatus.c.
*       @file       Uphole/src/UI_Frame/UI_FrameStatus.c
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <stm32f4xx.h>
#include <stdio.h>
#include "portable.h"
#include "adc.h"
#include "buzzer.h"
#include "lcd.h"
#include "DownholeBatteryAndLife.h"
#include "version.h"
#include "UI_Defs.h"
#include "UI_DataStructures.h"
#include "UI_Frame.h"
#include "UI_Alphabet.h"
#include "UI_LCDScreenInversion.h"
#include "UI_Primitives.h"
#include "BMP.h"

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 ; Function:
 ;   StatusFrameHandler()
 ;
 ; Description:
 ;   Handles events for the STATUS frame
 ;
 ; Parameters:
 ;   pEvent  =>  Periodic event to be handled.
 ;
 ; Reentrancy:
 ;   No.
 ;
 ;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void StatusFrameHandler(PERIODIC_EVENT *pEvent)
{
    if (pEvent == NULL)
    {
        return;
    }
    switch (pEvent->Action.eActionType)
    {
        case PUSH:
            BuzzerKeypress();
            M_Turn_LCD_On_And_Reset_Timer();
            break;

        case NO_ACTION:
        default:
            break;
    }
}

/*******************************************************************************
*       @details
*******************************************************************************/
void StatusPaint(FRAME* thisFrame)
{
	RTC_TimeTypeDef time;
	RTC_DateTypeDef date;
	char string[100];
    REAL32 DHBatteryV;
    REAL32 DHBatteryV2;
    
        //U_INT16 downholeBatteryVoltage;

	FRAME newFrame = *thisFrame;
	FRAME* pNewFrame = &newFrame;
	RTC_GetTime(RTC_Format_BIN, &time);
	RTC_GetDate(RTC_Format_BIN, &date);
	if(date.RTC_Date == 0 || date.RTC_Month == 0 || date.RTC_WeekDay == 0 || date.RTC_Year == 0)
	{
		date.RTC_Year = 1;
		date.RTC_Month = 1;
		date.RTC_Date = 1;
		date.RTC_WeekDay = 1;
	}
	snprintf(string, 100, "%d/%d/%02d %2d:%02d",
		date.RTC_Month,
		date.RTC_Date,
		date.RTC_Year,
		time.RTC_Hours,
		time.RTC_Minutes);
	UI_ClearLCDArea(&thisFrame->area, LCD_FOREGROUND_PAGE);
	UI_DrawRectangle(thisFrame->area, LCD_FOREGROUND_PAGE);
	UI_DisplayStringInStatusFrame(string, &thisFrame->area, 250);
//	snprintf(string, 100, "MWD %s", GetSWVersion());
	snprintf(string, 100, "Sig %04d", GetDownholeSignalStrength());
	UI_DisplayStringRightJustified(string, &thisFrame->area);
	LCD_Refresh(LCD_FOREGROUND_PAGE);
    DHBatteryV = (float)GetDownholeBatteryVoltage()/1000;
    DHBatteryV2 = (float)GetDownholeBattery2Voltage()/1000;
    if((float)GetDownholeBatteryVoltage()/1000 < 2)
    {
      DHBatteryV = 0.0;
    }
    if((float)GetDownholeBattery2Voltage()/1000 <2)
    {
      DHBatteryV2 = 0.0;
    }
    snprintf(string, 100, "U:%3.1fv D1:%3.1fv D2:%3.1fv", GetUpholeBatteryLife(), DHBatteryV, DHBatteryV2);
    UI_DisplayStringLeftJustified(string, &pNewFrame->area);
}