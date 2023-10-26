/*******************************************************************************
*       @brief      Implementation file for the Compass
*       @file       Uphole/src/Graph_Plot/Compass_Plot.c
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
#include "FlashMemory.h"
#include "UI_Alphabet.h"
#include "UI_ScreenUtilities.h"
#include "UI_RecordDataPanel.h"
#include <stdio.h>
#include "UI_Frame.h"
#include <math.h>
#include "Compass_Plot.h"
#include "Manager_DataLink.h"
#include "UI_Primitives.h"
#include "UI_ToolFacePanels.h"

//============================================================================//
//      DATA DEFINITIONS                                                      //
//============================================================================//

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

void DrawCompass(void)
{
  //RECT rect;
  //rect.ptTopLeft.nRow = 0; rect.ptTopLeft.nCol = 0;
  //rect.ptBottomRight.nRow = 240; rect.ptBottomRight.nCol = 320;
  //UI_ClearLCDArea(&HomeFrame.area, LCD_FOREGROUND_PAGE);
  //UI_ClearLCDArea(&HomeFrame.area, LCD_BACKGROUND_PAGE);
  
  
  
  clearLCD();
  LCD_Refresh(LCD_FOREGROUND_PAGE);
  LCD_Refresh(LCD_BACKGROUND_PAGE);
  drawScreenBorder();
  PaintNow(&StatusFrame);  //StatusFrame
  int HomePaint();
  
  int DrawGammaCompass();
  
  
  //DrawAzimuthCompass();
  //DrawRollCompass();
  //DrawPitchCompass();
}

void DrawAzimuthCompass(void)
{
  FRAME ScaleNewFrame = HomeFrame;
  char Text[20];
  char Title[20];

  REAL32 Azimuth;
  INT16 X_Azimuth;
  INT16 Y_Azimuth;
  U_INT16 Radii = 45;
  U_INT16 AZCirXCent = 80;
  U_INT16 AZCirYCent = 85;
  Azimuth = ((GetSurveyAzimuth()/10) - 90);
  Azimuth = Azimuth * (3.14159 / 180); //convert to radian
  X_Azimuth = (INT16) (AZCirXCent + (Radii * cos(Azimuth)));
  Y_Azimuth = (INT16) (AZCirYCent + (Radii * sin(Azimuth)));
  //GLCD_Circle(AZCirXCent, AZCirYCent, Radii);
  GLCD_Circle(AZCirXCent, AZCirYCent, Radii-3);
  GLCD_Circle(AZCirXCent, AZCirYCent, Radii+3);
  GLCD_Line(AZCirXCent, AZCirYCent, X_Azimuth, Y_Azimuth);
  GLCD_Line(AZCirXCent+1, AZCirYCent+1, X_Azimuth, Y_Azimuth);
  GLCD_Line(AZCirXCent-1, AZCirYCent-1, X_Azimuth, Y_Azimuth);
  GLCD_Line(AZCirXCent+1, AZCirYCent-1, X_Azimuth, Y_Azimuth);
  GLCD_Line(AZCirXCent-1, AZCirYCent+1, X_Azimuth, Y_Azimuth);
  GLCD_Circle(AZCirXCent, AZCirYCent, 5);
  GLCD_Circle(AZCirXCent, AZCirYCent, 4);
  GLCD_Circle(AZCirXCent, AZCirYCent, 3);
  GLCD_Circle(AZCirXCent, AZCirYCent, 2);
  GLCD_Circle(AZCirXCent, AZCirYCent, 1);
  GLCD_Circle(X_Azimuth, Y_Azimuth, 3);
  GLCD_Circle(X_Azimuth, Y_Azimuth, 2);
  GLCD_Circle(X_Azimuth, Y_Azimuth, 1);
  snprintf(Text, 20, "N");
  UI_DisplayString(Text, &ScaleNewFrame.area, 45, 77);
  snprintf(Text, 20, "S");
  UI_DisplayString(Text, &ScaleNewFrame.area, 117, 77);
  snprintf(Text, 20, "E");
  UI_DisplayString(Text, &ScaleNewFrame.area, 81, 113);
  snprintf(Text, 20, "W");
  UI_DisplayString(Text, &ScaleNewFrame.area, 81, 40);
  snprintf(Text, 20, "0");
  UI_DisplayString(Text, &ScaleNewFrame.area, 26, 77);
  snprintf(Text, 20, "180");
  UI_DisplayString(Text, &ScaleNewFrame.area, 136, 70);
  snprintf(Text, 20, "90");
  UI_DisplayString(Text, &ScaleNewFrame.area, 81, 131);
  snprintf(Text, 20, "270");
  UI_DisplayString(Text, &ScaleNewFrame.area, 81, 8);
  Azimuth = (GetSurveyAzimuth()/10);
  snprintf(Title, 20, "AZ = %.1f", Azimuth);
  UI_DisplayString(Title, &ScaleNewFrame.area, 10, 50);
  GLCD_Line(38, 7, 123, 7);
  GLCD_Line(37, 6, 124, 6);
  GLCD_Line(38, 7, 38, 21);
  GLCD_Line(37, 6, 37, 22);
  GLCD_Line(38, 21, 123, 21);
  GLCD_Line(37, 22, 124, 22);
  GLCD_Line(123, 7, 123, 21);
  GLCD_Line(124, 6, 124, 22);
  LCD_Refresh(LCD_FOREGROUND_PAGE);
  LCD_Refresh(LCD_BACKGROUND_PAGE);
}

void DrawRollCompass(void)
{
  FRAME ScaleNewFrame = HomeFrame;
  char Text[20];
  char Title[20];

  REAL32 TF;
  INT16 X_TF;
  INT16 Y_TF;
  U_INT16 Radii = 45;
  U_INT16 TFCirXCent = 240;
  U_INT16 TFCirYCent = 85;
  REAL32 Roll;
  INT16 X_Roll;
  INT16 Y_Roll;

  TF = ((GetSurveyRoll()/10) - 90);
  TF = TF * (3.14159 / 180); //convert to radian
  X_TF = (INT16) (TFCirXCent + (Radii * cos(TF)));
  Y_TF = (INT16) (TFCirYCent + (Radii * sin(TF)));
  Roll = (((GetSurveyRoll() + GetToolface())/10) - 90);
  Roll = Roll * (3.14159 / 180); //convert to radian
  X_Roll = (INT16) (TFCirXCent + (Radii * cos(Roll)));
  Y_Roll = (INT16) (TFCirYCent + (Radii * sin(Roll)));
  //GLCD_Circle(TFCirXCent, TFCirYCent, Radii);
  GLCD_Circle(TFCirXCent, TFCirYCent, Radii-3);
  GLCD_Circle(TFCirXCent, TFCirYCent, Radii+3);
  //Draw Roll
  //GLCD_Line(TFCirXCent, TFCirYCent, X_Roll, Y_Roll);
  GLCD_Circle(X_Roll, Y_Roll, 3);
  GLCD_Circle(X_Roll, Y_Roll, 2);
  GLCD_Circle(X_Roll, Y_Roll, 1);
  //Draw Tool Face
  GLCD_Line(TFCirXCent, TFCirYCent, X_TF, Y_TF);
  GLCD_Line(TFCirXCent+1, TFCirYCent+1, X_TF, Y_TF);
  GLCD_Line(TFCirXCent-1, TFCirYCent-1, X_TF, Y_TF);
  GLCD_Line(TFCirXCent+1, TFCirYCent-1, X_TF, Y_TF);
  GLCD_Line(TFCirXCent-1, TFCirYCent+1, X_TF, Y_TF);
  GLCD_Circle(TFCirXCent, TFCirYCent, 5);
  GLCD_Circle(TFCirXCent, TFCirYCent, 4);
  GLCD_Circle(TFCirXCent, TFCirYCent, 3);
  GLCD_Circle(TFCirXCent, TFCirYCent, 2);
  GLCD_Circle(TFCirXCent, TFCirYCent, 1);
  GLCD_Circle(X_TF, Y_TF, 3);
  GLCD_Circle(X_TF, Y_TF, 2);
  GLCD_Circle(X_TF, Y_TF, 1);
  snprintf(Text, 20, "0");
  UI_DisplayString(Text, &ScaleNewFrame.area, 26, 237);
  snprintf(Text, 20, "180");
  UI_DisplayString(Text, &ScaleNewFrame.area, 136, 230);
  snprintf(Text, 20, "90");
  UI_DisplayString(Text, &ScaleNewFrame.area, 81, 291);
  snprintf(Text, 20, "270");
  UI_DisplayString(Text, &ScaleNewFrame.area, 81, 167);
  TF = (GetSurveyRoll()/10);
  snprintf(Title, 20, "TF = %.1f", TF);
  UI_DisplayString(Title, &ScaleNewFrame.area, 10, 210);
  GLCD_Line(198, 7, 283, 7);
  GLCD_Line(197, 6, 284, 6);
  GLCD_Line(198, 7, 198, 21);
  GLCD_Line(197, 6, 197, 22);
  GLCD_Line(198, 21, 283, 21);
  GLCD_Line(197, 22, 284, 22);
  GLCD_Line(283, 7, 283, 21);
  GLCD_Line(284, 6, 284, 22);
  LCD_Refresh(LCD_FOREGROUND_PAGE);
  LCD_Refresh(LCD_BACKGROUND_PAGE);
}

void DrawPitchCompass(void)
{
  FRAME ScaleNewFrame = HomeFrame;
  char Text[20];
  char Title[20];

  REAL32 Pitch;
  INT16 X_Pitch;
  INT16 Y_Pitch;
  U_INT16 Radii = 45;
  U_INT16 PitchCirXCent = 150;
  U_INT16 PitchCirYCent = 157;

  Pitch = GetSurveyPitch()/10;
  Pitch = 90 - Pitch;
  //if(Pitch < 0.0)
  //{
  //  Pitch = 360 + Pitch;
  //}
  Pitch = Pitch - 90;
  Pitch = Pitch * (3.14159 / 180); //convert to radian
  X_Pitch = (INT16) (PitchCirXCent + (Radii * cos(Pitch)));
  Y_Pitch = (INT16) (PitchCirYCent + (Radii * sin(Pitch)));
  //GLCD_SemiCircle(PitchCirXCent, PitchCirYCent, Radii);
  GLCD_SemiCircle(PitchCirXCent, PitchCirYCent, Radii-3);
  GLCD_SemiCircle(PitchCirXCent, PitchCirYCent, Radii+3);
  GLCD_Line(PitchCirXCent, PitchCirYCent, X_Pitch, Y_Pitch);
  GLCD_Line(PitchCirXCent+1, PitchCirYCent+1, X_Pitch, Y_Pitch);
  GLCD_Line(PitchCirXCent-1, PitchCirYCent-1, X_Pitch, Y_Pitch);
  GLCD_Line(PitchCirXCent-1, PitchCirYCent+1, X_Pitch, Y_Pitch);
  GLCD_Line(PitchCirXCent+1, PitchCirYCent-1, X_Pitch, Y_Pitch);
  //GLCD_Line(PitchCirXCent, PitchCirYCent, X_Pitch-10, Y_Pitch-10);
  //GLCD_Line(PitchCirXCent, PitchCirYCent, X_Pitch-15, Y_Pitch-15);
  GLCD_SemiCircle(PitchCirXCent, PitchCirYCent, 5);
  GLCD_SemiCircle(PitchCirXCent, PitchCirYCent, 4);
  GLCD_SemiCircle(PitchCirXCent, PitchCirYCent, 3);
  GLCD_SemiCircle(PitchCirXCent, PitchCirYCent, 2);
  GLCD_SemiCircle(PitchCirXCent, PitchCirYCent, 1);
  //GLCD_SemiCircle(PitchCirXCent, PitchCirYCent, 0);
  //GLCD_Circle(X_Pitch, Y_Pitch, 4);
  GLCD_Circle(X_Pitch, Y_Pitch, 3);
  GLCD_Circle(X_Pitch, Y_Pitch, 2);
  GLCD_Circle(X_Pitch, Y_Pitch, 1);
  GLCD_Line(PitchCirXCent, PitchCirYCent-Radii-3, PitchCirXCent, PitchCirYCent+Radii+3);
  GLCD_Line(PitchCirXCent-1, PitchCirYCent-Radii-3, PitchCirXCent-1, PitchCirYCent+Radii+3);
  //GLCD_Line(PitchCirXCent-2, PitchCirYCent-Radii-3, PitchCirXCent-2, PitchCirYCent+Radii+3);
  snprintf(Text, 20, "0");
  UI_DisplayString(Text, &ScaleNewFrame.area, 153, 202);
  snprintf(Text, 20, "-90");
  UI_DisplayString(Text, &ScaleNewFrame.area, 208, 140);
  snprintf(Text, 20, "+90");
  UI_DisplayString(Text, &ScaleNewFrame.area, 97, 140);
  Pitch = (GetSurveyPitch()/10);
  snprintf(Title, 20, "PITCH = %.1f", Pitch);
  UI_DisplayString(Title, &ScaleNewFrame.area, 180, 50);
  GLCD_Line(44, 177, 138, 177);
  GLCD_Line(43, 176, 139, 176);
  GLCD_Line(44, 177, 44, 191);
  GLCD_Line(43, 176, 43, 192);
  GLCD_Line(44, 191, 138, 191);
  GLCD_Line(43, 192, 139, 192);
  GLCD_Line(138, 191, 138, 177);
  GLCD_Line(139, 192, 139, 176);
  LCD_Refresh(LCD_FOREGROUND_PAGE);
  LCD_Refresh(LCD_BACKGROUND_PAGE);
}

void drawScreenBorder(void)
{
    RECT rect;
    memcpy(&rect, &HomeFrame.area, sizeof(RECT));
    UI_DrawRectangle(rect, LCD_BACKGROUND_PAGE);

    rect.ptTopLeft.nRow++;
    rect.ptTopLeft.nCol++;
    rect.ptBottomRight.nRow--;
    rect.ptBottomRight.nCol--;
    UI_DrawRectangle(rect, LCD_BACKGROUND_PAGE);
    LCD_Refresh(LCD_BACKGROUND_PAGE);
}
