/*!
********************************************************************************
*       @brief      Implementation file for the UI TAB Frames section on the 
*                   screen from Plots
*       @file       Uphole/src/Graph_Plot/Gamma_Compass.c
*       @author     Walter Rodrigues
*       @date       June 2016
*       @copyright  COPYRIGHT (c) 2016 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <stdio.h>
#include <stdlib.h>
#include "intf0003.h"
#include "lcd.h"
#include "adc.h"
#include "Manager_DataLink.h"
#include "RASP.h"
#include "TextStrings.h"
#include "UI_Alphabet.h"
#include "UI_ScreenUtilities.h"
#include "UI_LCDScreenInversion.h"
#include "UI_Frame.h"
#include "UI_api.h"
#include "UI_BooleanField.h"
#include "UI_FixedField.h"
#include "UI_StringField.h"
#include "SecureParameters.h"
#include "RecordManager.h"
#include "intf0008.h"
//#include "Graph_Plot.h"
#include "UI_Alphabet.h"
#include "Gamma_Compass.h"
#include <math.h>
#include "UI_Primitives.h"
#include "UI_DataStructures.h"


//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

///@brief  
///@param  
///@return 
static MENU_ITEM* GetGammaCompassMenuItem(TAB_ENTRY* tab, U_BYTE index);

///@brief  
///@param  
///@return 
static U_BYTE GetGammaCompassMenuSize(TAB_ENTRY* tab);

///@brief  
///@param  
///@return 
static void GammaCompassTabPaint(TAB_ENTRY* tab);

///@brief  
///@param  
///@return 
static void GammaCompassTabMakeRequest(TAB_ENTRY* tab);

///@brief  
///@param  
///@return 
static void GammaCompassTabShow(TAB_ENTRY* tab);

///@brief  
///@param  
///@return 
void DrawGammaRadarCompass(void);

///@brief  
///@param  
///@return
void DrawGammaCompass(void);


//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

///@brief
const TAB_ENTRY GammaCompassTab = { &TabFrame8, TXT_GAMCOMP, ShowTab, GetGammaCompassMenuItem, GetGammaCompassMenuSize, GammaCompassTabPaint, GammaCompassTabShow, GammaCompassTabMakeRequest};

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*!
********************************************************************************
*       @details
*******************************************************************************/

static MENU_ITEM* GetGammaCompassMenuItem(TAB_ENTRY* tab, U_BYTE index)
{
    return NULL ;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static U_BYTE GetGammaCompassMenuSize(TAB_ENTRY* tab)
{
     return NULL ;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void GammaCompassTabPaint(TAB_ENTRY* tab)
{
    TabWindowPaint(tab);
    DrawGammaCompass();
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void GammaCompassTabMakeRequest(TAB_ENTRY* tab)
{
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void GammaCompassTabShow(TAB_ENTRY* tab)
{
    PaintNow(&HomeFrame);
}





void DrawGammaCompass(void)
{
  //RECT rect;
  //rect.ptTopLeft.nRow = 0; rect.ptTopLeft.nCol = 0;
  //rect.ptBottomRight.nRow = 240; rect.ptBottomRight.nCol = 320;
  //UI_ClearLCDArea(&HomeFrame.area, LCD_FOREGROUND_PAGE);
  //UI_ClearLCDArea(&HomeFrame.area, LCD_BACKGROUND_PAGE);
  //clearLCD();
  //LCD_Refresh(LCD_FOREGROUND_PAGE);
  //LCD_Refresh(LCD_BACKGROUND_PAGE);
  
  //drawScreenBorder();
  //PaintNow(&StatusFrame);
  
  DrawGammaRadarCompass();
 }

void DrawGammaRadarCompass(void)
{
  FRAME ScaleNewFrame = HomeFrame;
  char Text[7];
  char Title[9];
    
  REAL32 Toolface;
  INT16 X_Toolface;
  INT16 Y_Toolface;
  U_INT16 Radii = 70;
  U_INT16 TFCirXCent = 160;
  U_INT16 TFCirYCent = 120;
  
  Toolface = ((GetSurveyRoll()/10) - 90); 
  
  Toolface = Toolface * (3.14159 / 180); //convert to radian
  
  X_Toolface = TFCirXCent + (Radii * cos(Toolface));
  Y_Toolface = TFCirYCent + (Radii * sin(Toolface));
  
  //GLCD_Circle(TFCirXCent, TFCirYCent, Radii);
  GLCD_Circle(TFCirXCent, TFCirYCent, Radii-3);
  GLCD_Circle(TFCirXCent, TFCirYCent, Radii+3);
  
  
  GLCD_Line(TFCirXCent, TFCirYCent, X_Toolface, Y_Toolface);
  GLCD_Line(TFCirXCent+1, TFCirYCent+1, X_Toolface, Y_Toolface);
  GLCD_Line(TFCirXCent-1, TFCirYCent-1, X_Toolface, Y_Toolface);
  GLCD_Line(TFCirXCent+1, TFCirYCent-1, X_Toolface, Y_Toolface);
  GLCD_Line(TFCirXCent-1, TFCirYCent+1, X_Toolface, Y_Toolface);
  
  GLCD_Circle(TFCirXCent, TFCirYCent, 5);
  GLCD_Circle(TFCirXCent, TFCirYCent, 4);
  GLCD_Circle(TFCirXCent, TFCirYCent, 3);
  GLCD_Circle(TFCirXCent, TFCirYCent, 2);
  GLCD_Circle(TFCirXCent, TFCirYCent, 1);
  
  GLCD_Circle(X_Toolface, Y_Toolface, 3);
  GLCD_Circle(X_Toolface, Y_Toolface, 2);
  GLCD_Circle(X_Toolface, Y_Toolface, 1);
  
  sprintf(Text, "0");
  UI_DisplayString(Text, &ScaleNewFrame.area, 70, 157);
  
  sprintf(Text, "180");
  UI_DisplayString(Text, &ScaleNewFrame.area, 165, 150);
  
  sprintf(Text, "90");
  UI_DisplayString(Text, &ScaleNewFrame.area, 116, 200);
  
  sprintf(Text, "270");
  UI_DisplayString(Text, &ScaleNewFrame.area, 116, 100);
  
  Toolface = (GetSurveyRoll()/10);
  
  sprintf(Title, "TF = %.1f", Toolface);
  UI_DisplayString(Title, &ScaleNewFrame.area, 50, 10);
  
  sprintf(Title, "Gamma = %d", GetSurveyGamma());
  UI_DisplayString(Title, &ScaleNewFrame.area, 35, 10);
  
  //GLCD_Line(38, 7, 123, 7);
  //GLCD_Line(37, 6, 124, 6);
  //GLCD_Line(38, 7, 38, 21);
  //GLCD_Line(37, 6, 37, 22);
  //GLCD_Line(38, 21, 123, 21);
  //GLCD_Line(37, 22, 124, 22);
  //GLCD_Line(123, 7, 123, 21);
  //GLCD_Line(124, 6, 124, 22);
  
  
  LCD_Refresh(LCD_FOREGROUND_PAGE);
  LCD_Refresh(LCD_BACKGROUND_PAGE);
  
}