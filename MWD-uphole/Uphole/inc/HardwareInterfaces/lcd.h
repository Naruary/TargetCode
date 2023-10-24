/*******************************************************************************
*       @brief      Contains header information for the lcd module.
*       @file       Uphole/inc/HardwareInterfaces/lcd.h
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef LCD_H
#define LCD_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//

#define LCD_FOREGROUND_PAGE 0
#define LCD_BACKGROUND_PAGE 1
#define MAX_TEXT_ROW 60
#define MAX_TEXT_COL 40
#define MAX_PIXEL_ROW 240
#define MAX_PIXEL_COL 320
#define MAX_PIXEL_COL_STORAGE (MAX_PIXEL_COL/8)
#define LCD_POWER_PORT  GPIOB
#define LCD_POWER_PIN   GPIO_Pin_1

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

	void LCD_InitPins(void);
	void LCD_Init(void);
	void LCD_Update(void);
	void LCD_SetBacklight(BOOL bOnState);
	U_BYTE* GetLcdBackgroundPage(void);
	U_BYTE* GetLcdForegroundPage(void);
	void LCD_Refresh(BOOL bPage);
	void LCD_ClearRow(U_INT16 nRowPosn, U_INT16 nColLoLimit, U_INT16 nColHiLimit, BOOL bPage);
	void LCD_InvertRow(U_INT16 nRowPosn, U_INT16 nColLoLimit, U_INT16 nColHiLimit, BOOL bPage);
	void LCD_OFF(void);
	void LCD_ON(void);
	BOOL LCDStatus(void);
	void GLCD_Line(int X1, int Y1,int X2,int Y2);
	void GLCD_Circle(U_INT16 cx, U_INT16 cy ,U_INT16 radius);
	void GLCD_SemiCircle(U_INT16 cx, U_INT16 cy ,U_INT16 radius);
	void clearLCD(void);

#ifdef __cplusplus
}
#endif

#endif // LCD_H
