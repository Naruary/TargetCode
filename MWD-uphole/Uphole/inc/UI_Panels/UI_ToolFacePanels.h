/*!*****************************************************************************
*       @brief      This header file contains callable functions and public
*                   data for the Tool Face Panels
*       @file       Uphole/inc/UI_Panels/UI_ToolFacePanels.h
*       @date       August 2018
*       @copyright  COPYRIGHT (c) 2018 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef UI_TOOL_FACE_PANELS_H
#define UI_TOOL_FACE_PANELS_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "UI_DataTab.h"

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

extern PANEL ToolFaceZero_DecisionPanel;

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

	BOOL GetToolFaceZeroStartValue(void);
    TIME_LR GetToolFaceZeroTimer(void);
	void SetToolFaceZeroFinalValue(MENU_ITEM* item);
	ANGLE_TIMES_TEN GetToolFaceValue(void);
	void ClearToolFaceZero(MENU_ITEM* item);
        
        void DownloadToUSB(MENU_ITEM* item);
        void CreateFile(MENU_ITEM* item);
        void UploadFile(MENU_ITEM* item); //ZD 21September2023 This decalres an UploadFile function that is a Menu item for the user.
        void WriteFile(MENU_ITEM* item);
        void UnmountDrive(MENU_ITEM* item);
        ANGLE_TIMES_TEN GetGTFMain(void);

#ifdef __cplusplus
}
#endif

#endif // UI_TOOL_FACE_PANELS_H
