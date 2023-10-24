/*******************************************************************************
*       @brief      Header File for UI_Primitives.c
*       @file       Uphole/inc/UI_Tools/UI_Primitives.h
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef UI_PRIMITIVES_H
#define UI_PRIMITIVES_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

	void UI_DrawLine(U_BYTE *pMemoryBase, U_INT32 nTop, U_INT32 nLeft, U_INT32 nLength, BOOL bDirection);
	void UI_DrawRectangle(RECT rctDisplay, BOOL bPage);
	void UI_RenderBitmap(U_INT16 nYOrigin, U_INT16 nXOrigin, const UIBITMAP *pbpBitmap, BYTE *pnRc);
	void UI_PrintLine(U_INT16 nYOrigin, U_INT16 nXOrigin, U_INT16 nHieght, U_INT16 nLength);
	void InitLineBuffer(void);
	void AddLineDataToBuffer(U_BYTE* pData, U_BYTE nLength);

#ifdef __cplusplus
}
#endif
#endif
