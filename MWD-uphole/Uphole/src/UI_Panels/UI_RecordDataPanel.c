/*******************************************************************************
 *       @brief      This file contains the implementation for the Record Data
 *                   Panel on the Data tab. It display the current hole data on the screen
 *       @file       Uphole/src/UI_Panels/UI_RecordDataPanel.c
 *       @date       July 2014
 *       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
 *                   reserved.  Reproduction in whole or in part is prohibited
 *                   without the prior written consent of the copyright holder.
 *******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "LoggingManager.h"
#include "RecordManager.h"
#include "FlashMemory.h"
#include "UI_Alphabet.h"
#include "UI_ScreenUtilities.h"
#include "UI_Frame.h"
#include "UI_LCDScreenInversion.h"
#include "UI_api.h"
#include "UI_RecordDataPanel.h"
#include "UI_SurveyEditPanel.h"
#include "UI_DataTab.h"
#include "UI_JobTab.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

#define NUM_ROWS 16
#define ROW_HEIGHT 10

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

//   Initializes Columns for Record Data Panel
void RecordData_InitializeColumn(RECT *frame, U_INT16 offset, U_INT16 width, RECT *area);
//   Prepares next column
static void RecordData_IncrementColumn(RECT *rect);
//   Displays column and value on screen
void RecordData_DisplayColumn(char *strValue, RECT *rect);
//   Displays column header
void RecordData_DisplayColumnHeader(TXT_VALUES text, RECT *rect);
//   Displays tab data onto screen
static void RecordData_Paint(TAB_ENTRY *tab);
//   Reveals tab on screen
static void RecordData_Show(TAB_ENTRY *tab);
//   Determines if a key was pressed
static void RecordData_KeyPressed(TAB_ENTRY *tab, BUTTON_VALUE key);
//   Record Data Timer
static void RecordData_TimerElapsed(TAB_ENTRY *tab);
//   Gets selected survey variable
static U_INT32 RecordData_RetrieveSelectSurveyIndex(void);
static REAL32 RealValue(INT16 value);
static REAL32 RealValue32(INT32 value);
//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

static U_INT32 RecordOffset = 0;
// static U_INT32 SavedRecordOffset = 0;
static U_INT32 surveySelect = 0;
static U_INT32 HoleIndex = 1;
static U_INT32 storedSurveyIndex = 0;
static STRUCT_RECORD_DATA static_record;
/// Used to keep track as record offset for screen display
// static U_INT16 record_count;
PANEL RecordDataPanel = {0, 0, RecordData_Paint, RecordData_Show, RecordData_KeyPressed, RecordData_TimerElapsed};

INT16 m = 0, n = 0;

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*******************************************************************************
 *       @details
 *******************************************************************************/
void RecordDataPanelInit(void)
{
    if (InitNewHole_KeyPress())
    {
        memset((void *)&static_record, 0, sizeof(static_record)); // get zeros
        RecordOffset = GetRecordCount();
        surveySelect = GetRecordCount();
        HoleIndex = 1;
        // record_count=0;
    }
    else
    {
        RECORD_GetRecord(&static_record, (GetRecordCount() - 1));
        RecordOffset = GetRecordCount() - static_record.nRecordNumber;
        surveySelect = GetRecordCount() - static_record.nRecordNumber;
        HoleIndex = 1;
        // record_count=0;
    }
}

/*******************************************************************************
 *       @details
 *******************************************************************************/
void RecordData_InitializeColumn(RECT *frame, U_INT16 offset, U_INT16 width, RECT *area)
{
    area->ptTopLeft.nCol = frame->ptTopLeft.nCol + 1 + offset;
    area->ptTopLeft.nRow = frame->ptTopLeft.nRow + 2;
    area->ptBottomRight.nCol = area->ptTopLeft.nCol + width;
    area->ptBottomRight.nRow = area->ptTopLeft.nRow + ROW_HEIGHT;
}

/*******************************************************************************
 *       @details
 *******************************************************************************/
static void RecordData_IncrementColumn(RECT *rect)
{
    rect->ptTopLeft.nRow += ROW_HEIGHT;
    rect->ptBottomRight.nRow = rect->ptTopLeft.nRow + ROW_HEIGHT;
}

/*******************************************************************************
 *       @details
 *******************************************************************************/
void RecordData_DisplayColumn(char *strValue, RECT *rect)
{
    UI_DisplayStringCentered(strValue, rect);
    if (surveySelect == HoleIndex && rect->ptTopLeft.nCol < 25)
    {
        UI_InvertLCDArea(rect, LCD_FOREGROUND_PAGE);
        RecordData_StoreSelectSurveyIndex(surveySelect);
    }
}

/*******************************************************************************
 *       @details
 *******************************************************************************/
void RecordData_DisplayColumnHeader(TXT_VALUES text, RECT *rect)
{
    UI_DisplayStringCentered(GetTxtString(text), rect);
    UI_InvertLCDArea(rect, LCD_FOREGROUND_PAGE);
}

/*******************************************************************************
 *       @details
 *******************************************************************************/
static void RecordData_Paint(TAB_ENTRY *tab)
{
    static char strValue[100];
    STRUCT_RECORD_DATA record;
    U_INT32 lastRecord;
    U_INT32 loopy;
    REAL32 TotalLength = 0.0;
    U_INT16 RecordNumber = 0;
    RECT markerBox;
    RECT recordColumn, lengthColumn, azimuthColumn, pitchColumn, rollColumn;
    RECT gammaColumn, upDownColumn, leftRightColumn, distanceColumn;
    RECT *area = (RECT *)&WindowFrame.area;

    RecordData_InitializeColumn(area, 0, 32, &recordColumn);
    // a square just to the right of the index
    RecordData_InitializeColumn(area, 17, 8, &markerBox);
    RecordData_InitializeColumn(area, 33, 34, &lengthColumn);
    RecordData_InitializeColumn(area, 66, 35, &azimuthColumn); // 67,40
    RecordData_InitializeColumn(area, 102, 35, &pitchColumn);  // 108,40
    RecordData_InitializeColumn(area, 138, 22, &rollColumn);   // 149,40
    RecordData_InitializeColumn(area, 161, 22, &gammaColumn);
    RecordData_InitializeColumn(area, 184, 42, &distanceColumn);
    RecordData_InitializeColumn(area, 227, 42, &leftRightColumn);
    RecordData_InitializeColumn(area, 270, 42, &upDownColumn);
    RecordData_DisplayColumnHeader(TXT_HASH, &recordColumn);
    RecordData_IncrementColumn(&recordColumn);
    RecordData_DisplayColumnHeader(TXT_LENGTH_ABREV, &lengthColumn);
    RecordData_IncrementColumn(&lengthColumn);
    RecordData_DisplayColumnHeader(TXT_AZIMUTH_ABREV, &azimuthColumn);
    RecordData_IncrementColumn(&azimuthColumn);
    RecordData_DisplayColumnHeader(TXT_PITCH_ABREV, &pitchColumn);
    RecordData_IncrementColumn(&pitchColumn);
    RecordData_DisplayColumnHeader(TXT_TOOLFACE_ABREV, &rollColumn);
    RecordData_IncrementColumn(&rollColumn);
    RecordData_DisplayColumnHeader(TXT_G, &gammaColumn);
    RecordData_IncrementColumn(&gammaColumn);
    RecordData_DisplayColumnHeader(TXT_DOWNTRACK_ABREV, &distanceColumn);
    RecordData_IncrementColumn(&distanceColumn);
    RecordData_DisplayColumnHeader(TXT_LEFTRIGHT_ABREV, &leftRightColumn);
    RecordData_IncrementColumn(&leftRightColumn);
    RecordData_DisplayColumnHeader(TXT_UPDOWN_ABREV, &upDownColumn);
    RecordData_IncrementColumn(&upDownColumn);
    lastRecord = RecordOffset + NUM_ROWS;
    if (lastRecord > GetRecordCount())
    {
        lastRecord = GetRecordCount();
    }
    if (RECORD_GetRefreshSurveys())
    {
        LoggingManager_StartUpload();
        RECORD_SetRefreshSurveys(false);
    }
    for (loopy = RecordOffset; loopy < lastRecord; loopy++)
    {
        // condition to take care that records from previous holes dont show up
        // after all records from the current hole are deleted
        if (loopy <= PreviousHoleEndingRecordNumber())
        {
            break;
        }
        if (RECORD_GetRecord(&record, loopy))
        {
            HoleIndex = loopy;
            m = record.StatusCode % 10;

            // if this is a branch hole, mark it
            if (record.NumOfBranch != 0)
            {
                snprintf(strValue, 100, "%dB", record.nRecordNumber - record.GammaShotNumCorrected); // record.nRecordNumber);
            }
            else if (record.InvalidDataFlag == true)
            {
                snprintf(strValue, 100, "%dS", record.nRecordNumber - record.GammaShotNumCorrected); // S for sidetrack
            }
            else if (record.GammaShotLock == 1)
            {
                snprintf(strValue, 100, "-G%d", record.nRecordNumber - record.GammaShotNumCorrected);
            }
            else
            {
                snprintf(strValue, 100, "%d", record.nRecordNumber - record.GammaShotNumCorrected);
            }
            RecordData_DisplayColumn(strValue, &recordColumn);
            RecordData_IncrementColumn(&recordColumn);
            snprintf(strValue, 100, "%d", record.nTotalLength); /// 10);
            RecordData_DisplayColumn(strValue, &lengthColumn);
            RecordData_IncrementColumn(&lengthColumn);
            snprintf(strValue, 100, "%4.1f", RealValue(record.nAzimuth));
            RecordData_DisplayColumn(strValue, &azimuthColumn);
            RecordData_IncrementColumn(&azimuthColumn);
            snprintf(strValue, 100, "%4.1f", RealValue(record.nPitch));
            RecordData_DisplayColumn(strValue, &pitchColumn);
            RecordData_IncrementColumn(&pitchColumn);
            snprintf(strValue, 100, "%.0f", RealValue(record.nRoll));
            RecordData_DisplayColumn(strValue, &rollColumn);
            RecordData_IncrementColumn(&rollColumn);

            snprintf(strValue, 50, "%d", record.nGamma);
            RecordData_DisplayColumn(strValue, &gammaColumn);
            RecordData_IncrementColumn(&gammaColumn);

            snprintf(strValue, 100, "%4.1f", RealValue32(record.Z));
            RecordData_DisplayColumn(strValue, &distanceColumn);
            RecordData_IncrementColumn(&distanceColumn);
            snprintf(strValue, 100, "%4.1f", RealValue(record.X));
            RecordData_DisplayColumn(strValue, &leftRightColumn);
            RecordData_IncrementColumn(&leftRightColumn);
            snprintf(strValue, 100, "%4.1f", RealValue(record.Y) / (double)10.0);
            RecordData_DisplayColumn(strValue, &upDownColumn);
            RecordData_IncrementColumn(&upDownColumn);
        }
        TotalLength = GetLastLength();
        RecordNumber = static_record.nRecordNumber;
    }
    snprintf(strValue, 100, "%s: Tot Surv=%d / Tot Len=%.0f",
             GetBoreholeName(),
             RecordNumber,
             TotalLength);
    RecordData_Show(tab);
    TabWindowPaint(tab);
    ShowStatusMessage(strValue);
}

/*******************************************************************************
 *       @details
 *******************************************************************************/
static REAL32 RealValue(INT16 value)
{
    return (REAL32)(value / 10.);
}

static REAL32 RealValue32(INT32 value)
{
    return (REAL32)(value / 10.);
}

/*******************************************************************************
 *       @details
 *******************************************************************************/
static void RecordData_Show(TAB_ENTRY *tab)
{
    UI_SetActiveFrame(tab->frame);
    SetActiveLabelFrame(NO_FRAME);
    SetActiveValueFrame(NO_FRAME);
}

/*******************************************************************************
 *       @details
 *******************************************************************************/
void RecordData_KeyPressed(TAB_ENTRY *tab, BUTTON_VALUE key)
{
    STRUCT_RECORD_DATA record;

    switch (key)
    {
    case BUTTON_UP:
        RecordOffset = surveySelect;
        if (RecordOffset > (GetRecordCount() - static_record.nRecordNumber))
        {
            RECORD_GetRecord(&record, RecordOffset);
            if (record.PreviousBranchRecordNum)
            {
                RecordOffset = record.PreviousBranchRecordNum;
            }
            else
            {
                RecordOffset--;
            }
        }
        if (surveySelect > (GetRecordCount() - static_record.nRecordNumber))
        {
            RECORD_GetRecord(&record, surveySelect);
            if (record.PreviousBranchRecordNum)
            {
                surveySelect = record.PreviousBranchRecordNum;
            }
            else
            {
                surveySelect--;
            }
        }
        RepaintNow(&WindowFrame);
        n = record.StatusCode % 10;
        if (GetRecordCount() - static_record.nRecordNumber == surveySelect)
        {
            n = 0;
        }
        break;
    case BUTTON_DOWN:
        RecordOffset = surveySelect;
        if (surveySelect < (GetRecordCount() - 1))
        {
            if (surveySelect < GetLastRecordNumber() + PreviousHoleEndingRecordNumber())
            {
                RECORD_GetRecord(&record, surveySelect);
                surveySelect++;
                if (record.NumOfBranch)
                {
                    surveySelect = record.NextBranchRecordNum;
                }
                RECORD_GetRecord(&record, RecordOffset);
                if (record.NumOfBranch)
                {
                    RecordOffset = record.NextBranchRecordNum;
                }
                else
                {
                    RecordOffset++;
                }
            }
        }
        RepaintNow(&WindowFrame);
        n = record.StatusCode % 10;
        break;
    case BUTTON_ONE:
        RecordOffset = surveySelect;
        for (int page_record = 0; page_record < GetRecordCount() - 1; page_record++)
        {
            if (surveySelect < (GetRecordCount() - 1))
            {
                if (surveySelect < GetLastRecordNumber() + PreviousHoleEndingRecordNumber())
                {
                    // RecordOffset = surveySelect;
                    RECORD_GetRecord(&record, surveySelect);
                    surveySelect++;
                    if (record.NumOfBranch)
                    {
                        surveySelect = record.NextBranchRecordNum;
                    }
                    RECORD_GetRecord(&record, RecordOffset);
                    if (record.NumOfBranch)
                    {
                        RecordOffset = record.NextBranchRecordNum;
                    }
                    else
                    {
                        RecordOffset++;
                    }
                }
            }
            RepaintNow(&WindowFrame);
        }
        n = llabs(record.StatusCode % 10);
        break;
    case BUTTON_TWO:
        RecordOffset = surveySelect;
        for (int page_record = 0; page_record < 10; page_record++)
        {
            if (RecordOffset > (GetRecordCount() - static_record.nRecordNumber))
            {
                RECORD_GetRecord(&record, RecordOffset);
                if (record.PreviousBranchRecordNum)
                {
                    RecordOffset = record.PreviousBranchRecordNum;
                }
                else
                {
                    RecordOffset--;
                }
            }
            if (surveySelect > (GetRecordCount() - static_record.nRecordNumber))
            {
                RECORD_GetRecord(&record, surveySelect);
                if (record.PreviousBranchRecordNum)
                {
                    surveySelect = record.PreviousBranchRecordNum;
                }
                else
                {
                    surveySelect--;
                }
            }
            RepaintNow(&WindowFrame);
        }
        break;
    case BUTTON_THREE:
        for (int page_record = 0; page_record < 10; page_record++)
        {
            if (RecordOffset < GetLastRecordNumber() + PreviousHoleEndingRecordNumber())
            {
                RECORD_GetRecord(&record, RecordOffset);
                RecordOffset++;
            }
            RepaintNow(&WindowFrame);
        }
        break;
    case BUTTON_FOUR:
        for (int page_record = 0; page_record < 10; page_record++)
        {
            if (RecordOffset > (GetRecordCount() - static_record.nRecordNumber))
            {
                RECORD_GetRecord(&record, RecordOffset);
                RecordOffset--;
            }
            RepaintNow(&WindowFrame);
        }
        break;
    case BUTTON_DASH:
        LoggingManager_StartUpload();
        RepaintNow(&WindowFrame);
        break;
    case BUTTON_SELECT:
        RECORD_StoreSelectSurvey(RecordData_RetrieveSelectSurveyIndex());
        setSurveyEditPanelActive(true);
        RepaintNow(&WindowFrame);
        break;
    case BUTTON_SHIFT:
        RECORD_StoreSelectSurvey(RecordData_RetrieveSelectSurveyIndex());
        setSurveyEditPanelActive(true);
        RepaintNow(&WindowFrame);
        break;
    default:
    {
        // To fix the buzzer being continuously ON, since All Keypad key
        // operations are not defined,
        SetAllowKeypadActions(false);
        PERIODIC_EVENT event;
        event.Action.eFrameID = TAB2;
        event.Action.eActionType = PUSH;
        //      event.Action.eButtonType = KEYPAD;
        //      event.Action.eDepressType = SHORT_DEPRESS;
        event.Action.nValue = key;
        event.tTriggerTime = TRIGGER_TIME_NOW;
        AddPeriodicEvent(&event);
        // To fix the buzzer being continuously ON
        SetAllowKeypadActions(true);
    }
    break;
    }
}

/*******************************************************************************
 *       @details
 *******************************************************************************/
static void RecordData_TimerElapsed(TAB_ENTRY *tab)
{
    //	RepaintNow(&WindowFrame);
}

/*******************************************************************************
 *       @details
 *******************************************************************************/
void RecordData_StoreSelectSurveyIndex(U_INT32 index)
{
    storedSurveyIndex = index;
}

/*******************************************************************************
 *       @details
 *******************************************************************************/
static U_INT32 RecordData_RetrieveSelectSurveyIndex(void)
{
    return storedSurveyIndex;
}

/*******************************************************************************
 *       @details
 *******************************************************************************/
U_INT16 GetStartRecordNumber(void)
{
    return static_record.nRecordNumber;
}
