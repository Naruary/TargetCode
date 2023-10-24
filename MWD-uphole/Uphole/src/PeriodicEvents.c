/*******************************************************************************
*       @brief      This module provides the routines that handle management of
*                   the pending and periodic event queues for the UI.
*       @file       Uphole/src/PeriodicEvents.c
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <stdbool.h>
#include <stdlib.h>
#include "buzzer.h"
#include "PeriodicEvents.h"
#include "LCD.h"
#include "UI_Frame.h"
#include "UI_ScreenUtilities.h"
#include "SysTick.h"

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

// Structures and typedefs used only in this module
typedef struct __PENDING_EVENT__
{
    PERIODIC_EVENT Event;
    BOOL bEntryAdded;
} PENDING_EVENT;

// List element
#define PERIODIC_INTERRUPT_LIST_LENGTH  40

// stores whether to allow button pushes
static BOOL m_bAllowPushActions;

// Counter to keep track of events not adding to pending event Q because it overflowed
static U_INT16 m_nPendingOverFlowCnt;

// Pending Event Q
static PENDING_EVENT m_PendingEvents[PERIODIC_INTERRUPT_LIST_LENGTH];

// Periodic Event Q
static PERIODIC_EVENT m_PeriodicEvents[PERIODIC_INTERRUPT_LIST_LENGTH];

static BOOL EventFlag = false;

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

// Clears a pending event
static void clearPendingEvent(PENDING_EVENT *pPendingEvent);
// Finds higher priority
static BOOL isHigherPriority(ACTION Source, ACTION Dest);
static void insertEventInPeriodicArray(PERIODIC_EVENT *pEvent, INT32 nAtIndex);
static void compressPeriodicArray(void);
static void popNextPeriodicEvent(PERIODIC_EVENT *pEvent);

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

// Const "Null" element returns for certain functions
static const PERIODIC_EVENT m_InitPeriodicEvent =
{
	{
		NO_FRAME,
		UN_USED_ACTION,
		BUTTON_NONE,
		{NO_TASK},
		{TXT_NONE}
	},
	(TIME_LR)0
};

static const PERIODIC_EVENT m_NullPeriodicEvent =
{
	{
		NO_FRAME,
		NO_ACTION,
		BUTTON_NONE,
		{NO_TASK},
		{TXT_NONE}
	},
	(TIME_LR)0
};

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   UI_ClearEvent()
;
; Description:
;   Empties the event (no turn, no press, etc.)
;
; Parameters:
;   pEvent  =>  Event to be cleared out.
;
; Reentrancy:
;   No.
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void UI_ClearEvent(PERIODIC_EVENT *pEvent)
{
	if(pEvent == NULL)
	{
		// Just ignore a request to clear an event that does not exist.
		return;
	}
	*pEvent = m_NullPeriodicEvent;
}// End UI_ClearEvent()

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   clearPendingEvent()
;
; Description:
;   Empties the event (no turn, no press, etc.)
;
; Parameters:
;   pPendingEvent  => Pending Event to be cleared out.
;
; Reentrancy:
;   No.
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static void clearPendingEvent(PENDING_EVENT *pPendingEvent)
{
	if(pPendingEvent == NULL)
	{
		// Just ignore a request to clear an event that does not exist.
		return;
	}
	pPendingEvent->Event = m_NullPeriodicEvent;
	pPendingEvent->bEntryAdded = false;
}// End clearPendingEvent()

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   isHigherPriority();
;
; Description:
;   This function determines if an action is higher priority than another action
;
; Parameters:
;   ACTION Source - the action to be checked if it has higher priority
;   ACTION Dest - the action that is being checked against
;
; Returns:
;   True if pSource has a higher priority than pDest
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static BOOL isHigherPriority(ACTION Source, ACTION Dest)
{
	return (Source.eActionType > Dest.eActionType);
}// End isHigherPriority()

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   CheckWaterMarkInPendingArray();
;
; Description:
;   This function determines the largest number of events that have been in the
;   pending array at one time since that array was last initialized.
;
; Returns:
;   U_BYTE => The largest number of pending array elements that have been used.
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
U_BYTE CheckWaterMarkInPendingArray(void)
{
	INT32 nIndex;

	for(nIndex = (PERIODIC_INTERRUPT_LIST_LENGTH - 1); nIndex >= 0; nIndex-- )
	{
		if(m_PendingEvents[nIndex].Event.Action.eActionType != UN_USED_ACTION)
		{
			break;
		}
	}
	return (U_BYTE)(nIndex + 1);
}// End CheckWaterMarkInPendingArray()

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   InitPeriodicEvents();
;
; Description:
;   This function initializes the Periodic Event handler and the Frame Instance
;   list. It must be called once before attempting to use the UI Processing sub-system.
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void InitPeriodicEvents(void)
{
	INT16 loopy;
	// Create the Periodic Interrupt queue. Initialize all elements to the Null Element
	for (loopy = 0; loopy < PERIODIC_INTERRUPT_LIST_LENGTH; loopy++)
	{
		// Setup the queue that is processed via the main loop
		m_PeriodicEvents[loopy] = m_InitPeriodicEvent;
		// Setup the pending event queue that is accessed via the cycle handler
		m_PendingEvents[loopy].Event = m_InitPeriodicEvent;
		m_PendingEvents[loopy].bEntryAdded = false;
	}
	m_bAllowPushActions = false;
	m_nPendingOverFlowCnt = 0;
}

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   insertEventInPeriodicArray();
;
; Description:
;   This function inserts an event into the Periodic Event Array
;   (m_PeriodicEvents) at the specified array index.
;
;   If the specified array index is out of range, selection of the last entry
;   in the array will be assumed.
;
;   If the specified index selects an empty event (NO_ACTION or UN_USED_ACTION),
;   or if it selects the last entry in the array, then the new event will be
;   copied directly into that entry and no shifting of entries is required.
;
;   If the specified index selects an existing event, then all entries in the
;   array will be shifted up one entry beginning with the specified entry.
;   The new event is then copied into the array at the specified index.  If the
;   array is full, then this will result in the last event in the array being
;   discarded.
;
; Parameters:
;   pEvent   - The address of the event to be inserted into the Periodic Event
;              Array.
;   nAtIndex - The array index where the new event will be inserted.
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static void insertEventInPeriodicArray(PERIODIC_EVENT *pEvent, INT32 nAtIndex)
{
	INT32 nWorkingIndex;
	BOOL  bArrayMoved = false;

	if (pEvent == NULL)
	{
		//ErrorState(ERR_NULL_PTR);
		return;
	}
	// If the index is out of range, just put this entry at the end of the array.
	if (nAtIndex >= PERIODIC_INTERRUPT_LIST_LENGTH)
	{
		nAtIndex = PERIODIC_INTERRUPT_LIST_LENGTH - 1;
	}
	// If the selected entry
	if ((m_PeriodicEvents[nAtIndex].Action.eActionType > NO_ACTION) &&
		(nAtIndex < (PERIODIC_INTERRUPT_LIST_LENGTH - 1)))
	{
		// Work from the top of the array down to the entry that we want to manipulate
		// Evaluate two conditions.
		// The first is to determine if we have hit the entry that we want manipulate.
		// The second is to check for roll under when the index we want to manipulate is a zero.
		for( nWorkingIndex = (PERIODIC_INTERRUPT_LIST_LENGTH - 2);
			((nWorkingIndex >= nAtIndex) && (nWorkingIndex >= 0));
			nWorkingIndex-- )
		{
			//Don't do any work until we find the first real data to move.
			if((m_PeriodicEvents[nWorkingIndex].Action.eActionType > NO_ACTION) || bArrayMoved)
			{
				m_PeriodicEvents[nWorkingIndex + 1] = m_PeriodicEvents[nWorkingIndex];
				bArrayMoved = true;
			}
		}
	}
	// Finally put the new entry into the array at the point that we wanted.
	m_PeriodicEvents[nAtIndex] = *pEvent;
}

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   compressPeriodicArray();
;
; Description:
;   This function finds the first empty entry (NO_ACTION) in the periodic
;   events queue and replaces that entry with the next following entry that is
;   not empty.  This will then be repeated for each subsequent entry until all
;   used entries are contiguous at the front of the array.  The order of the
;   used entries will not be changed.  Each entry that is moved is replaced by
;   an empty entry. No action will be taken if the array is already compressed,
;   completely empty, or completely unused (all UN_USED_ACTION).
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static void compressPeriodicArray(void)
{
	INT32 nFromIndex;
	INT32 nEmptyIndex;
	// Find the first empty entry.
	for(nEmptyIndex = 0; nEmptyIndex < PERIODIC_INTERRUPT_LIST_LENGTH; nEmptyIndex++)
	{
		if (m_PeriodicEvents[nEmptyIndex].Action.eActionType == NO_ACTION)
		{
			break;
		}
		else if (m_PeriodicEvents[nEmptyIndex].Action.eActionType == UN_USED_ACTION)
		{
			// We have come to the unused entries at the end of the array.
			// There is nothing to compress.  Just return.
			return;
		}
	}
	// Start with the entry following the empty entry and find the next used entry.
	// Replace the empty entry with the used entry and then empty that used entry.
	// Increment the empty index to select the next entry, which is now guaranteed
	// to be empty.  Repeat this process until the last used entry has been moved.
	for(nFromIndex = nEmptyIndex + 1; nFromIndex < PERIODIC_INTERRUPT_LIST_LENGTH; nFromIndex++)
	{
		if (m_PeriodicEvents[nFromIndex].Action.eActionType > NO_ACTION)
		{
			// This entry is in use.  Copy it to the empty entry.
			// (Lint does not understand that if nEmptyIndex exits the
			// previous for loop with a value greater than or equal to
			// (PERIODIC_INTERRUPT_LIST_LENGTH - 1), then this for loop will
			// not execute because nFromIndex will be out of range. Suppress e661.)
			m_PeriodicEvents[nEmptyIndex] = m_PeriodicEvents[nFromIndex];//lint !e661
			// Make this used entry an empty entry.
			m_PeriodicEvents[nFromIndex] = m_NullPeriodicEvent;
			// The entry following what was the empty entry is now the new empty
			// entry.
			nEmptyIndex++;
		}
		else if (m_PeriodicEvents[nFromIndex].Action.eActionType == UN_USED_ACTION)
		{
			// We have come to the unused entries at the end of the array.
			// There is nothing more to compress.  Just return.
			return;
		}
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   AddPeriodicEvent();
;
; Description:
;   This function places a new Periodic Element on the pending events queue.
;
; Parameters:
;   pEvent - Address of the event that will need to be processed.
;
; Returns:
;   true/false - True if the element was added, false is there is no room on the queue
;
; Reentrancy:
;   No
;
; Assumptions
;   If the queue is blocked, an event can be dropped
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
BOOL AddPeriodicEvent(const PERIODIC_EVENT *pEvent)
{
//	U_INT32 nOldPSW;
	INT32	loopy;
	BOOL    bEventAdded = false;

	// If push actions are not allowed (for example, during an animation),
	// the action is not added.
	if((pEvent->Action.eActionType == PUSH) && !m_bAllowPushActions)
	{
		return false;
	}
//	nOldPSW = ReadInterruptStatusAndDisable();
	for(loopy = 0; loopy < PERIODIC_INTERRUPT_LIST_LENGTH; loopy++)
	{
		// Run through the Pending list to see if there is room to add an event.
		if(!m_PendingEvents[loopy].bEntryAdded)
		{
			// Good to load the event into the queue.
			clearPendingEvent(&m_PendingEvents[loopy]);
			m_PendingEvents[loopy].Event = *pEvent;
			// Assigning this element as "Entry Added" completes the entry.
			// The cycle handler will move this entry from the pending list
			// to the periodic list the next time it re-orders the list.
			m_PendingEvents[loopy].bEntryAdded = true;
			bEventAdded = true;
			break;
		}
	}
//	RestoreInterruptStatus(nOldPSW);
	if(!bEventAdded)
	{
		m_nPendingOverFlowCnt++;  //Queue was full
	}
	return bEventAdded;
}

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   GetNextPeriodicEvent();
;
; Description:
;   This function copies the next (if any) event to be processed
;
; Parameters:
;   PERIODIC_EVENT *pEvent - structure to hold potential event
;
; Returns:
;   true/false - True if there is an element to process, false otherwise
;
; Reentrancy:
;   No
;
; Assumptions:
;   This will be called out of the Main while(1)
;   loop only, or out of the error loop if the device is in error state.
;   The function is called as fast as the processor can handle.
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
BOOL GetNextPeriodicEvent(PERIODIC_EVENT *pEvent)
{
//	PERIODIC_EVENT NewEvent;
	ACTION LocalAction;
	INT32 i, j = 0;
	BOOL bEventAdded = false;
	BOOL bCurrentEventAdded = false;
	TIME_LR tCurrentTime = ElapsedTimeLowRes((TIME_LR)0);

	// Check the pending elements and re-order the  list if necessary.
	//These elements have been added by the cycle handler.
	for ( i = 0; i < PERIODIC_INTERRUPT_LIST_LENGTH; i++)
	{
		// See if there is a PendingEvent in the queue.  Also check to see if it is time to move it to the
		// PeriodicEvent queue.
		if((m_PendingEvents[i].bEntryAdded) && (tCurrentTime >= m_PendingEvents[i].Event.tTriggerTime))
		{
			// Examine the event for priority and adjust the queue
			for(j = 0; j < PERIODIC_INTERRUPT_LIST_LENGTH; j++)
			{
				if((m_PeriodicEvents[j].Action.eActionType == NO_ACTION) ||
					(m_PeriodicEvents[j].Action.eActionType == UN_USED_ACTION))
				{
					m_PeriodicEvents[j]= m_PendingEvents[i].Event;
					//Set flag to show the event Q needs to be sorted
					bCurrentEventAdded = true;
					break;
				}
				else if(isHigherPriority(m_PendingEvents[i].Event.Action, m_PeriodicEvents[j].Action))
				{
					// Move the list down and insert this element
					insertEventInPeriodicArray(&m_PendingEvents[i].Event, j);
					//Set flag to show the event Q needs to be sorted
					bCurrentEventAdded = true;
					break;
				}
			}
			if(bCurrentEventAdded)
			{
				bEventAdded = true;
				m_PendingEvents[i].bEntryAdded = false;
			}
		}
	}
	//There are no actions to be processed, copy null event and return.
	if(m_PeriodicEvents[0].Action.eActionType == NO_ACTION)
	{
		return false;
	}
	//only combine movements if new events added
	if(bEventAdded)
	{
		//scroll through queue to combine like button pushes
		//and similar button pushes (e.g. short-depress knob push) are combined into 1 event
		//screen tasks and humidifier changes are left alone
		for(i = 0; i < PERIODIC_INTERRUPT_LIST_LENGTH; i++)
		{
			if(m_PeriodicEvents[i].Action.eActionType > NO_ACTION)
			{
				LocalAction = m_PeriodicEvents[i].Action;
				for(j = i+1; j < PERIODIC_INTERRUPT_LIST_LENGTH; j++)
				{
					if ((LocalAction.eActionType == m_PeriodicEvents[j].Action.eActionType) && (LocalAction.eActionType == PUSH))
					{
						m_PeriodicEvents[j] = m_NullPeriodicEvent;
					}
					else if ((LocalAction.eActionType == m_PeriodicEvents[j].Action.eActionType) && (LocalAction.eActionType == SCREEN))
					{
						// combine screen tasks if the task and the destination frame id are the same.
						if ((LocalAction.ScreenTask == m_PeriodicEvents[j].Action.ScreenTask) &&
						(LocalAction.eFrameID == m_PeriodicEvents[j].Action.eFrameID))
						{
							m_PeriodicEvents[j] = m_NullPeriodicEvent;
						}
					}
				}
			}
		}
		compressPeriodicArray();
	}
	popNextPeriodicEvent(pEvent);
	return pEvent->Action.eActionType != NO_ACTION;
}

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   popNextPeriodicEvent();
;
; Description:
;   Get a copy of the next event on the periodic event queue, then remove that
;   event from the periodic event queue.
;
; Parameters:
;   PERIODIC_EVENT *pEvent - structure to hold a copy of the next event
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static void popNextPeriodicEvent(PERIODIC_EVENT *pEvent)
{
	*pEvent = m_PeriodicEvents[0];
	// Shift up from element 1 to element 0. Bottom will be replaced
	m_PeriodicEvents[0] = m_NullPeriodicEvent;
	compressPeriodicArray();
}

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   ProcessPeriodicEvent();
;
; Description:
;   This function processes a periodic event
;
; Parameters:
;   PERIODIC_EVENT *pEvent - pointer to periodic event
;
; Reentrancy:
;   No
;
; Assumptions:
;   This will be called out of the Main while(1) loop only.
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void ProcessPeriodicEvent(PERIODIC_EVENT *pEvent)
{
	const FRAME* frame;

	if ( pEvent == NULL )
	{
		return;
	}

	if(pEvent->Action.eFrameID != NO_FRAME)
	{
		frame = GetFrame(pEvent->Action.eFrameID);
	}
	else
	{
		frame = UI_GetActiveFrame();
	}

	if ( (pEvent->Action.ScreenTask == REPAINT) && (frame->Paint) )
	{
		SetAllowKeypadActions(false);
		frame->Paint((FRAME*)frame);
		SetAllowKeypadActions(true);
	}
	else if (pEvent->Action.eActionType == TIMER_ELAPSED)
	{
		TAB_ENTRY* tab = GetActiveTab();
		if (tab->OneSecondTimerElapsed)
		{
			tab->OneSecondTimerElapsed(tab);
		}
		RepaintNow(&StatusFrame);
	}
	else if(frame->ProcessEvent != NULL)
	{
		frame->ProcessEvent(pEvent);
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   UI_RemoveAllEventsQueue()
;
; Description:
;   This clears all events from the periodic event Q
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void UI_RemoveAllEventsQueue(void)
{
	INT32 loopy;

	for (loopy = 0; loopy < PERIODIC_INTERRUPT_LIST_LENGTH; loopy++)
	{
		// Make all used events empty.
		// Both empty and unused events will simply be ignored.
		if (m_PeriodicEvents[loopy].Action.eActionType > NO_ACTION)
		{
			m_PeriodicEvents[loopy] = m_NullPeriodicEvent;
		}
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   UI_ClearPendingEventsQueue()
;
; Description:
;   This clears all events from the pending event Q
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void UI_ClearPendingEventsQueue(void)
{
	INT32 loopy;

	for (loopy = 0; loopy < PERIODIC_INTERRUPT_LIST_LENGTH; loopy++)
	{
		if(m_PendingEvents[loopy].Event.Action.eActionType > NO_ACTION)
		{
			clearPendingEvent(&m_PendingEvents[loopy]);
		}
		else
		{
			// The pending events queue is always in a compressed state.
			// Stop as soon as we find the first empty or unused entry.
			break;
		}
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   UI_FlushBothEventQueues()
;
; Description:
;   This clears all events from both the pending event queue and the periodic
;   event queue.
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void UI_FlushBothEventQueues(void)
{
	UI_ClearPendingEventsQueue();
	UI_RemoveAllEventsQueue();
}

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   AddButtonEvent()
;
; Description:
;   Puts a Button Push Event on to the Periodic Event queue
;
; Parameters:
;   BUTTON_VALUE nButtonValue -  The button value.
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void AddButtonEvent(BUTTON_VALUE nButtonValue)
{
	PERIODIC_EVENT event;

	UI_ClearEvent(&event);
	event.Action.eActionType = PUSH;
	event.Action.nValue = nButtonValue;
	event.tTriggerTime = ElapsedTimeLowRes((TIME_LR)0);
	(void)AddPeriodicEvent(&event);
	Reset_Event_Flag();
}

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   AddButtonEventWithFrame()
;
; Description:
;   Puts a Button Push Event on to the Periodic Event queue
;
; Parameters:
;   BUTTON_VALUE nButtonValue -  The button value.
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void AddButtonEventWithFrame(BUTTON_VALUE nButtonValue, FRAME_ID eFrameID)
{
	PERIODIC_EVENT event;

	UI_ClearEvent(&event);
	event.Action.eFrameID = eFrameID;
	event.Action.eActionType = PUSH;
	event.Action.nValue = nButtonValue;
	event.tTriggerTime = ElapsedTimeLowRes((TIME_LR)0);
	(void)AddPeriodicEvent(&event);
	Reset_Event_Flag();
}

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   AddScreenEvent()
;
; Description:
;   Put a screen event on the periodic queue for immediate action.
;
; Parameters:
;   SCREEN_TASK eTask        - The SCREEN_TASK of the screen event action.
;   FRAME_ID eFrameID - The ID of the frame to which the event applies.
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void AddScreenEvent(SCREEN_TASK eTask, FRAME_ID eFrameID, TIME_LR tTrigger)
{
	PERIODIC_EVENT event;

	UI_ClearEvent(&event);

	if(eTask != NO_TASK)
	{
		event.Action.eActionType = SCREEN;
		event.Action.ScreenTask = eTask;
		event.Action.eFrameID = eFrameID;
		event.tTriggerTime = (ElapsedTimeLowRes((TIME_LR)0) + tTrigger);
		(void)AddPeriodicEvent(&event);
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   SetAllowKeypadActions();
;
; Description:
;   This function gates if key actions are allowed to be added to the queue
;
; Parameters:
;   BOOL bAllow - true is allow key actions
;                 false is don't allow key actions
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void SetAllowKeypadActions(BOOL bAllow)
{
	m_bAllowPushActions = bAllow;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   UI_KeyActivity();
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
BOOL UI_KeyActivity(void)
{
	return EventFlag;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   Reset_Event_Flag();
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void Reset_Event_Flag(void)
{
	EventFlag = true;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   SetUIKeyPressEvent();
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void SetUIKeyPressEvent(void)
{
	EventFlag = false;
}
