/*!
********************************************************************************
*       @brief      Header File for the Periodic Events Subsystem that defines 
*                   all the data structures and data types for the Periodic 
*                   Events.  Documented by Josh Masters Dec. 2014
*       @file       Uphole/inc/PeriodicEvents.h
*       @author     Bill Kamienik
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
enum ACTION_TYPE
{
    UN_USED_ACTION,
    NO_ACTION,
    SCREEN,
    PUSH,
    ALERT,
    TIMER_ELAPSED,
};

typedef struct __BUTTON_PUSH__
{
    BUTTON_TYPE eButtonType;
    BUTTON_DEPRESS_TYPE eDepressType;
    BUTTON_VALUE nValue;
} BUTTON_PUSH;

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

typedef enum __ALERT_TYPE__
{
    NO_ALERT,
    REMINDER,
    CONFIRMATON,
    ALARM
} ALERT_TYPE;

typedef enum __ALERT_PRIORITY__
{
    NO_ALARM_PRIORITY,
    LO_ALARM_PRIORITY,
    MED_ALARM_PRIORITY,
    HIGH_ALARM_PRIORITY
} ALARM_PRIORITY;

typedef struct __ALERT_EVENT__
{
    ALERT_TYPE      eAlertType;
    ALARM_PRIORITY  eAlertPriority;
    TXT_VALUES      eHeader;
    TXT_VALUES      eMessage;
} ALERT_EVENT;

//
// An Action is a single thing that has happened.
//
typedef struct __ACTION__
{
         FRAME_ID           eFrameID;
    enum ACTION_TYPE        eActionType;
         BUTTON_PUSH        ButtonPush;
         SCREEN_TASK        ScreenTask;
         ALERT_EVENT        AlertEvent;
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

    ///@brief  UI_InitPeriodicEvents is called during the overall initialization of the UI.
    ///@param  
    ///@return
    void InitPeriodicEvents(void);

    ///@brief  AddPeriodicEvent is called out of the 10ms Cycle Handler when a knob or button Event is detected. 
    ///@param  
    ///@return
    BOOL AddPeriodicEvent(const PERIODIC_EVENT *pEvent);

    ///@brief  GetNextPeriodicEvent is called from the Main loop to process all Periodic Events
    ///@param  
    ///@return
    BOOL GetNextPeriodicEvent(PERIODIC_EVENT *pEvent);

    ///@brief  ProcessPeriondEvent calls the Periodic Event Handler that is associated with the current frame.
    ///@param  
    ///@return
    void ProcessPeriodicEvent(PERIODIC_EVENT *pEvent);

    ///@brief  Initialize an event
    ///@param  
    ///@return
    void UI_ClearEvent(PERIODIC_EVENT *pEvent);

    ///@brief  Sets whether to allow knob and button actions into the event queue
    ///@param  
    ///@return
    void SetAllowKeypadActions(BOOL bAllow);

    ///@brief  Clears all events from the queue
    ///@param  
    ///@return
    void UI_RemoveAllEventsQueue(void);

    ///@brief  Clear pending event queue
    ///@param  
    ///@return
    void UI_ClearPendingEventsQueue(void);

    ///@brief  Fully clears Both event queues
    ///@param  
    ///@return
    void UI_FlushBothEventQueues(void);
    
    ///@brief   Accessor function to add a button push event to the Pending Array
    ///@param  
    ///@return
    void AddButtonEvent(BUTTON_TYPE eButton, BUTTON_DEPRESS_TYPE eDuration, BUTTON_VALUE nButtonValue);
    
    ///@brief  
    ///@param  
    ///@return
    void AddButtonEventWithFrame(BUTTON_TYPE eButton, BUTTON_DEPRESS_TYPE eDuration, BUTTON_VALUE nButtonValue, FRAME_ID eFrameID);

    ///@brief  Accessor function to add a screen event to the Pending Array
    ///@param  
    ///@return
    void AddScreenEvent(SCREEN_TASK eTask, FRAME_ID eFrameID, TIME_LR tTrigger);
    
    BOOL UI_KeyActivity(void);
    
    void Reset_Event_Flag(void);
    
    void Set_Event_Flag(void);

#ifdef __cplusplus
}
#endif
#endif