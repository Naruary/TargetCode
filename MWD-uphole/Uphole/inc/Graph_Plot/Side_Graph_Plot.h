/*******************************************************************************
*       @brief      Provides prototypes for public functions in
*                   Graph_Plot/Side_Graph_Plot.c
*       @file       Uphole/inc/Graph_Plot/Side_Graph_Plot.h
*       @date       Feb 2016
*       @copyright  COPYRIGHT (c) 2016 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef SIDE_GRAPH_PLOT_H
#define SIDE_GRAPH_PLOT_H

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

	void DrawSideGraph(void);
	void DrawPositiveQuadSideGraph(INT16 X_max, INT16 X_min, INT16 Y_max, INT16 Y_min);
	void DrawPositiveNegativeQuadSideGraph(INT16 X_max, INT16 X_min, INT16 Y_max, INT16 Y_min);
	void DrawNegativeQuadSideGraph(INT16 X_max, INT16 X_min, INT16 Y_max, INT16 Y_min);
	void Find_Side_Graph_Scale_Max_Min(void);

#ifdef __cplusplus
}
#endif
#endif
