/*******************************************************************************
*       @brief      This source file contains functions to calculate the
*                   Average Angle Minimum Curvature.
*       @file       Uphole/src/Logging/Calc_AveAngleMinCurve.c
*       @date       Sept 2015
*       @copyright  COPYRIGHT (c) 2014 Target Drilling Inc. All rights are
*                   reserved.  Reproduction in whole or in part is prohibited
*                   without the prior written consent of the copyright holder.
*******************************************************************************/

//============================================================================//
//      INCLUDES                                                              //
//============================================================================//

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include "portable.h"
#include "UI_JobTab.h"
#include "Calc_AveAngleMinCurve.h"


//============================================================================//
//      FUNCTION PROTOTYPES                                                   //
//============================================================================//

//   Initializes angle data to radians
///@param  nAngle - pointer to trigonometric data
///@return void
static void initDataSet(ANGLE_DATA_STRUCT *nAngle);

//REAL32 UD;
//REAL32 LR;
//REAL32 DT;
//REAL32 a;
//REAL32 b;
//REAL32 c;
//REAL32 d;

//============================================================================//
//      FUNCTION IMPLEMENTATIONS                                              //
//============================================================================//

/*!
********************************************************************************
*       @details
*******************************************************************************/

BOOL Calc_AveAngleMinCurve(EASTING_NORTHING_DATA_STRUCT *nResult,
                           POSITION_DATA_STRUCT         *nStarting,
                           POSITION_DATA_STRUCT         *nEnding)
{
    REAL64 fBetavalue;
    REAL64 fWorking;
    REAL64 fHalfMeasuredDepth;

    if((nResult == NULL) || (nStarting == NULL) || (nEnding == NULL)) // check if all data is present
    {
        return false;
    }

    
    if (nEnding->nInclination.fDeg == 0)
    {
        nEnding->nInclination.fDeg += 0.0001;
    }
    
    // If there is no difference in the starting and ending angles then add
    // a very small number to avoid a div by zero error and continue
    if((nStarting->nAzimuth.fDeg == nEnding->nAzimuth.fDeg) &&
       (nStarting->nInclination.fDeg == nEnding->nInclination.fDeg))
    {
        nEnding->nAzimuth.fDeg += 0.0001; // why only Azimuth and not Inclination ?
        nEnding->nInclination.fDeg += 0.0001;
    }    
    
    // Initialize all data to be used for the calculation
    initDataSet(&nStarting->nAzimuth); // convert to radian and calculate sin and cos
    initDataSet(&nStarting->nInclination); // convert to radian and calculate sin and cos

    initDataSet(&nEnding->nAzimuth); // convert to radian and calculate sin and cos
    initDataSet(&nEnding->nInclination); // convert to radian and calculate sin and cos

    // Run through drilling formulas
    // Reference - http://www.drillingformulas.com/tangential-method-calculation/
    fHalfMeasuredDepth = ((nEnding->nPipeLength - nStarting->nPipeLength) * .5); // half measured depth (MD/2)

    fWorking = cos(nEnding->nInclination.fRad - nStarting->nInclination.fRad); // a = cos(cur_pitch - prev_pitch)

    fBetavalue = (1 - (cos(nEnding->nAzimuth.fRad - nStarting->nAzimuth.fRad))); // b = 1 - cos(cur_azi - prev_azi)
    fBetavalue *= (nStarting->nInclination.fSin * nEnding->nInclination.fSin); // b = b * sin(prev_pitch) * sin(cur_pitch)

    fBetavalue = (fWorking - fBetavalue); // beta = a - b
    fBetavalue = acos(fBetavalue); // beta = cos inverse (beta) // dog leg severity calculate as in Functional Specification

    fWorking = ((2 / fBetavalue) * tan(fBetavalue / 2)); // ratio factor // Rf=(2/�) * tan(�/2) // as in Functional Specification

    // Equations given by steve verified with AMT data. These formulas have to sum the previous Up/Down, Left/Right, DT, but thats done in record manager, In function MergeRecordCommon(), boreholeStatistics maintains total Up/Down, Left/Right, Down track (DT)
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Store North delta distance between surveys (Elevation or UP/Down)
    nResult->fNorthing = (REAL32)((fWorking * fHalfMeasuredDepth * (nStarting->nInclination.fSin + nEnding->nInclination.fSin)))*100; // verify if divide by 10 is needed. Check if the stored desired angle is multipled by 10
        
   // UD = (REAL32)((fWorking * fHalfMeasuredDepth * (nStarting->nInclination.fSin + nEnding->nInclination.fSin)))*100;
    // Store East delta distance between surveys (Left/Right)
    nResult->fEasting = (REAL32)((fWorking * fHalfMeasuredDepth * (sin(nStarting->nAzimuth.fRad - ((GetDesiredAzimuth() * (3.14159265358979323846 / 180)) / 10.0)) * cos(nStarting->nInclination.fRad) + sin(nEnding->nAzimuth.fRad - ((GetDesiredAzimuth() * (3.14159265358979323846 / 180)) / 10.0)) * cos(nEnding->nInclination.fRad))))*10; // verify if divide by 10 is needed. Check if the stored desired angle is multipled by 10

   // LR = (REAL32)((fWorking * fHalfMeasuredDepth * (sin(nStarting->nAzimuth.fRad - ((GetDesiredAzimuth() * (3.14159265358979323846 / 180)) / 10.0)) * cos(nStarting->nInclination.fRad) + sin(nEnding->nAzimuth.fRad - ((GetDesiredAzimuth() * (3.14159265358979323846 / 180)) / 10.0)) * cos(nEnding->nInclination.fRad))))*10;
   
    //a = sin(nStarting->nAzimuth.fRad - ((GetDesiredAzimuth() * (3.14159265358979323846 / 180)) / 10.0));
    
    //b = cos(nStarting->nInclination.fRad);
      
    //c = sin(nEnding->nAzimuth.fRad - ((GetDesiredAzimuth() * (3.14159265358979323846 / 180)) / 10.0));
      
    //d = cos(nEnding->nInclination.fRad);
    
    
    // Store Depth delta distance between surveys (Down track or Vertical section)
    nResult->fDepth = (REAL32)((fWorking * fHalfMeasuredDepth * (cos(nStarting->nAzimuth.fRad - ((GetDesiredAzimuth() * (3.14159265358979323846 / 180)) / 10.0)) * cos(nStarting->nInclination.fRad) + cos(nEnding->nAzimuth.fRad - ((GetDesiredAzimuth() * (3.14159265358979323846 / 180)) / 10.0)) * cos(nEnding->nInclination.fRad))))*100; // verify if divide by 10 is needed. Check if the stored desired angle is multipled by 10
    if (nResult->fDepth < 0.0)
    {
        nResult->fDepth = 0.0;
    }
    //DT = (REAL32)((fWorking * fHalfMeasuredDepth * (cos(nStarting->nAzimuth.fRad - ((GetDesiredAzimuth() * (3.14159265358979323846 / 180)) / 10.0)) * cos(nStarting->nInclination.fRad) + cos(nEnding->nAzimuth.fRad - ((GetDesiredAzimuth() * (3.14159265358979323846 / 180)) / 10.0)) * cos(nEnding->nInclination.fRad))))*100;
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //* Coded by walter Standard Min Curve Method*//

    // nResult->fNorthing = (INT32)(fHalfMeasuredDepth * ((nStarting->nInclination.fSin * nStarting->nAzimuth.fCos) + (nEnding->nInclination.fSin * nEnding->nAzimuth.fCos)) * fWorking);

   //  nResult->fEasting = (INT32)(fHalfMeasuredDepth * ((nStarting->nInclination.fSin * nStarting->nAzimuth.fSin) + (nEnding->nInclination.fSin * nEnding->nAzimuth.fSin)) * fWorking);

   //  nResult->fDepth = (INT32)(fHalfMeasuredDepth * (nStarting->nInclination.fCos + nEnding->nInclination.fCos) * fWorking);

   ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    return true;
}

/*!
********************************************************************************
*       @details
*******************************************************************************/

static void initDataSet(ANGLE_DATA_STRUCT *nAngle)
{
    REAL32 fDegToRadConv = 3.14159265358979323846/180.0; // multiplier convert degrees to radians

    //This is used to handle cases when angles are near 0 and we actually
    //want to use positive quadrantal angle values with respect to the
    //0 axis.
    if(nAngle->fDeg >= 359.5)
    {
        nAngle->fDeg = 360 - nAngle->fDeg;
    }

    nAngle->fRad = nAngle->fDeg * fDegToRadConv; // convert degrees to radians
    nAngle->fSin = sin(nAngle->fRad); // sin(x)
    nAngle->fCos = cos(nAngle->fRad); // cos(x)
}
