/*******************************************************************************
*       @brief      Provides prototypes for public functions in
*                   Graph_Plot/SideGammaGamma_Graph_Plot.c
*       @file       Uphole/inc/Graph_Plot/SideGamma_Graph_Plot.h
*       @date       Feb 2016
*       @copyright  COPYRIGHT (c) 2016 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef SIDE_GAMMA_GRAPH_PLOT_H
#define SIDE_GAMMA_GRAPH_PLOT_H

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

	void DrawSideGammaGraph(void);
	void DrawPositiveQuadSideGammaGraph(INT16 X_max, INT16 X_min, INT16 Y_max, INT16 Y_min);
	void DrawPositiveNegativeQuadSideGammaGraph(INT16 X_max, INT16 X_min, INT16 Y_max, INT16 Y_min);
	void DrawNegativeQuadSideGammaGraph(INT16 X_max, INT16 X_min, INT16 Y_max, INT16 Y_min);
	void Find_SideGamma_Graph_Scale_Max_Min(void);
	void DrawPositiveQuadGammaLineSuperimpose(INT16 X_max_PL, INT16 X_min_PL, INT16 Y_max_GAMMA, INT16 Y_min_GAMMA);
//	void Find_Gamma_Scale_Max_Min(void);

#ifdef __cplusplus
}
#endif
#endif
