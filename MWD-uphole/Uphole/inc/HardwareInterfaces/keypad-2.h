/*!
********************************************************************************
*       @brief      Contains header information for the keypad module.
*                   Documented by Josh Masters Dec. 2014
*       @file       Uphole/inc/HardwareInterfaces/keypad.h
*       @author     Bill Kamienik
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef KEY_PAD_H
#define KEY_PAD_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

typedef enum __BUTTON_TYPE__
{
    NO_BUTTON,
    KEYPAD,
    SURVEY,
    SHIFT,
    BOTH_SHIFT_SURVEY,
    BOTH_SHIFT_KEYPAD,
    MAX_BUTTON_TYPE
}BUTTON_TYPE;

// All depress types are defined here
typedef enum __DEPRESS_TYPE__
{
    NO_DEPRESS,
    SHORT_DEPRESS,
    LONG_DEPRESS,
    MAX_DEPRESS_TYPE
}BUTTON_DEPRESS_TYPE;

typedef enum
{
    KEYPAD_KEY,
    SURVEY_KEY,
    SHIFT_KEY,
    EXTRA_1_KEY,
    EXTRA_2_KEY,
    MAX_KEYS
} KEYS;

typedef enum __KEY_STATE_
{
    KEY_OPEN,
    KEY_CLOSING,
    KEY_CLOSED,
    KEY_OPENING,
    KEY_STUCK,
    KEY_LOOSENED
} KEY_STATE;

#define KEY_NONE 0

// A button push is a combination of a particular
// button and a depress type
typedef enum _BUTTON_VALUE
{
    BUTTON_LEFT = 'a',
    BUTTON_RIGHT = 'd',
    BUTTON_DOWN = 'x',
    BUTTON_UP = 'w',
    BUTTON_SELECT = 's',
    BUTTON_ZERO = '0',
    BUTTON_ONE = '1',
    BUTTON_TWO = '2',
    BUTTON_THREE = '3',
    BUTTON_FOUR = '4',
    BUTTON_FIVE = '5',
    BUTTON_SIX = '6',
    BUTTON_SEVEN = '7',
    BUTTON_EIGHT = '8',
    BUTTON_NINE = '9',
    BUTTON_PERIOD = '.',
    BUTTON_DASH = '-',
    BUTTON_SHIFT_LEFT = 'A',
    BUTTON_SHIFT_RIGHT = 'D',
    BUTTON_SHIFT_DOWN = 'X',
    BUTTON_SHIFT_UP = 'W',
    BUTTON_SHIFT_SELECT = 'S',
    BUTTON_SHIFT_ZERO = ')',
    BUTTON_SHIFT_ONE = '!',
    BUTTON_SHIFT_TWO = '@',
    BUTTON_SHIFT_THREE = '#',
    BUTTON_SHIFT_FOUR = '$',
    BUTTON_SHIFT_FIVE = '%',
    BUTTON_SHIFT_SIX = '^',
    BUTTON_SHIFT_SEVEN = '&',
    BUTTON_SHIFT_EIGHT = '*',
    BUTTON_SHIFT_NINE = '(',
    BUTTON_SHIFT_PERIOD = '>',
    BUTTON_SHIFT_DASH = '<',
    BUTTON_SHIFT = 'j',
    BUTTON_NONE = '\0'
} BUTTON_VALUE;

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

    ///@brief  
    ///@param  
    ///@return 
    void KEYPAD_InitPins(void);

    ///@brief  
    ///@param  
    ///@return 
    void KeyPadManager(void);

    ///@brief  
    ///@param  
    ///@return 
    void Keypad_StartCapture(void);
    
#ifdef __cplusplus
}
#endif
#endif