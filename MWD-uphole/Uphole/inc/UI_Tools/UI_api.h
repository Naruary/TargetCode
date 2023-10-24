/*******************************************************************************
*       @brief      Header File for application interface with the UI.  This
*                   does a better job at trying to make the UI portable.
*       @file       Uphole/inc/UI_Tools/UI_api.h
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef UI_API_H
#define UI_API_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"
#include "PeriodicEvents.h"
#include "UI_Frame.h"
#include "UI_ScreenUtilities.h"

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

extern TAB_ENTRY const * const tabs[];

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

	void StartupFrameHandler(PERIODIC_EVENT *pEvent);
	void HomeFrameHandler(PERIODIC_EVENT *pEvent);
	void HomePaint(FRAME* frame);
	void TabPaint(FRAME* frame);
	void TabFrameHandler(PERIODIC_EVENT *pEvent);
	FRAME_ID GetActiveTabFrame(void);
	U_BYTE GetTabCount(void);
	void TabInitialize(FRAME* frame);
	void TabWindowPaint(TAB_ENTRY* tab);
	void WindowFrameHandler(PERIODIC_EVENT *pEvent);
	void WindowPaint(FRAME* frame);
	void LabelFrameHandler(PERIODIC_EVENT *pEvent);
	void LabelInitialize(FRAME* frame);
	void LabelPaint(FRAME* frame);
	void SetActiveLabelFrame(FRAME_ID frame);
	void CurrrentLabelFrame(MENU_ITEM* item);
	void ValueFrameHandler(PERIODIC_EVENT *pEvent);
	void ValueInitialize(FRAME* frame);
	void ValuePaint(FRAME* frame);
	void SetActiveValueFrame(FRAME_ID eNewFrame);
	void StatusFrameHandler(PERIODIC_EVENT *pEvent);
	void StatusPaint(FRAME* thisFrame);
	void AlertFrameHandler(PERIODIC_EVENT *pEvent);
	void AlertPaint(FRAME* thisFrame);

#ifdef __cplusplus
}
#endif
#endif
