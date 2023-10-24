/*******************************************************************************
*       @brief      This module contains code and bitmaps to render all 26
*                   English alphabet characters in both upper and lower case as
*                   well as numbers 0 - 9 and special characters.
*       @file       Uphole/src/UI_Tools/UI_Alphabet.c
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"
#include <string.h>
#include "fontdef.h"
#include "UI_DataStructures.h"
#include "UI_Alphabet.h"
#include "UI_Defs.h"
#include "UI_Primitives.h"

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

static void setBitmapSize(UIBITMAP *bp);
static void UI_DisplayText(const char* psDisplayStr, RECT rctDisplay);

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 ; Function:
 ;   UI_DisplayCharacter()
 ;
 ; Description:
 ;   Draws a single character at the given coordinates.  The column pointer
 ;   will be adjusted to reflect the size of the character rendered.
 ;
 ; Parameters:
 ;   phDC     =>  Handle to the display context.
 ;   sDisplayString      =>  Character to be displayed.
 ;   pRow     =>  The top row of the display to start the rendering.
 ;   pCol     =>  The left column of the display to start the rendering. This
 ;               will be modified on exit to reflect the size of the
 ;               character that has been rendered.
 ;   nFlags  =>  0         : Use the default 8x8 character set.
 ;               F_ALTFONT : Use the secondary (11x11) characters.
 ;
 ; Reentrancy:
 ;   No.
 ;
 ; Note:
 ;   We support two fonts: 8 pixel and 11 pixel. the 11 pixel is selected by
 ;   using the F_ALTFONT flag. The default is the 8 pixel font.
 ;
 ;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void UI_DisplayCharacter(HDC *phDC, CHAR sDisplayChar, U_BYTE *pRow, U_BYTE *pCol, U_BYTE nFlags)
{
    UIBITMAP bp;
    UIALPHABITMAP *abp;

    if ((pRow == NULL ) || (pCol == NULL ))
    {
        return; //shuts up Lint
    }
    setBitmapSize(&bp);
    if (sDisplayChar != 0)
    {
        abp = (UIALPHABITMAP *) &alphabet[sDisplayChar];
        bp.pData = abp->pData;
        bp.nKern = abp->nKern;
        bp.nFlags = abp->nFlags;

        *pCol += abp->nKern;
    }
}

/*******************************************************************************
*       @details
*******************************************************************************/
void UI_DisplayStringCentered(const char* string, RECT* area)
{
    RECT rect;
    U_INT16 nLen = UI_GetTextSize(string);
    rect.ptTopLeft.nRow = ((area->ptBottomRight.nRow - area->ptTopLeft.nRow - 9) / 2) + area->ptTopLeft.nRow + 1;
    rect.ptTopLeft.nCol = ((area->ptBottomRight.nCol - area->ptTopLeft.nCol - nLen) / 2) + area->ptTopLeft.nCol;
    rect.ptBottomRight.nRow = area->ptBottomRight.nRow;
    rect.ptBottomRight.nCol = area->ptBottomRight.nCol;
    UI_DisplayText(string, rect);
}

/*******************************************************************************
*       @details
*******************************************************************************/
void UI_DisplayStringLeftJustified(const char* string, RECT* area)
{
    RECT rect;
    rect.ptTopLeft.nRow = ((area->ptBottomRight.nRow - area->ptTopLeft.nRow - 9) / 2) + area->ptTopLeft.nRow + 1;
    rect.ptTopLeft.nCol = area->ptTopLeft.nCol + 3; //This used to be +1
    rect.ptBottomRight.nRow = area->ptBottomRight.nRow;
    rect.ptBottomRight.nCol = area->ptBottomRight.nCol;
    UI_DisplayText(string, rect);
}

/*******************************************************************************
*       @details
*******************************************************************************/
void UI_DisplayStringRightJustified(const char* string, RECT* area)
{
    RECT rect;
    U_INT16 nLen = UI_GetTextSize(string);
    rect.ptTopLeft.nRow = ((area->ptBottomRight.nRow - area->ptTopLeft.nRow - 9) / 2) + area->ptTopLeft.nRow + 1;
    rect.ptTopLeft.nCol = area->ptBottomRight.nCol - 1 - nLen;
    rect.ptBottomRight.nRow = area->ptBottomRight.nRow;
    rect.ptBottomRight.nCol = area->ptBottomRight.nCol;
    UI_DisplayText(string, rect);
}

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 ; Function:
 ;   UI_DisplayText()
 ;
 ; Description:
 ;   Displays a string of text in a given rectangle. The flags are used to set
 ;   various options such as bold and underline.
 ;
 ; Parameters:
 ;   displayStr  =>  String that is to be displayed.
 ;   rctDisplay  =>  Rectangle of coordinates for placing the text.
 ;
 ; Reentrancy:
 ;   No.
 ;
 ; Note:
 ;
 ;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static void UI_DisplayText(const char* psDisplayStr, RECT rctDisplay)
{
    BYTE nRc;
    U_BYTE loopy;
	U_BYTE kern;
    UIBITMAP bp;
    UIALPHABITMAP const *abp;
    POINT ptTopLeft, ptBtmRight;
    static U_INT16 nRow, nCol, nLength;

    if (psDisplayStr == NULL)
    {
        return;
    }

    ptTopLeft = rctDisplay.ptTopLeft;
    ptBtmRight = rctDisplay.ptBottomRight;
    nRow = 0;
    nCol = 0;
    setBitmapSize(&bp);
    // Get each character one at a time and call the correct rendering
    // routine. Adjust the x and y positions at the end of the loop.
    loopy = 0;
    while (psDisplayStr[loopy] != 0)
    {
        abp = &alphabet[(U_BYTE)psDisplayStr[loopy]];
        //set data and kern
        bp.pData = abp->pData;
        bp.nKern = abp->nKern;
        bp.nFlags = abp->nFlags;
        if (nCol > ptBtmRight.nCol - (bp.nKern & 0x0F))
        {
//            return;
        }
        //render the bitmap and adjust the col to col += bitmap width
        UI_RenderBitmap(nRow, nCol, &bp, &nRc);
        kern = abp->nKern & 0x0F;
        if (kern == 0x0F)
        {
            nCol += 16;
        }
        else
        {
            nCol += kern;
        }
        loopy++;
    }
    if (nCol != 0)
    {
        nLength = nCol;
        nRow = ptTopLeft.nRow;
        nCol = ptTopLeft.nCol;
        UI_PrintLine(nRow, nCol, 12, nLength);
    }
}

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 ; Function:
 ;   UI_GetTextSize
 ;
 ; Description:
 ;               Returns the number of X space pixels that a
 ;               text string will use.
 ; Parameters:
 ;       CHAR psDisplayStr - String that is to be displayed.
 ;       U_BYTE *xSize - number of pixels of width the string uses
 ;       nFlags - F_ALTFONT: Use the secondary font
 ;
 ; Reentrancy
 ;    No
 ;
 ; Note:
 ;       We support two fonts: 8 pixel and 11 pixel. the 11 pixel
 ;       is selected by using the F_ALTFONT flag. The default is
 ;       the 8 pixel font.
 ;
 ;
 ;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
U_INT16 UI_GetTextSize(const char* psDisplayStr)
{
    U_INT16 kern;
	U_INT16 nCol = 0;
	U_BYTE loopy;

    if (psDisplayStr == NULL)
    {
        return 0;
    }
    for (loopy=0; psDisplayStr[loopy] != 0; loopy++)
    {
        kern = (U_INT16)alphabet[(U_BYTE)psDisplayStr[loopy]].nKern & 0x0F;
        if (kern == 0x000F)
        {
            nCol += 16;
        }
        else
        {
            nCol += kern;
        }
    }
    return nCol;
}

/*******************************************************************************
*       @details
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 ; Function:
 ;   setBitmapSize
 ;
 ; Description
 ;    Returns the number of X space pixels that a text string will use.
 ;
 ; Parameters:
 ;    *bp - bitmap to set the size of
 ;    nFlags - F_ALTFONT: Use the secondary font
 ;
 ; Reentrancy
 ;    No
 ;
 ; Note:
 ;       We support two fonts: 8 pixel and 11 pixel. the 11 pixel
 ;       is selected by using the F_ALTFONT flag. The default is
 ;       the 8 pixel font.
 ;
 ;
 ;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static void setBitmapSize(UIBITMAP *bp)
{
    if (bp == NULL)
    {
        return;
    }
    bp->nXSize = 8;
    bp->nYSize = 9;
    bp->nNumData = 8;
}

/*******************************************************************************
*       @details
*******************************************************************************/
void UI_DisplayStringInStatusFrame(const char* string, RECT* area, INT16 y)
{
    RECT rect;
    U_INT16 nLen = UI_GetTextSize(string);
    rect.ptTopLeft.nRow = ((area->ptBottomRight.nRow - area->ptTopLeft.nRow - 9) / 2) + area->ptTopLeft.nRow + 0; //This used to be +1 at the end
    rect.ptTopLeft.nCol = ((area->ptBottomRight.nCol - area->ptTopLeft.nCol - nLen) + y ) + area->ptTopLeft.nCol;
    rect.ptBottomRight.nRow = area->ptBottomRight.nRow;
    rect.ptBottomRight.nCol = area->ptBottomRight.nCol;
    UI_DisplayText(string, rect);
}

void UI_DisplayXScaleGraphFrame(const char* string, RECT* area, INT16 y)
{
    RECT rect;
    U_INT16 nLen = UI_GetTextSize(string);
    rect.ptTopLeft.nRow = 195;
    rect.ptTopLeft.nCol = y;
    rect.ptBottomRight.nRow = 217;
    rect.ptBottomRight.nCol = 316;
    UI_DisplayText(string, rect);
}

void UI_DisplayYScaleGraphFrame(const char* string, RECT* area, INT16 y)  // scale for left side of graph
{
    RECT rect;
    U_INT16 nLen = UI_GetTextSize(string);
    rect.ptTopLeft.nRow = y;
    rect.ptTopLeft.nCol = 5;
    rect.ptBottomRight.nRow = 217;
    rect.ptBottomRight.nCol = 316;
    UI_DisplayText(string, rect);
}

void UI_DisplayRightYScaleGraphFrame(const char* string, RECT* area, INT16 y) // scale for right side of graph
{
    RECT rect;
    U_INT16 nLen = UI_GetTextSize(string);
    rect.ptTopLeft.nRow = y;
    rect.ptTopLeft.nCol = 285;
    rect.ptBottomRight.nRow = 217;
    rect.ptBottomRight.nCol = 316;
    UI_DisplayText(string, rect);
}

void UI_DisplayGraphTitleFrame(const char* string, RECT* area, INT16 x, INT16 y)
{
    RECT rect;
    U_INT16 nLen = UI_GetTextSize(string);
    rect.ptTopLeft.nRow = x;
    rect.ptTopLeft.nCol = y;
    rect.ptBottomRight.nRow = 217;
    rect.ptBottomRight.nCol = 316;
    UI_DisplayText(string, rect);
}

void UI_DisplayString(const char* string, RECT* area, INT16 x, INT16 y)
{
    RECT rect;
    U_INT16 nLen = UI_GetTextSize(string);
    rect.ptTopLeft.nRow = x;
    rect.ptTopLeft.nCol = y;
    rect.ptBottomRight.nRow = 217;
    rect.ptBottomRight.nCol = 316;
    UI_DisplayText(string, rect);
}
