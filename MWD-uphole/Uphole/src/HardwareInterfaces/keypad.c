/*******************************************************************************
*       @brief      This module provides functions to handle the use of the
*                   keypad.
*       @file       Uphole/src/HardwareInterfaces/keypad.c
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
#include <stdbool.h>
#include <string.h>
#include "portable.h"
#include "keypad.h"
#include "PeriodicEvents.h"
#include "systick.h"
#include "timer.h"
#include "lcd.h"
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

#define SW_ONE_MASK     0x01
#define SW_TWO_MASK     0x02

#define SW_MASK         0x3000
#define SW_SHIFT        12
#define COL_MASK        0x000F
#define	MAX_KEYPAD_ROWS     4

uint8_t KeyDebounceCount = 15; // x10 ms 15 // x10 ms
// Changing the debounce to a higher number (Around 10) makes keystrokes less sensitive
// This prevents more than 1 keystroke at a time. MB 6/21/2021
//ZD 6/13/2023 I boosted this up from 4 to 15 due to drillers finding double typing when entering numbers in this seemingly made a fix for that issue

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

typedef struct
{
	U_BYTE debounce;
	U_BYTE pressed;
	U_BYTE pressed_last;
} _key_data;

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

static _key_data key_data[BUTTON_QUANTITY];
static const struct {
	GPIO_TypeDef*   GPIOx;
	uint16_t        GPIO_Pin;
	U_BYTE          nShiftCount;
} m_nKeypadRowDriver[4] = {
	{ROW_PORT_12, ROW_ONE, 0},
	{ROW_PORT_12, ROW_TWO, 4},
	{ROW_PORT_34, ROW_THREE , 8},
	{ROW_PORT_34, ROW_FOUR, 12}
};

// convert our key data into a button code
BUTTON_VALUE convert_to_button[BUTTON_QUANTITY] = {
	BUTTON_ONE,
	BUTTON_TWO,
	BUTTON_THREE,
	BUTTON_UP,
	BUTTON_FOUR,
	BUTTON_FIVE,
	BUTTON_SIX,
	BUTTON_DOWN,
	BUTTON_SEVEN,
	BUTTON_EIGHT,
	BUTTON_NINE,
	BUTTON_RIGHT,
	BUTTON_PERIOD,
	BUTTON_ZERO,
	BUTTON_DASH,
	BUTTON_LEFT,
	BUTTON_SHIFT,
	BUTTON_SELECT
};

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   KEYPAD_InitPins()
;
; Description:
;   Sets up keypad driver pins as inputs or output as required
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void KEYPAD_InitPins(void)
{
	U_BYTE index;

	GPIO_InitTypeDef GPIO_InitStructure;
	// This is a good configuration for input switches
	GPIO_StructInit(&GPIO_InitStructure);
	// This is the Switches
	GPIO_InitStructure.GPIO_Pin = (SW_ONE | SW_TWO);
	GPIO_Init(SW_PORT, &GPIO_InitStructure);
	// This is the Keypad Columns
	GPIO_InitStructure.GPIO_Pin = (COL_ONE | COL_TWO | COL_THREE | COL_FOUR);
	GPIO_Init(COL_PORT, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Pin = (ROW_ONE | ROW_TWO);
	GPIO_Init(ROW_PORT_12, &GPIO_InitStructure);
	GPIO_WriteBit(ROW_PORT_12, (ROW_ONE | ROW_TWO), Bit_SET);
	GPIO_InitStructure.GPIO_Pin = (ROW_THREE | ROW_FOUR);
	GPIO_Init(ROW_PORT_34, &GPIO_InitStructure);
	GPIO_WriteBit(ROW_PORT_34, (ROW_THREE | ROW_FOUR), Bit_SET);
	for(index=0; index<BUTTON_QUANTITY; index++)
	{
		key_data[index].debounce = 0;
		key_data[index].pressed = 0;
		key_data[index].pressed_last = 0;
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   KeyPadManager()
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void KeyPadManager(void)
{
	static U_BYTE index = 0;
	static U_INT32 m_nKeypadRowData = 0ul;
	static U_BYTE m_nKeypadSampleState = 0;
	static U_BYTE  m_nSwitchScanResult  = 0;

	// get here once per mS from the systick
	switch(m_nKeypadSampleState)
	{
		case 0:
			GPIO_ResetBits(m_nKeypadRowDriver[index].GPIOx, m_nKeypadRowDriver[index].GPIO_Pin);
			m_nKeypadRowData |= ((GPIO_ReadInputData(COL_PORT) & COL_MASK) << m_nKeypadRowDriver[index].nShiftCount);
			GPIO_SetBits(m_nKeypadRowDriver[index].GPIOx, m_nKeypadRowDriver[index].GPIO_Pin);
			if((++index) >= MAX_KEYPAD_ROWS)
			{
				m_nKeypadRowData ^= 0x0000FFFFul;
				m_nKeypadSampleState++;
			}
			break;
		case 1:
			m_nSwitchScanResult = ((U_BYTE)(~((GPIO_ReadInputData(SW_PORT) & SW_MASK) >> SW_SHIFT)));
			if(m_nSwitchScanResult & 0x0001)
			{
				m_nKeypadRowData |= 0x10000ul;
			}
			if(m_nSwitchScanResult & 0x0002)
			{
				m_nKeypadRowData |= 0x20000ul;
			}
			m_nKeypadSampleState++;
			break;
		case 2:
			for(index=0; index<BUTTON_QUANTITY; index++)
			{
				if(m_nKeypadRowData & (1ul << index))
				{
					if(key_data[index].debounce < 0xFF)
						key_data[index].debounce++;
				}
				else
				{
					key_data[index].debounce = 0;
				}
				key_data[index].pressed =
					(key_data[index].debounce > KeyDebounceCount) ? 1 : 0;
				if((key_data[index].pressed_last==0) && (key_data[index].pressed==1))
				{
					AddButtonEvent(convert_to_button[index]);
					// on any keypress, if compass is shown, unshow it..
					if(getCompassDecisionPanelActive() == true)
					{
						setCompassDecisionPanelActive(false);
					}
				}
				key_data[index].pressed_last = key_data[index].pressed;
			}
			m_nKeypadSampleState++;
			break;
		case 3:
		case 4:
		case 5:
		case 6:
			if(m_nKeypadSampleState >= 6)
			{
				index = 0;
				m_nKeypadRowData = 0ul;
				m_nKeypadSampleState = 0;
			}
			else
			{
				m_nKeypadSampleState++;
			}
			break;
	}
}
INT16 GetDebounceTime(void)
{
    return KeyDebounceCount;
}

void SetDebounceTime(INT16 value)
{
    if(value >= 1 && value <= 20)
    {
        KeyDebounceCount = value;
    }
}
