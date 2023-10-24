/*******************************************************************************
*       @brief      Provides prototypes for public functions in
*                   Graph_Plot/Graph_Plot.c
*       @file       Uphole/inc/Graph_Plot/Graph_Plot.h
*       @date       Feb 2016
*       @copyright  COPYRIGHT (c) 2016 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef GRAPH_PLOT_H
#define GRAPH_PLOT_H

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

struct GraphPlotArea
{
	INT16 Top_Left_X;
	INT16 Top_Left_Y;
	INT16 Top_Right_X;
	INT16 Top_Right_Y;
	INT16 Bottom_Left_X;
	INT16 Bottom_Left_Y;
	INT16 Bottom_Right_X;
	INT16 Bottom_Right_Y;
};

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

	INT16 Round_Down_10(INT16 num);
	INT16 Round_Up_10(INT16 num);
	void PlotGraph(void);

#ifdef __cplusplus
}
#endif
#endif
