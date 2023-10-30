/*******************************************************************************
*       @brief      Implementation file for the UI TAB Frames section on the
*                   screen from Plots
*       @file       Uphole/src/Graph_Plot/Gamma_Compass.c
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
#include "lcd.h"
#include "adc.h"
#include "GammaSensor.h"
#include "Manager_DataLink.h"
#include "TextStrings.h"
#include "UI_Alphabet.h"
#include "UI_ScreenUtilities.h"
#include "UI_LCDScreenInversion.h"
#include "UI_Frame.h"
#include "UI_api.h"
#include "UI_BooleanField.h"
#include "UI_FixedField.h"
#include "UI_StringField.h"
#include "FlashMemory.h"
#include "RecordManager.h"
//#include "Graph_Plot.h"
#include "UI_Alphabet.h"
#include "Gamma_Compass.h"
#include <math.h>
#include "UI_Primitives.h"
#include "UI_DataStructures.h"







#include "UI_ToolFacePanels.h"
#include "Compass_Panel.h"
#include "UI_DownholeMainPanel.h"
#include "TargetProtocol.h"








//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

static MENU_ITEM* GetGammaCompassMenuItem(TAB_ENTRY* tab, U_BYTE index);
static U_BYTE GetGammaCompassMenuSize(TAB_ENTRY* tab);
static void GammaCompassTabPaint(TAB_ENTRY* tab);
static void GammaCompassTabShow(TAB_ENTRY* tab);
void DrawGammaRadarCompass(void);
void DrawGammaCompass(void);


//static MENU_ITEM* GetMenu(U_BYTE index);
static void TableTimeElapsed(TAB_ENTRY* tab);
static void TableKeyPressed(TAB_ENTRY* tab, BUTTON_VALUE key);

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

const TAB_ENTRY GammaCompassTab = {&TabFrame10, TXT_GAMCOMP, ShowTab, GetGammaCompassMenuItem, GetGammaCompassMenuSize, GammaCompassTabPaint, GammaCompassTabShow, TableTimeElapsed, TableKeyPressed};//GammaCompassTabMakeRequest};

enum { UI_GAMMA_COMPASS, };

U_INT16 panelIndex1 = UI_GAMMA_COMPASS;
REAL32 GTF1;
REAL32 GTF2;
REAL32 Toolface1;

#define MENU_SIZE (sizeof(menuGamma) / sizeof(MENU_ITEM))

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//


static PANEL *CurrentState(void)
{
	switch(panelIndex1)
	{
		case UI_GAMMA_COMPASS:
			return &Compass_DecisionPanel;
			break;
	}
	return NULL;
#if 0
	if(getStartNewHoleDecisionPanelActive())
	{
		return &StartNewHole_DecisionPanel;
	}
	else if(getClearAllHoleDataDecisionPanelActive())
	{
		return &ClearAllHoleData_DecisionPanel;
	}
//	else if(getUpdateDiagnosticDownholeDecisionPanelActive())
//	{
//		return &UpdateDiagnosticDownhole_DecisionPanel;
//	}
	else if(getCompassDecisionPanelActive())
	{
		return &Compass_DecisionPanel;
	}
	else if(getChangePipeLengthDecisionPanelActive())
	{
		return &ChangePipeLength_DecisionPanel;
	}
	else if(getEnterNewPipeLengthPanelActive())
	{
		return &EnterNewPipeLength_Panel;
	}
	else if(getChangePipeLengthCorrectDecisionPanelActive())
	{
		return &ChangePipeLengthCorrect_DecisionPanel;
	}
	else
	{
		// can do main screen stuff here if necessary
	}
	return &DownholePanel;
#endif
}

static void TableKeyPressed(TAB_ENTRY* tab, BUTTON_VALUE key)
{
#if 0
	// original non panelized did nothing here
#else
	if(CurrentState()->KeyPressed)
	{
		CurrentState()->KeyPressed(tab, key);
	}
#endif
}

static void TableTimeElapsed(TAB_ENTRY* tab)
{
#if 0
    MENU_ITEM* time = &menu[0];
    if ((!time->editing) && (UI_GetActiveFrame()->eID != ALERT_FRAME))
    {
        RepaintNow(time->valueFrame);
    }
#else
	if(CurrentState()->TimerElapsed)
	{
		CurrentState()->TimerElapsed(tab);
	}
#endif
}


/*******************************************************************************
*       @details
*******************************************************************************/
static MENU_ITEM* GetGammaCompassMenuItem(TAB_ENTRY* tab, U_BYTE index)
{
	PANEL *apanel;
	if(index < tab->MenuSize(tab))
	{
		apanel = CurrentState();
		if(apanel == NULL) return NULL;
		return CurrentState()->MenuItem(index);
	}
	return NULL;
}

/*******************************************************************************
*       @details
*******************************************************************************/
static U_BYTE GetGammaCompassMenuSize(TAB_ENTRY* tab)
{
	return CurrentState()->MenuCount;
}

/*******************************************************************************
*       @details
*******************************************************************************/
static void GammaCompassTabPaint(TAB_ENTRY* tab)
{
	CurrentState()->Paint(tab);
	DrawGammaCompass();
}
/*******************************************************************************
*       @details
*******************************************************************************/
static void GammaCompassTabShow(TAB_ENTRY* tab)
{
	if(CurrentState()->Show)
	{
		CurrentState()->Show(tab);
	}
	PaintNow(&HomeFrame);
}

void DrawGammaCompass(void)
{
//	RECT rect;
//	rect.ptTopLeft.nRow = 0; rect.ptTopLeft.nCol = 0;
//	rect.ptBottomRight.nRow = 240; rect.ptBottomRight.nCol = 320;
//	UI_ClearLCDArea(&HomeFrame.area, LCD_FOREGROUND_PAGE);
//	UI_ClearLCDArea(&HomeFrame.area, LCD_BACKGROUND_PAGE);
//	clearLCD();
//	LCD_Refresh(LCD_FOREGROUND_PAGE);
//	LCD_Refresh(LCD_BACKGROUND_PAGE);
//	drawScreenBorder();
//	PaintNow(&StatusFrame);
	DrawGammaRadarCompass();
}

void DrawGammaRadarCompass(void)
{
	FRAME ScaleNewFrame = HomeFrame;
	char Text[20];
	char Title[20];

	REAL32 Toolface;
	INT16 X_Toolface;
	INT16 Y_Toolface;
	U_INT16 Radii = 70;
//	U_INT16 Radii2 = 10; //Start of Gamma ticker mark
	U_INT16 TFCirXCent = 160;
	U_INT16 TFCirYCent = 120;
//	U_INT16 X_Toolface_Tick1;
//	U_INT16 Y_Toolface_Tick1;
//	U_INT16 X_Toolface_Tick2;
//	U_INT16 Y_Toolface_Tick2;
	REAL32 GTF;
//	REAL32 GTF1;
//	REAL32 GTF2;

	Toolface = ((GetSurveyRoll()/10) - 90);

	Toolface = Toolface * (PI / ONE_EIGHTY_DEGREES); //convert to radian


        GTF = (float)((GetSurveyRoll()/10)-90) + (float)GetToolFaceValue()/10;


        GTF = GTF * (PI / ONE_EIGHTY_DEGREES);



	// sin/cos is double in, double out..
	X_Toolface = TFCirXCent + (U_INT16)(Radii * cos(GTF)); //was Toolface in cos argument.
	Y_Toolface = TFCirYCent + (U_INT16)(Radii * sin(GTF));

//        U_INT16 Gamma_Scale_Max = 500;
//       U_INT16 Gamma_Compass_Offset;
//        U_INT16 tmp;
/*
        tmp = GetSurveyGamma();

        if (tmp < Gamma_Scale_Max/10)
        {
          Gamma_Compass_Offset = 10;
        }
        else
        {
          Gamma_Compass_Offset = 17;
        }
            X_Toolface_Tick1 = X_Toolface + (U_INT16)(Radii2 * cos(Toolface));
            Y_Toolface_Tick1 = Y_Toolface + (U_INT16)(Radii2 * sin(Toolface));

            X_Toolface_Tick2 = X_Toolface_Tick1 + (U_INT16)(Gamma_Compass_Offset * cos(Toolface));
            Y_Toolface_Tick2 = Y_Toolface_Tick1 + (U_INT16)(Gamma_Compass_Offset * sin(Toolface));

        if (Toolface_Offset == -90)
        {
            X_Toolface_Tick1 = (X_Toolface + (U_INT16)(Radii2 * cos(Toolface))) + X_Toolface_Offset;
            Y_Toolface_Tick1 = Y_Toolface + (U_INT16)(Radii2 * sin(Toolface)) + Y_Toolface_Offset + Y_Toolface;

            X_Toolface_Tick2 = X_Toolface_Tick1 + (U_INT16)(Gamma_Compass_Offset * cos(Toolface_Offset));
            Y_Toolface_Tick2 = Y_Toolface_Tick1 + (U_INT16)(Gamma_Compass_Offset * sin(Toolface_Offset));
        }

 */

	//GLCD_Circle(TFCirXCent, TFCirYCent, Radii);
	GLCD_Circle(TFCirXCent, TFCirYCent, Radii-3);
	GLCD_Circle(TFCirXCent, TFCirYCent, Radii+3);


	GLCD_Line(TFCirXCent, TFCirYCent, X_Toolface, Y_Toolface);   //this one
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

  //      GLCD_Line(X_Toolface_Tick1, Y_Toolface_Tick1, X_Toolface_Tick2, Y_Toolface_Tick2);






	snprintf(Text, 20, "0");
	UI_DisplayString(Text, &ScaleNewFrame.area, 70, 157);

	snprintf(Text, 20, "180");
	UI_DisplayString(Text, &ScaleNewFrame.area, 165, 150);

	snprintf(Text, 20, "90");
	UI_DisplayString(Text, &ScaleNewFrame.area, 116, 200);

	snprintf(Text, 20, "270");
	UI_DisplayString(Text, &ScaleNewFrame.area, 116, 100);

	Toolface = (GetSurveyRoll()/10);

	snprintf(Title, 20, "TF = %f", Toolface);
	UI_DisplayString(Title, &ScaleNewFrame.area, 50, 10); //point 1 different by 15 to be symmetric

	snprintf(Title, 20, "Gamma = %d", GetSurveyGamma());
	UI_DisplayString(Title, &ScaleNewFrame.area, 35, 235); //35,235
/*
        snprintf(Title, 20, "Push Shift Button");              //////
	UI_DisplayString(Title, &ScaleNewFrame.area, 5, 10);         //////

        snprintf(Title, 20, "to Record Gamma:");                     //////
	UI_DisplayString(Title, &ScaleNewFrame.area, 5, 135);
*/
  /*
        if (Toolface > (GetToolFaceValue()/10.0))
        {
            snprintf(Title, 20, "GTF = %.1f", (float)((GetToolFaceValue()/10.0)) + Toolface);    //////
            UI_DisplayString(Title, &ScaleNewFrame.area, 20, 10);
        }
        else
        {
*/

/*
        GTF = (float)((GetSurveyRoll()/10)) + (float)GetToolFaceValue()/10;

     //  GTF = (GTF * (ONE_EIGHTY_DEGREES / PI)) + 90;


        if (GTF > 360)
        {
          GTF = GTF - 360; // (ONE_EIGHTY_DEGREES / PI))-90;
        }
        else
        {

        }
*/
        if ((float)GetToolFaceValue()/10 > 360)
        {
          Toolface = (float)GetToolFaceValue()/10 - 360;
        }
        else
        {
          Toolface = (float)GetToolFaceValue()/10;
        }

        GTF1 = Toolface + (float)((GetSurveyRoll()/10));
   //     GTF = (GTF * (ONE_EIGHTY_DEGREES / PI)) + 90;

        if (GTF1 >= 360)
        {
          GTF2 = GTF1 - 360;
        }
        else
        {
          GTF2 = GTF1;
        }

        snprintf(Title, 20, "GTF = %.1f", GTF2);    //////
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

INT16 GetGTF(void)
{
        if ((float)GetToolFaceValue()/10 > 360)
        {
          Toolface1 = (float)GetToolFaceValue()/10 - 360;
        }
        else
        {
          Toolface1 = (float)GetToolFaceValue()/10;
        }

        GTF1 = Toolface1 + (float)((GetSurveyRoll()/10));
   //     GTF = (GTF * (ONE_EIGHTY_DEGREES / PI)) + 90;

        if (GTF1 >= 360)
        {
          GTF2 = GTF1 - 360;
        }
        else
        {
          GTF2 = GTF1;
        }
        return (INT16)GTF2;
}