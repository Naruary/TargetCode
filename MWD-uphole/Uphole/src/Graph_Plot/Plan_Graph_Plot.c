/*******************************************************************************
*       @brief      Implementation file for the Graphs
*       @file       Uphole/src/Graph_Plot/Plan_Graph_Plot.c
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
#include "Plan_Graph_Plot.h"
#include "buzzer.h"

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

static INT16 X_max_DT = 0; // downtrack
static INT16 X_min_DT = 0; // downtrack

static INT16 Y_max_LR = 0; // Left/Right
static INT16 Y_min_LR = 0; // Left/Right


static INT16 X_min_DT_Scanned = 0; // To check for -ve DT

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

void DrawPlanGraph(void)
{
    Find_Plan_Graph_Scale_Max_Min();

    BuzzerHandler(); // To make the beep shorter while plotting graph.

    if(Y_max_LR >= 0 && Y_min_LR >= 0)
      DrawPositiveQuadPlanGraph(X_max_DT, X_min_DT, Y_max_LR, Y_min_LR);
    else if(Y_max_LR > 0 && Y_min_LR < 0)
      DrawPositiveNegativeQuadPlanGraph(X_max_DT, X_min_DT, Y_max_LR, Y_min_LR);
    else if(Y_max_LR <= 0 && Y_min_LR < 0)
      DrawNegativeQuadPlanGraph(X_max_DT, X_min_DT, Y_max_LR, Y_min_LR);

}

/*******************************************************************************
*       @details
*******************************************************************************/
void DrawPositiveQuadPlanGraph(INT16 X_max, INT16 X_min, INT16 Y_max, INT16 Y_min)
{
	FRAME ScaleNewFrame = WindowFrame;
	char Scale[20];
	char Title[20];

	REAL32 ScaleInt = 0;
	struct GraphPlotArea PlanPlot;
	PlanPlot.Top_Left_X = 40;
	PlanPlot.Top_Left_Y = 30;
	PlanPlot.Top_Right_X = 310;
	PlanPlot.Top_Right_Y = 30;
	PlanPlot.Bottom_Left_X = 40;
	PlanPlot.Bottom_Left_Y = 185;
	PlanPlot.Bottom_Right_X = 310;
	PlanPlot.Bottom_Right_Y = 185;
	INT16 X_Range = X_max - X_min;
	INT16 Y_Range = Y_max - Y_min;
	//printf("X_Range = %d\n", X_Range);
	//printf("Y_Range = %d\n", Y_Range);
	REAL32 X_Scale_Resolution = X_Range/17.0; // 17 subdvisions
	REAL32 Y_Scale_Resolution = Y_Range/10.0; // 10 subdvisions
	//printf("X_Scale_Rous = %f\n", X_Scale_Resolution);
	//printf("Y_Scale_Rous = %f\n", Y_Scale_Resolution);
	snprintf(Title, 20, "L/R vs DT");
	UI_DisplayGraphTitleFrame(Title, &ScaleNewFrame.area, 37, 150);
	INT16 StatingPosition = PlanPlot.Bottom_Left_X;
	ScaleInt = X_min;
	INT16 ScaleLoop = 0;
	while((StatingPosition+15) < PlanPlot.Bottom_Right_X)
	{
		StatingPosition = StatingPosition + 15;
		GLCD_Line(StatingPosition, 183, StatingPosition, 185);
		GLCD_Line(StatingPosition, 30, StatingPosition, 32);
		ScaleInt += X_Scale_Resolution;
		if(ScaleLoop%5 == 0)
		{
			snprintf(Scale, 20, "%d", (int) ScaleInt);
			UI_DisplayXScaleGraphFrame(Scale, &ScaleNewFrame.area, StatingPosition-10);
			GLCD_Line(StatingPosition, 183, StatingPosition, 188);
		}
		ScaleLoop++;
	}
	GLCD_Line(40, 30, 40, 185);
	GLCD_Line(310, 30, 310, 185);
	StatingPosition = PlanPlot.Bottom_Left_Y;
	ScaleInt = Y_min;
	ScaleLoop = 0;
	while((StatingPosition-15) > PlanPlot.Top_Left_Y)
	{
		StatingPosition = StatingPosition - 15;
		GLCD_Line(40, StatingPosition, 42, StatingPosition);
		GLCD_Line(308, StatingPosition, 310, StatingPosition);
		ScaleInt += Y_Scale_Resolution;
		if(ScaleLoop%2 == 0)
		{
			snprintf(Scale, 20, "%d", (int) ScaleInt);
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
	INT16 PipeDTCurrent;
	INT16 PipeDTNext;
	RECORD_GetRecord(&current_record, StartRecord);
	for (int i = StartRecord; i < (lastRecord-1); i++)
	{
		RECORD_GetRecord(&next_record, i+1);
		if(next_record.PreviousBranchRecordNum)
		{
			RECORD_GetRecord(&current_record, next_record.PreviousBranchRecordNum);
		}
		PipeDTCurrent = current_record.Z;
		PipeDTNext = next_record.Z;
		//Check for -ve x-axis
		if((PipeDTCurrent < 0) || (PipeDTNext < 0) || (X_min_DT_Scanned < 0))
		{
			snprintf(Title, 20, "ERROR: DT cannot be Negative");
			UI_DisplayGraphTitleFrame(Title, &ScaleNewFrame.area, 150, 80);
			break;
		}
		X1 = (INT16) (PlanPlot.Bottom_Left_X + (((current_record.Z-(X_min*10))/10.0)/X_Scale_Resolution) * 15.0); // 15 pixels between subdivisions
		X2 = (INT16) (PlanPlot.Bottom_Left_X + (((next_record.Z-(X_min*10))/10.0)/X_Scale_Resolution) * 15.0); // 15 pixels between subdivisions
		Y1 = (INT16) (PlanPlot.Bottom_Left_Y - (((current_record.X-(Y_min*10))/10.0)/Y_Scale_Resolution) * 15.0); // 15 pixels between subdivisions
		Y2 = (INT16) (PlanPlot.Bottom_Left_Y - (((next_record.X-(Y_min*10))/10.0)/Y_Scale_Resolution) * 15.0); // 15 pixels between subdivisions
		GLCD_Line(X1, Y1, X2, Y2);
		memcpy(&current_record, &next_record, sizeof(STRUCT_RECORD_DATA));
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
void DrawPositiveNegativeQuadPlanGraph(INT16 X_max, INT16 X_min, INT16 Y_max, INT16 Y_min)
{
	FRAME ScaleNewFrame = WindowFrame;
	char Scale[20];
	char Title[20];

	REAL32 ScaleInt = 0;
	struct GraphPlotArea PlanPlot;
	PlanPlot.Top_Left_X = 40;
	PlanPlot.Top_Left_Y = 30;
	PlanPlot.Top_Right_X = 310;
	PlanPlot.Top_Right_Y = 30;
	PlanPlot.Bottom_Left_X = 40;
	PlanPlot.Bottom_Left_Y = 185;
	PlanPlot.Bottom_Right_X = 310;
	PlanPlot.Bottom_Right_Y = 185;
	INT16 X_Range = X_max - X_min;
	INT16 Y_Range = Y_max - Y_min;
	//printf("X_Range = %d\n", X_Range);
	//printf("Y_Range = %d\n", Y_Range);
	REAL32 X_Scale_Resolution = X_Range/17.0; // 17 subdvisions
	REAL32 Y_Scale_Resolution = Y_Range/10.0; // 10 subdvisions
	//printf("X_Scale_Rous = %f\n", X_Scale_Resolution);
	//printf("Y_Scale_Rous = %f\n", Y_Scale_Resolution);
	snprintf(Title, 20, "L/R vs DT");
	UI_DisplayGraphTitleFrame(Title, &ScaleNewFrame.area, 37, 150);
	INT16 StatingPosition = PlanPlot.Bottom_Left_X;
	ScaleInt = X_min;
	INT16 ScaleLoop = 0;
	while((StatingPosition+15) < PlanPlot.Bottom_Right_X)
	{
		StatingPosition = StatingPosition + 15;
		GLCD_Line(StatingPosition, 108, StatingPosition, 112);
		GLCD_Line(StatingPosition, 188, StatingPosition, 190);
		GLCD_Line(StatingPosition, 30, StatingPosition, 32);
		ScaleInt += X_Scale_Resolution;
		if(ScaleLoop%5 == 0)
		{
			snprintf(Scale, 20, "%d", (int) ScaleInt);
			UI_DisplayXScaleGraphFrame(Scale, &ScaleNewFrame.area, StatingPosition-10);
			GLCD_Line(StatingPosition, 188, StatingPosition, 193);
		}
		ScaleLoop++;
	}
	GLCD_Line(40, 30, 40, 190);
	GLCD_Line(310, 30, 310, 190);
	StatingPosition = PlanPlot.Bottom_Left_Y;
	ScaleInt = Y_min;
	ScaleLoop = 0;
	while((StatingPosition-15) > PlanPlot.Top_Left_Y)
	{
		StatingPosition = StatingPosition - 15;
		GLCD_Line(40, StatingPosition, 42, StatingPosition);
		GLCD_Line(308, StatingPosition, 310, StatingPosition);
		ScaleInt += Y_Scale_Resolution;
		if(ScaleLoop%2 == 0)
		{
			snprintf(Scale, 20, "%d", (int) ScaleInt);
			UI_DisplayYScaleGraphFrame(Scale, &ScaleNewFrame.area, StatingPosition-5);
			GLCD_Line(37, StatingPosition, 42, StatingPosition);
		}
		ScaleLoop++;
	}
	GLCD_Line(40, 110, 310, 110);
	GLCD_Line(40, 190, 310, 190);
	GLCD_Line(40, 30, 310, 30);
	INT16 X1, X2, Y1, Y2;
	STRUCT_RECORD_DATA current_record;
	STRUCT_RECORD_DATA next_record;
	INT16 StartRecord = GetRecordCount() - GetStartRecordNumber();
	INT16 lastRecord = GetRecordCount();
	INT16 Y_CenterPoint = (INT16) (Y_min + (5 * Y_Scale_Resolution));
	INT16 PipeDTCurrent;
	INT16 PipeDTNext;
	RECORD_GetRecord(&current_record, StartRecord);
	for (int i = StartRecord; i < (lastRecord-1); i++)
	{
		RECORD_GetRecord(&next_record, i+1);
		if(next_record.PreviousBranchRecordNum)
		{
			RECORD_GetRecord(&current_record, next_record.PreviousBranchRecordNum);
		}
		PipeDTCurrent = current_record.Z;
		PipeDTNext = next_record.Z;
		//Check for -ve x-axis
		if((PipeDTCurrent < 0) || (PipeDTNext < 0) || (X_min_DT_Scanned < 0))
		{
			snprintf(Title, 20, "ERROR: DT cannot be Negative");
			UI_DisplayGraphTitleFrame(Title, &ScaleNewFrame.area, 150, 80);
			break;
		}
		X1 = (INT16) (PlanPlot.Bottom_Left_X + (((current_record.Z-(X_min*10))/10.0)/X_Scale_Resolution) * 15.0); // 15 pixels between subdivisions
		X2 = (INT16) (PlanPlot.Bottom_Left_X + (((next_record.Z-(X_min*10))/10.0)/X_Scale_Resolution) * 15.0); // 15 pixels between subdivisions
		Y1 = (INT16) (110 - (((current_record.X-(Y_CenterPoint*10))/10.0)/Y_Scale_Resolution) * 15.0); // 15 pixels between subdivisions
		Y2 = (INT16) (110 - (((next_record.X-(Y_CenterPoint*10))/10.0)/Y_Scale_Resolution) * 15.0); // 15 pixels between subdivisions
		GLCD_Line(X1, Y1, X2, Y2);
		memcpy(&current_record, &next_record, sizeof(STRUCT_RECORD_DATA));
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
void DrawNegativeQuadPlanGraph(INT16 X_max, INT16 X_min, INT16 Y_max, INT16 Y_min)
{
	FRAME ScaleNewFrame = WindowFrame;
	char Scale[20];
	char Title[20];
	REAL32 ScaleInt = 0;
	struct GraphPlotArea PlanPlot;
	PlanPlot.Top_Left_X = 40;
	PlanPlot.Top_Left_Y = 30;
	PlanPlot.Top_Right_X = 310;
	PlanPlot.Top_Right_Y = 30;
	PlanPlot.Bottom_Left_X = 40;
	PlanPlot.Bottom_Left_Y = 185;
	PlanPlot.Bottom_Right_X = 310;
	PlanPlot.Bottom_Right_Y = 185;
	INT16 X_Range = X_max - X_min;
	INT16 Y_Range = Y_max - Y_min;
	//printf("X_Range = %d\n", X_Range);
	//printf("Y_Range = %d\n", Y_Range);
	REAL32 X_Scale_Resolution = X_Range/17.0; // 17 subdvisions
	REAL32 Y_Scale_Resolution = Y_Range/10.0; // 10 subdvisions
	//printf("X_Scale_Rous = %f\n", X_Scale_Resolution);
	//printf("Y_Scale_Rous = %f\n", Y_Scale_Resolution);
	snprintf(Title, 20, "L/R vs DT");
	UI_DisplayGraphTitleFrame(Title, &ScaleNewFrame.area, 40, 150);
	INT16 StatingPosition = PlanPlot.Bottom_Left_X;
	ScaleInt = X_min;
	INT16 ScaleLoop = 0;
	while((StatingPosition+15) < PlanPlot.Bottom_Right_X)
	{
		StatingPosition = StatingPosition + 15;
		GLCD_Line(StatingPosition, 35, StatingPosition, 37);
		GLCD_Line(StatingPosition, 188, StatingPosition, 190);
		ScaleInt += X_Scale_Resolution;
		if(ScaleLoop%5 == 0)
		{
			snprintf(Scale, 20, "%d", (int) ScaleInt);
			UI_DisplayXScaleGraphFrame(Scale, &ScaleNewFrame.area, StatingPosition-10);
			GLCD_Line(StatingPosition, 188, StatingPosition, 193);
		}
		ScaleLoop++;
	}
	GLCD_Line(40, 35, 40, 190);
	GLCD_Line(310, 35, 310, 190);
	StatingPosition = PlanPlot.Bottom_Left_Y;
	ScaleInt = Y_min;
	ScaleLoop = 0;
	while((StatingPosition-15) > PlanPlot.Top_Left_Y)
	{
		StatingPosition = StatingPosition - 15;
		GLCD_Line(40, StatingPosition, 42, StatingPosition);
		GLCD_Line(308, StatingPosition, 310, StatingPosition);
		ScaleInt += Y_Scale_Resolution;
		if(ScaleLoop%2 == 0)
		{
			snprintf(Scale, 20, "%d", (int) ScaleInt);
			UI_DisplayYScaleGraphFrame(Scale, &ScaleNewFrame.area, StatingPosition-5);
			GLCD_Line(37, StatingPosition, 42, StatingPosition);
		}
		ScaleLoop++;
	}
	GLCD_Line(40, 35, 310, 35);
	GLCD_Line(40, 190, 310, 190);
	INT16 X1, X2, Y1, Y2;
	STRUCT_RECORD_DATA current_record;
	STRUCT_RECORD_DATA next_record;
	INT16 StartRecord = GetRecordCount() - GetStartRecordNumber();
	INT16 lastRecord = GetRecordCount();
	INT16 PipeDTCurrent;
	INT16 PipeDTNext;
	RECORD_GetRecord(&current_record, StartRecord);
	for (int i = StartRecord; i < (lastRecord-1); i++)
	{
		RECORD_GetRecord(&next_record, i+1);
		if(next_record.PreviousBranchRecordNum)
		{
			RECORD_GetRecord(&current_record, next_record.PreviousBranchRecordNum);
		}
		PipeDTCurrent = current_record.Z;
		PipeDTNext = next_record.Z;
		//Check for -ve x-axis
		if((PipeDTCurrent < 0) || (PipeDTNext < 0) || (X_min_DT_Scanned < 0))
		{
			snprintf(Title, 20, "ERROR: DT cannot be Negative");
			UI_DisplayGraphTitleFrame(Title, &ScaleNewFrame.area, 150, 80);
			break;
		}
		X1 = (INT16) (PlanPlot.Bottom_Left_X + (((current_record.Z-(X_min*10))/10.0)/X_Scale_Resolution) * 15.0); // 15 pixels between subdivisions
		X2 = (INT16) (PlanPlot.Bottom_Left_X + (((next_record.Z-(X_min*10))/10.0)/X_Scale_Resolution) * 15.0); // 15 pixels between subdivisions
		Y1 = (INT16) (35 - (((current_record.X-(Y_max*10))/10.0)/Y_Scale_Resolution) * 15.0); // 15 pixels between subdivisions
		Y2 = (INT16) (35 - (((next_record.X-(Y_max*10))/10.0)/Y_Scale_Resolution) * 15.0); // 15 pixels between subdivisions
		GLCD_Line(X1, Y1, X2, Y2);
		memcpy(&current_record, &next_record, sizeof(STRUCT_RECORD_DATA));
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
void Find_Plan_Graph_Scale_Max_Min(void)
{
	INT16 StartRecord;
	STRUCT_RECORD_DATA record;

	StartRecord = GetRecordCount() - GetStartRecordNumber();
	INT16 lastRecord = GetRecordCount();
	if(!(InitNewHole_KeyPress() || IsClearHoleSelected()))
	{
		RECORD_GetRecord(&record, StartRecord);
		X_max_DT = record.Z; // Downtarck ->  Z = TotalDepth;
		X_min_DT = record.Z; // Downtarck ->  Z = TotalDepth;
		Y_max_LR= record.X; // Downtarck ->  Z = TotalDepth;
		Y_min_LR= record.X; // Downtarck ->  Z = TotalDepth;
	}
	else
	{
		X_max_DT = 0; // Downtarck ->  Z = TotalDepth;
		X_min_DT = 0; // Downtarck ->  Z = TotalDepth;
		Y_max_LR = 0; // UP/DOWN ->  Y =
		Y_min_LR = 0; // UP/DOWN ->  Y =
	}
	for (int i = StartRecord+1; i < lastRecord; i++)
	{
		RECORD_GetRecord(&record, i);
		if(record.Z > X_max_DT)
		{
			X_max_DT = record.Z;
		}
		if(record.Z < X_min_DT)
		{
			X_min_DT = record.Z;
		}
		if(record.X > Y_max_LR)
		{
			Y_max_LR = record.X;
		}
		if(record.X < Y_min_LR)
		{
			Y_min_LR = record.X;
		}
	}
	X_min_DT_Scanned = X_min_DT;
	X_max_DT = Round_Up_10((X_max_DT/10));
	X_min_DT = Round_Down_10((X_min_DT/10));
	if(X_max_DT - X_min_DT == 0)
	{
		X_max_DT = X_max_DT + 10;
		X_min_DT = X_min_DT - 10;
	}
	// Takes care of round_up bug when data is negative
	if(Y_max_LR <= -1)
	{
		Y_max_LR = Y_max_LR + 100;
	}
	// Takes care of round_up error when data is between 0.1 to 0.9
	if(Y_max_LR >= 1 && Y_max_LR <= 9)
	{
		Y_max_LR = Y_max_LR + 10;
	}
	// Takes care of round_down bug when data is negative
	if(Y_min_LR <= -1)
	{
		Y_min_LR = Y_min_LR - 100;
	}
	Y_max_LR = Round_Up_10((Y_max_LR/10));
	Y_min_LR = Round_Down_10((Y_min_LR/10));
	if(Y_max_LR - Y_min_LR == 0)
	{
		Y_max_LR = Y_max_LR + 10;
		Y_min_LR = Y_min_LR - 10;
	}
}

/*******************************************************************************
*       @details
*******************************************************************************/
INT16 Find_X_Scale_Max_Depth(void)
{
	INT16 StartRecord;
	STRUCT_RECORD_DATA record;
	INT16 Max;

	StartRecord = GetRecordCount() - GetStartRecordNumber();
	INT16 lastRecord = GetRecordCount();
	RECORD_GetRecord(&record, StartRecord);
	Max = record.Z; // Downtarck ->  Z = TotalDepth;
	for (int i = StartRecord+1; i < lastRecord; i++)
	{
		RECORD_GetRecord(&record, i);
		if(record.Z > Max)
		{
			Max = record.Z;
		}
	}
	Max = Round_Up_10(Max/10);
	return Max;
}

/*******************************************************************************
*       @details
*******************************************************************************/
INT16 Find_Y_Scale_Max_East(void)
{
	INT16 StartRecord;
	STRUCT_RECORD_DATA record;
	INT16 Max;
	StartRecord = GetRecordCount() - GetStartRecordNumber();
	INT16 lastRecord = GetRecordCount();
	RECORD_GetRecord(&record, StartRecord);
	Max = record.X; // Downtarck ->  Z = TotalDepth;
	for (int i = StartRecord+1; i < lastRecord; i++)
	{
		RECORD_GetRecord(&record, i);
		if(record.X > Max)
		{
			Max = record.X;
		}
	}
	Max = Round_Up_10(Max/10);
	return Max;
}

/*******************************************************************************
*       @details
*******************************************************************************/
INT16 Find_X_Scale_Min_Depth(void)
{
	INT16 StartRecord;
	STRUCT_RECORD_DATA record;
	INT16 Min = 0;
	StartRecord = GetRecordCount() - GetStartRecordNumber();
	INT16 lastRecord = GetRecordCount();
	RECORD_GetRecord(&record, StartRecord);
	Min = record.Z; // Downtarck ->  Z = TotalDepth;
	for (INT16 i = StartRecord+1; i < lastRecord; i++)
	{
		RECORD_GetRecord(&record, i);
		if(record.Z < Min)
		{
			Min = record.Z;
		}
	}
	Min = Round_Down_10(Min/10);
	return Min;
}

/*******************************************************************************
*       @details
*******************************************************************************/
INT16 Find_Y_Scale_Min_East(void)
{
	INT16 StartRecord;
	STRUCT_RECORD_DATA record;
	INT16 Min = 0;
	StartRecord = GetRecordCount() - GetStartRecordNumber();
	INT16 lastRecord = GetRecordCount();
	RECORD_GetRecord(&record, StartRecord);
	Min = record.X; // Downtarck ->  Z = TotalDepth;
	for (int i = StartRecord+1; i < lastRecord; i++)
	{
		RECORD_GetRecord(&record, i);
		if((record.X) < Min)
		{
			Min = record.X;
		}
	}
	Min = Round_Down_10(Min/10);
	return Min;
}
