/*******************************************************************************
*       @brief      This header file contains callable functions to calculate
*                   Average Angle Minimum Curvature.
*       @file       Uphole/inc/Logging/Calc_AveAngleMinCurve.h
*       @date       July 2014
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

#ifndef CALC_AVE_ANGLE_MIN_CURVE_H
#define CALC_AVE_ANGLE_MIN_CURVE_H

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include "portable.h"

//============================================================================//
//      DATA DECLARATIONS                                                     //
//============================================================================//

typedef struct __ANGLE_DATA_STRUCT__
{
	REAL64  fDeg;
	REAL64  fRad;
	REAL64  fSin;
	REAL64  fCos;
} ANGLE_DATA_STRUCT;

typedef struct __POSITION_DATA_STRUCT__
{
	ANGLE_DATA_STRUCT   nAzimuth;
	ANGLE_DATA_STRUCT   nInclination;
	INT32               nPipeLength;
} POSITION_DATA_STRUCT;

typedef struct __EASTING_NORTHING_DATA_STRUCT__
{
	REAL32 fEasting;
	REAL32 fNorthing;
	REAL32 fDepth;
} EASTING_NORTHING_DATA_STRUCT;

//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

#ifdef __cplusplus
extern "C" {
#endif

    //  Calculates the Average Angle Minimum Curvature
    ///@param nResult   - pointer to Easting and Northing Data
    ///@param nStarting - pointer to starting location borehole data
    ///@param nEnding   - pointer to ending location borehole data
    ///@return BOOL
    BOOL Calc_AveAngleMinCurve(EASTING_NORTHING_DATA_STRUCT *nResult,
                               POSITION_DATA_STRUCT         *nStarting,
                               POSITION_DATA_STRUCT         *nEnding);

#ifdef __cplusplus
}
#endif
#endif
