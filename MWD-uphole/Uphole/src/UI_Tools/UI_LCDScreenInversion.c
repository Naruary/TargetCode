/*******************************************************************************
*       @brief      Implementation File for inverting (inverting pixels in)
*                   various regions on LCD.
*       @file       Uphole/src/UI_Tools/UI_LCDScreenInversion.c
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"
#include "UI_Defs.h"
#include "UI_Frame.h"
#include "UI_DataStructures.h"
#include "UI_LCDScreenInversion.h"

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*!
********************************************************************************
*       @details
*******************************************************************************/
/*------------------------------------------------------------------
; Function: UI_InvertLCDArea
;
; Description:
;   Inverts the pixels state from 0/1 to 1/0 on the LCD in a given
;   four point region. Checks if the passed Region forms a Rectangle,
;   if it forms then it inverts all the vertical lines that are inside
;   the rectangle, If it is not a rectangle then it draws an imaginary
;   boundary using the four points and then it inverts all the pixels
;   that are in the region formed by the boundary
;
; Paramaters:
;   RECT *pRect - Pointer to a rectangle
;
; Reentrancy:
;   No
;
;------------------------------------------------------------------*/
void UI_InvertLCDArea(const RECT *pRect, BOOL bPage)
{
    U_INT16 nRowPosn;

    U_INT16 nColLoLimit;
    U_INT16 nRowLoLimit;

    U_INT16 nColHiLimit;
    U_INT16 nRowHiLimit;

    nColLoLimit = pRect->ptTopLeft.nCol;
    nColHiLimit = pRect->ptBottomRight.nCol;

    nRowLoLimit = pRect->ptTopLeft.nRow;
    nRowHiLimit = pRect->ptBottomRight.nRow;

    nRowPosn = nRowLoLimit;

    while(nRowPosn <= nRowHiLimit)
    {
        LCD_InvertRow(nRowPosn, nColLoLimit, nColHiLimit, bPage);
        nRowPosn++;
    }

}//end UI_InvertLCDArea

/*!
********************************************************************************
*       @details
*******************************************************************************/

void UI_ClearLCDArea(const RECT *pRect, BOOL bPage)
{
    U_INT16 nRowPosn;

    U_INT16 nColLoLimit;
    U_INT16 nRowLoLimit;

    U_INT16 nColHiLimit;
    U_INT16 nRowHiLimit;

    nColLoLimit = pRect->ptTopLeft.nCol;
    nColHiLimit = pRect->ptBottomRight.nCol;

    nRowLoLimit = pRect->ptTopLeft.nRow;
    nRowHiLimit = pRect->ptBottomRight.nRow;

    nRowPosn = nRowLoLimit;

    while(nRowPosn <= nRowHiLimit)
    {
        LCD_ClearRow(nRowPosn, nColLoLimit, nColHiLimit, bPage);
        nRowPosn++;
    }
}
