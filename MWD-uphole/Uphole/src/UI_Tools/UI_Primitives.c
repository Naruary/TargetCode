/*******************************************************************************
*       @brief      This module provides Low Level drawing functionallity to
*                   the LCD.
*       @file       Uphole/src/UI_Tools/UI_Primitives.c
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <stm32f4xx.h>
#include <stdbool.h>
#include <string.h>
#include "portable.h"
#include "lcd.h"
#include "UI_DataStructures.h"
#include "UI_Primitives.h"

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

extern U_BYTE m_nPixelData[MAX_PIXEL_ROW][MAX_PIXEL_COL_STORAGE];
static U_BYTE nLineBuffer[60][40];
static U_BYTE nLineIndex;

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

void InitLineBuffer(void)
{
	nLineIndex = 0;
	memset((void *)&nLineBuffer[0][0], 0, sizeof(nLineBuffer));
}

void AddLineDataToBuffer(U_BYTE* pData, U_BYTE nLength)
{
	memcpy((void *)&nLineBuffer[nLineIndex++][0], (const void *)pData, nLength);
}

/*******************************************************************************
*       @details
*******************************************************************************/
void UI_DrawLine(U_BYTE *pMemoryBase, U_INT32 nTop, U_INT32 nLeft, U_INT32 nLength, BOOL bDirectionDown)
{
	U_INT32 *pBBMemoryBaseAddress;
	INT32 nLeftOffset = 0;
	U_INT32 nLengthCount = 0;
	U_INT32 nBitCount = 0;

	pBBMemoryBaseAddress = (U_INT32 *)((((U_INT32)pMemoryBase - SRAM1_BASE) * 32) + SRAM1_BB_BASE);
	pBBMemoryBaseAddress += (nTop * 320);
	pBBMemoryBaseAddress += (nLeft / 8) * 8;
	nBitCount = nLeft % 8;
	switch(nBitCount)
	{
		case 0: nLeftOffset = 7; break;
		case 1: nLeftOffset = 6; break;
		case 2: nLeftOffset = 5; break;
		case 3: nLeftOffset = 4; break;
		case 4: nLeftOffset = 3; break;
		case 5: nLeftOffset = 2; break;
		case 6: nLeftOffset = 1; break;
		case 7: nLeftOffset = 0; break;
		default: break;
	}
	pBBMemoryBaseAddress += nLeftOffset;
	if(bDirectionDown)
	{
		if(nLength > (240 - nTop))
		{
			nLength = 240 - nTop;
		}
		while(nLengthCount < nLength)
		{
			*pBBMemoryBaseAddress = 1;
			nLengthCount++;
			pBBMemoryBaseAddress += 320;
		}
	}
	else
	{
		while(nLengthCount < nLength)
		{
			while((nLengthCount < nLength) && (nLeftOffset > -1))
			{
				*pBBMemoryBaseAddress-- = 1;
				nLeftOffset--;
				nLengthCount++;
			}
			pBBMemoryBaseAddress += 16;
			nLeftOffset = 7;
		}
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
void UI_DrawRectangle(RECT rctDisplay, BOOL bPage)
{
	U_BYTE *pPixelPage;

	pPixelPage = (bPage)?GetLcdBackgroundPage():GetLcdForegroundPage();
	UI_DrawLine(pPixelPage, rctDisplay.ptTopLeft.nRow, rctDisplay.ptTopLeft.nCol, (rctDisplay.ptBottomRight.nCol - rctDisplay.ptTopLeft.nCol) + 1, false);
	UI_DrawLine(pPixelPage, rctDisplay.ptBottomRight.nRow, rctDisplay.ptTopLeft.nCol, (rctDisplay.ptBottomRight.nCol - rctDisplay.ptTopLeft.nCol) + 1, false);
	UI_DrawLine(pPixelPage, rctDisplay.ptTopLeft.nRow, rctDisplay.ptTopLeft.nCol, (rctDisplay.ptBottomRight.nRow - rctDisplay.ptTopLeft.nRow) + 1, true);
	UI_DrawLine(pPixelPage, rctDisplay.ptTopLeft.nRow, rctDisplay.ptBottomRight.nCol, (rctDisplay.ptBottomRight.nRow - rctDisplay.ptTopLeft.nRow) + 1, true);
}

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Function:
;   UI_RenderBitmap()
;
; Description:
;   Draws a RunLengthEncoded (RLE) bitmap onto the screen. The bitmap should
;   already be defined in the Bitmap structure and the x and y coordinates
;   are the top left hand corner of the bitmap.
;
;   Currently two formats are supported: RLE and RLE2. Each of these are
;   described in detail in the code where they are decompressed and rendered.
;
; Parameters:
;   phDC       =>  Handle to the display context.
;   nYOrigin   =>  Starting Y coordinate.
;   nXOrigin   =>  Starting X coordinate.
;   pbpBitmap  =>  Pointer to a Bitmap structure containing the data for the bitmap.
;   pnRc       =>  A dual purpose parameter. It reports a result of the operation,
;                 where -1 means nothing was drawn and 0 means success.
;
; Reentrancy:
;   No.
;
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void UI_RenderBitmap(U_INT16 nYOrigin, U_INT16 nXOrigin, const UIBITMAP *pbpBitmap, BYTE *pnRc)
{
	U_BYTE      *pPixelByteData;
	U_BYTE      nBytePosn = nXOrigin / BITS_IN_BYTE;
	U_BYTE      nBitsRemainder = nXOrigin % BITS_IN_BYTE;
	U_BYTE      nBitsLeftShift = BITS_IN_BYTE - nBitsRemainder;
	U_INT16     nTargetByte;
	U_BYTE      nIndex = 0;
	U_BYTE      nCharHieght = 0;
	U_BYTE      nCharWidth = 0;

	// KickWatchdog();
	if(pnRc == NULL)
	{
		//ErrorState(ERR_NULL_PTR);
		return; //shuts up Lint
	}
	// Validate our other pointers are not null
	if((pbpBitmap == NULL))
	{
		// calling function can check pnRc for -1 if necessary for null pointer
		*pnRc = -1;
		return;
	}
	pPixelByteData = (U_BYTE *)pbpBitmap->pData;
	if(nXOrigin == 0)
	{
		memset((void *)&nLineBuffer[0][0], 0x00, sizeof(nLineBuffer));
	}
	nCharHieght = ((pbpBitmap->nKern & 0xF0) >> 0x04);
	nCharWidth = (pbpBitmap->nKern & 0x0F);
	if(nCharWidth > 8)
	{
		while(nIndex < nCharHieght)
		{
			nLineBuffer[nIndex][nBytePosn] = pPixelByteData[(nIndex * 2)];
			nLineBuffer[nIndex][(nBytePosn + 1)] = pPixelByteData[((nIndex * 2)+1)];
			nIndex++;
		}
	}
	else
	{
		while(nIndex < nCharHieght)
		{
			nTargetByte = (U_INT16)pPixelByteData[nIndex];
			nTargetByte <<= nBitsLeftShift;
			nLineBuffer[nIndex][nBytePosn] |= (U_BYTE)(nTargetByte >> BITS_IN_BYTE);
			nLineBuffer[nIndex++][nBytePosn + 1] |= (U_BYTE)(nTargetByte);
		}
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
void UI_PrintLine(U_INT16 nYOrigin, U_INT16 nXOrigin, U_INT16 nHieght, U_INT16 nLength)
{
	U_INT16 nWorkingBytes;
	U_BYTE nScanRow = 0;
	U_BYTE nBytesLength;
	U_BYTE nBitsLengthRemainder;
	U_BYTE nBytesOrigin;
	U_BYTE nBitsOriginRemainder;
	U_BYTE nCalc;
	U_BYTE nWorkingLength;
	U_BYTE nWorkingPosition;
	U_BYTE nLineIndex;

	nBytesLength = nLength / BITS_IN_BYTE;
	nBitsLengthRemainder = nLength % BITS_IN_BYTE;
	nBytesOrigin = nXOrigin / BITS_IN_BYTE;
	nBitsOriginRemainder = nXOrigin % BITS_IN_BYTE;
	nCalc = (U_BYTE)(((nLength + nBitsOriginRemainder) % BITS_IN_BYTE) != false);
	if(nBitsLengthRemainder != 0)
	{
		nBytesLength++;
	}
	while(nHieght--)
	{
		nWorkingLength = nBytesLength;
		nWorkingPosition = nBytesOrigin;
		nLineIndex = 0;
		while(nWorkingLength != 0)
		{
			nWorkingBytes = ((nLineBuffer[nScanRow][nWorkingLength-1] << 8) | (nLineBuffer[nScanRow][nWorkingLength]));
			nWorkingBytes >>= nBitsOriginRemainder;
			nLineBuffer[nScanRow][nWorkingLength] = (U_BYTE)nWorkingBytes;
			nWorkingLength--;
		}
		nLineBuffer[nScanRow][0] = (U_BYTE)(nWorkingBytes >> 8);
		nWorkingLength = nBytesOrigin + nBytesLength + nCalc;
		nWorkingPosition = nBytesOrigin;
		nLineIndex = 0;
		while(nWorkingPosition < nWorkingLength)
		{
			m_nPixelData[nYOrigin][nWorkingPosition++] |= nLineBuffer[nScanRow][nLineIndex++];
		}
		nScanRow++;
		nYOrigin++;
	}
}
