/*******************************************************************************
*       @brief      Header File for the Periodic Events Subsystem that defines
*                   all the data structures and data types for the Periodic
*                   Events.
*       @file       Uphole/inc/PeriodicEvents.h
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef PERIODIC_EVENTS_H
#define PERIODIC_EVENTS_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"
#include "keypad.h"
#include "textStrings.h"
#include "timer.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

// Actions are in order of priority. First (real action) element
// has the highest priority, last element the lowest
typedef enum __ACTION_TYPE__
{
	UN_USED_ACTION,
	NO_ACTION,
	SCREEN,
	PUSH,
	ALERT,
	TIMER_ELAPSED,
} ACTION_TYPE;

// All Frames are identified here. There is no implied priority to the order
typedef enum _FRAME_ID
{
	NO_FRAME,
	STARTUP,
	HOME,
	TAB1,
	TAB2,
	TAB3,
	TAB4,
	TAB5,
	TAB6,
	TAB7,
	TAB8,
	TAB9,
	TAB10,
	WINDOW,
	LABEL1,
	LABEL2,
	LABEL3,
	LABEL4,
	LABEL5,
	LABEL6,
	LABEL7,
	LABEL8,
	LABEL9,
	LABEL10,
	VALUE1,
	VALUE2,
	VALUE3,
	VALUE4,
	VALUE5,
	VALUE6,
	VALUE7,
	VALUE8,
	VALUE9,
	VALUE10,
	STATUS,
	ALERT_FRAME,
	LAST_FRAME  //<---- Must be the last element on the enum list
} FRAME_ID;

typedef enum __TASK__
{
	NO_TASK,
	REPAINT,
	ANIMATION,
	BLINK_CURSOR,
	BLINK_ICON,
	ANIMATE_ICON,
} SCREEN_TASK;

// An Action is a single thing that has happened.
typedef struct __ACTION__
{
	FRAME_ID		eFrameID;
	ACTION_TYPE		eActionType;
	BUTTON_VALUE	nValue;
	SCREEN_TASK		ScreenTask;
	TXT_VALUES      eMessage;
} ACTION;

// A Periodic Event is an action combined with a time stamp
typedef struct __PERIODIC_EVENT__
{
	// What the event was
	ACTION Action;
	TIME_LR tTriggerTime;
} PERIODIC_EVENT;

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

	// UI_InitPeriodicEvents is called during the overall initialization of the UI.
	void InitPeriodicEvents(void);
	// AddPeriodicEvent is called out of the 10ms Cycle Handler when a knob or button Event is detected.
	BOOL AddPeriodicEvent(const PERIODIC_EVENT *pEvent);
	//  GetNextPeriodicEvent is called from the Main loop to process all Periodic Events
	BOOL GetNextPeriodicEvent(PERIODIC_EVENT *pEvent);
	//  ProcessPeriondEvent calls the Periodic Event Handler that is associated with the current frame.
	void ProcessPeriodicEvent(PERIODIC_EVENT *pEvent);
	//  Initialize an event
	void UI_ClearEvent(PERIODIC_EVENT *pEvent);
	//  Sets whether to allow knob and button actions into the event queue
	void SetAllowKeypadActions(BOOL bAllow);
	//  Clears all events from the queue
	void UI_RemoveAllEventsQueue(void);
	//  Clear pending event queue
	void UI_ClearPendingEventsQueue(void);
	//  Fully clears Both event queues
	void UI_FlushBothEventQueues(void);
	//   Accessor function to add a button push event to the Pending Array
	void AddButtonEvent(BUTTON_VALUE nButtonValue);
	void AddButtonEventWithFrame(BUTTON_VALUE nButtonValue, FRAME_ID eFrameID);
	//  Accessor function to add a screen event to the Pending Array
	void AddScreenEvent(SCREEN_TASK eTask, FRAME_ID eFrameID, TIME_LR tTrigger);
	BOOL UI_KeyActivity(void);
	void Reset_Event_Flag(void);
	void SetUIKeyPressEvent(void);

#ifdef __cplusplus
}
#endif

#endif // PERIODIC_EVENTS_H
