/*******************************************************************************
*       @brief      This header file contains callable functions and public
*                   data for the Record Data Panel under the Data tab.
*       @file       Uphole/inc/UI_Panels/UI_RecordDataPanel.h
*       @date       July 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef UI_RECORD_DATA_PANEL_H
#define UI_RECORD_DATA_PANEL_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "UI_DataTab.h"

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

extern PANEL RecordDataPanel;


//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

	//   Initializes cursor on Record Data Panel
	void RecordDataPanelInit(void);
	//  Sets selected survey variable
	void RecordData_StoreSelectSurveyIndex(U_INT32 index);
	U_INT16 GetStartRecordNumber(void);

#ifdef __cplusplus
}
#endif
#endif
