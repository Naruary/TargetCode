/*******************************************************************************
*       @brief      Implementation file for the Graphs
*       @file       Uphole/src/Graph_Plot/Side_Graph_Plot.c
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
#include "Side_Tab_Graph.h"
#include "UI_DataStructures.h"
#include "UI_LCDScreenInversion.h"
#include "LoggingManager.h"
#include "RecordManager.h"
#include "UI_Alphabet.h"
#include "UI_ScreenUtilities.h"
#include "UI_RecordDataPanel.h"
#include <stdio.h>
#include "UI_Frame.h"
#include "buzzer.h"
#include "Side_Graph_Plot.h"
#include <math.h>

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

static INT16 X_max_PL = 0; // downtrack
static INT16 X_min_PL = 0; // downtrack

static INT16 Y_max_UD = 0; // Left/Right
static INT16 Y_min_UD = 0; // Left/Right

static INT16 X_min_PL_Scanned = 0; // To check for -ve PL

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

void DrawSideGraph(void)
{
    Find_Side_Graph_Scale_Max_Min();

    BuzzerHandler(); // To make the beep shorter while plotting graph.

    if(Y_max_UD >= 0 && Y_min_UD >= 0)
      DrawPositiveQuadSideGraph(X_max_PL, X_min_PL, Y_max_UD, Y_min_UD);
    else if(Y_max_UD > 0 && Y_min_UD < 0)
      DrawPositiveNegativeQuadSideGraph(X_max_PL, X_min_PL, Y_max_UD, Y_min_UD);
    else if(Y_max_UD <= 0 && Y_min_UD < 0)
      DrawNegativeQuadSideGraph(X_max_PL, X_min_PL, Y_max_UD, Y_min_UD);

}

/*!
********************************************************************************
*       @details
*******************************************************************************/
void DrawPositiveQuadSideGraph(INT16 X_max, INT16 X_min, INT16 Y_max, INT16 Y_min)
{
    FRAME ScaleNewFrame = WindowFrame;
    char Scale[20];
    char Title[20];
    REAL32 ScaleInt = 0;
    struct GraphPlotArea SidePlot;
    SidePlot.Top_Left_X = 40;
    SidePlot.Top_Left_Y = 30;
    SidePlot.Top_Right_X = 310;
    SidePlot.Top_Right_Y = 30;
    SidePlot.Bottom_Left_X = 40;
    SidePlot.Bottom_Left_Y = 185;
    SidePlot.Bottom_Right_X = 310;
    SidePlot.Bottom_Right_Y = 185;
    INT16 X_Range = X_max - X_min;
    INT16 Y_Range = Y_max - Y_min;
    REAL32 X_Scale_Resolution = X_Range/17.0; // 17 subdvisions
    REAL32 Y_Scale_Resolution = Y_Range/10.0; // 10 subdvisions
    snprintf(Title, 20, "U/D vs PL");
    UI_DisplayGraphTitleFrame(Title, &ScaleNewFrame.area, 37, 145);
    INT16 StatingPosition = SidePlot.Bottom_Left_X;
    ScaleInt = X_min;
    INT16 ScaleLoop = 0;
    while((StatingPosition+15) < SidePlot.Bottom_Right_X)
    {
      StatingPosition = StatingPosition + 15;
      GLCD_Line(StatingPosition, 183, StatingPosition, 185);
      GLCD_Line(StatingPosition, 30, StatingPosition, 32);
      ScaleInt += X_Scale_Resolution;
      if(ScaleLoop%5 == 0)
      {
//
        snprintf(Scale, 20, "%.0f", (double)ScaleInt*10);
//
        UI_DisplayXScaleGraphFrame(Scale, &ScaleNewFrame.area, StatingPosition-10);
        GLCD_Line(StatingPosition, 183, StatingPosition, 188);
      }
      ScaleLoop++;
    }
    GLCD_Line(40, 30, 40, 185);
    GLCD_Line(310, 30, 310, 185);
    StatingPosition = SidePlot.Bottom_Left_Y;
    ScaleInt = Y_min;
    ScaleLoop = 0;
    while((StatingPosition-15) > SidePlot.Top_Left_Y)
    {
      StatingPosition = StatingPosition - 15;
      GLCD_Line(40, StatingPosition, 42, StatingPosition);
      GLCD_Line(308, StatingPosition, 310, StatingPosition);
      ScaleInt += Y_Scale_Resolution;
      if(ScaleLoop%2 == 0)
      {
//
          snprintf(Scale, 20, "%.1f", (double)ScaleInt / 10);
//
        UI_DisplayYScaleGraphFrame(Scale, &ScaleNewFrame.area, StatingPosition-5);
        GLCD_Line(37, StatingPosition, 42, StatingPosition);
      }
      ScaleLoop++;
    }
    GLCD_Line(40, 185, 310, 185);
    GLCD_Line(40, 30, 310, 30);
    INT16 X1, X2, Y1, Y2;
    STRUCT_RECORD_DATA current_record;
    STRUCT_RECORD_DATA next_record;
    INT16 StartRecord = GetRecordCount() - GetStartRecordNumber();
    INT16 lastRecord = GetRecordCount();
    INT16 PipeLengthCurrent;
    INT16 PipeLengthNext;
    RECORD_GetRecord(&current_record, StartRecord);
    for (int i = StartRecord; i < (lastRecord-1); i++)
    {
      RECORD_GetRecord(&next_record, i+1);
      if(next_record.PreviousBranchRecordNum)
      {
        RECORD_GetRecord(&current_record, next_record.PreviousBranchRecordNum);
      }
      PipeLengthCurrent = current_record.nTotalLength;
      PipeLengthNext = next_record.nTotalLength;
      //Check for -ve x-axis
      if((PipeLengthCurrent < 0) || (PipeLengthNext < 0)  || (X_min_PL_Scanned < 0))
      {
        snprintf(Title, 20, "ERROR: PL cannot be Negative");
        UI_DisplayGraphTitleFrame(Title, &ScaleNewFrame.area, 150, 80);
        break;
      }
      X1 = (INT16) (SidePlot.Bottom_Left_X + (((current_record.nTotalLength-(X_min*10))/10.0)/X_Scale_Resolution) * 15.0); // 15 pixels between subdivisions
      X2 = (INT16) (SidePlot.Bottom_Left_X + (((next_record.nTotalLength-(X_min*10))/10.0)/X_Scale_Resolution) * 15.0); // 15 pixels between subdivisions
      Y1 = (INT16) (SidePlot.Bottom_Left_Y - (((current_record.Y-(Y_min*10))/10.0)/Y_Scale_Resolution) * 15.0); // 15 pixels between subdivisions
      Y2 = (INT16) (SidePlot.Bottom_Left_Y - (((next_record.Y-(Y_min*10))/10.0)/Y_Scale_Resolution) * 15.0); // 15 pixels between subdivisions
      GLCD_Line(X1, Y1, X2, Y2);
      memcpy(&current_record, &next_record, sizeof(STRUCT_RECORD_DATA));
    }
}

/*!
********************************************************************************
*       @details
*******************************************************************************/
void DrawPositiveNegativeQuadSideGraph(INT16 X_max, INT16 X_min, INT16 Y_max, INT16 Y_min)
{
    FRAME ScaleNewFrame = WindowFrame;
    char Scale[20];
    char Title[20];
    REAL32 ScaleInt = 0;
    struct GraphPlotArea SidePlot;
    SidePlot.Top_Left_X = 40;
    SidePlot.Top_Left_Y = 30;
    SidePlot.Top_Right_X = 310;
    SidePlot.Top_Right_Y = 30;
    SidePlot.Bottom_Left_X = 40;
    SidePlot.Bottom_Left_Y = 185;
    SidePlot.Bottom_Right_X = 310;
    SidePlot.Bottom_Right_Y = 185;
    INT16 X_Range = X_max - X_min;
    INT16 Y_Range = Y_max - Y_min;
    REAL32 X_Scale_Resolution = X_Range/17.0; // 17 subdvisions
    REAL32 Y_Scale_Resolution = Y_Range/10.0; // 10 subdvisions
    snprintf(Title, 20, "U/D vs PL");
    UI_DisplayGraphTitleFrame(Title, &ScaleNewFrame.area, 37, 145);
    INT16 StatingPosition = SidePlot.Bottom_Left_X;
    ScaleInt = X_min;
    INT16 ScaleLoop = 0;
    while((StatingPosition+15) < SidePlot.Bottom_Right_X)
    {
      StatingPosition = StatingPosition + 15;
      GLCD_Line(StatingPosition, 108, StatingPosition, 112);
      GLCD_Line(StatingPosition, 188, StatingPosition, 190);
      GLCD_Line(StatingPosition, 30, StatingPosition, 32);
      ScaleInt += X_Scale_Resolution;
      if(ScaleLoop%5 == 0)
      {
//
          snprintf(Scale, 20, "%.0f", (double)ScaleInt * 10);
//
        UI_DisplayXScaleGraphFrame(Scale, &ScaleNewFrame.area, StatingPosition-10);
        GLCD_Line(StatingPosition, 188, StatingPosition, 193);
      }
      ScaleLoop++;
    }
    GLCD_Line(40, 30, 40, 190);
    GLCD_Line(310, 30, 310, 190);
    StatingPosition = SidePlot.Bottom_Left_Y;
    ScaleInt = Y_min;
    ScaleLoop = 0;
    while((StatingPosition-15) > SidePlot.Top_Left_Y)
    {
      StatingPosition = StatingPosition - 15;
      GLCD_Line(40, StatingPosition, 42, StatingPosition);
      GLCD_Line(308, StatingPosition, 310, StatingPosition);
      ScaleInt += Y_Scale_Resolution;
      if(ScaleLoop%2 == 0)
      {
//
          snprintf(Scale, 20, "%.1f", (double)ScaleInt / 10);
//
        UI_DisplayYScaleGraphFrame(Scale, &ScaleNewFrame.area, StatingPosition-5);
        GLCD_Line(37, StatingPosition, 42, StatingPosition);
      }
      ScaleLoop++;
    }
    GLCD_Line(40, 185, 42, 185);//
    GLCD_Line(308,185, 310,185);//
    GLCD_Line(40, 110, 310, 110); // x axis
    GLCD_Line(40, 190, 310, 190);
    GLCD_Line(40, 30, 310, 30);
    INT16 X1, X2, Y1, Y2;
    STRUCT_RECORD_DATA current_record;
    STRUCT_RECORD_DATA next_record;
    INT16 StartRecord = GetRecordCount() - GetStartRecordNumber();
    INT16 lastRecord = GetRecordCount();
    INT16 Y_CenterPoint = (INT16) (Y_min + (5 * Y_Scale_Resolution));
    INT16 PipeLengthCurrent;
    INT16 PipeLengthNext;
    RECORD_GetRecord(&current_record, StartRecord);
    for (int i = StartRecord; i < (lastRecord-1); i++)
    {
      RECORD_GetRecord(&next_record, i+1);
      if(next_record.PreviousBranchRecordNum)
      {
        RECORD_GetRecord(&current_record, next_record.PreviousBranchRecordNum);
      }
      PipeLengthCurrent = current_record.nTotalLength;
      PipeLengthNext = next_record.nTotalLength;
      //Check for -ve x-axis
      if((PipeLengthCurrent < 0) || (PipeLengthNext < 0) || (X_min_PL_Scanned < 0))
      {
        snprintf(Title, 20, "ERROR: PL cannot be Negative");
        UI_DisplayGraphTitleFrame(Title, &ScaleNewFrame.area, 150, 80);
        break;
      }
      X1 = (INT16) (SidePlot.Bottom_Left_X + (((current_record.nTotalLength-(X_min*10))/10.0)/X_Scale_Resolution) * 15); // 15 pixels between subdivisions
      X2 = (INT16) (SidePlot.Bottom_Left_X + (((next_record.nTotalLength-(X_min*10))/10.0)/X_Scale_Resolution) * 15); // 15 pixels between subdivisions
      Y1 = (INT16) (110 - (((current_record.Y-(Y_CenterPoint*10))/10.0)/Y_Scale_Resolution) * 15.0); // 15 pixels between subdivisions
      Y2 = (INT16) (110 - (((next_record.Y-(Y_CenterPoint*10))/10.0)/Y_Scale_Resolution) * 15.0); // 15 pixels between subdivisions
      GLCD_Line(X1, Y1, X2, Y2);
      memcpy(&current_record, &next_record, sizeof(STRUCT_RECORD_DATA));
    }
}

/*!
********************************************************************************
*       @details
*******************************************************************************/
void DrawNegativeQuadSideGraph(INT16 X_max, INT16 X_min, INT16 Y_max, INT16 Y_min)
{
    FRAME ScaleNewFrame = WindowFrame;
    char Scale[20];
    char Title[20];
    REAL32 ScaleInt = 0;
    struct GraphPlotArea SidePlot;
    SidePlot.Top_Left_X = 40;
    SidePlot.Top_Left_Y = 30;
    SidePlot.Top_Right_X = 310;
    SidePlot.Top_Right_Y = 30;
    SidePlot.Bottom_Left_X = 40;
    SidePlot.Bottom_Left_Y = 185;
    SidePlot.Bottom_Right_X = 310;
    SidePlot.Bottom_Right_Y = 185;
    INT16 X_Range = X_max - X_min;
    INT16 Y_Range = Y_max - Y_min;
    REAL32 X_Scale_Resolution = X_Range/17.0; // 17 subdvisions
    REAL32 Y_Scale_Resolution = Y_Range/10.0; // 10 subdvisions
    snprintf(Title, 20, "U/D vs PL");
    UI_DisplayGraphTitleFrame(Title, &ScaleNewFrame.area, 42, 145);
    INT16 StatingPosition = SidePlot.Bottom_Left_X;
    ScaleInt = X_min;
    INT16 ScaleLoop = 0;
    while((StatingPosition+15) < SidePlot.Bottom_Right_X)
    {
      StatingPosition = StatingPosition + 15;
      GLCD_Line(StatingPosition, 35, StatingPosition, 37);
      GLCD_Line(StatingPosition, 188, StatingPosition, 190);
      ScaleInt += X_Scale_Resolution;
      if(ScaleLoop%5 == 0)
      {
//
          snprintf(Scale, 20, "%.0f", (double)ScaleInt * 10);
//
        UI_DisplayXScaleGraphFrame(Scale, &ScaleNewFrame.area, StatingPosition-10);
        GLCD_Line(StatingPosition, 188, StatingPosition, 193);
      }
      ScaleLoop++;
    }
    GLCD_Line(40, 35, 40, 190);
    GLCD_Line(310, 35, 310, 190);
    StatingPosition = SidePlot.Bottom_Left_Y;
    ScaleInt = Y_min;
    ScaleLoop = 0;
    while((StatingPosition-15) > SidePlot.Top_Left_Y)
    {
      StatingPosition = StatingPosition - 15;
      GLCD_Line(40, StatingPosition, 42, StatingPosition);
      GLCD_Line(308, StatingPosition, 310, StatingPosition);
      ScaleInt += Y_Scale_Resolution;
      if(ScaleLoop%2 == 0)
      {
//
          snprintf(Scale, 20, "%.1f", (double)ScaleInt / 10);
//
        UI_DisplayYScaleGraphFrame(Scale, &ScaleNewFrame.area, StatingPosition-5);
        GLCD_Line(37, StatingPosition, 42, StatingPosition);
      }
      ScaleLoop++;
    }
    GLCD_Line(40, 185, 42, 185);//
    GLCD_Line(308,185, 310,185);//
    GLCD_Line(40, 35, 310, 35);
    GLCD_Line(40, 190, 310, 190);
    INT16 X1, X2, Y1, Y2;
    STRUCT_RECORD_DATA current_record;
    STRUCT_RECORD_DATA next_record;
    INT16 StartRecord = GetRecordCount() - GetStartRecordNumber();
    INT16 lastRecord = GetRecordCount();
    INT16 PipeLengthCurrent;
    INT16 PipeLengthNext;
    RECORD_GetRecord(&current_record, StartRecord);
    for (int i = StartRecord; i < (lastRecord-1); i++)
    {
      RECORD_GetRecord(&next_record, i+1);
      if(next_record.PreviousBranchRecordNum)
      {
        RECORD_GetRecord(&current_record, next_record.PreviousBranchRecordNum);
      }
      PipeLengthCurrent = current_record.nTotalLength;
      PipeLengthNext = next_record.nTotalLength;
      //Check for -ve x-axis
      if((PipeLengthCurrent < 0) || (PipeLengthNext < 0) || (X_min_PL_Scanned < 0))
      {
        snprintf(Title, 20, "ERROR: PL cannot be Negative");
        UI_DisplayGraphTitleFrame(Title, &ScaleNewFrame.area, 150, 80);
        break;
      }
      X1 = (INT16) (SidePlot.Bottom_Left_X + (((current_record.nTotalLength-(X_min*10))/10.0)/X_Scale_Resolution) * 15.0); // 15 pixels between subdivisions
      X2 = (INT16) (SidePlot.Bottom_Left_X + (((next_record.nTotalLength-(X_min*10))/10.0)/X_Scale_Resolution) * 15.0); // 15 pixels between subdivisions
      Y1 = (INT16) (35 - (((current_record.Y-(Y_max*10))/10.0)/Y_Scale_Resolution) * 15.0); // 15 pixels between subdivisions
      Y2 = (INT16) (35 - (((next_record.Y-(Y_max*10))/10.0)/Y_Scale_Resolution) * 15.0); // 15 pixels between subdivisions
      GLCD_Line(X1, Y1, X2, Y2);
      memcpy(&current_record, &next_record, sizeof(STRUCT_RECORD_DATA));
    }
}

/*!
********************************************************************************
*       @details
*******************************************************************************/
void Find_Side_Graph_Scale_Max_Min(void)
{
  INT16 StartRecord;
  STRUCT_RECORD_DATA record;

  StartRecord = GetRecordCount() - GetStartRecordNumber();
  INT16 lastRecord = GetRecordCount();

  if(!(InitNewHole_KeyPress() || IsClearHoleSelected()))
  {
    RECORD_GetRecord(&record, StartRecord);
    X_max_PL = record.nTotalLength; // PipeLength;
    X_min_PL = record.nTotalLength; // PipeLength;
    Y_max_UD = record.Y; // UP/DOWN ->  Y =
    Y_min_UD = record.Y; // UP/DOWN ->  Y =
  }
  else
  {
    X_max_PL = 0; // PipeLength;
    X_min_PL = 0; // PipeLength;
    Y_max_UD = 0; // UP/DOWN ->  Y =
    Y_min_UD = 0; // UP/DOWN ->  Y =
  }

  for (int i = StartRecord+1; i < lastRecord; i++)
    {
      RECORD_GetRecord(&record, i);

      if(record.nTotalLength > X_max_PL)
      {
        X_max_PL = record.nTotalLength;
      }

      if(record.nTotalLength < X_min_PL)
      {
        X_min_PL = record.nTotalLength;
      }

      if(record.Y > Y_max_UD)
      {
        Y_max_UD = record.Y;
      }

      if(record.Y < Y_min_UD)
      {
        Y_min_UD = record.Y;
      }
    }

  X_min_PL_Scanned = X_min_PL;

  X_max_PL = Round_Up_10((X_max_PL/10));
  X_min_PL = Round_Down_10((X_min_PL/10));

  if(X_max_PL - X_min_PL == 0)
  {
    X_max_PL = X_max_PL + 10;
    X_min_PL = X_min_PL - 10;
  }

//Takes care of round_up bug when data is negative
  if(Y_max_UD <= -1)
  {
    Y_max_UD = Y_max_UD + 100;
  }

  // Takes care of round_up error when data is between 0.1 to 0.9
  if(Y_max_UD >= 1 && Y_max_UD <= 9)
  {
    Y_max_UD = Y_max_UD + 10;
  }

  // Takes care of round_down bug when data is negative
  if(Y_min_UD <= -1)
  {
    Y_min_UD = Y_min_UD - 100;
  }

  Y_max_UD = Round_Up_10((Y_max_UD/10));
  Y_min_UD = Round_Down_10((Y_min_UD/10));

  if(Y_max_UD - Y_min_UD == 0)
  {
    Y_max_UD = Y_max_UD + 10;
    Y_min_UD = Y_min_UD - 10;
  }

//  if(Y_max_UD > 0 && Y_min_UD < 0)
//  {
//    if( abs(Y_max_UD) > abs(Y_min_UD) )
//    {
//      Y_min_UD = -1* Y_max_UD;
//    }
//    else
//    {
//      Y_max_UD = -1*Y_min_UD;
//    }
//  }

}
