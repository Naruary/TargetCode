/*******************************************************************************
*       @brief      Implementation file for the Graphs
*       @file       Uphole/src/Graph_Plot/Graph_Plot.c
*       @date       Feb 2016
*       @copyright  COPYRIGHT (c) 2016 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <stdlib.h>
#include "lcd.h"
#include "Graph_Plot.h"
#include "Plan_Tab_Graph.h"
#include "UI_DataStructures.h"
#include "UI_LCDScreenInversion.h"
#include "LoggingManager.h"
#include "RecordManager.h"
#include "UI_Alphabet.h"
#include "UI_ScreenUtilities.h"
#include "UI_RecordDataPanel.h"
#include <stdio.h>
#include "UI_Frame.h"

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

void PlotGraph(void)
{
  RECT rect;
  rect.ptTopLeft.nRow = 0; rect.ptTopLeft.nCol = 0;
  rect.ptBottomRight.nRow = 238; rect.ptBottomRight.nCol = 319;
  UI_ClearLCDArea(&rect, LCD_FOREGROUND_PAGE);
  UI_ClearLCDArea(&rect, LCD_BACKGROUND_PAGE);
  LCD_Refresh(LCD_FOREGROUND_PAGE);
  LCD_Refresh(LCD_BACKGROUND_PAGE);
  GLCD_Circle(100, 100, 50);
  LCD_Refresh(LCD_FOREGROUND_PAGE);
  LCD_Refresh(LCD_BACKGROUND_PAGE);

  //LCD_Refresh(false);

}


/*!
********************************************************************************
*       @details
*******************************************************************************/

INT16 Round_Up_10(INT16 num)
{
    INT16 sign = 1;
    //INT16 tens = 1;

    if (num < 0)
    {
        num = (-1) * num;
        sign = -1;
    }
    num = (num + 9) / 10;
    num = num * 10;

    return sign * num;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

INT16 Round_Down_10(INT16 num)
{
    INT16 sign = 1;
    //INT16 tens = 1;

    if (num < 0)
    {
        num = (-1) * num;
        sign = -1;
        num = num + 9;
    }
    num = num / 10;
    num = num * 10;

    return sign * num;
}

