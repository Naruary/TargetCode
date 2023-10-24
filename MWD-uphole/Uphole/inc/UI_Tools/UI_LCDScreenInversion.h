/*******************************************************************************
*       @brief      Header File for the UI Screen Inversion related functions
*       @file       Uphole/inc/UI_Tools/UI_LCDScreenInversion.h
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef UI_LCD_SCREEN_INVERSION_H
#define UI_LCD_SCREEN_INVERSION_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"
#include "UI_DataStructures.h"

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

	void UI_InvertLCDArea(const RECT *pRect, BOOL bPage);
	void UI_ClearLCDArea(const RECT *pRect, BOOL bPage);

#ifdef __cplusplus
}
#endif
#endif
