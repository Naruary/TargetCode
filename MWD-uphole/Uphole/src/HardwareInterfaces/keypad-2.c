/*!
********************************************************************************
*       @brief      This module provides functions to handle the use of the
*                   keypad.  Documented by Josh Masters Dec. 2014
*       @file       Uphole/src/HardwareInterfaces/keypad.c
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
#include <string.h>
#include "portable.h"
#include "keypad.h"
#include "PeriodicEvents.h"
#include "systick.h"
#include "timer.h"
#include "Compass_Panel.h"
#include "UI_api.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

#define COL_PORT    GPIOE
#define SW_PORT     GPIOE
#define ROW_PORT_12 GPIOB
#define ROW_PORT_34 GPIOE

#define COL_ONE     GPIO_Pin_0
#define COL_TWO     GPIO_Pin_1
#define COL_THREE   GPIO_Pin_2
#define COL_FOUR    GPIO_Pin_3

#define ROW_ONE     GPIO_Pin_8
#define ROW_TWO     GPIO_Pin_9
#define ROW_THREE   GPIO_Pin_5
#define ROW_FOUR    GPIO_Pin_4

#define SW_ONE      GPIO_Pin_12  // Shift Button
#define SW_TWO      GPIO_Pin_13  // Survey/Select Button
#define SW_THREE    GPIO_Pin_14
#define SW_FOUR     GPIO_Pin_15

#define SW_ONE_MASK     0x01
#define SW_TWO_MASK     0x02
#define SW_THREE_MASK   0x04
#define SW_FOUR_MASK    0x08

// This is the mask for all four external switches.
//#define SW_MASK         0xF000
// This one only uses the first three and assigns the fourth to an IRQ.
#define SW_MASK         0x7000

#define COL_MASK        0x000F

#define KEY_BUFFR_SIZE      8

#define	MAX_KEYPAD_ROWS     4
#define	KEYPAD_ROW0         0
#define	KEYPAD_ROW1         1
#define	KEYPAD_ROW2         2
#define	KEYPAD_ROW3         3

#define NO_KEY                  0x0000
#define KEY_ZERO                0x2000
#define KEY_ONE                 0x0001
#define KEY_TWO                 0x0002
#define KEY_THREE               0x0004
#define KEY_FOUR                0x0010
#define KEY_FIVE                0x0020
#define KEY_SIX                 0x0040
#define KEY_SEVEN               0x0100
#define KEY_EIGHT               0x0200
#define KEY_NINE                0x0400
#define KEY_DOT                 0x1000
#define KEY_DASH                0x4000
#define KEY_UP                  0x0008
#define KEY_DOWN                0x0080
#define KEY_LEFT                0x8000
#define KEY_RIGHT               0x0800

#define KEY_SELECT              0x0200
#define KEY_SHIFT               0x0400

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

typedef enum __KEYPAD_CAPTURE_STATE__
{
    KEYPAD_READ_ROW,
    KEYPAD_BUTTONS,
    KEYPAD_PROCESS,
    KEYPAD_WAITING,
    MAX_KEYPAD// <---- Must be the LAST entry
} KEYPAD_CAPTURE_STATE;

typedef struct
{
    U_BYTE          bKeyBufferEmpty     :1;
    U_BYTE          bKeyBufferFull      :1;
    U_BYTE          nKeyBufferHead      :3;
    U_BYTE          nKeyBufferTail      :3;

    U_BYTE          nKeyBuffer[KEY_BUFFR_SIZE];
}KEY_BUFFER_DATA_STRUCT;

typedef struct
{
    U_BYTE eKeyState            : 3 ; //
    U_BYTE nDebounced           : 2 ; //
    U_BYTE bPressed             : 1 ; // Pressed, but not necessarily debounced
    U_BYTE bFullPressCycle      : 1 ; // Open->Closing->Closed->Opening->Open
    U_BYTE bKeySent             : 1 ; //
    U_BYTE nKeyValue;
} KEY_DATA_TYPE;

typedef struct
{
    TIME_LR         nHoldTime;
    KEY_DATA_TYPE   bfKey;
} BUTTON_DATA;

typedef struct
{
    BOOL    bActive;
    TIME_LR tHoldTimer;
} EVENT_DATA;

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

///@brief
///@param
///@return
static void keypadReadRow(void);

///@brief
///@param
///@return
static void keypadBuildResult(void);

///@brief
///@param
///@return
static void keyStateManager(void);

///@brief
///@param
///@return
static void runKeyStateMachine(KEYS eKey);

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

///@brief
static KEYPAD_CAPTURE_STATE m_nKeypadSampleState;

///@brief
static BOOL    m_bKeyScanError = FALSE;

///@brief
static BOOL    m_nKeySent = FALSE;

///@brief
static U_BYTE  m_nSwitchScanResult  = 0;

///@brief
static BUTTON_VALUE  m_nKeyScanResult = BUTTON_NONE;

///@brief
static U_BYTE  m_nProcessedResult[MAX_KEYS];

///@brief
static U_BYTE m_nRowIndex = 0;

///@brief
static U_INT16 m_nKeypadRowData;

///@brief
static BUTTON_DATA m_Button[MAX_KEYS];

// The Event Value encapsulates all key states into a single value so that all
// keys can be processed together.
///@brief
static U_INT16 m_nEventValue;

///@brief
static const struct {
    GPIO_TypeDef*   GPIOx;
    uint16_t        GPIO_Pin;
    U_BYTE          nShiftCount;
}m_nKeypadRowDriver[4] = {
    {ROW_PORT_12, ROW_ONE, 0},
    {ROW_PORT_12, ROW_TWO, 4},
    {ROW_PORT_34, ROW_THREE , 8},
    {ROW_PORT_34, ROW_FOUR, 12}
};

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
;   KEYPAD_InitPins()
;
; Description:
;   Sets up keypad driver pins as inputs or output as required
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void KEYPAD_InitPins(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // This is a good configuration for input switches
    GPIO_StructInit(&GPIO_InitStructure);

    // This is the Switches
    GPIO_InitStructure.GPIO_Pin = (SW_ONE | SW_TWO | SW_THREE | SW_FOUR);
    GPIO_Init(SW_PORT, &GPIO_InitStructure);

    // This is the Keypad Columns
    GPIO_InitStructure.GPIO_Pin = (COL_ONE | COL_TWO | COL_THREE | COL_FOUR);
    GPIO_Init(COL_PORT, &GPIO_InitStructure);

    // GPIO LED status Pins
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;

    GPIO_InitStructure.GPIO_Pin = (ROW_ONE | ROW_TWO);
    GPIO_Init(ROW_PORT_12, &GPIO_InitStructure);

    GPIO_WriteBit(ROW_PORT_12, (ROW_ONE | ROW_TWO), Bit_SET);

    GPIO_InitStructure.GPIO_Pin = (ROW_THREE | ROW_FOUR);
    GPIO_Init(ROW_PORT_34, &GPIO_InitStructure);

    GPIO_WriteBit(ROW_PORT_34, (ROW_THREE | ROW_FOUR), Bit_SET);
}//end KEYPAD_InitPins

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   KeyPadManager()
;
; Description:
;
;
;
; Reentrancy:
;   No.
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void KeyPadManager(void)
{
    switch(m_nKeypadSampleState)
    {
      case KEYPAD_READ_ROW:
        keypadReadRow();

        if((++m_nRowIndex) >= MAX_KEYPAD_ROWS)
        {
            m_nKeypadRowData = ~m_nKeypadRowData;
            m_nKeypadSampleState = KEYPAD_BUTTONS;
        }
        break;

      case KEYPAD_BUTTONS:
        m_nSwitchScanResult = ((U_BYTE)(~((GPIO_ReadInputData(SW_PORT) & SW_MASK) >> 12)) & 0x07);
        m_nKeypadSampleState = KEYPAD_PROCESS;
        break;

      case KEYPAD_PROCESS:
        keypadBuildResult();
        keyStateManager();
        m_nKeypadSampleState = KEYPAD_WAITING;
        break;

      case KEYPAD_WAITING:
        asm("nop");
        break;
    }
}//end KeyPadManager

/*!
********************************************************************************
*       @details
*******************************************************************************/

void Keypad_StartCapture(void)
{
    m_nRowIndex = 0;
    m_nKeypadRowData = 0;
    m_nKeypadSampleState = KEYPAD_READ_ROW;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void keypadReadRow(void)
{
    GPIO_ResetBits(m_nKeypadRowDriver[m_nRowIndex].GPIOx, m_nKeypadRowDriver[m_nRowIndex].GPIO_Pin);
    m_nKeypadRowData |= ((GPIO_ReadInputData(COL_PORT) & COL_MASK) << m_nKeypadRowDriver[m_nRowIndex].nShiftCount);
    GPIO_SetBits(m_nKeypadRowDriver[m_nRowIndex].GPIOx, m_nKeypadRowDriver[m_nRowIndex].GPIO_Pin);
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void keypadBuildResult(void)
{
    switch(m_nKeypadRowData)
    {
        case NO_KEY:
            m_nKeyScanResult = BUTTON_NONE;
            if(m_bKeyScanError == TRUE)
            {
                m_bKeyScanError = FALSE;
            }
            break;
        case KEY_ZERO:
            m_nKeyScanResult = BUTTON_ZERO;
            break;
        case KEY_ONE:
            m_nKeyScanResult = BUTTON_ONE;
            break;
        case KEY_TWO:
            m_nKeyScanResult = BUTTON_TWO;
            break;
        case KEY_THREE:
            m_nKeyScanResult = BUTTON_THREE;
            break;
        case KEY_FOUR:
            m_nKeyScanResult = BUTTON_FOUR;
            break;
        case KEY_FIVE:
            m_nKeyScanResult = BUTTON_FIVE;
            break;
        case KEY_SIX:
            m_nKeyScanResult = BUTTON_SIX;
            break;
        case KEY_SEVEN:
            m_nKeyScanResult = BUTTON_SEVEN;
            break;
        case KEY_EIGHT:
            m_nKeyScanResult = BUTTON_EIGHT;
            break;
        case KEY_NINE:
            m_nKeyScanResult = BUTTON_NINE;
            break;

        case KEY_DOT:
            m_nKeyScanResult = BUTTON_PERIOD;
            break;
        case KEY_DASH:
            m_nKeyScanResult = BUTTON_DASH;
            break;

        case KEY_UP:
            m_nKeyScanResult = BUTTON_UP;
            break;
        case KEY_DOWN:
            m_nKeyScanResult = BUTTON_DOWN;
            break;
        case KEY_LEFT:
            m_nKeyScanResult = BUTTON_LEFT;
            break;
        case KEY_RIGHT:
            m_nKeyScanResult = BUTTON_RIGHT;
            break;

        default:
            m_nKeyScanResult = BUTTON_NONE;
            m_bKeyScanError = TRUE;
            break;
    }

    if(m_bKeyScanError == TRUE)
    {
        m_nKeyScanResult = BUTTON_NONE;
    }
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void keyStateManager(void)
{
#define NUM_COMBO_EVENTS 2
#define COMBO_COUNT 2
#define FIRST_KEY 0
#define SECOND_KEY 1

    typedef struct _COMBO_EVENTS_
    {
        BUTTON_TYPE         eKeyThatWillBeTheEvent;
        KEYS                eRequiredKeys[COMBO_COUNT];
        BOOL                bNeedsToBeSent;
    } COMBO_EVENT;

    KEYS eKey;
    static KEY_STATE nCurrentButtonState[MAX_KEYS];

    // Define a sequence of buttons that if they are both detected
    // "closed" at the same time, will send a SHORT_DEPRESS event
    // when both keys are lifted. Note: this will not report
    // until both have come open.
    //static COMBO_EVENT comboEvents[NUM_COMBO_EVENTS] = {
    //  {BOTH_SHIFT_SURVEY, {SHIFT_KEY, SURVEY_KEY}, 0},
    //  {BOTH_SHIFT_KEYPAD, {SHIFT_KEY, KEYPAD_KEY}, 0}};

    U_BYTE nTotalClosed = 0;
//    static BOOL bTooManyPushed = FALSE;
//    BOOL bAllOpen;
//    U_BYTE i;

    // Rest event value, keys will be shifted into the event value as they
    // are processed in the keys state machine.
    m_nEventValue = 0;

//    bAllOpen = TRUE;

    for (eKey = KEYPAD_KEY; eKey < MAX_KEYS; eKey++)
    {
        // Do the normal state machine work
        runKeyStateMachine(eKey);

        // Record the current state after state machine processing
        nCurrentButtonState[eKey] = (KEY_STATE)(m_Button[eKey].bfKey.eKeyState);

        if(nCurrentButtonState[eKey] == KEY_CLOSED)
        {
//            bAllOpen = FALSE;
            nTotalClosed++;
        }
    }

    if((m_nEventValue & 0x0FFF) != 0)
    {
        BOOL bKeyFound = TRUE;

        switch(m_nEventValue)
        {
            case KEY_SELECT:
                m_nEventValue = BUTTON_SELECT;
                break;
            case KEY_SHIFT:
                m_nEventValue = BUTTON_SHIFT;
                break;
            case BUTTON_ZERO:
            case BUTTON_ONE:
            case BUTTON_TWO:
            case BUTTON_THREE:
            case BUTTON_FOUR:
            case BUTTON_FIVE:
            case BUTTON_SIX:
            case BUTTON_SEVEN:
            case BUTTON_EIGHT:
            case BUTTON_NINE:
            case BUTTON_PERIOD:
            case BUTTON_DASH:
            case BUTTON_UP:
            case BUTTON_DOWN:
            case BUTTON_LEFT:
            case BUTTON_RIGHT:
                break;

            case BUTTON_SELECT + KEY_SHIFT:
                m_nEventValue = BUTTON_SHIFT_SELECT;
                break;
            case BUTTON_ZERO + KEY_SHIFT:
                m_nEventValue = BUTTON_SHIFT_ZERO;
                break;
            case BUTTON_ONE + KEY_SHIFT:
                m_nEventValue = BUTTON_SHIFT_ONE;
                break;
            case BUTTON_TWO + KEY_SHIFT:
                m_nEventValue = BUTTON_SHIFT_TWO;
                break;
            case BUTTON_THREE + KEY_SHIFT:
                m_nEventValue = BUTTON_SHIFT_THREE;
                break;
            case BUTTON_FOUR + KEY_SHIFT:
                m_nEventValue = BUTTON_SHIFT_FOUR;
                break;
            case BUTTON_FIVE + KEY_SHIFT:
                m_nEventValue = BUTTON_SHIFT_FIVE;
                break;
            case BUTTON_SIX + KEY_SHIFT:
                m_nEventValue = BUTTON_SHIFT_SIX;
                break;
            case BUTTON_SEVEN + KEY_SHIFT:
                m_nEventValue = BUTTON_SHIFT_SEVEN;
                break;
            case BUTTON_EIGHT + KEY_SHIFT:
                m_nEventValue = BUTTON_SHIFT_EIGHT;
                break;
            case BUTTON_NINE + KEY_SHIFT:
                m_nEventValue = BUTTON_SHIFT_NINE;
                break;
            case BUTTON_PERIOD + KEY_SHIFT:
                m_nEventValue = BUTTON_SHIFT_PERIOD;
                break;
            case BUTTON_DASH + KEY_SHIFT:
                m_nEventValue = BUTTON_SHIFT_DASH;
                break;
            case BUTTON_LEFT + KEY_SHIFT:
                m_nEventValue = BUTTON_SHIFT_LEFT;
                break;
            case BUTTON_RIGHT + KEY_SHIFT:
                m_nEventValue = BUTTON_SHIFT_RIGHT;
                break;
            case BUTTON_UP + KEY_SHIFT:
                m_nEventValue = BUTTON_SHIFT_UP;
                break;
            case BUTTON_DOWN + KEY_SHIFT:
                m_nEventValue = BUTTON_SHIFT_DOWN;
                break;

            default:
                bKeyFound = FALSE;
                break;
        }

        if((bKeyFound) && (!m_nKeySent))
        {
            // This if-else is to enable only '-' while analog meters are shown
            if(m_nEventValue != BUTTON_DASH && getCompassDecisionPanelActive() == TRUE)
            {
              m_nKeySent = FALSE;
              m_nEventValue = BUTTON_NONE;
              AddButtonEvent((BUTTON_VALUE)(m_nEventValue & 0x00FF));
//              AddButtonEvent(KEYPAD, SHORT_DEPRESS, (BUTTON_VALUE)(m_nEventValue & 0x00FF));
            }
            else if(m_nEventValue == BUTTON_DASH && getCompassDecisionPanelActive() == TRUE && (GetActiveTabFrame() == TAB3 || GetActiveTabFrame() == TAB4 || GetActiveTabFrame() == TAB5 || GetActiveTabFrame() == TAB6 || GetActiveTabFrame() == TAB7))
            {
              m_nKeySent = TRUE;
              AddButtonEvent((BUTTON_VALUE)(m_nEventValue & 0x00FF));
//              AddButtonEvent(KEYPAD, SHORT_DEPRESS, (BUTTON_VALUE)(m_nEventValue & 0x00FF));
              setCompassDecisionPanelActive(FALSE);
            }
            else
            {
              m_nKeySent = TRUE;
              AddButtonEvent((BUTTON_VALUE)(m_nEventValue & 0x00FF));
//              AddButtonEvent(KEYPAD, SHORT_DEPRESS, (BUTTON_VALUE)(m_nEventValue & 0x00FF));
            }
        }
    }
    else
    {
        m_nKeySent = FALSE;
    }
}

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   runKeyStateMachine(KEYS eKey)
;
; Description:
;   This function is a state machine for the keys. It reads the Key Status and
;   determines the Key State.
;
; Parameters:
;   KEYS eKey - Button to be processed.
;
; Reentrancy:
;   No
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
static void runKeyStateMachine(KEYS eKey)
{
#define DEBOUNCE_COUNT 2  // 20 ms

    switch(eKey)
    {
        case SHIFT_KEY:
            m_nProcessedResult[eKey] = ((m_nSwitchScanResult & SW_ONE_MASK) == SW_ONE_MASK);
            break;

        case SURVEY_KEY:
            m_nProcessedResult[eKey] = ((m_nSwitchScanResult & SW_TWO_MASK) == SW_TWO_MASK);
            break;

      case EXTRA_1_KEY:
            m_nProcessedResult[eKey] = ((m_nSwitchScanResult & SW_THREE_MASK) == SW_THREE_MASK);
            break;

        case EXTRA_2_KEY:
            m_nProcessedResult[eKey] = ((m_nSwitchScanResult & SW_FOUR_MASK) == SW_FOUR_MASK);
            break;

        case KEYPAD_KEY:
            m_nProcessedResult[eKey] = m_nKeyScanResult;
            break;

        case MAX_KEYS:
        default:
            return;
    }

    m_Button[eKey].bfKey.bPressed = ((m_nProcessedResult[eKey] != '\0') ? TRUE : FALSE);

    // Run the state machine:
    //
    //  Debounce is implemented by requiring Pressed or Released events over
    //  time before a key is considered closed or open.
    //
    //  State:         Event:           Actions:          New State:
    //
    //  KEY_OPEN       Released < 5 s   Button Press      KEY_OPEN
    //                 Released >= 5 s  Button Hold       KEY_OPEN
    //                 Pressed          None              KEY_CLOSING
    //  KEY_CLOSING    Released         None (Debounced)  KEY_OPEN
    //                 Pressed > 20 ms  Start Hold Timer  KEY_CLOSED
    //  KEY_CLOSED     Released         None (Debounced)  KEY_OPENING
    //                 Pressed          Do Stuck Test     KEY_CLOSED
    //                 Stuck            None              KEY_STUCK
    //  KEY_OPENING    Released > 20 ms Set Full Cycle    KEY_OPEN
    //                 Pressed          None (Debounced)  KEY_CLOSED
    //  KEY_STUCK      Released         None              KEY_LOOSENED
    //                 Pressed          None              KEY_STUCK
    //  KEY_LOOSENED   Released > 20 ms None              KEY_OPEN
    //                 Pressed          None (Debounced)  KEY_STUCK
    //
    switch (m_Button[eKey].bfKey.eKeyState)
    {
        case KEY_OPEN:
            if(m_Button[eKey].bfKey.bFullPressCycle)
            {
                m_Button[eKey].bfKey.bFullPressCycle = FALSE;
                m_Button[eKey].nHoldTime = 0;
            }
            else
            {
                if(m_Button[eKey].bfKey.bPressed)
                {
                    m_Button[eKey].bfKey.eKeyState = KEY_CLOSING;
                    m_Button[eKey].bfKey.nDebounced = 0;
                    m_Button[eKey].bfKey.nKeyValue = m_nProcessedResult[eKey];
                }
            }
            break;

        case KEY_CLOSING:
            if((m_Button[eKey].bfKey.bPressed == TRUE) && (m_Button[eKey].bfKey.nKeyValue == m_nProcessedResult[eKey]))
            {
                if(++m_Button[eKey].bfKey.nDebounced >= DEBOUNCE_COUNT)
                {
                    m_Button[eKey].bfKey.eKeyState = KEY_CLOSED;
                    m_Button[eKey].nHoldTime = ElapsedTimeLowRes(START_LOW_RES_TIMER);
                }
            }
            else
            {
                m_Button[eKey].bfKey.eKeyState = KEY_OPEN;
            }
            break;

        case KEY_CLOSED:
            // Shift the key value into the event value to be processed
//            if(!m_Button[eKey].bfKey.bKeySent)
            {
                if(eKey == KEYPAD_KEY)
                {
                    m_nEventValue |= (U_INT16)m_nProcessedResult[eKey];
                }
                else
                {
                    m_nEventValue |= ((U_INT16)1 << ((U_INT16)eKey + (U_INT16)8));
                }
            }

            if(m_Button[eKey].bfKey.bPressed)
            {
                // Now we really are a stuck button.
                if (ElapsedTimeLowRes(m_Button[eKey].nHoldTime) > THIRTY_SECOND)
                {
                    m_Button[eKey].bfKey.eKeyState = KEY_STUCK;
                }
            }
            else
            {
                m_Button[eKey].bfKey.eKeyState = KEY_OPENING;
                m_Button[eKey].bfKey.nDebounced = 0;
            }
            break;

        case KEY_OPENING:
            if(m_Button[eKey].bfKey.bPressed)
            {
                m_Button[eKey].bfKey.eKeyState = KEY_CLOSED;
            }
            else
            {
                if (++m_Button[eKey].bfKey.nDebounced >= DEBOUNCE_COUNT)
                {
                    m_Button[eKey].bfKey.eKeyState = KEY_OPEN;
                    m_Button[eKey].bfKey.bFullPressCycle = TRUE;
                }
            }
            break;

        case KEY_STUCK:
            if (!m_Button[eKey].bfKey.bPressed)
            {
                m_Button[eKey].bfKey.eKeyState = KEY_LOOSENED;
                m_Button[eKey].bfKey.nDebounced = 0;
            }
            break;

        case KEY_LOOSENED:
            if (m_Button[eKey].bfKey.bPressed)
            {
                m_Button[eKey].bfKey.eKeyState = KEY_STUCK;
            }
            else
            {
                if (++m_Button[eKey].bfKey.nDebounced >= DEBOUNCE_COUNT)
                {
                    m_Button[eKey].bfKey.eKeyState = KEY_OPEN;
                }
            }
            break;

        default:
            m_Button[eKey].bfKey.eKeyState = KEY_OPEN;
            break;
    }

#undef DEBOUNCE_COUNT
}// End runKeysStateMachine()