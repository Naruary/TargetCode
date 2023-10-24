/*******************************************************************************
*       @brief      Header File for UI_Frame.c.
*       @file       Uphole/inc/UI_Frame/UI_Frame.h
*       @date       December 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef UI_FRAME_H
#define UI_FRAME_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"
#include "periodicEvents.h"
#include "UI_DataStructures.h"

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

typedef struct _FRAME
{
	FRAME_ID eID;
	RECT area;
	void (*ProcessEvent)(PERIODIC_EVENT* event);
	void (*Paint)(struct _FRAME* frame);
	void (*Initialize)(struct _FRAME* frame);
} FRAME;

extern const FRAME EmptyFrame;
extern const FRAME StartupFrame;
extern const FRAME HomeFrame;
extern const FRAME TabFrame1;
extern const FRAME TabFrame2;
extern const FRAME TabFrame3;
extern const FRAME TabFrame4;
extern const FRAME TabFrame5;
extern const FRAME TabFrame6;
extern const FRAME TabFrame7;
extern const FRAME TabFrame8;
extern const FRAME TabFrame9;
extern const FRAME TabFrame10;
extern const FRAME WindowFrame;
extern const FRAME LabelFrame1;
extern const FRAME LabelFrame2;
extern const FRAME LabelFrame3;
extern const FRAME LabelFrame4;
extern const FRAME LabelFrame5;
extern const FRAME LabelFrame6;
extern const FRAME LabelFrame7;
extern const FRAME LabelFrame8;
extern const FRAME LabelFrame9;
extern const FRAME LabelFrame10;
extern const FRAME ValueFrame1;
extern const FRAME ValueFrame2;
extern const FRAME ValueFrame3;
extern const FRAME ValueFrame4;
extern const FRAME ValueFrame5;
extern const FRAME ValueFrame6;
extern const FRAME ValueFrame7;
extern const FRAME ValueFrame8;
extern const FRAME ValueFrame9;
extern const FRAME ValueFrame10;
extern const FRAME StatusFrame;
extern const FRAME AlertFrame;

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

	void UI_SetActiveFrame(const FRAME* frame);
	const FRAME* GetFrame(FRAME_ID eID);
	FRAME* UI_GetActiveFrame(void);
	void PaintNow(const FRAME* frame);
	void RepaintNow(const FRAME* frame);
	void MainToParam(void);

#ifdef __cplusplus
}
#endif
#endif
