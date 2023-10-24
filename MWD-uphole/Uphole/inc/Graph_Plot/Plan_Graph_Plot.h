/*******************************************************************************
*       @brief      Provides prototypes for public functions in
*                   Graph_Plot/Plan_Graph_Plot.c
*       @file       Uphole/inc/Graph_Plot/Plan_Graph_Plot.h
*       @date       Feb 2016
*       @copyright  COPYRIGHT (c) 2016 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef PLAN_GRAPH_PLOT_H
#define PLAN_GRAPH_PLOT_H

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

//INT16 Xscale =3;

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

	void DrawPlanGraph(void);
	INT16 Find_X_Scale_Max_Depth(void);
	INT16 Find_X_Scale_Min_Depth(void);
	INT16 Find_Y_Scale_Max_East(void);
	INT16 Find_Y_Scale_Min_East(void);
	void DrawPositiveQuadPlanGraph(INT16 X_max, INT16 X_min, INT16 Y_max, INT16 Y_min);
	void DrawPositiveNegativeQuadPlanGraph(INT16 X_max, INT16 X_min, INT16 Y_max, INT16 Y_min);
	void DrawNegativeQuadPlanGraph(INT16 X_max, INT16 X_min, INT16 Y_max, INT16 Y_min);
	void Find_Plan_Graph_Scale_Max_Min(void);

#ifdef __cplusplus
}
#endif
#endif
