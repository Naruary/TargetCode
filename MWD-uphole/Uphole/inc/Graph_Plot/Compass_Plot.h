/*******************************************************************************
*       @brief      Provides prototypes for public functions in
*                   Graph_Plot/Compass_Plot.c
*       @file       Uphole/inc/Graph_Plot/Compass_Plot.h
*       @date       Feb 2016
*       @copyright  COPYRIGHT (c) 2016 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef COMPASS_PLOT_H
#define COMPASS_PLOT_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"

//============================================================================//
//      CONSTANTS                                                             //
//============================================================================//


//============================================================================//
//      MACROS                                                                //
//============================================================================//



//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//


//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

	void DrawCompass(void);
	void DrawAzimuthCompass(void);
	void DrawRollCompass(void);
	void DrawPitchCompass(void);
	void drawScreenBorder(void);

#ifdef __cplusplus
}
#endif
#endif
