/*******************************************************************************
*       @brief      This header file contains callable functions and public
*                   data for the Survey Edit Panel under the Data tab.
*       @file       Uphole/inc/UI_Panels/UI_EnterNewPipeLength.h
*       @date       November 2015
*       @copyright  COPYRIGHT (c) 2015 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef UI_ENTERSURVEY_PANEL_H
#define UI_ENTERSURVEY_PANEL_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "UI_MainTab.h"

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

extern PANEL EnterSurvey_Panel;

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

	void setEnterSurveyPanelActive(BOOL bFlag);
	BOOL getEnterSurveyPanelActive(void);
	TIME_LR GetEnterSurveyTimer(void);
	INT16 GetEnterSurvey(void);
	void SetEnterSurvey(INT16 length);
        void SetManualAzimuth(INT16 length);
        void SetManualPitch(INT16 length);
        void SetManualToolface(INT16 length);
        INT16 GetEnterAzimuth(void);
        INT16 GetEnterPitch(void);
        INT16 GetEnterToolface(void);
        
        
        extern volatile BOOL Shift_Button_Pushed_Flag;
        

#ifdef __cplusplus
}
#endif
#endif
