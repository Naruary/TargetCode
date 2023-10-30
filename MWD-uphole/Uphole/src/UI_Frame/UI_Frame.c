 /*******************************************************************************
*       @brief      Source file for UI_FRAME.c
*       @file       Uphole/src/UI_Frame/UI_Frame.c
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"
#include "PeriodicEvents.h"
#include "UI_Frame.h"
#include "UI_DataStructures.h"
#include "UI_Defs.h"
#include "UI_api.h"

//============================================================================//
//      MACROS                                                                //
//============================================================================//

#define SCREEN_AREA                 {{0,0}, {238, 319}}
#define EMPTY_FRAME()               {NO_FRAME, SCREEN_AREA}
#define STARTUP_FRAME()             {STARTUP, SCREEN_AREA, StartupFrameHandler}
#define HOME_FRAME()                {HOME, SCREEN_AREA, HomeFrameHandler, HomePaint}
#define WINDOW_FRAME()              {WINDOW,  {{25, 3}, {217, 316}}, WindowFrameHandler, WindowPaint}
#define STATUS_FRAME()              {STATUS, {{219, 3}, {235, 316}}, StatusFrameHandler, StatusPaint}
#define ALERT_FRAME()               {ALERT_FRAME, {{40, 53},  {199, 256}}, AlertFrameHandler, AlertPaint}

#define MIN_TAB_ROW                 3
#define MAX_TAB_ROW                 24
#define TAB_WIDTH                   29.6 //30
#define TAB_SPACING                 2
#define MIN_TAB_COL(tab)            (((tab - 1) * (TAB_WIDTH + TAB_SPACING)) + 3)
#define MAX_TAB_COL(tab)            (MIN_TAB_COL(tab) + TAB_WIDTH)
#define TAB_FRAME(tab)              {TAB##tab, {{(INT16)MIN_TAB_ROW, (INT16)MIN_TAB_COL(tab)}, {(INT16)MAX_TAB_ROW, (INT16)MAX_TAB_COL(tab)}}, TabFrameHandler, TabPaint, TabInitialize}

#define MIN_MENU_ROW(item)          (30 + ((item - 1) * 15))
#define MAX_MENU_ROW(item)          (MIN_MENU_ROW(item) + 14)

#define LABEL_TOP_LEFT(label)       {MIN_MENU_ROW(label), 5}
#define LABEL_BOTTOM_RIGHT(label)   {MAX_MENU_ROW(label), 169}
#define LABEL_FRAME(label)          {LABEL##label, {LABEL_TOP_LEFT(label), LABEL_BOTTOM_RIGHT(label)}, LabelFrameHandler, LabelPaint, LabelInitialize}

#define VALUE_TOP_LEFT(value)       {MIN_MENU_ROW(value), 172}
#define VALUE_BOTTOM_RIGHT(value)   {MAX_MENU_ROW(value), 282}
#define VALUE_FRAME(value)          {VALUE##value, {VALUE_TOP_LEFT(value), VALUE_BOTTOM_RIGHT(value)}, ValueFrameHandler, ValuePaint, ValueInitialize}

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

static FRAME* m_ActiveFrameInstance;
const FRAME EmptyFrame = EMPTY_FRAME();
const FRAME StartupFrame = STARTUP_FRAME();
const FRAME HomeFrame = HOME_FRAME();
const FRAME TabFrame1 = TAB_FRAME(1);
const FRAME TabFrame2 = TAB_FRAME(2);
const FRAME TabFrame3 = TAB_FRAME(3);
const FRAME TabFrame4 = TAB_FRAME(4);
const FRAME TabFrame5 = TAB_FRAME(5);
const FRAME TabFrame6 = TAB_FRAME(6);
const FRAME TabFrame7 = TAB_FRAME(7);
const FRAME TabFrame8 = TAB_FRAME(8);
const FRAME TabFrame9 = TAB_FRAME(9);
const FRAME TabFrame10 = TAB_FRAME(10);
const FRAME WindowFrame = WINDOW_FRAME();
const FRAME LabelFrame1 = LABEL_FRAME(1);
const FRAME LabelFrame2 = LABEL_FRAME(2);
const FRAME LabelFrame3 = LABEL_FRAME(3);
const FRAME LabelFrame4 = LABEL_FRAME(4);
const FRAME LabelFrame5 = LABEL_FRAME(5);
const FRAME LabelFrame6 = LABEL_FRAME(6);
const FRAME LabelFrame7 = LABEL_FRAME(7);
const FRAME LabelFrame8 = LABEL_FRAME(8);
const FRAME LabelFrame9 = LABEL_FRAME(9);
const FRAME LabelFrame10 = LABEL_FRAME(10);
const FRAME ValueFrame1 = VALUE_FRAME(1);
const FRAME ValueFrame2 = VALUE_FRAME(2);
const FRAME ValueFrame3 = VALUE_FRAME(3);
const FRAME ValueFrame4 = VALUE_FRAME(4);
const FRAME ValueFrame5 = VALUE_FRAME(5);
const FRAME ValueFrame6 = VALUE_FRAME(6);
const FRAME ValueFrame7 = VALUE_FRAME(7);
const FRAME ValueFrame8 = VALUE_FRAME(8);
const FRAME ValueFrame9 = VALUE_FRAME(9);
const FRAME ValueFrame10 = VALUE_FRAME(10);
const FRAME StatusFrame = STATUS_FRAME();
const FRAME AlertFrame = ALERT_FRAME();

const FRAME* m_FramesDefault[] =
{
    &EmptyFrame,
    &StartupFrame,
    &HomeFrame,
    &TabFrame1,
    &TabFrame2,
    &TabFrame3,
    &TabFrame4,
    &TabFrame5,
    &TabFrame6,
    &TabFrame7,
    &TabFrame8,
    &TabFrame9,
    &TabFrame10,
    &WindowFrame,
    &LabelFrame1,
    &LabelFrame2,
    &LabelFrame3,
    &LabelFrame4,
    &LabelFrame5,
    &LabelFrame6,
    &LabelFrame7,
    &LabelFrame8,
    &LabelFrame9,
    &LabelFrame10,
    &ValueFrame1,
    &ValueFrame2,
    &ValueFrame3,
    &ValueFrame4,
    &ValueFrame5,
    &ValueFrame6,
    &ValueFrame7,
    &ValueFrame8,
    &ValueFrame9,
    &ValueFrame10,
    &StatusFrame,
    &AlertFrame,
};

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*******************************************************************************
*       @details
*******************************************************************************/
void UI_SetActiveFrame(const FRAME* frame)
{
	m_ActiveFrameInstance = (FRAME*)frame;
}

/*******************************************************************************
*       @details
*******************************************************************************/
const FRAME* GetFrame(FRAME_ID eID)
{
	return (FRAME*)m_FramesDefault[eID];
}

/*******************************************************************************
*       @details
*******************************************************************************/
FRAME* UI_GetActiveFrame(void)
{
	return m_ActiveFrameInstance;
}

/*******************************************************************************
*       @details
*******************************************************************************/
void RepaintNow(const FRAME* frame)
{
	AddScreenEvent(REPAINT, frame->eID, TRIGGER_TIME_NOW);
}

/*******************************************************************************
*       @details
*******************************************************************************/
void PaintNow(const FRAME* frame)
{
	frame->Paint((FRAME*)frame);
}
