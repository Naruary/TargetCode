/*******************************************************************************
*       @brief      Header file for UI_Alphabet.c module.
*       @file       Uphole/inc/UI_Tools/UI_Alphabet.h
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef UI_ALPHABET_H
#define UI_ALPHABET_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"
#include "UI_Defs.h"

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

typedef struct
{
	U_INT32         nFlags;          // is first bit on or off
	U_BYTE          nKern;
	const void      *pData;           // data
} UI_ALPHA_BITMAP;

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

	void UI_DisplayStringCentered(const char* string, RECT* area);
	void UI_DisplayStringLeftJustified(const char* string, RECT* area);
	void UI_DisplayStringRightJustified(const char* string, RECT* area);
	U_INT16 UI_GetTextSize(const char* psDisplayStr);
	void UI_DisplayStringInStatusFrame(const char* string, RECT* area, INT16 y);
	void UI_DisplayXScaleGraphFrame(const char* string, RECT* area, INT16 y);
	void UI_DisplayYScaleGraphFrame(const char* string, RECT* area, INT16 y);
	void UI_DisplayGraphTitleFrame(const char* string, RECT* area, INT16 x, INT16 y);
	void UI_DisplayString(const char* string, RECT* area, INT16 x, INT16 y);
	void UI_DisplayRightYScaleGraphFrame(const char* string, RECT* area, INT16 y);

#ifdef __cplusplus
}
#endif
#endif
