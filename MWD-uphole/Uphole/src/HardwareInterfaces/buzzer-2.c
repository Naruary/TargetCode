/*!
********************************************************************************
*       @brief      This module provides functions to handle the use of the 
*                   onboard buzzer.  Documented by Josh Masters Dec. 2014
*       @file       Uphole/src/HardwareInterfaces/buzzer.c
*       @author     Bill Kamienik
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <intrinsics.h>
#include <stm32f4xx.h>
#include "portable.h"
#include "board.h"
#include "buzzer.h"
#include "SecureParameters.h"
#include "systick.h"
#include "timer.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//
//
// These are the values passed to SetBuzzerToneType() as arguments.
// These represent the bits in the tone queue bitfield.
//
#define NO_BUZZER_TONES                 0
#define BUZZER_CONTINUOUS               (1 << (U_BYTE)CONTINUOUS_TONE)
#define BUZZER_KEY_PRESS                (1 << (U_BYTE)CONFIRMATION)

typedef struct
{
    TIME_LR tOn;
    TIME_LR tOff;           // only matters if nNumPulses > 1
    TIME_LR tGap;           // only matters if nNumPulseTrains > 1 or infinite
    U_BYTE nNumPulses;      // 0 means continuous tone
    U_BYTE nNumPulseTrains; // 0 means infinite pulse train
} BUZZER_TONE;

//============================================================================//
//      MACROS                                                                //
//============================================================================//

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

typedef U_INT32 ACTIVE_TONE_BITFIELD;

//
// These are the values used for indexing the tone queue bitfield.
//
typedef enum
{
    CONTINUOUS_TONE,
    CONFIRMATION,
    NUM_TONE_TYPES  // this must be the last entry
} BUZZER_TONE_TYPE;

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

///@brief  
///@param  
///@return
static void beeperControl(BUZZER_TONE_TYPE eIndex);

///@brief  
///@param  
///@return
static void updateToneQueue(BUZZER_TONE_TYPE eIndex);

///@brief  
///@param  
///@return
static BUZZER_TONE_TYPE findHighestPriorityTone(void);

///@brief  
///@param  
///@return
static void setBuzzerToneType(ACTIVE_TONE_BITFIELD bfToneOn, ACTIVE_TONE_BITFIELD bfToneOff);

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

///@brief 
static const BUZZER_TONE m_BuzzerTone[NUM_TONE_TYPES] =
{
//tOn, tOff, tGap, NumPulse, NumPulseTrains
    {0, 0, 0, 0, 0},                                         // CONTINUOUS_TONE
    {10*TEN_MILLI_SECONDS, 0, 0, 1, 1},                       // KEY_PRESS_TONE
//    {5*TEN_MILLI_SECONDS, 5*TEN_MILLI_SECONDS, 50*TEN_MILLI_SECONDS, 3, 1},// ALARM
};

///@brief 
static const U_INT16 m_nMask[NUM_TONE_TYPES] =
{
    BUZZER_CONTINUOUS,
    BUZZER_KEY_PRESS,
//    BUZZER_ALARM,
};

///@brief 
static ACTIVE_TONE_BITFIELD m_bfTones = NO_BUZZER_TONES;

///@brief 
static BOOL m_bBuzzerWasActive;

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   BUZZER_InitPins()
;
; Description:
;   Initializes the Buzzer by initializing the GPIO's
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void BUZZER_InitPins(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_StructInit(&GPIO_InitStructure);

    // GPIO LED status Pins
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;

    GPIO_InitStructure.GPIO_Pin  = AUDIBLE_ALARM_PIN;
    GPIO_Init(AUDIBLE_ALARM_PORT, &GPIO_InitStructure);

    GPIO_WriteBit(AUDIBLE_ALARM_PORT, AUDIBLE_ALARM_PIN, Bit_RESET);
} // end BUZZER_InitPins

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   SetBuzzerToneType()
;
; Description:
;   First sets the bfToneOn bits then clears the bfToneOff bits in the
;   bitfield.
;
; Parameters:
;       ACTIVE_TONE_BITFIELD bfToneOn - bit pattern indicating which tone(s)
;                               should be turned on.
;       ACTIVE_TONE_BITFIELD bfToneOff - bit pattern indicating which tone(s)
;                               should be turned off.
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
static void setBuzzerToneType(ACTIVE_TONE_BITFIELD bfToneOn, ACTIVE_TONE_BITFIELD bfToneOff)
{
    m_bfTones |= bfToneOn;
    m_bfTones &= ~bfToneOff;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   BuzzerHandler()
;
; Description:
;   Monitors the tones bitfield and reacts to the highest priority
;   tone.  More than one tone can exist at once, but only the highest
;   priority can be active.
;
; Reentrancy:
;   No
;
; Assumptions:
;   This is the Buzzer State machine and is called from the CycleHandler() at a
;   10 ms period.
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void BuzzerHandler(void)
{
//    if(m_bBuzzerWasActive)
    {
        if(m_bfTones != NO_BUZZER_TONES)
        {
            m_bBuzzerWasActive = TRUE;
            //
            // This is the active tone buzzer control state machine
            //
            beeperControl(findHighestPriorityTone());
        }

        // This conditional is intentionally not an "else" of the above conditional
        // because we want another cycle to go by before stopping the buzzer.
        // This makes for a cleaner sound to the tone as it goes off.
        if((m_bfTones == NO_BUZZER_TONES) && m_bBuzzerWasActive)
        {
            m_bBuzzerWasActive = FALSE;
            M_BuzzerOff();
        }
    }
}

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   beeperControl()
;
; Description:
;   Handles beeping the buzzer according to the defined pattern.  Defined
;   patterns (tones) are limited to simple pulses, constant periodic pulse trains,
;   and continuous tone.
;
; Paramters:
;      U_BYTE eIndex - The index of the tone that is currently active.
;
; Reentrancy:
;   No
;
; Assumptions:
;   This function is not called if there are no active tones.
;
;   Only the highest priority active tone is passed in to the function
;   when called.
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
static void beeperControl(BUZZER_TONE_TYPE eIndex)
{
    static TIME_LR tBeepTime;
    static U_BYTE nPulseCounter, nTrainCounter;
    static BUZZER_TONE_TYPE eLastIndex = NUM_TONE_TYPES;
    static enum
    {
        BEEP_START,
        BEEP_STOP,
        BEEP_NEXT,
        BEEP_CHECK_FOR_REPEAT,
        BEEP_DONE
    } eBeepState = BEEP_START;

    //
    // Ignore invalid tone index values.
    //
    if (eIndex >= NUM_TONE_TYPES)
    {
        return;
    }

    //
    // If a new tone comes in, reset everything and start the new one
    // immediately.
    //
    if (eIndex != eLastIndex)
    {
        M_BuzzerOff();
        nPulseCounter = nTrainCounter = 0;
        eBeepState = BEEP_START;

        eLastIndex = eIndex;
    }

    do
    {
        switch( eBeepState )
        {
            case BEEP_START:
                if(GetBuzzerAvailable())
                {
                    M_BuzzerOn();
                }

                if( m_BuzzerTone[eLastIndex].nNumPulses == 0 )
                {
                    // it's a single continuous tone so don't turn off, we're finished
                    eBeepState = BEEP_DONE;
                }
                else
                {
                    //We are in charge of starting the timer here.  This is for the On Time.
                    tBeepTime = ElapsedTimeLowRes(START_LOW_RES_TIMER);
                    eBeepState = BEEP_STOP;
                }
                break;

            case BEEP_STOP:
                if( ElapsedTimeLowRes(tBeepTime) >= m_BuzzerTone[eLastIndex].tOn )
                {
                    //We are in charge of resetting the timer here.
                    //This is for the inter-beep delay, and the inter-train delay.
                    tBeepTime = ElapsedTimeLowRes(START_LOW_RES_TIMER);
                    M_BuzzerOff();
                    eBeepState = BEEP_NEXT;
                }
                break;

            case BEEP_NEXT:
                if( ElapsedTimeLowRes(tBeepTime) >= m_BuzzerTone[eLastIndex].tOff )
                {
                    //Don't reset the timer here.  BEEP_START will take care of it.
                    //10mS has expired already, so if we are starting a train we can still use
                    //this value.  We will be 10mS late on the start of the train otherwise.

                    if( m_BuzzerTone[eLastIndex].nNumPulses > ++nPulseCounter )
                    {
                        eBeepState = BEEP_START;
                    }
                    else
                    {
                        nTrainCounter++;
                        eBeepState = BEEP_CHECK_FOR_REPEAT;
                    }
                }
                break;

            case BEEP_CHECK_FOR_REPEAT:
                if( (m_BuzzerTone[eLastIndex].nNumPulseTrains > nTrainCounter) ||
                    (m_BuzzerTone[eLastIndex].nNumPulseTrains == 0) /*repeat forever*/ )
                {
                    if( ElapsedTimeLowRes(tBeepTime) >= m_BuzzerTone[eLastIndex].tGap )
                    {
                        //Don't reset the timer here.  BEEP_START will take care of it.
                        nPulseCounter = 0;
                        eBeepState = BEEP_START;
                    }
                }
                else
                {
                    //
                    // this tone is finished
                    // initialize for the next tone
                    //
                    updateToneQueue(eLastIndex);
                    eLastIndex = NUM_TONE_TYPES;
                    nPulseCounter = nTrainCounter = 0;
                    eBeepState = BEEP_START;
                }
                break;

            case BEEP_DONE:
                if (eIndex == CONTINUOUS_TONE)
                {
                    //
                    // it's not new but somebody may have just restarted a continuous
                    // tone
                    //
                    if(GetBuzzerAvailable())
                    {
                        M_BuzzerOn();
                    }
                }
                break;

            default:
                break;
        }
    }
    while(eBeepState == BEEP_START);//End do.
    //The do{}while(BEEP_START) is here to remove the extra 10mS that is added
    //by waiting for the cycle handler to call us again during a repeat beep.
}

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   updateToneQueue()
;
; Description:
;   After an active tone of the highest priority is complete, that is the
;   generation of its tone is finished, disable it and any non-infinite periodic
;   tone of lower priority that is still in the queue. We do this because we
;   don't want to sound tones long after the actual event cause has passed.
;   Infinite periodic signals are expected to remain active long after the event.
;
; Parameters:
;   eIndex - The index of the tone that is currently active.
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
static void updateToneQueue(BUZZER_TONE_TYPE eIndex)
{
    // protect against indexing beyond the size of m_nMask
    if (eIndex < NUM_TONE_TYPES)
    {
        // remove the specified active tone
        m_bfTones &= ~m_nMask[eIndex++];

        // remove any lower priority non-periodics
        // eIndex is already initialized before entering the loop.
        while (eIndex < NUM_TONE_TYPES)
        {
            // Infinite periodic signals have nNumPulseTrains of zero.
            if (m_BuzzerTone[eIndex].nNumPulseTrains != 0)
            {
                m_bfTones &= ~m_nMask[eIndex];
            }
            eIndex++;
        }
    }
}

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   findHighestPriorityTone()
;
; Description:
;   Searches the tone queue for the highest priority tone and returns an index
;   to that tone's definition in m_BuzzerTone[].  If there are no active
;   tones, then NUM_TONE_TYPES will be returned.
;
; Returns:
;   tone_TONE_TYPE - enumerated index of the highest priority tone in the
;                     queue
;
; Assumptions:
;   Tone definitions are defined in order of decreasing priority.
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
static BUZZER_TONE_TYPE findHighestPriorityTone(void)
{
    BUZZER_TONE_TYPE i = (BUZZER_TONE_TYPE)0;
    ACTIVE_TONE_BITFIELD mask = 1;

    // search for the highest active priority
    while(((m_bfTones & mask) == 0) && (i < NUM_TONE_TYPES))
    {
        mask <<= 1;
        i++;
    }

    return i;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

void BuzzerKeypress(void)
{
    setBuzzerToneType(BUZZER_KEY_PRESS, NO_BUZZER_TONES);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

void BuzzerAlarm(void)
{
    setBuzzerToneType(BUZZER_KEY_PRESS, NO_BUZZER_TONES);
//    setBuzzerToneType(BUZZER_ALARM, NO_BUZZER_TONES);
}