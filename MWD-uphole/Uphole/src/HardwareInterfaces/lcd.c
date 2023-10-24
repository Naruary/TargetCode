/*******************************************************************************
*       @brief      This module provides functions to handle the use of the lcd.
*       @file       Uphole/src/HardwareInterfaces/lcd.c
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
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "portable.h"
#include "lcd.h"
#include "Manager_DataLink.h"
#include "systick.h"
#include "timer.h"
#include "FlashMemory.h"
#include "UI_Frame.h"
#include "UI_LCDScreenInversion.h"
#include "Compass_Plot.h"
#include "Compass_Panel.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

#define LCD_RESET_PORT      GPIOE
#define LCD_RESET_PIN       GPIO_Pin_11
#define LCD_BACKLIGHT_PORT  GPIOB
#define LCD_BACKLIGHT_PIN   GPIO_Pin_5
#define LCD_DATA_BUS_PINS_PORT_D   (GPIO_Pin_14 | GPIO_Pin_15 | GPIO_Pin_0| GPIO_Pin_1)
#define LCD_DATA_BUS_PINS_PORT_E   (GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10)


#define SED1335_SAD1L       0
#define SED1335_SAD1H       0
#define SED1335_SAD2L       0xB0
#define SED1335_SAD2H       0x04
#define SED1335_GRAPHICSTART    ((SED1335_SAD1H << 8) + SED1335_SAD1L)
#define SED1335_MWRITE       0x42
#define SED1335_MREAD        0x43
#define SED1335_FX           7

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

static void lcd_WritePixelDataBuffer(void);
static void lcd_WritePixelBackgroundBuffer(void);
static void lcd_Write(U_BYTE nCmd, U_BYTE *pData, U_INT32 nLength);

void GLCD_SetPixel(unsigned int x,unsigned int y);
void GLCD_SetCursorAddress(int addr);
U_BYTE TakeData(void);
U_BYTE GLCD_ReadData(void);

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

#pragma diag_suppress=Pe550
#pragma section="LCD_DATA_PORT"
__no_init static U_BYTE m_nLcdDataPort @ "LCD_DATA_PORT";

#pragma section="LCD_COMMAND_PORT"
__no_init static U_BYTE m_nLcdCommandPort @ "LCD_COMMAND_PORT";

U_BYTE m_nPixelData[MAX_PIXEL_ROW][MAX_PIXEL_COL_STORAGE];
U_BYTE m_nPixelBackground[MAX_PIXEL_ROW][MAX_PIXEL_COL_STORAGE];

// variable to keep track of the elapsed time for LCD backlight (LED) dimming
static TIME_LR m_tLcdBacklightTimer;
static BOOL m_bPaintLcdBackground;
static BOOL m_bPaintLcdForeground;
static BOOL LcdOnOffFlag = true;
static BOOL LCDRefreshSwitch = true;
//static BOOL Horizontal_Line = false;

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   LCD_InitPins()
;
; Description:
;   Sets up LCD driver pin as an output and turns off the LED
;
; Reentrancy:
;   No
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void LCD_InitPins(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	static FSMC_NORSRAMInitTypeDef FSMC_NORSRAMInitStructure;

	LCDRefreshSwitch = true;
	GPIO_StructInit(&GPIO_InitStructure);
	// GPIO LED status Pins
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
        
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
        GPIO_InitStructure.GPIO_Pin = LCD_POWER_PIN;
        GPIO_Init(LCD_POWER_PORT, &GPIO_InitStructure);
        GPIO_ResetBits(LCD_POWER_PORT, LCD_POWER_PIN); // Set LCD_POWER_PIN low to ground it
    
	GPIO_InitStructure.GPIO_Pin = LCD_RESET_PIN;
	GPIO_Init(LCD_RESET_PORT, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = LCD_BACKLIGHT_PIN;
	GPIO_Init(LCD_BACKLIGHT_PORT, &GPIO_InitStructure);
	//GPIO_InitStructure.GPIO_Pin = LCD_POWER_PIN;
	GPIO_Init(LCD_POWER_PORT, &GPIO_InitStructure);
	//GPIO_WriteBit(LCD_POWER_PORT, LCD_POWER_PIN, Bit_SET);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;      //This is the Enable Pin
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Pin = LCD_DATA_BUS_PINS_PORT_D;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = LCD_DATA_BUS_PINS_PORT_E;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = (GPIO_Pin_11 | GPIO_Pin_5 | GPIO_Pin_7);
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOD, (GPIO_PinSource14), GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, (GPIO_PinSource15), GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, (GPIO_PinSource0), GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, (GPIO_PinSource1), GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, (GPIO_PinSource7), GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, (GPIO_PinSource8), GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, (GPIO_PinSource9), GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, (GPIO_PinSource10), GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource11, GPIO_AF_FSMC); //A16
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource5, GPIO_AF_FSMC);  //W/~R
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource7, GPIO_AF_FSMC);  //~CS
	FSMC_NORSRAMStructInit(&FSMC_NORSRAMInitStructure);
	FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM1;
	FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
	FSMC_NORSRAMInitStructure.FSMC_MemoryType = FSMC_MemoryType_SRAM;
	FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_8b;
	FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
	FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Enable;
	FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct->FSMC_AccessMode = FSMC_AccessMode_A;
	FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct->FSMC_AddressHoldTime = 4;
	FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct->FSMC_AddressSetupTime = 4;
	FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct->FSMC_BusTurnAroundDuration = 4;
	FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct->FSMC_CLKDivision = 4;
	FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct->FSMC_DataLatency = 4;
	FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct->FSMC_DataSetupTime = 16;
	FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct->FSMC_AccessMode = FSMC_AccessMode_A;
	FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct->FSMC_AddressHoldTime = 4;
	FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct->FSMC_AddressSetupTime = 4;
	FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct->FSMC_BusTurnAroundDuration = 4;
	FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct->FSMC_CLKDivision = 4;
	FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct->FSMC_DataLatency = 4;
	FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct->FSMC_DataSetupTime = 16;
	FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);
	FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM1, ENABLE);
	GPIO_SetBits(LCD_BACKLIGHT_PORT, LCD_BACKLIGHT_PIN);
	GPIO_ResetBits(LCD_RESET_PORT, LCD_RESET_PIN);
	Delay5us();
	GPIO_SetBits(LCD_RESET_PORT, LCD_RESET_PIN);
	GPIO_SetBits(GPIOD, GPIO_Pin_6);
	Delay5us();
}//end LCD_InitPins

/*******************************************************************************
*       @details
*******************************************************************************/
void LCD_SetBacklight(BOOL bOnState)
{
	GPIO_WriteBit(LCD_BACKLIGHT_PORT, LCD_BACKLIGHT_PIN, (bOnState == true ? Bit_SET : Bit_RESET));
}

/*******************************************************************************
*       @details
*******************************************************************************/
void M_Turn_LCD_On_And_Reset_Timer(void)
{
	LCD_SetBacklight(true);
	m_tLcdBacklightTimer = ElapsedTimeLowRes(START_LOW_RES_TIMER);
//	ResetLcdBacklightTimer();
}

/*******************************************************************************
*       @details
*******************************************************************************/
void LCD_Init(void)
{
	m_nLcdCommandPort = 0x40;
	m_nLcdDataPort = 0x34;
	m_nLcdDataPort = 0x87;
	m_nLcdDataPort = 0x07;
	m_nLcdDataPort = 0x27;
	m_nLcdDataPort = 0x39;
	m_nLcdDataPort = 0xEF;
	m_nLcdDataPort = 0x28;
	m_nLcdDataPort = 0x00;

	m_nLcdCommandPort = 0x44;

	m_nLcdDataPort = 0x00;
	m_nLcdDataPort = 0x00;
	m_nLcdDataPort = 0xEF;
	m_nLcdDataPort = 0x80;
	m_nLcdDataPort = 0x25;
	m_nLcdDataPort = 0xEF;
	m_nLcdDataPort = 0x00;
	m_nLcdDataPort = 0x00;
	m_nLcdDataPort = 0x00;
	m_nLcdDataPort = 0x00;

	m_nLcdCommandPort = 0x5A;
	m_nLcdDataPort = 0x00;

	m_nLcdCommandPort = 0x5B;
	m_nLcdDataPort = 0x0C;

	m_nLcdCommandPort = 0x5D;
	m_nLcdDataPort = 0x04;
	m_nLcdDataPort = 0x86;

	m_nLcdCommandPort = 0x59;
	m_nLcdDataPort = 0x14;

	memset((void *) m_nPixelData, 0x00, sizeof(m_nPixelData));
	memset((void *) m_nPixelBackground, 0x00, sizeof(m_nPixelBackground));

	lcd_WritePixelDataBuffer();
	lcd_WritePixelBackgroundBuffer();
}

/*******************************************************************************
*       @details
*******************************************************************************/
void LCD_Update(void)
{
	if(m_bPaintLcdBackground)
	{
		lcd_WritePixelBackgroundBuffer();
		m_bPaintLcdBackground = false;
	}
	if(m_bPaintLcdForeground)
	{
		lcd_WritePixelDataBuffer();
		m_bPaintLcdForeground = false;
	}
//	if (ElapsedTimeLowRes(m_tLcdBacklightTimer) >= (FOURTYFIVE_SECOND))
//	{
//		LCD_SetBacklight(OFF);
//	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
void LCD_ClearRow(U_INT16 nRowPosn, U_INT16 nColLoLimit, U_INT16 nColHiLimit, BOOL bPage)
{
	U_BYTE nLoBytePosn;
	U_BYTE nLoBitPosn;
	U_BYTE nLoBitMask;
	U_BYTE nHiBytePosn;
	U_BYTE nHiBitPosn;
	U_BYTE nHiBitMask;
	U_INT32 nIndex;
	U_BYTE *nWorkingRow;

	if(bPage)
	{
		nWorkingRow = &m_nPixelBackground[nRowPosn][0];
	}
	else
	{
		nWorkingRow = &m_nPixelData[nRowPosn][0];
	}

	nLoBytePosn = (U_BYTE)(nColLoLimit / BITS_IN_BYTE);
	nLoBitPosn = (U_BYTE)(BITS_IN_BYTE - (nColLoLimit % BITS_IN_BYTE));
	switch(nLoBitPosn)
	{
		case 0: nLoBitMask = 0xFF; break;
		case 1: nLoBitMask = 0xFE; break;
		case 2: nLoBitMask = 0xFC; break;
		case 3: nLoBitMask = 0xF8; break;
		case 4: nLoBitMask = 0xF0; break;
		case 5: nLoBitMask = 0xE0; break;
		case 6: nLoBitMask = 0xC0; break;
		case 7: nLoBitMask = 0x80; break;
		default:
			break;
	}

	nHiBytePosn = (U_BYTE)(nColHiLimit / BITS_IN_BYTE);
	nHiBitPosn = (U_BYTE)(nColHiLimit % BITS_IN_BYTE);
	switch(nHiBitPosn)
	{
		case 0: nHiBitMask = 0xFF; break;
		case 1: nHiBitMask = 0x7F; break;
		case 2: nHiBitMask = 0x3F; break;
		case 3: nHiBitMask = 0x1F; break;
		case 4: nHiBitMask = 0x0F; break;
		case 5: nHiBitMask = 0x07; break;
		case 6: nHiBitMask = 0x03; break;
		case 7: nHiBitMask = 0x01; break;
		default:
			break;
	}

	nIndex = nLoBytePosn;
	while(nIndex <= nHiBytePosn)
	{
		if(nIndex == nLoBytePosn)
		{
			nWorkingRow[nIndex] = nWorkingRow[nIndex] & nLoBitMask;
		}
		else if(nIndex == nHiBytePosn)
		{
			nWorkingRow[nIndex] = nWorkingRow[nIndex] & nHiBitMask;
		}
		else
		{
			nWorkingRow[nIndex] = 0x00;
		}
		nIndex++;
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
void LCD_InvertRow(U_INT16 nRowPosn, U_INT16 nColLoLimit, U_INT16 nColHiLimit, BOOL bPage)
{
	U_BYTE nLoBytePosn;
	U_BYTE nLoBitMask;
	U_BYTE nHiBytePosn;
	U_BYTE nHiBitMask;
	U_INT32 nIndex;
	U_BYTE *nWorkingRow;
	U_BYTE nWorkingByte;

	if(bPage)
	{
		nWorkingRow = &m_nPixelBackground[nRowPosn][0];
	}
	else
	{
		nWorkingRow = &m_nPixelData[nRowPosn][0];
	}

	nLoBytePosn = (U_BYTE) (nColLoLimit / BITS_IN_BYTE);
	switch(nColLoLimit % BITS_IN_BYTE)
	{
		case 0: nLoBitMask = 0xFF; break;
		case 1: nLoBitMask = 0x7F; break;
		case 2: nLoBitMask = 0x3F; break;
		case 3: nLoBitMask = 0x1F; break;
		case 4: nLoBitMask = 0x0F; break;
		case 5: nLoBitMask = 0x07; break;
		case 6: nLoBitMask = 0x03; break;
		case 7: nLoBitMask = 0x01; break;
		default:
			break;
	}

	nHiBytePosn = (U_BYTE) (nColHiLimit / BITS_IN_BYTE);
	switch(nColHiLimit % BITS_IN_BYTE)
	{
		case 0: nHiBitMask = 0x00; break;
		case 1: nHiBitMask = 0x80; break;
		case 2: nHiBitMask = 0xC0; break;
		case 3: nHiBitMask = 0xE0; break;
		case 4: nHiBitMask = 0xF0; break;
		case 5: nHiBitMask = 0xF8; break;
		case 6: nHiBitMask = 0xFC; break;
		case 7: nHiBitMask = 0xFE; break;
		default:
			break;
	}

	for(nIndex = nLoBytePosn; nIndex <= nHiBytePosn; nIndex++)
	{
		if(nIndex == nLoBytePosn)
		{
			nWorkingByte = ((~nWorkingRow[nIndex]) & nLoBitMask);
			nWorkingRow[nIndex] = ((nWorkingRow[nIndex] & ~nLoBitMask) | nWorkingByte);
		}
		else if(nIndex == nHiBytePosn)
		{
			nWorkingByte = ((~nWorkingRow[nIndex]) & nHiBitMask);
			nWorkingRow[nIndex] = ((nWorkingRow[nIndex] & ~nHiBitMask) | nWorkingByte);
		}
		else
		{
			nWorkingRow[nIndex] = ~nWorkingRow[nIndex];
		}
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
void LCD_ClearPixel(U_INT16 nColPosn, U_BYTE nRowPosn, BOOL bPage)
{
	U_BYTE nTestByte;
	U_BYTE nBytePosn;
	U_BYTE nBitPosn;

	nBytePosn = (U_BYTE)(nColPosn / 8);
	nBitPosn = (U_BYTE)(nColPosn % 8);

	switch(nBitPosn)
	{
		case 0: nBitPosn = 0x80; break;
		case 1: nBitPosn = 0x40; break;
		case 2: nBitPosn = 0x20; break;
		case 3: nBitPosn = 0x10; break;
		case 4: nBitPosn = 0x08; break;
		case 5: nBitPosn = 0x04; break;
		case 6: nBitPosn = 0x02; break;
		case 7: nBitPosn = 0x01; break;
		default:
			break;
	}

	if(bPage)
	{
		nTestByte = m_nPixelBackground[nRowPosn][nBytePosn];
	}
	else
	{
		nTestByte = m_nPixelData[nRowPosn][nBytePosn];
	}

	nTestByte &= ~nBitPosn;
	if(bPage)
	{
		m_nPixelBackground[nRowPosn][nBytePosn] = nTestByte;
	}
	else
	{
		m_nPixelData[nRowPosn][nBytePosn] = nTestByte;
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void lcd_WritePixelDataBuffer(void)
{
//	U_INT32 nOldPSW;
	U_BYTE nCmdData[] = {0x00, 0x00};

//	nOldPSW = ReadPriorityStatusAndChange(BOOST_THREAD_PRIORITY);

	lcd_Write(0x46, nCmdData, sizeof(nCmdData));
	lcd_Write(0x42, &m_nPixelData[0][0], sizeof(m_nPixelData));

//	RestorePriorityStatus(nOldPSW);
}//end lcd_WritePixelDataBuffer

/*******************************************************************************
*       @details
*******************************************************************************/
static void lcd_WritePixelBackgroundBuffer(void)
{
//	U_INT32 nOldPSW;
	U_BYTE nCmdData[] = {0x80, 0x25};

//	nOldPSW = ReadPriorityStatusAndChange(BOOST_THREAD_PRIORITY);

	lcd_Write(0x46, nCmdData, sizeof(nCmdData));
	lcd_Write(0x42, &m_nPixelBackground[0][0], sizeof(m_nPixelBackground));

//	RestorePriorityStatus(nOldPSW);
}//end lcd_WritePixelBackgroundBuffer

/*******************************************************************************
*       @details
*******************************************************************************/
static void lcd_Write(U_BYTE nCmd, U_BYTE *pData, U_INT32 nLength)
{
	U_INT32 nIndex = 0;

	m_nLcdCommandPort = nCmd;
	while(nIndex < nLength)
	{
		m_nLcdDataPort = pData[nIndex++];
	}
}//end lcd_Write

/*******************************************************************************
*       @details
*******************************************************************************/
U_BYTE* GetLcdBackgroundPage(void)
{
	return &m_nPixelBackground[0][0];
}

/*******************************************************************************
*       @details
*******************************************************************************/
U_BYTE* GetLcdForegroundPage(void)
{
	return &m_nPixelData[0][0];
}

/*******************************************************************************
*       @details
*******************************************************************************/
void LCD_Refresh(BOOL bPage)
{
	if(LCDRefreshSwitch == true)
	{
		if(bPage)
		{
			m_bPaintLcdBackground = true;
		}
		else
		{
			m_bPaintLcdForeground = true;
		}
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
void LCD_OFF(void)
{

	memset((void *) m_nPixelData, 0x00, sizeof(m_nPixelData));
	memset((void *) m_nPixelBackground, 0x00, sizeof(m_nPixelBackground));
	lcd_WritePixelDataBuffer();
	lcd_WritePixelBackgroundBuffer();

	LcdOnOffFlag = false;

	LCD_Refresh(LCD_FOREGROUND_PAGE);
	LCD_Refresh(LCD_BACKGROUND_PAGE);

	LCD_Update();

	// Make all the LCD Databus pins high impedance
	// This is done making the output open drain and setting the bit high
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_StructInit(&GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

	GPIO_InitStructure.GPIO_Pin = LCD_DATA_BUS_PINS_PORT_D;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = LCD_DATA_BUS_PINS_PORT_E;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = (GPIO_Pin_11 | GPIO_Pin_5 | GPIO_Pin_7);
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = LCD_RESET_PIN;
	GPIO_Init(LCD_RESET_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = LCD_BACKLIGHT_PIN;
	GPIO_Init(LCD_BACKLIGHT_PORT, &GPIO_InitStructure);

	GPIO_WriteBit(GPIOD, LCD_DATA_BUS_PINS_PORT_D, Bit_SET);
	GPIO_WriteBit(GPIOE, LCD_DATA_BUS_PINS_PORT_E, Bit_SET);

	GPIO_WriteBit(GPIOD, GPIO_Pin_11, Bit_SET);
	GPIO_WriteBit(GPIOD, GPIO_Pin_5, Bit_SET);
	GPIO_WriteBit(GPIOD, GPIO_Pin_6, Bit_SET);
	GPIO_WriteBit(GPIOD, GPIO_Pin_7, Bit_SET);

	GPIO_WriteBit(LCD_BACKLIGHT_PORT, LCD_BACKLIGHT_PIN, Bit_SET);
	GPIO_WriteBit(LCD_RESET_PORT, LCD_RESET_PIN, Bit_SET);

	LCDRefreshSwitch = false;

	//GPIO_WriteBit(LCD_POWER_PORT, LCD_POWER_PIN, Bit_RESET);
}

/*******************************************************************************
*       @details
*******************************************************************************/
void LCD_ON(void)
{
	LCDRefreshSwitch = true;
	m_nLcdCommandPort = 0x59;
	m_nLcdDataPort = 0x00;

	//Turn the LCD ON with init pins and LCD init
	LCD_InitPins();
	LCD_SetBacklight(true);
	LCD_Init();
	LcdOnOffFlag = true;
	if(getCompassDecisionPanelActive() == true)
	{
		DrawCompass();  
	}
	else
	{
		PaintNow(&HomeFrame);
		PaintNow(&WindowFrame);
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
BOOL LCDStatus(void)
{
	return LcdOnOffFlag;
}

/*******************************************************************************
*       @details
*******************************************************************************/
void GLCD_Line(int X1, int Y1,int X2,int Y2)
{
	int CurrentX, CurrentY, Xinc, Yinc, Dx, Dy, TwoDx, TwoDy, TwoDxAccumulatedError, TwoDyAccumulatedError;

	Dx = (X2-X1);
	Dy = (Y2-Y1);

	TwoDx = Dx + Dx;
	TwoDy = Dy + Dy;

	CurrentX = X1;
	CurrentY = Y1;

	Xinc = 1;
	Yinc = 1;

	if(Dx < 0)
	{
		Xinc = -1;
		Dx = -Dx;
		TwoDx = -TwoDx;
	}

	if (Dy < 0)
	{
		Yinc = -1;
		Dy = -Dy;
		TwoDy = -TwoDy;
	}

	GLCD_SetPixel(X1,Y1);

	if ((Dx != 0) || (Dy != 0))
	{
		if (Dy <= Dx)
		{
			TwoDxAccumulatedError = 0;
			do
			{
				CurrentX += Xinc;
				TwoDxAccumulatedError += TwoDy;
				if(TwoDxAccumulatedError > Dx)
				{
					CurrentY += Yinc;
					TwoDxAccumulatedError -= TwoDx;
				}
				GLCD_SetPixel(CurrentX,CurrentY);
			} while (CurrentX != X2);
		}
		else
		{
			TwoDyAccumulatedError = 0;
			do
			{
				CurrentY += Yinc;
				TwoDyAccumulatedError += TwoDx;
				if(TwoDyAccumulatedError > Dy)
				{
					CurrentX += Xinc;
					TwoDyAccumulatedError -= TwoDy;
				}
				GLCD_SetPixel(CurrentX,CurrentY);
			} while (CurrentY != Y2);
		}
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
void GLCD_SetPixel(unsigned int x,unsigned int y)
{
//	unsigned char tmp = 0;
	unsigned int address = SED1335_GRAPHICSTART + (40 * y) + (x/8);
	U_BYTE * DataAddress = &m_nPixelData[0][0];

//	GLCD_SetCursorAddress(address);
//	m_nLcdCommandPort = SED1335_MREAD;
//	tmp = GLCD_ReadData();
//	tmp &= (1 << (SED1335_FX - (x % 8)));
	DataAddress[address] |= (1 << (7 - (x % 8)));

//	GLCD_SetCursorAddress(address);
//	m_nLcdCommandPort = SED1335_MWRITE;
//	m_nLcdDataPort = tmp;
//	DataAddress[address] = tmp;
}

/*******************************************************************************
*       @details
*******************************************************************************/
//void GLCD_SetCursorAddress(int addr)
//{
//   U_BYTE adress;
//   m_nLcdCommandPort = 0x46;
//   adress = addr & 0xFF;
//   m_nLcdDataPort = adress;
//   adress = addr >> 8;
//   m_nLcdDataPort = adress;
//}

/*******************************************************************************
*       @details
*******************************************************************************/
//U_BYTE GLCD_ReadData(void)
//{
//  U_BYTE tmp;
//
//  GPIO_ResetBits(GPIOD, GPIO_PinSource5);
//  GPIO_ResetBits(GPIOD, GPIO_PinSource7);
//  Delay5us();
//  tmp = TakeData();
//  GPIO_SetBits(GPIOD, GPIO_PinSource5);
//  GPIO_SetBits(GPIOD, GPIO_PinSource7);
//
//  return tmp;
//}

/*******************************************************************************
*       @details
*******************************************************************************/
//U_BYTE TakeData(void)
//   {
//   U_BYTE data = 0;
//   data += GPIO_ReadInputDataBit(GPIOD,  GPIO_Pin_14);
//   data += GPIO_ReadInputDataBit(GPIOD,  GPIO_Pin_15) * 2;
//   data += GPIO_ReadInputDataBit(GPIOD,  GPIO_Pin_0) * 4;
//   data += GPIO_ReadInputDataBit(GPIOD,  GPIO_Pin_1) * 8;
//   data += GPIO_ReadInputDataBit(GPIOE,  GPIO_Pin_7) * 16;
//   data += GPIO_ReadInputDataBit(GPIOE,  GPIO_Pin_8) * 32;
//   data += GPIO_ReadInputDataBit(GPIOE,  GPIO_Pin_9) * 64;
//   data += GPIO_ReadInputDataBit(GPIOE,  GPIO_Pin_10) * 128;
//   return data;
//   }

/*******************************************************************************
*       @details
*******************************************************************************/
void GLCD_Circle(U_INT16 cx, U_INT16 cy ,U_INT16 radius)
{
	INT16 x, y, xchange, ychange, radiusError;
	x = radius;
	y = 0;
	xchange = 1 - 2 * radius;
	ychange = 1;
	radiusError = 0;
	while(x >= y)
	{
		GLCD_SetPixel(cx+x, cy+y);
		GLCD_SetPixel(cx-x, cy+y);
		GLCD_SetPixel(cx-x, cy-y);
		GLCD_SetPixel(cx+x, cy-y);
		GLCD_SetPixel(cx+y, cy+x);
		GLCD_SetPixel(cx-y, cy+x);
		GLCD_SetPixel(cx-y, cy-x);
		GLCD_SetPixel(cx+y, cy-x);
		y++;
		radiusError += ychange;
		ychange += 2;
		if ( 2*radiusError + xchange > 0 )
		{
			x--;
			radiusError += xchange;
			xchange += 2;
		}
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
void GLCD_SemiCircle(U_INT16 cx, U_INT16 cy ,U_INT16 radius)
{
	INT16 x, y, xchange, ychange, radiusError;
	x = radius;
	y = 0;
	xchange = 1 - 2 * radius;
	ychange = 1;
	radiusError = 0;
	while(x >= y)
	{
		GLCD_SetPixel(cx+x, cy+y);
		//GLCD_SetPixel(cx-x, cy+y);
		//GLCD_SetPixel(cx-x, cy-y);
		GLCD_SetPixel(cx+x, cy-y);
		GLCD_SetPixel(cx+y, cy+x);
		//GLCD_SetPixel(cx-y, cy+x);
		//GLCD_SetPixel(cx-y, cy-x);
		GLCD_SetPixel(cx+y, cy-x);
		y++;
		radiusError += ychange;
		ychange += 2;
		if ( 2*radiusError + xchange > 0 )
		{
			x--;
			radiusError += xchange;
			xchange += 2;
		}
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
void clearLCD(void)
{
	U_BYTE * DataAddress = &m_nPixelData[0][0];
	U_BYTE * BackgdAddress = &m_nPixelBackground[0][0];

	memset(DataAddress, 0, sizeof(U_BYTE)*MAX_PIXEL_ROW*MAX_PIXEL_COL_STORAGE);
	memset(BackgdAddress, 0, sizeof(U_BYTE)*MAX_PIXEL_ROW*MAX_PIXEL_COL_STORAGE);
}

