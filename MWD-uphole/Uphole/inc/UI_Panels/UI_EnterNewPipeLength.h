/*******************************************************************************
*       @brief      This header file contains callable functions and public
*                   data for the Survey Edit Panel under the Data tab.
*       @file       Uphole/inc/UI_Panels/UI_EnterNewPipeLength.h
*       @date       November 2015
*       @copyright  COPYRIGHT (c) 2015 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef UI_ENTERNEWPIPELENGTH_PANEL_H
#define UI_ENTERNEWPIPELENGTH_PANEL_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "UI_MainTab.h"

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

extern PANEL EnterNewPipeLength_Panel;

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

	void setEnterNewPipeLengthPanelActive(BOOL bFlag);
	BOOL getEnterNewPipeLengthPanelActive(void);
	TIME_LR GetEnterNewPipeLengthTimer(void);
	INT16 GetNewPipeLength(void);
	void SetNewPipeLength(INT16 length);
        
        
        extern volatile BOOL Shift_Button_Pushed_Flag;
        extern volatile BOOL ManualChangePipeLengthFlag;
        

#ifdef __cplusplus
}
#endif
#endif
