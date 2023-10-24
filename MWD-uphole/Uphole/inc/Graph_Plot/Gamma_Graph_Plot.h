/*******************************************************************************
*       @brief      Provides prototypes for public functions in
*                   Graph_Plot/Side_Graph_Plot.c
*       @file       Uphole/inc/Graph_Plot/Gamma_Graph_Plot.h
*       @date       June 2016
*       @copyright  COPYRIGHT (c) 2016 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef GAMMA_GRAPH_PLOT_H
#define GAMMA_GRAPH_PLOT_H

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

	void DrawGammaGraph(void);
	void DrawPositiveQuadGammaGraph(INT16 X_max, INT16 X_min, INT16 Y_max, INT16 Y_min);
	void DrawPositiveNegativeQuadGammaGraph(INT16 X_max, INT16 X_min, INT16 Y_max, INT16 Y_min);
	void DrawNegativeQuadGammaGraph(INT16 X_max, INT16 X_min, INT16 Y_max, INT16 Y_min);
	void Find_Gamma_Graph_Scale_Max_Min(void);

#ifdef __cplusplus
}
#endif
#endif
