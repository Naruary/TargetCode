/*******************************************************************************
*       @brief      Contains header information for the keypad module.
*       @file       Uphole/inc/HardwareInterfaces/keypad.h
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
#include "stdint.h"
//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

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
	BUTTON_SHIFT = 'j',
	BUTTON_NONE = '\0'
} BUTTON_VALUE;
#define BUTTON_QUANTITY 18

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

	void KEYPAD_InitPins(void);
	void KeyPadManager(void);
        extern uint8_t KeyDebounceCount; // declaration for external linkage
        // Function declarations
        INT16 GetDebounceTime(void);
        void SetDebounceTime(INT16 value);

#ifdef __cplusplus
}
#endif

#endif // KEY_PAD_H
